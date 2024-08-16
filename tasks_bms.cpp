#include "includes_bms.h"

void blink (void* arg)
{
	pinMode(13, OUTPUT);
	while(1)
	{
		digitalWrite(13, LOW);
		digitalWrite(YLW1, LOW);
		vTaskDelay((1000L * configTICK_RATE_HZ) / 1000L);
		digitalWrite(13, HIGH);
		digitalWrite(YLW1, HIGH);
		vTaskDelay((1000L * configTICK_RATE_HZ) / 1000L);
		modulate_oil_temp();
		pump_control();
	}
}
void purge_control(void* arg)
{
	while(1)
	{
		digitalWrite(O2PURGE_R,HIGH);
		hmi.Display[PURGE_IND] = 1;
		vTaskDelay(2L * configTICK_RATE_HZ);
		digitalWrite(O2PURGE_R,LOW);
		hmi.Display[PURGE_IND] = 0;
		vTaskDelay(120L * configTICK_RATE_HZ);
	}
}
void hmi_read (void* arg)
{
	uint16_t start_address 	= ADDRESS_SETTINS;
	uint16_t n_tags 		= TOTAL_TAGS;
	static uint8_t request[6] = {1, 03, highByte(start_address),lowByte(start_address),highByte(n_tags),lowByte(n_tags)};
  	static uint16_t crc_x = calc_crc(request, 6);
  	static uint32_t now;
  	static uint16_t timeout;
  	static uint16_t upcheck;
  	static uint16_t resetcheck;
  	static uint16_t purgecheck;
	while(1)
	{
		if(hmi_RW != NULL )
	    {
	        if( xSemaphoreTake( hmi_RW, portTICK_PERIOD_MS*500 ) == pdTRUE )
	        {
	        	#ifdef DEBUG_EN
	            	Debug.println("Got the semaphore in read !");
	            #endif
	            uint8_t junk; 
	            /* Check if there is something in buffer then discard it */
	            	while(Screen.available()!=0)
  					{
    					junk=Screen.read();
  					}
  				/* Buffer is empty now */

  				upcheck = hmi.Settings[updateSave];
  				resetcheck = hmi.Settings[BMS_RESET];
  				purgecheck = hmi.Settings[O2PURGE];
  				Screen.write(0x01);
				Screen.write(0x03);
				Screen.write(highByte(start_address));
				Screen.write(lowByte(start_address));
				Screen.write(highByte(n_tags));
				Screen.write(lowByte(n_tags));
				Screen.write(lowByte(crc_x));
				Screen.write(highByte(crc_x));

				vTaskDelay((5L * configTICK_RATE_HZ) / 1000L); // 5 ms delay
				
				now = millis();
				while (!Screen.available())
				{
				  if((millis()- now)>=3000)
				  {
				  	#ifdef DEBUG_EN
					  Debug.println("Timeout");
					#endif
					  digitalWrite(RED1, HIGH);
					  timeout = 1;
					  break;
					  
				  }
				  else
				  {
				  	taskYIELD();

				  }
				}
				if(timeout == 0)
				{
					digitalWrite(RED1, LOW);
					taskENTER_CRITICAL();
					static uint16_t i =0;
					static uint8_t* response;
					response = new uint8_t [(n_tags*2)+5];
					while (Screen.available())
					{
						response[i]=Screen.read();
						delayMicroseconds(500);
						i++;
					}
					i=0;
		            taskEXIT_CRITICAL();
					if(( calc_crc(response, (n_tags*2)+5))==0)
					{
						for(uint16_t x=3;x<((n_tags*2)+3);x+=2)
						{
							hmi.Settings[i] = (response[x] << 8) | response[x+1] ;
							i++;
						}
						delete[] response;
						}
					else
					{
						delete[] response;
					}
				}
				xSemaphoreGive(hmi_RW);
				timeout = 0;
				if(hmi.Settings[updateSave] != upcheck)
				{
					//taskENTER_CRITICAL();
					vTaskSuspendAll ();
					Debug.println("Saving Data !");
					Setting_Save();
					print_data(hmi.Settings, TOTAL_TAGS, 6);
					Debug.println(".......  Saved  .......");
					xTaskResumeAll ();
					//taskEXIT_CRITICAL();
				}
				if(resetcheck != hmi.Settings[BMS_RESET])
					digitalWrite(ALARM_R,LOW);
				if(purgecheck != hmi.Settings[O2PURGE])
				{
					digitalWrite(O2PURGE_R,HIGH);
					xTimerStart(xTimer4,0);
				}
	            vTaskDelay((500L * configTICK_RATE_HZ) / 1000L);

	        }
	    }
	}
}
void hmi_write (void* arg)
{
	static uint8_t update[(DISPLAY_TAGS*2)+7];
	static uint16_t crc_d;
	while(1)
	{
		fill_display();
		if(hmi_RW != NULL )
	    {
	        if( xSemaphoreTake( hmi_RW, portTICK_PERIOD_MS*500 ) == pdTRUE )
	        {
	        	#ifdef DEBUG_EN
	        		Debug.println("Got the semaphore in write !");
	        	#endif
	        	uint8_t junk;
				while(Screen.available())
				{
					junk=Screen.read();
				}
				if(hmi.Display[CON_CHECK] == 0)
					hmi.Display[CON_CHECK] = 1;
				else
					hmi.Display[CON_CHECK] = 0;
				update[0]=1;
				update[1]=16;
				update[2]=0;
				update[3]=100;
				update[4]=0;
				update[5]= DISPLAY_TAGS;
				update[6]= DISPLAY_TAGS*2;

				static uint8_t d_ind;
				d_ind = 7;
				taskENTER_CRITICAL();
				for(uint16_t x=0; x<DISPLAY_TAGS; x++)
				{
					update[d_ind]=highByte(hmi.Display[x]);
					update[d_ind+1]=lowByte(hmi.Display[x]);
					d_ind+=2;
				}
				crc_d = calc_crc(update, (DISPLAY_TAGS*2)+7);
				d_ind = 0;
				for(uint16_t x=0; x<(DISPLAY_TAGS*2)+7; x++)
				{
					Screen.write(update[x]);
				}
				Screen.write(lowByte(crc_d));
				Screen.write(highByte(crc_d));
				taskEXIT_CRITICAL();
	            xSemaphoreGive(hmi_RW);
	            vTaskDelay((500L * configTICK_RATE_HZ) / 1000L);

	        }
	    }
	}
}

