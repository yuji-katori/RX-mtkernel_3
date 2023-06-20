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
 * File Name    : r_usb_hreg_abs.c
 * Description  : Call USB Host register access function 
 ***********************************************************************************************************************/
/**********************************************************************************************************************
 * History : DD.MM.YYYY Version Description
 *         : 08.01.2014 1.00 First Release
 *         : 26.12.2014 1.10 RX71M is added
 *         : 30.09.2015 1.11 RX63N/RX631 is added.
 *         : 30.09.2016 1.20 RX65N/RX651 is added.
 *         : 30.09.2017 1.22 RX62N/RX630/RX63T-H is added.
 *         : 31.03.2018 1.23 Supporting Smart Configurator
 *         : 31.05.2019 1.26 Added support for GNUC and ICCRX.
 *         : 30.07.2019 1.27 RX72M is added.
 *         : 01.03.2020 1.30 RX72N/RX66N is added and uITRON is supported.
 *         : 30.04.2020 1.31 RX671 is added.
 ***********************************************************************************************************************/

/******************************************************************************
 Includes   <System Includes> , "Project Includes"
 ******************************************************************************/

#include "r_usb_basic_if.h"
#include "r_usb_typedef.h"
#include "r_usb_extern.h"
#include "r_usb_bitdefine.h"
#include "r_usb_reg_access.h"
#if (BSP_CFG_RTOS_USED != 0)        /* Use RTOS */
#include "r_rtos_abstract.h"
#include "r_usb_cstd_rtos.h"
#endif /* (BSP_CFG_RTOS_USED != 0) */


#if ((USB_CFG_MODE & USB_CFG_HOST) == USB_CFG_HOST)
/******************************************************************************
 Function Name   : usb_hstd_set_hub_port
 Description     : Set up-port hub
 Arguments       : usb_utr_t *ptr    : Pointer to usb_utr_t structure.
                 : uint16_t addr     : device address
                 : uint16_t upphub   : up-port hub address
                 : uint16_t hubport  : hub port number
 Return value    : none
 ******************************************************************************/
void usb_hstd_set_hub_port (usb_utr_t *ptr, uint16_t addr, uint16_t upphub, uint16_t hubport)
{
#if defined(BSP_MCU_RX71M)
    if (USB_IP1 == ptr->ip)
    {
        hw_usb_hrmw_devadd(ptr, addr, (upphub|hubport), (uint16_t)(USB_UPPHUB | USB_HUBPORT));
    }
#endif  /* defined(BSP_MCU_RX71M) */
}
/******************************************************************************
 End of function usb_hstd_set_hub_port
 ******************************************************************************/

/******************************************************************************
 Function Name   : usb_hstd_interrupt_handler
 Description     : Analyzes which USB interrupt is generated
 Arguments       : usb_utr_t *ptr    : Pointer to usb_utr_t structure.
 Return          : none
 ******************************************************************************/
