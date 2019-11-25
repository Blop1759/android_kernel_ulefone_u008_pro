#ifndef __HCT_BOARD_CONFIG_H__
#define __HCT_BOARD_CONFIG_H__


#undef HCT_YES
#define HCT_YES 1
#undef HCT_NO
#define HCT_NO 0

/*audio releated */
/* phone mic mode



*/
#define __HCT_PHONE_MIC_MODE__   2
 
 /*phone use exp audio pa*/
#define __HCT_USING_EXTAMP_HP__  HCT_YES
#define __HCT_EXTAMP_GPIO_NUM__   124

#if  __HCT_USING_EXTAMP_HP__  
   #define __HCT_EXTAMP_HP_MODE__    3
#endif

/****POWER RELEATED****/
#define __HCT_CAR_TUNE_VALUE__   88
#define __HCT_R_SENSE_VALUE__    56

/*############### USB OTG releated config--START #####################*/

#define __HCT_USB_MTK_OTG_SUPPORT__  HCT_YES

#define __HCT_5G_WIFI_SUPPORT__  HCT_NO

/*############### USB OTG releated config--END #####################*/

#include "hct_custom_config.h"
#endif
