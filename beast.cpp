#define BOOST_BEAST_USE_STD_STRING_VIEW 1
#define BOOST_ASIO_USE_TS_EXECUTOR_AS_DEFAULT 1

#if defined(_MSC_VER)
#   pragma warning (disable: 4100)
#endif
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>

#include <iostream>
#include <chrono>
#include <thread>
#include <vector>

#define PROXY 1

namespace net = boost::asio;
namespace beast = boost::beast;
using tcp = net::ip::tcp;

class ServerSession : public std::enable_shared_from_this<ServerSession> {
public:
    using WS = beast::websocket::stream<tcp::socket>;
    WS m_ws;

    std::vector<uint8_t> m_curBuf; // buffer which is being sent/written
    int numSent = 0;

    beast::flat_buffer m_readBuf; // unused

    ServerSession(tcp::socket socket)
        : m_ws(std::move(socket))
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

        for (int i=0; i<20; ++i) {
            beast::error_code e;
            auto vec = getRandomBuf();
            m_ws.text(false);
            m_ws.write(
                net::buffer(vec.data(), vec.size()),
                e
            );
            std::cout << "  Server completed send of blob\n";
            if (e) return failed(e, "write");
        }

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

    Server()
        : m_ctx(1)
        , m_acceptor(m_ctx, tcp::endpoint(tcp::v4(), 7654))
    {
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
        auto session = std::make_shared<ServerSession>(std::move(socket));
        session->accept();

        // accept more sessions
        doAccept();
    }
};


class ClientSession : public std::enable_shared_from_this<ClientSession> {
public:
    using WS = beast::websocket::stream<tcp::socket>;
    WS m_ws;

    std::string m_host;

    beast::flat_buffer m_readBuf;

    ClientSession(net::io_context& ctx)
        : m_ws(ctx)
    {}

    void onConnectionEstablished(beast::error_code e) {
        if (e) return failed(e, "establish");
        doRead();
    }

    void onConnect(beast::error_code e) {
        if (e) return failed(e, "ws connect");

        m_ws.async_handshake(m_host, "/",
            beast::bind_front_handler(&ClientSession::onConnectionEstablished, this));
    }

    void connect(tcp::endpoint endpoint) {
        beast::get_lowest_layer(m_ws).async_connect(endpoint,
            beast::bind_front_handler(&ClientSession::onConnect, this));
    }

    void onRead(beast::error_code e, size_t)
    {
        if (e == beast::websocket::error::closed) return closed();
        if (e) return failed(e, "read");

        std::cout << "Client received blob of size " << double(m_readBuf.size()) / 1024 << " KB\n";
#if !PROXY
        std::cout << "  Sleeping now...\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
#endif

        m_readBuf.clear();
        doRead();
    }

    void doRead() {
        m_ws.async_read(m_readBuf, beast::bind_front_handler(&ClientSession::onRead, this));
    }

    void failed(beast::error_code e, const char* source) {
        std::cerr << source << " error: " << e.message() << '\n';
    }

    void closed() {
        std::cout << "session closed\n";
    }
};


void runClient() {
    net::io_context ctx(1);

    std::string addr = "localhost";
    std::string port =
#if PROXY
        "9654";
#else
        "7654";
#endif
    tcp::resolver resolver{ctx};

    auto results = resolver.resolve(tcp::v4(), addr, port);
    if (results.empty())
    {
        std::cerr << "Could not resolve " << addr << '\n';
        return;
    }

    ClientSession session(ctx);
    session.m_host = addr;
    session.m_host += ':';
    session.m_host += port;
    session.connect(results.begin()->endpoint());

    ctx.run();
}

int main() {
    Server srv;

    runClient();

    return 0;
}