void usb_hstd_interrupt_handler (usb_utr_t *ptr)
{
    uint16_t intsts0;
    uint16_t intenb0;
    uint16_t ists0;

    uint16_t intsts1;
    uint16_t intenb1;
    uint16_t ists1;

    uint16_t brdysts;
    uint16_t brdyenb;
    uint16_t bsts;

    uint16_t nrdysts;
    uint16_t nrdyenb;
    uint16_t nsts;

    uint16_t bempsts;
    uint16_t bempenb;
    uint16_t ests;

    intsts0 = ptr->ipp->INTSTS0.WORD;
    intsts1 = ptr->ipp->INTSTS1.WORD;
    brdysts = ptr->ipp->BRDYSTS.WORD;
    nrdysts = ptr->ipp->NRDYSTS.WORD;
    bempsts = ptr->ipp->BEMPSTS.WORD;
    intenb0 = ptr->ipp->INTENB0.WORD;
    intenb1 = ptr->ipp->INTENB1.WORD;
    brdyenb = ptr->ipp->BRDYENB.WORD;
    nrdyenb = ptr->ipp->NRDYENB.WORD;
    bempenb = ptr->ipp->BEMPENB.WORD;

    /* Interrupt Status Get */
    ptr->keyword = USB_INT_UNKNOWN;
    ptr->status = 0;

    ists0 = (uint16_t) (intsts0 & intenb0);
    ists1 = (uint16_t) (intsts1 & intenb1);

    /*  ists2 = (uint16_t)(intsts2 & intenb2);*/
    bsts = (uint16_t) (brdysts & brdyenb);
    nsts = (uint16_t) (nrdysts & nrdyenb);
    ests = (uint16_t) (bempsts & bempenb);

    /***** Processing Setup transaction *****/
    if (USB_SACK == (ists1 & USB_SACK))
    {
        /***** Setup ACK *****/
        /* SACK Clear */
        ptr->ipp->INTSTS1.WORD = (uint16_t) ((~USB_SACK) & INTSTS1_MASK);

        /* Setup Ignore,Setup Acknowledge disable */
        ptr->ipp->INTENB1.WORD &= (uint16_t) ~(USB_SIGNE | USB_SACKE);
        ptr->keyword = USB_INT_SACK;
    }
    else if (USB_SIGN == (ists1 & USB_SIGN))
    {
        /***** Setup Ignore *****/
        /* SIGN Clear */
        ptr->ipp->INTSTS1.WORD = (uint16_t) ((~USB_SIGN) & INTSTS1_MASK);

        /* Setup Ignore,Setup Acknowledge disable */
        ptr->ipp->INTENB1.WORD &= (uint16_t) ~((USB_SIGNE) | USB_SACKE);
        ptr->keyword = USB_INT_SIGN;
    }

    /***** Processing PIPE0-MAX_PIPE_NO data *****/
    else if (USB_BRDY == (ists0 & USB_BRDY)) /***** EP0-7 BRDY *****/
    {
        ptr->ipp->BRDYSTS.WORD = (uint16_t) ((~bsts) & BRDYSTS_MASK);
        ptr->keyword = USB_INT_BRDY;
        ptr->status = bsts;
    }
    else if (USB_BEMP == (ists0 & USB_BEMP)) /***** EP0-7 BEMP *****/
    {
        ptr->ipp->BEMPSTS.WORD = (uint16_t) ((~ests) & BEMPSTS_MASK);
        ptr->keyword = USB_INT_BEMP;
        ptr->status = ests;
    }
    else if (USB_NRDY == (ists0 & USB_NRDY)) /***** EP0-7 NRDY *****/
    {
        ptr->ipp->NRDYSTS.WORD = (uint16_t) ((~nsts) & NRDYSTS_MASK);
        ptr->keyword = USB_INT_NRDY;
        ptr->status = nsts;
    }

    /***** Processing rootport0 *****/
    else if (USB_OVRCR == (ists1 & USB_OVRCR)) /***** OVER CURRENT *****/
    {
        /* OVRCR Clear */
        ptr->ipp->INTSTS1.WORD = (uint16_t) ((~USB_OVRCR) & INTSTS1_MASK);
        ptr->keyword = USB_INT_OVRCR0;
    }
    else if (USB_ATTCH == (ists1 & USB_ATTCH)) /***** ATTCH INT *****/
    {
        /* DTCH  interrupt disable */
        usb_hstd_bus_int_disable(ptr);
        ptr->keyword = USB_INT_ATTCH0;
    }
    else if (USB_EOFERR == (ists1 & USB_EOFERR)) /***** EOFERR INT *****/
    {
        /* EOFERR Clear */
        ptr->ipp->INTSTS1.WORD = (uint16_t) ((~USB_EOFERR) & INTSTS1_MASK);
        ptr->keyword = USB_INT_EOFERR0;
    }
    else if (USB_BCHG == (ists1 & USB_BCHG)) /***** BCHG INT *****/
    {
        /* BCHG  interrupt disable */
        usb_hstd_bchg_disable(ptr);
        ptr->keyword = USB_INT_BCHG0;
    }
    else if (USB_DTCH == (ists1 & USB_DTCH)) /***** DETACH *****/
    {
        /* DTCH  interrupt disable */
        usb_hstd_bus_int_disable(ptr);
        ptr->keyword = USB_INT_DTCH0;
    }
#if USB_CFG_BC == USB_CFG_ENABLE
    else if (USB_PDDETINT == (ists1 & USB_PDDETINT)) /***** PDDETINT INT *****/
    {
        if (USB_IP1 == ptr -> ip)
        {
            /* PDDETINT  interrupt disable */
            ptr->ipp1->INTSTS1.WORD = (uint16_t) ((~USB_PDDETINT) & INTSTS1_MASK);
            ptr->keyword = USB_INT_PDDETINT0;
        }
    }
#endif  /* USB_CFG_BC == USB_CFG_ENABLE */
    /***** Processing VBUS/SOF *****/
    else if (USB_VBINT == (ists0 & USB_VBINT)) /***** VBUS change *****/
    {
        /* Status Clear */
        ptr->ipp->INTSTS0.WORD = (uint16_t) ~USB_VBINT;
        ptr->keyword = USB_INT_VBINT;
    }
    else if (USB_SOFR == (ists0 & USB_SOFR)) /***** SOFR change *****/
    {
        /* SOFR Clear */
        ptr->ipp->INTSTS0.WORD = (uint16_t) ~USB_SOFR;
        ptr->keyword = USB_INT_SOFR;
    }

    else
    {
        /* Non */
    }
}
/******************************************************************************
 End of function usb_hstd_interrupt_handler
 ******************************************************************************/

/******************************************************************************
 Function Name   : usb_hstd_chk_attach
 Description     : Checks whether USB Device is attached or not and return USB speed
                 : of USB Device
 Arguments       : usb_utr_t *ptr    : Pointer to usb_utr_t structure.
 Return value    : uint16_t          : connection status
                 :                   : (USB_ATTACHF/USB_ATTACHL/USB_DETACH/USB_OK)
 Note            : Please change for your SYSTEM
 ******************************************************************************/
