/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Author : SIPp Benchmark Suite
 */

#define GLOBALS_FULL_DEFINITION
#include "sipp.hpp"
#include "message.hpp"
#include "scenario.hpp"
#include "variables.hpp"
#include "sip_parser.hpp"
#include "call.hpp"
#include "strings.hpp"

#include <benchmark/benchmark.h>
#include <string>
#include <cstring>

// Global variables are already defined in sipp.hpp via GLOBALS_FULL_DEFINITION
// No need to redefine them here

// Forward declaration
void InitializeBenchmarkGlobals();

// Simple string replacement function for benchmarking
void substitute_keyword(char** str, const char* keyword, const char* replacement) {
    char* pos = strstr(*str, keyword);
    if (pos) {
        size_t keyword_len = strlen(keyword);
        size_t replacement_len = strlen(replacement);
        size_t old_len = strlen(*str);
        size_t new_len = old_len - keyword_len + replacement_len;

        char* new_str = (char*)malloc(new_len + 1);

        // Copy part before keyword
        size_t prefix_len = pos - *str;
        memcpy(new_str, *str, prefix_len);

        // Copy replacement
        memcpy(new_str + prefix_len, replacement, replacement_len);

        // Copy part after keyword
        memcpy(new_str + prefix_len + replacement_len,
               pos + keyword_len,
               old_len - prefix_len - keyword_len);

        new_str[new_len] = '\0';

        free(*str);
        *str = new_str;
    }
}

// Setup and teardown
class Sipp : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) override {
        InitializeBenchmarkGlobals();
    }

    void TearDown(const ::benchmark::State& state) override {
        // Keep globals alive between benchmarks for consistency
    }
};

// Benchmark message parsing performance
BENCHMARK_F(Sipp, MessageParsing)(benchmark::State& state) {
    const char* sip_message =
        "INVITE sip:bob@biloxi.com SIP/2.0\r\n"
        "Via: SIP/2.0/UDP pc33.atlanta.com;branch=z9hG4bKnashds8\r\n"
        "Max-Forwards: 70\r\n"
        "To: Bob <sip:bob@biloxi.com>\r\n"
        "From: Alice <sip:alice@atlanta.com>;tag=1928301774\r\n"
        "Call-ID: a84b4c76e66710@pc33.atlanta.com\r\n"
        "CSeq: 314159 INVITE\r\n"
        "Contact: <sip:alice@pc33.atlanta.com>\r\n"
        "Content-Type: application/sdp\r\n"
        "Content-Length: 142\r\n"
        "\r\n"
        "v=0\r\n"
        "o=alice 53655765 2353687637 IN IP4 pc33.atlanta.com\r\n"
        "s=-\r\n"
        "c=IN IP4 pc33.atlanta.com\r\n"
        "t=0 0\r\n"
        "m=audio 3456 RTP/AVP 0\r\n"
        "a=rtpmap:0 PCMU/8000\r\n";

    for (auto _ : state) {
        char* msg_copy = strdup(sip_message);

        // Parse first line and extract method, URI, version
        char* first_line = get_first_line(msg_copy);
        if (first_line) {
            char* method = strtok(first_line, " ");
            char* uri = strtok(nullptr, " ");
            char* version = strtok(nullptr, " ");

            benchmark::DoNotOptimize(method);
            benchmark::DoNotOptimize(uri);
            benchmark::DoNotOptimize(version);
        }

        free(msg_copy);
    }
}

