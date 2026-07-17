# Plan 0 Test Suite

This directory contains the comprehensive test suite for the Plan 0 operating system kernel.

## Directory Structure

```
tests/
â”œâ”€â”€ README.md                 # This file
â”œâ”€â”€ Makefile                 # Test suite build system
â”œâ”€â”€ test_framework.h         # Test framework header
â”œâ”€â”€ test_framework.c         # Test framework implementation
â”œâ”€â”€ test_runner.c           # Main test runner
â”œâ”€â”€ unit/                   # Unit tests
â”‚   â”œâ”€â”€ test_memory.c       # Memory management tests
â”‚   â”œâ”€â”€ test_filesystem.c   # Filesystem tests
â”‚   â””â”€â”€ test_process.c      # Process management tests
â”œâ”€â”€ integration/            # Integration tests
â”‚   â””â”€â”€ test_kernel_integration.c  # Kernel component integration tests
â””â”€â”€ mocks/                  # Mock hardware drivers
    â”œâ”€â”€ mock_hardware.h     # Mock hardware interface
    â””â”€â”€ mock_hardware.c     # Mock hardware implementation
```

## Test Categories

### Unit Tests

Unit tests test individual components in isolation:

- **Memory Management Tests** (`test_memory.c`)
  - Memory allocation and deallocation
  - Memory statistics and information
  - Memory alignment and boundary checks
  - Error handling for invalid operations

- **Filesystem Tests** (`test_filesystem.c`)
  - Filesystem structure validation
  - Boot sector and MFT entry handling
  - Attribute and flag validation
  - File name and metadata handling

- **Process Management Tests** (`test_process.c`)
  - Process control block structure
  - Process state transitions
  - CPU context management
  - Process scheduling and timing

### Integration Tests

Integration tests test multiple components working together:

- **Kernel Integration Tests** (`test_kernel_integration.c`)
  - Memory and process management integration
  - Filesystem and memory integration
  - Component lifecycle simulation
  - Memory pressure handling

### Mock Hardware

Mock hardware drivers provide a controlled environment for testing:

- **Mock Disk Driver** - Simulates disk I/O operations
- **Mock VGA Display** - Simulates text output
- **Mock Timer** - Simulates system timer
- **Mock PIC** - Simulates interrupt controller
- **Mock Serial Port** - Simulates serial communication
- **Mock RTC** - Simulates real-time clock

## Building and Running Tests

### Prerequisites

- GCC compiler
- Make utility
- Standard C library

### Build Commands

From the root directory:

```bash
# Build all tests
make test

# Build only unit tests
make test-unit

# Build only integration tests
make test-integration

# Build with verbose output
make test-all

# Clean test build artifacts
make test-clean

# Clean all build artifacts (including tests)
make clean-all
```

From the `tests/` directory:

```bash
# Build test runner
make

# Run all tests
make test

# Run unit tests only
make unit

# Run integration tests only
make integration

# Run specific test suites
make test-memory
make test-filesystem
make test-process
make test-integration

# Run with verbose output
make test-verbose

# Run with quiet output
make test-quiet

# List available test suites
make list-tests

# Clean build artifacts
make clean
```

### Running Tests

#### Using the Test Runner Directly

```bash
# Run all tests
./bin/test_runner

# Run specific test suite
./bin/test_runner --suite "Memory Management"
./bin/test_runner --suite "Filesystem"
./bin/test_runner --suite "Process Management"
./bin/test_runner --suite "Kernel Integration"

# Verbose output
./bin/test_runner --verbose

# Quiet output
./bin/test_runner --quiet

# List available test suites
./bin/test_runner --list

# Show help
./bin/test_runner --help
```

#### Test Output Format

