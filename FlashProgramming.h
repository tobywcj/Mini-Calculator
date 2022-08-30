#ifndef FLASHPROGRAMMING_H
#define FLASHPROGRAMMING_H
#include <stdint.h>

void Flash_init(void);
int Flash_erase(int block_counter);
int Flash_write(const void* data, int target_register);
uint32_t Flash_read(void* data, int target_register);

#endif