uint16_t usb_hstd_chk_attach (usb_utr_t *ptr)
{
    uint16_t buf[3];

    usb_hstd_read_lnst(ptr, buf);

    if (USB_UNDECID == (uint16_t) (buf[1] & USB_RHST))
    {
        if (USB_FS_JSTS == (buf[0] & USB_LNST))
        {
            /* High/Full speed device */
            USB_PRINTF0(" Detect FS-J\n");
#if defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX71M)
            usb_hstd_set_hse(ptr, g_usb_hstd_hs_enable[ptr->ip]);
#endif  /* defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX71M) */
            return USB_ATTACHF;
        }
        else if (USB_LS_JSTS == (buf[0] & USB_LNST))
        {
            /* Low speed device */
            USB_PRINTF0(" Attach LS device\n");
#if defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX71M)
            usb_hstd_set_hse(ptr, USB_HS_DISABLE);
#endif  /* defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX71M) */
            return USB_ATTACHL;
        }
        else if (USB_SE0 == (buf[0] & USB_LNST))
        {
            USB_PRINTF0(" Detach device\n");
        }
        else
        {
            USB_PRINTF0(" Attach unknown speed device\n");
        }
    }
    else
    {
        USB_PRINTF0(" Already device attached\n");
        return USB_OK;
    }
    return USB_DETACH;
}
/******************************************************************************
 End of function usb_hstd_chk_attach
 ******************************************************************************/

/******************************************************************************
 Function Name   : usb_hstd_chk_clk
 Description     : Checks SOF sending setting when USB Device is detached or suspended
                 : , BCHG interrupt enable setting and clock stop processing
 Arguments       : usb_utr_t *ptr    : Pointer to usb_utr_t structure.
                 : uint16_t event    : device state
 Return value    : none
 ******************************************************************************/
void usb_hstd_chk_clk (usb_utr_t *ptr, uint16_t event)
{

    if ((USB_DETACHED == g_usb_hstd_mgr_mode[ptr->ip]) || (USB_SUSPENDED == g_usb_hstd_mgr_mode[ptr->ip]))
    {
        usb_hstd_chk_sof(ptr);

        /* Enable port BCHG interrupt */
        usb_hstd_bchg_enable(ptr);
    }

} /* End of function usb_hstd_chk_clk */
/******************************************************************************
 End of function usb_hstd_chk_clk
 ******************************************************************************/

/******************************************************************************
 Function Name   : usb_hstd_detach_process
 Description     : Handles the require processing when USB device is detached
                 : (Data transfer forcibly termination processing to the connected USB Device,
                 : the clock supply stop setting and the USB interrupt dissable setteing etc)
 Arguments       : usb_utr_t *ptr    : Pointer to usb_utr_t structure.
 Return value    : none
 ******************************************************************************/
void usb_hstd_detach_process (usb_utr_t *ptr)
{
    uint16_t connect_inf;
    uint16_t md;
    uint16_t i;
    uint16_t addr;

    /* ATTCH interrupt disable */
    usb_hstd_attch_disable(ptr);

    /* DTCH  interrupt disable */
    usb_hstd_dtch_disable(ptr);
    usb_hstd_bchg_disable(ptr);

    /* WAIT_LOOP */
    for (md = 1u; md < (USB_MAXDEVADDR + 1u); md++)
    {
        addr = (uint16_t) (md << USB_DEVADDRBIT);
        if (usb_hstd_chk_dev_addr(ptr, addr) != USB_NOCONNECT)
        {
            if (USB_IDLEST != g_usb_hstd_ctsq[ptr->ip])
            {
                /* Control Read/Write End */
                usb_hstd_ctrl_end(ptr, (uint16_t) USB_DATA_ERR);
            }
            /* WAIT_LOOP */
            for (i = USB_MIN_PIPE_NO; i <= USB_MAX_PIPE_NO; i++)
            {
                /* Not control transfer */
                /* Agreement device address */
                if (usb_hstd_get_devsel(ptr, i) == addr)
                {
                    /* PID=BUF ? */
                    if (USB_PID_BUF == usb_cstd_get_pid(ptr, i))
                    {
                        /* End of data transfer (IN/OUT) */
                        usb_hstd_forced_termination(ptr, i, (uint16_t) USB_DATA_STOP);
                    }
                    usb_cstd_clr_pipe_cnfg(ptr, i);
                }
            }
            usb_hstd_set_dev_addr(ptr, addr, USB_NOCONNECT);
            usb_hstd_set_hub_port(ptr, addr, USB_NULL, USB_NULL);
            USB_PRINTF1("*** Device address %d clear.\n",md);
        }
    }

    /* Decide USB Line state (ATTACH) */
    connect_inf = usb_hstd_chk_attach(ptr);
    switch (connect_inf)
    {
        case USB_ATTACHL :
            usb_hstd_attach(ptr, connect_inf);
        break;
        case USB_ATTACHF :
            usb_hstd_attach(ptr, connect_inf);
        break;
        case USB_DETACH :

            /* USB detach */
            usb_hstd_detach(ptr);

            /* Check clock */
            usb_hstd_chk_clk(ptr, (uint16_t) USB_DETACHED);
        break;
        default :

            /* USB detach */
            usb_hstd_detach(ptr);

            /* Check clock */
            usb_hstd_chk_clk(ptr, (uint16_t) USB_DETACHED);
        break;
    }
}
/******************************************************************************
 End of function usb_hstd_detach_process
 ******************************************************************************/

