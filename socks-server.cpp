#include "socks-server.inl"

int main(int argc, char* argv[]) {
    uint16_t port = 7654;
    int packets = 20;

    if (argc > 1) {
        if (argc != 3) {
            printf("Usage: %s <port> <packets>\n", argv[0]);
            return -1;
        }
        port = atoi(argv[1]);
        packets = atoi(argv[2]);
    }

    return server(port, packets);
}