```
=== Running Test Suite: Memory Management ===
Running: memory_init
Testing memory initialization...
PASS: memory_init

Running: memory_stats
Testing memory statistics...
PASS: memory_stats

...

=== Test Suite Summary: Memory Management ===
Total: 9, Passed: 9, Failed: 0
âœ“ All tests passed!

========================================
Overall Test Results
========================================
Test suites run: 4
Total tests: 47
Passed: 47
Failed: 0
âœ“ All tests passed!
```

## Writing New Tests

### Test Framework API

The test framework provides simple assertion macros:

```c
// Basic assertions
ASSERT(condition, message)
ASSERT_EQ(expected, actual, message)
ASSERT_NE(expected, actual, message)
ASSERT_NULL(ptr, message)
ASSERT_NOT_NULL(ptr, message)

// Test completion
TEST_PASS()
```

### Adding a New Test

1. Create a new test function:

```c
void test_new_feature(void) {
    printf("Testing new feature...\n");
    
    // Your test code here
    int result = some_function();
    
    ASSERT_EQ(expected_result, result, "Function should return expected value");
    TEST_PASS();
}
```

2. Add the test to a test suite:

```c
test_suite_t* create_your_test_suite(void) {
    static test_suite_t suite;
    test_suite_init(&suite, "Your Test Suite");
    
    test_suite_add_test(&suite, "test_name", test_function);
    
    return &suite;
}
```

3. Register the test suite in `test_runner.c`:

```c
extern test_suite_t* create_your_test_suite(void);

static test_suite_info_t test_suites[] = {
    // ... existing suites ...
    {"Your Test Suite", create_your_test_suite, 1},
};
```

### Test Naming Conventions

- Test files: `test_<module>.c`
- Test functions: `test_<feature>()`
- Test suite names: `<Module> Tests`

## Mock Hardware Usage

Mock hardware drivers allow testing without actual hardware:

```c
#include "mocks/mock_hardware.h"

void test_with_mock_disk(void) {
    // Create mock disk
    mock_disk_t* disk = mock_disk_create(1024*1024, 512);
    ASSERT_NOT_NULL(disk, "Mock disk should be created");
    
    // Test disk operations
    char buffer[512];
    int result = mock_disk_read(disk, 0, 1, buffer);
    ASSERT_EQ(0, result, "Disk read should succeed");
    
    // Clean up
    mock_disk_destroy(disk);
    TEST_PASS();
}
```

## Continuous Integration

The test suite is designed to work with CI/CD pipelines:

```bash
# CI-friendly commands
make test-quiet && echo "TESTS_PASSED=true" || echo "TESTS_PASSED=false"
```

## Coverage

The test suite currently covers:

- **Memory Management**: Allocation, deallocation, statistics, error handling
- **Filesystem**: Structure validation, metadata handling, constants
- **Process Management**: PCB structure, state management, scheduling
- **Integration**: Component interaction, lifecycle, memory pressure

## Future Enhancements

Planned additions to the test suite:

- Performance benchmarks
- Stress testing
- Memory leak detection
- Code coverage analysis
- Automated test generation
- Hardware-in-the-loop testing

## Troubleshooting

### Common Issues

1. **Build Failures**: Ensure GCC and Make are installed and in PATH
2. **Link Errors**: Check that all required headers are included
3. **Test Failures**: Examine test output for specific failure messages

### Debugging Tests

Use verbose output to see detailed test execution:

```bash
make test-verbose
# or
./bin/test_runner --verbose
```

### Adding Debug Output

Add printf statements in test functions for debugging:

```c
void test_debug_example(void) {
    printf("DEBUG: Starting test...\n");
    
    int result = some_function();
    printf("DEBUG: Function returned: %d\n", result);
    
    ASSERT_EQ(expected, result, "Function should return expected value");
    TEST_PASS();
}
```

## Contributing

When adding new tests:

1. Follow the existing code style
2. Add comprehensive assertions
3. Test both success and failure cases
4. Update this README if adding new test categories
5. Ensure all tests pass before submitting

## License

This test suite is licensed under the same terms as the Plan 0 project (GNU Affero General Public License v3.0).
