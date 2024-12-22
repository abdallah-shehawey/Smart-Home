/*
 * Smart_Home.c
 *
 *  Created on: Aug 29, 2024
 *      Author: Mega
 */

#define F_CPU 8000000UL
#include <util/delay.h>

#include "STD_TYPES.h"
#include "STD_MACROS.h"

#include "../MCAL_Layer/DIO/DIO_interface.h"
#include "../MCAL_Layer/DIO/DIO_private.h"
#include "../MCAL_Layer/ADC/ADC_interface.h"
#include "../MCAL_Layer/GIE/GIE_interface.h"
#include "../MCAL_Layer/SPI/SPI_interface.h"
#include "../MCAL_Layer/TIMER/TIMER_interface.h"

#include "../HAL_Layer/CLCD/CLCD_interface.h"
#include "../HAL_Layer/LED/LED_interface.h"
#include "../HAL_Layer/LM35/LM35_interface.h"
#include "../HAL_Layer/SERVO_MOTOR/Servo_Motor_interface.h"

#define DUMMY_DATA 0XFF
#define Kitchen_Fan      2
#define Auto_Fan          3

void RecSalDoor();
void LCD_Print();
void Rec_Fan_Control();
void Sal_Fan_Control();
void Kitch_Fan_Control();

volatile u8 Error_State, SPI_Recieve, Temp_Flag = 0;
LM35_Config LM35 = {ADC_CHANNEL0, 5, ADC_RES_10_BIT};
volatile u8 LM35_Temp = 0, Temp_Detect = 0, Rec_Fan_Save_State = 0, Ret_Rec_Fan_State_Flag = 0, Fire_Flag = 0;
volatile u8 FireNum_Flag = 0;
void main()
{
  DIO_enumSetPinDir(DIO_PORTA, Auto_Fan, DIO_PIN_OUTPUT);//Auto Fan Pin3
  CLCD_vInit();
  SPI_vInit();
  ADC_vInit();
  DIO_enumSetPinDir(DIO_PORTB, DIO_PIN3, DIO_PIN_OUTPUT);
  TIMER0_vInit();
  DIO_enumSetPinDir(DIO_PORTD, DIO_PIN5, DIO_PIN_OUTPUT);
  TIMER1_vInit();
  DIO_enumSetPinDir(DIO_PORTD, DIO_PIN7, DIO_PIN_OUTPUT);
  TIMER2_vInit();
  SM_vInit();

  DIO_enumSetPinDir  (DIO_PORTD, DIO_PIN0, DIO_PIN_OUTPUT);
  DIO_enumSetPinDir  (DIO_PORTD, DIO_PIN1, DIO_PIN_OUTPUT);

  DIO_enumSetPinDir  (DIO_PORTA, DIO_PIN1, DIO_PIN_OUTPUT);//Generate Interrupt for Master uC
  DIO_enumSetPinDir  (DIO_PORTA, Kitchen_Fan, DIO_PIN_OUTPUT);//Kitchen Fan

  DIO_enumWritePinVal(DIO_PORTD, DIO_PIN0, DIO_HIGH);
  DIO_enumWritePinVal(DIO_PORTD, DIO_PIN1, DIO_HIGH);

  DIO_enumWritePinVal(DIO_PORTA, DIO_PIN1, DIO_LOW);
  DIO_enumWritePinVal(DIO_PORTA, Kitchen_Fan, DIO_LOW);


  SM_vTimer1Degree(90);
  _delay_ms(300);
  DIO_enumWritePinVal(DIO_PORTD, DIO_PIN0, DIO_LOW);
  DIO_enumWritePinVal(DIO_PORTD, DIO_PIN1, DIO_LOW);
  CLCD_vSendString("Temp : ");
  while(1)
  {
    Error_State = SPI_u8Tranceive(DUMMY_DATA, &SPI_Recieve);
    if (Error_State == OK)
    {
      if (SPI_Recieve > 0 && SPI_Recieve < 5)
      {
        RecSalDoor();
      }
      else if (SPI_Recieve > 4 && SPI_Recieve < 10)
      {
        Rec_Fan_Control();
      }
      else if (SPI_Recieve > 9 && SPI_Recieve < 15)
      {
        Sal_Fan_Control();
      }
      else if (SPI_Recieve == 15)
      {
        Kitch_Fan_Control();
      }
      else if(SPI_Recieve == 16 || SPI_Recieve == 17)
      {
        if (SPI_Recieve == 16)
        {
          DIO_enumWritePinVal(DIO_PORTA, Auto_Fan, DIO_HIGH);
        }
        else if (SPI_Recieve == 17)
        {
          DIO_enumWritePinVal(DIO_PORTA, Auto_Fan, DIO_LOW);
        }
        else
        {

        }
      }
      else
      {

      }
    }
    LM35_u8GetTemp(&LM35, &LM35_Temp);
    LCD_Print();
    if (LM35_Temp >= 46)
    {
      CLCD_vSetPosition(2, 1);
      CLCD_vSendString("Fire Alarm");
      DIO_enumTogglePinVal(DIO_PORTA, DIO_PIN1);
      Fire_Flag = 1;
      FireNum_Flag++;
      if (FireNum_Flag == 250)
      {
        FireNum_Flag = 0;
      }
      _delay_ms(200);
    }
    else
    {

      if (Fire_Flag == 1)
      {
        if (FireNum_Flag % 2 != 0)
        {
          DIO_enumTogglePinVal(DIO_PORTA, DIO_PIN1);
        }
        else
        {

        }
        CLCD_vSetPosition(2, 1);
        CLCD_vSendString("           ");
        Fire_Flag = 0;
        FireNum_Flag = 0;
      }
      else
      {

      }
    }
  }
}

