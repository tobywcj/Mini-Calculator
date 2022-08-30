// ***** Documentation Section *****
//   ELEC 3662 Embedded System Project
//   Calculator
//   Runs on TM4C123
//   Chen Jing Wong, TOBY
//   January 15, 2022
//   please reference before use

// main.c
#include "TExaS.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>   // standard C library 

#include "FlashProgramming.h"
#include "PLL.h"
#include "LCD.h"
#include "keypad.h"

#define SYSCTL_RCGC2_R        (*((volatile unsigned long *)0x400FE108))
	
// PLL related defines
#define SYSCTL_RIS_R          (*((volatile unsigned long *)0x400FE050))	
#define SYSCTL_RCC_R          (*((volatile unsigned long *)0x400FE060))
#define SYSCTL_RCC2_R         (*((volatile unsigned long *)0x400FE070))	

// SysTick related defines	
#define NVIC_ST_CTRL_R        (*((volatile unsigned long *)0xE000E010))
#define NVIC_ST_RELOAD_R      (*((volatile unsigned long *)0xE000E014))
#define NVIC_ST_CURRENT_R     (*((volatile unsigned long *)0xE000E018))
	
// LED
#define GPIO_PORTF_DATA_R       (*((volatile unsigned long *)0x400253FC))
#define GPIO_PORTF_DIR_R        (*((volatile unsigned long *)0x40025400))
#define GPIO_PORTF_AFSEL_R      (*((volatile unsigned long *)0x40025420))
#define GPIO_PORTF_PUR_R        (*((volatile unsigned long *)0x40025510))
#define GPIO_PORTF_DEN_R        (*((volatile unsigned long *)0x4002551C))
#define GPIO_PORTF_LOCK_R       (*((volatile unsigned long *)0x40025520))
#define GPIO_PORTF_CR_R         (*((volatile unsigned long *)0x40025524))
#define GPIO_PORTF_AMSEL_R      (*((volatile unsigned long *)0x40025528))
#define GPIO_PORTF_PCTL_R       (*((volatile unsigned long *)0x4002552C))
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))
	
// Instructions related Defines
#define clear_display         0x01
#define return_home           0x02
#define right_shift_cursor    0x14
#define left_shift_cursor     0x10
#define next_row              0xC0
#define right_shift_display   0x1C
#define left_shift_display    0x18

// math constant
#define PI 3.14159

// password encryption
void password_encryption(void);
void change_password(void);
char password_entry_buffer[10];
int password_entry = 0;
int password_user_entered = 0; // password "7777" is stored in Flash memory of the MCU
int password_stored = 7777;
int change_password_flag = 0;

/////   Global Variables /////
char first_read, second_read;
int CursorAddress = 0;
char answer_buffer[10]; // buffer for storing the final answer
char equation_buffer[15]; // buffer for storing the equation of calculation
float final_answer = 0, first_number = 0, second_number = 0, third_number = 0;
float first_part = 0;
char key;
int operator_position[3]; // '+' = 1, '-' = 2, 'x' = 3, 0xFD(division sign) = 4

// Flags
int shift_flag = 0;
int decimal_flag = 0;
int continue_calculation_flag = 0;
int repressed_key_flag = 0;
int negative_flag = 0;
int FirstNumberNegative_flag = 0;
int DisplayShift_flag = 0;
int delete_ContinueCalculation_flag = 0;
int reboot_flag = 0;

// Counters
float decimal_counter = 1.0;
int number_counter = 0;
int number_prioritized_operands = 0;
int syntax_error_counter = 0;
int decimal_syntax_error_counter = 0;
int decimal_digit_counter = 0;

//// shifted-key math functions ////

// cosine function prototype and its variables
void output_cosine_answer(void);
void delete_entire_cosine_function(int cosine_decimal_digit_counter, int cosine_decimal_flag);
int cosine_flag = 0;
int cosine_syntax_error_counter = 0;
int max_op_cosine_flag = 0;

// sine function prototype and its variables
void output_sine_answer(void);
void delete_entire_sine_function(int sine_decimal_digit_counter, int sine_decimal_flag);
int sine_flag = 0;
int sine_syntax_error_counter = 0;
int max_op_sine_flag = 0;

// tangent function prototype and its variables
void output_tan_answer(void);
void delete_entire_tan_function(int tan_decimal_digit_counter, int tan_decimal_flag);
int tan_flag = 0;
int tan_syntax_error_counter = 0;
int max_op_tan_flag = 0;

// Times ten to the power function prototype and its variables
float PowNumber(char PowNum_key);
void output_TenPow_answer(void);
void delete_entire_tenpow_function(int tenpow_decimal_digit_counter, int tenpow_decimal_flag);
float PowNum = 0.0;
int TenPow_flag = 0;
int delete_TenPow_flag = 0;

// log10 function prototype and its variables
void output_log10_answer(void);
void delete_entire_log10_function(int log10_decimal_digit_counter, int log10_decimal_flag);
int log10_flag = 0;
int log10_syntax_error_counter = 0;
int max_op_log10_flag = 0;

// sqrt function prototype and its variables
/*void output_sqrt_answer(void);
void delete_entire_sqrt_function(int sqrt_decimal_digit_counter, int sqrt_decimal_flag);
int sqrt_flag = 0;
int sqrt_syntax_error_counter = 0;
int max_op_sqrt_flag = 0;*/

// LED
void PortF_Init(void);

// Calculation Function Prototypes
float FirstNumber(char key);
float SecondNumber(char key);
float ThirdNumber(char key);
void recalculation(void);
int update_CursorAddress(int n);
void delete_last_printed_entry(void);
float RemoveLastDigit_AnyNumber(float input, int decimal_digit_counter);
int Count_AnyNumber_digit (float input, int count_decimal_digit_counter, int count_decimal_syntax_error_counter);
void rubout_last_entry (void);

