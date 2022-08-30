#ifndef LCD_H
#define LCD_H
#include "PLL.h"

// LCD Function Prototypes
void LCD_Port_Init(void);
void InitDisplayPort(void);                                          // LCD initialization
void SendDisplayNibble(unsigned char nibble, unsigned char control); // send 4-bit data/instruction to LCD
void SendDisplayByte(unsigned char byte, unsigned char control);     // send 8-bit data to LCD
void ClearDisplayScreen(void);                                       // clear display
void SendDisplayString(char *string);                                // print a string of characters on the LCD
void InitLCD(void);
void next_line(void);
void Entry_graphic(void);
void SendDisplayString_OneByOne(char *string);

#endif