#include "includes_bms.h"

uint16_t calc_crc(uint8_t *buf, uint8_t len)
{
  uint16_t error = 0xFFFF;
  int pos = 0;
  int i = 0;
  for (pos = 0; pos < len; pos++) 
  {
      error ^= (uint16_t)buf[pos];          // XOR byte into least sig. byte of crc
 
      for (i = 8; i != 0; i--) 
    {                     // Loop over each bit
          if ((error & 0x0001) != 0) 
      {                     // If the LSB is set
            error >>= 1;                    // Shift right and XOR 0xA001
            error ^= 0xA001;
          }
       else                               // Else LSB is not set
          error >>= 1;                      // Just shift right
      }
    }
  // Note, this number has low and high bytes swapped, so use it accordingly (or swap bytes)
  return error;  
}

double interpolate( double x,double x0, double x1, double y0, double y1)
{
   double a = (y1 - y0) / (x1 - x0);
   double b = -a*x0 + y0;
   double y = a * x + b;
   return y;
}

void fill_display(void)
{
  if(AIR_LEVEL_CURR > 0)
    hmi.Display[AIR_DISPLAY]      =   AIR_LEVEL_CURR;
  else
    hmi.Display[AIR_DISPLAY]      =   0;

  if(bms.DUEL)
  {
    hmi.Display[GAS_DISPLAY]      =   0;
    if(FRO_CURR > 0)
      hmi.Display[OIL_DISPLAY]    =   FRO_CURR;
    else
      hmi.Display[OIL_DISPLAY]    =   0;
  }
  else
  {
    hmi.Display[OIL_DISPLAY]    =   0;
    if(GASS_CURR > 0)
      hmi.Display[GAS_DISPLAY]    =   GASS_CURR;
    else
      hmi.Display[GAS_DISPLAY]    =   0;
  }

  if(OIL_T_CURR > 0)
    hmi.Display[OIL_TEMP]        =   OIL_T_CURR;
  else
    hmi.Display[OIL_TEMP]        =   0;

  if(STACK_T_CURR > 0)
    hmi.Display[STACK_TEMP]      =  STACK_T_CURR;
  else
    hmi.Display[STACK_TEMP]      = 0;
  if(FW_T_CURR > 0)      
    hmi.Display[FW_TEMP]         = FW_T_CURR;
  else
    hmi.Display[FW_TEMP]          = 0;
  
  
  hmi.Display[O2_LEVEL]           =   get_o2();
  hmi.Display[RAW_MV]             =   O2LV;
  
  if(WTR_L_CURR > 0)
  hmi.Display[DRUM_LEVEL]         = (uint16_t)WTR_L_CURR;
  else
	  hmi.Display[DRUM_LEVEL] 		= 0;
  
  if(PROCESS_CURR > 0)
    hmi.Display[PROCESS_POINT]    =   PROCESS_CURR;
  else
    hmi.Display[PROCESS_POINT]    =   0;
  
  hmi.Display[SET_POINT_DISP]     =   hmi.Settings[SET_POINT];
  hmi.Display[PID_VALUE]          =   fusion.desired_air_level;

  if(indicator.FD_FAN_ON)
  {
    hmi.Display[FD_FAN]++;
    if(hmi.Display[FD_FAN]==6)
      hmi.Display[FD_FAN] = 1;
  }
  else
    hmi.Display[FD_FAN] = 0;

  if(indicator.PILOT_ON)
  {
    hmi.Display[PILOT] = 1;
  }
  else
    hmi.Display[PILOT] = 0;
  if(indicator.MAIN_VALVE_ON)
  {
    hmi.Display[MAIN_VALVE] = 1;
  }
  else
    hmi.Display[MAIN_VALVE] = 0;

  hmi.Display[OIL_GAS]     = (uint16_t)DUEL_SELECT;
  hmi.Display[CH3_RAW]     = (uint16_t)map(AI_3,0,4095,0,3000);
  hmi.Display[GAS_PRS]     = (uint16_t)GAS_STATUS;
  hmi.Display[STM_PRS]     = (uint16_t)PRS_STATUS;
  hmi.Display[WTR_LVL]     = (uint16_t)WTR_STATUS;
  hmi.Display[FLAME_IND]   = (uint16_t)FLM_STATUS;
  hmi.Display[AIR_SW_IND]  = (uint16_t)AIR_STATUS; 
  hmi.Display[EXT12]     = (uint16_t)DUEL_SELECT;
}



