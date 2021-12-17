#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <thread>
#include <cstdio>
#include <vector>
#include <cstdlib>

int client(const char* addr, uint16_t port, bool sleep) {
    printf("Starting client on %s:%d (sleep: %d)\n", addr, port, sleep);

    int sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0) return puts("socket fail");

    sockaddr_in client = {};
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = inet_addr(addr);
    client.sin_port = htons(port);

    if (connect(sd, (sockaddr*)&client, sizeof(client)) < 0) {
        return puts("connect fail");
    }

    {
        int data;
        socklen_t size = sizeof(data);
        getsockopt(sd, SOL_SOCKET, SO_RCVBUF, &data, &size);
        printf("connected: %d\n", int(data));
    }

    std::vector<uint8_t> buf(600*1024);
    ssize_t total;
    int nrec = 0;
    while (true) {
        auto s = recv(sd, buf.data(), buf.size(), 0);
        if (s < 0) {
            puts("recv fail");
            break;
        }
        if (s == 0) {
            std::this_thread::yield();
            continue;
        }
        total += s;
        ++nrec;
        printf("%d. Client received %.1f KB (total %.1f KB)\n", nrec, double(s)/1024, double(total)/1024);
        if (sleep) std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    return close(sd);
}