//======================================================================================================================================//
//Recieve Data from Master uC To open Doors or close it
void RecSalDoor()
{
  switch (SPI_Recieve)
  {
  //open Reception Door
  case 0x01:
    DIO_enumWritePinVal(DIO_PORTD, DIO_PIN0, DIO_HIGH);
    SM_vTimer1Degree(0);
    _delay_ms(300);
    DIO_enumWritePinVal(DIO_PORTD, DIO_PIN0, DIO_LOW);
    break;
    //close Reception Door
  case 0x02:
    DIO_enumWritePinVal(DIO_PORTD, DIO_PIN0, DIO_HIGH);
    SM_vTimer1Degree(90);
    _delay_ms(300);
    DIO_enumWritePinVal(DIO_PORTD, DIO_PIN0, DIO_LOW);
    break;
    //open Salon Door
  case 0x03 :
    DIO_enumWritePinVal(DIO_PORTD, DIO_PIN1, DIO_HIGH);
    SM_vTimer1Degree(0);
    _delay_ms(300);
    DIO_enumWritePinVal(DIO_PORTD, DIO_PIN1, DIO_LOW);
    break;
    //close Salon Door
  case 0x04 :
    DIO_enumWritePinVal(DIO_PORTD, DIO_PIN1, DIO_HIGH);
    SM_vTimer1Degree(90);
    _delay_ms(300);
    DIO_enumWritePinVal(DIO_PORTD, DIO_PIN1, DIO_LOW);
    break;
  default :
    break;
  }
}
//======================================================================================================================================//
//Recieve Data from Master uC and control on Reception Fan
void Rec_Fan_Control()
{
  switch (SPI_Recieve)
  {
  case 0x05:
    TIMER0_vSetCTC(0);
    Rec_Fan_Save_State = 0;
    break;
  case 0x06:
    TIMER0_vSetCTC(63);
    Rec_Fan_Save_State = 1;
    break;
  case 0x07:
    TIMER0_vSetCTC(126);
    Rec_Fan_Save_State = 2;
    break;
  case 0x08:
    TIMER0_vSetCTC(189);
    Rec_Fan_Save_State = 3;
    break;
  case 0x09:
    TIMER0_vSetCTC(255);
    Rec_Fan_Save_State = 4;
    break;
  default :
    break;
  }
}
//======================================================================================================================================//
//Recieve Data from Master uC and control on Salon Fan
void Sal_Fan_Control()
{
  switch (SPI_Recieve)
  {
  case 0x0A:
    TIMER2_vSetCTC(0);
    break;
  case 0x0B:
    TIMER2_vSetCTC(63);
    break;
  case 0x0C:
    TIMER2_vSetCTC(126);
    break;
  case 0x0D:
    TIMER2_vSetCTC(189);
    break;
  case 0x0E:
    TIMER2_vSetCTC(255);
    break;
  default :
    break;
  }
}
//======================================================================================================================================//
void Kitch_Fan_Control(void)
{
  switch (SPI_Recieve)
  {
  case 0x0F:
    DIO_enumTogglePinVal(DIO_PORTA, Kitchen_Fan);
    break;
  default :
    break;
  }
}