int main(void){
	
	///////////////// system frequency, ports, LCD and keypad initializations ////////////////////////////////////
  PLL_Init(80000000);	
	SysTick_Init();
	LCD_Port_Init();
	PortF_Init(); // LED, PF3,2,1,0 output  
	keypad_init();
	Flash_init();
	InitDisplayPort();
	
	///////////////////////////////// PASSWORD ENCRYPTION ////////////////////////////////////////////////////////
	
	while (1) {
	  SendDisplayString("ENTER PASSWORD:"); next_line();
	
	while (password_entry == 0) { // cannot access to the calculator until the correct pw has been entered
		password_encryption();
	}
	
	ClearDisplayScreen();
	
	///////////////////////////////// START CALCULATION ///////////////////////////////////////////////////////////
	
	while (password_entry == 1) {
		
		first_read = keypad_KeyPressed(shift_flag);
		SysTick_Wait(10000000); // delay to avoid collision of characters pressed popped up at the same time
		second_read = keypad_KeyPressed(shift_flag); // avoid continuously writing the same character to the LCD when holding the key
	
		 if (keypad_KeyPressed(shift_flag) != 'X' && (first_read != second_read)) {
			 
			 key = keypad_KeyPressed(shift_flag);
			 shift_flag = 0; // shift mode will only be valid for ONE time after the shift key is pressed 
			 GPIO_PORTF_DATA_R = 0x08;
			 
			 if ((key == 'S') && (reboot_flag == 1)) { reboot_flag = 1; } // for reboot the calculator to enter password
			 else if (key != 'D') { reboot_flag = 0; }
			 
			 if (key != 'S') {
				 GPIO_PORTF_DATA_R = 0x0A;
			   delay_us(80000);
				 GPIO_PORTF_DATA_R = 0x08;
				 if (key == 'R') { // rubout last character
				 } else if (key == 'A') { // cosine
					 CursorAddress = CursorAddress + 3;
					 cosine_syntax_error_counter++;
				 } else if (key == 'C') { // sine
					 CursorAddress = CursorAddress + 3;
					 sine_syntax_error_counter++;
				 } else if (key == 'F') { // tangent
					 CursorAddress = CursorAddress + 3;
					 tan_syntax_error_counter++;
				 } else if (key == 'l') { // log10
					 CursorAddress = CursorAddress + 3;
					 log10_syntax_error_counter++;
				 /*} else if (key == 'r') { // sqrt
					 CursorAddress = CursorAddress + 4;
					 sqrt_syntax_error_counter++;*/
				 } else { 
				   CursorAddress++;  // increment the cursor address everytime a character is displayed 
				 }   
			   if (CursorAddress >= 16) { // exceed the display position of the first row
			  	 SendDisplayByte(left_shift_display, 0x00); // display shift to continue the writing key on the first row
					 DisplayShift_flag++;
			   }
			 }
			 
			 // "Syntax ERROR" is printed to prevent the below cases of syntax error that calculator cannot operate
			 
			 if ((key == '+') || (key == '-') || (key == 'x') || (key == 0xFD)) {
				 syntax_error_counter++;
				 cosine_syntax_error_counter = 0;
				 sine_syntax_error_counter = 0;
				 tan_syntax_error_counter = 0;
				 delete_TenPow_flag++;
				 log10_syntax_error_counter = 0;
				 //sqrt_syntax_error_counter = 0;
			 }
			 
			 if (key == '=') { delete_TenPow_flag++; }
			 
			 if (syntax_error_counter == 1) {
				 if (key == '=') { syntax_error_counter++; } // to prevent the case of "A + B +="
				 if (key == 'E') { delete_TenPow_flag++; }  // to prevent the case of "1+E"
			 }
			 if ((syntax_error_counter == 2) || (cosine_syntax_error_counter == 2) || (sine_syntax_error_counter == 2) || (tan_syntax_error_counter == 2)
				 || (delete_TenPow_flag == 2) || (log10_syntax_error_counter == 2) /*|| (sqrt_syntax_error_counter == 2)*/) {
				 
				 if (syntax_error_counter == 2) { CursorAddress--; syntax_error_counter = 1; } // to prevent the cases of (1) two operands are printed in a row (2) "A + B +=" (3) "1.2.3"/"1..2"
				 if (cosine_syntax_error_counter == 2) { CursorAddress = CursorAddress - 3; cosine_syntax_error_counter = 1;}  // to prevent the cases of (1) "1+1cos(" (2) "cos(cos(" (3) 1+.cos(" (4) “cos(-” / “cos(*” /“cos(/” for all trigo functions
				 if (sine_syntax_error_counter == 2) { CursorAddress = CursorAddress - 3; sine_syntax_error_counter = 1;} 
				 if (tan_syntax_error_counter == 2) { CursorAddress = CursorAddress - 3; tan_syntax_error_counter = 1;}  
				 if (delete_TenPow_flag == 2) { CursorAddress--; delete_TenPow_flag = 1;} // to prevent the cases of (1) "1+10E+", "1+10E-", "1+10Ex", "1+10E/", "1+10E1.", "1+10E=" (2) "1+E" (4) "1+10EE" 
				 if (log10_syntax_error_counter == 2) { CursorAddress = CursorAddress - 3; log10_syntax_error_counter = 1;}  // to prevent the cases of (1) "1+1log" (2) "loglog" (3) 1+.log" (4) “log-” / “log*” /“log/”
				 //if (sqrt_syntax_error_counter == 2) { CursorAddress = CursorAddress - 4; sqrt_syntax_error_counter = 1;}  // to prevent the cases of (1) "1+1sqrt" (2) "sqrtsqrt" (3) 1+.sqrt" (4) “sqrt-” / “sqrt*” /“sqrt/”
				 
				 repressed_key_flag = 1;
				 ClearDisplayScreen(); // did not print, thus no need to delete last entry						 
				 SendDisplayString("Syntax ERROR"); // buffer for showing "Syntax ERROR"
				 delay_us(1000000);
			   ClearDisplayScreen();			
				 SendDisplayString(equation_buffer);
				 
			 } else { repressed_key_flag = 0; }
			 
			 
			 // store the previous answer into the first number to continue the calculation
			 
			 if (continue_calculation_flag == 1) {
				 
				 first_number = 0;
				 second_number = 0;
				 third_number = 0;
				 memset(equation_buffer, 0, sizeof(equation_buffer)); // clear equation buffer to print the previous answer to continue calculation
				 
				 if (key == '=') { // display screen remain unchanged as user keeps pressing equal sign
					 repressed_key_flag = 1;
				 }
				 else if ((key != '0') && (key != '1') && (key != '2') // only can type operators for math calculation, excluding '=' as well
					&& (key != '3') && (key != '4') && (key != '5')
				  && (key != '6') && (key != '7') && (key != '8')
				  && (key != '9') && (key != '.') && (key != 'E')
 				  && (key != 'l')) {
					
				   continue_calculation_flag = 0;
					 if (key == 'R') { // Rubout after pressed '=' is invalid, will just refresh equation
					   repressed_key_flag = 1;
					 } else { repressed_key_flag = 0; }
					 
				   first_number = final_answer;
				   sprintf(equation_buffer, "%0.4f", first_number);
				   ClearDisplayScreen();			
				   SendDisplayString(equation_buffer);
					 CursorAddress = 1;
					 CursorAddress = update_CursorAddress(first_number);
					 if ((number_counter == 0) && (negative_flag == 1)) { // the first number might be negative
						 first_number = first_number * -1;
					 }
						
				 } else {
						
					 repressed_key_flag = 1;
				 	 ClearDisplayScreen(); // did not print, thus no need to delete last entry		
				   SendDisplayString("Enter Operand"); // buffer for showing "Enter Operand"
					 delay_us(1000000);
					
					 ClearDisplayScreen();
					 memset(answer_buffer, 0, sizeof(answer_buffer));
           sprintf(answer_buffer, "%0.4f", final_answer);
			     SendDisplayString(answer_buffer);
					 
				  }
       }
		 
			 // performing calculation
			 
			 if (repressed_key_flag == 0) {
				 
			   if (key != '=') {
					 
				   switch (key){
				  	 
				  	 case '.':
					  	 decimal_flag = 1;
						   syntax_error_counter = 0;
						   decimal_syntax_error_counter++;
						   cosine_syntax_error_counter = 1;
						   sine_syntax_error_counter = 1;
						   tan_syntax_error_counter = 1;
						   delete_TenPow_flag = 0;
						   log10_syntax_error_counter = 1;
						   //sqrt_syntax_error_counter = 1;
						   
						   if ((decimal_syntax_error_counter == 2) || (TenPow_flag == 1)) { // to prevent the cases of (1) "1.2.3"/"1..2" (2) "1E2."
								 
								 CursorAddress--;
								 ClearDisplayScreen();						 
				         SendDisplayString("Syntax ERROR"); // buffer for showing "Syntax ERROR"
				         delay_us(1000000);
								 
								 ClearDisplayScreen();						 
				         SendDisplayString(equation_buffer);
							 }
				  	 break;
				  	 
					   case '0' ... '9':
							 
						   syntax_error_counter = 0;
						   delete_TenPow_flag = 0;
						 
						   cosine_syntax_error_counter = 1;
						   sine_syntax_error_counter = 1;
						   tan_syntax_error_counter = 1;
						   log10_syntax_error_counter = 1;
							 
					  	 // first number operation
						   if (number_counter == 0) {
								 if (TenPow_flag == 1) {
									 PowNum = PowNumber(key);
								 } else { first_number = FirstNumber(key); }	 
							   final_answer = first_number;
						   }
							 
						   // second number operation
						   if (number_counter == 1) {
								 
								 if (negative_flag == 1) { // the first number might be negative
									 negative_flag = 0;
									 first_number = first_number * -1;
								 }
								 
								 if (TenPow_flag == 1) {
									 PowNum = PowNumber(key);
								 } else { second_number = SecondNumber(key); }	

							   if (operator_position[1] == 1) { // specify the second operator is addition
							     final_answer = first_number + second_number;
							   }
							   if (operator_position[1] == 2) { // specify the second operator is subtraction
							     final_answer = first_number - second_number;
							   }
								 if (operator_position[1] == 3) { // specify the second operator is multiplication
							     final_answer = first_number * second_number;
							   }
							   if (operator_position[1] == 4) { // specify the second operator is division
							     final_answer = first_number / second_number;
							   }
								 
						   }
							 
							 // third number operation
						   if (number_counter == 2) {
								 if (TenPow_flag == 1) {
									 PowNum = PowNumber(key);
								 } else { third_number = ThirdNumber(key); }	
								 
					///////////////////////// precedence operations ////////////////////////////
								 
								 number_prioritized_operands = 0;
								 if ((operator_position[1] == 3) || (operator_position[1] == 4)){
									 number_prioritized_operands++;
							   }
								 if ((operator_position[2] == 3) || (operator_position[2] == 4)){
									 number_prioritized_operands++;
							   }
								 
								 //  three-number operations involved only plus and minus
								 if (number_prioritized_operands == 0) {
			           
							     if ((operator_position[1] == 1) && (operator_position[2] == 1)) {
									   final_answer = first_number + second_number + third_number;
							     }
								   else if ((operator_position[1] == 2) && (operator_position[2] == 2)) {
									   final_answer = first_number - second_number - third_number;
								   }
								   else if ((operator_position[1] == 1) && (operator_position[2] == 2)) {
								  	 final_answer = first_number + second_number - third_number;
							     }
							     else if ((operator_position[1] == 2) && (operator_position[2] == 1)) {
								  	 final_answer = first_number - second_number + third_number;
							     }
									 
								 }
								 
								  // three-number operations involved only one prioritized operand
								  if (number_prioritized_operands == 1) {
										
										  if ((operator_position[1] == 3) || (operator_position[1] == 4)) {
												  if (operator_position[1] == 3) {
												    first_part = first_number * second_number;
												  } else if (operator_position[1] == 4){
													  first_part = first_number / second_number;
												  }
												  if (operator_position[2] == 1) {
													  final_answer = first_part + third_number;
												  } else if (operator_position[2] == 2){
													  final_answer = first_part - third_number;
												  }
											} else if ((operator_position[2] == 3) || (operator_position[2] == 4)) {
												  if (operator_position[2] == 3) {
												    first_part = second_number * third_number;
												  } else if (operator_position[2] == 4){
													  first_part = second_number / third_number;
												  }
												  if (operator_position[1] == 1) {
													  final_answer = first_number + first_part;
												  } else if (operator_position[1] == 2){
													  final_answer = first_number - first_part;
												  }
										  }
	                    
								  }
									
								  // three-number operations involved all prioritized operands
								  if (number_prioritized_operands == 2) {
										
									  if ((operator_position[1] == 3) && (operator_position[2] == 3)) {
									    final_answer = first_number * second_number * third_number;
							      }
								    else if ((operator_position[1] == 4) && (operator_position[2] == 4)) {
									    final_answer = first_number / second_number / third_number;
								    }
								    else if ((operator_position[1] == 3) && (operator_position[2] == 4)) {
								  	  final_answer = first_number * second_number / third_number;
							      }
							      else if ((operator_position[1] == 4) && (operator_position[2] == 3)) {
								  	  final_answer = first_number / second_number * third_number;
							      }
									  
									}
									
						   }
							 
				     break;
							 
					   case '+':
							 
							 output_cosine_answer();
						   output_sine_answer();
						   output_tan_answer();
						   output_TenPow_answer();
						   output_log10_answer();
						   //output_sprt_answer();
						 
						   decimal_flag = 0; decimal_counter = 1.0; decimal_syntax_error_counter = 0; // reset the decimal flag for inputting an integer
					     number_counter++; // move to next number
						   decimal_digit_counter = 0;
					     operator_position[number_counter] = 1; // plus = 1
					   break;
						 
					   case '-':
							 
							 output_cosine_answer();
						   output_sine_answer();
						   output_tan_answer();
						   output_TenPow_answer();
						 	 output_log10_answer();
					  	 //output_sprt_answer();
						 
							 if ((number_counter == 0) && (FirstNumberNegative_flag == 0)) { // only valid for the first number
								 FirstNumberNegative_flag = 1;
							   operator_position[0] = 2; // the first number is negative
								 negative_flag = 1;
							 } else {
						     decimal_flag = 0; decimal_counter = 1.0; decimal_syntax_error_counter = 0; // reset the decimal flag for inputting an integer
					       number_counter++; // move to next number
					       operator_position[number_counter] = 2; // minus = 2
							 }
							 decimal_digit_counter = 0;
					   break;
							 
						 case 'x':
							 output_cosine_answer();
						   output_sine_answer();
						   output_tan_answer();
						   output_TenPow_answer();
						   output_log10_answer();
						   //output_sprt_answer();
						 
							 decimal_flag = 0; decimal_counter = 1.0; decimal_syntax_error_counter = 0; // reset the decimal flag for inputting an integer
					     number_counter++; // move to next number
					     operator_position[number_counter] = 3; // times = 3
						   decimal_digit_counter = 0;
						 break;
					 
						 case 0xFD: // division
							 
							 output_cosine_answer();
						   output_sine_answer();
						   output_tan_answer();
						   output_TenPow_answer();
						   output_log10_answer();
						   //output_sprt_answer();
						 
							 decimal_flag = 0; decimal_counter = 1.0; decimal_syntax_error_counter = 0; // reset the decimal flag for inputting an integer
					     number_counter++; // move to next number
					     operator_position[number_counter] = 4; // divided = 4
						   decimal_digit_counter = 0;
						 break;
						 
						 case 'E': // Times ten to the power
						   TenPow_flag = 1;
						   delete_TenPow_flag++;
						 
						   decimal_flag = 0; decimal_counter = 1.0; decimal_syntax_error_counter = 0; decimal_digit_counter = 0;
						 
						   if (delete_TenPow_flag == 2) { // to prevent the case of "1+10EE"
								 CursorAddress--;
								 ClearDisplayScreen();						 
				         SendDisplayString("Syntax ERROR");
				         delay_us(1000000);
								 ClearDisplayScreen();						 
				         SendDisplayString(equation_buffer);
							 }
							 
						 break;
							 
						 case 'l': // logarithm with base 10 of the input
						   log10_flag = 1;
						   syntax_error_counter = 1;
						 break;
						 
						 case 'A': // cosine function
						   cosine_flag = 1;
						   syntax_error_counter = 1;
						 break;
						 
						 case 'C': // sine function
						   sine_flag = 1;
						   syntax_error_counter = 1;
						 break;
							 
						 case 'F': // tangent function
						   tan_flag = 1;
						   syntax_error_counter = 1;
						 break; 
						
						 /*case 'r': // sqrt
						   sqrt_flag = 1;
						   syntax_error_counter = 1;
						   if (max_op_sqrt_flag == 1) { max_op_sqrt_flag = 0; number_counter++ ; } // to prevent continue the fourth number operation
							 
						 break;*/
							 
						 case 'R': // Rubout last digit
							 
						   cosine_syntax_error_counter = 0;
				       sine_syntax_error_counter = 0;
				       tan_syntax_error_counter = 0;
						   log10_syntax_error_counter = 0;
						   //sqrt_syntax_error_counter = 0;
							 
				 	/////////////////////////// delete entire cosine input /////////////////////////////
						   if (cosine_flag == 1) {
								 cosine_flag = 0;
								 delete_entire_cosine_function(decimal_digit_counter, decimal_flag);

				 	/////////////////////////// delete entire sine input /////////////////////////////
							 } else if (sine_flag == 1) {
								 sine_flag = 0;
								 delete_entire_sine_function(decimal_digit_counter, decimal_flag);
	 
				 	/////////////////////////// delete entire tangent input /////////////////////////////
							 } else if (tan_flag == 1) {
								 tan_flag = 0;
								 delete_entire_tan_function(decimal_digit_counter, decimal_flag);
								 
				 	/////////////////////////// delete entire Ten power input /////////////////////////////
					     } else if (TenPow_flag == 1) {
								 TenPow_flag = 0;
								 delete_entire_tenpow_function(decimal_digit_counter, decimal_flag);
								 
				 	/////////////////////////// delete entire log10 input /////////////////////////////
							 } else if (log10_flag == 1) {
								 log10_flag = 0;
								 delete_entire_log10_function(decimal_digit_counter, decimal_flag);

				 	/////////////////////////// delete entire sqrt input /////////////////////////////
							 /*} else if (sqrt_flag == 1) {
								 sqrt_flag = 0;
								 delete_entire_sqrt_function(decimal_digit_counter, decimal_flag);*/
								 
					 ///////////////////// rubout the last character of each number on the display including each operand /////////////////////////////
							 } else {
								 rubout_last_entry();
                 recalculation(); // recalculate the final answer at once after delete last digit(s)
							 
						   }
							 
						 break;
							 
						 case 'D':
							 
						   reboot_flag++;
						   if (reboot_flag == 2) { 
							   reboot_flag = 0; 
							   password_entry = 0;
								 ClearDisplayScreen();
								 SendDisplayString("Bye! See Ya!");
								 delay_us(1500000);
							 }
						   ClearDisplayScreen();
						   
						   // memory
						   CursorAddress = 1;
							 memset(answer_buffer,0, sizeof(answer_buffer));
						   memset(equation_buffer,0, sizeof(equation_buffer));
               final_answer = 0; first_number = 0; second_number = 0; third_number = 0;
               first_part = 0;
						   operator_position[0] = 0; operator_position[1] = 0; operator_position[2] = 0;

               // Flags
               shift_flag = 0;
               decimal_flag = 0;
               continue_calculation_flag = 0;
               repressed_key_flag = 0;
               negative_flag = 0;
               FirstNumberNegative_flag = 0;
               DisplayShift_flag = 0;
               delete_ContinueCalculation_flag = 0;

               // Counters
               decimal_counter = 1.0;
               number_counter = 0;
               number_prioritized_operands = 0;
               syntax_error_counter = 0;
               decimal_syntax_error_counter = 0;
               decimal_digit_counter = 0;
							 
							 // cosine
							 cosine_flag = 0;
							 max_op_cosine_flag = 0;
							 cosine_syntax_error_counter = 0;
							 
							 // sine
							 sine_flag = 0;
							 max_op_sine_flag = 0;
							 sine_syntax_error_counter = 0;
							 
							 // tangent
							 tan_flag = 0;
							 max_op_tan_flag = 0;
							 tan_syntax_error_counter = 0;
							 
							 // Ten to the power
							 TenPow_flag = 0;
               PowNum = 0;
               delete_TenPow_flag = 0;
							 
							 // log10
							 log10_flag = 0;
							 max_op_log10_flag = 0;
							 log10_syntax_error_counter = 0;
							 
							 // sqrt not enough RAM
							 
							 
						 break;
						 
				   }
					 
					 // control which key print to display
           
					 if (key == 'S') {
						 shift_flag = 1;
						 GPIO_PORTF_DATA_R = 0x0C;
						 
				   } else if (key == 'A') { // cosine function
						 if (cosine_syntax_error_counter == 1) {
							 if ((number_counter == 0) ||(number_counter == 1) ||(number_counter == 2)) {
								 strcat(equation_buffer, "cos");
						     ClearDisplayScreen();
						     SendDisplayString(equation_buffer);
							 }
						 } 
						 
				   } else if (key == 'C') { // sine function
						 if (sine_syntax_error_counter == 1) {
							 if ((number_counter == 0) ||(number_counter == 1) ||(number_counter == 2)) {
								 strcat(equation_buffer, "sin");
						     ClearDisplayScreen();
						     SendDisplayString(equation_buffer);
							 }
						 } 
						 
				   } else if (key == 'F') { // tangent function
						 if (tan_syntax_error_counter == 1) {
							 if ((number_counter == 0) ||(number_counter == 1) ||(number_counter == 2)) {
								 strcat(equation_buffer, "tan");
						     ClearDisplayScreen();
						     SendDisplayString(equation_buffer);
							 }
						 } 
						 
				   } else if (key == 'l') { // log10
						 if (log10_syntax_error_counter == 1) {
							 if ((number_counter == 0) ||(number_counter == 1) ||(number_counter == 2)) {
								 strcat(equation_buffer, "log");
						     ClearDisplayScreen();
						     SendDisplayString(equation_buffer);
							 }
						 } 
						 
				   /*} else if (key == 'r') { // sqrt
						 if (sqrt_syntax_error_counter == 1) {
							 if ((number_counter == 0) ||(number_counter == 1) ||(number_counter == 2)) {
								 strcat(equation_buffer, "sqrt");
						     ClearDisplayScreen();
						     SendDisplayString(equation_buffer);
							 }
						 } */
						 
					 } else if ((decimal_flag == 1) && (TenPow_flag == 1)) {
						 decimal_flag = 0;
						 
					 } else if ((key == 'D') || (number_counter > 2)) {
						 CursorAddress--;
						 
					 } else if ((delete_TenPow_flag == 2) || (decimal_syntax_error_counter == 2)) {
						 decimal_syntax_error_counter = 1;
						 delete_TenPow_flag = 1;
						 
					 } else if (key == 'R') {
						 
					 } else {
				     SendDisplayByte(key, 0x08); // print the keypad input to the LCD
						 strcat(equation_buffer, &key);
				   }
					 
			   }
			 
			   if (key == '=') {
					 
					 for (int i = 0; i < DisplayShift_flag; i++) { // cancel out the left-shifted display shifted by writing equation
						 SendDisplayByte(right_shift_display, 0x00);
					 }
					 DisplayShift_flag = 0;
					 
					 if ((number_counter == 0) && (negative_flag == 1)) { // the first number might be negative
						 final_answer = final_answer * -1;
					 }
				   
					 output_cosine_answer(); // cosine function
					 output_sine_answer(); // cosine function	 
					 output_tan_answer(); // tangent function
					 output_TenPow_answer(); // times ten to the power
					 output_log10_answer(); // log10
					 //output_sqrt_answer(); //sqrt
					 recalculation();
					 
					 // print answer
					 memset(answer_buffer, 0, sizeof(answer_buffer));
				   sprintf(answer_buffer, "= %0.4f", final_answer);	// correct to 4 decimal places
					 next_line();
				   SendDisplayString(answer_buffer);
					 
					 // reset counters, flags and number memory to continue calculation
				   number_counter = 0;
				   continue_calculation_flag = 1;
					 operator_position[0] = 0; operator_position[1] = 0; operator_position[2] = 0;
					 first_part = 0;
					 decimal_digit_counter = 4;
					 delete_ContinueCalculation_flag = 1;
					 
					 cosine_syntax_error_counter = 0;
					 sine_syntax_error_counter = 0;
					 tan_syntax_error_counter = 0;
					 delete_TenPow_flag = 0;
					 log10_syntax_error_counter = 0;
					 //sqrt_syntax_error_counter = 0;
					 
			   }
			 
		   }
			 
			 repressed_key_flag = 0;
			 			 
			 // reminder and action to prevent the user from having more than two operands
			 if (number_counter > 2) {
				 
				 syntax_error_counter = 0;
				 number_counter = 2;
				 cosine_flag = 0;
				 sine_flag = 0;
				 tan_flag = 0;
				 log10_flag = 0;
				 //sqrt_flag = 0;
				 
         ClearDisplayScreen();			 
				 SendDisplayString("Max 2 operands"); // buffer for showing "Max 2 operands"
				 delay_us(1000000);
				 
				 ClearDisplayScreen();
         SendDisplayString(equation_buffer);
				 
			 }
				 
	  }
  }
  }
}

