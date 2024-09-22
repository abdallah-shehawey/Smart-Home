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
#include "Security.h"

volatile u8 Error_State, KPD_Press, SPI_Recieve;
extern u8 UserName[20];
volatile u8 KPD_PressLength = 0;
volatile u32 PressVal = 0;
void main()
{
	DIO_enumSetPortDir(DIO_PORTA, DIO_PORT_OUTPUT);
	DIO_enumSetPortDir(DIO_PORTC, DIO_PORT_OUTPUT);
	DIO_enumSetPortDir(DIO_PORTD, 0xFE);

	USART_vInit();
	SPI_vInit();
	EEPROM_vInit();

	Sign_In();
	USART_u8SendStringSynch("Welcome ");
	USART_u8SendStringSynch(UserName);
	USART_u8SendData(0X0D);

	while (1)
	{
		//		USART_u8SendStringSynch("Select Room :");
		//		USART_u8SendData(0X0D);
		//		USART_u8SendStringSynch("1- Stair             ");
		//		USART_u8SendStringSynch("2- Reception");
		//		USART_u8SendData(0X0D);
		//		USART_u8SendStringSynch("3- Salon             ");
		//		USART_u8SendStringSynch("4- Bed Room");
		//		USART_u8SendData(0X0D);
		//		USART_u8SendStringSynch("5- Children Room 1   ");
		//		USART_u8SendStringSynch("6- Children Room 2");
		//		USART_u8SendData(0X0D);
		//		USART_u8SendStringSynch("7- Bath Room         ");
		//		USART_u8SendStringSynch("8- Kitchen");
		//		USART_u8SendData(0X0D);
		//		USART_u8SendStringSynch("9- Corridor          ");
		//		USART_u8SendStringSynch("10- Balacon");
		//		USART_u8SendData(0X0D);
		do
		{
			KPD_PressLength = 0, PressVal = 0;
			while (1)
			{
				// get input from Laptop
				Error_State = USART_u8ReceiveData(&KPD_Press);
				if (Error_State == OK && KPD_PressLength == 0)
				{
					//if user press enter
					if (KPD_Press == 0x0D || KPD_Press == 0x0F)
					{
					}
					//if user press back space
					else if (KPD_Press == 0x08)
					{
					}
					else
					{
						KPD_Press -= 48;
						PressVal = PressVal * 10 + KPD_Press;
						KPD_PressLength++;
					}
				}
				else if (Error_State == OK && KPD_PressLength > 0)
				{
					//if user press enter
					if (KPD_Press == 0x0D || KPD_Press == 0x0F)
					{
						break;
					}
					//if user press back space
					else if (KPD_Press == 0x08)
					{
						PressVal = PressVal / 10;
						KPD_PressLength--;
					}
					//if user enter valid data
					else
					{
						KPD_Press -= 48;
						PressVal = PressVal * 10 + KPD_Press;
						KPD_PressLength++;
					}
				}
				else
				{

				}
			}
			if (PressVal > 10)
			{
				USART_u8SendStringSynch("Invalid Choise");
				USART_u8SendData(0X0D);
			}
		} while (PressVal > 10);


		switch (PressVal)
		{
		case 10 :
			USART_u8SendStringSynch("9- Stair");
			USART_u8SendData(0X0D);
			break;
		default :
			USART_u8SendStringSynch("abd");
			USART_u8SendData(0X0D);
			break;
		}
		//		Error_State = USART_u8ReceiveData(&KPD_Press);
		//		if (Error_State == OK)
		//		{
		//			SPI_u8Tranceive(KPD_Press, &SPI_Recieve);
		//		}
	}
}

//======================================================================================================================================//