void dig_read (void* arg)
{
	static uint8_t di;
	static uint8_t din[12]={46,48,50,52,53,51,49,47,45,43,41,39};
	di = 0;
	for(di=0;di<12;di++)
		pinMode(din[di],INPUT);
	while(1)
	{
		di=0;
		for(di = 0;di<12;di++)
		{
			if(!digitalRead(din[di]))
			{
				if(d_input.p_states[di]==1)
				{
					d_input.states[di] = 1;
				}
				d_input.p_states[di] = 1;
			}
			else
			{
				if(d_input.p_states[di]==0)
				{
					d_input.states[di] = 0;
				}
				d_input.p_states[di] = 0;
			}
		}
		vTaskDelay((500L * configTICK_RATE_HZ) / 1000L);
	}

}

void an_read (void* arg)
{
	static uint8_t ai;
	while(1)
	{
		ai = 0;
		motor_control();
		for(ai=0;ai<AVG_POINTS;ai++)
		{
			a_input.RawAnalog[0] 	+= analogRead(A0);
			a_input.RawAnalog[1] 	+= analogRead(A1);
			a_input.RawAnalog[2] 	+= analogRead(A2);
			a_input.RawAnalog[3] 	+= analogRead(A3);
			a_input.RawAnalog[4] 	+= analogRead(A4);
			a_input.RawAnalog[5] 	+= analogRead(A5);
			a_input.RawAnalog[6] 	+= analogRead(A6);
			a_input.RawAnalog[7] 	+= analogRead(A7);
			a_input.RawAnalog[8] 	+= analogRead(A8);
			a_input.RawAnalog[9] 	+= analogRead(A9);
			a_input.RawAnalog[10] 	+= analogRead(A10);
			a_input.RawAnalog[11] 	+= analogRead(A11);
		}
		a_input.smoothedAnalog[0] 	= (uint16_t)(a_input.RawAnalog[0]/AVG_POINTS);
		a_input.smoothedAnalog[1] 	= (uint16_t)(a_input.RawAnalog[1]/AVG_POINTS);
		a_input.smoothedAnalog[2] 	= (uint16_t)(a_input.RawAnalog[2]/AVG_POINTS);
		a_input.smoothedAnalog[3] 	= (uint16_t)(a_input.RawAnalog[3]/AVG_POINTS);
		a_input.smoothedAnalog[4] 	= (uint16_t)(a_input.RawAnalog[4]/AVG_POINTS);
		a_input.smoothedAnalog[5] 	= (uint16_t)(a_input.RawAnalog[5]/AVG_POINTS);
		a_input.smoothedAnalog[6] 	= (uint16_t)(a_input.RawAnalog[6]/AVG_POINTS);
		a_input.smoothedAnalog[7] 	= (uint16_t)(a_input.RawAnalog[7]/AVG_POINTS);
		a_input.smoothedAnalog[8] 	= (uint16_t)(a_input.RawAnalog[8]/AVG_POINTS);
		a_input.smoothedAnalog[9] 	= (uint16_t)(a_input.RawAnalog[9]/AVG_POINTS);
		a_input.smoothedAnalog[10] 	= (uint16_t)(a_input.RawAnalog[10]/AVG_POINTS);
		a_input.smoothedAnalog[11] 	= (uint16_t)(a_input.RawAnalog[11]/AVG_POINTS);
		
		a_input.RawAnalog[0] 	= 0;
		a_input.RawAnalog[1] 	= 0;
		a_input.RawAnalog[2] 	= 0;
		a_input.RawAnalog[3] 	= 0;
		a_input.RawAnalog[4] 	= 0;
		a_input.RawAnalog[5] 	= 0;
		a_input.RawAnalog[6] 	= 0;
		a_input.RawAnalog[7] 	= 0;
		a_input.RawAnalog[8] 	= 0;
		a_input.RawAnalog[9] 	= 0;
		a_input.RawAnalog[10] 	= 0;
		a_input.RawAnalog[11] 	= 0;
		vTaskDelay((100L * configTICK_RATE_HZ) / 1000L);
	}
}

