/*
 * main.c
 *
 *  Created on: Feb 4, 2015
 *      Author: jcobb
 */
#include <avr/io.h>
#include "FreeRTOS.h"
#include "led.h"
#include "task.h"
#include "apptasks.h"

// #define mainLED_TASK_PRIORITY			(tskIDLE_PRIORITY)
#define mainLED_TASK_PRIORITY 			(tskIDLE_PRIORITY+3)
// #define mainNEXT_TASK_2				(tskIDLE_PRIORITY+2)
// #define mainNEXT_TASK_3				(tskIDLE_PRIORITY+3)


// TODO: Research moving vLEDFlashTask method to apptasks.c
void vLEDFlashTask(void *pvParms)
{
	vLEDInit();
	portTickType xLastWakeTime;
	const portTickType xFrequency = 1000;
	xLastWakeTime = xTaskGetTickCount();

	for(;;) {
		vLEDToggle();
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
	}
}

void vApplicationIdleHook( void );

portSHORT main(void)
{
	xTaskCreate(vLEDFlashTask,"LED", configMINIMAL_STACK_SIZE, NULL, mainLED_TASK_PRIORITY, NULL);

	vTaskStartScheduler();

	return 0;
}

void vApplicationIdleHook( void )
{
	//vCoRoutineSchedule();
	//vLEDOff();
}

