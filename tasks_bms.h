#ifndef TASKS_BMS_H_
#define TASKS_BMS_H_
//#define DEBUG_EN


#define AVG_POINTS 		30

#define DI_1 	d_input.states[0]
#define DI_2 	d_input.states[1]
#define DI_3	d_input.states[2]
#define DI_4	d_input.states[3]
#define DI_5	d_input.states[4]
#define DI_6	d_input.states[5]
#define DI_7	d_input.states[6]
#define DI_8	d_input.states[7]
#define DI_9	d_input.states[8]
#define DI_10	d_input.states[9]
#define DI_11	d_input.states[10]
#define DI_12	d_input.states[11]

#define DO_1 	28
#define DO_2 	26
#define DO_3 	24
#define DO_4 	22
#define DO_5 	23
#define DO_6 	25
#define DO_7 	27
#define DO_8 	29
#define DO_9 	31
#define DO_10 	33
#define DO_11 	35
#define DO_12 	37
#define DO_13 	6
#define DO_14 	7
#define DO_15 	9


#define LED1 	38
#define LED2 	40
#define LED3 	42
#define LED4 	44

#define RED1 	LED1
#define YLW1 	LED2
#define GRN1 	LED3
#define YLW2 	LED4


#define AI_1 	a_input.smoothedAnalog[0]
#define AI_2 	a_input.smoothedAnalog[1]
#define AI_3 	a_input.smoothedAnalog[2]
#define AI_4	a_input.smoothedAnalog[3]
#define AI_5	a_input.smoothedAnalog[4]
#define AI_6	a_input.smoothedAnalog[5]
#define AI_7	a_input.smoothedAnalog[6]
#define AI_8	a_input.smoothedAnalog[7]
#define AI_9	a_input.smoothedAnalog[8]
#define AI_10	a_input.smoothedAnalog[9]
#define AI_11	a_input.smoothedAnalog[10]
#define AI_12	a_input.smoothedAnalog[11]


#define VFD_A 					AI_1
#define GFB_A					AI_2
#define O2_A  					AI_12

#define PRS_A 					AI_5
#define AUX2_A 					AI_6
#define FRO_A 					AI_7

#define FEED_WTR_A 				AI_9
#define STP_A 					AI_10
#define OIL_TEMP_A   			AI_11


#define AIR_LEVEL_CURR	 		map(VFD_A, 0, 4095, 0, hmi.Settings[VFD_RANGE])
#define O2LV					map(O2_A, 0, 4095, 0, hmi.Settings[SCL_02L])
#define PROCESS_CURR			map(PRS_A, 873, 4095, 0,hmi.Settings[SCL_SPRESURE])
#define GASS_CURR				map(GFB_A,hmi.Settings[LOW_RANGE],hmi.Settings[HIGH_RANGE],0,hmi.Settings[GAS_VALVE_RANGE])
#define FRO_CURR				map(FRO_A,hmi.Settings[OIL_LOW],hmi.Settings[OIL_HIGH],0,hmi.Settings[OFD_SCL])
#define PROCESS_P 				PROCESS_CURR
#define WTR_L_CURR 				map(AUX2_A, 950, 4095, 0,hmi.Settings[SCL_DRUML])
#define OIL_T_CURR 				map(OIL_TEMP_A,1092,2150,0,hmi.Settings[SCL_OIL_TEMP])
#define STACK_T_CURR 			map(STP_A, 1092, 2150, 0, hmi.Settings[SCL_STACKTEMP])
#define FW_T_CURR 				map(FEED_WTR_A, 1092, 2150, 0, hmi.Settings[SCL_FW])

#define AIR_MAX 				hmi.Settings[C8AIR]
#define AIR_MIN 				hmi.Settings[C1AIR]
#define AIR2_MAX 				hmi.Settings[C82AIR]
#define AIR2_MIN 				hmi.Settings[C12AIR]
#define GAS_MAX 				hmi.Settings[C8GAS]
#define GAS_MIN 				hmi.Settings[C1GAS]
#define OIL_MIN 				hmi.Settings[C1OIL]
#define OIL_MAX 				hmi.Settings[C8OIL]

#define AO_1 	6
#define AO_2	7
#define AO_3	8
#define AO_4	9


#define FLM_STATUS				DI_1
#define PRS_STATUS				!DI_2
#define GAS_STATUS				!DI_3
#define WTR_STATUS				!DI_4
#define AIR_STATUS				DI_5
#define COM_STATUS				DI_6
#define DUEL_SELECT				DI_7
#define HIGH_WATER 				DI_8
#define BYPASS_STATUS			DI_9



#define MT_F 					DO_1
#define MT_B 					DO_2
#define MAIN_V 					DO_3
#define VFD_R_O					DO_4
#define ALARM_R 				DO_5
#define SPARK					DO_6
#define PUMP1					DO_7 		
#define DUEL_S_R 				DO_8
#define HEAT1 					DO_9
#define HEAT2					DO_10
#define O2PURGE_R 				DO_11	
#define PURGE_R 				DO_12


#define GM_REF 					2
#define VFD_P 					13
#define VFD_W_P 				12
#define OIL_M_R                 12

#define VFD_FORWARD 				DO_14 //7
#define VFD_REVERSE					DO_15 //9
#define VFD_R 						DO_13 //6

#define FLM_ERR1_SET 			0x0001
#define FLM_ERR1_CLR			0xFFFE

#define FLM_ERR2_SET 			0x0002
#define FLM_ERR2_CLR			0xFFFD

#define FLM_ERR3_SET 			0x0004
#define FLM_ERR3_CLR			0xFFFB

#define FLM_ERR4_SET 			0x0008
#define FLM_ERR4_CLR			0xFFF7

#define PRS_ERR_SET				0x0010
#define PRS_ERR_CLR 			0xFFEF

#define GAS_ERR_SET 			0x0020
#define GAS_ERR_CLR 			0xFFDF

#define WTR_ERR_SET 			0x0040
#define WTR_ERR_CLR 			0xFFBF

#define WTH_ERR_SET 			0x0080
#define WTH_ERR_CLR 			0xFF7F

#define AIR_ERR1_SET 			0x0100
#define AIR_ERR1_CLR 			0xFEFF

#define AIR_ERR2_SET 			0x0200
#define AIR_ERR2_CLR 			0xFDFF



void blink (void* arg);
void hmi_read (void* arg);
void hmi_write (void* arg);
void bms_ops (void* arg);
void dig_read (void* arg);
void an_read (void* arg);
void burner_control (void * arg);
void pump_control();
void purge_control(void* arg);

#endif