#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <thread>
#include <cstdio>
#include <vector>
#include <cstdlib>

int client() {
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0) return puts("socket fail");

    sockaddr_in client = {};
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = inet_addr("127.0.0.1");
#if PROXY
    client.sin_port = htons(9654);
#else
    client.sin_port = htons(7654);
#endif

    if (connect(sd, (sockaddr*)&client, sizeof(client)) < 0) {
        return puts("connect fail");
    }

    {
        int data;
        socklen_t size = sizeof(data);
        getsockopt(sd, SOL_SOCKET, SO_RCVBUF, &data, &size);
        printf("connected: %d\n", int(data));
    }

    std::vector<uint8_t> buf(1024*1024);
    while (true) {
        auto s = recv(sd, buf.data(), buf.size(), 0);
        if (s <= 0) {
            puts("recv fail");
            break;
        }
        printf("Client received %.1f KB\n", double(s)/1024);
#if !PROXY
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
#endif
    }

    return close(sd);
}