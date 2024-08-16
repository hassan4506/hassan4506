#include "includes_bms.h"

void AirSwitchError (TimerHandle_t xTimer)
{
  Debug.println("AirSwitch Timeout !");
  bms.start = 0;
  bms.state = -1;
  hmi.Display[ERRORS] |= AIR_ERR2_SET;
}
void PurgingError (TimerHandle_t xTimer)
{
  Debug.println("Purging Timeout !");
  bms.state = -1;
  bms.start = 0;
}

void PurgeOff (TimerHandle_t xTimer)
{
  Debug.println("O2 Off !");
  digitalWrite(O2PURGE_R,LOW);
}

void Bms_auto_start (TimerHandle_t xTimer)
{
  Debug.println("Auto Start !");
  if(bms.start == 0)
  {
    Debug.println("Auto Start OK!");
    if(motor.position == 0 && indicator.motor_done == 1 && vfd_position())
      bms.start = 1;
  }

}


void starter()
{
    if(fusion.state == -1)      //fusion reset due to some error
    {
      motor.position = 0;
      fusion.current_duty = 0;
      analogWrite(VFD_P,0);
      digitalWrite(VFD_R,LOW);
      digitalWrite(VFD_R_O,LOW);
      digitalWrite(PURGE_R,LOW);
      //digitalWrite(LFH,LOW);
      fusion.desired_air_level = 0;
      motor.oil_vfd = map(motor.position,0,OPT_FULL,0,PWM_FULL);
      analogWrite(OIL_M_R,motor.oil_vfd);
    }
    else if(fusion.state == 1)    // indicator.purging
    {
      digitalWrite(VFD_R,HIGH);
      digitalWrite(VFD_R_O,HIGH);
      digitalWrite(PURGE_R,HIGH);
      motor.position = 100;
      fusion.current_duty = 100;
      analogWrite(VFD_P,map(fusion.current_duty,0,OPT_FULL,0,PWM_FULL));
     motor.oil_vfd = map(motor.position,0,OPT_FULL,0,PWM_FULL);
      if(bms.DUEL)
        analogWrite(OIL_M_R,motor.oil_vfd);
    }
    else if(fusion.state == 2)    //pilot Position
    {
      digitalWrite(PURGE_R,LOW);
      if(bms.DUEL)
      {
        motor.position = hmi.Settings[CPILOT_OIL];
        fusion.current_duty = hmi.Settings[CPILOT_2AIR];
      }
      else
      {
        motor.position = hmi.Settings[CPILOT_GAS];
        fusion.current_duty = hmi.Settings[CPILOT_AIR];
      }
      analogWrite(VFD_P,map(fusion.current_duty,0,OPT_FULL,0,PWM_FULL));
     motor.oil_vfd = map(motor.position,0,OPT_FULL,0,PWM_FULL);
      if(bms.DUEL)
        analogWrite(OIL_M_R,motor.oil_vfd);
    }
    else if(fusion.state == 4)    //1st Step startup fusion startup
    {
      digitalWrite(PURGE_R,HIGH);
      if(bms.DUEL)
      {
        motor.position = hmi.Settings[C1OIL];
        fusion.current_duty = hmi.Settings[C12AIR];
      }
      else
      {
        motor.position = hmi.Settings[C1GAS];
        fusion.current_duty = hmi.Settings[C1AIR];
      }
      
      analogWrite(VFD_P,map(fusion.current_duty,0,OPT_FULL,0,PWM_FULL));
     motor.oil_vfd = map(motor.position,0,OPT_FULL,0,PWM_FULL);
      if(bms.DUEL)
        analogWrite(OIL_M_R,motor.oil_vfd);
      
    }
    
}

void worker()
{
  switch(hmi.Settings[MODE])
  {
    case 0:
    #ifdef DEBUGEN
      Serial.println("Auto Selected");
    #endif
     system_auto();
      break;
    case 1:
    #ifdef DEBUGEN
      Serial.println("Manual Selected");
    #endif
     system_lfh();
      break;
    case 2:
    #ifdef DEBUGEN
      Serial.println("Manual Selected");
    #endif
    system_manual();
      break;
      //Serial.println("Invalid Mode !");
  }
}

