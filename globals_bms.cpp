#include <includes_bms.h>

Inputs_t d_input;
BMS_t bms;
FUSION_t fusion;
Indicators_t indicator;
Hmi_t hmi;
Motor_t motor;

portBASE_TYPE s1,s2,s3,s4,s5,s6,s7,s8;
SemaphoreHandle_t hmi_RW,fd_ON,flm_ON;
TimerHandle_t xTimer1,xTimer2,xTimer3,xTimer4;


double g_Setpoint; 
double g_Input;
double g_Output;

double w_Setpoint; 
double w_Input;
double w_Output;

PID GAS_PID(&g_Input, &g_Output, &g_Setpoint, 2, 5, 1, DIRECT);
PID PUMP_PID(&w_Input, &w_Output, &w_Setpoint, 2, 5, 1, DIRECT);

Anlg_t a_input;