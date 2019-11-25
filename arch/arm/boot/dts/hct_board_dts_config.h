#define PUNMUX_GPIO_NONE_FUNC_NONE    0

#include "hct_common_dts_config.h"

/**************************ACCDET MIC Related*********/
#define __HCT_ACCDET_MIC_MODE__  1

/**************************SD CARD hotplug Related*********/
#if __HCT_SD_CARD_HOTPLUG_SUPPORT__
#define __HCT_MSDC_CD_EINT_NUM__  5
#define __HCT_MSDC_CD_GPIO_PIN_NUM__  5
#endif


/**************************  Hall  Related*********/
#if __HCT_HALL_SUPPORT__

#define __HCT_HALL_EINT_PIN_NUM__           123
#define __HCT_KPD_SLIDE_EINT_PIN__          PINMUX_GPIO123__FUNC_GPIO123
#define __HCT_HALL_EINT_GPIO_NUM__          123
#endif
  
/**************************Camera Related*********/
#define __HCT_GPIO_CAMERA_LDO_EN_PINMUX__  PUNMUX_GPIO_NONE_FUNC_NONE
//-----------main camera
#define __HCT_GPIO_CAMERA_CMRST_PINMUX__ PINMUX_GPIO3__FUNC_GPIO3
#define __HCT_GPIO_CAMERA_CMPDN_PINMUX__ PINMUX_GPIO7__FUNC_GPIO7
//-----------sub camera
#define __HCT_GPIO_CAMERA_CMRST1_PINMUX__ PINMUX_GPIO11__FUNC_GPIO11
#define __HCT_GPIO_CAMERA_CMPDN1_PINMUX__ PINMUX_GPIO12__FUNC_GPIO12


/**************************FlashLight Related*********/
/*flashlight enable pin*/
#if __HCT_FLASHLIGHT_OCP8132_SUPPORT__
#define __HCT_GPIO_FLASHLIGHT_EN_PIN__			PINMUX_GPIO55__FUNC_GPIO55
#else
	#define __HCT_GPIO_FLASHLIGHT_EN_PIN__			PINMUX_GPIO59__FUNC_GPIO59
#endif

/*flashlight pwm pin*/
#if __HCT_FLASHLIGHT_SGM3785_SUPPORT__

#define __HCT_GPIO_FLASHLIGHT_PWM_AS_PWM_PIN__		PINMUX_GPIO64__FUNC_PWM2
#define __HCT_GPIO_FLASHLIGHT_PWM_AS_GPIO_PIN__		PINMUX_GPIO64__FUNC_GPIO64
#define __HCT_FLASHLIGHT_PWM_NUM__                      2

#endif

#if __HCT_FLASHLIGHT_AW3641_SUPPORT__||__HCT_FLASHLIGHT_AW3141_SUPPORT__
#define __HCT_GPIO_FLASHLIGHT_MODE_PINMUX__			PINMUX_GPIO64__FUNC_GPIO64
#endif

#if __HCT_FLASHLIGHT_OCP8132_SUPPORT__
#define __HCT_GPIO_FLASHLIGHT_MODE_PINMUX__			 PINMUX_GPIO43__FUNC_GPIO43
#endif
/*flashlight sub en*/

#if __HCT_SUB_FLASHLIGHT_SUPPORT__
#define __HCT_GPIO_FLASHLIGHT_SUB_EN_PIN__		PINMUX_GPIO44__FUNC_GPIO44
#if __HCT_SUB_FLASHLIGHT_AW3641_SUPPORT__
#define __HCT_GPIO_FLASHLIGHT_SUB_MODE_PINMUX__		PINMUX_GPIO64__FUNC_GPIO64
#endif
#endif

#if __HCT_SUB_FLASHLIGHT_PWM_SUPPORT__
#define __HCT_GPIO_FLASHLIGHT_SUB_PWM_AS_PWM_PIN__		PINMUX_GPIO44__FUNC_PWM0
#define __HCT_GPIO_FLASHLIGHT_SUB_PWM_AS_GPIO_PIN__		PINMUX_GPIO44__FUNC_GPIO44
#define __HCT_SUB_FLASHLIGHT_PWM_NUM__                      0
#endif
/**************************Audio EXMP Related*********/
#define __HCT_GPIO_EXTAMP_EN_PIN__     PINMUX_GPIO124__FUNC_GPIO124
#define __HCT_GPIO_EXTAMP2_EN_PIN__    PUNMUX_GPIO_NONE_FUNC_NONE


