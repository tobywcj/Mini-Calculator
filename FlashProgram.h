#ifndef FLASHPROGRAM_H
#define FLASHPROGRAM_H
#include <stdint.h>

void Flash_Init(uint8_t systemClockFreqMHz);
int Flash_Write(uint32_t addr, uint32_t data);
int Flash_WriteArray(uint32_t *source, uint32_t addr, uint16_t count);
int Flash_FastWrite(uint32_t *source, uint32_t addr, uint16_t count);
int Flash_Erase(uint32_t addr);



#endif