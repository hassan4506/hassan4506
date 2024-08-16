#include "includes_bms.h"

void app_init(void)
{
	pinMode(DO_1,OUTPUT);
	pinMode(DO_2,OUTPUT);
	pinMode(DO_3,OUTPUT);
	pinMode(DO_4,OUTPUT);
	pinMode(DO_5,OUTPUT);
	pinMode(DO_6,OUTPUT);
	pinMode(DO_7,OUTPUT);
	pinMode(DO_8,OUTPUT);
	pinMode(DO_9,OUTPUT);
	pinMode(DO_10,OUTPUT);
	pinMode(DO_11,OUTPUT);
	pinMode(DO_12,OUTPUT);

	pinMode(DO_13,OUTPUT);
	pinMode(DO_14,OUTPUT);
	pinMode(DO_15,OUTPUT);
	pinMode(LED1,OUTPUT);
	pinMode(LED2,OUTPUT);
	pinMode(LED3,OUTPUT);
	pinMode(LED4,OUTPUT);
	pinMode(VFD_FORWARD,OUTPUT);
	pinMode(VFD_REVERSE,OUTPUT);

	digitalWrite(DO_1,LOW);
	digitalWrite(DO_2,LOW);
	digitalWrite(DO_3,LOW);
	digitalWrite(DO_4,LOW);
	digitalWrite(DO_5,LOW);
	digitalWrite(DO_6,LOW);
	digitalWrite(DO_7,LOW);
	digitalWrite(DO_8,LOW);
	digitalWrite(DO_9,LOW);
	digitalWrite(DO_10,LOW);
	digitalWrite(DO_11,LOW);
	digitalWrite(DO_12,LOW);
	digitalWrite(DO_13,LOW);
	digitalWrite(DO_14,LOW);
	digitalWrite(DO_15,LOW);
	digitalWrite(LED1,LOW);
	digitalWrite(LED2,LOW);
	digitalWrite(LED3,LOW);
	digitalWrite(LED4,LOW);

	digitalWrite(VFD_FORWARD,LOW);
	digitalWrite(VFD_REVERSE,LOW);


	analogWrite(GM_REF,PWM_FULL);
	analogWrite(OIL_M_R,0);

	analogWrite(VFD_P,0);


	pinMode(49,INPUT);
	if(!digitalRead(49))
	{
		bms.DUEL = 1;
		digitalWrite(DUEL_S_R,HIGH);
	}
	else
	{
		bms.DUEL = 0;
		digitalWrite(DUEL_S_R,LOW);
	}
	Setting_Load();
	hmi.Settings[BMS_ENABLE] = 0;
	print_data(hmi.Settings, TOTAL_TAGS, 6);
	while(!upload_settings())
	{
		Debug.println("Could not upload!");
		delay(1000);
	}
	Debug.println("uploaded!");

	hmi_RW  = xSemaphoreCreateMutex();
	fd_ON	= xSemaphoreCreateMutex();
	flm_ON 	= xSemaphoreCreateMutex();
	bms.state = -1;
	if(hmi_RW == NULL || fd_ON == NULL || flm_ON == NULL)
    {
        Serial.println("Semaphore Creation problem");
    }
    else
    {
        Serial.println("Semaphore Created");
    }

    xTimer1 = xTimerCreate("Timer1",(100L * configTICK_RATE_HZ),pdFALSE,(void *) 0, AirSwitchError);
    xTimer2 = xTimerCreate("Timer2",(100L * configTICK_RATE_HZ),pdFALSE,(void *) 0, PurgingError);
    xTimer3 = xTimerCreate("Timer3",(30L * configTICK_RATE_HZ),pdTRUE,(void *) 0, Bms_auto_start);
    xTimer4 = xTimerCreate("Timer4",(10L * configTICK_RATE_HZ),pdFALSE,(void *) 0, PurgeOff);
 

	s1 = xTaskCreate(blink, NULL, configMINIMAL_STACK_SIZE, NULL, 2, NULL);
	s2 = xTaskCreate(hmi_read, NULL, configMINIMAL_STACK_SIZE, NULL, 2, NULL);
	s3 = xTaskCreate(hmi_write, NULL, configMINIMAL_STACK_SIZE, NULL, 2, NULL);
	s4 = xTaskCreate(bms_ops, NULL, configMINIMAL_STACK_SIZE, NULL, 3, NULL);
	s5 = xTaskCreate(dig_read, NULL, configMINIMAL_STACK_SIZE, NULL, 2, NULL);
	s6 = xTaskCreate(an_read, NULL, configMINIMAL_STACK_SIZE, NULL, 2, NULL);
	s7 = xTaskCreate(burner_control, NULL, configMINIMAL_STACK_SIZE, NULL, 4, NULL);
	s8 = xTaskCreate(purge_control, NULL, configMINIMAL_STACK_SIZE, NULL, 1, NULL);

	if (s1 != pdPASS || s2 != pdPASS || s3 != pdPASS || s4 != pdPASS || s5 != pdPASS || s6 != pdPASS || s7 != pdPASS || s8 != pdPASS) 
	{
    	Serial.println("Task Creation problem");
    	while(1);
  	}
  	g_Input = 0;
	g_Setpoint = 0;
	g_Output = 0;
	w_Input = 0;
	w_Setpoint = 0;
	w_Output = 0;
	
	GAS_PID.SetMode(AUTOMATIC);
	GAS_PID.SetSampleTime(1000);

	PUMP_PID.SetMode(AUTOMATIC);
	PUMP_PID.SetSampleTime(1000);
	PUMP_PID.SetOutputLimits(0.0,100.0);
}