/****************************IRTX Related************************/
#define __HCT_IRTX_PWR_EN_PIN__             PINMUX_GPIO76__FUNC_GPIO76
#define __HCT_IRTX_OUT_PIN_DEFAULT__        PINMUX_GPIO61__FUNC_GPIO61
#define __HCT_IRTX_OUT_PIN_LED_EN__         PINMUX_GPIO61__FUNC_IRTX_OUT

/**************************CTP Related****************/
#define __HCT_CTP_EINT_ENT_PINNUX__         PINMUX_GPIO10__FUNC_GPIO10
#define __HCT_CTP_EINT_EN_PIN_NUM__      10 

#define __HCT_CTP_RESET_PINNUX__          PINMUX_GPIO126__FUNC_GPIO126

#define __HCT_CTP_FT_ALL_SENSOR_ADDR__     0x38
#define __HCT_TPD_ICN85XX_I2C_ADDR__			0x48
#define __HCT_CTP_MSG5846_SENSOR_ADDR__    0x26
/***************************lcm related********************/
#define __HCT_GPIO_LCM_POWER_DM_PINMUX__	PINMUX_GPIO57__FUNC_GPIO57//1.8v ldo
#define __HCT_GPIO_LCM_POWER_DP_PINMUX__	PINMUX_GPIO58__FUNC_GPIO58 //2.8v ldo
/**************************RGB GPIO Related****************/
#if __HCT_RED_LED_MODE__ ==	__HCT_MT65XX_LED_MODE_PMIC__
#define __HCT_RED_LED_DATA__	__HCT_MT65XX_LED_PMIC_NLED_ISINK1__
#endif
#if __HCT_GREEN_LED_MODE__ == __HCT_MT65XX_LED_MODE_GPIO_G__
#define __HCT_GPIO_GREEN_GPIO_EN_PIN__ PINMUX_GPIO44__FUNC_GPIO44
#endif
#if __HCT_BLUE_LED_MODE__ == __HCT_MT65XX_LED_MODE_PMIC__
#define __HCT_BLUE_LED_DATA__	__HCT_MT65XX_LED_PMIC_NLED_ISINK0__
#endif

#if __HCT_BUTTON_BACKLIGHT_LED_MODE__ == __HCT_MT65XX_LED_MODE_GPIO_KPD__
#define __HCT_GPIO_BUTTON_BACKLIGHT_GPIO_EN_PIN__    		PINMUX_GPIO80__FUNC_GPIO80
#endif
/**************************Accdet Related****************/

#define __HCT_ACCDET_EINT_PIN__     PINMUX_GPIO6__FUNC_GPIO6
#define __HCT_ACCDET_EINT_PIN_NUM__     6


/**************************Gsensor Related****************/
/* Gsensor releated*/
/*Step1: define this macro*/
/*Step2: need define dws int tab with the right bus num*/
#define __HCT_GSENSOR_I2C_BUS_NUM__       2

#define __HCT__KXTJ2_SENSOR_ADDR__     0x40
/*kxtj2_1009 Sensor Direction*/
#define __HCT__KXTJ2_SENSOR_DIRECTION__     5
/*kxtj2_1009 Sensor batch supported*/
#define __HCT__KXTJ2_SENSOR_BATCH_SUPPORT__     0

#define __HCT__MXC400X_SENSOR_ADDR__	0x15
#define __HCT__MXC400X_SENSOR_DIRECTION__	5
#define __HCT__MXC400X_SENSOR_BATCH_SUPPORT__ 0

