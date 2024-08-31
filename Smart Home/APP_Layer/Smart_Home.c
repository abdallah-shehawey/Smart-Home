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
#include "../MCAL_Layer/ADC/ADC_interface.h"
#include "../MCAL_Layer/GIE/GIE_interface.h"
#include "../MCAL_Layer/EEPROM/EEPROM_interface.h"
#include "../MCAL_Layer/SPI/SPI_interface.h"
#include "../MCAL_Layer/USART/USART_interface.h"
#include "../MCAL_Layer/TIMER/TIMER_interface.h"

#include "../HAL_Layer/CLCD/CLCD_interface.h"
#include "../HAL_Layer/LED/LED_interface.h"
#include "../HAL_Layer/LM35/LM35_interface.h"
#include "../HAL_Layer/SERVO_MOTOR/Servo_Motor_interface.h"
#include "../HAL_Layer/SW/SW_interface.h"

#define NOTPRESSED 0xFF

#define EEPROM_UserNameStatus 0x10
#define EEPROM_PassWordStatus  0x11
#define EEPROM_TimeLocation      0x12
#define EEPROM_USNL_Location    0x13
#define EEPROM_USNL_Location    0x13

#define EEPROM_UserNameStartLocation 0x21
#define EEPROM_UserNameEndLocation   0x40

#define EEPROM_PassWordStartLocation  0x41
#define EEPROM_PassWordEndLocation    0x60

#define EEPROM_UserNameMaximumLength 20
#define EEPROM_PassWordMaximumLength  20

char UserName[EEPROM_PassWordMaximumLength];
char Password  [EEPROM_UserNameMaximumLength];

volatile u8 KPD_Press, Error_State = 1,USNL = 0, PWL = 0;
volatile u8 Check[21] ;
/*
 * USNL ==> User Name Length
 * PWL   ==> PassWord Length
 */

void Clear_Char            (void);
void Set_UserName     (void);
void Set_PassWord      (void);
void Check_UserName(void);


void main(void)
{
	USNL = EEPROM_vRead(EEPROM_USNL_Location);
	/* Initialize CLCD On PORTB And 4Bit Mode And Connected on Low Nibble */
	CLCD_vInit();

	/* Initialize SPI */
	SPI_vInit();

	/* Initialize USART to communicate with laptop */
	USART_vInit();

	CLCD_vSetPosition(1, 7);
	CLCD_vSendString("Welcome");
	_delay_ms(500);

	if(EEPROM_vRead(EEPROM_UserNameStatus) == NOTPRESSED)
	{
		Set_UserName();
	}
	if(EEPROM_vRead(EEPROM_PassWordStatus) == NOTPRESSED)
	{
		Set_PassWord();
	}

	Check_UserName();
	while (1)
	{
	}
}

//======================================================================================================================================//

