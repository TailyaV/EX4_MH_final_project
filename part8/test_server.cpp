#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string>
#include <thread>
#include <vector>
#include <cstring>
#include <cerrno>

// We include the implementation TU so tests can call my_handler.
// (main() in server.cpp is excluded by -DUNIT_TEST)
#include "graph.hpp"
#include "lf.hpp"
#include "server.hpp"

// =========================== Helpers ===========================

// Helper: Ignore SIGPIPE globally once so writes to a closed peer don't kill the process.
static void ignore_sigpipe_once() {
    static bool done = false;
    if (!done) { ::signal(SIGPIPE, SIG_IGN); done = true; }
}

// Helper: Set or clear O_NONBLOCK on a file descriptor.
static void set_nonblock(int fd, bool on=true) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return;
    fcntl(fd, F_SETFL, on ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK));
}

// Helper: Write a single 32-bit int fully to fd (handles EINTR).
static void send_int(int fd, int x) {
    const char* p = reinterpret_cast<const char*>(&x);
    size_t left = sizeof(x);
    while (left) {
        ssize_t w = ::write(fd, p, left);
        if (w < 0) { if (errno == EINTR) continue; break; }
        p += w; left -= (size_t)w;
    }
}

// Helper: Read from fd (non-blocking loop) until a delimiter is seen, EOF, or temporary no-data.
static std::string read_until_delim(int fd, char delim) {
    set_nonblock(fd, true);
    std::string out;
    char buf[1024];
    for (int spins = 0; spins < 2000; ++spins) { // ~2s if usleep(1000)
        if (out.find(delim) != std::string::npos) break;

        ssize_t n = ::read(fd, buf, sizeof(buf));
        if (n > 0) {
            out.append(buf, buf + n);
            if (out.find(delim) != std::string::npos) break;
        } else if (n == 0) {
            // peer closed
            break;
        } else {
            if (errno == EINTR) continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // temporarily no data
                usleep(1000);
                continue;
            }
            // read error
            break;
        }
    }
    return out;
}

// Helper: Create a bidirectional local socketpair for in-process client/server tests.
static void make_socketpair(int fds[2]) {
    int rc = ::socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
    REQUIRE(rc == 0);
}

// =========================== Tests ===========================

TEST_CASE("choice=1 (manual graph) → handler writes full algorithms report and keeps loop alive") {
    ignore_sigpipe_once();
    int sp[2]; make_socketpair(sp);
    int cli = sp[0];  // client side writes
    int srv = sp[1];  // handler reads/writes on this end

    // Run my_handler on a thread to emulate the server-side loop.
    std::thread th([&]{ my_handler(srv); });

    // Request: choice=1, number of vertices, edges (u,v)... then -1 -1 to end.
    const int V = 4;
    send_int(cli, 1);        // choice = 1 (manual graph)
    send_int(cli, V);        // vertices
    // Directed graph (server constructs Graph(V, true))
    send_int(cli, 0); send_int(cli, 1);
    send_int(cli, 1); send_int(cli, 2);
    send_int(cli, 2); send_int(cli, 3);
    // End of edges
    send_int(cli, -1); send_int(cli, -1);

    // Read response until the terminating '}' delimiter.
    std::string resp = read_until_delim(cli, '}');

    // Check the main headings and footer.
    CHECK(resp.find("rum all algorithm:") != std::string::npos);
    CHECK(resp.find("Graph adjacency list:") != std::string::npos);
    CHECK(resp.find("MST algorithm:") != std::string::npos);
    CHECK(resp.find("MST weight:") != std::string::npos);
    CHECK(resp.find("COUNT CLIQUES algorithm:") != std::string::npos);
    CHECK(resp.find("number of cliques:") != std::string::npos);
    CHECK(resp.find("FIND SCCs algorithm:") != std::string::npos);
    CHECK(resp.find("Strongly Connected Components:") != std::string::npos);
    CHECK(resp.find("MAX FLOW algorithm:") != std::string::npos);
    CHECK(resp.find("max flow:") != std::string::npos);
    CHECK(resp.size() > 0);
    CHECK(resp.back() == '}');

    // End the loop by sending choice=0, then close write end.
    send_int(cli, 0);
    ::shutdown(cli, SHUT_WR);

    th.join();
    ::close(cli);
    ::close(srv);
}

