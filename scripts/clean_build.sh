#!/bin/bash

# pgv_faiss Build Cleanup Script
# This script removes temporary build files and artifacts

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="${SCRIPT_DIR}/.."
BUILD_DIR="${PROJECT_ROOT}/build"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_header() {
    echo -e "${BLUE}=== pgv_faiss Build Cleanup Script ===${NC}"
    echo
}

print_usage() {
    echo "Usage: $0 [command]"
    echo
    echo "Commands:"
    echo "  all       - Clean all build files and artifacts"
    echo "  build     - Clean only build directory contents"
    echo "  temp      - Clean temporary files (CMake cache, logs)"
    echo "  results   - Clean benchmark and test result files"
    echo "  backup    - Clean backup files (.bak, .orig, ~)"
    echo "  force     - Force clean everything (including .git ignored files)"
    echo "  help      - Show this help message"
    echo
    echo "Examples:"
    echo "  $0 all              # Complete cleanup"
    echo "  $0 build            # Clean build directory only"
    echo "  $0 temp             # Clean temporary files only"
    echo "  $0 results          # Clean result files only"
}

check_project_root() {
    if [ ! -f "$PROJECT_ROOT/CMakeLists.txt" ]; then
        echo -e "${RED}Error: Not in pgv_faiss project directory${NC}"
        echo "Please run this script from the pgv_faiss project root or scripts directory"
        exit 1
    fi
}

