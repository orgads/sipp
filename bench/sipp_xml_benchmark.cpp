/*
 *  SIPp XML and Scenario Parsing Performance Benchmarks
 *
 *  This file contains benchmarks for XML parsing and scenario
 *  processing operations.
 */

#include "sipp.hpp"
#include "scenario.hpp"
#include "xp_parser.h"

#include <benchmark/benchmark.h>
#include <string.h>

class XML : public benchmark::Fixture {
};

// Benchmark simple XML parsing
BENCHMARK_F(XML, SimpleXMLParsing)(benchmark::State& state) {
    const char* simple_xml =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<!DOCTYPE scenario SYSTEM \"sipp.dtd\">\n"
        "<scenario name=\"Basic UAC\">\n"
        "  <send retrans=\"500\">\n"
        "    <![CDATA[\n"
        "      INVITE sip:[service]@[remote_ip]:[remote_port] SIP/2.0\n"
        "      Via: SIP/2.0/[transport] [local_ip]:[local_port];branch=[branch]\n"
        "      From: sipp <sip:sipp@[local_ip]:[local_port]>;tag=[pid]SIPpTag00[call_number]\n"
        "      To: [service] <sip:[service]@[remote_ip]:[remote_port]>\n"
        "      Call-ID: [call_id]\n"
        "      CSeq: 1 INVITE\n"
        "      Content-Length: 0\n"
        "    ]]>\n"
        "  </send>\n"
        "  <recv response=\"100\" optional=\"true\">\n"
        "  </recv>\n"
        "  <recv response=\"180\" optional=\"true\">\n"
        "  </recv>\n"
        "  <recv response=\"200\" rtd=\"true\">\n"
        "  </recv>\n"
        "  <send>\n"
        "    <![CDATA[\n"
        "      ACK sip:[service]@[remote_ip]:[remote_port] SIP/2.0\n"
        "      Via: SIP/2.0/[transport] [local_ip]:[local_port];branch=[branch]\n"
        "      From: sipp <sip:sipp@[local_ip]:[local_port]>;tag=[pid]SIPpTag00[call_number]\n"
        "      To: [service] <sip:[service]@[remote_ip]:[remote_port]>[peer_tag_param]\n"
        "      Call-ID: [call_id]\n"
        "      CSeq: 1 ACK\n"
        "      Content-Length: 0\n"
        "    ]]>\n"
        "  </send>\n"
        "  <pause milliseconds=\"5000\"/>\n"
        "  <send retrans=\"500\">\n"
        "    <![CDATA[\n"
        "      BYE sip:[service]@[remote_ip]:[remote_port] SIP/2.0\n"
        "      Via: SIP/2.0/[transport] [local_ip]:[local_port];branch=[branch]\n"
        "      From: sipp <sip:sipp@[local_ip]:[local_port]>;tag=[pid]SIPpTag00[call_number]\n"
        "      To: [service] <sip:[service]@[remote_ip]:[remote_port]>[peer_tag_param]\n"
        "      Call-ID: [call_id]\n"
        "      CSeq: 2 BYE\n"
        "      Content-Length: 0\n"
        "    ]]>\n"
        "  </send>\n"
        "  <recv response=\"200\" crlf=\"true\">\n"
        "  </recv>\n"
        "</scenario>";

    for (auto _ : state) {
        char* xml_copy = strdup(simple_xml);

        // Simulate XML parsing by finding key elements
        char* scenario_start = strstr(xml_copy, "<scenario");
        char* send_elements = xml_copy;
        int send_count = 0;

        while ((send_elements = strstr(send_elements, "<send")) != nullptr) {
            send_count++;
            send_elements++;
        }

        char* recv_elements = xml_copy;
        int recv_count = 0;

        while ((recv_elements = strstr(recv_elements, "<recv")) != nullptr) {
            recv_count++;
            recv_elements++;
        }

        benchmark::DoNotOptimize(scenario_start);
        benchmark::DoNotOptimize(send_count);
        benchmark::DoNotOptimize(recv_count);

        free(xml_copy);
    }
}

