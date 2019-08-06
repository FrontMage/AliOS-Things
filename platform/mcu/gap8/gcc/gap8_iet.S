/******************************************************************************************
 *
 * Copyright (c) 2018 , GreenWaves Technologies, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of GreenWaves Technologies, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************************/


        .file "GAP8_iet.S"

/*******************************************************************************
	This file contains the Ecall handler and the Interrupt handler.
*******************************************************************************/

/*******************************************************************************
	EXTERNAL VARIABLES & FUNCTIONS
*******************************************************************************/
	//.extern _ecall_vector
        //.extern pxCurrentTCB
        //.extern xISRStack
/******************************************************************************/


/*******************************************************************************
	MACRO DEFINITION
*******************************************************************************/

.macro save_minimal_context
    addi sp, sp, -0x58
    sw  x1, 0x00(sp)  // ra
    sw  x3, 0x04(sp)  // gp
    sw  x4, 0x08(sp)  // tp
    sw  x5, 0x0c(sp)  // t0
    sw  x6, 0x10(sp)  // t1
    sw  x7, 0x14(sp)  // t2
    sw x10, 0x18(sp)  // a0
    sw x11, 0x1c(sp)  // a1
    sw x12, 0x20(sp)  // a2
    sw x13, 0x24(sp)  // a3
    sw x14, 0x28(sp)  // a4
    sw x15, 0x2c(sp)  // a5
    sw x16, 0x30(sp)  // a6
    sw x17, 0x34(sp)  // a7

    csrr a0, 0x7B0
    csrr a1, 0x7B1
    csrr a2, 0x7B2
    sw a0, 0x38(sp)  // lpstart[0]
    sw a1, 0x3c(sp)  // lpend[0]
    sw a2, 0x40(sp)  // lpcount[0]
    csrr a0, 0x7B4
    csrr a1, 0x7B5
    csrr a2, 0x7B6
    sw a0, 0x44(sp)  // lpstart[1]
    sw a1, 0x48(sp)  // lpend[1]
    sw a2, 0x4c(sp)  // lpcount[1]

    csrr a0, mepc
    sw a0, 0x50(sp)  // mepc
    csrr a0, mstatus
    sw a0, 0x54(sp)  // mstatus
.endm

.macro restore_minimal_context
    lw a0, 0x54(sp)  // mstatus
    csrrw x0, mstatus, a0
    lw a0, 0x50(sp)  // mepc
    csrrw x0, mepc, a0

    lw a0, 0x44(sp)  // lpstart[1]
    lw a1, 0x48(sp)  // lpend[1]
    lw a2, 0x4c(sp)  // lpcount[1]
    csrrw x0, 0x7B4, a0
    csrrw x0, 0x7B5, a1
    csrrw x0, 0x7B6, a2
    lw a0, 0x38(sp)  // lpstart[0]
    lw a1, 0x3c(sp)  // lpend[0]
    lw a2, 0x40(sp)  // lpcount[0]
    csrrw x0, 0x7B0, a0
    csrrw x0, 0x7B1, a1
    csrrw x0, 0x7B2, a2

    lw  x1, 0x00(sp)  // ra
    lw  x3, 0x04(sp)  // gp
    lw  x4, 0x08(sp)  // tp
    lw  x5, 0x0c(sp)  // t0
    lw  x6, 0x10(sp)  // t1
    lw  x7, 0x14(sp)  // t2
    lw x10, 0x18(sp)  // a0
    lw x11, 0x1c(sp)  // a1
    lw x12, 0x20(sp)  // a2
    lw x13, 0x24(sp)  // a3
    lw x14, 0x28(sp)  // a4
    lw x15, 0x2c(sp)  // a5
    lw x16, 0x30(sp)  // a6
    lw x17, 0x34(sp)  // a7
    addi sp, sp, 0x58
.endm

.macro save_extra_context
    lw t0, 0*0(tp)
    addi t0,   t0, -0x40
    sw   x8,   0x00(t0)
    sw   x9,   0x04(t0)
    sw   x18,  0x08(t0)
    sw   x19,  0x0c(t0)
    sw   x20,  0x10(t0)
    sw   x21,  0x14(t0)
    sw   x22,  0x18(t0)
    sw   x23,  0x1c(t0)
    sw   x24,  0x20(t0)
    sw   x25,  0x24(t0)
    sw   x26,  0x28(t0)
    sw   x27,  0x2c(t0)
    sw   x28,  0x30(t0)
    sw   x29,  0x34(t0)
    sw   x30,  0x38(t0)
    sw   x31,  0x3c(t0)
    lw   t1,   g_active_task
    sw   t0,   (t1)
.endm