void password_encryption(void) {
	 
    char password_buffer[10];
		GPIO_PORTF_DATA_R = 0x02;
		change_password_flag = 0;
		first_read = keypad_KeyPressed(shift_flag);
		SysTick_Wait(10000000);
		second_read = keypad_KeyPressed(shift_flag);
		
		if (keypad_KeyPressed(shift_flag) != 'X' && (first_read != second_read)) {
			
			key = keypad_KeyPressed(shift_flag);
			
			if ((key != '=') && (key != '.') && (key != 'R') && (key != '+') && (key != '-') && (key != 'S')) {
			  GPIO_PORTF_DATA_R = 0x0A;
			  delay_us(80000);
			  SendDisplayByte(key, 0x08);
				strcat(password_entry_buffer, &key);
				strcpy(password_buffer, password_entry_buffer);
			}
			else if (key == '=') {
				password_user_entered = atoi(password_buffer); // convert string to integer
				
				if (Flash_read(password_stored, sizeof(password_stored) / sizeof(uint32_t)) == password_user_entered) {
					GPIO_PORTF_DATA_R = 0x08;
					password_user_entered = 0;
          Entry_graphic();
					
					// change password
					memset(password_entry_buffer, 0, sizeof(password_entry_buffer));
					change_password();
					
				} else {
					password_user_entered = 0;
					ClearDisplayScreen();
					SendDisplayString("WRONG Password"); next_line();
					SendDisplayString("(*") ; SendDisplayByte(0x6F, 0x08);
					SendDisplayString("*) !!") ;
					delay_us(1000000);
					ClearDisplayScreen();
				  SendDisplayString("ENTER PASSWORD:"); next_line();
				}
				
				memset(password_entry_buffer, 0, sizeof(password_entry_buffer));
				memset(password_buffer, 0, sizeof(password_buffer));
			}
		  
		}
		
}