#define __HCT__BMA156_NEW_SENSOR_ADDR__        0x10
#define __HCT__BMA156_NEW_SENSOR_DIRECTION__   5
#define __HCT__BMA156_NEW_SENSOR_BATCH_SUPPORT__  0
/**************************Gyro sensor Related****************/
#define __HCT_GYRO_I2C_BUS_NUM__       2
#define __HCT__GYRO2_SENSOR_ADDR__     0x68

#define __HCT__BMG160_NEW_SENSOR_ADDR__     0x68
#define __HCT__BMG160_NEW_SENSOR_DIRECTION__           5

#define __HCT_GYRO_EINT_PINMUX__     0

/**************************Msensor Related********************/
#define __HCT_MSENSOR_I2C_BUS_NUM__                    2
       
#define __HCT__BMM156_NEW_SENSOR_ADDR__                0x12    
#define __HCT__BMM156_NEW_SENSOR_DIRECTION__           5
#define __HCT__BMM156_NEW_SENSOR_BATCH_SUPPORT__       0

/**************************ALSPS Related****************/
/*Step1: define this macro*/
/*Step2: need define dws int tab with the right bus num*/
#define __HCT_ALSPS_I2C_BUS_NUM__               2
#define __HCT_STK3X1X_SENSOR_ADDR__			0x48
#define __HCT_HCT_STK3X1X_PS_THRELD_HIGH__		1700
#define __HCT_HCT_STK3X1X_PS_THRELD_LOW__		1500
/*EPL259X Sensor Customize ----start*****/
/*EPL259X Sensor I2C addr*/
#define __HCT_EPL259X_SENSOR_ADDR__            0x49

#define __HCT_EPL2182_SENSOR_ADDR__            0x49

#define __HCT_ALSPS_EINT_PINMUX__                  PINMUX_GPIO95__FUNC_GPIO95
#define __HCT_ALSPS_EINT_PIN_NUM__              95


/**************************Switch Charging Related****************/
/*Step1: define this macro*/
/*Step2: need define dws int tab with the right bus num*/
#define __HCT_SWITCH_CHARGER_I2C_BUS_NUM__       2
#define __HCT_NCP1854_CHARGER_ADDR__             0X36
#define __HCT_NCP1851_CHARGER_ADDR__             0X36

/**************************USB OTG Related****************/
#if __HCT_USB_MTK_OTG_SUPPORT__
#define __HCT_GPIO_DRV_VBUS_PIN__     			PINMUX_GPIO96__FUNC_GPIO96
#define __HCT_GPIO_USB_IDDIG_PINMUX_PIN__                    PINMUX_GPIO0__FUNC_IDDIG

#define __HCT_USB_VBUS_EN_PIN__     		96
#define __HCT_USB_VBUS_EN_PIN_MODE__     	0

#define __HCT_USB_IDDIG_PIN__     		0
#define __HCT_USB_IDDIG_PIN_MODE__     	        1
#endif
/**************************GPS LNA Related****************/
#define __HCT_GPS_LNA_EN_PINMUX__     			PINMUX_GPIO79__FUNC_GPIO79
/**************************AW9136  Related****************/
#if __HCT_AW9136_TS_SUPPORT__
#define __HCT_AW9136_TS_I2C_BUS_NUM__	1
#define __HCT_AW9136_TS_ADDR__			0x2C
#define __HCT_AW9136_INT_PINNUX__		PINMUX_GPIO89__FUNC_GPIO89
#define __HCT_GPIO_AW9136_PDN_PINMUX__	PINMUX_GPIO83__FUNC_GPIO83
#define __HCT_CTP_AW9136_INT_NUM__		89
#endif
/*************************FINGERPRINT Related****************/
#if __HCT_FINGERPRINT_SUPPORT__
#define __HCT_FINGERPRINT_EINT_EN_PIN_NUM__	125
#define __HCT_FINGERPRINT_EINT_PIN__	PINMUX_GPIO125__FUNC_GPIO125

#define __HCT_FINGERPRINT_POWER_PIN__	PINMUX_GPIO70__FUNC_GPIO70
#define __HCT_FINGERPRINT_RESET_PIN__	PINMUX_GPIO93__FUNC_GPIO93
#endif

//////////*****************customise end******************//////////
#include "hct_custom_config.h"