void modulate_oil_temp()
{
  if(bms.DUEL)
  {
    if(OIL_T_CURR > 0 && OIL_T_CURR < hmi.Settings[OIL_TEMP_SP])
    {
      digitalWrite(HEAT2,HIGH);
      digitalWrite(HEAT1,HIGH);
    }
    else if(OIL_T_CURR > (hmi.Settings[OIL_TEMP_SP]+5) && OIL_T_CURR < (hmi.Settings[OIL_TEMP_SP]+10))
    {
      digitalWrite(HEAT2,HIGH);
      digitalWrite(HEAT1,LOW);
    }
    else if(OIL_T_CURR > (hmi.Settings[OIL_TEMP_SP]+15))
    {
      digitalWrite(HEAT1,LOW);
      digitalWrite(HEAT2,LOW);
    }
  }
  else
  {
    digitalWrite(HEAT1,LOW);
    digitalWrite(HEAT2,LOW);
  }
}


uint16_t get_o2()
{
  static double mv;
  static uint16_t o2_val;
  mv = interpolate((double)O2_A, 0.0, 4095.0, 0.0, (double)hmi.Settings[SCL_02L]);

  if((uint16_t)mv < hmi.Settings[O2_1])
  {
    o2_val = hmi.Settings[O2_8];
  }
  else if((uint16_t)mv > hmi.Settings[O2_7])
  {
    o2_val = hmi.Settings[O2_14];
  }
  else if((uint16_t)mv >= hmi.Settings[O2_1] && (uint16_t)mv < hmi.Settings[O2_2])
  {
    o2_val = (uint16_t)(interpolate(mv,hmi.Settings[O2_1], hmi.Settings[O2_2], hmi.Settings[O2_8], hmi.Settings[O2_9]));
  }
  else if((uint16_t)mv >= hmi.Settings[O2_2] && (uint16_t)mv < hmi.Settings[O2_3])
  {
    o2_val = (uint16_t)(interpolate(mv,hmi.Settings[O2_2], hmi.Settings[O2_3], hmi.Settings[O2_9], hmi.Settings[O2_10]));
  }
  else if((uint16_t)mv >= hmi.Settings[O2_3] && (uint16_t)mv < hmi.Settings[O2_4])
  {
    o2_val = (uint16_t)(interpolate(mv,hmi.Settings[O2_3], hmi.Settings[O2_4], hmi.Settings[O2_10], hmi.Settings[O2_11]));
  }
  else if((uint16_t)mv >= hmi.Settings[O2_4] && (uint16_t)mv < hmi.Settings[O2_5])
  {
    o2_val = (uint16_t)(interpolate(mv,hmi.Settings[O2_4], hmi.Settings[O2_5], hmi.Settings[O2_11], hmi.Settings[O2_12]));
  }
  else if((uint16_t)mv >= hmi.Settings[O2_5] && (uint16_t)mv < hmi.Settings[O2_6])
  {
    o2_val = (uint16_t)(interpolate(mv,hmi.Settings[O2_5], hmi.Settings[O2_6], hmi.Settings[O2_12], hmi.Settings[O2_13]));
  }
  else if((uint16_t)mv >= hmi.Settings[O2_6] && (uint16_t)mv < hmi.Settings[O2_7])
  {
    o2_val = (uint16_t)(interpolate(mv,hmi.Settings[O2_6], hmi.Settings[O2_7], hmi.Settings[O2_13], hmi.Settings[O2_14]));
  }
  return o2_val;
}

void Setting_Save()
{
  static uint16_t add = 10;
  static uint16_t ind;
  eeprom_SaveByte(2,0);
  for(ind = 0; ind < TOTAL_TAGS-1; ind++)
  {
    eeprom_SaveByte(add++,highByte(hmi.Settings[ind]));
    eeprom_SaveByte(add++,lowByte(hmi.Settings[ind]));
  }
}

void Setting_Load()
{
  int add = 10;
  if(eeprom_ReadByte(2) == 0)
  {
    for(int ind = 0; ind < TOTAL_TAGS-1; ind++)
    {
      hmi.Settings[ind] = (int)eeprom_ReadByte(add++);
      hmi.Settings[ind] *= 256; 
      hmi.Settings[ind] += (int)eeprom_ReadByte(add++);
    }
  } 
  else
  {
    Debug.println("Loading Default !");
    Debug.println(eeprom_ReadByte(2));
    load_default();
  }
}