void change_password(void) {
	
	while (change_password_flag == 0) {
		
		ClearDisplayScreen();
		SendDisplayString("(1) Change PW"); next_line();
		SendDisplayString("(2) Start Math !");
		
		first_read = keypad_KeyPressed(shift_flag);
		SysTick_Wait(10000000);
		second_read = keypad_KeyPressed(shift_flag);
		
		if (keypad_KeyPressed(shift_flag) != 'X' && (first_read != second_read)) {
			
			key = keypad_KeyPressed(shift_flag);
			
			if ((key == '1') || (key == '2')) {
				
				if (key == '1') {
					ClearDisplayScreen();
					SendDisplayString("Eeter NEW PW:"); next_line();
					
					while (1) {
						first_read = keypad_KeyPressed(shift_flag);
		        SysTick_Wait(10000000);
		        second_read = keypad_KeyPressed(shift_flag);
		
		        if (keypad_KeyPressed(shift_flag) != 'X' && (first_read != second_read)) {
							key = keypad_KeyPressed(shift_flag);
							if ((key != '=') && (key != '.') && (key != 'R') && (key != '+') && (key != '-') && (key != 'S')) {
								SendDisplayByte(key,0x08);
							  strcat(password_entry_buffer, &key);
							} else if (key == '=') {
								password_user_entered = atoi(password_entry_buffer);
								password_stored = password_user_entered;
								password_user_entered = 0;
								break;
							}
						}
					}
					
					ClearDisplayScreen();
					SendDisplayString("RE-ENTER NEW PW:"); next_line();
					change_password_flag = 1;
				}
				else if (key == '2') {
  				password_entry = 1; 
				  change_password_flag = 1;
				}
				
			}
		}
		
	}
	
}

