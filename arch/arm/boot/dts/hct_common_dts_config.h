#ifndef __HCT_CUSTOM_DTS_CONFIG_H__
#define __HCT_CUSTOM_DTS_CONFIG_H__


/*############### led releated config #####################*/
/*led mode define*/

#define __HCT_MT65XX_LED_MODE_NONE__      0
#define __HCT_MT65XX_LED_MODE_PWM__       1
#define __HCT_MT65XX_LED_MODE_GPIO__      2
#define __HCT_MT65XX_LED_MODE_PMIC__      3
#define __HCT_MT65XX_LED_MODE_CUST_LCM__  4 
#define __HCT_MT65XX_LED_MODE_CUST_BLS_PWM__  5
#define __HCT_MT65XX_LED_MODE_GPIO_R__     6
#define __HCT_MT65XX_LED_MODE_GPIO_G__      7
#define __HCT_MT65XX_LED_MODE_GPIO_B__      8  
#define __HCT_MT65XX_LED_MODE_GPIO_KPD__      9 
#define __HCT_MT65XX_LED_MODE_BREATHLIGHT__      6 

#define __HCT_MT65XX_LED_PMIC_LCD_ISINK__      0
#define __HCT_MT65XX_LED_PMIC_NLED_ISINK0__      1
#define __HCT_MT65XX_LED_PMIC_NLED_ISINK1__      2
#define __HCT_MT65XX_LED_PMIC_NLED_ISINK2__      3
#define __HCT_MT65XX_LED_PMIC_NLED_ISINK3__      4

#define __HCT_MT65XX_LED_PWM_NLED_PWM1__      0
#define __HCT_MT65XX_LED_PWM_NLED_PWM2__      1
#define __HCT_MT65XX_LED_PWM_NLED_PWM3__      2
#define __HCT_MT65XX_LED_PWM_NLED_PWM4__      3
#define __HCT_MT65XX_LED_PWM_NLED_PWM5__      4
/*###################### default msdc detect level is low ##########*/
// Note: default: sd card detection is low level
#ifndef __HCT_SD_CARD_DET_LEVEL__
#define __HCT_SD_CARD_DET_LEVEL__ HCT_NO
#endif

// Note: default: c2k config is off
/*
    when you switch c2k on, you need config __HCT_C2K_DTS_SUPPORT__ HCT_YES
*/
#ifndef __HCT_C2K_DTS_SUPPORT__
#define __HCT_C2K_DTS_SUPPORT__  HCT_NO
#endif




#endif

