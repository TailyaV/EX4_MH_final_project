#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "lf.hpp"

#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include <atomic>
#include <cerrno>
#include <cstring>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

//obtain an ephemeral TCP port by binding to 127.0.0.1:0, then reading back the assigned port via getsockname().
static int find_free_port() {
    int s = ::socket(AF_INET, SOCK_STREAM, 0);
    REQUIRE(s >= 0);

    sockaddr_in a{}; 
    a.sin_family = AF_INET;
    a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    a.sin_port = htons(0); // Let the OS pick a free port

    REQUIRE(::bind(s, (sockaddr*)&a, sizeof(a)) == 0);

    socklen_t len = sizeof(a);
    REQUIRE(::getsockname(s, (sockaddr*)&a, &len) == 0);
    int port = ntohs(a.sin_port);
    ::close(s);
    return port;
}

//repeatedly attempts to connect to the server until it becomes ready (or retries are exhausted)
static int connect_with_retry(int port, int retries=200, int sleep_ms=10) {
    int fd = -1;
    for (int i=0; i<retries; ++i) {
        fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) return -1;

        sockaddr_in sa{};
        sa.sin_family = AF_INET;
        sa.sin_port   = htons((uint16_t)port);
        inet_pton(AF_INET, "127.0.0.1", &sa.sin_addr);

        if (::connect(fd, (sockaddr*)&sa, sizeof(sa)) == 0) {
            return fd;
        }
        ::close(fd);
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
    }
    return -1;
}

//drains the socket until EOF, handling EINTR (error).
static std::string read_all(int fd) {
    std::string out;
    char buf[1024];
    while (true) {
        ssize_t n = ::read(fd, buf, sizeof(buf));
        if (n == 0) break;                // Peer closed (EOF)
        if (n < 0) {
            if (errno == EINTR) continue; /// Retry on signal interruption
            break;                        // Any other read error → stop
        }
        out.append(buf, buf + n);
    }
    return out;
}

//writes exactly N bytes, retrying on EINTR.
static bool write_all(int fd, const void* p, size_t n) {
    const char* c = static_cast<const char*>(p);
    size_t left = n;
    while (left) {
        ssize_t w = ::write(fd, c, left);
        if (w < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        c += w;
        left -= (size_t)w;
    }
    return true;
}

TEST_CASE("LFServer: request_stop without start - no crash, safe no-op") {
    int port = find_free_port();
    //srv get port, number of threads, handler
    LFServer srv(port, 2, [](int){ });
    // Calling request_stop() before start() should be safe and not crash.
    srv.request_stop();
    CHECK(true);  // If we got here, it didn't crash.
}

TEST_CASE("LFServer: start → single client, handler writes, client reads") {
    int port = find_free_port();
    const char msg[] = "HI\n";

    LFServer srv(port, 2, [&](int cfd){
        // Normal handle_one path: write a small message and return.
        (void)write_all(cfd, msg, sizeof(msg)-1);
    });

    std::thread th([&]{ srv.start(); });

    // Ensure server is listening before connecting.
    int cfd = connect_with_retry(port);
    REQUIRE(cfd >= 0);

    std::string got = read_all(cfd);
    ::close(cfd);

    CHECK(got == "HI\n");

    // Gracefully stop the server and join the thread.
    srv.request_stop();
    th.join();
}

TEST_CASE("LFServer:multiple back-to-back clients exercise Leader/Followers handoff") {
    int port = find_free_port();
    std::atomic<int> served{0};

    LFServer srv(port, 4, [&](int cfd){
        // Each connection gets a small response; count how many were served.
        const char line[] = "OK";
        write_all(cfd, line, sizeof(line)-1);
        served.fetch_add(1);
    });

    std::thread th([&]{ srv.start(); });

    // Open several quick connections so multiple workers will accept and handle.
    std::vector<int> fds;
    for (int i=0;i<5;++i) {
        int fd = connect_with_retry(port);
        REQUIRE(fd >= 0);
        fds.push_back(fd);
    }

    // Read each response and close.
    for (int fd : fds) {
        std::string s = read_all(fd);
        CHECK(s == "OK");
        ::close(fd);
    }

    // Wait briefly for all handlers to finish (avoid racy check).
    for (int i=0;i<100 && served.load() < 5; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    CHECK(served.load() == 5);

    srv.request_stop();
    th.join();
}

TEST_CASE("LFServer:  handler throws → caught in handle_one, client sees EOF") {
    int port = find_free_port();
    
    LFServer srv(port, 1, [&](int){
        // Simulate an exception in user handler; handle_one() must catch it.
        throw std::runtime_error("boom");
    });

    std::thread th([&]{ srv.start(); });

    int cfd = connect_with_retry(port);
    REQUIRE(cfd >= 0);

    // Since handler throws and writes nothing, client should read EOF (empty).
    std::string s = read_all(cfd);
    close(cfd);

    CHECK(s.empty()); 

    srv.request_stop();
    th.join();
}

TEST_CASE("LFServer: request_stop while idle releases accept() and stops cleanly") {
    int port = find_free_port();

    LFServer srv(port, 2, [](int){  });

    std::thread th([&]{ srv.start(); });
     // Give the server a short moment to enter accept().
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // request_stop() triggers shutdown(listen_fd_) and wakes the leader,
    // causing accept() to return with an error → worker loop exits.
    srv.request_stop();
    th.join();

     // If we got here, shutdown/unblock worked.
    CHECK(true);
}