float FirstNumber(char first_number_key) {
	
	if (decimal_flag == 0) {
		FirstNumberNegative_flag = 1;
		first_number = first_number * 10 + (first_number_key - '0'); // converting ASCII to actual number
	}
	if (decimal_flag == 1) {
		FirstNumberNegative_flag = 1;
		decimal_digit_counter++;
		first_number = first_number + (first_number_key - '0') * 0.1 * decimal_counter;
		decimal_counter = decimal_counter * 0.1;
	}
	return first_number;
	
}

float SecondNumber(char second_number_key) {
	
	if (decimal_flag == 0) {
		second_number = second_number * 10 + (second_number_key - '0'); // converting ASCII to actual number
	}
	if (decimal_flag == 1) {
		decimal_digit_counter++;
		second_number = second_number + (second_number_key - '0') * 0.1 * decimal_counter;
		decimal_counter = decimal_counter * 0.1;
	}
	return second_number;
	
}

float ThirdNumber(char third_number_key) {
	
	if (decimal_flag == 0) {
		third_number = third_number * 10 + (third_number_key - '0'); // converting ASCII to actual number
	}
	if (decimal_flag == 1) {
		decimal_digit_counter++;
		third_number = third_number + (third_number_key - '0') * 0.1 * decimal_counter;
		decimal_counter = decimal_counter * 0.1;
	}
	return third_number;
	
}

