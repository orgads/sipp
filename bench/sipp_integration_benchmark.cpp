/*
 *  SIPp Integration Performance Benchmarks
 *
 *  This file contains end-to-end performance benchmarks that test
 *  complete SIPp workflows and scenarios.
 */

#include "sipp.hpp"
#include "scenario.hpp"
#include "call.hpp"

#include <benchmark/benchmark.h>
#include <string>
#include <vector>
#include <chrono>

extern AllocVariableTable* userVariables;

// Forward declarations
extern void substitute_keyword(char** str, const char* keyword, const char* replacement);

class Integration : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) override {
        // Set up common test variables
        userVariables->getVar(userVariables->find("service", true))->setString(strdup("test"));
        userVariables->getVar(userVariables->find("remote_ip", true))->setString(strdup("127.0.0.1"));
        userVariables->getVar(userVariables->find("remote_port", true))->setString(strdup("5060"));
        userVariables->getVar(userVariables->find("local_ip", true))->setString(strdup("127.0.0.1"));
        userVariables->getVar(userVariables->find("local_port", true))->setString(strdup("5061"));
        userVariables->getVar(userVariables->find("transport", true))->setString(strdup("UDP"));
        userVariables->getVar(userVariables->find("version", true))->setString(strdup("3.7"));
    }
};

// Benchmark scenario parsing and compilation
BENCHMARK_F(Integration, ScenarioParsingAndCompilation)(benchmark::State& state) {
    const char* scenario_xml = R"(
<?xml version="1.0" encoding="UTF-8"?>
<scenario name="Benchmark Scenario">
  <send retrans="500">
    <![CDATA[
      INVITE sip:[service]@[remote_ip]:[remote_port] SIP/2.0
      Via: SIP/2.0/[transport] [local_ip]:[local_port];branch=[branch]
      From: sipp <sip:sipp@[local_ip]:[local_port]>;tag=[call_number]
      To: [service] <sip:[service]@[remote_ip]:[remote_port]>
      Call-ID: [call_id]
      CSeq: 1 INVITE
      Content-Length: 0
    ]]>
  </send>
  <recv response="200"></recv>
  <send>
    <![CDATA[
      ACK sip:[service]@[remote_ip]:[remote_port] SIP/2.0
      Via: SIP/2.0/[transport] [local_ip]:[local_port];branch=[branch]
      From: sipp <sip:sipp@[local_ip]:[local_port]>;tag=[call_number]
      To: [service] <sip:[service]@[remote_ip]:[remote_port]>[peer_tag_param]
      Call-ID: [call_id]
      CSeq: 1 ACK
      Content-Length: 0
    ]]>
  </send>
  <send>
    <![CDATA[
      BYE sip:[service]@[remote_ip]:[remote_port] SIP/2.0
      Via: SIP/2.0/[transport] [local_ip]:[local_port];branch=[branch]
      From: sipp <sip:sipp@[local_ip]:[local_port]>;tag=[call_number]
      To: [service] <sip:[service]@[remote_ip]:[remote_port]>[peer_tag_param]
      Call-ID: [call_id]
      CSeq: 2 BYE
      Content-Length: 0
    ]]>
  </send>
  <recv response="200"></recv>
</scenario>
)";

    for (auto _ : state) {
        // Simulate scenario parsing
        char* xml_copy = strdup(scenario_xml);

        // Count and parse elements
        std::vector<std::string> send_messages;
        std::vector<std::string> recv_responses;

        char* ptr = xml_copy;
        char* send_start;

        // Extract send messages
        while ((send_start = strstr(ptr, "<send")) != nullptr) {
            char* cdata_start = strstr(send_start, "<![CDATA[");
            if (cdata_start) {
                cdata_start += 9;
                char* cdata_end = strstr(cdata_start, "]]>");
                if (cdata_end) {
                    std::string message(cdata_start, cdata_end - cdata_start);
                    send_messages.push_back(message);
                }
            }
            ptr = send_start + 1;
        }

        // Extract recv responses
        ptr = xml_copy;
        char* recv_start;
        while ((recv_start = strstr(ptr, "<recv")) != nullptr) {
            char* response_attr = strstr(recv_start, "response=\"");
            if (response_attr) {
                response_attr += 10;
                char* end_quote = strchr(response_attr, '"');
                if (end_quote) {
                    std::string response(response_attr, end_quote - response_attr);
                    recv_responses.push_back(response);
                }
            }
            ptr = recv_start + 1;
        }

        benchmark::DoNotOptimize(send_messages);
        benchmark::DoNotOptimize(recv_responses);
        free(xml_copy);
    }
}

