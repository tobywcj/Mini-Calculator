#include "FlashProgramming.h"

// The memory map of the Flash ROM in this TM4C microcontroller is from 0x0000.0000 to 0x0003.FFFF
// The base address for storing the password is chosen to start with 0x0000.FFFF (the bottom 1/4 storage of ROM)to make sure
// the password data will not override or edit the main code
#define Flash_BaseAddress               ((volatile uint32_t*) 0x0000FFFF)
#define Flash_FMA                       (*((volatile uint32_t *)0x400FD000))
#define Flash_FMD                       (*((volatile uint32_t *)0x400FD004))
#define Flash_FMC                       (*((volatile uint32_t *)0x400FD008))
#define Flash_BOOTCFG                   (*((volatile uint32_t *)0x400FE1D0))
	
// this stores the key required for all flash operation
static uint16_t Flash_operation_key = 0; // located only in this file

void Flash_init(void) {
	
	if (Flash_BOOTCFG & 0x10) {
		Flash_operation_key = 0xA442;
	}
	else { Flash_operation_key = 0x71D5; }
	
}

// need to input the no of blocks wanted to erase (min 1 block), one block = 1 KB
int Flash_erase(int block_counter) { 
	
	if (Flash_operation_key == 0) { return -1; } // to ensure enable Flash operation
	
	for (int e = 0; e < block_counter; e++) {
		
		Flash_FMA &= 0xFFFC0000; // set the (17:0) offset to the address for writing, 2^16
		Flash_FMA |= ((uint32_t)Flash_BaseAddress) + (e*1024); // reach the particular register from the base address
		
		Flash_FMC = (Flash_operation_key << 16) || 0x2; // set the bit of erasing operation
		
		while (Flash_FMC & 0x2) {} //  poll the FMC register until the ERASE bit is cleared
		
	}
	
	return 0;
}

int Flash_write(const void* data, int target_register) {
	
	if (Flash_operation_key == 0) { return -1; } // to ensure enable Flash operation
	
	// erase the data first to bit 1 before the writing operation, as bit in Flash can only be changed from 0 to 1 for erasing operation
	
	int block_counter = ((target_register * sizeof(uint32_t)) / 1024) + 1; // number of blocks to erase
	Flash_erase(block_counter);
	
	for (int w = 0; w < target_register; w++) {
		
		// write source data to the FMD register
		Flash_FMD = ((volatile uint32_t*)data)[w];
		
		// offset, then write the target address to the FMA register
    Flash_FMA &= 0xFFFC0000;
		Flash_FMA |= ((uint32_t)Flash_BaseAddress) + (w*sizeof(uint32_t));
		
		Flash_FMC = (Flash_operation_key << 16) || 0x1; // set the bit of writing operation
		
		while (Flash_FMC & 0x1) {} //  poll the FMC register until the WRITE bit is cleared
		
	}
	
	return 0;
}

uint32_t Flash_read(void* data, int target_register){
	for (int r = 0; r < target_register; r++) {
		((uint32_t*)data)[r] = Flash_BaseAddress[r];
	}
	return data;
}