float PowNumber(char PowNum_key) {
	
	if (decimal_flag == 0) {
		PowNum = PowNum * 10 + (PowNum_key - '0'); // converting ASCII to actual number
	}
	if (decimal_flag == 1) {
		decimal_digit_counter++;
		PowNum = PowNum + (PowNum_key - '0') * 0.1 * decimal_counter;
		decimal_counter = decimal_counter * 0.1;
	}
	return PowNum;
	
}

void recalculation(void) {
	
	if (number_counter == 0) {
		final_answer = first_number;
	}
							 
	// second number operation
	if (number_counter == 1) {
								 
		if (negative_flag == 1) { // the first number might be negative
			negative_flag = 0;
		}
								 
		if (operator_position[1] == 1) { // specify the second operator is addition
			final_answer = first_number + second_number;
		}
		if (operator_position[1] == 2) { // specify the second operator is subtraction
			final_answer = first_number - second_number;
		}
		if (operator_position[1] == 3) { // specify the second operator is multiplication
			final_answer = first_number * second_number;
		}
		if (operator_position[1] == 4) { // specify the second operator is division
			final_answer = first_number / second_number;
		}
								 
	}
							 
	// third number operation
	if (number_counter == 2) {
										 
	//  three-number operations involved only plus and minus
		if (number_prioritized_operands == 0) {
			           
			if ((operator_position[1] == 1) && (operator_position[2] == 1)) {
				final_answer = first_number + second_number + third_number;
			}
		  else if ((operator_position[1] == 2) && (operator_position[2] == 2)) {
		    final_answer = first_number - second_number - third_number;
	    }
			else if ((operator_position[1] == 1) && (operator_position[2] == 2)) {
				final_answer = first_number + second_number - third_number;
			}
			else if ((operator_position[1] == 2) && (operator_position[2] == 1)) {
				final_answer = first_number - second_number + third_number;
			}
									 
		}
								 
		// three-number operations involved only one prioritized operand
		if (number_prioritized_operands == 1) {
										
			if ((operator_position[1] == 3) || (operator_position[1] == 4)) {
				if (operator_position[1] == 3) {
					first_part = first_number * second_number;
				} else if (operator_position[1] == 4){
					first_part = first_number / second_number;
				}
				if (operator_position[2] == 1) {
					final_answer = first_part + third_number;
				} else if (operator_position[2] == 2){
					final_answer = first_part - third_number;
				}
			} else if ((operator_position[2] == 3) || (operator_position[2] == 4)) {
				if (operator_position[2] == 3) {
				  first_part = second_number * third_number;
				} else if (operator_position[2] == 4){
					first_part = second_number / third_number;
				}
				if (operator_position[1] == 1) {
					final_answer = first_number + first_part;
				} else if (operator_position[1] == 2){
					final_answer = first_number - first_part;
				}
		  }
	                    
	  }
									
		// three-number operations involved all prioritized operands
		if (number_prioritized_operands == 2) {
										
			if ((operator_position[1] == 3) && (operator_position[2] == 3)) {
				final_answer = first_number * second_number * third_number;
			}
			else if ((operator_position[1] == 4) && (operator_position[2] == 4)) {
				final_answer = first_number / second_number / third_number;
			}
		  else if ((operator_position[1] == 3) && (operator_position[2] == 4)) {
				final_answer = first_number * second_number / third_number;
			}
			else if ((operator_position[1] == 4) && (operator_position[2] == 3)) {
				final_answer = first_number / second_number * third_number;
			}
									  
		}
									
	}
							 
}

int update_CursorAddress(int n) { // to increment the cursor address to the digit size of the final answer
  int count = 1;
	if (n < 0) { count++; }
  while (n != 0) {
      n = n / 10;
      count++;
  }
  return count + 5; // + 5 because 4 decimal places and the decimal point
}

void delete_last_printed_entry(void) {
	strcpy(&equation_buffer[CursorAddress - 1], ""); // replace the printed operand sign to the digit just pressed
  ClearDisplayScreen();
  SendDisplayString(equation_buffer);
  CursorAddress--;
}

float RemoveLastDigit_AnyNumber(float input, int delete_decimal_digit_counter) {
  int integer_input = 0, delete_negative_flag = 0;	
  float decimal = 0.0, answer = 0.0;
	
	if (input == floor(input)) {
	  answer = floor(input * 0.1);
	}
	else {
	  if (input < 0) {
	    input = input * -1;
      delete_negative_flag = 1;		
	  }
	
	  integer_input = floor(input);
	  decimal = input - integer_input;
	  decimal = floor(decimal * 10 * (delete_decimal_digit_counter - 1));
	  decimal = decimal / ((delete_decimal_digit_counter - 1) * 10);
	  answer = integer_input + decimal;
  }
	
	if (delete_negative_flag == 1) { answer = answer * -1; }
	
	return answer;
}
 