// Benchmark complex XML parsing with many elements
BENCHMARK_F(XML, ComplexXMLParsing)(benchmark::State& state) {
    std::string complex_xml =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<!DOCTYPE scenario SYSTEM \"sipp.dtd\">\n"
        "<scenario name=\"Complex Scenario\">\n";

    // Add many send/recv pairs
    for (int i = 0; i < 50; i++) {
        complex_xml += "  <send retrans=\"500\">\n";
        complex_xml += "    <![CDATA[\n";
        complex_xml += "      METHOD" + std::to_string(i) + " sip:user@example.com SIP/2.0\n";
        complex_xml += "      Via: SIP/2.0/UDP [local_ip]:[local_port];branch=[branch]\n";
        complex_xml += "      Content-Length: 0\n";
        complex_xml += "    ]]>\n";
        complex_xml += "  </send>\n";
        complex_xml += "  <recv response=\"" + std::to_string(200 + i % 100) + "\">\n";
        complex_xml += "  </recv>\n";
    }

    complex_xml += "</scenario>";

    for (auto _ : state) {
        const char* xml_str = complex_xml.c_str();
        char* xml_copy = strdup(xml_str);

        // Count elements
        char* ptr = xml_copy;
        int total_elements = 0;

        while ((ptr = strchr(ptr, '<')) != nullptr) {
            if (ptr[1] != '/' && ptr[1] != '!' && ptr[1] != '?') {
                total_elements++;
            }
            ptr++;
        }

        benchmark::DoNotOptimize(total_elements);
        free(xml_copy);
    }
}

// Benchmark CDATA extraction
BENCHMARK_F(XML, CDATAExtraction)(benchmark::State& state) {
    const char* cdata_xml =
        "<send>\n"
        "  <![CDATA[\n"
        "    INVITE sip:user@example.com SIP/2.0\n"
        "    Via: SIP/2.0/UDP [local_ip]:[local_port];branch=[branch]\n"
        "    From: <sip:caller@[local_ip]>;tag=[call_number]\n"
        "    To: <sip:user@example.com>\n"
        "    Call-ID: [call_id]\n"
        "    CSeq: 1 INVITE\n"
        "    Content-Type: application/sdp\n"
        "    Content-Length: [len]\n"
        "    \n"
        "    v=0\n"
        "    o=caller 123456 654321 IN IP4 [local_ip]\n"
        "    s=SIP Call\n"
        "    c=IN IP4 [local_ip]\n"
        "    t=0 0\n"
        "    m=audio [audio_port] RTP/AVP 0\n"
        "    a=rtpmap:0 PCMU/8000\n"
        "  ]]>\n"
        "</send>";

    for (auto _ : state) {
        char* xml_copy = strdup(cdata_xml);

        char* cdata_start = strstr(xml_copy, "<![CDATA[");
        char* cdata_end = strstr(xml_copy, "]]>");

        if (cdata_start && cdata_end) {
            cdata_start += 9; // Skip "<![CDATA["
            size_t cdata_len = cdata_end - cdata_start;

            char* cdata_content = (char*)malloc(cdata_len + 1);
            memcpy(cdata_content, cdata_start, cdata_len);
            cdata_content[cdata_len] = '\0';

            benchmark::DoNotOptimize(cdata_content);
            free(cdata_content);
        }

        free(xml_copy);
    }
}

