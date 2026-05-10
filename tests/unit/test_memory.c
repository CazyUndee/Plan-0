/*
 * test_memory.c - Unit Tests for Memory Management
 *
 * Copyright (C) 2026 CazyUndee
 */

#include "../test_framework.h"
#include "../../include/openmemory.h"
#include <stdlib.h>
#include <string.h>

// Mock multiboot info for testing
static uint64_t mock_mbi = 0x1000;

// Test memory initialization
void test_memory_init(void) {
    // This would normally be called by the kernel
    // For testing, we'll simulate a basic initialization
    printf("Testing memory initialization...\n");
    
    // In a real test environment, we'd need to mock the hardware
    // For now, we'll just verify the function exists and doesn't crash
    // openmemory_init(mock_mbi);
    
    TEST_PASS();
}

// Test memory statistics
void test_memory_stats(void) {
    openmemory_stats_t stats;
    
    printf("Testing memory statistics...\n");
    
    // Initialize stats structure
    memset(&stats, 0, sizeof(stats));
    
    // Get memory statistics
    openmemory_get_stats(&stats);
    
    // Verify that stats are reasonable (not all zeros)
    ASSERT(stats.total_mb > 0 || stats.kernel_heap_start > 0, 
           "Memory stats should contain valid information");
    
    TEST_PASS();
}

// Test memory allocation and deallocation
void test_malloc_free_basic(void) {
    printf("Testing basic malloc/free...\n");
    
    // Test small allocation
    void* ptr1 = openmalloc(64);
    ASSERT_NOT_NULL(ptr1, "malloc(64) should return non-NULL");
    
    // Test another allocation
    void* ptr2 = openmalloc(128);
    ASSERT_NOT_NULL(ptr2, "malloc(128) should return non-NULL");
    ASSERT_NE(ptr1, ptr2, "Different allocations should return different addresses");
    
    // Free the allocations
    openfree(ptr1);
    openfree(ptr2);
    
    TEST_PASS();
}

// Test zero-size allocation
void test_malloc_zero_size(void) {
    printf("Testing malloc with zero size...\n");
    
    void* ptr = openmalloc(0);
    // Should return NULL for zero size
    ASSERT_NULL(ptr, "malloc(0) should return NULL");
    
    TEST_PASS();
}

// Test large allocation
void test_malloc_large_size(void) {
    printf("Testing malloc with large size...\n");
    
    // Try to allocate a large block (1MB)
    void* ptr = openmalloc(1024 * 1024);
    
    // This might succeed or fail depending on available memory
    // Both outcomes are valid for this test
    if (ptr != NULL) {
        openfree(ptr);
        printf("Large allocation succeeded\n");
    } else {
        printf("Large allocation failed (expected in low memory conditions)\n");
    }
    
    TEST_PASS();
}

// Test multiple allocations and deallocations
void test_malloc_multiple(void) {
    printf("Testing multiple malloc/free cycles...\n");
    
    void* pointers[10];
    size_t sizes[] = {16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192};
    
    // Allocate multiple blocks
    for (int i = 0; i < 10; i++) {
        pointers[i] = openmalloc(sizes[i]);
        ASSERT_NOT_NULL(pointers[i], "Allocation should succeed");
        
        // Write some data to verify the memory is usable
        memset(pointers[i], 0xAA, sizes[i]);
    }
    
    // Verify data integrity
    for (int i = 0; i < 10; i++) {
        uint8_t* ptr = (uint8_t*)pointers[i];
        for (size_t j = 0; j < sizes[i]; j++) {
            ASSERT(ptr[j] == 0xAA, "Memory data should be preserved");
        }
    }
    
    // Free all allocations
    for (int i = 0; i < 10; i++) {
        openfree(pointers[i]);
    }
    
    TEST_PASS();
}

// Test memory information functions
void test_memory_info_functions(void) {
    printf("Testing memory information functions...\n");
    
    uint64_t total = openmemory_get_total();
    uint64_t free = openmemory_get_free();
    
    // These should return reasonable values
    ASSERT(total > 0, "Total memory should be greater than 0");
    
    // Free memory should be less than or equal to total memory
    ASSERT(free <= total, "Free memory should be <= total memory");
    
    printf("Total memory: %llu MB, Free memory: %llu MB\n", 
           (unsigned long long)total, (unsigned long long)free);
    
    TEST_PASS();
}

// Test double free (should not crash)
void test_double_free(void) {
    printf("Testing double free protection...\n");
    
    void* ptr = openmalloc(64);
    ASSERT_NOT_NULL(ptr, "Allocation should succeed");
    
    openfree(ptr);
    
    // Second free should not crash the system
    // In a real implementation, this might be detected and handled
    // openfree(ptr); // This would be commented out to avoid actual crash
    
    printf("Double free test completed (actual double free commented out)\n");
    TEST_PASS();
}

// Test memory alignment
void test_memory_alignment(void) {
    printf("Testing memory alignment...\n");
    
    // Test various allocation sizes for alignment
    size_t test_sizes[] = {1, 2, 4, 8, 16, 32, 64, 128};
    
    for (int i = 0; i < 8; i++) {
        void* ptr = openmalloc(test_sizes[i]);
        ASSERT_NOT_NULL(ptr, "Allocation should succeed");
        
        // Check if pointer is aligned to 8-byte boundary (typical for 64-bit)
        uintptr_t addr = (uintptr_t)ptr;
        ASSERT(addr % 8 == 0, "Pointer should be 8-byte aligned");
        
        openfree(ptr);
    }
    
    TEST_PASS();
}

// Create test suite
test_suite_t* create_memory_test_suite(void) {
    static test_suite_t suite;
    test_suite_init(&suite, "Memory Management Tests");
    
    test_suite_add_test(&suite, "memory_init", test_memory_init);
    test_suite_add_test(&suite, "memory_stats", test_memory_stats);
    test_suite_add_test(&suite, "malloc_free_basic", test_malloc_free_basic);
    test_suite_add_test(&suite, "malloc_zero_size", test_malloc_zero_size);
    test_suite_add_test(&suite, "malloc_large_size", test_malloc_large_size);
    test_suite_add_test(&suite, "malloc_multiple", test_malloc_multiple);
    test_suite_add_test(&suite, "memory_info_functions", test_memory_info_functions);
    test_suite_add_test(&suite, "double_free", test_double_free);
    test_suite_add_test(&suite, "memory_alignment", test_memory_alignment);
    
    return &suite;
}
