/* Keypad file */
#include "keypad.h"

// define Port E for digital input pins in the rows
#define GPIO_PORTE_DATA_R       (*((volatile unsigned long *)0x400243FC))
#define GPIO_PORTE_LOCK_R       (*((volatile unsigned long *)0x40024520))
#define GPIO_PORTE_CR_R         (*((volatile unsigned long *)0x40024524))
#define GPIO_PORTE_DIR_R        (*((volatile unsigned long *)0x40024400))
#define GPIO_PORTE_AFSEL_R      (*((volatile unsigned long *)0x40024420))
#define GPIO_PORTE_DEN_R        (*((volatile unsigned long *)0x4002451C))
#define GPIO_PORTE_AMSEL_R      (*((volatile unsigned long *)0x40024528))
#define GPIO_PORTE_PCTL_R       (*((volatile unsigned long *)0x4002452C))
#define GPIO_PORTE_PDR_R        (*((volatile unsigned long *)0x40024514))
	
// define Port D for digital output pins in the columns
#define GPIO_PORTD_DATA_R       (*((volatile unsigned long *)0x400073FC))
#define GPIO_PORTD_LOCK_R       (*((volatile unsigned long *)0x40007520))
#define GPIO_PORTD_CR_R         (*((volatile unsigned long *)0x40007524))
#define GPIO_PORTD_DIR_R        (*((volatile unsigned long *)0x40007400))
#define GPIO_PORTD_AFSEL_R      (*((volatile unsigned long *)0x40007420))
#define GPIO_PORTD_DEN_R        (*((volatile unsigned long *)0x4000751C))
#define GPIO_PORTD_AMSEL_R      (*((volatile unsigned long *)0x40007528))
#define GPIO_PORTD_PCTL_R       (*((volatile unsigned long *)0x4000752C))
	
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))
	
// define all the rows in Port E individually
#define PE0      (*((volatile unsigned long *)0x40024004))
#define PE1      (*((volatile unsigned long *)0x40024008))
#define PE2      (*((volatile unsigned long *)0x40024010))
#define PE3      (*((volatile unsigned long *)0x40024020))
// define all the columns in Port D individually
#define PD0      (*((volatile unsigned long *)0x40007004))
#define PD1      (*((volatile unsigned long *)0x40007008))
#define PD2      (*((volatile unsigned long *)0x40007010))
#define PD3      (*((volatile unsigned long *)0x40007020))

// extern = can be accessed through all files
// static = can only be accessed in local file
	
// Subroutine to initialize port D and port E pins to connect with rows and columns of keypad, as digital output and digital input pins respectively
// PE3-0 are digital input pins to the keypad
// PD3-0 are digital output pins to the keypad
// Notes: These 8 pins are connected to hardware on the LaunchPad
void keypad_init(void){ volatile unsigned long delay;
  // rows
	SYSCTL_RCGC2_R |= 0x00000010;                    // 1) E clock
  delay = SYSCTL_RCGC2_R;                          // delay     
  GPIO_PORTE_LOCK_R = 0x4C4F434B;                  // 2) unlock port E
	GPIO_PORTE_CR_R |= 0x0F;                         //    allow changes to PE3-0
  GPIO_PORTE_AMSEL_R &= 0x00;                      // 3) disable analog function
  GPIO_PORTE_PCTL_R &= 0x00000000;                 // 4) GPIO clear bit PCTL  
  GPIO_PORTE_DIR_R &= ~(unsigned long)(0x0F);      // 5) PE3-0 are the inputs
  GPIO_PORTE_AFSEL_R &= 0x00;                      // 6) no alternate function   
  GPIO_PORTE_DEN_R |= 0x0F;                        // 7) enable digital pins PE3-0
  GPIO_PORTE_PDR_R |= 0x0F;                        // 8) enable pull-down resistors on PE3-0 
 // columns
  SYSCTL_RCGC2_R |= 0x00000008;      // 1) D clock 
  delay = SYSCTL_RCGC2_R;            // delay       
	GPIO_PORTD_LOCK_R = 0x4C4F434B;    // 2) unlock port D
	GPIO_PORTD_CR_R |= 0x0F;           //    allow changes to PD3-0
  GPIO_PORTD_AMSEL_R &= 0x00;        // 3) disable analog function
  GPIO_PORTD_PCTL_R &= 0x00000000;   // 4) GPIO clear bit PCTL  
  GPIO_PORTD_DIR_R |= 0x0F;          // 5) PD3-0 are the outputs
  GPIO_PORTD_AFSEL_R &= 0x00;        // 6) no alternate function   
  GPIO_PORTD_DEN_R |= 0x0F;          // 7) enable digital pins PD3-0
}