TEST_CASE("choice=2 (random graph) → handler writes a report (content not strictly checked)") {
    ignore_sigpipe_once();
    int sp[2]; make_socketpair(sp);
    int cli = sp[0], srv = sp[1];

    std::thread th([&]{ my_handler(srv); });

    // Random graph request
    send_int(cli, 2);    // choice = 2
    send_int(cli, 4);    // vertices
    send_int(cli, 3);    // edges
    send_int(cli, 123);  // seed

    std::string resp = read_until_delim(cli, '}');
    CHECK(resp.find("rum all algorithm:") != std::string::npos);
    CHECK(resp.find("Graph adjacency list:") != std::string::npos);
    CHECK(resp.find("MST algorithm:") != std::string::npos);
    CHECK(resp.find("COUNT CLIQUES algorithm:") != std::string::npos);
    CHECK(resp.find("FIND SCCs algorithm:") != std::string::npos);
    CHECK(resp.find("MAX FLOW algorithm:") != std::string::npos);
    CHECK(resp.back() == '}');

    // End
    send_int(cli, 0);
    ::shutdown(cli, SHUT_WR);

    th.join();
    ::close(cli);
    ::close(srv);
}

TEST_CASE("choice=0 immediately → handler exits loop without writing") {
    ignore_sigpipe_once();
    int sp[2]; make_socketpair(sp);
    int cli = sp[0], srv = sp[1];

    std::thread th([&]{ my_handler(srv); });

    // Send 0 to terminate loop right away.
    send_int(cli, 0);
    ::shutdown(cli, SHUT_WR);

    // No output is expected.
    std::string resp = read_until_delim(cli, '}');
    bool ok = (resp.empty() || resp.find('}') == std::string::npos);
    CHECK(ok);
    th.join();
    ::close(cli);
    ::close(srv);
}

TEST_CASE("invalid choice (e.g., 7) → my_handler throws std::invalid_argument") {
    ignore_sigpipe_once();
    int sp[2]; make_socketpair(sp);
    int cli = sp[0], srv = sp[1];

    // Send only an invalid choice; handler should throw on first read cycle.
    send_int(cli, 7);

    CHECK_THROWS_AS(my_handler(srv), std::invalid_argument);

    ::close(cli);
    ::close(srv);
}

TEST_CASE("EOF before full int (partial read) → handler returns without output") {
    ignore_sigpipe_once();
    int sp[2]; make_socketpair(sp);
    int cli = sp[0], srv = sp[1];

    std::thread th([&]{ my_handler(srv); });

    // Send fewer than 4 bytes of an int and then close write end to simulate short read.
    unsigned short half = 1;
    ::write(cli, &half, sizeof(half));
    ::shutdown(cli, SHUT_WR);

    std::string resp = read_until_delim(cli, '}');
    CHECK(resp.empty());

    th.join();
    ::close(cli);
    ::close(srv);
}

TEST_CASE("multiple requests on same connection (1 → 2 → 0)") {
    ignore_sigpipe_once();
    int sp[2]; make_socketpair(sp);
    int cli = sp[0], srv = sp[1];

    std::thread th([&]{ my_handler(srv); });

    // 1) Manual graph
    send_int(cli, 1);
    send_int(cli, 3);
    send_int(cli, 0); send_int(cli, 1);
    send_int(cli, 1); send_int(cli, 2);
    send_int(cli, -1); send_int(cli, -1);

    std::string r1 = read_until_delim(cli, '}');
    CHECK_FALSE(r1.empty());
    CHECK(r1.find("MST weight:") != std::string::npos);
    CHECK(r1.back() == '}');

    // 2) Random graph
    send_int(cli, 2);
    send_int(cli, 3);  // V
    send_int(cli, 2);  // E
    send_int(cli, 42); // seed
    std::string r2 = read_until_delim(cli, '}');
    CHECK_FALSE(r2.empty());
    CHECK(r2.find("number of cliques:") != std::string::npos);
    CHECK(r2.back() == '}');

    // 3) Terminate loop
    send_int(cli, 0);
    ::shutdown(cli, SHUT_WR);

    th.join();
    ::close(cli);
    ::close(srv);
}
