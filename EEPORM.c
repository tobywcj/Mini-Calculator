#include "EEPROM.h"

// EEPROM defines
#define RCGCEEPROM 	(*((volatile unsigned long *) 0x400FE658))
#define EEBLOCK			(*((volatile unsigned long *) 0x400AF004))
#define EEOFFSET		(*((volatile unsigned long *) 0x400AF008))
#define EERDWR			(*((volatile unsigned long *) 0x400AF010))
#define PREEPROM 		(*((volatile unsigned long *) 0x400FEA58))
#define	EEDONE 			(*((volatile unsigned long *) 0x400AF018))

// Global Variables
unsigned long eeprom_password; 


/** Initialises EEPROM
*/
void eepromInit(void){
	RCGCEEPROM = 0x01; 				// Start the clock for EEPROM
	SysTick_Wait(160); 			// Wait 2us for few cycles of EEPROM clock
	while(PREEPROM == 0x00); 	// Wait PREEPROM to get ready
	while(EEDONE != 0x00); 		// If an operation is being peformed wait until it is done
	SysTick_Wait(40000); 		// Wait for 500us 
	EEBLOCK = 0x0001; 				// Set the EEPROM block to read/write from/to
	EEOFFSET = 0x1; 					// Set which word to read/write from/to
	if(EERDWR != 1111){
		eeprom_password = EERDWR; // Set the password_store value to the value at the block and word
		while(EEDONE != 0x00); // If an operation is being peformed wait until it is done
	} else {
		// if it is not set to 1111. This is the default password
		EERDWR = 1111;
		while(EEDONE != 0x00); // If an operation is being peformed wait until it is done		
		eeprom_password = EERDWR; // Set the password_store value to the value at the block and word
		while(EEDONE != 0x00); // If an operation is being peformed wait until it is done		
	}
}

/** Set a new password that is stored in the EEPROM
*@param The new password that has to be set
*/
void eeSetPass(int temp_password){
		EEBLOCK = 0x0001; 				// Set the EEPROM block to read/write from/to 
		EEOFFSET = 0x1; 					// Set which word to read/write from/to
		EERDWR = temp_password; 	// The neew value for the passwrd
		while(EEDONE != 0x00); 
		eeprom_password = EERDWR; // Update the old password to the new password
		while(EEDONE != 0x00); 
}

/** Get the current password that has been stored
*@return The current password
*/
unsigned long eeGetPass(void){
	return eeprom_password;
}