int Count_AnyNumber_digit(float input, int count_decimal_digit_counter, int count_decimal_flag) { // inputs: (1) inputted number (2) decimal places (3) decimal or integer 
	
	int integer = 0, integer_count = 0, decimal_count = 0, total_count = 0;
	float decimal = 0.0;
	
	integer = floor(abs(input));
  while (integer != 0) {
      integer = integer / 10;
      integer_count++;
  }
	
	if (count_decimal_flag == 1) {
	  decimal = input - floor(abs(input));
	  decimal = decimal * 10 * count_decimal_digit_counter;
		decimal = floor(decimal);
	  while (decimal != 0) {
      decimal = decimal / 10;
      decimal_count++;
    }
  }
	
	if (count_decimal_flag == 1) { decimal_count++; }
	total_count = integer_count + decimal_count;
	
	return total_count;
}

void rubout_last_entry (void) {
	int delete_total_digits = 0;
	int TotalDigits_one_time_flag = 0;
	delete_last_printed_entry();
								 
	if (TenPow_flag == 1) { delete_total_digits++; }
							 
	if (number_counter == 0) {
		if (delete_ContinueCalculation_flag == 1) { // delete decimal point and places
			for (int j = 0; j < 6; j++) {
			delete_last_printed_entry();
		  }
		  delete_ContinueCalculation_flag = 0;
	  }
		first_number = RemoveLastDigit_AnyNumber(first_number, decimal_digit_counter);
		if (TotalDigits_one_time_flag == 0) {
		  delete_total_digits =  Count_AnyNumber_digit(first_number, decimal_digit_counter, decimal_flag);
			delete_total_digits--;
			TotalDigits_one_time_flag = 1;
		}
		if (delete_total_digits == 0) {
			number_counter = 0;
			TotalDigits_one_time_flag = 0;
		}
	}
							 
	if (number_counter == 1) {
		second_number = RemoveLastDigit_AnyNumber(second_number, decimal_digit_counter);
		if (TotalDigits_one_time_flag == 0) {
			delete_total_digits =  Count_AnyNumber_digit(second_number, decimal_digit_counter, decimal_flag);
			delete_total_digits--;
		  TotalDigits_one_time_flag = 1;
		}
		if (delete_total_digits == -1) { // -1 to rubout the printed operand character
			number_counter--;
			TotalDigits_one_time_flag = 0;
		}
	}
							 
	if (number_counter == 2) {
		third_number = RemoveLastDigit_AnyNumber(third_number, decimal_digit_counter);
	  if (TotalDigits_one_time_flag == 0) {
		  delete_total_digits =  Count_AnyNumber_digit(third_number, decimal_digit_counter, decimal_flag);
			delete_total_digits--;
		  TotalDigits_one_time_flag = 1;
	  } 
	  if (delete_total_digits == -1) { // -1 to rubout the printed operand character
		  number_counter--;
		  TotalDigits_one_time_flag = 0;
	  }
	}
								 
	delete_total_digits = 0;
}

void output_cosine_answer(void) {
	
	if (cosine_flag == 1) {		
				 
		if (number_counter == 0) {			 
			//user writes in deg mode
			first_number = first_number * PI / 180; // convert rad to deg
			first_number = cos(first_number);
		}			 
		if (number_counter == 1) {					 
			// user writes in deg mode
			second_number = second_number * PI / 180; // convert rad to deg
			second_number = cos(second_number);
		}
		if (number_counter == 2) {					 
			// user writes in deg mode
			third_number = third_number * PI / 180; // convert deg to rad
			third_number = cos(third_number);
		}
	}
	cosine_flag = 0;
	
}

void delete_entire_cosine_function(int cosine_decimal_digit_counter, int cosine_decimal_flag) {
	int delete_cosine_digits_counter = 0;
									 
	if  (number_counter == 0) {
	  delete_cosine_digits_counter = 3 + Count_AnyNumber_digit(first_number, cosine_decimal_digit_counter, cosine_decimal_flag);
	  first_number = 0;
									 
	} else if (number_counter == 1) {
		delete_cosine_digits_counter = 3 + Count_AnyNumber_digit(second_number, cosine_decimal_digit_counter, cosine_decimal_flag);
		second_number = 0;
									 
	} else if (number_counter == 2) {
	  delete_cosine_digits_counter = 3 + Count_AnyNumber_digit(third_number, cosine_decimal_digit_counter, cosine_decimal_flag);
		third_number = 0;
	}
								 
	for (int c = 0; c < delete_cosine_digits_counter; c++) {
		delete_last_printed_entry();
	}
								 
	delete_cosine_digits_counter = 0;
	CursorAddress = CursorAddress - delete_cosine_digits_counter;
}

void output_sine_answer(void) {
	
	if (sine_flag == 1) {	
				 
		if (number_counter == 0) {					 
			// user writes in deg mode
			first_number = first_number * PI / 180; // convert rad to deg
			first_number = sin(first_number);
		}
		if (number_counter == 1) {					 
			// user writes in deg mode
			second_number = second_number * PI / 180; // convert rad to deg
			second_number = sin(second_number);
		}		 
		if (number_counter == 2) {				 
			// user writes in deg mode
			third_number = third_number * PI / 180; // convert deg to rad
			third_number = sin(third_number);
		}
	}
	sine_flag = 0;
}

void delete_entire_sine_function(int sine_decimal_digit_counter, int sine_decimal_flag) {
	int delete_sine_digits_counter = 0;
	
	if  (number_counter == 0) {
		delete_sine_digits_counter = 3 + Count_AnyNumber_digit(first_number, sine_decimal_digit_counter, sine_decimal_flag);
	  first_number = 0;
									 
	} else if (number_counter == 1) {
	  delete_sine_digits_counter = 3 + Count_AnyNumber_digit(second_number, sine_decimal_digit_counter, sine_decimal_flag);
		second_number = 0;
									 
	} else if (number_counter == 2) {
		delete_sine_digits_counter = 3 + Count_AnyNumber_digit(third_number, sine_decimal_digit_counter, sine_decimal_flag);
		third_number = 0;
	}
								 
  for (int c = 0; c < delete_sine_digits_counter; c++) {
		delete_last_printed_entry();
	}
								 
	delete_sine_digits_counter = 0;
	CursorAddress = CursorAddress - delete_sine_digits_counter;			 
}

void output_tan_answer(void) {
	
	if (tan_flag == 1) {	
				 
		if (number_counter == 0) {					 
			// user writes in deg mode
			first_number = first_number * PI / 180; // convert rad to deg
			first_number = tan(first_number);
		}
								 
		if (number_counter == 1) {						 
			// print calculated the answer, user writes in deg mode
			second_number = second_number * PI / 180; // convert rad to deg
			second_number = tan(second_number);
		}
					 
		if (number_counter == 2) {						 
			// print calculated the answer, user writes in deg mode
			third_number = third_number * PI / 180; // convert deg to rad
			third_number = tan(third_number);
		}
	}
	tan_flag = 0;
}

