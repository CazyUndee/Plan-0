#include "kstring.h"

int k_strlen(const char* s) {
    int n = 0;
    while (*s++) n++;
    return n;
}

int k_strcmp(const char* a, const char* b) {
    while (*a && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

int k_strncmp(const char* a, const char* b, int n) {
    while (n > 0 && *a && *a == *b) { a++; b++; n--; }
    if (n == 0) return 0;
    return (unsigned char)*a - (unsigned char)*b;
}

void k_memcpy(void* dst, const void* src, size_t n) {
    for (size_t i = 0; i < n; i++)
        ((unsigned char*)dst)[i] = ((const unsigned char*)src)[i];
}

void k_strcpy(char* dst, const char* src) {
    while (*src) *dst++ = *src++;
    *dst = 0;
}
