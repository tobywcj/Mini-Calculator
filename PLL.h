#ifndef PLL_H
#define PLL_H

void SysTick_Init(void);
void PLL_Init(unsigned long desired_freq);
void SysTick_Wait(unsigned long delay);
void delay_us(unsigned long delay);

#endif