void delete_entire_tan_function(int tan_decimal_digit_counter, int tan_decimal_flag) {
	int delete_tan_digits_counter = 0;
	
	if  (number_counter == 0) {
		delete_tan_digits_counter = 3 + Count_AnyNumber_digit(first_number, tan_decimal_digit_counter, tan_decimal_flag);
	  first_number = 0;
									 
	} else if (number_counter == 1) {
	  delete_tan_digits_counter = 3 + Count_AnyNumber_digit(second_number, tan_decimal_digit_counter, tan_decimal_flag);
		second_number = 0;
									 
	} else if (number_counter == 2) {
		delete_tan_digits_counter = 3 + Count_AnyNumber_digit(third_number, tan_decimal_digit_counter, tan_decimal_flag);
		third_number = 0;
	}
								 
	for (int c = 0; c < delete_tan_digits_counter; c++) {
		delete_last_printed_entry();
	}
								 
	delete_tan_digits_counter = 0;
	CursorAddress = CursorAddress - delete_tan_digits_counter;
}

void output_TenPow_answer(void) {
	
	if (TenPow_flag == 1) {			
				 
		if (number_counter == 0) {
			if (PowNum > 0) { first_number = pow(10, PowNum) * first_number; }
			else if (PowNum == 0) { first_number = first_number; }
			else if (PowNum < 0) { first_number = pow(0.1, PowNum) * first_number; }
		}
								 
		if (number_counter == 1) {
			if (PowNum > 0) { second_number = pow(10, PowNum) * second_number; }
			else if (PowNum == 0) { second_number = second_number; }
			else if (PowNum < 0) { second_number = pow(0.1, PowNum) * second_number; }

		}
					 
		if (number_counter == 2) {						 
			if (PowNum > 0) { third_number = pow(10, PowNum) * third_number; }
			else if (PowNum == 0) { third_number = third_number; }
			else if (PowNum < 0) { third_number = pow(0.1, PowNum) * third_number; }
		}
		
		TenPow_flag = 0;
		PowNum = 0;
	}
}

void delete_entire_tenpow_function(int tenpow_decimal_digit_counter, int tenpow_decimal_flag) {
	int delete_tenpow_digits_counter = 0;
	
	if  (number_counter == 0) {
		delete_tenpow_digits_counter = 1 + Count_AnyNumber_digit(PowNum, decimal_digit_counter, decimal_flag) + Count_AnyNumber_digit(first_number, decimal_digit_counter, decimal_flag) - 1;
	  first_number = 0;
									 
	} else if (number_counter == 1) {
	  delete_tenpow_digits_counter = 1 + Count_AnyNumber_digit(PowNum, decimal_digit_counter, decimal_flag) + Count_AnyNumber_digit(second_number, decimal_digit_counter, decimal_flag) - 1;
		second_number = 0;
									 
	} else if (number_counter == 2) {
		delete_tenpow_digits_counter = 1 + Count_AnyNumber_digit(PowNum, decimal_digit_counter, decimal_flag) + Count_AnyNumber_digit(third_number, decimal_digit_counter, decimal_flag) - 1;
		third_number = 0;
	}
								 
		for (int c = 0; c < delete_tenpow_digits_counter; c++) {
			delete_last_printed_entry();
		}
		
    PowNum = 0;		
		delete_tenpow_digits_counter = 0;
		CursorAddress = CursorAddress - delete_tenpow_digits_counter;
}

void output_log10_answer(void) {
	
	if (log10_flag == 1) {	
				 
		if (number_counter == 0) {					 
			first_number = log10(first_number);
		}					 
		if (number_counter == 1) {						 
			second_number = log10(second_number);
		}
		if (number_counter == 2) {						 
			third_number = log10(third_number);
		}
		
	}
	log10_flag = 0;
}

void delete_entire_log10_function(int log10_decimal_digit_counter, int log10_decimal_flag) {
	int delete_log10_digits_counter = 0;
	
	if  (number_counter == 0) {
		delete_log10_digits_counter = 3 + Count_AnyNumber_digit(first_number, log10_decimal_digit_counter, log10_decimal_flag);
	  first_number = 0;
									 
	} else if (number_counter == 1) {
	  delete_log10_digits_counter = 3 + Count_AnyNumber_digit(second_number, log10_decimal_digit_counter, log10_decimal_flag);
		second_number = 0;
									 
	} else if (number_counter == 2) {
		delete_log10_digits_counter = 3 + Count_AnyNumber_digit(third_number, log10_decimal_digit_counter, log10_decimal_flag);
		third_number = 0;
	}
								 
	for (int c = 0; c < delete_log10_digits_counter; c++) {
		delete_last_printed_entry();
	}
								 
	delete_log10_digits_counter = 0;
	CursorAddress = CursorAddress - delete_log10_digits_counter;
}

/*void output_sqrt_answer(void) {
	
	if (sqrt_flag == 1) {	
				 
		if (number_counter == 0) {					 
			first_number = sqrt(first_number);
		}					 
		if (number_counter == 1) {						 
			second_number = sqrt(second_number);
		}
		if (number_counter == 2) {						 
			third_number = sqrt(third_number);
		}
		
	}
	sqrt_flag = 0;
}

void delete_entire_sqrt_function(int sqrt_decimal_digit_counter, int sqrt_decimal_flag) {
	int delete_sqrt_digits_counter = 0;
	
	if  (number_counter == 0) {
		delete_sqrt_digits_counter = 3 + Count_AnyNumber_digit(first_number, sqrt_decimal_digit_counter, sqrt_decimal_flag);
	  first_number = 0;
									 
	} else if (number_counter == 1) {
	  delete_sqrt_digits_counter = 3 + Count_AnyNumber_digit(second_number, sqrt_decimal_digit_counter, sqrt_decimal_flag);
		second_number = 0;
									 
	} else if (number_counter == 2) {
		delete_sqrt_digits_counter = 3 + Count_AnyNumber_digit(third_number, sqrt_decimal_digit_counter, sqrt_decimal_flag);
		third_number = 0;
	}
								 
	for (int r = 0; r < delete_sqrt_digits_counter; r++) {
		delete_last_printed_entry();
	}
								 
	delete_sqrt_digits_counter = 0;
	CursorAddress = CursorAddress - delete_sqrt_digits_counter;
}*/


void PortF_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;     // 1) F clock
  delay = SYSCTL_RCGC2_R;           // delay   
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock PortF PF0  
  GPIO_PORTF_CR_R |= 0x1F;           // allow changes to PF4-0       
  GPIO_PORTF_AMSEL_R &= 0x00;        // 3) disable analog function
  GPIO_PORTF_PCTL_R &= 0x00000000;   // 4) GPIO clear bit PCTL  
  GPIO_PORTF_DIR_R |= 0x0F;          // 5) PF3,2,1,0 output  
  GPIO_PORTF_AFSEL_R &= 0x00;        // 6) no alternate function
  GPIO_PORTF_PUR_R |= 0x11;          // enable pullup resistors on PF4,PF0       
  GPIO_PORTF_DEN_R |= 0x1F;          // 7) enable digital pins PF4-PF0     
}