void bms_ops (void* arg)
{
	bms.start = 1;
	pinMode(MAIN_V,OUTPUT);
	pinMode(SPARK,OUTPUT);
	pinMode(ALARM_R,OUTPUT);
	vTaskDelay((5000L * configTICK_RATE_HZ) / 1000L);
	xTimerStart( xTimer3,0);
	while(1)
	{
		if(hmi.Settings[BMS_ENABLE] && bms.start == 1)
		{
			if(bms.state==-1)
			{
				Debug.println("state -1");
				fusion.state = -1;
				indicator.FD_FAN_ON = 0;
				indicator.PILOT_ON = 0;
				indicator.MAIN_VALVE_ON = 0;
				digitalWrite(SPARK,LOW);
				digitalWrite(MAIN_V,LOW);


				if(FLM_STATUS)
				{
					hmi.Display[ERRORS] |= FLM_ERR1_SET;
				}
				else
				{
					hmi.Display[ERRORS] &= FLM_ERR1_CLR;
				}
				if(PRS_STATUS)
				{
					hmi.Display[ERRORS] |= PRS_ERR_SET;
				}
				else
				{
					hmi.Display[ERRORS] &= PRS_ERR_CLR;
				}
				if(GAS_STATUS)
				{
					hmi.Display[ERRORS] |= GAS_ERR_SET;
				}
				else
				{
					hmi.Display[ERRORS] &= GAS_ERR_CLR;
				}
				if(WTR_STATUS)
				{
					hmi.Display[ERRORS] |= WTR_ERR_SET;
				}
				else
				{
					hmi.Display[ERRORS] &= WTR_ERR_CLR;
				}
				if(HIGH_WATER)
				{
					hmi.Display[ERRORS] |= WTH_ERR_SET;
				}
				else
				{
					hmi.Display[ERRORS] &= WTH_ERR_CLR;
				}
				if(AIR_STATUS)
				{
					hmi.Display[ERRORS] |= AIR_ERR1_SET;
				}
				else
				{
					hmi.Display[ERRORS] &= AIR_ERR1_CLR;
				}
				
				if(AIR_STATUS == 0 && FLM_STATUS == 0 && PRS_STATUS == 0 && GAS_STATUS == 0 && WTR_STATUS == 0 && HIGH_WATER == 0)
				{
					bms.state = 0;
				}
				else
				{
					Debug.println("BMS Error\n");
					bms.state = -1;
					bms.start = 0;
				}
			}
			else if(bms.state == 0)
			{
				Debug.println("state 0");
				hmi.Display[BMS_EXT1_ERR] = 1;
				indicator.FD_FAN_ON = 1;
				bms.state = 1;
				digitalWrite(ALARM_R,LOW);
				xTimerStart( xTimer1,0);
				hmi.Display[ERRORS] = 0;
			}
			else if(bms.state == 1 )
			{
				Debug.println("state 1");
				indicator.FD_FAN_ON = 1;
				if(FLM_STATUS || PRS_STATUS || GAS_STATUS || WTR_STATUS )
				{
					if(FLM_STATUS)
					{
						hmi.Display[ERRORS] |= FLM_ERR1_SET;
						digitalWrite(ALARM_R,HIGH);
					}
					if(WTR_STATUS)
					{
						hmi.Display[ERRORS] |= WTR_ERR_SET;
						digitalWrite(ALARM_R,HIGH);
					}
					if(PRS_STATUS)
					{
						hmi.Display[ERRORS] |= PRS_ERR_SET;
					}
					if(GAS_STATUS)
					{
						hmi.Display[ERRORS] |= GAS_ERR_SET;
						digitalWrite(ALARM_R,HIGH);
					}
					indicator.FD_FAN_ON = 0;
					indicator.PILOT_ON = 0;
					indicator.MAIN_VALVE_ON = 0;
					bms.start = 0;
				}
				else
				{
					if(AIR_STATUS)
					{
						xTimerStop( xTimer1, 0 );
						Debug.println("AIR Switch OK");
						hmi.Display[BMS_EXT1_ERR] = 2;
						xTimerStart( xTimer2,0);
						bms.state = 2;
					}
				}
			}
			else if(bms.state == 2 )
			{
				Debug.println("state 2");
				if(FLM_STATUS || PRS_STATUS || GAS_STATUS || WTR_STATUS || AIR_STATUS==0)
				{
					if(FLM_STATUS)
					{
						hmi.Display[ERRORS] |= FLM_ERR1_SET;
						digitalWrite(ALARM_R,HIGH);
					}
					if(WTR_STATUS)
					{
						hmi.Display[ERRORS] |= WTR_ERR_SET;
						digitalWrite(ALARM_R,HIGH);
					}
					if(PRS_STATUS)
					{
						hmi.Display[ERRORS] |= PRS_ERR_SET;
					}
					if(GAS_STATUS)
					{
						hmi.Display[ERRORS] |= GAS_ERR_SET;
						digitalWrite(ALARM_R,HIGH);
					}
					if(!AIR_STATUS)
					{
						hmi.Display[ERRORS] |= AIR_ERR2_SET;
						digitalWrite(ALARM_R,HIGH);
					}
					indicator.FD_FAN_ON = 0;
					indicator.PILOT_ON = 0;
					indicator.MAIN_VALVE_ON = 0;
					bms.state = -1;
					bms.start = 0;
				}
				else
				{
					if(indicator.purging==1)
					{
						xTimerStop( xTimer2, 0 );
						Debug.println("Purging Done");
						hmi.Display[BMS_EXT1_ERR] = 3;
						bms.state = 3;
					}
				}
			}
			else if(bms.state==3)
			{
				if(PRS_STATUS || GAS_STATUS || WTR_STATUS || AIR_STATUS == 0)
				{
					if(WTR_STATUS)
					{
						hmi.Display[ERRORS] |= WTR_ERR_SET;
						digitalWrite(ALARM_R,HIGH);
					}
					if(PRS_STATUS)
					{
						hmi.Display[ERRORS] |= PRS_ERR_SET;
					}
					if(GAS_STATUS)
					{
						hmi.Display[ERRORS] |= GAS_ERR_SET;
						digitalWrite(ALARM_R,HIGH);
					}
					if(!AIR_STATUS)
					{
						hmi.Display[ERRORS] |= AIR_ERR2_SET;
						digitalWrite(ALARM_R,HIGH);
					}
					indicator.FD_FAN_ON = 0;
					indicator.PILOT_ON = 0;
					indicator.MAIN_VALVE_ON = 0;
					bms.state = -1;
					bms.start = 0;
				}
				else
				{
					vTaskDelay((3000L * configTICK_RATE_HZ) / 1000L);
					hmi.Display[BMS_EXT1_ERR] = 4;
					digitalWrite(SPARK,HIGH);
					indicator.PILOT_ON = 1;
					vTaskDelay((12000L * configTICK_RATE_HZ) / 1000L);
					digitalWrite(MAIN_V,HIGH);
					hmi.Display[BMS_EXT1_ERR] = 5;
					vTaskDelay((1000L * configTICK_RATE_HZ) / 1000L);
					digitalWrite(SPARK,LOW);
					if(FLM_STATUS)
					{
						bms.state = 4;

					}
					else
					{
						Debug.println("Flame Failure");
						hmi.Display[ERRORS] |= FLM_ERR4_SET;
						digitalWrite(ALARM_R,HIGH);
						digitalWrite(MAIN_V,LOW);
						digitalWrite(SPARK,LOW);
						bms.start = 0;
						bms.state = -1;
					}
				}
			}
			else if(bms.state == 4)
			{
				if(PRS_STATUS || GAS_STATUS || WTR_STATUS || AIR_STATUS == 0 || FLM_STATUS==0)
				{
					if(WTR_STATUS)
					{
						hmi.Display[ERRORS] |= WTR_ERR_SET;
						digitalWrite(ALARM_R,HIGH);
					}
					if(PRS_STATUS)
					{
						hmi.Display[ERRORS] |= PRS_ERR_SET;
					}
					if(GAS_STATUS)
					{
						hmi.Display[ERRORS] |= GAS_ERR_SET;
						digitalWrite(ALARM_R,HIGH);
					}
					if(!AIR_STATUS)
					{
						hmi.Display[ERRORS] |= AIR_ERR2_SET;
						digitalWrite(ALARM_R,HIGH);
					}
					if(!FLM_STATUS)
					{
						hmi.Display[ERRORS] |= FLM_ERR3_SET;
						digitalWrite(ALARM_R,HIGH);
					}
					indicator.FD_FAN_ON = 0;
					indicator.PILOT_ON = 0;
					indicator.MAIN_VALVE_ON = 0;
					bms.state = -1;
					bms.start = 0;
				}
				else
				{
					indicator.MAIN_VALVE_ON = 1;
					hmi.Display[BMS_EXT1_ERR] = 6;	
				}
			}
		}
		else
		{
			if(hmi.Settings[BMS_ENABLE] == 0)
			{
				bms.start = 1;
			}
			indicator.FD_FAN_ON = 0;
			indicator.PILOT_ON = 0;
			indicator.MAIN_VALVE_ON = 0;
			digitalWrite(MAIN_V,LOW);
			digitalWrite(SPARK,LOW);
			hmi.Display[BMS_EXT1_ERR] = 0;
			bms.state = -1;
			taskYIELD();
		}
		vTaskDelay((500L * configTICK_RATE_HZ) / 1000L);
		//Debug.println(hmi.Display[ERRORS]);
	}

}

