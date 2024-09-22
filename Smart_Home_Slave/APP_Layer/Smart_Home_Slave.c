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
#include "../MCAL_Layer/SPI/SPI_interface.h"
#include "../MCAL_Layer/TIMER/TIMER_interface.h"

#include "../HAL_Layer/CLCD/CLCD_interface.h"
#include "../HAL_Layer/LED/LED_interface.h"
#include "../HAL_Layer/LM35/LM35_interface.h"
#include "../HAL_Layer/SERVO_MOTOR/Servo_Motor_interface.h"
#include "../HAL_Layer/SW/SW_interface.h"

#define DUMMY_DATA 0XFF

volatile u8 Error_State, KPD_Press;

void main()
{
	CLCD_vInit();
	SPI_vInit();
	while(1)
	{
		Error_State = SPI_u8Tranceive(DUMMY_DATA, &KPD_Press);
		if (Error_State == OK)
		{
			CLCD_vSendData(KPD_Press);
		}
	}

}

//======================================================================================================================================//
