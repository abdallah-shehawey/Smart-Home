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
#include "../MCAL_Layer/GIE/GIE_interface.h"
#include "../MCAL_Layer/EEPROM/EEPROM_interface.h"
#include "../MCAL_Layer/SPI/SPI_interface.h"
#include "../MCAL_Layer/USART/USART_interface.h"
#include "../MCAL_Layer/EXTI/EXTI_interface.h"
#include "../MCAL_Layer/TIMER/TIMER_interface.h"

#include "../HAL_Layer/LED/LED_interface.h"
#include "Security.h"

#define While_Continue 1
#define While_Break      0
#define Time_Out           100UL
#define DUMMY_DATA 0XFF

volatile u8 While_Status1 = While_Continue , While_Status2 = While_Continue;
volatile u8 Error_State, KPD_Press, SPI_Recieve;
volatile u8 KPD_PressLength = 0,  PressVal = 0;
volatile u8 Error_Time_Out = 0;
extern u8 UserName[20];

//PORTA_PIN
LED_config uCKitLed_1    =  {DIO_PORTA, DIO_PIN0, HIGH};
LED_config uCKitLed_2    =  {DIO_PORTA, DIO_PIN1, HIGH};
LED_config uCBedLed_1    =  {DIO_PORTA, DIO_PIN2, HIGH};
LED_config uCBedLed_2   =  {DIO_PORTA, DIO_PIN3, HIGH};
LED_config uCBthLed       =  {DIO_PORTA, DIO_PIN4, HIGH};
LED_config uCCorLed_1    =  {DIO_PORTA, DIO_PIN5, HIGH};
LED_config uCCorLed_2    =  {DIO_PORTA, DIO_PIN6, HIGH};
LED_config uCStairLed    =  {DIO_PORTA, DIO_PIN7, HIGH};

//PORTD_PIN
LED_config uCBalLed_1    =  {DIO_PORTD, DIO_PIN2, HIGH};
LED_config uCBalLed_2    =  {DIO_PORTD, DIO_PIN3, HIGH};
LED_config uCBalLed_3    =  {DIO_PORTD, DIO_PIN4, HIGH};
LED_config uCBalLed_4    =  {DIO_PORTB, DIO_PIN1, HIGH};
LED_config uCSalLed_1    =  {DIO_PORTD, DIO_PIN6, HIGH};
LED_config uCSalLed_2    =  {DIO_PORTB, DIO_PIN0, HIGH};

//PORTC_PIN
LED_config uCRecLed_1    =  {DIO_PORTC, DIO_PIN0, HIGH};
LED_config uCRecLed_2    =  {DIO_PORTC, DIO_PIN1, HIGH};
LED_config uCRecLed_3    =  {DIO_PORTC, DIO_PIN2, HIGH};
LED_config uCCh_1_Led_1  =  {DIO_PORTC, DIO_PIN3, HIGH};
LED_config uCCh_1_Led_2  =  {DIO_PORTC, DIO_PIN4, HIGH};
LED_config uCCh_2_Led_1  =  {DIO_PORTC, DIO_PIN5, HIGH};
LED_config uCCh_2_Led_2  =  {DIO_PORTC, DIO_PIN6, HIGH};


void Home_vStair();
void Home_vReception();
void Home_vSalon();
void Home_vBed_Room();
void Home_vChildren_Room_1();
void Home_vChildren_Room_2();
void Home_vBath_Room();
void Home_vKitchen();
void Home_vCorridor();
void Home_vBalacon();
void Reception_Door();
void Salon_Door();

void Rec_vFan();
void Sal_vFan();
void Bed_vFan();
void Child_vFan();
void Auto_Fan_Control();

void ISR_EXTI_Interrupt(void);