/******************************************************************************
 Function Name   : usb_hstd_read_lnst
 Description     : Reads LNST register two times, checks whether these values
                 : are equal and returns the value of DVSTCTR register that correspond to
                 : the port specified by 2nd argument.
 Arguments       : usb_utr_t *ptr    : Pointer to usb_utr_t structure.
                 : uint16_t *buf     : Pointer to the buffer to store DVSTCTR register
 Return value    : none
 Note            : Please change for your SYSTEM
 ******************************************************************************/
void usb_hstd_read_lnst (usb_utr_t *ptr, uint16_t *buf)
{
    /* WAIT_LOOP */
    do
    {
        buf[0] = hw_usb_read_syssts(ptr);

        /* 30ms wait */
        usb_cpu_delay_xms((uint16_t) 30);
        buf[1] = hw_usb_read_syssts(ptr);
        if ((buf[0] & USB_LNST) == (buf[1] & USB_LNST))
        {
            /* 20ms wait */
            usb_cpu_delay_xms((uint16_t) 20);
            buf[1] = hw_usb_read_syssts(ptr);
        }
    } while ((buf[0] & USB_LNST) != (buf[1] & USB_LNST));
    buf[1] = hw_usb_read_dvstctr(ptr);
}
/******************************************************************************
 End of function usb_hstd_read_lnst
 ******************************************************************************/

/******************************************************************************
 Function Name   : usb_hstd_attach_process
 Description     : Interrupt disable setting when USB Device is attached and
                 : handles the required interrupt disable setting etc when USB device
                 : is attached.
 Arguments       : usb_utr_t *ptr    : Pointer to usb_utr_t structure.
 Return value    : none
 Note            : Please change for your SYSTEM
 ******************************************************************************/
void usb_hstd_attach_process (usb_utr_t *ptr)
{
    uint16_t connect_inf;

    /* ATTCH interrupt disable */
    usb_hstd_attch_disable(ptr);

    /* DTCH  interrupt disable */
    usb_hstd_dtch_disable(ptr);
    usb_hstd_bchg_disable(ptr);

    /* Decide USB Line state (ATTACH) */
    connect_inf = usb_hstd_chk_attach(ptr);
    switch (connect_inf)
    {
        case USB_ATTACHL :
            usb_hstd_attach(ptr, connect_inf);
        break;
        case USB_ATTACHF :
            usb_hstd_attach(ptr, connect_inf);
        break;
        case USB_DETACH :

            /* USB detach */
            usb_hstd_detach(ptr);

            /* Check clock */
            usb_hstd_chk_clk(ptr, (uint16_t) USB_DETACHED);
        break;
        default :
            usb_hstd_attach(ptr, (uint16_t) USB_ATTACHF);
        break;
    }
}
/******************************************************************************
 End of function usb_hstd_attach_process
 ******************************************************************************/

/******************************************************************************
 Function Name   : usb_hstd_chk_sof
 Description     : Checks whether SOF is sended or not
 Arguments       : usb_utr_t *ptr    : Pointer to usb_utr_t structure.
 Return value    : none
 ******************************************************************************/
void usb_hstd_chk_sof (usb_utr_t *ptr)
{
    usb_cpu_delay_1us((uint16_t) 1); /* Wait 640ns */
}
/******************************************************************************
 End of function usb_hstd_chk_sof
 ******************************************************************************/

/******************************************************************************
 Function Name   : usb_hstd_bus_reset
 Description     : Setting USB register when BUS Reset
 Arguments       : usb_utr_t *ptr    : Pointer to usb_utr_t structure.
 Return value    : none
 ******************************************************************************/
