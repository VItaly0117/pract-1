#ifndef SHA256_H
#define SHA256_H

#include <stddef.h>
#include <stdint.h>

#define SHA256_SIZE 32

void sha256(const uint8_t *data, size_t length, uint8_t digest[SHA256_SIZE]);

#endif