void load_default()
{
  hmi.Settings [0] = 26;
  hmi.Settings [1] = 11;
  hmi.Settings [2] = 35;
  hmi.Settings [3] = 30;
  hmi.Settings [4] = 16;
  hmi.Settings [5] = 42;
  hmi.Settings [6] = 40;
  hmi.Settings [7] = 40;
  hmi.Settings [8] = 21;
  hmi.Settings [9] = 49;
  hmi.Settings [10] = 46;
  hmi.Settings [11] = 48;
  hmi.Settings [12] = 26;
  hmi.Settings [13] = 56; 
  hmi.Settings [14] = 56;
  hmi.Settings [15] = 56;
  hmi.Settings [16] = 32;
  hmi.Settings [17] = 63;
  hmi.Settings [18] = 64;
  hmi.Settings [19] = 65;
  hmi.Settings [20] = 39; 
  hmi.Settings [21] = 74;
  hmi.Settings [22] = 75;
  hmi.Settings [23] = 77;
  hmi.Settings [24] = 45;
  hmi.Settings [25] = 81;
  hmi.Settings [26] = 82;
  hmi.Settings [27] = 85; 
  hmi.Settings [28] = 50;
  hmi.Settings [29] = 86;
  hmi.Settings [30] = 87;
  hmi.Settings [31] = 92;
  hmi.Settings [32] = 56;
  hmi.Settings [33] = 97;
  hmi.Settings [34] = 98; 
  hmi.Settings [35] = 80;
  hmi.Settings [36] = 300;
  hmi.Settings [37] = 22;
  hmi.Settings [38] = 300;
  hmi.Settings [39] = 600;
  hmi.Settings [40] = 10;
  hmi.Settings [41] = 100;  
  hmi.Settings [42] = 100;
  hmi.Settings [43] = 100;
  hmi.Settings [44] = 100;
  hmi.Settings [45] = 100;
  hmi.Settings [46] = 0;
  hmi.Settings [47] = 0;
  hmi.Settings [48] = 0;
  hmi.Settings [49] = 0;
  hmi.Settings [50] = 2600;
  hmi.Settings [51] = 200;  
  hmi.Settings [52] = 250;
  hmi.Settings [53] = 250;
  hmi.Settings [54] = 250;
  hmi.Settings [55] = 200;
  hmi.Settings [56] = 2600;
  hmi.Settings [57] = 0;
  hmi.Settings [58] = 0;
  hmi.Settings [59] = 0;
  hmi.Settings [60] = 0;
  hmi.Settings [61] = 0;
  hmi.Settings [62] = 0;
  hmi.Settings [63] = 0;  
  hmi.Settings [64] = 0;
  hmi.Settings [65] = 0;
  hmi.Settings [66] = 0;
  hmi.Settings [67] = 0;
  hmi.Settings [68] = 0;
  hmi.Settings [69] = 0;
  hmi.Settings [70] = 0;
  hmi.Settings [71] = 120; 
  hmi.Settings [72] = 50;
  hmi.Settings [73] = 0;
  hmi.Settings [74] = 0;
  hmi.Settings [75] = 0;
  hmi.Settings [76] = 30;
  hmi.Settings [77] = 120;
  hmi.Settings [78] = 0;
  hmi.Settings [79] = 0;
  hmi.Settings [80] = 0;
}

bool upload_settings()
{
  uint8_t update[(TOTAL_TAGS*2)+7],junk;
  uint16_t crc_d;
  while(Screen.available())
  {
    junk=Screen.read();
  }
  update[0]=1;
  update[1]=16;
  update[2]=0;
  update[3]=8;
  update[4]=0;
  update[5]= TOTAL_TAGS;
  update[6]= TOTAL_TAGS*2;
  uint8_t d_ind;
  d_ind = 7;
  for(uint16_t x=0; x<TOTAL_TAGS; x++)
  {
    update[d_ind]=highByte(hmi.Settings[x]);
    update[d_ind+1]=lowByte(hmi.Settings[x]);
    d_ind+=2;
  }
  crc_d = calc_crc(update, (TOTAL_TAGS*2)+7);
  d_ind = 0;
  for(uint16_t x=0; x<(TOTAL_TAGS*2)+7; x++)
  {
    Screen.write(update[x]);
  }
  Screen.write(lowByte(crc_d));
  Screen.write(highByte(crc_d));
  delay(50);
  uint32_t now = millis();
  uint8_t timeout = 0;
  while (!Screen.available())
  {
    if((millis()- now)>=3000)
    {
      Debug.println("Timeout");
      timeout = 1;
      break;
    }
  }
  if(timeout)
  {
    return 0;
  }
  else
    return 1;
}

void print_data(uint16_t* data, int len, int br)
  {
    for(int i = 0; i<len;i++)
    {
       Debug.print(data[i]);
       Debug.print(" ");
       if(i%br==br-1)
         Serial.println(" ");
    } 
  }
