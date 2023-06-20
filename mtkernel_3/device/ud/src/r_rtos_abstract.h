/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	r_rtos_abstract.h
 */
#ifndef R_RTOS_ABSTRACT_H
#define R_RTOS_ABSTRACT_H

#include "platform.h"
#include "tk/tkernel.h"

#define	RTOS_FOREVER		(-1)
#define	RTOS_ZERO		(0)
#define	RTOS_TMO		(3000)

typedef	ID			rtos_task_id_t;
typedef	ID			rtos_mbx_id_t;
typedef	ID			rtos_mem_id_t;
typedef	ID			rtos_sem_id_t;
typedef	TMO			rtos_time_t;

typedef uint8_t			rtos_semaphore_count_t;

typedef struct rtos_task_info
{
	ATR		tskatr;
	void	       *exinf;
	    FP              task;
    PRI             itskpri;
//    SIZE            stksz;
//    VP              stk;
} rtos_task_info_t;

typedef struct rtos_mbx_info
{
    ATR         mbxatr;
    PRI         maxpri;
//    VP          mprihd;
} rtos_mbx_info_t;

typedef struct rtos_mpf_info 
{
    ATR             mpfatr;
    UINT            blkcnt;
    UINT            blksz;
//    VP              mpf;
} rtos_mpf_info_t;

typedef struct rtos_sem_info 
{
    ATR             sematr;
    UINT            initial_count;
    UINT            max_count;
} rtos_sem_info_t;

typedef enum e_usb_rtos_err
{
    RTOS_SUCCESS,                   /* Success */
    RTOS_ERROR,                     /* Error */
} rtos_err_t;

/* For Fixed type memory pool */
rtos_err_t      rtos_create_fixed_memory(rtos_mem_id_t *p_id, rtos_mpf_info_t *p_info);
rtos_err_t      rtos_delete_fixed_memory(rtos_mem_id_t *p_id);
rtos_err_t      rtos_get_fixed_memory(rtos_mem_id_t *p_id, void **pp_memblk, rtos_time_t tmo_val);
rtos_err_t      rtos_get_fixed_memory_isr(rtos_mem_id_t *p_id, void **pp_memblk);
rtos_err_t      rtos_release_fixed_memory(rtos_mem_id_t *p_id, void *p_memblk);

/* For task */
rtos_err_t      rtos_create_task (rtos_task_id_t *p_id, rtos_task_info_t *p_info);
rtos_err_t      rtos_delete_task (rtos_task_id_t *p_id);
rtos_err_t      rtos_get_task_id (rtos_task_id_t *p_id);
rtos_err_t      rtos_start_task  (rtos_task_id_t *p_id);

/* For mailbox */
rtos_err_t      rtos_create_mailbox (rtos_mbx_id_t *p_id, rtos_mbx_info_t *p_info);
rtos_err_t      rtos_delete_mailbox (rtos_mbx_id_t *p_id);
rtos_err_t      rtos_send_mailbox (rtos_mbx_id_t *p_id, void *p_message);
rtos_err_t      rtos_send_mailbox_isr (rtos_mbx_id_t *p_id, void *p_message);
rtos_err_t      rtos_receive_mailbox (rtos_mbx_id_t *p_id, void **pp_message, rtos_time_t tmo_val);

/* For semaphore */
rtos_err_t      rtos_create_semaphore (rtos_sem_id_t *p_id, rtos_sem_info_t *p_info);
rtos_err_t      rtos_delete_semaphore (rtos_sem_id_t *p_id);
rtos_err_t      rtos_get_semaphore (rtos_sem_id_t *p_id, rtos_time_t tmo_val);
rtos_err_t      rtos_release_semaphore (rtos_sem_id_t *p_id);

#endif	/* R_RTOS_ABSTRACT_H */
