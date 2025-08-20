/*
 *  SIPp Network and Socket Performance Benchmarks
 *
 *  This file contains benchmarks for network-related operations
 *  that are critical for SIPp performance.
 */

#include "sipp.hpp"
#include "socket.hpp"
#include "variables.hpp"

#include <benchmark/benchmark.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

class Network : public benchmark::Fixture {
};

// Benchmark socket address parsing
BENCHMARK_F(Network, SocketAddressParsing)(benchmark::State& state) {
    const char* addresses[] = {
        "192.168.1.1:5060",
        "10.0.0.1:5061",
        "127.0.0.1:8080",
        "172.16.0.1:9999",
        "[::1]:5060",
        "[2001:db8::1]:5060"
    };
    const int num_addresses = sizeof(addresses) / sizeof(addresses[0]);
    int addr_index = 0;

    for (auto _ : state) {
        const char* addr = addresses[addr_index % num_addresses];
        addr_index++;

        struct sockaddr_storage ss;
        socklen_t ss_len = sizeof(ss);

        // Parse the address (simplified version)
        char* addr_copy = strdup(addr);
        char* port_str = strrchr(addr_copy, ':');
        if (port_str) {
            *port_str = '\0';
            port_str++;

            struct sockaddr_in* sin = (struct sockaddr_in*)&ss;
            sin->sin_family = AF_INET;
            sin->sin_port = htons(atoi(port_str));
            inet_pton(AF_INET, addr_copy, &sin->sin_addr);
        }

        benchmark::DoNotOptimize(ss_len);
        benchmark::DoNotOptimize(ss);
        free(addr_copy);
    }
}

// Benchmark message buffer preparation
BENCHMARK_F(Network, MessageBufferPreparation)(benchmark::State& state) {
    const char* template_msg =
        "INVITE sip:user@%s:%d SIP/2.0\r\n"
        "Via: SIP/2.0/UDP %s:%d;branch=z9hG4bK%s\r\n"
        "From: <sip:alice@%s>;tag=%s\r\n"
        "To: <sip:user@%s>\r\n"
        "Call-ID: %s\r\n"
        "CSeq: %d INVITE\r\n"
        "Content-Length: 0\r\n\r\n";

    for (auto _ : state) {
        char buffer[2048];
        snprintf(buffer, sizeof(buffer), template_msg,
                "192.168.1.100", 5060,           // remote
                "192.168.1.1", 5061,             // local
                "branch123",                      // branch
                "192.168.1.1",                   // from domain
                "tag456",                         // from tag
                "192.168.1.100",                 // to domain
                "callid789@example.com",         // call-id
                1);                               // cseq

        benchmark::DoNotOptimize(buffer);
    }
}

// Benchmark UDP packet size calculation
BENCHMARK_F(Network, PacketSizeCalculation)(benchmark::State& state) {
    const char* messages[] = {
        "INVITE sip:user@example.com SIP/2.0\r\nContent-Length: 0\r\n\r\n",
        "BYE sip:user@example.com SIP/2.0\r\nContent-Length: 0\r\n\r\n",
        "200 OK SIP/2.0\r\nContent-Length: 0\r\n\r\n",
        "ACK sip:user@example.com SIP/2.0\r\nContent-Length: 0\r\n\r\n"
    };
    const int num_messages = sizeof(messages) / sizeof(messages[0]);
    int msg_index = 0;

    for (auto _ : state) {
        const char* msg = messages[msg_index % num_messages];
        msg_index++;

        size_t len = strlen(msg);
        size_t ip_header = 20;   // IPv4 header
        size_t udp_header = 8;   // UDP header
        size_t total_size = ip_header + udp_header + len;

        benchmark::DoNotOptimize(total_size);
    }
}