.macro restore_extra_context
    /* Before calling this, the sp should be set the g_active_task stack pointer. */
    lw   x8,   0x00(sp)
    lw   x9,   0x04(sp)
    lw   x18,  0x08(sp)
    lw   x19,  0x0c(sp)
    lw   x20,  0x10(sp)
    lw   x21,  0x14(sp)
    lw   x22,  0x18(sp)
    lw   x23,  0x1c(sp)
    lw   x24,  0x20(sp)
    lw   x25,  0x24(sp)
    lw   x26,  0x28(sp)
    lw   x27,  0x2c(sp)
    lw   x28,  0x30(sp)
    lw   x29,  0x34(sp)
    lw   x30,  0x38(sp)
    lw   x31,  0x3c(sp)
    addi sp,   sp, 0x40
.endm


/******************************************************************************/
	.macro DECLARE Routine
	.global \Routine
	.func \Routine
	.type \Routine, %function
	\Routine:
	.endm
/******************************************************************************/

/******************************************************************************/
	.macro SAVE_CONTEXT

        lw sp, 0*0(tp)

        /* General purpose registers. */
	sw a6, 7*4(sp)
	sw a7, 8*4(sp)
        sw t0, 9*4(sp)
	sw t1, 10*4(sp)
	sw t2, 11*4(sp)
	sw t3, 12*4(sp)
	sw t4, 13*4(sp)
	sw t5, 14*4(sp)
	sw t6, 15*4(sp)
	sw s0, 16*4(sp)
	sw s1, 17*4(sp)
	sw s2, 18*4(sp)
	sw s3, 19*4(sp)
	sw s4, 20*4(sp)
	sw s5, 21*4(sp)
	sw s6, 22*4(sp)
	sw s7, 23*4(sp)
	sw s8, 24*4(sp)
	sw s9, 25*4(sp)
	sw s10, 26*4(sp)
	sw s11, 27*4(sp)

        /* MSTATUS */
        csrr t0, mstatus
        sw t0, 28*4(sp)

        /* MEPC */
        csrr t0, mepc
        sw t0, 29*4(sp)

        /* HW Loop regs. */
        csrr t0, 0x7B0
        sw t0, 30*4(sp)
        csrr t1, 0x7B1
        sw t1, 31*4(sp)
        csrr t2, 0x7B2
        sw t2, 32*4(sp)
        csrr t0, 0x7B4
        sw t0, 33*4(sp)
        csrr t1, 0x7B5
        sw t1, 34*4(sp)
        csrr t2, 0x7B6
        sw t2, 35*4(sp)
	.endm
/******************************************************************************/

/******************************************************************************/
	.macro RESTORE_CONTEXT

        //lw tp, pxCurrentTCB
        lw sp, 0*0(tp)

        /* HW Loop regs. */
        lw t2, 35*4(sp)
        csrw 0x7B6, t2
        lw t1, 34*4(sp)
        csrw 0x7B5, t1
        lw t0, 33*4(sp)
        csrw 0x7B4, t0
        lw t2, 32*4(sp)
        csrw 0x7B2, t2
        lw t1, 31*4(sp)
        csrw 0x7B1, t1
        lw t0, 30*4(sp)
        csrw 0x7B0, t0

        /* MEPC */
	lw t0, 29*4(sp)
	csrw mepc, t0

        /* MSTATUS */
	lw t0, 28*4(sp)
	csrw mstatus, t0

        /* General purpose registers. */
	lw s11, 27*4(sp)
	lw s10, 26*4(sp)
	lw s9, 25*4(sp)
	lw s8, 24*4(sp)
	lw s7, 23*4(sp)
	lw s6, 22*4(sp)
	lw s5, 21*4(sp)
	lw s4, 20*4(sp)
	lw s3, 19*4(sp)
	lw s2, 18*4(sp)
	lw s1, 17*4(sp)
	lw s0, 16*4(sp)
	lw t6, 15*4(sp)
	lw t5, 14*4(sp)
	lw t4, 13*4(sp)
	lw t3, 12*4(sp)
	lw t2, 11*4(sp)
	lw t1, 10*4(sp)
	lw t0, 9*4(sp)
	lw a7, 8*4(sp)
	lw a6, 7*4(sp)
	lw a5, 6*4(sp)
	lw a4, 5*4(sp)
	lw a3, 4*4(sp)
	lw a2, 3*4(sp)
	lw a1, 2*4(sp)
	lw a0, 1*4(sp)
	lw ra, 0*4(sp)

        addi sp, sp, (+35*4)
        mret
	.endm
/******************************************************************************/

.type systick_handler, %function
DECLARE systick_handler
    save_minimal_context
    lw tp, g_active_task
    sw sp, 0*0(tp)
    la sp, __irq_stack_start__

    jal ra, krhino_intrpt_enter
    jal ra, krhino_tick_proc
    jal ra, krhino_intrpt_exit

    /* Did the interrupt request a context switch? */
    lb a4, g_task_switch_required
    beqz a4, exit_without_switch