clean_build_directory() {
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    
    if [ -d "$BUILD_DIR" ]; then
        echo "Removing contents of: $BUILD_DIR"
        
        # List what will be removed
        if [ "$(ls -A "$BUILD_DIR" 2>/dev/null)" ]; then
            echo "Files to be removed:"
            ls -la "$BUILD_DIR" | head -10
            if [ $(ls -1 "$BUILD_DIR" | wc -l) -gt 8 ]; then
                echo "... and $(( $(ls -1 "$BUILD_DIR" | wc -l) - 8 )) more files"
            fi
            echo
            
            # Remove all contents but keep the directory
            rm -rf "$BUILD_DIR"/*
            rm -rf "$BUILD_DIR"/.*  2>/dev/null || true
            echo -e "${GREEN}✅ Build directory cleaned${NC}"
        else
            echo -e "${GREEN}✅ Build directory already clean${NC}"
        fi
    else
        echo -e "${GREEN}✅ Build directory doesn't exist${NC}"
    fi
}

clean_temp_files() {
    echo -e "${YELLOW}Cleaning temporary files...${NC}"
    
    cd "$PROJECT_ROOT"
    
    # CMake temporary files
    find . -name "CMakeCache.txt" -type f -delete 2>/dev/null || true
    find . -name "CMakeFiles" -type d -exec rm -rf {} + 2>/dev/null || true
    find . -name "cmake_install.cmake" -type f -delete 2>/dev/null || true
    find . -name "Makefile" -path "*/build/*" -type f -delete 2>/dev/null || true
    
    # Compilation artifacts
    find . -name "*.o" -type f -delete 2>/dev/null || true
    find . -name "*.so" -path "*/build/*" -type f -delete 2>/dev/null || true
    find . -name "*.a" -path "*/build/*" -type f -delete 2>/dev/null || true
    
    # Log files
    find . -name "*.log" -type f -delete 2>/dev/null || true
    find . -name "*.tmp" -type f -delete 2>/dev/null || true
    
    echo -e "${GREEN}✅ Temporary files cleaned${NC}"
}

clean_result_files() {
    echo -e "${YELLOW}Cleaning result and output files...${NC}"
    
    cd "$PROJECT_ROOT"
    
    # Benchmark results
    find . -name "*benchmark_results*.csv" -type f -delete 2>/dev/null || true
    find . -name "*test_results*.txt" -type f -delete 2>/dev/null || true
    find . -name "*output*.log" -type f -delete 2>/dev/null || true
    
    # Core dumps and crash files
    find . -name "core" -type f -delete 2>/dev/null || true
    find . -name "core.*" -type f -delete 2>/dev/null || true
    find . -name "vgcore.*" -type f -delete 2>/dev/null || true
    
    echo -e "${GREEN}✅ Result files cleaned${NC}"
}

clean_backup_files() {
    echo -e "${YELLOW}Cleaning backup files...${NC}"
    
    cd "$PROJECT_ROOT"
    
    # Backup files
    find . -name "*.bak" -type f -delete 2>/dev/null || true
    find . -name "*.orig" -type f -delete 2>/dev/null || true
    find . -name "*~" -type f -delete 2>/dev/null || true
    find . -name ".#*" -type f -delete 2>/dev/null || true
    find . -name "#*#" -type f -delete 2>/dev/null || true
    
    # Editor temporary files
    find . -name "*.swp" -type f -delete 2>/dev/null || true
    find . -name "*.swo" -type f -delete 2>/dev/null || true
    find . -name ".*.swp" -type f -delete 2>/dev/null || true
    
    echo -e "${GREEN}✅ Backup files cleaned${NC}"
}

force_clean() {
    echo -e "${RED}Force cleaning all files (including git-ignored)...${NC}"
    echo -e "${YELLOW}Warning: This will remove all untracked files!${NC}"
    
    read -p "Are you sure? This cannot be undone! [y/N]: " -n 1 -r
    echo
    
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        cd "$PROJECT_ROOT"
        
        # Use git clean if in a git repository
        if [ -d ".git" ]; then
            echo "Using git clean to remove untracked files..."
            git clean -fdx
        else
            echo "Not a git repository, performing manual cleanup..."
            clean_build_directory
            clean_temp_files
            clean_result_files
            clean_backup_files
        fi
        
        echo -e "${GREEN}✅ Force cleanup completed${NC}"
    else
        echo -e "${YELLOW}Force cleanup cancelled${NC}"
    fi
}

show_cleanup_summary() {
    echo
    echo -e "${BLUE}=== Cleanup Summary ===${NC}"
    
    # Check build directory size
    if [ -d "$BUILD_DIR" ]; then
        build_size=$(du -sh "$BUILD_DIR" 2>/dev/null | cut -f1)
        echo "Build directory size: $build_size"
    else
        echo "Build directory: Not present"
    fi
    
    # Count remaining files
    temp_files=$(find "$PROJECT_ROOT" -name "*.tmp" -o -name "*.log" -o -name "CMakeCache.txt" 2>/dev/null | wc -l)
    result_files=$(find "$PROJECT_ROOT" -name "*results*.csv" -o -name "*output*.log" 2>/dev/null | wc -l)
    backup_files=$(find "$PROJECT_ROOT" -name "*.bak" -o -name "*~" -o -name "*.orig" 2>/dev/null | wc -l)
    
    echo "Remaining temporary files: $temp_files"
    echo "Remaining result files: $result_files"
    echo "Remaining backup files: $backup_files"
    
    if [ $temp_files -eq 0 ] && [ $result_files -eq 0 ] && [ $backup_files -eq 0 ]; then
        echo -e "${GREEN}✅ Project is clean!${NC}"
    else
        echo -e "${YELLOW}⚠️  Some files remain (may be intentional)${NC}"
    fi
}

main() {
    print_header
    
    if [ $# -eq 0 ] || [ "$1" = "help" ] || [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
        print_usage
        exit 0
    fi
    
    check_project_root
    
    case "$1" in
        "all")
            echo -e "${YELLOW}Performing complete cleanup...${NC}"
            clean_build_directory
            clean_temp_files
            clean_result_files
            clean_backup_files
            ;;
        "build")
            clean_build_directory
            ;;
        "temp")
            clean_temp_files
            ;;
        "results")
            clean_result_files
            ;;
        "backup")
            clean_backup_files
            ;;
        "force")
            force_clean
            ;;
        *)
            echo -e "${RED}Error: Unknown command '$1'${NC}"
            echo
            print_usage
            exit 1
            ;;
    esac
    
    show_cleanup_summary
    
    echo
    echo -e "${GREEN}✅ Cleanup completed successfully!${NC}"
    echo "You can now run a fresh build with:"
    echo "  mkdir build && cd build"
    echo "  cmake .. && make"
}

# Run main function with all arguments
main "$@"