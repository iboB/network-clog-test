#include "beast-common.hpp"

class ServerSession : public std::enable_shared_from_this<ServerSession> {
public:
    using WS = beast::websocket::stream<tcp::socket>;
    WS m_ws;

    std::vector<uint8_t> m_curBuf; // buffer which is being sent/written
    int numSent = 0;

    beast::flat_buffer m_readBuf; // unused

    int m_numPackets;
    std::chrono::system_clock::time_point m_start;
    size_t m_total = 0;

    ServerSession(tcp::socket socket, int numPackets)
        : m_ws(std::move(socket))
        , m_numPackets(numPackets)
    {}

    static std::vector<uint8_t> getRandomBuf() {
        std::vector<uint8_t> buf;
        buf.reserve(500 * 1024);
        for (size_t i = 0; i < buf.capacity(); ++i) {
            buf.push_back(rand() % 256);
        }
        return buf;
    }

    void doSend() {
        std::cout << "Server sending blob\n";
        m_curBuf = getRandomBuf();
        m_ws.text(false);
        m_ws.async_write(
            net::buffer(m_curBuf.data(), m_curBuf.size()),
            beast::bind_front_handler(&ServerSession::onCompletedSend, shared_from_this()));
    }

    void onCompletedSend(beast::error_code e, size_t) {
        if (e) return failed(e, "write");
        std::cout << "Server sending blob\n";
        std::cout << "  Server completed send of blob\n";
        ++numSent;
        if (numSent >= 20)
        {
            std::cout << "Server stopped sending\n";
            return;
        }
        doSend();
    }

    void onConnectionEstablished(beast::error_code e) {
        if (e) return failed(e, "establish");
        //doSend();

        m_start = std::chrono::system_clock::now();

        for (int i=0; i<m_numPackets; ++i) {
            beast::error_code e;
            auto vec = getRandomBuf();
            printf("%d. Server sending %.1f KB blob\n", i, double(vec.size())/1024);
            m_ws.text(false);
            m_ws.write(
                net::buffer(vec.data(), vec.size()),
                e
            );
            m_total += vec.size();
            printf("  Server completed send of blob (total sent %.1f KB)\n", double(m_total)/1024);
            auto t = std::chrono::system_clock::now() - m_start;
            printf("T %d ms\n", int(std::chrono::duration_cast<std::chrono::milliseconds>(t).count()));
            if (e) return failed(e, "write");
        }

        auto time = std::chrono::system_clock::now() - m_start;
        printf("Server sending is DONE for %d ms\n", int(std::chrono::duration_cast<std::chrono::milliseconds>(time).count()));
        fflush(stdout);

        doRead();
    }

    void accept() {
        m_ws.async_accept(beast::bind_front_handler(&ServerSession::onConnectionEstablished, shared_from_this()));
    }

    void onRead(beast::error_code e, size_t)
    {
        if (e == beast::websocket::error::closed) return closed();
        if (e) return failed(e, "read");
        doRead();
    }

    void doRead() {
        m_ws.async_read(m_readBuf, beast::bind_front_handler(&ServerSession::onRead, shared_from_this()));
    }

    void failed(beast::error_code e, const char* source) {
        std::cerr << source << " error: " << e.message() << '\n';
    }

    void closed() {
        std::cout << "session closed\n";
    }
};

class Server {
public:
    net::io_context m_ctx;
    tcp::acceptor m_acceptor;
    std::thread m_thread;

    int m_numPackets;

    Server(uint16_t port, int numPackets)
        : m_ctx(1)
        , m_acceptor(m_ctx, tcp::endpoint(tcp::v4(), port))
        , m_numPackets(numPackets)
    {
        printf("Starging server on port %d (blobs to send %d)\n", port, numPackets);
        doAccept();
        m_thread = std::thread([this]() { m_ctx.run(); });
    }

    ~Server()
    {
        m_ctx.stop();
        m_thread.join();
    }

    void doAccept() {
        m_acceptor.async_accept(net::make_strand(m_ctx), beast::bind_front_handler(&Server::onAccept, this));
    }

    void onAccept(beast::error_code e, tcp::socket socket) {
        if (e)
        {
            std::cerr << "onAccept error: " << e << '\n';
            return;
        }

        // init session and owner
        auto session = std::make_shared<ServerSession>(std::move(socket), m_numPackets);
        session->accept();

        // accept more sessions
        doAccept();
    }
};