// Benchmark variable substitution performance
BENCHMARK_F(Sipp, VariableSubstitution)(benchmark::State& state) {
    const char* template_str =
        "INVITE sip:[field0]@[remote_ip]:[remote_port] SIP/2.0\r\n"
        "Via: SIP/2.0/UDP [local_ip]:[local_port];branch=z9hG4bK[branch]\r\n"
        "From: [field1] <sip:[field1]@[local_ip]>;tag=[call_number]\r\n"
        "To: [field0] <sip:[field0]@[remote_ip]>\r\n"
        "Call-ID: [call_id]\r\n"
        "CSeq: [cseq] INVITE\r\n"
        "Contact: <sip:[field1]@[local_ip]:[local_port]>\r\n"
        "Content-Length: [len]\r\n\r\n";

    // Set up some test variables
    if (userVariables) {
        userVariables->getVar(userVariables->find("field0", true))->setString(strdup("bob"));
        userVariables->getVar(userVariables->find("field1", true))->setString(strdup("alice"));
        userVariables->getVar(userVariables->find("remote_ip", true))->setString(strdup("192.168.1.100"));
        userVariables->getVar(userVariables->find("remote_port", true))->setString(strdup("5060"));
        userVariables->getVar(userVariables->find("local_ip", true))->setString(strdup("192.168.1.1"));
        userVariables->getVar(userVariables->find("local_port", true))->setString(strdup("5061"));
        userVariables->getVar(userVariables->find("branch", true))->setString(strdup("abc123def456"));
        userVariables->getVar(userVariables->find("call_number", true))->setString(strdup("12345"));
        userVariables->getVar(userVariables->find("call_id", true))->setString(strdup("test-call-id@example.com"));
        userVariables->getVar(userVariables->find("cseq", true))->setString(strdup("1"));
        userVariables->getVar(userVariables->find("len", true))->setString(strdup("0"));
    }

    for (auto _ : state) {
        char* result = strdup(template_str);
        // Simulate variable substitution (simplified version)
        substitute_keyword(&result, "[field0]", "bob");
        substitute_keyword(&result, "[field1]", "alice");
        substitute_keyword(&result, "[remote_ip]", "192.168.1.100");
        substitute_keyword(&result, "[remote_port]", "5060");
        substitute_keyword(&result, "[local_ip]", "192.168.1.1");
        substitute_keyword(&result, "[local_port]", "5061");
        substitute_keyword(&result, "[branch]", "abc123def456");
        substitute_keyword(&result, "[call_number]", "12345");
        substitute_keyword(&result, "[call_id]", "test-call-id@example.com");
        substitute_keyword(&result, "[cseq]", "1");
        substitute_keyword(&result, "[len]", "0");

        benchmark::DoNotOptimize(result);
        free(result);
    }
}

// Benchmark string operations
BENCHMARK_F(Sipp, StringOperations)(benchmark::State& state) {
    const char* test_string = "This is a test string with some content to search through and manipulate";

    for (auto _ : state) {
        // Test various string operations that SIPp performs frequently
        char* copy = strdup(test_string);

        // Find substring
        char* found = strstr(copy, "test");
        benchmark::DoNotOptimize(found);

        // Case insensitive search
        char* found_case = strcasestr(copy, "TEST");
        benchmark::DoNotOptimize(found_case);

        // Length calculation
        size_t len = strlen(copy);
        benchmark::DoNotOptimize(len);

        free(copy);
    }
}

// Benchmark call ID generation
BENCHMARK_F(Sipp, CallIdGeneration)(benchmark::State& state) {
    for (auto _ : state) {
        char call_id[256];
        // Simulate call ID generation (simplified)
        snprintf(call_id, sizeof(call_id), "%lu-%lu@%s",
                 (unsigned long)(state.iterations() % 100000),
                 (unsigned long)time(nullptr),
                 "127.0.0.1");
        benchmark::DoNotOptimize(call_id);
    }
}

// Benchmark memory allocation patterns
BENCHMARK_F(Sipp, MemoryAllocation)(benchmark::State& state) {
    for (auto _ : state) {
        // Simulate typical SIPp memory allocation patterns
        char* buffer1 = (char*)malloc(1024);
        char* buffer2 = (char*)malloc(2048);
        char* buffer3 = (char*)malloc(512);

        // Do some work with the buffers
        memset(buffer1, 'A', 1024);
        memset(buffer2, 'B', 2048);
        memset(buffer3, 'C', 512);

        benchmark::DoNotOptimize(buffer1);
        benchmark::DoNotOptimize(buffer2);
        benchmark::DoNotOptimize(buffer3);

        free(buffer1);
        free(buffer2);
        free(buffer3);
    }
}

