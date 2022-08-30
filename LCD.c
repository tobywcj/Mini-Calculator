/* LCD file
two lines of 16 characters with 8x5 font size
*/
#include "LCD.h"

#define DB                      (*((volatile unsigned long *)0x4000503C)) // bit-specific addressing PB3-0
#define GPIO_PORTB_DATA_R       (*((volatile unsigned long *)0x400053FC))
#define GPIO_PORTB_DIR_R        (*((volatile unsigned long *)0x40005400))
#define GPIO_PORTB_AFSEL_R      (*((volatile unsigned long *)0x40005420))
#define GPIO_PORTB_DEN_R        (*((volatile unsigned long *)0x4000551C))
#define GPIO_PORTB_LOCK_R       (*((volatile unsigned long *)0x40005520))
#define GPIO_PORTB_CR_R         (*((volatile unsigned long *)0x40005524))
#define GPIO_PORTB_AMSEL_R      (*((volatile unsigned long *)0x40005528))
#define GPIO_PORTB_PCTL_R       (*((volatile unsigned long *)0x4000552C))

#define RS                      (*((volatile unsigned long *)0x40004020)) // PA3
#define EN                      (*((volatile unsigned long *)0x40004010)) // PA2
#define GPIO_PORTA_DIR_R        (*((volatile unsigned long *)0x40004400))
#define GPIO_PORTA_AFSEL_R      (*((volatile unsigned long *)0x40004420))
#define GPIO_PORTA_DEN_R        (*((volatile unsigned long *)0x4000451C))
#define GPIO_PORTA_LOCK_R       (*((volatile unsigned long *)0x40004520))
#define GPIO_PORTA_CR_R         (*((volatile unsigned long *)0x40004524))
#define GPIO_PORTA_AMSEL_R      (*((volatile unsigned long *)0x40004528))
#define GPIO_PORTA_PCTL_R       (*((volatile unsigned long *)0x4000452C))
	
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))
	
// Instructions related Defines
#define clear_display         0x01
#define return_home           0x02
#define right_shift_cursor    0x14
#define left_shift_cursor     0x10
#define next_row              0xC0
#define right_shift_display   0x1C
#define left_shift_display    0x18

unsigned static char icon_1[] = {0x04,0x0E,0x15,0x11,0x11,0x11,0x0A,0x04};
unsigned static char icon_2[] = {0x04,0x0A,0x11,0x15,0x15,0x11,0x0A,0x04};
unsigned static char icon_3[] = {0x04,0x0A,0x11,0x11,0x11,0x15,0x0E,0x04};
unsigned static char arrow[] = 	{0x08,0x04,0x02,0x1F,0x02,0x04,0x08,0x00};
unsigned static char pass_icon1[] = {0x00,0x04,0x0E,0x1B,0x11,0x1B,0x0E,0x04};
unsigned static char pass_icon2[] = 	{0x1F,0x1B,0x11,0x04,0x0E,0x04,0x11,0x1B};

// Subroutine to initialize port B pins for selecting 4-bit mode to transfer data in two nibbles
// PA3-2 and PB3-0 are digital output pins to the LCD
// Notes: These 6 pins are connected to hardware on the LaunchPad
void LCD_Port_Init(void){ volatile unsigned long delay;
	
  SYSCTL_RCGC2_R |= 0x00000002;      // 1) B clock
  delay = SYSCTL_RCGC2_R;            // delay   
	GPIO_PORTB_CR_R |= 0x0F;           // 2) allow changes to PB3-0  
  GPIO_PORTB_AMSEL_R &= 0x00;        // 2) disable analog function
  GPIO_PORTB_PCTL_R &= 0x00000000;   // 3) GPIO clear bit PCTL  
  GPIO_PORTB_DIR_R |= 0x0F;          // 4) PB3-0 are the outputs
  GPIO_PORTB_AFSEL_R &= 0x00;        // 5) no alternate function   
  GPIO_PORTB_DEN_R |= 0x0F;          // 6) enable digital pins PB3-0

  SYSCTL_RCGC2_R |= 0x00000001;      // 1) A clock
  delay = SYSCTL_RCGC2_R;            // delay       
	GPIO_PORTA_CR_R |= 0x0C;           // 2) allow changes to PA3-2 
  GPIO_PORTA_AMSEL_R &= 0x00;        // 2) disable analog function
  GPIO_PORTA_PCTL_R &= 0x00000000;   // 3) GPIO clear bit PCTL  
  GPIO_PORTA_DIR_R |= 0x0C;          // 4) PA3-2 are the outputs
  GPIO_PORTA_AFSEL_R &= 0x00;        // 5) no alternate function   
  GPIO_PORTA_DEN_R |= 0x0C;          // 6) enable digital pins PA3-2
	
}

