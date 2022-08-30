#ifndef EEPROM_H
#define EEPROM_H
#include "PLL.h"

void eepromInit(void);
void eeSetPass(int temp_password);
unsigned long eeGetPass(void);

#endif