// Benchmark message template processing
BENCHMARK_F(Integration, MessageTemplateProcessing)(benchmark::State& state) {
    const char* invite_template =
        "INVITE sip:[service]@[remote_ip]:[remote_port] SIP/2.0\r\n"
        "Via: SIP/2.0/[transport] [local_ip]:[local_port];branch=z9hG4bK[branch]\r\n"
        "Max-Forwards: 70\r\n"
        "From: \"SIPp\" <sip:sipp@[local_ip]:[local_port]>;tag=[pid]SIPpTag[call_number]\r\n"
        "To: \"[service]\" <sip:[service]@[remote_ip]:[remote_port]>\r\n"
        "Call-ID: [call_id]\r\n"
        "CSeq: 1 INVITE\r\n"
        "Contact: <sip:sipp@[local_ip]:[local_port]>\r\n"
        "User-Agent: SIPp/[version]\r\n"
        "Content-Type: application/sdp\r\n"
        "Content-Length: [len]\r\n"
        "\r\n"
        "v=0\r\n"
        "o=user1 [call_number] [call_number] IN IP4 [local_ip]\r\n"
        "s=SIP Call\r\n"
        "c=IN IP4 [local_ip]\r\n"
        "t=0 0\r\n"
        "m=audio [audio_port] RTP/AVP 0\r\n"
        "a=rtpmap:0 PCMU/8000\r\n";

    for (auto _ : state) {
        char* message = strdup(invite_template);

        // Simulate variable substitution for a complete message
        char call_id[128];
        char branch[64];
        char call_number[32];
        char audio_port[16];

        snprintf(call_id, sizeof(call_id), "call%d@[local_ip]",
                 static_cast<int>(state.iterations()) % 100000);
        snprintf(branch, sizeof(branch), "branch%d",
                 static_cast<int>(state.iterations()) % 10000);
        snprintf(call_number, sizeof(call_number), "%d",
                 static_cast<int>(state.iterations()) % 1000);
        snprintf(audio_port, sizeof(audio_port), "%d",
                 10000 + (static_cast<int>(state.iterations()) % 1000) * 2);

        // Replace variables (simplified version)
        substitute_keyword(&message, "[call_id]", call_id);
        substitute_keyword(&message, "[branch]", branch);
        substitute_keyword(&message, "[call_number]", call_number);
        substitute_keyword(&message, "[audio_port]", audio_port);
        substitute_keyword(&message, "[pid]", "12345");

        // Calculate content length
        char* content_start = strstr(message, "\r\n\r\n");
        if (content_start) {
            content_start += 4;
            size_t content_len = strlen(content_start);
            char len_str[32];  // Increased buffer size to handle large numbers
            snprintf(len_str, sizeof(len_str), "%zu", content_len);
            substitute_keyword(&message, "[len]", len_str);
        }

        benchmark::DoNotOptimize(message);
        free(message);
    }
}

// Benchmark call state management
BENCHMARK_F(Integration, CallStateManagement)(benchmark::State& state) {
    struct CallState {
        unsigned int id;
        std::string call_id;
        std::string from_tag;
        std::string to_tag;
        int cseq;
        enum { IDLE, CALLING, CONNECTED, TERMINATING } state;
        std::chrono::time_point<std::chrono::steady_clock> start_time;
        std::chrono::time_point<std::chrono::steady_clock> connect_time;
    };

    std::vector<CallState> active_calls;
    active_calls.reserve(1000);

    for (auto _ : state) {
        // Simulate creating a new call
        CallState call;
        call.id = state.iterations() % 100000;
        call.call_id = "call" + std::to_string(call.id) + "@benchmark";
        call.from_tag = "tag" + std::to_string(call.id);
        call.to_tag = "";
        call.cseq = 1;
        call.state = CallState::CALLING;
        call.start_time = std::chrono::steady_clock::now();

        active_calls.push_back(call);

        // Simulate call progression
        if (active_calls.size() > 100) {
            // Process some existing calls
            for (auto& existing_call : active_calls) {
                if (existing_call.state == CallState::CALLING) {
                    existing_call.state = CallState::CONNECTED;
                    existing_call.connect_time = std::chrono::steady_clock::now();
                    existing_call.to_tag = "remote_tag" + std::to_string(existing_call.id);
                    existing_call.cseq++;
                } else if (existing_call.state == CallState::CONNECTED) {
                    existing_call.state = CallState::TERMINATING;
                    existing_call.cseq++;
                }
            }

            // Remove terminated calls
            active_calls.erase(
                std::remove_if(active_calls.begin(), active_calls.end(),
                    [](const CallState& call) { return call.state == CallState::TERMINATING; }),
                active_calls.end());
        }

        benchmark::DoNotOptimize(active_calls);
    }
}