void usb_hstd_bus_reset (usb_utr_t *ptr)
{
    uint16_t buf;
    uint16_t i;

    /* USBRST=1, UACT=0 */
    hw_usb_rmw_dvstctr(ptr, USB_USBRST, (USB_USBRST | USB_UACT));

    /* Wait 50ms */
    usb_cpu_delay_xms((uint16_t) 50);
    if (USB_IP1 == ptr->ip)
    {
        /* USBRST=0 */
        hw_usb_clear_dvstctr(ptr, USB_USBRST); /* for UTMI */
        usb_cpu_delay_1us(300);                            /* for UTMI */
    }

    /* USBRST=0, RESUME=0, UACT=1 */
    usb_hstd_set_uact(ptr);

    /* Wait 10ms or more (USB reset recovery) */
    usb_cpu_delay_xms((uint16_t) 20);
    /* WAIT_LOOP */
    for (i = 0, buf = USB_HSPROC; (i < 3) && (USB_HSPROC == buf); ++i)
    {
        /* DeviceStateControlRegister - ResetHandshakeStatusCheck */
        buf = hw_usb_read_dvstctr(ptr);
        buf = (uint16_t) (buf & USB_RHST);
        if (USB_HSPROC == buf)
        {
            /* Wait */
            usb_cpu_delay_xms((uint16_t) 10);
        }
    }

    /* 30ms wait */
    usb_cpu_delay_xms((uint16_t) 30);
}
/******************************************************************************
 End of function usb_hstd_bus_reset
 ******************************************************************************/

/******************************************************************************
 Function Name   : usb_hstd_resume_process
 Description     : Setting USB register when RESUME signal is detected
 Arguments       : usb_utr_t *ptr    : Pointer to usb_utr_t structure.
 Return value    : none
 ******************************************************************************/
void usb_hstd_resume_process (usb_utr_t *ptr)
{
    uint16_t buf;

    usb_hstd_bchg_disable(ptr);

    buf = hw_usb_read_dvstctr(ptr);

    /* Reset handshake status get */

    if (buf & USB_RESUME)
    {
        /* RWUPE=0 */
        hw_usb_clear_dvstctr(ptr, USB_RWUPE);
    }
    else
    {
        /* RESUME=1, RWUPE=0 */
        hw_usb_rmw_dvstctr(ptr, USB_RESUME, (USB_RESUME | USB_RWUPE));
    }

    /* Wait */
    usb_cpu_delay_xms((uint16_t) 20);

    /* USBRST=0, RESUME=0, UACT=1 */
    usb_hstd_set_uact(ptr);

    /* Wait */
    usb_cpu_delay_xms((uint16_t) 5);
}
/******************************************************************************
 End of function usb_hstd_resume_process
 ******************************************************************************/

/******************************************************************************
 Function Name   : usb_hstd_support_speed_check
 Description     : Get USB-speed of the specified port.
 Arguments       : usb_utr_t *ptr   : Pointer to usb_utr_t structure.
 Return value    : uint16_t         : HSCONNECT : Hi-Speed
                 :                  : FSCONNECT : Full-Speed
                 :                  : LSCONNECT : Low-Speed
                 :                  : NOCONNECT : not connect
 ******************************************************************************/
uint16_t usb_hstd_support_speed_check (usb_utr_t *ptr)
{
    uint16_t buf;
    uint16_t conn_inf;

    buf = hw_usb_read_dvstctr(ptr);

    /* Reset handshake status get */
    buf = (uint16_t) (buf & USB_RHST);

    switch (buf)
    {
        /* Get port speed */
        case USB_HSMODE :
            conn_inf = USB_HSCONNECT;
        break;
        case USB_FSMODE :
            conn_inf = USB_FSCONNECT;
        break;
        case USB_LSMODE :
#if defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX65N) || defined(BSP_MCU_RX71M) || defined(BSP_MCU_RX72T)\
    || defined (BSP_MCU_RX72M) || defined (BSP_MCU_RX72N) || defined (BSP_MCU_RX66N) || defined(BSP_MCU_RX671)
            conn_inf = USB_LSCONNECT;
#else   /* defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX65N) || defined(BSP_MCU_RX71M) || defined(BSP_MCU_RX72T)\
    || defined (BSP_MCU_RX72M) || defined (BSP_MCU_RX72N) || defined (BSP_MCU_RX66N) || defined(BSP_MCU_RX671) */
            conn_inf = USB_NOCONNECT;
#endif  /* defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX65N) || defined(BSP_MCU_RX71M) || defined(BSP_MCU_RX72T)\
    || defined (BSP_MCU_RX72M) || defined (BSP_MCU_RX72N) || defined (BSP_MCU_RX66N) || defined(BSP_MCU_RX671) */
        break;
        case USB_HSPROC :
            conn_inf = USB_NOCONNECT;
        break;
        default :
            conn_inf = USB_NOCONNECT;
        break;
    }

    return (conn_inf);
}
/******************************************************************************
 End of function usb_hstd_support_speed_check
 ******************************************************************************/

/******************************************************************************
 Function Name   : usb_hstd_write_fifo
 Description     : Write specified amount of data to specified USB FIFO. 
 Arguments       : usb_utr_t    *ptr       : Pointer to usb_utr_t structure.
                 : uint16_t     count      : Write size
                 : uint16_t     pipemode   : The mode of CPU/DMA(D0)/DMA(D1).
                 : uint16_t     *write_p   : Address of buffer of data to write.
 Return value    : The incremented address of last argument (write_p).
 ******************************************************************************/