void burner_control (void * arg)
{
	//vTaskSuspend(NULL);


	while(1)
	{
		if(hmi.Settings[MODE]==1)
		{
			digitalWrite(GRN1, HIGH);
			digitalWrite(YLW2, LOW);
		}
		else if(hmi.Settings[MODE]==2)
		{
			digitalWrite(YLW2, HIGH);
			digitalWrite(GRN1, LOW);
		}
		else
		{
			digitalWrite(YLW2, LOW);
			digitalWrite(GRN1, LOW);
		}
		if(indicator.FD_FAN_ON == 0 && indicator.PILOT_ON == 0 && indicator.MAIN_VALVE_ON == 0)
		{
			motor.position = 0;
			motor.feedback_check = 0;
			
			analogWrite(VFD_P,0);
			digitalWrite(VFD_R,LOW);
			digitalWrite(VFD_R_O,LOW);
			digitalWrite(PURGE_R,LOW);
			
			fusion.state = 0; 
			fusion.desired_air_level = 0;
			fusion.current_duty = 0;
			fusion.wait = 0;

			indicator.purging = 0;
			digitalWrite(VFD_P,LOW);
			analogWrite(OIL_M_R,0);
			digitalWrite(OIL_M_R,LOW);
		}
	
	if(fusion.state == -1)
	{
		motor.position = 0;
		motor.feedback_check = 0;
		
		analogWrite(VFD_P,0);
		digitalWrite(VFD_R,LOW);
		digitalWrite(VFD_R_O,LOW);
		digitalWrite(PURGE_R,LOW);
		
		fusion.state = 0; 
		fusion.desired_air_level = 0;
		fusion.current_duty = 0;
		fusion.wait = 0;

		indicator.purging = 0;
		digitalWrite(VFD_P,LOW);
		analogWrite(OIL_M_R,0);
		digitalWrite(OIL_M_R,LOW);
	}
	
	else if(fusion.state == 0)
	{

		Debug.println("FUSION state = 0\n");

		if(indicator.FD_FAN_ON == 1)
		{
			if(motor.position == 0 && indicator.motor_done == 1 && indicator.vfd_done)
			{
				if(fusion.wait == 3)
				{
					fusion.state = 1;			// PURGING POSITIONING ...
					fusion.wait = 0;
					starter();
				}
				else
				{
					fusion.wait++;
				}
			}
		}
	}
	
	
	else if(fusion.state == 1)
	{
		Debug.println("FUSION state = 1\n");
		if(indicator.FD_FAN_ON == 1)
		{
			if(motor.position == 100 && indicator.motor_done == 1 && indicator.vfd_done)
			{
				if(fusion.wait == 30)
				{
					fusion.state = 2;				// PILOT POSITIONING ...
					fusion.wait = 0;
					starter();
				}
				else
					fusion.wait++;
			}
		}
		else
			fusion.state = -1;
	}
	
	
	else if(fusion.state == 2)
	{
		Debug.println("FUSION state = 2\n");
		if(indicator.FD_FAN_ON == 1)
		{
			if(bms.DUEL)
			{
				if(motor.position == hmi.Settings[CPILOT_OIL] && indicator.motor_done == 1 && indicator.vfd_done)
				{	
					if(fusion.wait == 4)
					{
						//digitalWrite(LFH,HIGH);
						indicator.purging = 1;
						fusion.state = 3;
						fusion.wait = 0;
					}
					else
						fusion.wait++;
				}
			}
			else
			{
				if(motor.position == hmi.Settings[CPILOT_GAS] && indicator.motor_done == 1 && indicator.vfd_done)
				{	
					if(fusion.wait == 4)
					{
						//digitalWrite(LFH,HIGH);
						indicator.purging = 1;
						fusion.state = 3;
						fusion.wait = 0;
					}
					else
						fusion.wait++;
				}
			}
		}
		else
			fusion.state = -1;
	}
	
	else if(fusion.state == 3)
	{
		Debug.println("FUSION state = 3\n");
		if(indicator.FD_FAN_ON == 1 && indicator.MAIN_VALVE_ON == 1)
		{
			if(fusion.wait == 40)
			{
				fusion.state = 4;
				fusion.wait = 0;
				starter();
			}
			else
				fusion.wait++;
		}
		else if(fusion.wait > 0)
			fusion.state = -1;
	}
	
	else if(fusion.state == 4)
	{
		Debug.println("FUSION state = 4\n");
		if(indicator.FD_FAN_ON == 1 && indicator.MAIN_VALVE_ON == 1)
		{
			if(bms.DUEL)
			{
				if(motor.position == hmi.Settings[C1GAS] && indicator.motor_done == 1 && indicator.vfd_done)
					fusion.state = 5;
			}
			else
			{
				if(motor.position == hmi.Settings[C1OIL] && indicator.motor_done == 1 && indicator.vfd_done)
					fusion.state = 5;
			}
			fusion.state = 5;

		}
		else
		{
			fusion.state = -1;
		}
		
	}
	
	else if(fusion.state == 5)
	{

		Debug.println("FUSION state = 5\n");
		if(indicator.FD_FAN_ON == 1 && indicator.MAIN_VALVE_ON == 1)
		{
			if(fusion.wait >= 20 && indicator.motor_done && indicator.vfd_done)
			{
				
				g_Setpoint = (double)hmi.Settings[SET_POINT];
				g_Input =  (double)PROCESS_P;
				GAS_PID.SetTunings((double)hmi.Settings[G_KP]/1000,(double)hmi.Settings[G_KI]/1000,(double)hmi.Settings[G_KD]/1000);
				if(bms.DUEL)
				{
					GAS_PID.SetOutputLimits((double)hmi.Settings[C12AIR],(double)hmi.Settings[C82AIR]);
				}
				else
				{
					GAS_PID.SetOutputLimits((double)hmi.Settings[C1AIR],(double)hmi.Settings[C8AIR]);
				}
				GAS_PID.Compute();
				hmi.Display[PID_VALUE] = (uint16_t)g_Output;
				fusion.wait = 0;
				worker();
			}
			else
				fusion.wait++;
		}
		else
		{
			fusion.state = -1;
		}
	}
	vTaskDelay((500L * configTICK_RATE_HZ) / 1000L);
	}
}

void pump_control()
{
	static int current_drum_level = 0;
	static unsigned int control_valve = 0;
	static unsigned int duty = 0;
	if(hmi.Settings[PUMP1_ON])
	{
		digitalWrite(PUMP1,HIGH);
	}
	else
	{
		digitalWrite(PUMP1,LOW);
	}
	if(hmi.Settings[PUMP2_ON])
	{
		//digitalWrite(HEAT2,HIGH);
	}
	else
	{
		//digitalWrite(HEAT2,LOW);
	}
	PUMP_PID.SetTunings((double)hmi.Settings[G_KP]/1000,(double)hmi.Settings[G_KI]/1000,(double)hmi.Settings[G_KD]/1000);
	w_Setpoint = (double)hmi.Settings[PUMP_SET_POINT];
	w_Input = (double)WTR_L_CURR;
	PUMP_PID.Compute();
	control_valve = (unsigned int)w_Output;
	if(control_valve <= 15)
	{
		control_valve = 15;
	}
	else if(control_valve >=100)
	{
		control_valve = 100;
	}
	hmi.Display[PUMP_PID_LEVEL] = control_valve;
	duty = map(control_valve,0,120,0,PWM_FULL);
	analogWrite(VFD_W_P,duty);
	
}