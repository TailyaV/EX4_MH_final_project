#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "pipling.hpp"
#include "graph.hpp"
#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include <atomic>
#include <cerrno>
#include <csignal>
#include <system_error>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

// SUT
std::string to_string(Pipling::Result res);
bool my_handler(int new_socket);
int  bind_listen(int port);

//Globally ignore SIGPIPE exactly once so writes to closed sockets return EPIPE instead of terminating the process.
static void ignore_sigpipe_once() {
    static std::once_flag f;
    std::call_once(f, []{ ::signal(SIGPIPE, SIG_IGN); });
}

//Install a signal handler without SA_RESTART so blocking syscalls can return EINTR instead of auto-restarting.
static void install_sig_no_restart(int signo, void (*handler)(int)) {
    struct sigaction sa{};
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; // NO SA_RESTART → syscalls may return EINTR
    sigaction(signo, &sa, nullptr);
}

//No-op signal handler used to interrupt blocking syscalls and trigger EINTR in tests.
static void empty_sig(int){}

//Reliably write a 32-bit int (host endianness) to 'fd', retrying on EINTR until all bytes are sent.
static void send_int(int fd, int x) {
    const char* p = reinterpret_cast<const char*>(&x);
    size_t left = sizeof(x);
    while (left) {
        ssize_t w = ::write(fd, p, left);
        if (w < 0) { if (errno == EINTR) continue; break; }
        p += w; left -= (size_t)w;
    }
}