void SendDisplayNibble(unsigned char nibble, unsigned char control) {
	RS = control;
	SysTick_Wait(4); //address set-up time
	DB = nibble;
	EN = 0x04; 
	SysTick_Wait(20); // at least 450ns for one cycle
	EN = 0x00;
	SysTick_Wait(20);
}

// control 1/0 = data/instruction
void SendDisplayByte(unsigned char byte, unsigned char control) {
	SendDisplayNibble(((byte&0xF0) >> 4), control);
	SendDisplayNibble((byte&0x0F), control);
	delay_us(400); // at least 37 us
}


void InitDisplayPort(void) {

	delay_us(40000);// Wait for more than 15 ms
	SendDisplayNibble(0x03, 0x00);
	delay_us(4100); // Wait for more than 4.1 ms
	SendDisplayNibble(0x03, 0x00);
	delay_us(100); // Wait for more than 100 µs
	SendDisplayNibble(0x03, 0x00);
	delay_us(37); // Wait for more than 37 us
	SendDisplayNibble(0x02, 0x00);
	
	SendDisplayByte(0x28, 0x00); // 4-bit data (DL = 0), 2 display lines (N = 1) and a 5x8 font (F = 0)
	SendDisplayByte(0x08, 0x00); // display off
	SendDisplayByte(0x01, 0x00); // display clear
	SendDisplayByte(0x06, 0x00); // entry mode: cursor increments, no display shift
  InitLCD();
}

void next_line(void) {
	SendDisplayByte(next_row, 0x00);
}

void SendDisplayString(char *string) {
	while(*string){
		SendDisplayByte(*string, 0x08);
		string++;
	}
}

void ClearDisplayScreen(void) {
	SendDisplayByte(clear_display, 0x00);
	SendDisplayByte(return_home, 0x00);
	SendDisplayByte(right_shift_cursor, 0x00);
  SendDisplayByte(right_shift_cursor, 0x00);
}

void InitLCD(void) {
	SendDisplayByte(0x28, 0x00);
	SendDisplayByte(0x06, 0x00);
	SendDisplayByte(0x0F, 0x00); //sets display on, cursor on, blink on
}

void Entry_graphic(void) {
	/*ClearDisplayScreen();
	SendDisplayString("Bingo "); SendDisplayString("!(^") ;
	SendDisplayByte(0x6F, 0x08); SendDisplayString("^)!") ;
	next_line();
	SendDisplayString("Correct PW !");
	delay_us(1000000);
	for (int i = 0; i < 3; i++) {
		ClearDisplayScreen();
	  SendDisplayString("   Welcome !!");
	  next_line();
		SendDisplayString("loading ");
		for (int l = 0; l < 3; l++) {
		  SendDisplayByte('.', 0x08);
		  delay_us(150000);
		  SendDisplayByte(0xA5, 0x08);
		  delay_us(60000);
		}
	}
	next_line();
  SendDisplayString_OneByOne("Let's get start!");
	delay_us(300000);*/
}

void SendDisplayString_OneByOne(char *string) {
	while(*string){
		SendDisplayByte(*string, 0x08);
		delay_us(100000);
		string++;
	}
	SendDisplayByte(left_shift_cursor, 0x00);
}