void main()
{
	DIO_enumSetPortDir(DIO_PORTA, DIO_PORT_OUTPUT);
	DIO_enumSetPortDir(DIO_PORTC, DIO_PORT_OUTPUT);
	DIO_enumSetPortDir(DIO_PORTD, 0xFE);
	DIO_enumSetPinDir(DIO_PORTB, DIO_PIN0, DIO_PIN_OUTPUT);
	DIO_enumSetPinDir(DIO_PORTB, DIO_PIN1, DIO_PIN_OUTPUT);

	USART_vInit();
	SPI_vInit();
	EEPROM_vInit();
	DIO_enumSetPinDir(DIO_PORTB, DIO_PIN3, DIO_PIN_OUTPUT);
	TIMER0_vInit();
	//	DIO_enumSetPinDir(DIO_PORTD, DIO_PIN5, DIO_PIN_OUTPUT);
	//	TIMER1_vInit();
	DIO_enumSetPinDir(DIO_PORTD, DIO_PIN7, DIO_PIN_OUTPUT);
	TIMER2_vInit();

	GIE_vEnable();
	EXTI_vEnableInterrupt(EXTI_LINE2);
	EXTI_vSetSignal(EXTI_ON_CHANGE, EXTI_LINE2);
	EXTI_vSetCallBack(ISR_EXTI_Interrupt, EXTI_LINE2);
	DIO_enumSetPinDir(DIO_PORTB, DIO_PIN2, DIO_PIN_INPUT);
	//Sign_In();
	USART_u8SendStringSynch("Welcome ");
	USART_u8SendStringSynch(UserName);
	USART_u8SendData(0X0D);
	while (1)
	{
		USART_u8SendStringSynch("Select Room :");
		USART_u8SendData(0X0D);
		USART_u8SendStringSynch("1 - Stair            ");
		USART_u8SendStringSynch("2 - Reception");
		USART_u8SendData(0X0D);
		USART_u8SendStringSynch("3 - Salon            ");
		USART_u8SendStringSynch("4 - Bed Room");
		USART_u8SendData(0X0D);
		USART_u8SendStringSynch("5 - Children Room 1  ");  //21
		USART_u8SendStringSynch("6 - Children Room 2");
		USART_u8SendData(0X0D);
		USART_u8SendStringSynch("7 - Bath Room        ");
		USART_u8SendStringSynch("8 - Kitchen");
		USART_u8SendData(0X0D);
		USART_u8SendStringSynch("9 - Corridor         ");
		USART_u8SendStringSynch("10- Balacon");
		USART_u8SendData(0X0D);
		USART_u8SendStringSynch("11- Auto Fan Control");
		USART_u8SendData(0x0D);
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
						Error_Time_Out = 0;
					}
					//if user press back space
					else if (KPD_Press == 0x08)
					{
						Error_Time_Out = 0;
					}
					else
					{
						Error_Time_Out = 0;
						KPD_Press -= 48;
						PressVal = PressVal * 10 + KPD_Press;
						KPD_PressLength++;
					}
				}
				else if (Error_State == OK && KPD_PressLength <= 2)
				{
					//if user press enter
					if (KPD_Press == 0x0D || KPD_Press == 0x0F)
					{
						Error_Time_Out = 0;
						break;
					}
					//if user press back space
					else if (KPD_Press == 0x08)
					{
						Error_Time_Out = 0;
						PressVal = PressVal / 10;
						KPD_PressLength--;
					}
					//if user enter valid data
					else
					{
						Error_Time_Out = 0;
						KPD_Press -= 48;
						PressVal = PressVal * 10 + KPD_Press;
						KPD_PressLength++;
					}
				}
				else if (Error_State == OK && KPD_PressLength > 2)
				{
					//if user press enter
					if (KPD_Press == 0x0D || KPD_Press == 0x0F)
					{
						Error_Time_Out = 0;
						break;
					}
					//if user press back space
					else if (KPD_Press == 0x08)
					{
						Error_Time_Out = 0;
						KPD_PressLength--;
					}
					//if user enter valid data
					else
					{
						Error_Time_Out = 0;
						KPD_PressLength++;
					}
				}
				//				else if (Error_State == TIMEOUT_STATE)
				//				{
				//					Error_Time_Out ++;
				//					if (Error_Time_Out == Time_Out)
				//					{
				//						Error_Time_Out = 0;
				//						USART_u8SendData(0X0D);
				//						USART_u8SendStringSynch("Time Out");
				//						break;
				//					}
				//				}
				else
				{

				}
			}
			if (PressVal > 11)
			{
				USART_u8SendStringSynch("Invalid Choise");
				USART_u8SendData(0X0D);
			}
		} while (PressVal > 11);

		switch (PressVal)
		{
		case 1 :
			Home_vStair();
			break;
		case 2 :
			Home_vReception();
			break;
		case 3 :
			Home_vSalon();
			break;
		case 4 :
			Home_vBed_Room();
			break;
		case 5 :
			Home_vChildren_Room_1();
			break;
		case 6 :
			Home_vChildren_Room_2();
			break;
		case 7 :
			Home_vBath_Room();
			break;
		case 8 :
			Home_vKitchen();
			break;
		case 9 :
			Home_vCorridor();
			break;
		case 10 :
			Home_vBalacon();
			break;
		case 11:
			Auto_Fan_Control();
			break;
		default :
			break;
		}
	}
}

