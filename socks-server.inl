#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <thread>
#include <cstdio>
#include <vector>
#include <cstdlib>

static std::vector<uint8_t> getRandomBuf() {
    std::vector<uint8_t> buf;
    buf.reserve(500 * 1024);
    for (size_t i = 0; i < buf.capacity(); ++i) buf.push_back(rand() % 256);
    return buf;
}

int server(uint64_t port, const int blobs) {
    printf("Starging server on port %d (blobs to send %d)\n", port, blobs);
    auto sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0) return puts("socket fail");

    sockaddr_in srv = {};
    srv.sin_family = AF_INET;
    srv.sin_addr.s_addr = INADDR_ANY;
    srv.sin_port = htons(port);

    int enable = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        return puts("setsockopt fail");
    }

    if (bind(sd, (sockaddr*)&srv, sizeof(srv)) < 0) {
        return puts("bind fail");
    }

    listen(sd, 3);

    puts("listening...");

    sockaddr_in client;
    socklen_t csz = sizeof(client);
    auto sock = accept(sd, (sockaddr*)&client, &csz);
    if (sock < 0) return puts("accept fail");

    {
        int data;
        socklen_t size = sizeof(data);
        getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &data, &size);
        printf("accepted: %d\n", int(data));
    }

    size_t total = 0;
    for (int i=0; i<blobs; ++i) {
        auto buf = getRandomBuf();
        printf("%d. Server sending %.1f KB blob\n", i, double(buf.size())/1024);
        send(sock, buf.data(), buf.size(), 0);
        total += buf.size();
        printf("  Server completed send of blob (total sent %.1f KB)\n", double(total)/1024);
    }

    puts("Server sending is DONE");

    while (true) std::this_thread::yield();

    return close(sock);
}