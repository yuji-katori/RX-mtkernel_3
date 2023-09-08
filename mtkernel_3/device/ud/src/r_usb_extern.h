/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	r_usb_extern.h
 */

#ifndef R_USB_EXTERN_H
#define R_USB_EXTERN_H

/*****************************************************************************
 Public Variables
 ******************************************************************************/

/* r_usb_cdataio.c */
extern usb_utr_t *g_usb_hstd_pipe[USB_MAX_PIPE];		// Message pipe 
extern uint8_t   *g_usb_hstd_data_ptr[USB_MAX_PIPE];		// PIPEn Buffer pointer(8bit)
extern uint32_t   g_usb_hstd_data_cnt[USB_MAX_PIPE];		// PIPEn Buffer counter

/* r_usb_hdriver.c */
extern uint8_t  g_usb_hstd_device_state;			// Device state
extern uint8_t  g_usb_hstd_ctsq;				// Control transfer stage management
extern uint8_t  g_usb_hstd_mgr_mode;				// Manager mode
extern uint8_t  g_usb_hstd_ignore_cnt[USB_MAX_PIPE];		// Ignore count
extern uint16_t g_usb_hstd_device_addr;				// Device address
extern uint16_t g_usb_hstd_device_speed;			// Reset handshake result
extern uint16_t g_usb_hstd_dcp_register[USB_MAXDEVADR];		// DEVSEL & DCPMAXP (Multiple device)
extern uint16_t g_usb_hstd_device_info[5];			// port status, config num, interface class, speed

/* r_usb_hmanager.c */
extern uint8_t          g_usb_hstd_check_enu_result;		// Enumeration result check
extern usb_descriptor_t g_usb_hstd_device_descriptor[USB_DEVICESIZE/2];
extern usb_descriptor_t g_usb_hstd_config_descriptor[USB_CONFIGSIZE/2];

/* r_usb_hcontrolrw.c */
extern usb_pipe_table_t g_usb_pipe_table;

/* r_usb_creg_abs.c */
extern uint8_t g_drive_search_lock;
extern uint8_t g_drive_search_que_cnt;

/* r_usb_hscheduler.c */
extern uint8_t    g_usb_scheduler_id_use;
extern uint8_t    g_usb_buf[USB_BUFSIZE];
extern usb_utr_t  g_usb_scheduler_block[USB_BLKMAX];
extern usb_utr_t *g_usb_scheduler_dt_use;

/* r_usb_hmsc_driver.c */
extern uint8_t  g_drive_search_que;
extern uint16_t g_usb_hmsc_in_pipectr;
extern uint16_t g_usb_hmsc_out_pipectr;

// r_usb_rx_mcu.c
void      usb_module_start(void);
void      usb_module_stop(void);
void      usb_cpu_delay_xms(uint16_t time);
void      usb_cpu_delay_1us(uint16_t time);
void      usb_cpu_usbint_init(void);
uint16_t  usb_chattaring(volatile __evenaccess uint16_t *syssts);

void      usb_hstd_send_start(void);
void      usb_hstd_buf_to_fifo(void);
uint16_t  usb_hstd_write_data(uint16_t pipe, uint16_t pipemode);
void      usb_hstd_receive_start(void);
uint16_t  usb_hstd_read_data(void);
void      usb_hstd_data_end(uint16_t status);
void      usb_hstd_change_device_state(usb_cb_t complete, uint16_t msginfo);
void      usb_hstd_mgr_open(void);
void      usb_hstd_driver_registration(void);
void      usb_hstd_driver_release(void);
void      usb_hstd_set_pipe_info(uint16_t pipe, usb_pipe_table_reg_t *src_ep_tbl);
void      usb_hstd_return_enu_mgr(uint16_t cls_result);
void      usb_hstd_hcd_open(void);
uint8_t   usb_hstd_make_pipe_reg_info(uint16_t devadr, uint8_t *descriptor, usb_pipe_table_reg_t *pipe_table_work);
void      usb_hstd_nrdy_pipe_process(uint16_t bitsts);
void      usb_hstd_bemp_pipe_process(uint16_t bitsts);
void      usb_hstd_brdy_pipe_process(uint16_t bitsts);

// r_usb_clibusbip.c
uint16_t  usb_cstd_get_pid(uint16_t pipe);
uint16_t  usb_cstd_port_speed(void);
uint16_t  usb_cstd_get_maxpacket_size(uint16_t pipe);
uint16_t  usb_cstd_get_pipe_type(uint16_t pipe);
void      usb_cstd_set_buf(uint16_t pipe);
void      usb_cstd_clr_stall(uint16_t pipe);
void      usb_class_task(void);
uint8_t   usb_hstd_pipe_to_epadr(uint16_t pipe);
void      usb_hstd_dummy_function(usb_utr_t *msg, uint16_t data1, uint16_t data2);
void      usb_hstd_berne_enable(void);
void      usb_hstd_do_sqtgl(uint16_t pipe, uint16_t toggle);
uint16_t  usb_hstd_get_devsel(void);
uint16_t  usb_hstd_get_device_address(uint16_t pipe);

