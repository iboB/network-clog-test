#include "socks-server.inl"
#include "socks-client.inl"

int main() {
    std::thread srv([]() { server(7654, 20); });
    std::this_thread::sleep_for(std::chrono::milliseconds(300)); // give time for the server to start
    client("127.0.0.1", 7654, true);
    srv.join();
    return 0;
}