//======================================================================================================================================//
void Home_vStair()
{
	USART_u8SendStringSynch("Stair Options : ");
	USART_u8SendData(0X0D);

	USART_u8SendStringSynch("1- Reception Door    ");
	USART_u8SendStringSynch("2- Salon Door");
	USART_u8SendData(0X0D);
	USART_u8SendStringSynch("3- Open Doors        ");
	USART_u8SendStringSynch("4- Close Doors");
	USART_u8SendData(0X0D);
	USART_u8SendStringSynch("5- Led ON/OFF");
	USART_u8SendData(0X0D);

	While_Status1 = While_Continue;
	while(While_Status1 == While_Continue)
	{
		Error_State = USART_u8ReceiveData(&KPD_Press);
		if (Error_State == OK)
		{
			switch (KPD_Press)
			{
			case '1':
				USART_u8SendData(0X0D);
				Reception_Door();
				break;
			case '2':
				USART_u8SendData(0X0D);
				Salon_Door();
				break;
			case '3' :
				SPI_u8Tranceive(0x01, &SPI_Recieve);
				_delay_ms(50);
				SPI_u8Tranceive(0x03, &SPI_Recieve);
				break;
			case '4' :
				SPI_u8Tranceive(0x02, &SPI_Recieve);
				_delay_ms(50);
				SPI_u8Tranceive(0x04, &SPI_Recieve);
				break;
			case '5':
				LED_vTog(uCStairLed);
				break;
			case 0x08 :
				USART_u8SendData(0X0D);
				While_Status1 = While_Break;
				break;
			default :
				break;
			}
		}
	}
}

//======================================================================================================================================//
void Home_vReception()
{
	USART_u8SendStringSynch("Reception Options : ");
	USART_u8SendData(0X0D);

	USART_u8SendStringSynch("1- Led1 ON/OFF       ");
	USART_u8SendStringSynch("2- Led2 ON/OFF");
	USART_u8SendData(0X0D);
	USART_u8SendStringSynch("3- Led3 ON/OFF       ");
	USART_u8SendStringSynch("4- Rec Fan");
	USART_u8SendData(0X0D);

	While_Status1 = While_Continue;
	while(While_Status1 == While_Continue)
	{
		Error_State = USART_u8ReceiveData(&KPD_Press);
		if (Error_State == OK)
		{
			switch (KPD_Press)
			{
			case '1':
				LED_vTog(uCRecLed_1);
				break;
			case '2':
				LED_vTog(uCRecLed_2);
				break;
			case '3':
				LED_vTog(uCRecLed_3);
				break;
			case '4':
				USART_u8SendData(0X0D);
				Rec_vFan();
				break;
			case 0x08 :
				USART_u8SendData(0X0D);
				While_Status1 = While_Break;
				break;
			default :
				break;
			}
		}
	}
}

