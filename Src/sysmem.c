/**
 ******************************************************************************
 * @file      sysmem.c
 * @author    Nicolas Lacaille
 * @brief     System Memory calls for FreeRTOS
 ******************************************************************************
  */

/* Includes */
#include <errno.h>
#include <stdint.h>
#include <errno.h>
#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4xx.h"


/* Functions */

#if 1
/* wrap malloc on pvPortMalloc */

void * _malloc_r(struct _reent *reent, size_t nbytes) {
     return pvPortMalloc(nbytes);
};


void _free_r(struct _reent *reent,void* ptr)
{
        // We simply wrap the FreeRTOS call into a standard form
        vPortFree(ptr);
}

void *_realloc_r(struct _reent *reent, void *aptr, size_t nbytes)
{
	/* no equivalent function in free RTOS, some newlib functions use it, maybe better to trap
	 * it to debug  */
	return NULL;
}
#else
/* otherwise lock scheduller for _malloc, _free, _sbrk, ... */
void __malloc_lock()     {       vTaskSuspendAll(); };
void __malloc_unlock()   { (void)xTaskResumeAll();  };

/**
 _sbrk
 Increase program data space. Malloc and related functions depend on this
(if incr is negative _sbrk should free memory blocks)
 for FreeRTOS, we use the total space allocated @ link between
 .end (bss) and .end + _Min_Heap_Size
 or .end (bss) and _estack -  _Min_Stack_Size
 for the reent structure use
 #define configUSE_NEWLIB_REENTRANT              1
 in FreeRTOSConfig.h
**/

void * _sbrk_r(struct _reent *reent,int incr)
{
	extern char end asm("end");
	static char *heap_end;
	char *prev_heap_end,*start_stack_ptr,*min_stack_ptr;
	extern int *_Min_Stack_Size, *_Min_Heap_Size;	// from link script
	extern int errno;
	register char * stack_ptr asm("sp");
	volatile uint32_t msp;

	if (heap_end == 0)
		heap_end = &end;

	prev_heap_end = heap_end;

	/* here it only use the linker information do get the expected max value of
	 * stack from the linker
	 * Use SCB->VTOR register to locate the initial main stack pointer
	 * used at reset by main and ISR (the value at adress 0),
	 * should be &_estack (from linkscript */
	start_stack_ptr = (char*)(*(unsigned int *)*(unsigned int *)0xE000ED08);
	/* to get MSP to test stack overflow if needed */
	msp = __get_MSP();
	/* Locate the link script defined STACK bottom address */
	min_stack_ptr = start_stack_ptr - (unsigned) &_Min_Stack_Size;

	/* to test if there is enough memory use &_Min_Heap_Size or &_Min_Stack_Size */
	// if (heap_end + incr > &end + &_Min_Heap_Size) {/* not enough memory for heap */}
	if (heap_end + incr > min_stack_ptr){
		reent->_errno = ENOMEM; // newlib's thread-specific errno : _impure_ptr->_errno = ENOMEM;
		return (caddr_t) -1;
	}


	heap_end += incr;
	return (caddr_t) prev_heap_end;

}
#endif