char keypad_KeyPressed(int shift_flag) {
	
	if (shift_flag == 0) {
	  PD0 = 0x01; // enable first column, check the state of the corresponding row
	  PD1 = 0x00;
	  PD2 = 0x00;
	  PD3 = 0x00;
	  SysTick_Wait(10);
	  if (PE0 == 0x01) {return '1';} // Read 1st row
	  else if (PE1 == 0x02) {return '4';} // Read 2nd row
	  else if (PE2 == 0x04) {return '7';} // Read 3rd row
	  else if (PE3 == 0x08) {return 'R';} // Read 4th row, R = Rubout last character
	  
	  PD0 = 0x00; // enable second column, check the state of the corresponding row
	  PD1 = 0x02;
	  PD2 = 0x00;
	  PD3 = 0x00;
	  SysTick_Wait(10);
	  if (PE0 == 0x01) {return '2';}
	  else if (PE1 == 0x02) {return '5';}
	  else if (PE2 == 0x04) {return '8';} 
	  else if (PE3 == 0x08) {return '0';}
	  
	  PD0 = 0x00; // enable third column, check the state of the corresponding row
	  PD1 = 0x00;
	  PD2 = 0x04;
	  PD3 = 0x00;
	  SysTick_Wait(10);
	  if (PE0 == 0x01) {return '3';}
	  else if (PE1 == 0x02) {return '6';}
	  else if (PE2 == 0x04) {return '9';}
	  else if (PE3 == 0x08) {return '=';}
	  
	  PD0 = 0x00; // enable fourth column, check the state of the corresponding row
	  PD1 = 0x00;
	  PD2 = 0x00;
	  PD3 = 0x08;
	  SysTick_Wait(10);
	  if (PE0 == 0x01) {return '+';}
	  else if (PE1 == 0x02) {return '-';}
	  else if (PE2 == 0x04) {return '.';}
	  else if (PE3 == 0x08) {return 'S';} // S = shift
	  else{return 'X';} // nothing pressed
  }
	
		if (shift_flag == 1) {
	  PD0 = 0x01; // enable first column, check the state of the corresponding row
	  PD1 = 0x00;
	  PD2 = 0x00;
	  PD3 = 0x00;
	  SysTick_Wait(10);
	  if (PE0 == 0x01) {return 'A';} // cosine function
	  else if (PE1 == 0x02) {return 'l';}  // logarithm with base 10 of the input
	  else if (PE2 == 0x04) {return 'e';}
	  else if (PE3 == 0x08) {return 'D';} // D = Delete entire entry
	  
	  PD0 = 0x00; // enable second column, check the state of the corresponding row
	  PD1 = 0x02;
	  PD2 = 0x00;
	  PD3 = 0x00;
	  SysTick_Wait(10);
	  if (PE0 == 0x01) {return 'C';} // sine function
	  else if (PE1 == 0x02) {return 'r';} // empty
	  else if (PE2 == 0x04) {return 'e';}  // empty
	  else if (PE3 == 0x08) {return 'e';} // empty
	  
	  PD0 = 0x00; // enable third column, check the state of the corresponding row
	  PD1 = 0x00;
	  PD2 = 0x04;
	  PD3 = 0x00;
	  SysTick_Wait(10);
	  if (PE0 == 0x01) {return 'F';} // tangent function
	  else if (PE1 == 0x02) {return 'e';} // empty
	  else if (PE2 == 0x04) {return 'e';} // empty
	  else if (PE3 == 0x08) {return 'e';} // empty
	  
	  PD0 = 0x00; // enable fourth column, check the state of the corresponding row
	  PD1 = 0x00;
	  PD2 = 0x00;
	  PD3 = 0x08;
	  SysTick_Wait(10);
	  if (PE0 == 0x01) {return 'x';} // multiplication
	  else if (PE1 == 0x02) {return 0xFD;} // division
	  else if (PE2 == 0x04) {return 'E';} // Times ten to the power
	  else if (PE3 == 0x08) {return 'S';} // shift
	  else{return 'X';} // nothing pressed
  }
}

	
