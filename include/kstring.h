#ifndef KSTRING_H
#define KSTRING_H

#include <stddef.h>

int k_strlen(const char* s);
int k_strcmp(const char* a, const char* b);
int k_strncmp(const char* a, const char* b, int n);
void k_memcpy(void* dst, const void* src, size_t n);
void k_strcpy(char* dst, const char* src);

#endif
