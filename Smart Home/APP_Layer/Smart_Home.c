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

#define EEPROM_Status 0x20

#define EEPROM_UserNameStartLocation 0x21
#define EEPROM_UserNameEndLocation   0x40

#define EEPROM_PassWordStartLocation  0x41
#define EEPROM_PassWordEndLocation    0x60

#define EEPROM_UserNameMaximumLength 20
#define EEPROM_PassWordMaximumLength  20

char UserName[EEPROM_PassWordMaximumLength];
char Password[EEPROM_UserNameMaximumLength];

volatile u8 KPD_Press, Error_State = 1, USNL = 0, PWL = 0;
volatile u8 USN_Get[20];
/*
 * USNL ==> User Name Length
 * PWL   ==> PassWord Length
 */

void Clear_Char      (void);
void Set_UserName(void);


void main(void)
{
	/* Initialize CLCD On PORTB And 4Bit Mode And Connected on Low Nibble */
	CLCD_vInit();

	/* Initialize SPI */
	SPI_vInit();

	/* Initialize USART to communicate with laptop */
	USART_vInit();

	CLCD_vSetPosition(1, 7);
	CLCD_vSendString("Welcome");
	Set_UserName();
	while (1)
	{
	}
}

//======================================================================================================================================//

void Set_UserName(void)
{
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
					EEPROM_vWrite(EEPROM_PassWordStartLocation + USNL, KPD_Press);
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

	for (u8 i = 0; i < 20 - USNL; i++)
	{
		EEPROM_vWrite(EEPROM_UserNameEndLocation - i, 0xFF);
	}

}

//======================================================================================================================================//

void Clear_Char()
{
	CLCD_vSendCommand(CLCD_SHIFT_CURSOR_LEFT);
	CLCD_vSendData(' ');
	CLCD_vSendCommand(CLCD_SHIFT_CURSOR_LEFT);

}