//======================================================================================================================================//
//Show Temp On LCD
void LCD_Print()
{
  if(LM35_Temp < 10)
  {
    CLCD_vSetPosition(1, 8);
    CLCD_vSendIntNumber(LM35_Temp);
    CLCD_vSendData(0xDF);
    CLCD_vSendString("C  ");
  }
  else if (LM35_Temp < 100)
  {
    CLCD_vSetPosition(1, 8);
    CLCD_vSendIntNumber(LM35_Temp);
    CLCD_vSendData(0xDF);
    CLCD_vSendString("C ");
  }
  else
  {
    CLCD_vSetPosition(1, 8);
    CLCD_vSendIntNumber(LM35_Temp);
    CLCD_vSendData(0xDF);
    CLCD_vSendString("C");
  }
  if (READ_BIT(PORTA_REG, Auto_Fan) == 0)
  {
    if (Ret_Rec_Fan_State_Flag == 1)
    {
      Temp_Detect = 0;
      switch(Rec_Fan_Save_State)
      {
      case 0:
        TIMER0_vSetCTC(0);
        Ret_Rec_Fan_State_Flag = 0;
        break;
      case 1:
        TIMER0_vSetCTC(63);
        Ret_Rec_Fan_State_Flag = 0;
        break;
      case 2:
        TIMER0_vSetCTC(126);
        Ret_Rec_Fan_State_Flag = 0;
        break;
      case 3:
        TIMER0_vSetCTC(189);
        Ret_Rec_Fan_State_Flag = 0;
        break;
      case 4:
        TIMER0_vSetCTC(255);
        Ret_Rec_Fan_State_Flag = 0;
        break;
      default :
        break;
      }
    }
  }
  else if (READ_BIT(PORTA_REG, Auto_Fan) == 1)
  {
    if (LM35_Temp < 20)
    {
      Temp_Detect = 0;
      TIMER0_vSetCTC(0);
      Ret_Rec_Fan_State_Flag = 1;
    }
    else if (LM35_Temp >= 20 && LM35_Temp <= 25)
    {
      if (Temp_Detect != 1)
      {
        TIMER0_vSetCTC(50);
        Temp_Detect = 1;
        Ret_Rec_Fan_State_Flag = 1;
      }
    }
    else if (LM35_Temp >= 26 && LM35_Temp <= 30)
    {
      if (Temp_Detect != 2)
      {
        TIMER0_vSetCTC(100);
        Temp_Detect = 2;
        Ret_Rec_Fan_State_Flag = 1;
      }
    }
    else if (LM35_Temp >= 31 && LM35_Temp <= 35)
    {
      if (Temp_Detect != 3)
      {
        TIMER0_vSetCTC(150);
        Temp_Detect = 3;
        Ret_Rec_Fan_State_Flag = 1;
      }
    }
    else if (LM35_Temp >= 36 && LM35_Temp <= 40)
    {
      if (Temp_Detect != 4)
      {
        TIMER0_vSetCTC(200);
        Temp_Detect = 4;
        Ret_Rec_Fan_State_Flag = 1;
      }
    }
    else if (LM35_Temp >= 41 && LM35_Temp <= 45)
    {

      if (Temp_Detect != 5)
      {
        TIMER0_vSetCTC(255);
        Temp_Detect = 5;
        Ret_Rec_Fan_State_Flag = 1;
      }
    }
    else
    {

    }
  }
}
