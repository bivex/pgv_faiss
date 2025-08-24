# pgv_faiss Scripts

This directory contains utility scripts for managing and maintaining the pgv_faiss project.

## Available Scripts

### 1. Database Cleanup Script (`cleanup_db.sh`)

Manages PostgreSQL database cleanup operations for testing and benchmarking.

**Purpose**: Clear vector tables and FAISS indices from the database.

**Usage**:
```bash
./cleanup_db.sh [command]
```

**Commands**:
- `all` - Clear all vector tables and FAISS indices
- `vectors` - Clear only vector tables
- `indices` - Clear only FAISS index tables
- `stats` - Show database statistics
- `vacuum` - Clear all and vacuum database

**Example**:
```bash
./cleanup_db.sh all      # Complete database cleanup
./cleanup_db.sh stats    # Show database information
```

### 2. Build Cleanup Script (`clean_build.sh`)

Manages temporary build files and development artifacts.

**Purpose**: Clean build directory, temporary files, and development artifacts.

**Usage**:
```bash
./clean_build.sh [command]
```

**Commands**:
- `all` - Clean all build files and artifacts
- `build` - Clean only build directory contents
- `temp` - Clean temporary files (CMake cache, logs)
- `results` - Clean benchmark and test result files
- `backup` - Clean backup files (.bak, .orig, ~)
- `force` - Force clean everything (including git-ignored files)

**Example**:
```bash
./clean_build.sh all     # Complete cleanup
./clean_build.sh build   # Clean build directory only
./clean_build.sh temp    # Clean temporary files only
```

## Script Features

### Common Features:
- **Colorized Output**: Easy-to-read colored terminal output
- **Safety Checks**: Verify project directory and file existence
- **Help Documentation**: Built-in help with `--help` or `help` command
- **Error Handling**: Graceful error handling with informative messages

### Database Cleanup Features:
- **Selective Cleanup**: Choose specific database objects to clean
- **Safety Patterns**: Only removes tables matching vector-related patterns
- **Statistics Display**: Show database size and table information
- **Vacuum Support**: Optional database vacuum for space reclamation

### Build Cleanup Features:
- **Comprehensive Cleaning**: Removes all types of build artifacts
- **Git Integration**: Uses `git clean` when available for force cleanup
- **Project Detection**: Automatically detects pgv_faiss project structure
- **Summary Reports**: Shows cleanup summary with file counts and sizes

## Prerequisites

### For Database Cleanup:
- PostgreSQL server running
- Database 'vectordb' configured with proper credentials
- pgv_faiss db_cleanup tool compiled

### For Build Cleanup:
- Running from pgv_faiss project directory
- Standard Unix tools (find, rm, du)
- Optional: Git repository for force cleanup

## Installation

1. Make scripts executable:
```bash
chmod +x scripts/cleanup_db.sh
chmod +x scripts/clean_build.sh
```

2. Run from project root or scripts directory:
```bash
# From project root
./scripts/clean_build.sh all

# From scripts directory
cd scripts
./clean_build.sh all
```

## Development Workflow

### Version Control Integration:
The project includes a comprehensive [`.gitignore`](../.gitignore) that automatically excludes:
- Build artifacts (build/, *.so, *.exe)
- Temporary files (*.log, *.tmp, CMakeCache.txt)
- Editor files (*.swp, *~, .vscode/)
- Test results (*benchmark_results*.csv)

### Before Building:
```bash
./scripts/clean_build.sh all
mkdir build && cd build
cmake .. && make
```

### Before Benchmarking:
```bash
./scripts/cleanup_db.sh all      # Clean database
./scripts/clean_build.sh results # Clean old results
```

### Regular Maintenance:
```bash
./scripts/clean_build.sh temp    # Clean temporary files
./scripts/cleanup_db.sh stats    # Check database status
```

## Error Handling

Both scripts provide comprehensive error handling:

- **Permission Issues**: Clear messages about file permissions
- **Missing Dependencies**: Checks for required tools and databases
- **Invalid Commands**: Helpful error messages with usage examples
- **Safety Confirmations**: Prompts for destructive operations (force cleanup)

## Output Examples

### Successful Cleanup:
```
=== pgv_faiss Build Cleanup Script ===

Cleaning build directory...
Files to be removed:
total 2.3M
-rwxrwxrwx 1 user user 101496 Aug 24 23:27 advanced_example
-rwxrwxrwx 1 user user  72408 Aug 24 23:27 basic_example
... and 15 more files

✅ Build directory cleaned
✅ Temporary files cleaned
✅ Result files cleaned

=== Cleanup Summary ===
Build directory size: 0B
Remaining temporary files: 0
Remaining result files: 0
Remaining backup files: 0
✅ Project is clean!

✅ Cleanup completed successfully!
```

### Database Status:
```
=== pgv_faiss Database Cleanup Tool ===

Connected to database successfully!

=== Before Cleanup ===
Total tables: 5
Database size: 2.1 MB
Remaining tables:
  - test_vectors
  - benchmark_results
  - sample_embeddings

✅ Database cleanup completed successfully!
```

## Troubleshooting

### Common Issues:

1. **Script not executable**: Run `chmod +x scripts/*.sh`
2. **Database connection failed**: Check PostgreSQL service and credentials
3. **Permission denied**: Ensure write permissions on build directory
4. **Git not found**: Force cleanup falls back to manual cleanup

### Getting Help:

```bash
./scripts/clean_build.sh help
./scripts/cleanup_db.sh help
```

---

*These scripts are part of the pgv_faiss project development toolkit. For more information, see the main [manual.md](../manual.md) file.*