void Set_UserName(void)
{
	EEPROM_vWrite(EEPROM_USNL_Location, 0x00);
	USNL = EEPROM_vRead(EEPROM_USNL_Location);
	CLCD_vClearScreen();
	CLCD_vSetPosition(1, 5);
	CLCD_vSendString("Set UserName");
	CLCD_vSetPosition(2, 1);
	CLCD_vSendString("Maximum char : 20");
	CLCD_vSendExtraChar(4, 15);  // To Send Enter Symbol
	CLCD_vSetPosition(4, 16);
	CLCD_vSendString(" : OK");

	CLCD_vSetPosition(3, 1);
	USNL = 0;
	/*Get username from user*/
	do
	{
		//if username is less than 5 char
		if (USNL != 0)
		{
			CLCD_vClearScreen();
			CLCD_vSendString ("UserName Must be");
			CLCD_vSetPosition(2, 1);
			CLCD_vSendString("More than 5 Char");
			CLCD_vSendExtraChar(4, 1);
			CLCD_vSetPosition(4, 2);
			CLCD_vSendString(" : Exit");
			while(1)
			{
				//wait in error page until press enter
				Error_State = USART_u8ReceiveData(&KPD_Press);
				if (Error_State == OK)
				{
					if (KPD_Press == 0x0D || KPD_Press == 0x0F)
					{
						break;
					}
				}
			}
			CLCD_vClearScreen();
			CLCD_vSetPosition(1, 5);
			CLCD_vSendString("Set UserName");
			CLCD_vSetPosition(2, 1);
			CLCD_vSendString("Maximum char : 20");
			CLCD_vSendExtraChar(4, 15); // To Send Enter Symbol
			CLCD_vSetPosition(4, 16);
			CLCD_vSendString(" : OK");
			CLCD_vSetPosition(3, 1);
			USNL = 0;
		}
		CLCD_vSendCommand(CLCD_DISPLAYON_CURSORON);
		//get user name from user by using Keypoard
		while (1)
		{
			//get input from Laptop
			Error_State = USART_u8ReceiveData(&KPD_Press);
			//if user name length is valid
			if (Error_State == OK && USNL < 20)
			{
				if (KPD_Press == 0x0D || KPD_Press == 0x0F)
				{
					CLCD_vSendCommand(CLCD_DISPLAYON_CURSOROFF);
					break;
				}
				else if (KPD_Press == 0x08)
				{
					Clear_Char();
					USNL--;
				}
				else
				{
					CLCD_vSendData(KPD_Press);
					EEPROM_vWrite(EEPROM_UserNameStartLocation + USNL, KPD_Press);
					USNL++;
				}
			}
			//if user name length is more than 20 do no thing exept enter and delete
			else if (Error_State == OK && USNL >= 20)
			{
				if (KPD_Press == 0x0D || KPD_Press == 0x0F)
				{
					CLCD_vSendCommand(CLCD_DISPLAYON_CURSOROFF);
					break;
				}
				else if (KPD_Press == 0x08)
				{
					Clear_Char();
					USNL--;
				}
				else
				{

				}
			}
			else
			{

			}
		}
	}while(USNL <= 5);
	/*___________________________________________________________________________________________________________________*/

	for (u8 i = 0; i < 20 - (USNL - 1); i++)
	{
		EEPROM_vWrite(EEPROM_UserNameEndLocation - i, 0xFF);
	}
	EEPROM_vWrite(EEPROM_UserNameStatus, 0x00);
	EEPROM_vWrite(EEPROM_USNL_Location, USNL);
}

//======================================================================================================================================//

void Set_PassWord(void)
{
	CLCD_vClearScreen();
	CLCD_vSetPosition(1, 5);
	CLCD_vSendString("Set PassWord");
	CLCD_vSetPosition(2, 1);
	CLCD_vSendString("Maximum char : 20");
	CLCD_vSendExtraChar(4, 15);  // To Send Enter Symbol
	CLCD_vSetPosition(4, 16);
	CLCD_vSendString(" : OK");

	CLCD_vSetPosition(3, 1);
	PWL = 0;
	/*Get username from user*/
	do
	{
		//if username is less than 5 char
		if (PWL != 0)
		{
			CLCD_vClearScreen();
			CLCD_vSendString ("PassWord Must be");
			CLCD_vSetPosition(2, 1);
			CLCD_vSendString("More than 5 Char");
			CLCD_vSendExtraChar(4, 1);
			CLCD_vSetPosition(4, 2);
			CLCD_vSendString(" : Exit");
			while(1)
			{
				Error_State = USART_u8ReceiveData(&KPD_Press);
				if (Error_State == OK)
				{
					if (KPD_Press == 0x0D || KPD_Press == 0x0F)
					{
						break;
					}
				}
			}
			CLCD_vClearScreen();
			CLCD_vSetPosition(1, 5);
			CLCD_vSendString("Set PassWord");
			CLCD_vSetPosition(2, 1);
			CLCD_vSendString("Maximum char : 20");
			CLCD_vSendExtraChar(4, 15); // To Send Enter Symbol
			CLCD_vSetPosition(4, 16);
			CLCD_vSendString(" : OK");
			CLCD_vSetPosition(3, 1);
			PWL = 0;
		}
		CLCD_vSendCommand(CLCD_DISPLAYON_CURSORON);
		//get user name from user by using Keypoard
		while (1)
		{
			//get input from Laptop
			Error_State = USART_u8ReceiveData(&KPD_Press);
			//if user name length is valid
			if (Error_State == OK && PWL < 20)
			{
				if (KPD_Press == 0x0D || KPD_Press == 0x0F)
				{
					CLCD_vSendCommand(CLCD_DISPLAYON_CURSOROFF);
					break;
				}
				else if (KPD_Press == 0x08)
				{
					Clear_Char();
					PWL--;
				}
				else
				{
					CLCD_vSendData(KPD_Press);
					EEPROM_vWrite(EEPROM_PassWordStartLocation + PWL, KPD_Press);
					PWL++;
				}
			}
			//if user name length is more than 20 do no thing exept enter and delete
			else if (Error_State == OK && PWL >= 20)
			{
				if (KPD_Press == 0x0D || KPD_Press == 0x0F)
				{
					CLCD_vSendCommand(CLCD_DISPLAYON_CURSOROFF);
					break;
				}
				else if (KPD_Press == 0x08)
				{
					Clear_Char();
					PWL--;
				}
				else
				{

				}
			}
			else
			{

			}
		}
	}while(PWL <= 5);
	/*___________________________________________________________________________________________________________________*/

	for (u8 i = 0; i < 20 - (PWL - 1); i++)
	{
		EEPROM_vWrite(EEPROM_PassWordEndLocation - i, 0xFF);
	}
	EEPROM_vWrite(EEPROM_PassWordStatus, 0x00);
}