// Benchmark large message handling
BENCHMARK_F(Sipp, LargeMessageHandling)(benchmark::State& state) {
    // Create a large SIP message with SDP
    std::string large_message =
        "INVITE sip:bob@biloxi.com SIP/2.0\r\n"
        "Via: SIP/2.0/UDP pc33.atlanta.com;branch=z9hG4bKnashds8\r\n"
        "Max-Forwards: 70\r\n"
        "To: Bob <sip:bob@biloxi.com>\r\n"
        "From: Alice <sip:alice@atlanta.com>;tag=1928301774\r\n"
        "Call-ID: a84b4c76e66710@pc33.atlanta.com\r\n"
        "CSeq: 314159 INVITE\r\n"
        "Contact: <sip:alice@pc33.atlanta.com>\r\n"
        "Content-Type: application/sdp\r\n";

    // Add large SDP content
    std::string sdp_content =
        "v=0\r\n"
        "o=alice 53655765 2353687637 IN IP4 pc33.atlanta.com\r\n"
        "s=SIP Benchmark Session\r\n"
        "c=IN IP4 pc33.atlanta.com\r\n"
        "t=0 0\r\n";

    // Add multiple media streams to make it larger
    for (int i = 0; i < 50; ++i) {
        sdp_content += "m=audio " + std::to_string(3456 + i) + " RTP/AVP 0\r\n";
        sdp_content += "a=rtpmap:0 PCMU/8000\r\n";
        sdp_content += "a=sendrecv\r\n";
    }

    large_message += "Content-Length: " + std::to_string(sdp_content.length()) + "\r\n\r\n";
    large_message += sdp_content;

    for (auto _ : state) {
        char* msg_copy = strdup(large_message.c_str());

        // Simulate processing the large message
        size_t len = strlen(msg_copy);
        char* content_start = strstr(msg_copy, "\r\n\r\n");
        if (content_start) {
            content_start += 4; // Skip the CRLFCRLF
            benchmark::DoNotOptimize(content_start);
        }

        benchmark::DoNotOptimize(len);
        free(msg_copy);
    }
}

// Benchmark concurrent call simulation preparation
BENCHMARK_F(Sipp, CallDataStructures)(benchmark::State& state) {
    for (auto _ : state) {
        // Simulate creating call data structures
        struct {
            unsigned int id;
            unsigned long start_time;
            char call_id[128];
            char from_tag[64];
            char to_tag[64];
            int cseq;
            void* user_data;
        } call_data;

        call_data.id = state.iterations() % 100000;
        call_data.start_time = time(nullptr);
        snprintf(call_data.call_id, sizeof(call_data.call_id),
                 "call-%u@benchmark", call_data.id);
        snprintf(call_data.from_tag, sizeof(call_data.from_tag),
                 "tag-%u", call_data.id);
        call_data.to_tag[0] = '\0';
        call_data.cseq = 1;
        call_data.user_data = nullptr;

        benchmark::DoNotOptimize(call_data);
    }
}

// Main benchmark initialization (using benchmark::benchmark_main)
// Global initialization
static bool benchmark_initialized = false;

void InitializeBenchmarkGlobals() {
    if (!benchmark_initialized) {
        globalVariables = new AllocVariableTable(nullptr);
        userVariables = new AllocVariableTable(globalVariables);
        main_scenario = new scenario(0, 0);
        benchmark_initialized = true;
    }
}

// Custom main to initialize globals before benchmark main takes over
int main(int argc, char** argv) {
    InitializeBenchmarkGlobals();

    // Let benchmark library handle the rest
    ::benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv)) {
        return 1;
    }
    ::benchmark::RunSpecifiedBenchmarks();
    ::benchmark::Shutdown();

    // Cleanup
    delete main_scenario;
    delete userVariables;
    delete globalVariables;

    return 0;
}

// Stub for sipp_exit to satisfy linker
void sipp_exit(int rc, int rtp_errors, int echo_errors) {
    exit(rc);
}
