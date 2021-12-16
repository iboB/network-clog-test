#include <fishnets/WebSocketClient.hpp>
#include <fishnets/WebSocketServer.hpp>
#include <fishnets/WebSocketSession.hpp>

#include <cstdio>
#include <chrono>
#include <thread>
#include <vector>

class ClientSession final : public fishnets::WebSocketSession
{
    void wsOpened() override {}

    void wsClosed() override {}

    void wsReceivedBinary(itlib::memory_view<uint8_t> blob) override
    {
        printf("Client received blob of size %.1f KB\n   Sleeping now...\n", double(blob.size())/1024);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    void wsReceivedText(itlib::memory_view<char>) override {}

    void wsCompletedSend() override {}
};

class ServerSession final : public fishnets::WebSocketSession
{
    static std::vector<uint8_t> getRandomBuf()
    {
        std::vector<uint8_t> buf;
        buf.reserve(500 * 1024);
        for (size_t i = 0; i < buf.capacity(); ++i)
        {
            buf.push_back(rand() % 256);
        }
        return buf;
    }

    void doSend()
    {
        printf("Server sending blob\n");
        m_curBuf = getRandomBuf();
        wsSend(itlib::make_memory_view(m_curBuf));
    }

    void wsOpened() override
    {
        doSend();
    }

    void wsClosed() override {}

    void wsReceivedBinary(itlib::memory_view<uint8_t>) override {}

    void wsReceivedText(itlib::memory_view<char>) override {}

    void wsCompletedSend() override
    {
        printf("Server completed send of blob\n");
        ++numSent;
        if (numSent >= 20)
        {
            printf("Server stopped sending\n");
            return;
        }
        doSend();
    }

    int numSent = 0;
    std::vector<uint8_t> m_curBuf;
};

fishnets::WebSocketSessionPtr Make_ServerSession(const fishnets::WebSocketEndpointInfo&)
{
    return std::make_shared<ServerSession>();
}

int main()
{
    fishnets::WebSocketServer server(Make_ServerSession, 1234, 1);
    fishnets::WebSocketClient client(std::make_shared<ClientSession>(), "localhost", 1234);


    while (true) std::this_thread::yield();
    return 0;
}