void system_auto()
{
  if(PROCESS_P > hmi.Settings[SET_POINT])
    {
      if(PROCESS_P - hmi.Settings[SET_POINT] <= 3)
      {
        fusion.O2_Triming = 1;
        fusion.pid_on = 0;
      }
      else
      {
        fusion.O2_Triming = 0;
        fusion.o2_level_decreasing = 0;
        fusion.o2_level_increasing = 0;
        fusion.pid_on = 1;
        /* if(PROCESS_P - hmi.Settings[SET_POINT] < 10)
        {
          GAS_PID.SetTunings(0.07,0.05,0.07);
        }
        else
        {
          GAS_PID.SetTunings((double)hmi.Settings[G_KP]/1000,(double)hmi.Settings[G_KI]/1000,(double)hmi.Settings[G_KD]/1000);
        } */
      }
    }
    else if(PROCESS_P <= hmi.Settings[SET_POINT])
    {
      if(hmi.Settings[SET_POINT] - PROCESS_P <= 3)
      {
        fusion.O2_Triming = 1;
        fusion.pid_on = 0;
      }
      else
      {
        fusion.o2_level_decreasing = 0;
        fusion.o2_level_increasing = 0;
        fusion.O2_Triming = 0;
        fusion.pid_on = 1;
        /* if(hmi.Settings[SET_POINT]-PROCESS_P < 2)
        {
          GAS_PID.SetTunings(0.25,0.25,0.25);
        }
        else
        {
          GAS_PID.SetTunings((double)hmi.Settings[G_KP]/1000,(double)hmi.Settings[G_KI]/1000,(double)hmi.Settings[G_KD]/1000);
        } */
      }
    }
    
    if(fusion.O2_Triming)
    {
      #ifdef DEBUGEN
        Serial.println("O2 Triming Started");
      #endif
      if(O2LV < 2)
      {
        //fusion.o2_level_decreasing = 0;
        if(fusion.o2_level_increasing < 6)
        {
          fusion.o2_level_increasing++;
          fusion.current_duty+=1;
          if(fusion.current_duty >= 99)
            fusion.current_duty = 99;
          analogWrite(VFD_P,map(fusion.current_duty,0,OPT_FULL,0,PWM_FULL));
          return;
        }
        else
          return;
      }
      else if(O2LV > 4)
      {
        //fusion.o2_level_increasing = 0;
        if(fusion.o2_level_decreasing < 6)
        {
          fusion.o2_level_decreasing ++;
          fusion.current_duty -= 1;
          if(bms.DUEL)
          {
            if(fusion.current_duty <= hmi.Settings[CPILOT_2AIR])
              fusion.current_duty = hmi.Settings[CPILOT_2AIR];
          }
          else
          {
            if(fusion.current_duty <= hmi.Settings[CPILOT_AIR])
              fusion.current_duty = hmi.Settings[CPILOT_AIR];
          }
          analogWrite(VFD_P,map(fusion.current_duty,0,OPT_FULL,0,PWM_FULL));
          //Serial.println("diccreasing O2 = ");
          //Serial.println(fusion.current_duty);
          return;
        }
        else
          return;
      }
      return;
      
    }
    else if(fusion.pid_on)
    {
      fusion.desired_air_level = (unsigned int)g_Output;
      if(bms.DUEL)
      {
        get_set_oil();
      }
      else
      {
        get_set();
      }
    }
}

void system_lfh()
{
  
  if(bms.DUEL)
  {
    fusion.desired_air_level = hmi.Settings[C12AIR];
    get_set_oil();

  }
  else
  {
    fusion.desired_air_level = hmi.Settings[C1AIR];
    get_set();
  }
}

void system_manual()
{
  fusion.desired_air_level = hmi.Settings[MANUAL_IN];
  if(bms.DUEL)
  {
    get_set_oil();
  }
  else
  {
    get_set();
  }
}