uint8_t *usb_hstd_write_fifo (usb_utr_t *ptr, uint16_t count, uint16_t pipemode, uint8_t *write_p)
{
    uint16_t even;
#if defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX71M)
    uint16_t odd;
#endif  /* defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX71M) */

#if defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX71M)
    if (USB_IP0 == ptr->ip)
    {
#endif  /* defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX71M) */
        /* WAIT_LOOP */
        for (even = (uint16_t) (count >> 1); (0 != even); --even)
        {
            /* 16bit access */
            hw_usb_write_fifo16(ptr, pipemode, *((uint16_t *) write_p));

            /* Renewal write pointer */
            write_p += sizeof(uint16_t);
        }

        if ((count & (uint16_t) 0x0001u) != 0u)
        {
            /* 8bit access */
            /* count == odd */
            /* Change FIFO access width */
            hw_usb_set_mbw(ptr, pipemode, USB_MBW_8);

            /* FIFO write */
            hw_usb_write_fifo8(ptr, pipemode, *write_p);

            /* Return FIFO access width */
            hw_usb_set_mbw(ptr, pipemode, USB_MBW_16);

            /* Renewal write pointer */
            write_p++;
        }
#if defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX71M)
    }
    else if (USB_IP1 == ptr->ip)
    {
        /* WAIT_LOOP */
        for (even = (uint16_t)(count >> 2); (0 != even); --even)
        {
            /* 16bit access */
            hw_usb_write_fifo32(ptr, pipemode, *((uint32_t *)write_p));

            /* Renewal write pointer */
            write_p += sizeof(uint32_t);
        }
        odd = count % 4;
        if ((odd & (uint16_t)0x0002u) != 0u)
        {
            /* 16bit access */
            /* Change FIFO access width */
            hw_usb_set_mbw(ptr, pipemode, USB_MBW_16);

            /* FIFO write */
            hw_usb_write_fifo16(ptr, pipemode, *((uint16_t *)write_p));

            /* Renewal write pointer */
            write_p += sizeof(uint16_t);
        }
        if ((odd & (uint16_t)0x0001u) != 0u)
        {
            /* 8bit access */
            /* count == odd */
            /* Change FIFO access width */
            hw_usb_set_mbw(ptr, pipemode, USB_MBW_8);

            /* FIFO write */
            hw_usb_write_fifo8(ptr, pipemode, *write_p);

            /* Renewal write pointer */
            write_p++;
        }
        /* Return FIFO access width */
        hw_usb_set_mbw(ptr, pipemode, USB_MBW_32);
    }
    else
    {
        /* Non */
    }
#endif  /* defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX71M) */
    return write_p;

}
/******************************************************************************
 End of function usb_hstd_write_fifo
 ******************************************************************************/

/******************************************************************************
 Function Name   : usb_hstd_read_fifo
 Description     : Read specified buffer size from the USB FIFO.
 Arguments       : usb_utr_t    *ptr        : Pointer to usb_utr_t structure.
                 : uint16_t     count       : Read size
                 : uint16_t     pipemode    : The mode of CPU/DMA(D0)/DMA(D1).
                 : uint16_t     *read_p     : Address of buffer to store the read data
 Return value    : Pointer to a buffer that contains the data to be read next.
 ******************************************************************************/
uint8_t *usb_hstd_read_fifo (usb_utr_t *ptr, uint16_t count, uint16_t pipemode, uint8_t *read_p)
{
    uint16_t even;
#if defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX71M)
    uint16_t odd;
#endif  /* defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX71M) */
    uint32_t odd_byte_data_temp;

#if defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX71M)
#if USB_CFG_ENDIAN == USB_CFG_BIG
    uint16_t    i;

#endif  /* USB_CFG_ENDIAN == USB_CFG_LITTLE */
#endif  /* defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX71M) */

#if defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX71M)
    if (USB_IP0 == ptr->ip)
    {
#endif  /* defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX71M) */
        /* WAIT_LOOP */
        for (even = (uint16_t) (count >> 1); (0 != even); --even)
        {
            /* 16bit FIFO access */
            *(uint16_t *) read_p = hw_usb_read_fifo16(ptr, pipemode);

            /* Renewal read pointer */
            read_p += sizeof(uint16_t);
        }
        if ((count & (uint16_t) 0x0001) != 0)
        {
            /* 16bit FIFO access */
            odd_byte_data_temp = hw_usb_read_fifo16(ptr, pipemode);

            /* Condition compilation by the difference of the little endian */
#if USB_CFG_ENDIAN == USB_CFG_LITTLE
            *read_p = (uint8_t) (odd_byte_data_temp & 0x00ff);
#else   /* USB_CFG_ENDIAN == USB_CFG_LITTLE */
            *read_p = (uint8_t) (odd_byte_data_temp >> 8);
#endif  /* USB_CFG_ENDIAN == USB_CFG_LITTLE */

            /* Renewal read pointer */
            read_p += sizeof(uint8_t);
        }
#if defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX71M)
    }
    else if (USB_IP1 == ptr->ip)
    {
        /* WAIT_LOOP */
        for (even = (uint16_t)(count >> 2); (0 != even); --even)
        {
            /* 32bit FIFO access */
            *(uint32_t *)read_p= hw_usb_read_fifo32(ptr, pipemode);

            /* Renewal read pointer */
            read_p += sizeof(uint32_t);
        }
        odd = count % 4;
        if (count < 4)
        {
            odd = count;
        }
        if (odd != 0)
        {
            /* 32bit FIFO access */
            odd_byte_data_temp = hw_usb_read_fifo32(ptr, pipemode);

            /* Condition compilation by the difference of the little endian */
#if USB_CFG_ENDIAN == USB_CFG_LITTLE
            /* WAIT_LOOP */
            do
            {
                *read_p = (uint8_t)(odd_byte_data_temp & 0x000000ff);
                odd_byte_data_temp = odd_byte_data_temp >> 8;

                /* Renewal read pointer */
                read_p += sizeof(uint8_t);
                odd--;
            }while (odd != 0);
#else   /* USB_CFG_ENDIAN == USB_CFG_LITTLE */
            /* WAIT_LOOP */
            for (i = 0; i < odd; i++)
            {
                *read_p = (uint8_t)( ( odd_byte_data_temp >> (24 -(i*8))) & 0x000000ff );

                /* Renewal read pointer */
                read_p += sizeof(uint8_t);
            }
#endif  /* USB_CFG_ENDIAN == USB_CFG_LITTLE */
        }
    }
    else
    {
        /* None */
    }
#endif  /* defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX71M) */
    return read_p;
}
/******************************************************************************
 End of function usb_hstd_read_fifo
 ******************************************************************************/

