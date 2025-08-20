# SIPp Benchmark Suite

This directory contains performance benchmarks for the SIPp SIP testing tool. The benchmarks are designed to measure the performance of critical SIPp components and operations.

## Overview

The benchmark suite is organized into four main categories:

- **Core Performance** (`bench/sipp_benchmark.cpp`): Message parsing, variable substitution, memory management
- **Network Operations** (`bench/sipp_network_benchmark.cpp`): Socket operations, DNS resolution, packet handling
- **XML Processing** (`bench/sipp_xml_benchmark.cpp`): Scenario parsing, XML validation, CDATA extraction
- **Integration Tests** (`bench/sipp_integration_benchmark.cpp`): End-to-end workflows, concurrent execution

## Quick Start

The easiest way to use the benchmarks is with the provided script:

```bash
# Setup benchmark environment (initialize git submodules)
./run_benchmarks.sh setup

# Build benchmarks
./run_benchmarks.sh build

# Run all benchmarks
./run_benchmarks.sh run

# Run specific categories
./run_benchmarks.sh run-core
./run_benchmarks.sh run-network
./run_benchmarks.sh run-xml
./run_benchmarks.sh run-integration

# List all available benchmarks
./run_benchmarks.sh list
```

## Manual Build Process

If you prefer to build manually:

```bash
# Initialize the Google Benchmark submodule
git submodule update --init benchmark

# Configure with benchmarks enabled (they are disabled by default)
cmake -DBUILD_BENCHMARKS=ON -S . -B build

# Build benchmark executable
cmake --build build --target sipp_benchmark

# Run benchmarks
./build/sipp_benchmark
```

## Prerequisites

- **Google Benchmark library** (automatically downloaded as git submodule)
- **CMake 3.10 or higher**
- **C++17 compatible compiler**

## Benchmark Categories

### 1. Core Performance Benchmarks (`sipp_benchmark.cpp`)
- **MessageParsing**: Tests SIP message parsing performance
- **VariableSubstitution**: Tests variable substitution in message templates
- **StringOperations**: Tests common string operations used throughout SIPp
- **CallIdGeneration**: Tests call ID generation performance
- **MemoryAllocation**: Tests typical memory allocation patterns
- **LargeMessageHandling**: Tests performance with large SIP messages
- **CallDataStructures**: Tests call data structure creation and management

### 2. Network Performance Benchmarks (`sipp_network_benchmark.cpp`)
- **SocketAddressParsing**: Tests socket address parsing for IPv4/IPv6
- **MessageBufferPreparation**: Tests SIP message buffer preparation
- **PacketSizeCalculation**: Tests UDP packet size calculations
- **ConnectionStateTracking**: Tests connection state management
- **DNSResolutionSimulation**: Tests DNS resolution simulation
- **PortAllocation**: Tests RTP port allocation algorithms
- **ChecksumCalculation**: Tests checksum calculations for RTP/UDP
- **BufferManagement**: Tests network buffer management

### 3. XML/Scenario Performance Benchmarks (`sipp_xml_benchmark.cpp`)
- **SimpleXMLParsing**: Tests basic XML scenario parsing
- **ComplexXMLParsing**: Tests parsing of complex scenarios with many elements
- **CDATAExtraction**: Tests CDATA content extraction from XML
- **AttributeParsing**: Tests XML attribute parsing
- **XMLValidation**: Tests basic XML validation
- **ScenarioCompilation**: Tests scenario compilation from XML

## Understanding Results

Benchmark results show:
- **Time**: Average time per iteration
- **CPU**: CPU time per iteration
- **Iterations**: Number of iterations run
- **Bytes/second**: Throughput for data processing benchmarks

Example output:
```
Benchmark                           Time             CPU   Iterations
----------------------------------------------------------------
MessageParsing                   1234 ns         1230 ns       567890
VariableSubstitution             2345 ns         2340 ns       298765
LargeMessageHandling             5678 ns         5670 ns       123456
```

## Performance Optimization

Use these benchmarks to:

1. **Identify bottlenecks**: Compare before/after performance
2. **Regression testing**: Ensure performance doesn't degrade
3. **Optimization validation**: Verify that optimizations actually improve performance
4. **Platform comparison**: Compare performance across different systems

## Adding New Benchmarks

To add new benchmarks:

1. Choose the appropriate file based on category
2. Use the `BENCHMARK_F` macro with the appropriate fixture
3. Follow the existing patterns for setup and measurement
4. Use `benchmark::DoNotOptimize()` to prevent compiler optimizations that might skew results

Example:
```cpp
BENCHMARK_F(SippBenchmarkFixture, MyNewBenchmark)(benchmark::State& state) {
    for (auto _ : state) {
        // Your code to benchmark
        auto result = function_to_test();
        benchmark::DoNotOptimize(result);
    }
}
```

## Continuous Integration

Consider running benchmarks in CI to catch performance regressions:

```bash
# In CI script
./sipp_benchmark --benchmark_format=json --benchmark_out=ci_results.json
# Parse results and compare with baseline
```

## Tips for Accurate Benchmarking

1. **Consistent environment**: Run on dedicated hardware when possible
2. **Multiple runs**: Use `--benchmark_repetitions=N` for statistical significance
3. **Warm-up**: Benchmarks automatically include warm-up iterations
4. **System load**: Minimize other processes during benchmarking
5. **Compiler optimizations**: Build with `-O2` or `-O3` for realistic results

## Troubleshooting

### Google Benchmark not found
```
CMake Error: Could not find Google Benchmark
```
Install Google Benchmark development package or build from source.

### Linking errors
```
undefined reference to benchmark symbols
```
Ensure Google Benchmark library is properly installed and linked.

### Performance varies significantly
- Check system load during benchmarking
- Use `--benchmark_repetitions=10` for better statistics
- Consider CPU governor settings (performance vs powersave)
