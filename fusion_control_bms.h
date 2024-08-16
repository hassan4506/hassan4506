#ifndef FUSION_CONTROL_H_
#define FUSION_CONTROL_H_


#define PWM_FULL 		4095
#define OPT_FULL		125



void AirSwitchError (TimerHandle_t xTimer);
void PurgingError (TimerHandle_t xTimer);
void Bms_auto_start (TimerHandle_t xTimer);
void PurgeOff (TimerHandle_t xTimer);
void worker();
void starter();
void get_set_oil();
unsigned int get_Oil_value(unsigned int Air_val);
unsigned int get_Gas_value(unsigned int Air_val);
void get_set();
void system_manual();
void system_lfh();
void system_auto();
void motor_control();
bool vfd_position();
void vfd_control();


#endif