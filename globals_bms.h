#ifndef GLOBALS_BMS_H_
#define GLOBALS_BMS_H_

struct Inputs_t{
	bool states[12];
	bool p_states[12];
};

struct Anlg_t{
	uint16_t smoothedAnalog[12];
	uint32_t RawAnalog[12];
};

struct Hmi_t{
   uint16_t Settings[TOTAL_TAGS];
   uint16_t Display[DISPLAY_TAGS];
};

struct Indicators_t{
	bool purging;
	bool FD_FAN_ON;
	bool previos_fd_state;
	bool PILOT_ON;
	bool previos_pilot_state;
	bool MAIN_VALVE_ON;
	bool previos_main_state;
	bool COMM_ON;
	bool previos_com_state; 
	bool motor_done;
	bool first_start;
	bool vfd_done;
};

struct Motor_t{
	int position;
	int current_position;
	int previous_position;
	int current_air_level;
	int oil_vfd;
	unsigned int feedback_check;
};

struct BMS_t{
	int8_t state;
	uint16_t endis;
	uint16_t reset;
	uint8_t start;
	uint8_t DUEL;
};
struct FUSION_t{
	int8_t state;
	uint16_t mod_motor_position;
	uint16_t desired_air_level;
	uint16_t current_duty;
	uint16_t wait;
	bool O2_Triming;
	bool pid_on;
	int o2_level_increasing;
	int o2_level_decreasing;
};
extern BMS_t bms;
extern FUSION_t fusion;
extern Indicators_t indicator;
extern Motor_t motor;


extern portBASE_TYPE s1,s2,s3,s4,s5,s6,s7,s8;
extern SemaphoreHandle_t hmi_RW,fd_ON,flm_ON;
extern TimerHandle_t xTimer1,xTimer2,xTimer3,xTimer4;



extern Inputs_t d_input;
extern Hmi_t hmi;
extern Anlg_t a_input;



extern double g_Setpoint; 
extern double g_Input;
extern double g_Output;

extern double w_Setpoint; 
extern double w_Input;
extern double w_Output;

extern PID GAS_PID;
extern PID PUMP_PID;

#endif