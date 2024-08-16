#ifndef EEPROM_MEMORY_BMS_h
#define EEPROM_MEMORY_BMS_h

#define DEVICEADDRESS  0x50 // 0b0101 0 a2 a1 a0

#include <arduino.h>
#include <Wire.h>

	void eeprom_SaveByte(uint16_t theMemoryAddress, uint8_t u8Byte); 
	uint8_t eeprom_ReadByte(uint16_t theMemoryAddress); 
#endif