void get_set()
{
  if(fusion.current_duty < fusion.desired_air_level)
  {
    if(fusion.desired_air_level - fusion.current_duty <= 5)
    {
      fusion.current_duty+=1;
      if(fusion.current_duty >= hmi.Settings[C8AIR])
        fusion.current_duty = hmi.Settings[C8AIR];
      analogWrite(VFD_P,map(fusion.current_duty,0,OPT_FULL,0,PWM_FULL));
      motor.position = get_Gas_value(fusion.current_duty);
    }
    else if(fusion.desired_air_level - fusion.current_duty <= 10)
    {
      fusion.current_duty+=5;
      if(fusion.current_duty >= hmi.Settings[C8AIR])
        fusion.current_duty = hmi.Settings[C8AIR];
      analogWrite(VFD_P,map(fusion.current_duty,0,OPT_FULL,0,PWM_FULL));
      motor.position = get_Gas_value(fusion.current_duty);
    }
    else
    {
      fusion.current_duty+=10;
      if(fusion.current_duty >= hmi.Settings[C8AIR])
        fusion.current_duty = hmi.Settings[C8AIR];
      analogWrite(VFD_P,map(fusion.current_duty,0,OPT_FULL,0,PWM_FULL));
      motor.position = get_Gas_value(fusion.current_duty);
    }
  }
  else if(fusion.current_duty > fusion.desired_air_level)
  {
    if(fusion.current_duty - fusion.desired_air_level <= 5)
    {
      fusion.current_duty-=1;
      if(fusion.current_duty <= hmi.Settings[C1AIR])
        fusion.current_duty = hmi.Settings[C1AIR];
      analogWrite(VFD_P,map(fusion.current_duty,0,OPT_FULL,0,PWM_FULL));
      motor.position = get_Gas_value(fusion.current_duty);
    }
    else if(fusion.current_duty - fusion.desired_air_level <= 10)
    {
      fusion.current_duty-=5;
      if(fusion.current_duty <= hmi.Settings[C1AIR])
        fusion.current_duty = hmi.Settings[C1AIR];
      analogWrite(VFD_P,map(fusion.current_duty,0,OPT_FULL,0,PWM_FULL));
      motor.position = get_Gas_value(fusion.current_duty);
    }
    else
    {
      fusion.current_duty-=10;
      if(fusion.current_duty <= hmi.Settings[C1AIR])
        fusion.current_duty = hmi.Settings[C1AIR];
      analogWrite(VFD_P,map(fusion.current_duty,0,OPT_FULL,0,PWM_FULL));
      motor.position = get_Gas_value(fusion.current_duty);
    }
  }
  else if(fusion.current_duty == fusion.desired_air_level)
  {
    motor.position = get_Gas_value(fusion.current_duty);
  }
 motor.oil_vfd = map(motor.position,0,OPT_FULL,0,PWM_FULL);
  if(bms.DUEL)
    analogWrite(OIL_M_R,motor.oil_vfd);

}

unsigned int get_Gas_value(unsigned int Air_val)
{
  if(Air_val >= AIR_MAX)
  {
    return GAS_MAX;
  }
  else if(Air_val <= AIR_MIN)
  {
    return GAS_MIN;
  }
  else if(Air_val >= hmi.Settings[C1AIR] && Air_val < hmi.Settings[C2AIR])
  {
    return map(Air_val,hmi.Settings[C1AIR],hmi.Settings[C2AIR],hmi.Settings[C1GAS],hmi.Settings[C2GAS]);
  }
  else if(Air_val >= hmi.Settings[C2AIR] && Air_val < hmi.Settings[C3AIR])
  {
    return map(Air_val,hmi.Settings[C2AIR],hmi.Settings[C3AIR],hmi.Settings[C2GAS],hmi.Settings[C3GAS]);
  }
  else if(Air_val >= hmi.Settings[C3AIR] && Air_val < hmi.Settings[C4AIR])
  {
    return map(Air_val,hmi.Settings[C3AIR],hmi.Settings[C4AIR],hmi.Settings[C3GAS],hmi.Settings[C4GAS]);
  }
  else if(Air_val >= hmi.Settings[C4AIR] && Air_val < hmi.Settings[C5AIR])
  {
    return map(Air_val,hmi.Settings[C4AIR],hmi.Settings[C5AIR],hmi.Settings[C4GAS],hmi.Settings[C5GAS]);
  }
  else if(Air_val >= hmi.Settings[C5AIR] && Air_val < hmi.Settings[C6AIR])
  {
    return map(Air_val,hmi.Settings[C5AIR],hmi.Settings[C6AIR],hmi.Settings[C5GAS],hmi.Settings[C6GAS]);
  }
  else if(Air_val >= hmi.Settings[C6AIR] && Air_val < hmi.Settings[C7AIR])
  {
    return map(Air_val,hmi.Settings[C6AIR],hmi.Settings[C7AIR],hmi.Settings[C6GAS],hmi.Settings[C7GAS]);
  }
  else if(Air_val >= hmi.Settings[C7AIR] && Air_val < hmi.Settings[C8AIR])
  {
    return map(Air_val,hmi.Settings[C7AIR],hmi.Settings[C8AIR],hmi.Settings[C7GAS],hmi.Settings[C8GAS]);
  }
}

