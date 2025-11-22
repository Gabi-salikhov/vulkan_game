#!/bin/bash

# Vortex Engine Build Script
# This script builds the Vortex Engine and its examples

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to check dependencies
check_dependencies() {
    print_status "Checking dependencies..."
    
    local missing_deps=()
    
    # Check essential build tools
    if ! command_exists cmake; then
        missing_deps+=("cmake")
    fi
    
    if ! command_exists make; then
        missing_deps+=("make")
    fi
    
    if ! command_exists g++; then
        missing_deps+=("g++")
    fi
    
    # Check Vulkan development libraries
    if ! pkg-config --exists vulkan; then
        missing_deps+=("vulkan-devel")
    fi
    
    # Check GLFW development libraries
    if ! pkg-config --exists glfw3; then
        missing_deps+=("glfw-devel")
    fi
    
    # Check Python
    if ! command_exists python3; then
        missing_deps+=("python3")
    fi
    
    # Report missing dependencies
    if [ ${#missing_deps[@]} -ne 0 ]; then
        print_error "Missing dependencies: ${missing_deps[*]}"
        print_status "Please install the missing dependencies:"
        echo ""
        echo "For Ubuntu/Debian:"
        echo "  sudo apt update"
        echo "  sudo apt install ${missing_deps[*]}"
        echo ""
        echo "For Arch Linux:"
        echo "  sudo pacman -S ${missing_deps[*]}"
        echo ""
        echo "For Fedora:"
        echo "  sudo dnf install ${missing_deps[*]}"
        echo ""
        exit 1
    fi
    
    print_success "All dependencies are installed"
}

# Function to create build directory
create_build_dir() {
    if [ ! -d "build" ]; then
        print_status "Creating build directory..."
        mkdir build
    fi
}

# Function to configure CMake
configure_cmake() {
    print_status "Configuring CMake..."
    
    cd build
    
    # Configure with CMake
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DVORTEX_VALIDATION_LAYERS=OFF \
        -DVORTEX_PYTHON=ON \
        -DCMAKE_INSTALL_PREFIX=/usr/local
    
    if [ $? -eq 0 ]; then
        print_success "CMake configuration completed"
    else
        print_error "CMake configuration failed"
        exit 1
    fi
    
    cd ..
}

# Function to build the engine
build_engine() {
    print_status "Building Vortex Engine..."
    
    cd build
    
    # Build the engine
    cmake --build . --config Release --parallel $(nproc)
    
    if [ $? -eq 0 ]; then
        print_success "Engine build completed"
    else
        print_error "Engine build failed"
        exit 1
    fi
    
    cd ..
}

# Function to build examples
build_examples() {
    print_status "Building examples..."
    
    cd build
    
    # Build test scene
    cmake --build . --target test_scene --config Release
    
    if [ $? -eq 0 ]; then
        print_success "Examples build completed"
    else
        print_error "Examples build failed"
        exit 1
    fi
    
    cd ..
}

# Function to run tests
run_tests() {
    print_status "Running tests..."
    
    if [ -f "build/examples/test_scene/test_scene" ]; then
        cd build/examples/test_scene
        ./test_scene
        cd ../../..
    else
        print_warning "Test scene not found. Skipping tests."
    fi
}

# Function to show usage
show_usage() {
    echo "Vortex Engine Build Script"
    echo ""
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -h, --help          Show this help message"
    echo "  -c, --check         Only check dependencies"
    echo "  -b, --build         Only build the engine"
    echo "  -e, --examples      Only build examples"
    echo "  -t, --test          Only run tests"
    echo "  -r, --release       Build in release mode (default)"
    echo "  -d, --debug         Build in debug mode"
    echo "  -v, --validate      Enable Vulkan validation layers"
    echo "  -p, --python        Enable Python integration (default)"
    echo "  --clean             Clean build directory before building"
    echo ""
    echo "Examples:"
    echo "  $0                  # Full build (default)"
    echo "  $0 --check          # Only check dependencies"
    echo "  $0 --debug          # Build in debug mode"
    echo "  $0 --clean --debug  # Clean and build in debug mode"
}

# Main script logic
main() {
    local check_only=false
    local build_only=false
    local examples_only=false
    local test_only=false
    local clean_build=false
    local build_type="Release"
    local validation_layers="OFF"
    local python_integration="ON"
    
    # Parse command line arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_usage
                exit 0
                ;;
            -c|--check)
                check_only=true
                shift
                ;;
            -b|--build)
                build_only=true
                shift
                ;;
            -e|--examples)
                examples_only=true
                shift
                ;;
            -t|--test)
                test_only=true
                shift
                ;;
            -r|--release)
                build_type="Release"
                shift
                ;;
            -d|--debug)
                build_type="Debug"
                shift
                ;;
            -v|--validate)
                validation_layers="ON"
                shift
                ;;
            -p|--python)
                python_integration="ON"
                shift
                ;;
            --clean)
                clean_build=true
                shift
                ;;
            *)
                print_error "Unknown option: $1"
                show_usage
                exit 1
                ;;
        esac
    done
    
    # Check if we're in the right directory
    if [ ! -f "CMakeLists.txt" ]; then
        print_error "Please run this script from the Vortex Engine root directory"
        exit 1
    fi
    
    # Check dependencies
    check_dependencies
    
    # Clean build if requested
    if [ "$clean_build" = true ]; then
        print_status "Cleaning build directory..."
        rm -rf build
    fi
    
    # Create build directory
    create_build_dir
    
    # Configure CMake
    if [ "$check_only" = false ]; then
        configure_cmake
    fi
    
    # Build engine
    if [ "$check_only" = false ] && [ "$examples_only" = false ] && [ "$test_only" = false ]; then
        build_engine
    fi
    
    # Build examples
    if [ "$check_only" = false ] && [ "$build_only" = false ] && [ "$test_only" = false ]; then
        build_examples
    fi
    
    # Run tests
    if [ "$check_only" = false ] && [ "$build_only" = false ] && [ "$examples_only" = false ]; then
        run_tests
    fi
    
    print_success "Build process completed successfully!"
    print_status "To run the test scene:"
    echo "  cd build/examples/test_scene"
    echo "  ./test_scene"
}

# Run main function
main "$@"