// Read from fd until we see 'delim' (or timeout). Returns true and writes
// the full payload (including delim if keep_delim=true) into 'out'.
static bool read_until_delim(int fd, char delim, std::string& out,
                             int timeout_ms = 2000, bool keep_delim = false)
{
    out.clear();
    char buf[1024];
    int elapsed = 0;
    const int step = 20; // ms

    while (elapsed <= timeout_ms) {
        // If delim already in buffer (from previous read), finish
        auto pos = out.find(delim);
        if (pos != std::string::npos) {
            if (!keep_delim) out.erase(pos); // drop delim
            return true;
        }

        struct pollfd pfd{fd, POLLIN, 0};
        int pr = ::poll(&pfd, 1, step);
        if (pr < 0) { if (errno == EINTR) continue; return false; }
        if (pr == 0) { elapsed += step; continue; } // timeout slice

        ssize_t n = ::read(fd, buf, sizeof(buf));
        if (n == 0) {
            // Peer closed before delim; return what we have (treat as failure)
            return false;
        }
        if (n < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        out.append(buf, buf + n);
    }
    return false; // overall timeout
}

// Find an ephemeral free TCP port
static int find_free_port() {
    int s = ::socket(AF_INET, SOCK_STREAM, 0);
    REQUIRE(s >= 0);
    sockaddr_in a{};
    a.sin_family = AF_INET;
    a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    a.sin_port = htons(0);
    REQUIRE(::bind(s, (sockaddr*)&a, sizeof(a)) == 0);
    socklen_t len = sizeof(a);
    REQUIRE(::getsockname(s, (sockaddr*)&a, &len) == 0);
    int port = ntohs(a.sin_port);
    ::close(s);
    return port;
}


// to_string formatting
TEST_CASE("to_string formats fields and SCC list") {
    Pipling::Result r;
    r.mst_weight  = 7;
    r.num_cliques = 3;
    r.sccs = {{0,1},{2}};
    r.max_flow    = 2;

    std::string s = to_string(r);
    CHECK(s.find("MST weight:") != std::string::npos);
    CHECK(s.find("number of cliques:") != std::string::npos);
    CHECK(s.find("Strongly Connected Components:") != std::string::npos);
    CHECK(s.find("max flow:") != std::string::npos);
    CHECK(s.back() == '}');
    CHECK(s.find("0 1 \n") != std::string::npos);
    CHECK(s.find("2 \n") != std::string::npos);
}

// choice==1 — manual graph; read until '}'
TEST_CASE("my_handler: choice=1 (manual graph) produces a delimited response") {
    ignore_sigpipe_once();
    int sp[2]; REQUIRE(::socketpair(AF_UNIX, SOCK_STREAM, 0, sp) == 0);
    int srv = sp[0], cli = sp[1];

    std::thread t([&]{ CHECK(my_handler(srv) == true); });

    send_int(cli, 1);
    send_int(cli, 3);
    send_int(cli, 0); send_int(cli, 1);
    send_int(cli, 1); send_int(cli, 2);
    send_int(cli, -1); send_int(cli, -1);

    std::string resp;
    REQUIRE(read_until_delim(cli, '}', resp));
    ::close(cli);
    t.join();

    CHECK(resp.find("MST weight:") != std::string::npos);
    CHECK(resp.find("number of cliques:") != std::string::npos);
    CHECK(resp.find("Strongly Connected Components:") != std::string::npos);
    CHECK(resp.find("max flow:") != std::string::npos);
}

// choice==2 — random graph; read until '}'
TEST_CASE("my_handler: choice=2 (random graph) produces a delimited response") {
    ignore_sigpipe_once();
    int sp[2]; REQUIRE(::socketpair(AF_UNIX, SOCK_STREAM, 0, sp) == 0);
    int srv = sp[0], cli = sp[1];

    std::thread t([&]{ CHECK(my_handler(srv) == true); });

    send_int(cli, 2);
    send_int(cli, 4); // vertices
    send_int(cli, 0); // edges
    send_int(cli, 1); // seed

    std::string resp;
    REQUIRE(read_until_delim(cli, '}', resp));
    ::close(cli);
    t.join();

    CHECK(resp.find("MST weight:") != std::string::npos);
    CHECK(resp.find("max flow:") != std::string::npos);
}

// choice==0 — no work
TEST_CASE("my_handler: choice=0 returns false (no work)") {
    int sp[2]; REQUIRE(::socketpair(AF_UNIX, SOCK_STREAM, 0, sp) == 0);
    int srv = sp[0], cli = sp[1];

    send_int(cli, 0);
    ::shutdown(cli, SHUT_WR);
    ::close(cli);

    CHECK(my_handler(srv) == false);
}

// invalid choice — throws
TEST_CASE("my_handler: invalid choice throws invalid_argument") {
    int sp[2]; REQUIRE(::socketpair(AF_UNIX, SOCK_STREAM, 0, sp) == 0);
    int srv = sp[0], cli = sp[1];

    send_int(cli, 999);
    ::shutdown(cli, SHUT_WR);
    ::close(cli);

    CHECK_THROWS_AS(my_handler(srv), std::invalid_argument);
}

// EINTR during first read, then valid request; read response until '}'
TEST_CASE("my_handler: EINTR during read is retried (choice=1)") {
    ignore_sigpipe_once();
    install_sig_no_restart(SIGALRM, empty_sig);

    int sp[2]; REQUIRE(::socketpair(AF_UNIX, SOCK_STREAM, 0, sp) == 0);
    int srv = sp[0], cli = sp[1];

    std::thread writer([&]{
        ualarm(6000, 0); // interrupt the first blocking read
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        send_int(cli, 1);
        send_int(cli, 3);
        send_int(cli, 0); send_int(cli, 1);
        send_int(cli, 1); send_int(cli, 2);
        send_int(cli, -1); send_int(cli, -1);

        std::string resp;
        REQUIRE(read_until_delim(cli, '}', resp));
        ::close(cli);
    });

    CHECK(my_handler(srv) == true);
    writer.join();
}

// Peer closes before server writes → write_all fails; handler still exits
TEST_CASE("my_handler: peer closes early (EPIPE path) doesn't hang") {
    ignore_sigpipe_once();

    int sp[2]; REQUIRE(::socketpair(AF_UNIX, SOCK_STREAM, 0, sp) == 0);
    int srv = sp[0], cli = sp[1];

    std::thread t([&]{ CHECK(my_handler(srv) == true); });

    // Send a full request, then close immediately so server's write hits EPIPE
    send_int(cli, 1);
    send_int(cli, 3);
    send_int(cli, 0); send_int(cli, 1);
    send_int(cli, 1); send_int(cli, 2);
    send_int(cli, -1); send_int(cli, -1);
    ::close(cli);

    t.join();
    CHECK(true);
}

// bind_listen: accept a client and read probe
TEST_CASE("bind_listen: accepts client, returns a connected fd") {
    ignore_sigpipe_once();

    int port = find_free_port();

    std::thread client([&]{
        int c = ::socket(AF_INET, SOCK_STREAM, 0);
        REQUIRE(c >= 0);
        sockaddr_in sa{};
        sa.sin_family = AF_INET;
        sa.sin_port   = htons((uint16_t)port);
        inet_pton(AF_INET, "127.0.0.1", &sa.sin_addr);
        for (int i=0;i<100;i++) {
            if (::connect(c, (sockaddr*)&sa, sizeof(sa)) == 0) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        (void)::write(c, "ping", 4);
        ::shutdown(c, SHUT_WR);
        ::close(c);
    });

    int fd = bind_listen(port);       // blocks until client connects
    REQUIRE(fd >= 0);

    // read probe (we can use read_until_delim with timeout but here client closes)
    std::string got;
    char buf[16];
    ssize_t n = ::read(fd, buf, sizeof(buf));
    REQUIRE(n >= 0);
    got.append(buf, buf + (n > 0 ? n : 0));
    CHECK(got.find("ping") != std::string::npos);

    ::close(fd);
    client.join();
}
