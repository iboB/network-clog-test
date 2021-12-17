#define PROXY 0

#include "socks-server.inl"
#include "socks-client.inl"

int main() {
    std::thread srv(server);
    std::this_thread::sleep_for(std::chrono::milliseconds(300)); // give time for the server to start
    client();
    srv.join();
    return 0;
}
