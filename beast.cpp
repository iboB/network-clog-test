#include "beast-server.inl"
#include "beast-client.inl"


int main() {
    Server srv(7654, 20);

    runClient("localhost", "7654", true);

    return 0;
}
