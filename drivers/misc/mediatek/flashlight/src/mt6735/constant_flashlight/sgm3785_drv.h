/************************************************************************************************
*							*file name : sgm3785_drv.h
*							*Version : v1.0
*							*Author : erick
*							*Date : 2015.4.16
*************************************************************************************************/
#ifndef _SGM3785_DRV_H_
#define _SGM3785_DRV_H_
#include <linux/delay.h>
#include <linux/kernel.h>
#include <mt-plat/mt_pwm.h>
#include "kd_flashlight_type.h"
#include "kd_flashlight.h"
#include <linux/hct_board_config.h>


#if  __HCT_FLASHLIGHT_SGM3785_SUPPORT__

#if !defined( __HCT_FLASHLIGHT_PWM_F_DUTY__)||!defined( __HCT_FLASHLIGHT_PWM_T_DUTY__)
#error "sgm3785 defined , so ,f_duty and t_duty should defined in hct_board_config.h"
#else
     #if __HCT_FLASHLIGHT_PWM_F_DUTY__>10||__HCT_FLASHLIGHT_PWM_T_DUTY__>10
        #error "f_duty and t_duty  in hct_board_config.h is outof range"
     #endif
#endif
#define F_DUTY __HCT_FLASHLIGHT_PWM_F_DUTY__
#define T_DUTY __HCT_FLASHLIGHT_PWM_T_DUTY__

#endif


#define SGM3138_DEBUG
#ifdef SGM3138_DEBUG
#define sgm3138_dbg printk
#else
#define sgm3138_dbg //
#endif

enum SGM3785_MODE{
	MODE_MIN = 0,
	PWD_M = MODE_MIN,
	FLASH_M,
	TORCH_M,
	MODE_MAX
};

#endif
