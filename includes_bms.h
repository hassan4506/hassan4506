#ifndef INCLUDES_BMS_H
#define INCLUDES_BMS_H

#define Screen Serial2
#define Debug Serial


#include "Arduino.h"
#include <FreeRTOS_ARM.h>

#include "eeprom_memory_bms.h"
#include "hmitags_bms.h"
#include "PID_bms.h"
#include "globals_bms.h"
#include "fusion_control_bms.h"

#include "utilfunctions_bms.h"
#include "tasks_bms.h"
#include "inits_bms.h"


#endif