/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Nicolas Lacaille
 * @brief          : Main program body
 ******************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include "stm32f4xx.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define STACK_SIZE	512 // in words (= 4*STACK_SIZE bytes)

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
void pin_init(void);
void T0( void * pvParameters );
void T1( void * pvParameters );

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
	BaseType_t xReturned0, xReturned1;
	TaskHandle_t xHandle0 = 0, xHandle1 = 0;

	/*priority grouping 4 bits for preempt priority 0 bits for subpriority
	 * (No Subpriority) for FreeRTOS*/
	NVIC_SetPriorityGrouping( 3 );

	/*
	 * Peripheral init
	 */
	/* pin (GPIO) */
	pin_init();
	/* USART2 */
	USART2_Init(115200);

	/* tasks creation :
	 * priority are from 0 (tskIDLE_PRIORITY) to configMAX_PRIORITIES
	 * in FreeRTOSConfig.h
	 */

	// on a créé notre première tache pour allumer la led
    xReturned0 = xTaskCreate(
                    T0,       /* Function that implements the task. */
                    "Allumer led",          /* Text name for the task. */
                    STACK_SIZE,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    tskIDLE_PRIORITY+6,/* Priority at which the task is created. */
                    &xHandle0 );      /* Used to pass out the created task's handle. */

    // on a créé notre seconde tache pour éteindre la led
    xReturned1 = xTaskCreate(
                    T1,       /* Function that implements the task. */
                    "Eteindre led",          /* Text name for the task. */
                    STACK_SIZE,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    tskIDLE_PRIORITY+6,/* Priority at which the task is created. */
                    &xHandle1 );      /* Used to pass out the created task's handle. */
    if( xReturned0 == pdPASS && xReturned1 == pdPASS){
        /* The task was created.  The task's handle can be used
         * to delete the task. : vTaskDelete( xHandle ); */
    }else{
    	// not enough memory to create task
    	//printf(" not enough memory to create task T0 \n");
    }

    printf("start of scheduler \n");
    // start the scheduler, tasks will be started and the
    // function will not return
	vTaskStartScheduler();

	//* never go there if scheduler has started */
	/* Loop forever */
	for(;;);
}

/**
 * @brief	Task TO
 * @param	parameters given @ creation
 * @retval	none (should not return)
 */
void T0( void * pvParameters )
{
	// ininite loop :
	for( ;; ){
		// allumer led
		GPIOA->ODR |= GPIO_ODR_OD5;
	}
}

/**
 * @brief	Task T1
 * @param	parameters given @ creation
 * @retval	none (should not return)
 */
void T1( void * pvParameters )
{
	// ininite loop :
	for( ;; ){
		// eteindre led
		GPIOA->ODR &= ~(GPIO_ODR_OD5_Msk);
	}
}


/**
 * @brief	pin configuration for application
 * @param	none
 * @retval	none
 */
void pin_init(void)
{
	GPIO_TypeDef * PA = GPIOA, *PB = GPIOB;

	/* Pin configuration
	 *
	 */
	/****************************** PINS PA5  *********************************/
	/* Enable GPIOA and clock */
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	/* PA5 driven by alternate function : */
	/* select the alternate function first for PIN5  */
	PA->AFR[0] &= ~ (0xF << 20)	;
	PA->AFR[0] |= (0x1 << 20)	;	/* AF01 */
	/* clear MODER for PIN 5 */
	PA->MODER &= ~GPIO_MODER_MODER5;
#ifdef USE_PA5_PWM
	PA->MODER |= GPIO_MODER_MODER5_1; /* alternate function */
	/***************************** PINS PA5 ***********************************/
#else
	PA->MODER |= GPIO_MODER_MODER5_0; /* GPIO output */
#endif

	/******************* PINS PA2 and PA3 for USART2 **************************/
	/* PA2 et 3 in alternate function N°7 */
	PA->AFR[0] &= ~(0xF << (2*4) );	/* clear the 4 bits */
	PA->AFR[0] |= (7 << (2*4) ); 	/* set alternate function Nbr 7*/
	/* RX on PA3 alternate function 7 */
	PA->AFR[0] &= ~(0xF << (3*4) );	/* clear the 4 bits */
	PA->AFR[0] |= (7 << (3*4) );		/* set alternate function Nbr 7*/
	/* Configure alternate function for UART2 RX (PIN3) and TX (PIN2) */
	PA->MODER &= ~(3 << (2 * 2) );	/*TX*/
	PA->MODER &= ~(3 << (3 * 2) );	/*RX*/
	PA->MODER |= (2 << (2 * 2) );	/*TX*/
	PA->MODER |= (2 << (3 * 2) );	/*RX*/
	/************************** PINS PA2 and PA3 ******************************/

	/************************** PINS PB7 for TIM4 CH2 *************************/
	/* Enable GPIOB and clock */
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	/* PB7 as alternate function N°2*/
	PB->AFR[0] &= ~ GPIO_AFRL_AFSEL7_Msk	;
	PB->AFR[0] |= (0x2 << GPIO_AFRL_AFSEL7_Pos)	;	/* AF02 */
	PB->MODER &= ~GPIO_MODER_MODER7;
	PB->MODER |= GPIO_MODER_MODER7_1; /* alternate function */
	/************************** PINS PB7 for TIM4 CH2 *************************/

}

//#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(char* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1){

  }
}
//#endif
