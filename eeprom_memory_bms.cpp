#include "eeprom_memory_bms.h"


void eeprom_SaveByte(uint16_t theMemoryAddress, uint8_t u8Byte) 
{

    Wire.beginTransmission(DEVICEADDRESS);
    Wire.write( (theMemoryAddress >> 8) & 0xFF );
    Wire.write( (theMemoryAddress >> 0) & 0xFF );
    Wire.write(u8Byte);
    Wire.endTransmission();
    delay(5);

}

// ----------------------------------------------------------------
uint8_t eeprom_ReadByte(uint16_t theMemoryAddress) 
{
  uint8_t u8retVal = 0;
  Wire.beginTransmission(DEVICEADDRESS);
  Wire.write( (theMemoryAddress >> 8) & 0xFF );
  Wire.write( (theMemoryAddress >> 0) & 0xFF );
  Wire.endTransmission();
  delay(5);
  Wire.requestFrom(DEVICEADDRESS, 1);
  u8retVal = Wire.read();
  return u8retVal ;
}