#include "socks-client.inl"

int main(int argc, char* argv[]) {
    const char* addr = "127.0.0.1";
    uint16_t port = 7654;
    bool sleep = true;
    if (argc > 1) {
        if (argc == 2) {
            printf("Usage: %s <addr> <port> [sleep]\n");
            return -1;
        }
        addr = argv[1];
        port = atoi(argv[2]);
        if (argc == 4) {
            sleep = atoi(argv[3]) == 1;
        }
        if (argc > 4) {
            printf("Usage: %s <addr> <port> [sleep]\n");
            return -1;
        }
    }

    return client(addr, port, sleep);
}
