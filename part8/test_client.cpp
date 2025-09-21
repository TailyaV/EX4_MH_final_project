#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <thread>
#include <chrono>
#include <string>
#include <sstream>
#include <vector>
#include <atomic>
#include <cerrno>
#include <csignal>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

//Declarations from client
bool recv_until_delim(int sock, char delim, std::string& out, std::string& stash);
bool send_request(int sock);

//Toggle a file descriptor between non-blocking (on=true) and blocking (on=false) using fcntl.
static void set_nonblock(int fd, bool on=true) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return;
    fcntl(fd, F_SETFL, on ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK));
}

//Best-effort read of up to n bytes and return what was actually read.
static std::string read_exact_bytes(int fd, size_t n) {
    std::string out; out.resize(n);
    size_t got = 0;
    while (got < n) {
        ssize_t r = ::read(fd, &out[got], n - got);
        if (r == 0) break;
        if (r < 0) { if (errno == EINTR) continue; break; }
        got += size_t(r);
    }
    out.resize(got);
    return out;
}

//Temporarily replace std::cin's buffer with scripted input
struct CinReplacer {
    std::streambuf* old = nullptr;
    explicit CinReplacer(std::istream& in, std::streambuf* nb) { old = in.rdbuf(nb); }
    ~CinReplacer() { if (old) std::cin.rdbuf(old); }
};

//Install a signal handler without SA_RESTART so blocking syscalls can return EINTR.
static void install_sig_no_restart(int signo, void (*handler)(int)) {
    struct sigaction sa{};
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; // NO SA_RESTART → read() can return EINTR
    sigaction(signo, &sa, nullptr);
}

//No-op signal handler, used to interrupt blocking syscalls to trigger EINTR.
static void empty_sig(int){}

TEST_CASE("send_request: invalid choice then 1 (manual graph) writes expected ints") {
    int fds[2]; REQUIRE(::pipe(fds) == 0);
    int r = fds[0], w = fds[1];

    // Sequence: invalid 5 → valid 1 → V=3 → edges: 0 1, 1 2, -1 -1
    std::istringstream iss("5\n1\n3\n0 1\n1 2\n-1 -1\n");
    CinReplacer cr(std::cin, iss.rdbuf());

    bool ok = send_request(w);
    CHECK(ok == true);

    // Expect: 8 ints (choice, V, u,v,u,v,u,v)
    std::string raw = read_exact_bytes(r, sizeof(int) * 8);
    CHECK(raw.size() == sizeof(int) * 8);

    const int* p = reinterpret_cast<const int*>(raw.data());
    CHECK(p[0] == 1);
    CHECK(p[1] == 3);
    CHECK(p[2] == 0); CHECK(p[3] == 1);
    CHECK(p[4] == 1); CHECK(p[5] == 2);
    CHECK(p[6] == -1); CHECK(p[7] == -1);

    ::close(r); ::close(w);
}

TEST_CASE("send_request: choice=2 (random graph) writes V,E,seed") {
    int fds[2]; REQUIRE(::pipe(fds) == 0);
    int r = fds[0], w = fds[1];

    std::istringstream iss("2\n7\n10\n42\n");
    CinReplacer cr(std::cin, iss.rdbuf());

    bool ok = send_request(w);
    CHECK(ok == true);

    std::string raw = read_exact_bytes(r, sizeof(int) * 4);
    CHECK(raw.size() == sizeof(int) * 4);

    const int* p = reinterpret_cast<const int*>(raw.data());
    CHECK(p[0] == 2);
    CHECK(p[1] == 7);
    CHECK(p[2] == 10);
    CHECK(p[3] == 42);

    ::close(r); ::close(w);
}

TEST_CASE("send_request: choice=0 is written and function returns false") {
    int fds[2]; REQUIRE(::pipe(fds) == 0);
    int r = fds[0], w = fds[1];

    std::istringstream iss("0\n");
    CinReplacer cr(std::cin, iss.rdbuf());

    bool ok = send_request(w);
    CHECK(ok == false);

    std::string raw = read_exact_bytes(r, sizeof(int));
    CHECK(raw.size() == sizeof(int));
    const int* p = reinterpret_cast<const int*>(raw.data());
    CHECK(p[0] == 0);

    ::close(r); ::close(w);
}

TEST_CASE("recv_until_delim: delimiter already in stash → returns immediately") {
    int sp[2]; REQUIRE(::socketpair(AF_UNIX, SOCK_STREAM, 0, sp) == 0);
    int a = sp[0], b = sp[1];

    std::string stash = "hello}more";
    std::string msg;
    bool ok = recv_until_delim(a, '}', msg, stash);

    CHECK(ok == true);
    CHECK(msg == "hello");
    CHECK(stash == "more"); // removed message + delim

    ::close(a); ::close(b);
}

TEST_CASE("recv_until_delim: normal read in chunks until '}'") {
    int sp[2]; REQUIRE(::socketpair(AF_UNIX, SOCK_STREAM, 0, sp) == 0);
    int a = sp[0], b = sp[1];

    std::string stash;
    std::thread writer([&]{
        (void)::write(b, "abc", 3);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        (void)::write(b, "def}", 4);
        // keep connection open (no EOF) to ensure function stops due to delimiter
    });

    std::string msg;
    bool ok = recv_until_delim(a, '}', msg, stash);

    CHECK(ok == true);
    CHECK(msg == "abcdef");
    CHECK(stash.empty()); // writer didn't send more bytes after the delimiter

    ::close(b);
    ::close(a);
    writer.join();
}

TEST_CASE("recv_until_delim: non-blocking socket with no data → returns false (EAGAIN)") {
    int sp[2]; REQUIRE(::socketpair(AF_UNIX, SOCK_STREAM, 0, sp) == 0);
    int a = sp[0], b = sp[1];

    set_nonblock(a, true);
    std::string stash, msg;
    bool ok = recv_until_delim(a, '}', msg, stash);
    CHECK(ok == false);     // no data yet
    CHECK(stash.empty());
    CHECK(msg.empty());

    ::close(a); ::close(b);
}

TEST_CASE("recv_until_delim: EINTR then data arrives → succeeds") {
    int sp[2]; REQUIRE(::socketpair(AF_UNIX, SOCK_STREAM, 0, sp) == 0);
    int a = sp[0], b = sp[1];

    install_sig_no_restart(SIGALRM, empty_sig);

    std::thread writer([&]{
        // Interrupt the first blocked read
        ualarm(6000, 0); // ~6ms
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        (void)::write(b, "X}", 2);
    });

    std::string stash, msg;
    bool ok = recv_until_delim(a, '}', msg, stash);

    CHECK(ok == true);
    CHECK(msg == "X");
    CHECK(stash.empty());

    ::close(b);
    ::close(a);
    writer.join();
}

TEST_CASE("recv_until_delim: EOF (read returns 0) → returns false and keeps stash") {
    int sp[2]; REQUIRE(::socketpair(AF_UNIX, SOCK_STREAM, 0, sp) == 0);
    int a = sp[0], b = sp[1];

    std::string stash = "partial-without-delim";
    std::string msg;

    ::shutdown(b, SHUT_WR); // cause EOF on 'a'
    ::close(b);

    bool ok = recv_until_delim(a, '}', msg, stash);

    CHECK(ok == false);
    CHECK(msg.empty());
    CHECK(stash == "partial-without-delim"); // untouched

    ::close(a);
}