/******************************************************************************
 Function Name   : usb_hstd_forced_termination
 Description     : Terminate data transmission and reception.
 Arguments       : usb_utr_t    *ptr    : Pointer to usb_utr_t structure.
                 : uint16_t     pipe    : Pipe Number
                 : uint16_t     status  : Transfer status type
 Return value    : none
 Note            : In the case of timeout status, it does not call back.
 ******************************************************************************/
void usb_hstd_forced_termination (usb_utr_t *ptr, uint16_t pipe, uint16_t status)
{
    uint16_t buffer;

    /* PID = NAK */
    /* Set NAK */
    usb_cstd_set_nak(ptr, pipe);

    /* Disable Interrupt */
    /* Disable Ready Interrupt */
    hw_usb_clear_brdyenb(ptr, pipe);

    /* Disable Not Ready Interrupt */
    hw_usb_clear_nrdyenb(ptr, pipe);

    /* Disable Empty Interrupt */
    hw_usb_clear_bempenb(ptr, pipe);

    usb_cstd_clr_transaction_counter(ptr, pipe);

    /* Clear CFIFO-port */
    buffer = hw_usb_read_fifosel(ptr, USB_CUSE);
    if ((buffer & USB_CURPIPE) == pipe)
    {
        /* Changes the FIFO port by the pipe. */
        usb_cstd_chg_curpipe(ptr, (uint16_t) USB_PIPE0, (uint16_t) USB_CUSE, USB_FALSE);
    }

#if ((USB_CFG_DTC == USB_CFG_ENABLE) || (USB_CFG_DMA == USB_CFG_ENABLE))
    /* Clear D0FIFO-port */
    buffer = hw_usb_read_fifosel(ptr, USB_D0USE);
    if ((buffer & USB_CURPIPE) == pipe)
    {
        /* Changes the FIFO port by the pipe. */
        usb_cstd_chg_curpipe(ptr, (uint16_t) USB_PIPE0, (uint16_t) USB_D0USE, USB_FALSE);
    }

    /* Clear D1FIFO-port */
    buffer = hw_usb_read_fifosel(ptr, USB_D1USE);
    if ((buffer & USB_CURPIPE) == pipe)
    {
        /* Changes the FIFO port by the pipe. */
        usb_cstd_chg_curpipe(ptr, (uint16_t) USB_PIPE0, (uint16_t) USB_D1USE, USB_FALSE);
    }

#endif /* ((USB_CFG_DTC == USB_CFG_ENABLE) || (USB_CFG_DMA == USB_CFG_ENABLE)) */

    /* Do Aclr */
    usb_cstd_do_aclrm(ptr, pipe);

    /* FIFO buffer SPLIT transaction initialized */
    hw_usb_set_csclr(ptr, pipe);

    /* Call Back */
    if (USB_NULL != g_p_usb_hstd_pipe[ptr->ip][pipe])
    {
        /* Transfer information set */
        g_p_usb_hstd_pipe[ptr->ip][pipe]->tranlen = g_usb_hstd_data_cnt[ptr->ip][pipe];
        g_p_usb_hstd_pipe[ptr->ip][pipe]->status = status;
        g_p_usb_hstd_pipe[ptr->ip][pipe]->pipectr = hw_usb_read_pipectr(ptr, pipe);
        g_p_usb_hstd_pipe[ptr->ip][pipe]->errcnt = (uint8_t) g_usb_hstd_ignore_cnt[ptr->ip][pipe];
        g_p_usb_hstd_pipe[ptr->ip][pipe]->ipp = ptr->ipp;
        g_p_usb_hstd_pipe[ptr->ip][pipe]->ip = ptr->ip;
        if (USB_NULL != (g_p_usb_hstd_pipe[ptr->ip][pipe]->complete))
        {
            (g_p_usb_hstd_pipe[ptr->ip][pipe]->complete)(g_p_usb_hstd_pipe[ptr->ip][pipe], 0, 0);
        }

#if (BSP_CFG_RTOS_USED != 0)        /* Use RTOS */
        rtos_release_fixed_memory(&g_rtos_usb_mpf_id, (void *)g_p_usb_hstd_pipe[ptr->ip][pipe]);
        g_p_usb_hstd_pipe[ptr->ip][pipe] = (usb_utr_t*) USB_NULL;
        usb_rtos_resend_msg_to_submbx (ptr->ip, pipe, USB_HOST);

#else   /* (BSP_CFG_RTOS_USED != 0) */
        g_p_usb_hstd_pipe[ptr->ip][pipe] = (usb_utr_t*) USB_NULL;

#endif  /* (BSP_CFG_RTOS_USED != 0) */
    }
}
/******************************************************************************
 End of function usb_hstd_forced_termination
 ******************************************************************************/

