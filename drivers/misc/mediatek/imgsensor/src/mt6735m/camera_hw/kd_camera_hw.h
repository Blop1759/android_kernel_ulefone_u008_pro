#ifndef _KD_CAMERA_HW_H_
#define _KD_CAMERA_HW_H_


#include <linux/types.h>
#include "kd_camera_typedef.h"


#if 0//defined CONFIG_MTK_LEGACY
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>
#include "pmic_drv.h"
/*  */
/* Analog */
#define CAMERA_POWER_VCAM_A         PMIC_APP_MAIN_CAMERA_POWER_A
/* Digital */
#define CAMERA_POWER_VCAM_D         PMIC_APP_MAIN_CAMERA_POWER_D
/* AF */
#define CAMERA_POWER_VCAM_AF        PMIC_APP_MAIN_CAMERA_POWER_AF
/* digital io */
#define CAMERA_POWER_VCAM_IO        PMIC_APP_MAIN_CAMERA_POWER_IO
/* Digital for Sub */
#define SUB_CAMERA_POWER_VCAM_D     PMIC_APP_SUB_CAMERA_POWER_D


/* FIXME, should defined in DCT tool */

/* Main sensor */
    /* Common phone's reset pin uses extension GPIO10 of mt6306 */
    #define CAMERA_CMRST_PIN            GPIO_CAMERA_CMRST_PIN
    #define CAMERA_CMRST_PIN_M_GPIO     GPIO_CAMERA_CMRST_PIN_M_GPIO


#define CAMERA_CMPDN_PIN            GPIO_CAMERA_CMPDN_PIN
#define CAMERA_CMPDN_PIN_M_GPIO     GPIO_CAMERA_CMPDN_PIN_M_GPIO

/* FRONT sensor */
#define CAMERA_CMRST1_PIN           GPIO_CAMERA_CMRST1_PIN
#define CAMERA_CMRST1_PIN_M_GPIO    GPIO_CAMERA_CMRST1_PIN_M_GPIO

#define CAMERA_CMPDN1_PIN           GPIO_CAMERA_CMPDN1_PIN
#define CAMERA_CMPDN1_PIN_M_GPIO    GPIO_CAMERA_CMPDN1_PIN_M_GPIO

/* Define I2C Bus Num */
#define SUPPORT_I2C_BUS_NUM1        0
#define SUPPORT_I2C_BUS_NUM2        0
#else
#define CAMERA_CMRST_PIN            0
#define CAMERA_CMRST_PIN_M_GPIO     0

#define CAMERA_CMPDN_PIN            0
#define CAMERA_CMPDN_PIN_M_GPIO     0

/* FRONT sensor */
#define CAMERA_CMRST1_PIN           0
#define CAMERA_CMRST1_PIN_M_GPIO    0

#define CAMERA_CMPDN1_PIN           0
#define CAMERA_CMPDN1_PIN_M_GPIO    0

#endif /* End of #if defined CONFIG_MTK_LEGACY */

typedef enum {
       CAMPDN,
       CAMRST,
       CAM1PDN,
       CAM1RST,
       CAMLDO
} CAMPowerType;

typedef enum KD_REGULATOR_TYPE_TAG {
	VCAMA,
	VCAMD,
	VCAMIO,
	VCAMAF,
	SUB_VCAMA,
	SUB_VCAMD,
	SUB_VCAMIO
} KD_REGULATOR_TYPE_T;


extern void ISP_MCLK1_EN(BOOL En);
extern void ISP_MCLK2_EN(BOOL En);


extern bool _hwPowerDown(KD_REGULATOR_TYPE_T type);
extern bool _hwPowerOn(KD_REGULATOR_TYPE_T type, int powerVolt);

int mtkcam_gpio_set(int PinIdx, int PwrType, int Val);
int mtkcam_gpio_init(struct platform_device *pdev);

typedef enum{
	VDD_None=0,
	PDN,
	RST,
	SensorMCLK,
	AVDD , //CAMERA_POWER_VCAM_A,
	DVDD , // = CAMERA_POWER_VCAM_D,
	DOVDD, // = CAMERA_POWER_VCAM_IO,
	AFVDD, //= CAMERA_POWER_VCAM_AF
	POWR_TYPE_MAX
}PowerType;


typedef enum{
	Vol_Low =0,
	Vol_High=1,
	Vol_900  = 900000,
    Vol_1000 = 1000000,
    Vol_1100 = 1100000,
    Vol_1200 = 1200000,	
    Vol_1300 = 1300000,    
    Vol_1350 = 1350000,   
    Vol_1500 = 1500000,    
    Vol_1800 = 1800000,    
    Vol_2000 = 2000000,
    Vol_2100 = 2100000,
    Vol_2500 = 2500000,    
    Vol_2800 = 2800000,
    Vol_3000 = 3000000,
    Vol_3300 = 3300000,
    Vol_3400 = 3400000, 
    Vol_3500 = 3500000,
    Vol_3600 = 3600000  
}Voltage;


typedef struct{
	PowerType PowerType;
	Voltage Voltage;
	u32 Delay;
}PowerInformation;


typedef struct{
	char* SensorName;
	PowerInformation PowerInfo[12];
}PowerSequence;

typedef struct{
	PowerSequence PowerSeq[16];	
}PowerUp;

typedef struct{
	u32 Gpio_Pin;  
	u32 Gpio_Mode;
	Voltage Voltage;
}PowerCustInfo;

typedef struct{
	PowerCustInfo PowerCustInfo[6];
}PowerCust;

#endif
