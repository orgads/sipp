#!/bin/bash
# SIPp Benchmark Runner Script
# This script helps setup and run SIPp benchmarks

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

usage() {
    cat << EOF
SIPp Benchmark Runner

Usage: $0 [OPTIONS] [COMMAND]

OPTIONS:
    -h, --help          Show this help message
    -b, --build-dir     Build directory (default: build)
    -j, --jobs          Number of parallel jobs for compilation (default: nproc)
    --benchmark-args    Additional arguments to pass to benchmark executable

COMMANDS:
    setup               Initialize benchmark environment (git submodules)
    build               Build benchmarks (requires -DBUILD_BENCHMARKS=ON)
    run                 Run all benchmarks
    run-core            Run only core performance benchmarks
    run-network         Run only network benchmarks
    run-xml             Run only XML parsing benchmarks
    run-integration     Run only integration benchmarks
    list                List all available benchmarks
    clean               Clean benchmark build artifacts

EXAMPLES:
    $0 setup            # Initialize benchmark environment
    $0 build            # Build benchmarks
    $0 run              # Run all benchmarks
    $0 run --benchmark-args="--benchmark_filter=Core*"  # Run filtered benchmarks
    $0 run-core         # Run only core benchmarks

EOF
}

BUILD_DIR="build"
JOBS=$(nproc 2>/dev/null || echo "4")
BENCHMARK_ARGS=""

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            usage
            exit 0
            ;;
        -b|--build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        -j|--jobs)
            JOBS="$2"
            shift 2
            ;;
        --benchmark-args)
            BENCHMARK_ARGS="$2"
            shift 2
            ;;
        setup|build|run|run-core|run-network|run-xml|run-integration|list|clean)
            COMMAND="$1"
            shift
            ;;
        *)
            print_error "Unknown option: $1"
            usage
            exit 1
            ;;
    esac
done

if [ -z "$COMMAND" ]; then
    print_error "No command specified"
    usage
    exit 1
fi

# Check if we're in the SIPp project root
if [ ! -f "CMakeLists.txt" ] || [ ! -d "include" ]; then
    print_error "This script must be run from the SIPp project root directory"
    exit 1
fi

setup_benchmarks() {
    print_info "Setting up benchmark environment..."

    if [ ! -d "benchmark/.git" ]; then
        print_info "Initializing Google Benchmark submodule..."
        git submodule update --init benchmark
    else
        print_info "Google Benchmark submodule already initialized"
    fi

    print_info "Benchmark setup complete!"
    print_info "Next steps:"
    print_info "  1. Run: $0 build"
    print_info "  2. Run: $0 run"
}

build_benchmarks() {
    print_info "Building benchmarks..."

    if [ ! -d "benchmark/.git" ]; then
        print_warning "Google Benchmark submodule not found. Running setup first..."
        setup_benchmarks
    fi

    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    print_info "Configuring with benchmarks enabled..."
    cmake -DBUILD_BENCHMARKS=ON ..

    print_info "Building benchmark executable..."
    cmake --build . --target sipp_benchmark -j "$JOBS"

    cd ..
    print_info "Benchmark build complete!"
    print_info "Executable: $BUILD_DIR/sipp_benchmark"
}

run_benchmarks() {
    local filter="$1"
    local executable="$BUILD_DIR/sipp_benchmark"

    if [ ! -f "$executable" ]; then
        print_error "Benchmark executable not found at $executable"
        print_info "Run '$0 build' first to build the benchmarks"
        exit 1
    fi

    print_info "Running SIPp benchmarks..."
    if [ -n "$filter" ]; then
        print_info "Filter: $filter"
        "$executable" --benchmark_filter="$filter" $BENCHMARK_ARGS
    else
        "$executable" $BENCHMARK_ARGS
    fi
}

list_benchmarks() {
    local executable="$BUILD_DIR/sipp_benchmark"

    if [ ! -f "$executable" ]; then
        print_error "Benchmark executable not found at $executable"
        print_info "Run '$0 build' first to build the benchmarks"
        exit 1
    fi

    print_info "Available benchmarks:"
    "$executable" --benchmark_list_tests
}

clean_benchmarks() {
    print_info "Cleaning benchmark build artifacts..."
    if [ -d "$BUILD_DIR" ]; then
        rm -f "$BUILD_DIR/sipp_benchmark"
        rm -rf "$BUILD_DIR"/CMakeFiles/sipp_benchmark.dir/
        print_info "Benchmark artifacts cleaned"
    else
        print_info "Build directory not found, nothing to clean"
    fi
}

# Execute commands
case "$COMMAND" in
    setup)
        setup_benchmarks
        ;;
    build)
        build_benchmarks
        ;;
    run)
        run_benchmarks
        ;;
    run-core)
        run_benchmarks "SippBenchmarkFixture/*"
        ;;
    run-network)
        run_benchmarks "NetworkBenchmarkFixture/*"
        ;;
    run-xml)
        run_benchmarks "XMLBenchmarkFixture/*"
        ;;
    run-integration)
        run_benchmarks "IntegrationBenchmarkFixture/*"
        ;;
    list)
        list_benchmarks
        ;;
    clean)
        clean_benchmarks
        ;;
    *)
        print_error "Unknown command: $COMMAND"
        usage
        exit 1
        ;;
esac