// Benchmark connection state tracking
BENCHMARK_F(Network, ConnectionStateTracking)(benchmark::State& state) {
    struct connection_state {
        int socket_fd;
        struct sockaddr_storage remote_addr;
        socklen_t addr_len;
        unsigned long bytes_sent;
        unsigned long bytes_received;
        time_t last_activity;
        int state; // 0=idle, 1=sending, 2=receiving
    };

    for (auto _ : state) {
        connection_state conn;
        conn.socket_fd = -1;
        conn.addr_len = sizeof(struct sockaddr_in);
        conn.bytes_sent = 0;
        conn.bytes_received = 0;
        conn.last_activity = time(nullptr);
        conn.state = 0;

        // Simulate state updates
        conn.bytes_sent += 500;  // Typical SIP message size
        conn.last_activity = time(nullptr);
        conn.state = 1;

        benchmark::DoNotOptimize(conn);
    }
}

// Benchmark DNS resolution simulation
BENCHMARK_F(Network, DNSResolutionSimulation)(benchmark::State& state) {
    const char* hostnames[] = {
        "sip.example.com",
        "proxy.test.org",
        "192.168.1.100",
        "10.0.0.1",
        "localhost"
    };
    const int num_hosts = sizeof(hostnames) / sizeof(hostnames[0]);
    int host_index = 0;

    for (auto _ : state) {
        const char* hostname = hostnames[host_index % num_hosts];
        host_index++;

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;

        // Simulate quick DNS resolution (just check if it's an IP)
        if (inet_pton(AF_INET, hostname, &addr.sin_addr) == 1) {
            // It's already an IP address
            benchmark::DoNotOptimize(addr);
        } else {
            // Would normally do DNS lookup, but for benchmark just simulate
            addr.sin_addr.s_addr = inet_addr("192.168.1.1");
            benchmark::DoNotOptimize(addr);
        }
    }
}

// Benchmark port allocation
BENCHMARK_F(Network, PortAllocation)(benchmark::State& state) {
    int base_port = 10000;
    int current_port = base_port;

    for (auto _ : state) {
        // Simulate port allocation for RTP streams
        int rtp_port = current_port;
        int rtcp_port = current_port + 1;

        current_port += 2;
        if (current_port > 65000) {
            current_port = base_port;
        }

        benchmark::DoNotOptimize(rtp_port);
        benchmark::DoNotOptimize(rtcp_port);
    }
}

// Benchmark checksum calculation (for RTP/UDP)
BENCHMARK_F(Network, ChecksumCalculation)(benchmark::State& state) {
    unsigned char test_data[1500]; // MTU size
    for (int i = 0; i < 1500; i++) {
        test_data[i] = i % 256;
    }

    for (auto _ : state) {
        unsigned long checksum = 0;
        for (int i = 0; i < 1500; i += 2) {
            if (i + 1 < 1500) {
                checksum += (test_data[i] << 8) + test_data[i + 1];
            } else {
                checksum += test_data[i] << 8;
            }
        }

        // Handle carry
        while (checksum >> 16) {
            checksum = (checksum & 0xFFFF) + (checksum >> 16);
        }

        checksum = ~checksum;
        benchmark::DoNotOptimize(checksum);
    }
}

// Benchmark buffer management
BENCHMARK_F(Network, BufferManagement)(benchmark::State& state) {
    const int buffer_sizes[] = {512, 1024, 2048, 4096, 8192};
    const int num_sizes = sizeof(buffer_sizes) / sizeof(buffer_sizes[0]);
    int size_index = 0;

    for (auto _ : state) {
        int size = buffer_sizes[size_index % num_sizes];
        size_index++;

        char* buffer = (char*)malloc(size);
        memset(buffer, 0, size);

        // Simulate writing data to buffer
        const char* sample_data = "SIP/2.0 200 OK\r\n";
        int data_len = strlen(sample_data);
        if (data_len < size) {
            memcpy(buffer, sample_data, data_len);
        }

        benchmark::DoNotOptimize(buffer);
        free(buffer);
    }
}