// Benchmark statistics collection
BENCHMARK_F(Integration, StatisticsCollection)(benchmark::State& state) {
    struct Statistics {
        unsigned long total_calls;
        unsigned long successful_calls;
        unsigned long failed_calls;
        unsigned long total_messages_sent;
        unsigned long total_messages_received;
        unsigned long total_bytes_sent;
        unsigned long total_bytes_received;
        double min_response_time;
        double max_response_time;
        double avg_response_time;
        std::vector<double> response_times;
    };

    Statistics stats = {};
    stats.min_response_time = 999999.0;
    stats.max_response_time = 0.0;
    stats.response_times.reserve(10000);

    for (auto _ : state) {
        // Simulate processing a completed call
        stats.total_calls++;

        // Simulate random call outcome
        bool success = (state.iterations() % 10) != 0; // 90% success rate
        if (success) {
            stats.successful_calls++;
        } else {
            stats.failed_calls++;
        }

        // Simulate message counts
        int messages_per_call = success ? 6 : 4; // INVITE, 100, 180, 200, ACK, BYE, 200 vs INVITE, 100, 180, 4xx
        stats.total_messages_sent += messages_per_call / 2;
        stats.total_messages_received += messages_per_call / 2;

        // Simulate byte counts
        int avg_message_size = 500;
        stats.total_bytes_sent += (messages_per_call / 2) * avg_message_size;
        stats.total_bytes_received += (messages_per_call / 2) * avg_message_size;

        // Simulate response time
        double response_time = 50.0 + (state.iterations() % 200); // 50-250ms
        stats.response_times.push_back(response_time);

        if (response_time < stats.min_response_time) {
            stats.min_response_time = response_time;
        }
        if (response_time > stats.max_response_time) {
            stats.max_response_time = response_time;
        }

        // Calculate running average
        double sum = 0.0;
        for (double rt : stats.response_times) {
            sum += rt;
        }
        stats.avg_response_time = sum / stats.response_times.size();

        // Trim response times if too many
        if (stats.response_times.size() > 1000) {
            stats.response_times.erase(stats.response_times.begin(),
                                     stats.response_times.begin() + 500);
        }

        benchmark::DoNotOptimize(stats);
    }
}

// Benchmark concurrent scenario execution simulation
BENCHMARK_F(Integration, ConcurrentScenarioExecution)(benchmark::State& state) {
    const int max_concurrent_calls = 100;

    struct ScenarioStep {
        enum Type { SEND, RECV, PAUSE };
        Type type;
        std::string content;
        int timeout_ms;
    };

    std::vector<ScenarioStep> scenario_steps = {
        {ScenarioStep::SEND, "INVITE", 0},
        {ScenarioStep::RECV, "100", 5000},
        {ScenarioStep::RECV, "180", 5000},
        {ScenarioStep::RECV, "200", 30000},
        {ScenarioStep::SEND, "ACK", 0},
        {ScenarioStep::PAUSE, "", 1000},
        {ScenarioStep::SEND, "BYE", 0},
        {ScenarioStep::RECV, "200", 5000}
    };

    struct CallExecution {
        int call_id;
        int current_step;
        std::chrono::time_point<std::chrono::steady_clock> step_start_time;
        bool completed;
    };

    std::vector<CallExecution> executing_calls;
    executing_calls.reserve(max_concurrent_calls);

    for (auto _ : state) {
        auto current_time = std::chrono::steady_clock::now();

        // Start new call if under limit
        if (executing_calls.size() < max_concurrent_calls) {
            CallExecution new_call;
            new_call.call_id = state.iterations() % 1000000;
            new_call.current_step = 0;
            new_call.step_start_time = current_time;
            new_call.completed = false;
            executing_calls.push_back(new_call);
        }

        // Process existing calls
        for (auto& call : executing_calls) {
            if (call.completed) continue;

            const auto& current_step = scenario_steps[call.current_step];
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                current_time - call.step_start_time).count();

            // Simulate step completion
            bool step_complete = false;
            if (current_step.type == ScenarioStep::SEND) {
                step_complete = true; // Send is immediate
            } else if (current_step.type == ScenarioStep::RECV) {
                step_complete = elapsed > 100; // Simulate quick response
            } else if (current_step.type == ScenarioStep::PAUSE) {
                step_complete = elapsed >= current_step.timeout_ms;
            }

            if (step_complete) {
                call.current_step++;
                call.step_start_time = current_time;

                if (call.current_step >= static_cast<int>(scenario_steps.size())) {
                    call.completed = true;
                }
            }
        }

        // Remove completed calls
        executing_calls.erase(
            std::remove_if(executing_calls.begin(), executing_calls.end(),
                [](const CallExecution& call) { return call.completed; }),
            executing_calls.end());

        benchmark::DoNotOptimize(executing_calls);
    }
}
