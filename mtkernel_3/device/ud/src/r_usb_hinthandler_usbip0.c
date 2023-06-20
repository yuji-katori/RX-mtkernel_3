/***********************************************************************************************************************
 * DISCLAIMER
 * This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
 * other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
 * applicable laws, including copyright laws.
 * THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
 * THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
 * EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
 * SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS
 * SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
 * this software. By using this software, you agree to the additional terms and conditions found by accessing the
 * following link:
 * http://www.renesas.com/disclaimer
 *
 * Copyright (C) 2014(2020) Renesas Electronics Corporation. All rights reserved.
 ***********************************************************************************************************************/
/***********************************************************************************************************************
 * File Name    : r_usb_hinthandler_usbip0.c
 * Description  : USB IP0 Host and Peripheral interrupt handler code
 ***********************************************************************************************************************/
/**********************************************************************************************************************
 * History : DD.MM.YYYY Version Description
 *         : 08.01.2014 1.00 First Release
 *         : 26.12.2014 1.10 RX71M is added
 *         : 30.09.2015 1.11 RX63N/RX631 is added.
 *         : 30.09.2016 1.20 RX65N/RX651 is added.
 *         : 31.03.2018 1.23 Supporting Smart Configurator
 *         : 01.03.2020 1.30 RX72N/RX66N is added and uITRON is supported.
 ***********************************************************************************************************************/

/******************************************************************************
 Includes   <System Includes> , "Project Includes"
 ******************************************************************************/

#include "r_usb_basic_if.h"
#include "r_usb_typedef.h"
#include "r_usb_extern.h"

#if (BSP_CFG_RTOS_USED != 0)        /* Use RTOS */
#include "r_rtos_abstract.h"
#include "r_usb_cstd_rtos.h"
#endif /* (BSP_CFG_RTOS_USED != 0) */

#if ((USB_CFG_MODE & USB_CFG_HOST) == USB_CFG_HOST)
/******************************************************************************
 Exported global variables (to be accessed by other files)
 ******************************************************************************/
#if (BSP_CFG_RTOS_USED == 0)    /* Non-OS */
usb_utr_t g_usb_cstd_int_msg[USB_NUM_USBIP][USB_INTMSGMAX]; /* Interrupt message */
uint16_t g_usb_cstd_int_msg_cnt[USB_NUM_USBIP]; /* Interrupt message count */
#endif /* (BSP_CFG_RTOS_USED == 0) */


/******************************************************************************
 Renesas Abstracted common Interrupt handler functions
 ******************************************************************************/

/******************************************************************************
 Function Name   : usb_hstd_usb_handler
 Description     : USB interrupt routine. Analyze which USB interrupt occurred 
                 : and send message to PCD/HCD task.
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void usb_hstd_usb_handler (void)
{
    usb_utr_t *ptr;

#if (BSP_CFG_RTOS_USED != 0)        /* Use RTOS */
    rtos_err_t ret;

    /* Initial pointer */
    ret = rtos_get_fixed_memory_isr(&g_rtos_usb_mpf_id, (void **)&ptr);
    if (RTOS_ERROR == ret)
    {
        return; /* Error */
    }
    ptr->msginfo = USB_MSG_HCD_INT;

    ptr->ip = USB_IP0;
    ptr->ipp = usb_hstd_get_usb_ip_adr(ptr->ip);

    /* Host Function */
    /* Host Interrupt handler */
    usb_hstd_interrupt_handler(ptr);
    ptr->msghead = (usb_mh_t) USB_NULL;

    /* Send message */
    rtos_send_mailbox_isr (&g_rtos_usb_hcd_mbx_id, (void *)ptr);

#else  /* BSP_CFG_RTOS_USED != 0 */
    /* Initial pointer */
    ptr = &g_usb_cstd_int_msg[0][g_usb_cstd_int_msg_cnt[0]];
    ptr->ip = USB_IP0;
    ptr->ipp = usb_hstd_get_usb_ip_adr(ptr->ip);

    /* Host Function */
    /* Host Interrupt handler */
    usb_hstd_interrupt_handler(ptr);
    ptr->msghead = (usb_mh_t) USB_NULL;

    /* Send message */
    USB_ISND_MSG(USB_HCD_MBX, (usb_msg_t* )ptr);

    /* Renewal Message count  */
    g_usb_cstd_int_msg_cnt[0]++;
    if (USB_INTMSGMAX == g_usb_cstd_int_msg_cnt[0])
    {
        g_usb_cstd_int_msg_cnt[0] = 0;
    }

#endif /* BSP_CFG_RTOS_USED != 0 */
    }
/******************************************************************************
 End of function usb_hstd_usb_handler
 ******************************************************************************/

#if (BSP_CFG_RTOS_USED == 0)    /* Non-OS */
/******************************************************************************
 Function Name   : usb_hstd_init_usb_message
 Description     : Initialization of message to be used at time of USB interrupt.
 Arguments       : usb_utr_t *ptr : Pointer to usb_utr_t structure.
 Return value    : none
 ******************************************************************************/
void usb_hstd_init_usb_message (usb_utr_t *ptr)
{
    /* TD.5.4 The interruption message of PCD is notified to HCD.  */
    uint16_t i;
    uint16_t msg_info;

    /* Host Function */
    msg_info = USB_MSG_HCD_INT;

    /* WAIT_LOOP */
    for (i = 0; i < USB_INTMSGMAX; i++)
    {
        g_usb_cstd_int_msg[ptr->ip][i].msginfo = msg_info;
    }

    g_usb_cstd_int_msg_cnt[ptr->ip] = 0;
}
/******************************************************************************
 End of function usb_hstd_init_usb_message
 ******************************************************************************/
#endif /* BSP_CFG_RTOS_USED == 0 */

#endif  /* (USB_CFG_MODE & USB_CFG_HOST) == USB_CFG_HOST */

/******************************************************************************
 End  Of File
 ******************************************************************************/