/******************************************************************************
 Function Name   : usb_hstd_get_usb_ip_adr
 Description     : Get base address of the selected USB channel's peripheral 
                 : registers.
 Argument        : uint16_t ipnum  : USB_IP0 (0), or USB_IP1 (1).
 Return          : usb_regadr_t    : A pointer to the USB_597IP register 
                 : structure USB_REGISTER containing all USB
                 : channel's registers.
 ******************************************************************************/
usb_regadr_t usb_hstd_get_usb_ip_adr (uint16_t ipnum)
{
    usb_regadr_t ptr;

    if (USB_IP0 == ipnum)
    {
        ptr = (usb_regadr_t) &USB0;
    }
#if USB_NUM_USBIP == 2
    else if (USB_IP1 == ipnum)
    {
#if defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX71M)
        ptr = (usb_regadr_t)&USBA;

#endif  /* defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX71M) */
#if defined(BSP_MCU_RX63N) || defined(BSP_MCU_RX62N) || defined(BSP_MCU_RX671)
        ptr = (usb_regadr_t)&USB1;

#endif  /* defined(BSP_MCU_RX63N) || defined(BSP_MCU_RX62N) || defined(BSP_MCU_RX671) */
    }
#endif /* USB_NUM_USBIP == 2 */
    else
    {
        USB_DEBUG_HOOK(USB_DEBUG_HOOK_STD | USB_DEBUG_HOOK_CODE1);
    }

    return ptr;
}
/******************************************************************************
 End of function usb_hstd_get_usb_ip_adr
 ******************************************************************************/

/******************************************************************************
 Function Name   : usb_hstd_nrdy_endprocess
 Description     : NRDY interrupt processing. (Forced termination of data trans-
                 : mission and reception of specified pipe.)
 Arguments       : usb_utr_t    *ptr        : Pointer to usb_utr_t structure.
                 : uint16_t     pipe        : Pipe No
 Return value    : none
 Note            : none
 ******************************************************************************/
void usb_hstd_nrdy_endprocess (usb_utr_t *ptr, uint16_t pipe)
{
    uint16_t buffer;

    /*
     Host Function
     */
    buffer = usb_cstd_get_pid(ptr, pipe);

    /* STALL ? */
    if (USB_PID_STALL == (buffer & USB_PID_STALL))
    {
        /*USB_PRINTF1("### STALL Pipe %d\n", pipe);*/
        /* @4 */
        /* End of data transfer */
        usb_hstd_forced_termination(ptr, pipe, USB_DATA_STALL);
    }
    else
    {
        /* Wait for About 60ns */
        buffer = hw_usb_read_syssts(ptr);

        /* @3 */
        g_usb_hstd_ignore_cnt[ptr->ip][pipe]++;

        /*USB_PRINTF2("### IGNORE Pipe %d is %d times \n", pipe, g_usb_hstd_ignore_cnt[ptr->ip][pipe]);*/
        if (USB_PIPEERROR == g_usb_hstd_ignore_cnt[ptr->ip][pipe])
        {
            /* Data Device Ignore X 3 call back */
            /* End of data transfer */
            usb_hstd_forced_termination(ptr, pipe, USB_DATA_ERR);
        }
        else
        {
            /* 5ms wait */
            usb_cpu_delay_xms(5);

            /* PIPEx Data Retry */
            usb_cstd_set_buf(ptr, pipe);
        }
    }
}
/******************************************************************************
 End of function usb_hstd_nrdy_endprocess
 ******************************************************************************/

#endif  /* (USB_CFG_MODE & USB_CFG_HOST) == USB_CFG_HOST */

/******************************************************************************
 End of file
 ******************************************************************************/
