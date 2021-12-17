#include "beast-common.hpp"

class ClientSession : public std::enable_shared_from_this<ClientSession> {
public:
    using WS = beast::websocket::stream<tcp::socket>;
    WS m_ws;

    std::string m_host;

    beast::flat_buffer m_readBuf;

    bool m_sleep;

    int m_numPackets = 0;
    std::chrono::system_clock::time_point m_start;
    size_t m_total = 0;

    ClientSession(net::io_context& ctx, bool sleep)
        : m_ws(ctx)
        , m_sleep(sleep)
    {}

    void onConnectionEstablished(beast::error_code e) {
        if (e) return failed(e, "establish");
        m_start = std::chrono::system_clock::now();
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

        auto time = std::chrono::system_clock::now() - m_start;
        auto ms = int(std::chrono::duration_cast<std::chrono::milliseconds>(time).count());
        m_total += m_readBuf.size();
        ++m_numPackets;
        printf("%d. At %6d client received %.1f KB (total %.1f KB)\n", m_numPackets, ms, double(m_readBuf.size())/1024, double(m_total)/1024);
        if (m_sleep) {
            std::cout << "  Sleeping now...\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

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


void runClient(std::string addr, std::string port, bool sleep) {
    printf("Starting client on %s:%s (sleep: %d)\n", addr.c_str(), port.c_str(), sleep);

    net::io_context ctx(1);
    tcp::resolver resolver{ctx};

    auto results = resolver.resolve(tcp::v4(), addr, port);
    if (results.empty())
    {
        std::cerr << "Could not resolve " << addr << '\n';
        return;
    }

    ClientSession session(ctx, sleep);
    session.m_host = addr;
    session.m_host += ':';
    session.m_host += port;
    session.connect(results.begin()->endpoint());

    ctx.run();
}