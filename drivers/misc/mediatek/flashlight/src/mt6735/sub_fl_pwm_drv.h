
#ifndef _SUB_FL_PWM_DRV_H_
#define _SUB_FL_PWM_DRV_H_
#include <linux/delay.h>
#include <linux/kernel.h>
#include <mt-plat/mt_pwm.h>
#include "kd_flashlight_type.h"
#include "kd_flashlight.h"
#include <linux/hct_board_config.h>


#if  __HCT_SUB_FLASHLIGHT_PWM_SUPPORT__

#if !defined( __HCT_SUB_FLASHLIGHT_PWM_F_DUTY__)||!defined( __HCT_SUB_FLASHLIGHT_PWM_T_DUTY__)
#error "sub fl pwm defined , so ,f_duty and t_duty should defined in hct_board_config.h"
#else
     #if __HCT_SUB_FLASHLIGHT_PWM_F_DUTY__>10||__HCT_SUB_FLASHLIGHT_PWM_T_DUTY__>10
        #error "f_duty and t_duty  in hct_board_config.h is outof range"
     #endif
#endif
#define SUB_F_DUTY __HCT_SUB_FLASHLIGHT_PWM_F_DUTY__
#define SUB_T_DUTY __HCT_SUB_FLASHLIGHT_PWM_T_DUTY__

#endif


#define TAG_NAME "[sub_fl_pwm_drv.c]"
#define PK_DBG_NONE(fmt, arg...)    do {} while (0)
#define PK_DBG_FUNC(fmt, arg...)    pr_debug(TAG_NAME "%s: " fmt, __func__ , ##arg)
#define PK_WARN(fmt, arg...)        pr_warn(TAG_NAME "%s: " fmt, __func__ , ##arg)
#define PK_NOTICE(fmt, arg...)      pr_notice(TAG_NAME "%s: " fmt, __func__ , ##arg)
#define PK_INFO(fmt, arg...)        pr_info(TAG_NAME "%s: " fmt, __func__ , ##arg)
#define PK_TRC_FUNC(f)              pr_debug(TAG_NAME "<%s>\n", __func__)
#define PK_TRC_VERBOSE(fmt, arg...) pr_debug(TAG_NAME fmt, ##arg)
#define PK_ERROR(fmt, arg...)       pr_err(TAG_NAME "%s: " fmt, __func__ , ##arg)

#define DEBUG_SUB_LEDS_STROBE
#ifdef DEBUG_SUB_LEDS_STROBE
#define PK_DBG PK_DBG_FUNC
#define PK_VER PK_TRC_VERBOSE
#define PK_ERR PK_ERROR
#else
#define PK_DBG(a, ...)
#define PK_VER(a, ...)
#define PK_ERR(a, ...)
#endif

enum SUB_FL_MODE{
	MODE_MIN = 0,
	SUB_PWD_M = MODE_MIN,
	SUB_FLASH_M,
	SUB_TORCH_M,
	MODE_MAX
};

#endif