//======================================================================================================================================//
void Home_vSalon()
{
	USART_u8SendStringSynch("Salon Options : ");
	USART_u8SendData(0X0D);

	USART_u8SendStringSynch("1- Led1 ON/OFF       ");
	USART_u8SendStringSynch("2- Led2 ON/OFF");
	USART_u8SendData(0X0D);
	USART_u8SendStringSynch("3- Rec Fan");
	USART_u8SendData(0X0D);

	While_Status1 = While_Continue;
	while(While_Status1 == While_Continue)
	{
		Error_State = USART_u8ReceiveData(&KPD_Press);
		if (Error_State == OK)
		{
			switch (KPD_Press)
			{
			case '1':
				LED_vTog(uCSalLed_1);
				break;
			case '2':
				LED_vTog(uCSalLed_2);
				break;
			case '3' :
				Sal_vFan();
				break;
			case 0x08 :
				USART_u8SendData(0X0D);
				While_Status1 = While_Break;
				break;
			default :
				break;
			}
		}
	}
}
//======================================================================================================================================//
void Rec_vFan()
{
	USART_u8SendStringSynch("Fan Control : ");
	USART_u8SendData(0X0D);
	USART_u8SendStringSynch("1- Fan Off           ");
	USART_u8SendStringSynch("2- Speed 1");
	USART_u8SendData(0X0D);
	USART_u8SendStringSynch("3- Speed 2           ");
	USART_u8SendStringSynch("4- Speed 3");
	USART_u8SendData(0X0D);
	USART_u8SendStringSynch("5- Speed 4");
	USART_u8SendData(0X0D);

	While_Status2 = While_Continue;
	while(While_Status2 == While_Continue)
	{
		Error_State = USART_u8ReceiveData(&KPD_Press);
		if (Error_State == OK)
		{
			switch (KPD_Press)
			{
			case '1':
				SPI_u8Tranceive(0x05, &SPI_Recieve);
				break;
			case '2':
				SPI_u8Tranceive(0x06, &SPI_Recieve);
				break;
			case '3':
				SPI_u8Tranceive(0x07, &SPI_Recieve);
				break;
			case '4':
				SPI_u8Tranceive(0x08, &SPI_Recieve);
				break;
			case '5':
				SPI_u8Tranceive(0x09, &SPI_Recieve);
				break;
			case 0x08 :
				USART_u8SendData(0X0D);
				USART_u8SendStringSynch("Reception Options : ");
				USART_u8SendData(0X0D);

				USART_u8SendStringSynch("1- Led1 ON/OFF       ");
				USART_u8SendStringSynch("2- Led2 ON/OFF");
				USART_u8SendData(0X0D);
				USART_u8SendStringSynch("3- Led3 ON/OFF       ");
				USART_u8SendStringSynch("4- Rec Fan");
				USART_u8SendData(0X0D);
				While_Status2 = While_Break;
				break;
			default :
				break;
			}
		}
	}
}
//======================================================================================================================================//
void Sal_vFan()
{
	USART_u8SendStringSynch("Fan Control : ");
	USART_u8SendData(0X0D);
	USART_u8SendStringSynch("1- Fan Off           ");
	USART_u8SendStringSynch("2- Speed 1");
	USART_u8SendData(0X0D);
	USART_u8SendStringSynch("3- Speed 2           ");
	USART_u8SendStringSynch("4- Speed 3");
	USART_u8SendData(0X0D);
	USART_u8SendStringSynch("5- Speed 4");
	USART_u8SendData(0X0D);

	While_Status2 = While_Continue;
	while(While_Status2 == While_Continue)
	{
		Error_State = USART_u8ReceiveData(&KPD_Press);
		if (Error_State == OK)
		{
			switch (KPD_Press)
			{
			case '1':
				SPI_u8Tranceive(0x0A, &SPI_Recieve);
				break;
			case '2':
				SPI_u8Tranceive(0x0B, &SPI_Recieve);
				break;
			case '3':
				SPI_u8Tranceive(0x0C, &SPI_Recieve);
				break;
			case '4':
				SPI_u8Tranceive(0x0D, &SPI_Recieve);
				break;
			case '5':
				SPI_u8Tranceive(0x0E, &SPI_Recieve);
				break;
			case 0x08 :
				USART_u8SendData(0X0D);
				While_Status2 = While_Break;
				USART_u8SendStringSynch("Salon Options : ");
				USART_u8SendData(0X0D);

				USART_u8SendStringSynch("1- Led1 ON/OFF       ");
				USART_u8SendStringSynch("2- Led2 ON/OFF");
				USART_u8SendData(0X0D);
				USART_u8SendStringSynch("3- Led3 ON/OFF       ");
				USART_u8SendStringSynch("4- Rec Fan");
				USART_u8SendData(0X0D);
				break;
			default :
				break;
			}
		}
	}
}

