#ifndef __USART_H
#define __USART_H

/* exported function for USART2 */

// init baud rate
void USART2_Init(uint32_t baud);

// simple polling blocking functions
int __io_putchar(int ch) ;
int __io_getchar(void) ;


// transmission of frames
// polling blocking function
uint32_t USART2_Transmit(uint8_t * data, uint32_t len);
// asynchronous function using interrupt
int32_t USART2_Transmit_IRQ( uint8_t * data, uint32_t len);


// reception of frames
// polling blocking function
uint32_t USART2_Receive(uint8_t * data, uint32_t len);
// asynchronous function using interrupt
int32_t USART2_Receive_IRQ( uint8_t * data, uint32_t len);
// helper functions
// to test if a reception has completed
int32_t USART2_receive_complete_test(void);
// to get number of data received
uint32_t USART2_get_number_received(void);
// to ckow if the reception is busy
int32_t USART2_receive_not_busy(void);

// to reset the receive complete flag
void USART2_receive_complete_clear(void);

#endif /* __USART_H */

