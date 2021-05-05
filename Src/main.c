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
#include <string.h>

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define STACK_SIZE	512 // in words (= 4*STACK_SIZE bytes)

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
volatile SemaphoreHandle_t my_mutex;
volatile SemaphoreHandle_t my_binary_semaphore;

/* Private function prototypes -----------------------------------------------*/
void pin_init(void);
void T0( void * pvParameters );
void T1( void * pvParameters );
void T2( void * pvParameters );

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
	BaseType_t xReturned0;
	TaskHandle_t xHandle0 = 0;

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

	// première tache pour faire clignoter la led
    xReturned0 = xTaskCreate(
                    T0,       /* Function that implements the task. */
                    "Clignoter led",          /* Text name for the task. */
                    STACK_SIZE,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    tskIDLE_PRIORITY+7,/* Priority at which the task is created. */
                    &xHandle0 );      /* Used to pass out the created task's handle. */

    BaseType_t helloReturnTasks[2];
    TaskHandle_t handleHelloTasks[2] = {0};
    char name[30] = {0};
    char number[2] = {0};
    for(uint8_t i = 0; i < 2; ++i) {
    	strcpy(name, "Hello task ");
    	number[0] = '0' + i;
    	strcat(name, number);

    	// on a créé notre première tache pour allumer la led
    	helloReturnTasks[i] = xTaskCreate(
                        T2,       /* Function that implements the task. */
                        name,          /* Text name for the task. */
                        STACK_SIZE,      /* Stack size in words, not bytes. */
                        ( void * ) 2+i,    /* Parameter passed into the task. */
                        tskIDLE_PRIORITY+6,/* Priority at which the task is created. */
                        &(handleHelloTasks[i]) ); // manage task with handle
    }

    if( xReturned0 == pdPASS && helloReturnTasks[0] == pdPASS && helloReturnTasks[1] == pdPASS){
        /* The task was created.  The task's handle can be used
         * to delete the task. : vTaskDelete( xHandle ); */
    }else{
    	// not enough memory to create task
    	//printf(" not enough memory to create task T0 \n");
    }

    // create mutex
    my_mutex = xSemaphoreCreateMutex();
    my_binary_semaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(my_binary_semaphore);

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
	TickType_t tick = xTaskGetTickCount();
	// ininite loop :
	for( ;; ){
		// allumer led
		uint8_t state = !(GPIOA->ODR & GPIO_ODR_OD5_Msk);
		GPIOA->ODR &= ~(GPIOA->ODR & GPIO_ODR_OD5_Msk);
		GPIOA->ODR |= state << GPIO_ODR_OD5_Pos;

		if(state) {
			USART2_Transmit((uint8_t*) "jour\r\n", sizeof("jour\r\n"));
		} else {
			USART2_Transmit((uint8_t*) "nuit\r\n", sizeof("nuit\r\n"));
		}

		vTaskDelayUntil(&tick, configTICK_RATE_HZ/2);
	}
}

/**
 * @brief	Task T2
 * @param	Integer with task index
 * @retval	none (should not return)
 */
void T2( void * pvParameters )
{
	// on transforme notre entier en string
	char number[11] = {0};
	snprintf(number, 10, "%lu", (uint32_t) pvParameters);

	// on crée notre buffer
	char helloSentence[30] = "Hello from task ";

	// on ajoute notre nombre a la phrase
	strcat(helloSentence, number);

	// on ajoute un retour a la ligne
	strcat(helloSentence, "\r\n");

	// ininite loop :
	for( ;; ){
		// Afficher message polling
		// Attente max pour être sur de l'obtenir
		BaseType_t semTaken = xSemaphoreTake(my_binary_semaphore, portMAX_DELAY);
		if(semTaken == pdTRUE) {
			USART2_Transmit((uint8_t*) helloSentence, strlen(helloSentence));
			xSemaphoreGive(my_binary_semaphore);
		}
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
