#include <iostream>
#include <vector>
#include <poll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <map>
#include "../structure/protocol.hpp"
#include "../structure/order_manager.hpp"
#include "../structure/session.hpp"

#define PORT 8080
#define MAX_CLIENTS 8
#define RATE_LIMIT 100

bool isRateLimited(ClientSession& session) {
    auto now = std::chrono::steady_clock::now();

    while (!session.messageTimes.empty() &&
           std::chrono::duration_cast<std::chrono::seconds>(
               now - session.messageTimes.front()).count() >= 1) {
        session.messageTimes.pop_front();
    }

    if (session.messageTimes.size() >= RATE_LIMIT)
        return true;

    session.messageTimes.push_back(now);
    return false;
}

void handleClient(ClientSession& session, OrderManager& om) {
    while (session.buffer.size() >= 2) {
        uint16_t len = *reinterpret_cast<uint16_t*>(session.buffer.data());
        if (session.buffer.size() < len) break;

        uint16_t type = *reinterpret_cast<uint16_t*>(session.buffer.data() + 2);

        if (isRateLimited(session)) {
            OrderReject rej{};
            rej.messageType = (type == 1) ? 3 : 4;

            if (type == 1) {
                auto* req = reinterpret_cast<NewOrderRequest*>(session.buffer.data());
                rej.price = req->price;
                rej.size = req->size;
                rej.clientOrderId = req->clientOrderId;
                rej.serverOrderId = -1;
            } else {
                auto* req = reinterpret_cast<CancelOrderRequest*>(session.buffer.data());
                rej.price = req->price;
                rej.size = req->size;
                rej.clientOrderId = -1;
                rej.serverOrderId = req->serverOrderId;
            }

            send(session.fd, &rej, sizeof(rej), 0);
        }
        else if (type == 1) {
            auto* req = reinterpret_cast<NewOrderRequest*>(session.buffer.data());
            uint64_t sid = om.addOrder(*req);

            OrderAck ack{};
            ack.messageType = 1;
            ack.price = req->price;
            ack.size = req->size;
            ack.clientOrderId = req->clientOrderId;
            ack.serverOrderId = sid;

            send(session.fd, &ack, sizeof(ack), 0);
        }
        else if (type == 2) {
            auto* req = reinterpret_cast<CancelOrderRequest*>(session.buffer.data());
            int32_t outClientId;

            if (om.tryCancel(*req, outClientId)) {
                OrderAck ack{};
                ack.messageType = 2;
                ack.price = req->price;
                ack.size = req->size;
                ack.clientOrderId = outClientId;
                ack.serverOrderId = req->serverOrderId;
                send(session.fd, &ack, sizeof(ack), 0);
            } else {
                OrderReject rej{};
                rej.messageType = 4;
                rej.price = req->price;
                rej.size = req->size;
                rej.clientOrderId = outClientId;
                rej.serverOrderId = req->serverOrderId;
                send(session.fd, &rej, sizeof(rej), 0);
            }
        }

        session.buffer.erase(session.buffer.begin(), session.buffer.begin() + len);
    }
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    bind(server_fd, (sockaddr*)&addr, sizeof(addr));
    listen(server_fd, MAX_CLIENTS);

    std::vector<pollfd> fds;
    fds.push_back({server_fd, POLLIN, 0});

    std::map<int, ClientSession> sessions;
    OrderManager om;

    while (true) {
        poll(fds.data(), fds.size(), -1);

        for (size_t i = 0; i < fds.size(); ++i) {
            if (!(fds[i].revents & POLLIN)) continue;

            if (fds[i].fd == server_fd) {
                int cfd = accept(server_fd, nullptr, nullptr);
                if (fds.size() <= MAX_CLIENTS) {
                    fds.push_back({cfd, POLLIN, 0});
                    sessions[cfd] = {cfd, {}, {}};
                } else close(cfd);
            } else {
                char buf[1024];
                int n = read(fds[i].fd, buf, sizeof(buf));
                if (n <= 0) {
                    close(fds[i].fd);
                    sessions.erase(fds[i].fd);
                    fds.erase(fds.begin() + i);
                } else {
                    auto& s = sessions[fds[i].fd];
                    s.buffer.insert(s.buffer.end(), buf, buf + n);
                    handleClient(s, om);
                }
            }
        }
    }
}
