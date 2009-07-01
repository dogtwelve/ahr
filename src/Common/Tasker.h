/*
   ARM TASK SWITCHER by dnicolier@gameloft.com

*/

#ifndef _TASKER_H_
#define _TASKER_H_

#ifndef WIN32

#include <setjmp.h>

inline void		task_save( void* ptr )
{
	asm volatile ( "stmia   r0, {v1-v6, sl, fp, sp, lr}" );
}

#define TASK_SAVE( ptr )	{					\
							(ptr)->valid = 1;	\
							setjmp( (long unsigned int*) ptr );	\
							}
/*
#define TASK_SAVE( ptr )	{												\
							(ptr)->valid = 1;								\
							asm volatile ( "mov r4, %0" ::"r"(ptr):"r4" );	\
							asm volatile ( "str r0, [r4, #0]" );			\
							asm volatile ( "mrs r0, CPSR" :::"r0");			\
							asm volatile ( "str r0, [r4, #60]" );			\
							asm volatile ( "mrs r0, SPSR" :::"r0");			\
							asm volatile ( "str r3, [r4, #64]" );			\
							asm volatile ( "str r1, [r4, #4]" );			\
							asm volatile ( "str r2, [r4, #8]" );			\
							asm volatile ( "str r3, [r4, #12]" );			\
							asm volatile ( "str r5, [r4, #16]" );			\
							asm volatile ( "str r6, [r4, #20]" );			\
							asm volatile ( "str r7, [r4, #24]" );			\
							asm volatile ( "str r8, [r4, #28]" );			\
							asm volatile ( "str r9, [r4, #32]" );			\
							asm volatile ( "str r10, [r4, #36]" );			\
							asm volatile ( "str r11, [r4, #40]" );			\ 
							asm volatile ( "str r12, [r4, #44]" );			\
							asm volatile ( "str r13, [r4, #48]" );			\
							asm volatile ( "str r14, [r4, #52]" );			\
							asm volatile ( "str r15, [r4, #56]" );			\
							}
  */

inline void		task_restore( void* ptr )
{
	asm volatile ( "ldmia     ip ,  {v1-v6, sl, fp, sp, pc}" );
}

#define TASK_RESTORE( ptr )	{						\
							(ptr)->valid = 0;		\
							longjmp( (long unsigned int*)ptr, 1 );	\
							}


/*#define TASK_RESTORE( ptr )	{												\
							(ptr)->valid = 0;								\
							asm volatile ( "mov r4, %0" ::"r"(ptr):"r4" );	\
							asm volatile ( "ldr r0, [r4, #60]" );			\
							asm volatile ( "msr CPSR, r0");					\
							asm volatile ( "ldr r0, [r4, #64]" );			\
							asm volatile ( "msr SPSR, r0");					\
							asm volatile ( "ldr r0, [r4, #0]" );			\
							asm volatile ( "ldr r1, [r4, #4]" );			\
							asm volatile ( "ldr r2, [r4, #8]" );			\
							asm volatile ( "ldr r3, [r4, #12]" );			\
							asm volatile ( "ldr r5, [r4, #16]" );			\
							asm volatile ( "ldr r6, [r4, #20]" );			\
							asm volatile ( "ldr r7, [r4, #24]" );			\
							asm volatile ( "ldr r8, [r4, #28]" );			\
							asm volatile ( "ldr r9, [r4, #32]" );			\
							asm volatile ( "ldr r10, [r4, #36]" );			\
							asm volatile ( "ldr r11, [r4, #40]" );			\
							asm volatile ( "ldr r12, [r4, #44]" );			\
							asm volatile ( "ldr r13, [r4, #48]" );			\
							asm volatile ( "ldr r14, [r4, #52]" );			\
							asm volatile ( "ldr r15, [r4, #56]" );			\
							}*/


#define TASK_SAVE_STACK( ptr )	{												\
								asm volatile ( "mov r4, %0" ::"r"(ptr):"r4" );	\
								asm volatile ( "str r13, [r4, #0]" );			\
								}


#define TASK_SET_STACK( ptr )	{												\
								asm volatile ( "mov r4, %0" ::"r"(ptr):"r4" );	\
								asm volatile ( "ldr r13, [r4, #0]" );			\
								}

#define TASK_ISVALID( a )		((a)->valid)


#define DBG_DUMP_REGS( a )			{\
									DBGPRINTF("r0 %x", a->regs[0]);\
									DBGPRINTF("r1 %x", a->regs[1]);\
									DBGPRINTF("r2 %x", a->regs[2]);\
									DBGPRINTF("r3 %x", a->regs[3]);\
									DBGPRINTF("r5 %x", a->regs[4]);\
									DBGPRINTF("r6 %x", a->regs[5]);\
									DBGPRINTF("r7 %x", a->regs[6]);\
									DBGPRINTF("r8 %x", a->regs[7]);\
									DBGPRINTF("r9 %x", a->regs[8]);\
									DBGPRINTF("r10 %x", a->regs[9]);\
									DBGPRINTF("r11 %x", a->regs[10]);\
									DBGPRINTF("r12 %x", a->regs[11]);\
									DBGPRINTF("r13 %x", a->regs[12]);\
									DBGPRINTF("r14 %x", a->regs[13]);\
									DBGPRINTF("r15 %x", a->regs[14]);\
									DBGPRINTF("cpsr %x", a->regs[15]);\
									DBGPRINTF("sprs %x", a->regs[16]);\
									}

#define TASK_BREAK( taskmain, taskcurr )	TASK_SAVE( taskcurr )			\
											if (TASK_ISVALID( taskcurr ))	\
											{\
												TASK_RESTORE( taskmain )	\
											}

#define TASK_HANDLER_INIT( _stack, _stacksize, _stacknew, _taskmain, _taskbreak )	\
					_stacknew->stack = (int*)(_stack + _stacksize - 4);			\
					_taskmain->valid = 0;										\
					_taskbreak->valid = 0;

#define TASK_HANDLER( function, stackmain, stacknew, taskmain, taskbreak )		\
					TASK_SAVE_STACK(stackmain);			\
					if (TASK_ISVALID(taskbreak))		\
					{									\
						TASK_SAVE(taskmain);			\
						if (TASK_ISVALID(taskmain))		\
							TASK_RESTORE(taskbreak);	\
					}									\
					else								\
					{									\
						TASK_SAVE(taskmain);			\
						if (TASK_ISVALID(taskmain))		\
						{								\
							TASK_SET_STACK(stacknew)	\
							function;					\
							TASK_SET_STACK(stackmain)	\
						}								\
					}

typedef struct
{
	int		regs[32/*17*/];	// from r0 to r15 (-r4)  + CPSR + SPSR
	int		valid;	// 0 = invalid

} TASK_DATA;

typedef struct
{
	int*	stack;

} TASK_STACK_DATA;

#endif

#endif