//======================================================================================================================================//

void Check_UserName(void)
{
	CLCD_vClearScreen();
	CLCD_vSendString("Check UserName");
	CLCD_vSetPosition(2, 1);
	CLCD_vSendCommand(CLCD_DISPLAYON_CURSORON);
	u8 CheckLength = 0, Check_Flag = 1;

	while(1)
	{
		Error_State = USART_u8ReceiveData(&KPD_Press);
		//if user name length is valid
		if (Error_State == OK && CheckLength < 20)
		{
			if (KPD_Press == 0x0D || KPD_Press == 0x0F)
			{
				CLCD_vSendCommand(CLCD_DISPLAYON_CURSOROFF);
				break;
			}
			else if (KPD_Press == 0x08)
			{
				Clear_Char();
				CheckLength--;
			}
			else
			{
				CLCD_vSendData(KPD_Press);
				Check[CheckLength] = KPD_Press;
				CheckLength++;
			}
		}
		//if user name length is more than 20 do no thing exept enter and delete
		else if (Error_State == OK && CheckLength>= 20)
		{
			if (KPD_Press == 0x0D || KPD_Press == 0x0F)
			{
				CLCD_vSendCommand(CLCD_DISPLAYON_CURSOROFF);
				break;
			}
			else if (KPD_Press == 0x08)
			{
				Clear_Char();
				CheckLength--;
			}
			else
			{

			}
		}
		else
		{

		}
	}
	//Check if UserName is correct or not
	if (CheckLength == USNL)
	{
		for(u8 i = 0; i < USNL; i++)
		{
			if (Check[i] != EEPROM_vRead(EEPROM_UserNameStartLocation + i))
			{
				Check_Flag = 0;
				CLCD_vClearScreen();
				CLCD_vSendString("Wrong UserName");
				_delay_ms(500);
				Check_UserName();
			}
			else
			{

			}
		}
		if(Check_Flag == 1)
		{
			CLCD_vClearScreen();
			CLCD_vSendString("True UserName");
		}
	}
	else if (CheckLength != USNL)
	{
		CLCD_vClearScreen();
		CLCD_vSendString("Wrong UserName");
		_delay_ms(500);
		Check_UserName();
	}
	else
	{

	}
}

//======================================================================================================================================//
void Clear_Char()
{
	CLCD_vSendCommand(CLCD_SHIFT_CURSOR_LEFT);
	CLCD_vSendData(' ');
	CLCD_vSendCommand(CLCD_SHIFT_CURSOR_LEFT);

}















































