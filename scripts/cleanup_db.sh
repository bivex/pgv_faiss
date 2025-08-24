#!/bin/bash

# pgv_faiss Database Cleanup Script
# This script provides easy access to database cleanup operations

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/../build"
CLEANUP_TOOL="${BUILD_DIR}/examples/db_cleanup"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_header() {
    echo -e "${BLUE}=== pgv_faiss Database Cleanup Script ===${NC}"
    echo
}

print_usage() {
    echo "Usage: $0 [command]"
    echo
    echo "Commands:"
    echo "  all       - Clear all vector tables and FAISS indices"
    echo "  vectors   - Clear only vector tables"
    echo "  indices   - Clear only FAISS index tables"
    echo "  stats     - Show database statistics"
    echo "  vacuum    - Clear all and vacuum database"
    echo "  help      - Show this help message"
    echo
    echo "Examples:"
    echo "  $0 all              # Complete cleanup"
    echo "  $0 vectors          # Clear only vector tables"
    echo "  $0 stats            # Show database info"
    echo "  $0 vacuum           # Full cleanup with vacuum"
}

check_prerequisites() {
    # Check if build directory exists
    if [ ! -d "$BUILD_DIR" ]; then
        echo -e "${RED}Error: Build directory not found at $BUILD_DIR${NC}"
        echo "Please build the project first:"
        echo "  cd pgv_faiss && mkdir build && cd build"
        echo "  cmake .. && make"
        exit 1
    fi
    
    # Check if cleanup tool exists
    if [ ! -f "$CLEANUP_TOOL" ]; then
        echo -e "${RED}Error: Database cleanup tool not found at $CLEANUP_TOOL${NC}"
        echo "Please build the project first:"
        echo "  cd pgv_faiss/build && make db_cleanup"
        exit 1
    fi
    
    # Check if cleanup tool is executable
    if [ ! -x "$CLEANUP_TOOL" ]; then
        echo -e "${YELLOW}Warning: Making cleanup tool executable...${NC}"
        chmod +x "$CLEANUP_TOOL"
    fi
}

run_cleanup() {
    local command="$1"
    
    echo -e "${BLUE}Setting up environment...${NC}"
    export LD_LIBRARY_PATH="${BUILD_DIR}/src/lib:$LD_LIBRARY_PATH"
    
    case "$command" in
        "all")
            echo -e "${YELLOW}Performing complete database cleanup...${NC}"
            "$CLEANUP_TOOL" --all
            ;;
        "vectors")
            echo -e "${YELLOW}Clearing vector tables...${NC}"
            "$CLEANUP_TOOL" --vectors
            ;;
        "indices")
            echo -e "${YELLOW}Clearing FAISS indices...${NC}"
            "$CLEANUP_TOOL" --indices
            ;;
        "stats")
            echo -e "${YELLOW}Showing database statistics...${NC}"
            "$CLEANUP_TOOL" --stats
            ;;
        "vacuum")
            echo -e "${YELLOW}Performing complete cleanup with vacuum...${NC}"
            "$CLEANUP_TOOL" --all --vacuum
            ;;
        *)
            echo -e "${RED}Error: Unknown command '$command'${NC}"
            echo
            print_usage
            exit 1
            ;;
    esac
    
    local exit_code=$?
    
    if [ $exit_code -eq 0 ]; then
        echo
        echo -e "${GREEN}✅ Operation completed successfully!${NC}"
    else
        echo
        echo -e "${RED}❌ Operation failed with exit code $exit_code${NC}"
        echo "Please check the error messages above."
    fi
    
    return $exit_code
}

main() {
    print_header
    
    if [ $# -eq 0 ] || [ "$1" = "help" ] || [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
        print_usage
        exit 0
    fi
    
    check_prerequisites
    run_cleanup "$1"
}

# Run main function with all arguments
main "$@"