// r_usb_creg_abs.c
uint16_t  usb_cstd_is_set_frdy(uint16_t pipe, uint16_t pipemode, uint16_t isel);
void      usb_cstd_chg_curpipe(uint16_t pipe, uint16_t pipemode, uint16_t isel);
void      usb_cstd_clr_transaction_counter(uint16_t pipe);
void      usb_cstd_pipe_init(void);
void      usb_cstd_clr_pipe_cnfg(void);
void      usb_cstd_set_nak(uint16_t pipe);
uint16_t  usb_cstd_get_buf_size(uint16_t pipe);
uint8_t  *usb_hstd_write_fifo(uint16_t count, uint16_t pipemode, uint8_t  *write);
uint8_t  *usb_hstd_read_fifo(uint16_t count, uint8_t  *read);
void      usb_hstd_forced_termination(uint16_t status);
void      usb_hstd_nrdy_endprocess(void);

/* r_usb_cinthandler_usbip0.c */
void      usb_hstd_usb_handler(void);
void      usb_hstd_init_usb_message(void);

// r_usb_hdriver.c
void      usb_hstd_hcd_snd_mbx(uint16_t msginfo, uint16_t pipe, uint16_t *adr, usb_cb_t callback);
void      usb_hstd_mgr_snd_mbx(uint16_t msginfo, uint16_t pipe, uint16_t result);
void      usb_hstd_hcd_task(usb_utr_t *msg);
usb_er_t  usb_hstd_transfer_start(usb_utr_t *msg);
usb_er_t  usb_hstd_transfer_start_req(usb_utr_t *msg);
void      usb_hstd_clr_feature(uint16_t addr, uint16_t epnum, usb_cb_t complete);
void      usb_hstd_bus_int_disable(void);

// r_usb_hcontrolrw.c
uint16_t  usb_hstd_ctrl_write_start(uint32_t Bsize, uint8_t  *Table);
void      usb_hstd_ctrl_read_start(uint32_t Bsize, uint8_t  *Table);
void      usb_hstd_status_start(void);
void      usb_hstd_ctrl_end(uint16_t status);
void      usb_hstd_setup_start(void);

// r_usb_hintfifo.c
void      usb_hstd_brdy_pipe(uint16_t bitsts);
void      usb_hstd_nrdy_pipe(uint16_t bitsts);
void      usb_hstd_bemp_pipe(uint16_t bitsts);

// r_usb_hstdfunction.c
void      usb_hdriver_init(void);

// r_usb_hreg_abs
void      usb_hstd_interrupt_handler(usb_utr_t *msg);
uint16_t  usb_hstd_chk_attach(void);
void      usb_hstd_chk_clk(uint16_t event);
void      usb_hstd_detach_process(void);
void      usb_hstd_attach_process(void);
void      usb_hstd_bus_reset(void);
uint16_t  usb_hstd_support_speed_check(void);

// r_usb_hlibusbip.c
void      usb_hstd_set_dev_addr(uint16_t devsel, uint16_t speed);
void      usb_hstd_bchg_enable(void);
void      usb_hstd_set_uact(void);
void      usb_hstd_ovrcr_enable(void);
void      usb_hstd_ovrcr_disable(void);
void      usb_hstd_attch_enable(void);
void      usb_hstd_attch_disable(void);
void      usb_hstd_dtch_enable(void);
void      usb_hstd_dtch_disable(void);
void      usb_hstd_set_pipe_reg(void);
void      usb_hstd_clr_pipe_table(uint16_t device_address);
void      usb_hstd_clr_pipe_table_ip(void);
uint16_t  usb_hstd_chk_dev_addr(uint16_t devsel);
void      usb_hstd_bchg_disable(void);

// r_usb_hsignal.c
void      usb_hstd_vbus_control(uint16_t command);
void      usb_hstd_attach(uint16_t result);
void      usb_hstd_detach(void);

// r_usb_hmanager.c
void      usb_hstd_notif_ator_detach(uint16_t result);
void      usb_hstd_ovcr_notifiation(void);
void      usb_hstd_status_result(usb_utr_t *msg, uint16_t data, uint16_t result);
void      usb_hstd_mgr_task(usb_utr_t *msg);
void      usb_hstd_enum_get_descriptor(uint16_t devadr, uint16_t cnt_value);

// r_usb_hscheduler.c
void       usb_cstd_wait_scheduler(void);
void       usb_cstd_sche_init(void);
uint8_t    usb_cstd_scheduler(void);
void       usb_cstd_set_task_pri(uint8_t taskid, uint8_t pri);
void       usb_cstd_snd_msg(uint8_t id, usb_utr_t* msg);
usb_utr_t *usb_cstd_get_blk(void);
void       usb_cstd_rel_blk(usb_utr_t *blk);

// r_usb_hmsc_driver.c
void       usb_hmsc_task(usb_utr_t *msg);
void       usb_hmsc_class_check(uint16_t **table);
void       usb_hmsc_configured(void);
void       usb_hmsc_detach(uint16_t devadr);
void       usb_hmsc_drive_complete(usb_utr_t *msg, uint16_t devadr, uint16_t data2);
void       usb_hmsc_driver_start(void);
uint16_t   usb_hmsc_ref_drvno(uint16_t devadr);

// r_usb_hstorage_driver.c
void       usb_hmsc_strg_drive_task(usb_utr_t *msg);
void       usb_hmsc_strg_drive_search(uint16_t devadr, usb_cb_t complete);

// usb_driver.c
void       USB_InterruptEvent(void);
void       USB_AttachEvent(void);
void       USB_DetachEvent(void);
void       USB_NoSupportEvent(void);
void       USB_StrgRWEndEvent(_BOOL);
void       USB_DmaCompleteEvent(void);

#endif  /* R_USB_EXTERN_H */