//======================================================================================================================================//
void Home_vBed_Room()
{
	USART_u8SendStringSynch("Bed Room Options : ");
	USART_u8SendData(0X0D);

	USART_u8SendStringSynch("1- Led1 ON/OFF       ");
	USART_u8SendStringSynch("2- Led2 ON/OFF");
	USART_u8SendData(0X0D);
	USART_u8SendStringSynch("3- Bed Fan");
	USART_u8SendData(0X0D);

	While_Status1 = While_Continue;
	while(While_Status1 == While_Continue)
	{
		Error_State = USART_u8ReceiveData(&KPD_Press);
		if (Error_State == OK)
		{
			switch (KPD_Press)
			{
			case '1':
				LED_vTog(uCBedLed_1);
				break;
			case '2':
				LED_vTog(uCBedLed_2);
				break;
			case '3':
				Bed_vFan();
				break;
			case 0x08 :
				USART_u8SendData(0X0D);
				While_Status1 = While_Break;
				break;
			default :
				break;
			}
		}
	}
}
//======================================================================================================================================//
void Bed_vFan()
{
	USART_u8SendStringSynch("Fan Control : ");
	USART_u8SendData(0X0D);
	USART_u8SendStringSynch("1- Fan Off           ");
	USART_u8SendStringSynch("2- Speed 1");
	USART_u8SendData(0X0D);
	USART_u8SendStringSynch("3- Speed 2           ");
	USART_u8SendStringSynch("4- Speed 3");
	USART_u8SendData(0X0D);
	USART_u8SendStringSynch("5- Speed 4");
	USART_u8SendData(0X0D);

	While_Status2 = While_Continue;
	while(While_Status2 == While_Continue)
	{
		Error_State = USART_u8ReceiveData(&KPD_Press);
		if (Error_State == OK)
		{
			switch (KPD_Press)
			{
			case '1':
				TIMER0_vSetCTC(0);
				break;
			case '2':
				TIMER0_vSetCTC(63);
				break;
			case '3':
				TIMER0_vSetCTC(126);
				break;
			case '4':
				TIMER0_vSetCTC(189);
				break;
			case '5':
				TIMER0_vSetCTC(255);
				break;
			case 0x08 :
				USART_u8SendData(0X0D);
				While_Status2 = While_Break;
				USART_u8SendStringSynch("Bed Room Options : ");
				USART_u8SendData(0X0D);

				USART_u8SendStringSynch("1- Led1 ON/OFF       ");
				USART_u8SendStringSynch("2- Led2 ON/OFF");
				USART_u8SendData(0X0D);
				USART_u8SendStringSynch("3- Bed Fan");
				USART_u8SendData(0X0D);
				break;
			default :
				break;
			}
		}
	}
}
//======================================================================================================================================//
void Home_vChildren_Room_1()
{
	USART_u8SendStringSynch("Children Room 1 Options : ");
	USART_u8SendData(0X0D);

	USART_u8SendStringSynch("1- Led1 ON/OFF       ");
	USART_u8SendStringSynch("2- Led2 ON/OFF");
	USART_u8SendData(0X0D);
	USART_u8SendStringSynch("3- Child Fan");
	USART_u8SendData(0X0D);

	While_Status1 = While_Continue;
	while(While_Status1 == While_Continue)
	{
		Error_State = USART_u8ReceiveData(&KPD_Press);
		if (Error_State == OK)
		{
			switch (KPD_Press)
			{
			case '1':
				LED_vTog(uCCh_1_Led_1);
				break;
			case '2':
				LED_vTog(uCCh_1_Led_2);
				break;
			case '3' :
				Child_vFan();
				break;
			case 0x08 :
				USART_u8SendData(0X0D);
				While_Status1 = While_Break;
				break;
			default :
				break;
			}
		}
	}
}
//======================================================================================================================================//
void Home_vChildren_Room_2()
{
	USART_u8SendStringSynch("Children Room 2 Options : ");
	USART_u8SendData(0X0D);

	USART_u8SendStringSynch("1- Led1 ON/OFF       ");
	USART_u8SendStringSynch("2- Led2 ON/OFF");
	USART_u8SendData(0X0D);
	USART_u8SendStringSynch("3- Child Fan");
	USART_u8SendData(0X0D);

	While_Status1 = While_Continue;
	while(While_Status1 == While_Continue)
	{
		Error_State = USART_u8ReceiveData(&KPD_Press);
		if (Error_State == OK)
		{
			switch (KPD_Press)
			{
			case '1':
				LED_vTog(uCCh_2_Led_1);
				break;
			case '2':
				LED_vTog(uCCh_2_Led_2);
				break;
			case '3' :
				Child_vFan();
				break;
			case 0x08 :
				USART_u8SendData(0X0D);
				While_Status1 = While_Break;
				break;
			default :
				break;
			}
		}
	}
}
//======================================================================================================================================//
void Child_vFan()
{
	USART_u8SendStringSynch("Fan Control : ");
	USART_u8SendData(0X0D);
	USART_u8SendStringSynch("1- Fan Off           ");
	USART_u8SendStringSynch("2- Speed 1");
	USART_u8SendData(0X0D);
	USART_u8SendStringSynch("3- Speed 2           ");
	USART_u8SendStringSynch("4- Speed 3");
	USART_u8SendData(0X0D);
	USART_u8SendStringSynch("5- Speed 4");
	USART_u8SendData(0X0D);

	While_Status2 = While_Continue;
	while(While_Status2 == While_Continue)
	{
		Error_State = USART_u8ReceiveData(&KPD_Press);
		if (Error_State == OK)
		{
			switch (KPD_Press)
			{
			case '1':
				TIMER2_vSetCTC(0);
				break;
			case '2':
				TIMER2_vSetCTC(63);
				break;
			case '3':
				TIMER2_vSetCTC(126);
				break;
			case '4':
				TIMER2_vSetCTC(189);
				break;
			case '5':
				TIMER2_vSetCTC(255);
				break;
			case 0x08 :
				USART_u8SendData(0X0D);
				While_Status2 = While_Break;
				USART_u8SendStringSynch("Children Room Options : ");
				USART_u8SendData(0X0D);

				USART_u8SendStringSynch("1- Led1 ON/OFF       ");
				USART_u8SendStringSynch("2- Led2 ON/OFF");
				USART_u8SendData(0X0D);
				USART_u8SendStringSynch("3- Child Fan");
				USART_u8SendData(0X0D);
				break;
			default :
				break;
			}
		}
	}
}
//======================================================================================================================================//
void Home_vBath_Room()
{
	USART_u8SendStringSynch("Bath Room Options : ");
	USART_u8SendData(0X0D);

	USART_u8SendStringSynch("1- Led1 ON/OFF");
	USART_u8SendData(0X0D);

	While_Status1 = While_Continue;
	while(While_Status1 == While_Continue)
	{
		Error_State = USART_u8ReceiveData(&KPD_Press);
		if (Error_State == OK)
		{
			switch (KPD_Press)
			{
			case '1':
				LED_vTog(uCBthLed);
				break;
			case 0x08 :
				USART_u8SendData(0X0D);
				While_Status1 = While_Break;
				break;
			default :
				break;
			}
		}
	}
}
//======================================================================================================================================//
void Home_vKitchen()
{
	USART_u8SendStringSynch("Kitchen Options : ");
	USART_u8SendData(0X0D);

	USART_u8SendStringSynch("1- Led1 ON/OFF       ");
	USART_u8SendStringSynch("2- Led2 ON/OFF");
	USART_u8SendData(0X0D);
	USART_u8SendStringSynch("3- Kitchen Fan");
	USART_u8SendData(0X0D);

	While_Status1 = While_Continue;
	while(While_Status1 == While_Continue)
	{
		Error_State = USART_u8ReceiveData(&KPD_Press);
		if (Error_State == OK)
		{
			switch (KPD_Press)
			{
			case '1':
				LED_vTog(uCKitLed_1);
				break;
			case '2':
				LED_vTog(uCKitLed_2);
				break;
			case '3' :
				SPI_u8Tranceive(0x0F, SPI_Recieve);
				break;
			case 0x08 :
				USART_u8SendData(0X0D);
				While_Status1 = While_Break;
				break;
			default :
				break;
			}
		}
	}
}
//======================================================================================================================================//
void Home_vCorridor()
{
	USART_u8SendStringSynch("Corridor Options : ");
	USART_u8SendData(0X0D);

	USART_u8SendStringSynch("1- Led1 ON/OFF       ");
	USART_u8SendStringSynch("2- Led2 ON/OFF");
	USART_u8SendData(0X0D);

	While_Status1 = While_Continue;
	while(While_Status1 == While_Continue)
	{
		Error_State = USART_u8ReceiveData(&KPD_Press);
		if (Error_State == OK)
		{
			switch (KPD_Press)
			{
			case '1':
				LED_vTog(uCCorLed_1);
				break;
			case '2':
				LED_vTog(uCCorLed_2);
				break;
			case 0x08 :
				USART_u8SendData(0X0D);
				While_Status1 = While_Break;
				break;
			default :
				break;
			}
		}
	}
}
//======================================================================================================================================//
void Home_vBalacon()
{
	USART_u8SendStringSynch("Balacon Options : ");
	USART_u8SendData(0X0D);

	USART_u8SendStringSynch("1- Led1 ON/OFF       ");
	USART_u8SendStringSynch("2- Led2 ON/OFF");
	USART_u8SendData(0X0D);
	USART_u8SendStringSynch("3- Led1 ON/OFF       ");
	USART_u8SendStringSynch("4- Led2 ON/OFF");
	USART_u8SendData(0X0D);

	While_Status1 = While_Continue;
	while(While_Status1 == While_Continue)
	{
		Error_State = USART_u8ReceiveData(&KPD_Press);
		if (Error_State == OK)
		{
			switch (KPD_Press)
			{
			case '1':
				LED_vTog(uCBalLed_1);
				break;
			case '2':
				LED_vTog(uCBalLed_2);
				break;
			case '3':
				LED_vTog(uCBalLed_3);
				break;
			case '4':
				LED_vTog(uCBalLed_4);
				break;
			case 0x08 :
				USART_u8SendData(0X0D);
				While_Status1 = While_Break;
				break;
			default :
				break;
			}
		}
	}
}
//======================================================================================================================================//
void Reception_Door(void)
{
	USART_u8SendStringSynch("Reception Door : ");
	USART_u8SendData(0X0D);
	USART_u8SendStringSynch("1- Open              ");
	USART_u8SendStringSynch("2- Lock");
	USART_u8SendData(0X0D);
	do
	{
		Error_State = USART_u8ReceiveData(&KPD_Press);
		if (Error_State == OK)
		{
			switch (KPD_Press)
			{
			case '1':
				SPI_u8Tranceive(0x01, &SPI_Recieve);
				break;
			case '2':
				SPI_u8Tranceive(0x02, &SPI_Recieve);
				break;
			case 0x08 :
				USART_u8SendData(0X0D);
				USART_u8SendStringSynch("Stair Options : ");
				USART_u8SendData(0X0D);

				USART_u8SendStringSynch("1- Reception Door    ");
				USART_u8SendStringSynch("2- Salon Door");
				USART_u8SendData(0X0D);
				USART_u8SendStringSynch("3- Open Doors        ");
				USART_u8SendStringSynch("4- Close Doors");
				USART_u8SendData(0X0D);
				USART_u8SendStringSynch("5- Led ON/OFF");
				USART_u8SendData(0X0D);
				break;
			default :
				break;
			}
		}
	}while (KPD_Press != 0x08);
}
//======================================================================================================================================//
void Salon_Door()
{
	USART_u8SendStringSynch("Salon Door : ");
	USART_u8SendData(0X0D);
	USART_u8SendStringSynch("1- Open              ");
	USART_u8SendStringSynch("2- Lock");
	USART_u8SendData(0X0D);
	do
	{
		Error_State = USART_u8ReceiveData(&KPD_Press);
		if (Error_State == OK)
		{
			switch (KPD_Press)
			{
			case '1':
				SPI_u8Tranceive(0x03, &SPI_Recieve);
				break;
			case '2':
				SPI_u8Tranceive(0x04, &SPI_Recieve);
				break;
			case 0x08 :
				USART_u8SendData(0X0D);
				USART_u8SendStringSynch("Stair Options : ");
				USART_u8SendData(0X0D);

				USART_u8SendStringSynch("1- Reception Door    ");
				USART_u8SendStringSynch("2- Salon Door");
				USART_u8SendData(0X0D);
				USART_u8SendStringSynch("3- Open Doors        ");
				USART_u8SendStringSynch("4- Close Doors");
				USART_u8SendData(0X0D);
				USART_u8SendStringSynch("5- Led ON/OFF");
				USART_u8SendData(0X0D);
				break;
			default :
				break;
			}
		}
	}while (KPD_Press != 0x08);
}
//======================================================================================================================================//
void Auto_Fan_Control()
{
	USART_u8SendStringSynch("Auto Fan Control");
	USART_u8SendData(0X0D);
	USART_u8SendStringSynch("1- Open              ");
	USART_u8SendStringSynch("2- Close");
	USART_u8SendData(0X0D);
	do
	{
		Error_State = USART_u8ReceiveData(&KPD_Press);
		if (Error_State == OK)
		{
			switch (KPD_Press)
			{
			case '1':
				SPI_u8Tranceive(0x10, &SPI_Recieve);
				break;
			case '2':
				SPI_u8Tranceive(0x11, &SPI_Recieve);
				break;
			case 0x08 :
				USART_u8SendData(0X0D);
				break;
			default :
				break;
			}
		}
	}while (KPD_Press != 0x08);

}
//======================================================================================================================================//

void ISR_EXTI_Interrupt(void)
{
	LED_vTog(uCKitLed_1);
	LED_vTog(uCKitLed_2);
	LED_vTog(uCBedLed_1);
	LED_vTog(uCBedLed_2);
	LED_vTog(uCBthLed);
	LED_vTog(uCCorLed_1);
	LED_vTog(uCCorLed_2);
	LED_vTog(uCStairLed);
	LED_vTog(uCBalLed_1);
	LED_vTog(uCBalLed_2);
	LED_vTog(uCBalLed_3);
	LED_vTog(uCBalLed_4);
	LED_vTog(uCSalLed_1);
	LED_vTog(uCSalLed_2);
	LED_vTog(uCRecLed_1);
	LED_vTog(uCRecLed_2);
	LED_vTog(uCRecLed_3);
	LED_vTog(uCCh_1_Led_1);
	LED_vTog(uCCh_1_Led_2);
	LED_vTog(uCCh_2_Led_1);
	LED_vTog(uCCh_2_Led_2);
	_delay_ms(100);
}