// Benchmark attribute parsing
BENCHMARK_F(XML, AttributeParsing)(benchmark::State& state) {
    const char* element_with_attrs =
        "<recv response=\"200\" rtd=\"true\" timeout=\"5000\" optional=\"false\" crlf=\"true\">";

    for (auto _ : state) {
        char* elem_copy = strdup(element_with_attrs);

        // Extract attributes
        struct {
            char response[32];
            char rtd[16];
            char timeout[16];
            char optional[16];
            char crlf[16];
        } attrs;

        memset(&attrs, 0, sizeof(attrs));

        // Simulate attribute extraction
        char* response_attr = strstr(elem_copy, "response=\"");
        if (response_attr) {
            response_attr += 10;
            char* end_quote = strchr(response_attr, '"');
            if (end_quote) {
                size_t len = std::min((size_t)(end_quote - response_attr), sizeof(attrs.response) - 1);
                memcpy(attrs.response, response_attr, len);
            }
        }

        char* rtd_attr = strstr(elem_copy, "rtd=\"");
        if (rtd_attr) {
            rtd_attr += 5;
            char* end_quote = strchr(rtd_attr, '"');
            if (end_quote) {
                size_t len = std::min((size_t)(end_quote - rtd_attr), sizeof(attrs.rtd) - 1);
                memcpy(attrs.rtd, rtd_attr, len);
            }
        }

        benchmark::DoNotOptimize(attrs);
        free(elem_copy);
    }
}

// Benchmark XML validation
BENCHMARK_F(XML, XMLValidation)(benchmark::State& state) {
    const char* xml_samples[] = {
        "<scenario name=\"test\"></scenario>",
        "<send><![CDATA[SIP MESSAGE]]></send>",
        "<recv response=\"200\"></recv>",
        "<pause milliseconds=\"1000\"/>",
        "<nop></nop>"
    };
    const int num_samples = sizeof(xml_samples) / sizeof(xml_samples[0]);
    int sample_index = 0;

    for (auto _ : state) {
        const char* xml = xml_samples[sample_index % num_samples];
        sample_index++;

        char* xml_copy = strdup(xml);

        // Basic XML validation
        bool valid = true;
        char* ptr = xml_copy;
        int bracket_count = 0;

        while (*ptr) {
            if (*ptr == '<') {
                bracket_count++;
            } else if (*ptr == '>') {
                bracket_count--;
                if (bracket_count < 0) {
                    valid = false;
                    break;
                }
            }
            ptr++;
        }

        if (bracket_count != 0) {
            valid = false;
        }

        benchmark::DoNotOptimize(valid);
        free(xml_copy);
    }
}

// Benchmark scenario compilation
BENCHMARK_F(XML, ScenarioCompilation)(benchmark::State& state) {
    const char* scenario_xml =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<scenario name=\"Benchmark Scenario\">\n"
        "  <send retrans=\"500\">\n"
        "    <![CDATA[INVITE sip:user@[remote_ip] SIP/2.0]]>\n"
        "  </send>\n"
        "  <recv response=\"100\" optional=\"true\"></recv>\n"
        "  <recv response=\"200\"></recv>\n"
        "  <send>\n"
        "    <![CDATA[ACK sip:user@[remote_ip] SIP/2.0]]>\n"
        "  </send>\n"
        "  <pause milliseconds=\"1000\"/>\n"
        "  <send>\n"
        "    <![CDATA[BYE sip:user@[remote_ip] SIP/2.0]]>\n"
        "  </send>\n"
        "  <recv response=\"200\"></recv>\n"
        "</scenario>";

    for (auto _ : state) {
        char* xml_copy = strdup(scenario_xml);

        // Simulate scenario compilation by counting and categorizing steps
        struct {
            int send_count;
            int recv_count;
            int pause_count;
            int total_steps;
        } stats;

        memset(&stats, 0, sizeof(stats));

        char* ptr = xml_copy;
        while ((ptr = strstr(ptr, "<send")) != nullptr) {
            stats.send_count++;
            stats.total_steps++;
            ptr++;
        }

        ptr = xml_copy;
        while ((ptr = strstr(ptr, "<recv")) != nullptr) {
            stats.recv_count++;
            stats.total_steps++;
            ptr++;
        }

        ptr = xml_copy;
        while ((ptr = strstr(ptr, "<pause")) != nullptr) {
            stats.pause_count++;
            stats.total_steps++;
            ptr++;
        }

        benchmark::DoNotOptimize(stats);
        free(xml_copy);
    }
}