unsigned int get_Oil_value(unsigned int Air_val)
{
  if(Air_val >= AIR2_MAX)
  {
    return OIL_MAX;
  }
  else if(Air_val <= AIR2_MIN)
  {
    return OIL_MIN;
  }
  else if(Air_val >= hmi.Settings[C12AIR] && Air_val < hmi.Settings[C22AIR])
  {
    return map(Air_val,hmi.Settings[C12AIR],hmi.Settings[C22AIR],hmi.Settings[C1OIL],hmi.Settings[C2OIL]);
  }
  else if(Air_val >= hmi.Settings[C22AIR] && Air_val < hmi.Settings[C32AIR])
  {
    return map(Air_val,hmi.Settings[C22AIR],hmi.Settings[C32AIR],hmi.Settings[C2OIL],hmi.Settings[C3OIL]);
  }
  else if(Air_val >= hmi.Settings[C32AIR] && Air_val < hmi.Settings[C42AIR])
  {
    return map(Air_val,hmi.Settings[C32AIR],hmi.Settings[C42AIR],hmi.Settings[C3OIL],hmi.Settings[C4OIL]);
  }
  else if(Air_val >= hmi.Settings[C42AIR] && Air_val < hmi.Settings[C52AIR])
  {
    return map(Air_val,hmi.Settings[C42AIR],hmi.Settings[C52AIR],hmi.Settings[C4OIL],hmi.Settings[C5OIL]);
  }
  else if(Air_val >= hmi.Settings[C52AIR] && Air_val < hmi.Settings[C62AIR])
  {
    return map(Air_val,hmi.Settings[C52AIR],hmi.Settings[C62AIR],hmi.Settings[C5OIL],hmi.Settings[C6OIL]);
  }
  else if(Air_val >= hmi.Settings[C62AIR] && Air_val < hmi.Settings[C72AIR])
  {
    return map(Air_val,hmi.Settings[C62AIR],hmi.Settings[C72AIR],hmi.Settings[C6OIL],hmi.Settings[C7OIL]);
  }
  else if(Air_val >= hmi.Settings[C72AIR] && Air_val < hmi.Settings[C82AIR])
  {
    return map(Air_val,hmi.Settings[C72AIR],hmi.Settings[C82AIR],hmi.Settings[C7OIL],hmi.Settings[C8OIL]);
  }
}