switch_before_exit:
    /*
     * A context swtich is to be performed. Clear the context switch
     * pending flag.
     */
    sb zero, g_task_switch_required, a5

    save_extra_context
    la    t0, g_active_task                         // g_active_task = g_preferred_ready_task;
    la    t1, g_preferred_ready_task
    lw    t2, (t1)
    sw    t2, (t0)

    lw    sp,   (t2)

    restore_extra_context /* sp is set to task stack pointer here. */
    j end_handler

exit_without_switch:
    lw sp, 0*0(tp)

end_handler:
    restore_minimal_context
    mret
.endfunc

/*******************************************************************************
    FC soc event handler
********************************************************************************/
.type fc_soc_event_handle_it, %function
DECLARE fc_soc_event_handle_it
    save_minimal_context
    lw tp, g_active_task
    sw sp, 0*0(tp)
    la sp, __irq_stack_start__

    jal ra, krhino_intrpt_enter
    jal ra, fc_soc_event_handler
    jal ra, krhino_intrpt_exit

    /* Did the interrupt request a context switch? */
    lb a4, g_task_switch_required
    beqz a4, exit_without_switch
    j switch_before_exit
.endfunc

/*******************************************************************************
    Cluster notify event handler
********************************************************************************/
.type cl_notify_fc_event_handler_it, %function
DECLARE cl_notify_fc_event_handler_it
    save_minimal_context
    lw tp, g_active_task
    sw sp, 0*0(tp)
    la sp, __irq_stack_start__

    jal ra, krhino_intrpt_enter
    jal ra, cl_notify_fc_event_handler
    jal ra, krhino_intrpt_exit

    /* Did the interrupt request a context switch? */
    lb a4, g_task_switch_required
    beqz a4, exit_without_switch
    j switch_before_exit
.endfunc

/*******************************************************************************
	ECALL HANDLER
*******************************************************************************/
	.section .text, "ax"
	DECLARE ecall_handler
	/* Save on the stack used registers. */
        addi sp, sp, (-8*4)
        sw ra, 0*4(sp)
        sw t0, 1*4(sp)
        sw t1, 2*4(sp)
	csrr t0, mepc
        addi t0, t0, 4
	sw t0, 3*4(sp)
        //lw tp, pxCurrentTCB
        sw sp, 0*0(tp)

        /* ISR Stack. */
        la sp, __irq_stack_start__
        lw sp, 0*0(sp)

	/* Search ecall :  a7 contains ecall number. */
	andi t0, a7, 0xf
	//la t1, _ecall_vector
	sll t0, t0, 0x2
	add t1, t1, t0
	lw t0, 0*4(t1)

	/* Execute demanded service. */
	jalr ra, t0

	/* Restore registers used. */
        lw sp, 0*0(tp)
        lw t0, 3*4(sp)
	csrw mepc, t0
        lw t1, 2*4(sp)
	lw t0, 1*4(sp)
        lw ra, 0*4(sp)

	addi sp, sp, (+8*4)
	mret
	.endfunc
/******************************************************************************/


/*******************************************************************************
	IRQ HANDLER
*******************************************************************************/
	/* Default handler, weak func, should be redefined to be overwritten. */
	.weak default_handler
	DECLARE default_handler
	j default_handler
	.endfunc

	/* PendSV_Handler. */
	DECLARE pendSV_handler
        addi sp, sp, (-35*4)
        sw ra, 0*4(sp)
        sw a0, 1*4(sp)
        sw a1, 2*4(sp)
        sw a2, 3*4(sp)
        sw a3, 4*4(sp)
        sw a4, 5*4(sp)
        sw a5, 6*4(sp)
        sw sp, 0*0(tp)

        /* ISR Stack. */
        la sp, __irq_stack_start__
        lw sp, 0*0(sp)
        beq tp, a0, _no_switch_pendsv
        SAVE_CONTEXT
        RESTORE_CONTEXT
_no_switch_pendsv:
        lw sp, 0*0(tp)
        lw ra, 0*4(sp)
        lw a0, 1*4(sp)
        lw a1, 2*4(sp)
        lw a2, 3*4(sp)
        lw a3, 4*4(sp)
        lw a4, 5*4(sp)
        lw a5, 6*4(sp)
        addi sp, sp, (+35*4)
	mret
	.endfunc
/******************************************************************************/


/*******************************************************************************
	TO DO : ILL_INS HANDLER
*******************************************************************************/
	DECLARE ill_ins_handler
	j ill_ins_handler
	.endfunc
/******************************************************************************/
