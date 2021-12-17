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
    for (size_t i = 0; i < buf.capacity(); ++i) {
        buf.push_back(rand() % 256);
    }
    return buf;
}

void server() {
    auto sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0) {
        puts("socket fail");
        return;
    }

    sockaddr_in srv = {};
    srv.sin_family = AF_INET;
    srv.sin_addr.s_addr = INADDR_ANY;
    srv.sin_port = htons(7654);

    int enable = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        puts("setsockopt fail");
        return;
    }

    if (bind(sd, (sockaddr*)&srv, sizeof(srv)) < 0) {
        puts("bind fail");
        return;
    }

    listen(sd, 3);

    puts("listening...");

    sockaddr_in client;
    socklen_t csz = sizeof(client);
    auto sock = accept(sd, (sockaddr*)&client, &csz);
    if (sock < 0) {
        perror("accept fail");
        return;
    }

    puts("accepted");

    for (int i=0; i<20; ++i) {
        auto buf = getRandomBuf();
        puts("Server sending blob");
        send(sock, buf.data(), buf.size(), 0);
        puts("  Server completed send of blob");
    }

    while (true) std::this_thread::yield();

    close(sock);
}

void client() {
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0) {
        puts("socket fail");
        return;
    }

    sockaddr_in client = {};
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = inet_addr("127.0.0.1");
    client.sin_port = htons(7654);

    if (connect(sd, (sockaddr*)&client, sizeof(client)) < 0) {
        puts("connect fail");
        return;
    }

    puts("connected");

    std::vector<uint8_t> buf(1024*1024);
    while (true) {
        auto s = recv(sd, buf.data(), buf.size(), 0);
        if (s < 0) {
            puts("recv fail");
            break;
        }
        printf("Client received %.1f KB\n", double(s)/1024);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    close(sd);
}

int main() {
    std::thread srv(server);

    client();

    srv.join();
    return 0;
}