void get_set_oil()
{
  if(fusion.current_duty < fusion.desired_air_level)
  {
    if(fusion.desired_air_level - fusion.current_duty <= 5)
    {
      fusion.current_duty+=1;
      if(fusion.current_duty >= hmi.Settings[C82AIR])
        fusion.current_duty = hmi.Settings[C82AIR];
      analogWrite(VFD_P,map(fusion.current_duty,0,OPT_FULL,0,PWM_FULL));
      motor.position = get_Oil_value(fusion.current_duty);
    }
    else if(fusion.desired_air_level - fusion.current_duty <= 10)
    {
      fusion.current_duty+=5;
      if(fusion.current_duty >= hmi.Settings[C82AIR])
        fusion.current_duty = hmi.Settings[C82AIR];
      analogWrite(VFD_P,map(fusion.current_duty,0,OPT_FULL,0,PWM_FULL));
      motor.position = get_Oil_value(fusion.current_duty);
    }
    else
    {
      fusion.current_duty+=10;
      if(fusion.current_duty >= hmi.Settings[C82AIR])
        fusion.current_duty = hmi.Settings[C82AIR];
      analogWrite(VFD_P,map(fusion.current_duty,0,OPT_FULL,0,PWM_FULL));
      motor.position = get_Oil_value(fusion.current_duty);
    }
  }
  else if(fusion.current_duty > fusion.desired_air_level)
  {
    if(fusion.current_duty - fusion.desired_air_level <= 5)
    {
      fusion.current_duty-=1;
      if(fusion.current_duty <= hmi.Settings[C12AIR])
        fusion.current_duty = hmi.Settings[C12AIR];
      analogWrite(VFD_P,map(fusion.current_duty,0,OPT_FULL,0,PWM_FULL));
      motor.position = get_Oil_value(fusion.current_duty);
    }
    else if(fusion.current_duty - fusion.desired_air_level <= 10)
    {
      fusion.current_duty-=5;
      if(fusion.current_duty <= hmi.Settings[C12AIR])
        fusion.current_duty = hmi.Settings[C12AIR];
      analogWrite(VFD_P,map(fusion.current_duty,0,OPT_FULL,0,PWM_FULL));
      motor.position = get_Oil_value(fusion.current_duty);
    }
    else
    {
      fusion.current_duty-=10;
      if(fusion.current_duty <= hmi.Settings[C12AIR])
        fusion.current_duty = hmi.Settings[C12AIR];
      analogWrite(VFD_P,map(fusion.current_duty,0,OPT_FULL,0,PWM_FULL));
      motor.position = get_Oil_value(fusion.current_duty);
    }
  }
  else if(fusion.current_duty == fusion.desired_air_level)
  {
    motor.position = get_Oil_value(fusion.current_duty);
  }
 motor.oil_vfd = map(motor.position,0,OPT_FULL,0,PWM_FULL);
  if(bms.DUEL)
    analogWrite(OIL_M_R,motor.oil_vfd);
}
void motor_control()
{
  vfd_control();
    if(bms.DUEL)
    {
      if(FRO_CURR > 0)
        motor.current_position = FRO_CURR;
      else
        motor.current_position = 0;
    }
    else
    {
      if(GASS_CURR > 0)
        motor.current_position = GASS_CURR;
      else
        motor.current_position = 0;
    }

    if(motor.current_position-motor.position > 2 || motor.current_position-motor.position < -2)
    {
      if(motor.current_position < motor.position)
      {
        digitalWrite(MT_B, LOW);
        digitalWrite(MT_F, HIGH);
      }
      else if(motor.current_position > motor.position)
      {
        digitalWrite(MT_F, LOW);
        digitalWrite(MT_B, HIGH);
      }
      indicator.motor_done = 0;
    }
    else
    {
      motor.feedback_check = 0;
      indicator.motor_done = 1;
      digitalWrite(MT_B, LOW);
      digitalWrite(MT_F, LOW);
    }
}


bool vfd_position()
{
  motor.current_air_level = AIR_LEVEL_CURR;

  int diffrence_bt = fusion.current_duty - motor.current_air_level;
  if(diffrence_bt > -5 && diffrence_bt < 5)
    return 1;
  else
    return 0;
}


void vfd_control()
{

  if(AIR_LEVEL_CURR-fusion.current_duty > 1 ||AIR_LEVEL_CURR-fusion.current_duty < -1)
    {
      if(AIR_LEVEL_CURR < fusion.current_duty)
      {
        digitalWrite(VFD_REVERSE, LOW);
        digitalWrite(VFD_FORWARD, HIGH);
      }
      else if(AIR_LEVEL_CURR > fusion.current_duty)
      {
        digitalWrite(VFD_FORWARD, LOW);
        digitalWrite(VFD_REVERSE, HIGH);
      }
      indicator.vfd_done = 0;
    }
    else
    {
      motor.feedback_check = 0;
      indicator.vfd_done = 1;
      digitalWrite(VFD_REVERSE, LOW);
      digitalWrite(VFD_FORWARD, LOW);
    }
}