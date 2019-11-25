#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>

#include "kd_camera_typedef.h"
#include <mt-plat/mt_gpio.h>
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_camera_feature.h"
#include "kd_camera_hw.h"
#include <linux/regulator/consumer.h>
/******************************************************************************
 * Debug configuration
******************************************************************************/
#define PFX "[kd_camera_hw]"
#define PK_DBG_NONE(fmt, arg...)    do {} while (0)
#define PK_DBG_FUNC(fmt, args...)    pr_debug(PFX  fmt, ##args)

#define DEBUG_CAMERA_HW_K
#ifdef DEBUG_CAMERA_HW_K
#define PK_DBG(fmt, args...) pr_debug(PFX  fmt, ##args)
#define PK_ERR(fmt, arg...)         pr_err(fmt, ##arg)
#define PK_XLOG_INFO(fmt, args...)  pr_debug(PFX  fmt, ##args);
#else
#define PK_DBG(a, ...)
#define PK_ERR(a, ...)
#define PK_XLOG_INFO(fmt, args...)
#endif

#if 1//!defined(CONFIG_MTK_LEGACY)
/* GPIO Pin control*/
struct platform_device *cam_plt_dev = NULL;
struct pinctrl *camctrl = NULL;
struct pinctrl_state *cam0_pnd_h = NULL;
struct pinctrl_state *cam0_pnd_l = NULL;
struct pinctrl_state *cam0_rst_h = NULL;
struct pinctrl_state *cam0_rst_l = NULL;
struct pinctrl_state *cam1_pnd_h = NULL;
struct pinctrl_state *cam1_pnd_l = NULL;
struct pinctrl_state *cam1_rst_h = NULL;
struct pinctrl_state *cam1_rst_l = NULL;
struct pinctrl_state *cam_ldo0_h = NULL;
struct pinctrl_state *cam_ldo0_l = NULL;

int mtkcam_gpio_init(struct platform_device *pdev)
{
	int ret = 0;

	camctrl = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR(camctrl)) {
		dev_err(&pdev->dev, "Cannot find camera pinctrl!");
		ret = PTR_ERR(camctrl);
	}
    /*Cam0 Power/Rst Ping initialization*/
	cam0_pnd_h = pinctrl_lookup_state(camctrl, "cam0_pnd1");
	if (IS_ERR(cam0_pnd_h)) {
		ret = PTR_ERR(cam0_pnd_h);
		pr_debug("%s : pinctrl err, cam0_pnd_h\n", __func__);
	}

	cam0_pnd_l = pinctrl_lookup_state(camctrl, "cam0_pnd0");
	if (IS_ERR(cam0_pnd_l)) {
		ret = PTR_ERR(cam0_pnd_l);
		pr_debug("%s : pinctrl err, cam0_pnd_l\n", __func__);
	}


	cam0_rst_h = pinctrl_lookup_state(camctrl, "cam0_rst1");
	if (IS_ERR(cam0_rst_h)) {
		ret = PTR_ERR(cam0_rst_h);
		pr_debug("%s : pinctrl err, cam0_rst_h\n", __func__);
	}

	cam0_rst_l = pinctrl_lookup_state(camctrl, "cam0_rst0");
	if (IS_ERR(cam0_rst_l)) {
		ret = PTR_ERR(cam0_rst_l);
		pr_debug("%s : pinctrl err, cam0_rst_l\n", __func__);
	}

    /*Cam1 Power/Rst Ping initialization*/
	cam1_pnd_h = pinctrl_lookup_state(camctrl, "cam1_pnd1");
	if (IS_ERR(cam1_pnd_h)) {
		ret = PTR_ERR(cam1_pnd_h);
		pr_debug("%s : pinctrl err, cam1_pnd_h\n", __func__);
	}

	cam1_pnd_l = pinctrl_lookup_state(camctrl, "cam1_pnd0");
	if (IS_ERR(cam1_pnd_l )) {
		ret = PTR_ERR(cam1_pnd_l );
		pr_debug("%s : pinctrl err, cam1_pnd_l\n", __func__);
	}


	cam1_rst_h = pinctrl_lookup_state(camctrl, "cam1_rst1");
	if (IS_ERR(cam1_rst_h)) {
		ret = PTR_ERR(cam1_rst_h);
		pr_debug("%s : pinctrl err, cam1_rst_h\n", __func__);
	}


	cam1_rst_l = pinctrl_lookup_state(camctrl, "cam1_rst0");
	if (IS_ERR(cam1_rst_l)) {
		ret = PTR_ERR(cam1_rst_l);
		pr_debug("%s : pinctrl err, cam1_rst_l\n", __func__);
	}
	/*externel LDO enable */
	cam_ldo0_h = pinctrl_lookup_state(camctrl, "cam_ldo0_1");
	if (IS_ERR(cam_ldo0_h)) {
		ret = PTR_ERR(cam_ldo0_h);
		pr_debug("%s : pinctrl err, cam_ldo0_h\n", __func__);
	}


	cam_ldo0_l = pinctrl_lookup_state(camctrl, "cam_ldo0_0");
	if (IS_ERR(cam_ldo0_l)) {
		ret = PTR_ERR(cam_ldo0_l);
		pr_debug("%s : pinctrl err, cam_ldo0_l\n", __func__);
	}
	return ret;
}


static char pwrType2name[5][10]={"CAMPDN","CAMRST","CAMLDO","CAM1PDN","CAM1RST"};
int mtkcam_gpio_set(int PinIdx, int PwrType, int Val)
{
	int ret = 0;

       if(PwrType>CAMLDO)
        return ret;
       
	PK_DBG("PinIdx(%s) PwrType(%s) val(%d)\n", PinIdx?"sub":"main", pwrType2name[PwrType], Val);

	switch (PwrType) {
	case CAMRST:
		if (PinIdx == 0) {
			if (Val == 0)
                   {         
				if (!IS_ERR(cam0_rst_l))
				pinctrl_select_state(camctrl, cam0_rst_l);
                         else
                            PK_DBG("cam0_rst_l pinctrl is invalid !!\n");
                   }
			else
                   {         
				if (!IS_ERR(cam0_rst_l))
				pinctrl_select_state(camctrl, cam0_rst_h);
                         else
                            PK_DBG("cam0_rst_h pinctrl is invalid !!\n");
                    }
		} else {
			if (Val == 0)
                   {         
				if (!IS_ERR(cam1_rst_l))
				pinctrl_select_state(camctrl, cam1_rst_l);
                         else
                            PK_DBG("cam1_rst_l pinctrl is invalid !!\n");
                   }
			else
                  {         
				if (!IS_ERR(cam1_rst_h))
				pinctrl_select_state(camctrl, cam1_rst_h);
                         else
                            PK_DBG("cam1_rst_h pinctrl is invalid !!\n");
                   }
		}
		break;
	case CAMPDN:
		if (PinIdx == 0) {
			if (Val == 0)
                  {         
				if (!IS_ERR(cam0_pnd_l))
				pinctrl_select_state(camctrl, cam0_pnd_l);
                        else
                           PK_DBG("cam0_pnd_l pinctrl is invalid !!\n");
                   }
			else
                  {         
				if (!IS_ERR(cam0_pnd_h))
				pinctrl_select_state(camctrl, cam0_pnd_h);
                         else
                            PK_DBG("cam0_pnd_h pinctrl is invalid !!\n");
             }
		} else {
			if (Val == 0)
                  {         
				if (!IS_ERR(cam1_pnd_l))
				pinctrl_select_state(camctrl, cam1_pnd_l);
                         else
                            PK_DBG("cam1_pnd_l pinctrl is invalid !!\n");
                  }
			else
                   {         
				if (!IS_ERR(cam1_pnd_h))
				pinctrl_select_state(camctrl, cam1_pnd_h);
                         else
                            PK_DBG("cam1_pnd_h pinctrl is invalid !!\n");
                  }
		}

		break;
	case CAMLDO:
		if (Val == 0)
            {      
			if (!IS_ERR(cam_ldo0_l))
			pinctrl_select_state(camctrl, cam_ldo0_l);
                   else
                      PK_DBG("cam_ldo0_l pinctrl is invalid !!\n");
            }
		else
            {      
			if (!IS_ERR(cam_ldo0_h))
			pinctrl_select_state(camctrl, cam_ldo0_h);
                   else
                      PK_DBG("cam_ldo0_h pinctrl is invalid !!\n");
          }
		break;
	default:
		PK_DBG("PwrType(%d) is invalid !!\n", PwrType);
		break;
	};

	PK_DBG("PinIdx(%d) PwrType(%d) val(%d)\n", PinIdx, PwrType, Val);

	return ret;
}

#if 0
static char Powertpye_Toname[POWR_TYPE_MAX][32]=
{
"VDD_NONE","PDN","RST","MCLK",CAMERA_POWER_VCAM_A,CAMERA_POWER_VCAM_D,CAMERA_POWER_VCAM_IO,CAMERA_POWER_VCAM_AF
};
#endif



//extern struct platform_device *camerahw_platform_device;
#define GPIO_UNSUPPORTED -1

#define IDX_PS_MODE 1
#define IDX_PS_ON   2
#define IDX_PS_OFF  3


#define IDX_PS_CMRST 0
#define IDX_PS_CMPDN 4


#ifndef BOOL
typedef unsigned char BOOL;
#endif


extern char 	g_MainSensorName[32];
extern char 	g_SubSensorName[32];
unsigned int	g_main_rst_output_when_sensor_disable 	= Vol_Low;
unsigned int	g_main_pwd_output_when_sensor_disable 	= Vol_High;
unsigned int	g_sub_rst_output_when_sensor_disable 	= Vol_Low;
unsigned int	g_sub_pwd_output_when_sensor_disable	= Vol_High;

u32 pinSetIdx = 0;//default main sensor
u32 pinSet[3][8] = {

	{CAMERA_CMRST_PIN,
	 CAMERA_CMRST_PIN_M_GPIO,	/* mode */
	 GPIO_OUT_ONE,	/* ON state */
	 GPIO_OUT_ZERO,	/* OFF state */
	 CAMERA_CMPDN_PIN,
	 CAMERA_CMPDN_PIN_M_GPIO,
	 GPIO_OUT_ONE,
	 GPIO_OUT_ZERO,
	 },
	{CAMERA_CMRST1_PIN,
	 CAMERA_CMRST1_PIN_M_GPIO,
	 GPIO_OUT_ONE,
	 GPIO_OUT_ZERO,
	 CAMERA_CMPDN1_PIN,
	 CAMERA_CMPDN1_PIN_M_GPIO,
	 GPIO_OUT_ONE,
	 GPIO_OUT_ZERO,
	 },
	{GPIO_CAMERA_INVALID,
	 GPIO_CAMERA_INVALID,	/* mode */
	 GPIO_OUT_ONE,	/* ON state */
	 GPIO_OUT_ZERO,	/* OFF state */
	 GPIO_CAMERA_INVALID,
	 GPIO_CAMERA_INVALID,
	 GPIO_OUT_ONE,
	 GPIO_OUT_ZERO,
	 }
};

PowerCust PowerCustList={
	{
	{GPIO_UNSUPPORTED,GPIO_MODE_GPIO,Vol_Low},   //for AVDD;
	{GPIO_UNSUPPORTED,GPIO_MODE_GPIO,Vol_Low},   //for DVDD;
	{GPIO_UNSUPPORTED,GPIO_MODE_GPIO,Vol_Low},   //for DOVDD;
	{GPIO_UNSUPPORTED,GPIO_MODE_GPIO,Vol_Low},   //for AFVDD;
	{GPIO_UNSUPPORTED,GPIO_MODE_GPIO,Vol_Low},   //for AFEN;
	}
};

PowerUp PowerOnList={
	{
#if defined(OV9760_MIPI_RAW)
		{SENSOR_DRVNAME_OV9760_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{DOVDD, Vol_1800, 10},
				{AVDD,  Vol_2800, 10},
				{DVDD,  Vol_1200, 10},
				{AFVDD, Vol_2800, 15},
				{PDN,   Vol_Low,  10},
				{RST,   Vol_Low,  10},
				{PDN,   Vol_High, 10},
				{RST,   Vol_High, 10}
			},
		},
#endif

#if defined(OV9762_MIPI_RAW)
		{SENSOR_DRVNAME_OV9762_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{DOVDD, Vol_1800, 10},
				{AVDD,  Vol_2800, 10},
				{DVDD,  Vol_1200, 10},
				{AFVDD, Vol_2800, 15},
				{PDN,   Vol_Low,  10},
				{RST,   Vol_Low,  10},
				{PDN,   Vol_High, 10},
				{RST,   Vol_High, 10}
			},
		},
#endif
#if defined(OV13850_MIPI_RAW)
		{SENSOR_DRVNAME_OV13850_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{DOVDD, Vol_1800, 1},
				{AVDD,  Vol_2800, 1},
				{DVDD,  Vol_1200, 1},
				{AFVDD, Vol_2800, 5},
				{PDN,   Vol_Low,  1},
				{RST,   Vol_Low,  1},
				{PDN,   Vol_High, 0},
				{RST,   Vol_High, 0}
			},
		},
#endif
#if defined(OV13850R1A_MIPI_RAW)
		{SENSOR_DRVNAME_OV13850R1A_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{DOVDD, Vol_1800, 1},
				{AVDD,  Vol_2800, 1},
				{DVDD,  Vol_1200, 1},
				{AFVDD, Vol_2800, 5},
				{PDN,   Vol_Low,  1},
				{RST,   Vol_Low,  1},
				{PDN,   Vol_High, 0},
				{RST,   Vol_High, 0}
			},
		},
#endif
#if defined(OV13850R2A_MIPI_RAW)
		{SENSOR_DRVNAME_OV13850R2A_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{DOVDD, Vol_1800, 1},
				{AVDD,  Vol_2800, 1},
				{DVDD,  Vol_1200, 1},
				{AFVDD, Vol_2800, 5},
				{PDN,   Vol_Low,  1},
				{RST,   Vol_Low,  1},
				{PDN,   Vol_High, 0},
				{RST,   Vol_High, 0}
			},
		},
#endif

#if defined(OV8865_MIPI_RAW)
		{SENSOR_DRVNAME_OV8865_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{DOVDD, Vol_1800, 1},
				{AVDD,  Vol_2800, 1},
				{DVDD,  Vol_1200, 1},
				{AFVDD, Vol_2800, 5},
				{PDN,   Vol_Low,  1},
				{RST,   Vol_Low,  1},
				{PDN,   Vol_High, 0},
				{RST,   Vol_High, 0}
			},
		},
#endif
#if defined(OV8858_MIPI_RAW)
		{SENSOR_DRVNAME_OV8858_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{AVDD,  Vol_2800, 1},
				{DOVDD, Vol_1800, 1},
				{DVDD,  Vol_1200, 1},
				{AFVDD, Vol_2800, 5},
				{PDN,   Vol_Low,  1},
				{RST,   Vol_Low,  1},
				{PDN,   Vol_High, 0},
				{RST,   Vol_High, 0}
			},
		},
#endif
#if defined(S5K5E2YA_MIPI_RAW)
		{SENSOR_DRVNAME_S5K5E2YA_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{DOVDD, Vol_1800, 0},
				{AVDD,  Vol_2800, 0},
				{DVDD,  Vol_1200, 0},
				{AFVDD, Vol_2800, 5},
				{PDN,   Vol_Low,  4},
				{PDN,   Vol_High, 0},
				{RST,   Vol_Low,  1},
				{RST,   Vol_High, 0},
			},
		},
#endif
#if defined(S5K2P8_MIPI_RAW)
		{SENSOR_DRVNAME_S5K2P8_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{DOVDD, Vol_1800, 0},
				{AVDD,  Vol_2800, 0},
				{DVDD,  Vol_1200, 0},
				{AFVDD, Vol_2800, 5},
				{PDN,   Vol_Low,  4},
				{PDN,   Vol_High, 0},
				{RST,   Vol_Low,  1},
				{RST,   Vol_High, 0},
			},
		},
#endif
#if defined(OV5648_MIPI_RAW)
		{SENSOR_DRVNAME_OV5648_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{DOVDD, Vol_1800, 1},
				{AVDD,  Vol_2800, 1},
				{DVDD,  Vol_1500, 1},
				{AFVDD, Vol_2800, 5},
				{PDN,   Vol_Low,  1},
				{RST,   Vol_Low,  1},
				{PDN,   Vol_High, 0},
				{RST,   Vol_High, 0}
			},
		},
#endif
#if defined(OV2680_MIPI_RAW)
		{SENSOR_DRVNAME_OV2680_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{DOVDD, Vol_1800, 1},
				{AVDD,  Vol_2800, 1},
				{DVDD,  Vol_1500, 1},
				{AFVDD, Vol_2800, 5},
				{PDN,   Vol_Low,  1},
				{RST,   Vol_Low,  1},
				{PDN,   Vol_High, 0},
				{RST,   Vol_High, 0}
			},
		},
#endif
#if defined(OV16825_MIPI_RAW)
		{SENSOR_DRVNAME_OV16825_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{DOVDD, Vol_1800, 0},
				{AVDD,  Vol_2800, 0},
				{DVDD,  Vol_1200, 0},
				{AFVDD, Vol_2800, 5},
				{PDN,   Vol_Low,  0},
				{RST,   Vol_Low,  0},
				{RST,   Vol_High, 0},
			},
		},
#endif
#if defined(IMX135_MIPI_RAW)
		{SENSOR_DRVNAME_IMX135_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{AVDD,	Vol_2800, 10},
				{DOVDD, Vol_1800, 10},
				{DVDD,	Vol_1100, 10},
				{AFVDD, Vol_2800, 5},
                {PDN,	Vol_Low, 0},
                {PDN,	Vol_High, 0},
                {RST,   Vol_Low,  0},
				{RST,	Vol_High, 0}
			},
		},
#endif
#if defined(IMX175_MIPI_RAW)
		{SENSOR_DRVNAME_IMX175_MIPI_RAW,
			{
/*
				{AVDD, Vol_2800, 1},
				{DOVDD,	Vol_1800, 1},
				{SensorMCLK,Vol_High, 1},
				{AFVDD, Vol_2800, 5},
				{PDN,	Vol_Low,  1},
				{RST,	Vol_Low,  1},
				{PDN,	Vol_High, 1},
				{RST,	Vol_High, 1},
				{DVDD,	Vol_1200, 1}

*/
				{AVDD, Vol_2800, 1},
				{DVDD,	Vol_1200, 1},
				{DOVDD,	Vol_1800, 1},
				{SensorMCLK,Vol_High, 1},
				{AFVDD, Vol_2800, 5},
				{PDN,	Vol_Low,  1},
				{RST,	Vol_Low,  1},
				{PDN,	Vol_High, 1},
				{RST,	Vol_High, 1}
			},
		},
#endif
#if defined(IMX145_MIPI_RAW)
		{SENSOR_DRVNAME_IMX145_MIPI_RAW,
			{
				{AVDD, Vol_2800, 1},
				{DOVDD,	Vol_1800, 1},
				{SensorMCLK,Vol_High, 1},
				{AFVDD, Vol_2800, 5},
				{PDN,	Vol_Low,  1},
				{RST,	Vol_Low,  1},
				{PDN,	Vol_High, 1},
				{RST,	Vol_High, 1},
				{DVDD,	Vol_1200, 1}
			},
		},
#endif
#if defined(IMX179_MIPI_RAW)
		{SENSOR_DRVNAME_IMX179_MIPI_RAW,
			{
				{AVDD, Vol_2800, 1},
				{DOVDD,	Vol_1800, 1},

				{SensorMCLK,Vol_High, 1},
				{AFVDD, Vol_2800, 5},
				{PDN,	Vol_Low,  1},
				{RST,	Vol_Low,  1},
				{PDN,	Vol_High, 1},
				{RST,	Vol_High, 1},
				{DVDD,	Vol_1200, 1}

			},
		},
#endif
#if defined(HI841_MIPI_RAW)
		{SENSOR_DRVNAME_HI841_MIPI_RAW,
			{
				{PDN,	Vol_High, 1},
				{RST,   Vol_Low,  1},
				{DOVDD,	Vol_1800, 1},
				{AVDD,	Vol_2800, 1},
				{DVDD,	Vol_1200, 1},
				{AFVDD, Vol_2800, 5},
				{SensorMCLK,Vol_High, 0},
				{PDN,	Vol_Low, 0},
				{RST,	Vol_High, 0}
			},
		},
#endif
#if defined(HI545_MIPI_RAW)
		{SENSOR_DRVNAME_HI545_MIPI_RAW,
			{
				/*{SensorMCLK,Vol_High, 0},
				{DOVDD, Vol_1800, 1},
				{AVDD,  Vol_2800, 1},
				{DVDD,  Vol_1200, 1},
				{AFVDD, Vol_2800, 5},
				{PDN,   Vol_Low,  1},
				{RST,   Vol_Low,  1},
				{PDN,   Vol_High, 0},
				{RST,   Vol_High, 0}
				*/
				{PDN, Vol_Low, 1},
				{RST, Vol_Low, 1},
				{DOVDD, Vol_1800, 1},
				{AVDD, Vol_2800, 1},
				{DVDD, Vol_1200, 1},
				{AFVDD, Vol_2800, 5},
				{PDN, Vol_High, 0},
				{SensorMCLK, Vol_High, 0},
				{RST, Vol_High, 0}
			},
		},
#endif
#if defined(HI544_MIPI_RAW)
		{SENSOR_DRVNAME_HI544_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{DOVDD, Vol_1800, 1},
				{AVDD,  Vol_2800, 1},
				{DVDD,  Vol_1200, 1},
				{AFVDD, Vol_2800, 5},
				{PDN,   Vol_Low,  1},
				{RST,   Vol_Low,  1},
				{PDN,   Vol_High, 0},
				{RST,   Vol_High, 0}
			},
		},
#endif
#if defined(GC0310_MIPI_YUV)
		{SENSOR_DRVNAME_GC0310_MIPI_YUV,
			{
				{PDN,	Vol_High, 1},
				{RST,   Vol_Low,  1},
				{DOVDD,	Vol_1800, 1},
				{DVDD,	Vol_1500, 1},
				{AVDD,	Vol_2800, 1},
				{AFVDD, Vol_2800, 5},
				{SensorMCLK,Vol_High, 0},
				{PDN,	Vol_Low, 0},
				{RST,	Vol_High, 0}
			},
		},
#endif
#if defined(GC5005_MIPI_RAW)
                {SENSOR_DRVNAME_GC5005MIPI_RAW,
                       {
                              {PDN,   Vol_High, 1},
                              {RST,   Vol_Low,  1},
                              {DOVDD, Vol_1800, 1},
                              {DVDD,  Vol_1200, 1},
                              {AVDD,  Vol_2800, 1},
                              {AFVDD, Vol_2800, 5},
                              {SensorMCLK,Vol_High, 0},
                              {PDN,   Vol_Low, 0},
                              {RST,   Vol_High, 0}
                        },
                },
#endif

#if defined(GC2355_MIPI_RAW)
		{SENSOR_DRVNAME_GC2355_MIPI_RAW,
			{
				{PDN,	Vol_High, 1},
				{RST,   Vol_Low,  1},
				{DOVDD,	Vol_1800, 1},
				{DVDD,	Vol_1500, 1},
				{AVDD,	Vol_2800, 1},
				{AFVDD, Vol_2800, 5},
				{SensorMCLK,Vol_High, 0},
				{PDN,	Vol_Low, 0},
				{RST,	Vol_High, 0}
			},
		},
#endif
#if defined(GC2365_MIPI_RAW)
		{SENSOR_DRVNAME_GC2365_MIPI_RAW,
			{
				{PDN,	Vol_High, 1},
				{RST,   Vol_Low,  1},
				{DOVDD,	Vol_1800, 1},
				{DVDD,	Vol_1500, 1},
				{AVDD,	Vol_2800, 1},
				{AFVDD, Vol_2800, 5},
				{SensorMCLK,Vol_High, 0},
				{PDN,	Vol_Low, 0},
				{RST,	Vol_High, 0}
			},
		},
#endif
#if defined(GC0409_MIPI_RAW)
		{SENSOR_DRVNAME_GC0409MIPI_RAW,
			{
				{PDN,	Vol_High, 1},
				{RST,   Vol_Low,  1},
				{DOVDD,	Vol_1800, 1},
				{DVDD,	Vol_1800, 1},
				{AVDD,	Vol_2800, 1},
				{AFVDD, Vol_2800, 5},
				{SensorMCLK,Vol_High, 0},
				{PDN,	Vol_Low, 0},
				{RST,	Vol_High, 0}
			},
		},
#endif
#if defined(GC5004_MIPI_RAW)
		{SENSOR_DRVNAME_GC5004MIPI_RAW,
			{
				{PDN,	Vol_High, 1},
				{RST,   Vol_Low,  1},
				{DOVDD,	Vol_1800, 1},
				{DVDD,	Vol_1500, 1},
				{AVDD,	Vol_2800, 1},
				{AFVDD, Vol_2800, 5},
				{SensorMCLK,Vol_High, 0},
				{PDN,	Vol_Low, 0},
				{RST,	Vol_High, 0}
			},
		},
#endif
#if defined(GC5024_MIPI_RAW)
		{SENSOR_DRVNAME_GC5024MIPI_RAW,
			{
				{PDN,	Vol_High, 1},
				{RST,   Vol_Low,  1},
				{DOVDD,	Vol_1800, 1},
				{DVDD,	Vol_1500, 1},
				{AVDD,	Vol_2800, 1},
				{AFVDD, Vol_2800, 5},
				{SensorMCLK,Vol_High, 0},
				{PDN,	Vol_Low, 0},
				{RST,	Vol_High, 0}
			},
		},
#endif
#if defined(GC2355_RAW)
		{SENSOR_DRVNAME_GC2355_RAW,
			{
				{PDN,	Vol_High, 1},
				{RST,   Vol_Low,  1},
				{DOVDD,	Vol_1800, 1},
				{DVDD,	Vol_1500, 1},
				{AVDD,	Vol_2800, 1},
				{SensorMCLK,Vol_High, 0},
				{PDN,	Vol_Low, 0},
				{RST,	Vol_High, 0}
			},
		},
#endif

#if defined(GC2145_YUV)
		{SENSOR_DRVNAME_GC2145_YUV,
			{
				{PDN,	Vol_High, 1},
				{RST,   Vol_Low,  1},
				{DOVDD,	Vol_1800, 1},
				{DVDD,	Vol_1500, 1},//no used
				{AVDD,	Vol_2800, 1},
				{SensorMCLK,Vol_High, 0},
				{PDN,	Vol_Low, 0},
				{RST,	Vol_High, 0}
			},
		},
#endif

#if defined(GC0329_YUV)
		{SENSOR_DRVNAME_GC0329_YUV,
			{
				{PDN,	Vol_High, 1},
				{RST,   Vol_Low,  1},
				{DOVDD,	Vol_1800, 1},
				{DVDD,	Vol_1500, 1},//no used
				{AVDD,	Vol_2800, 1},
				{SensorMCLK,Vol_High, 0},
				{PDN,	Vol_Low, 0},
				{RST,	Vol_High, 0}
			},
		},
#endif

#if defined(GC0312_YUV)
		{SENSOR_DRVNAME_GC0312_YUV,
			{
				{PDN,	Vol_High, 1},
				{RST,   Vol_Low,  1},
				{DOVDD,	Vol_1800, 1},
				{DVDD,	Vol_1500, 1},
				{AVDD,	Vol_2800, 1},
				{SensorMCLK,Vol_High, 0},
				{PDN,	Vol_Low, 0},
				{RST,	Vol_High, 0}
			},
		},
#endif

#if defined(HM8131_MIPI_RAW)
				{SENSOR_DRVNAME_HM8131_MIPI_RAW,
					{
						{AVDD,  Vol_2800, 1},
						{DVDD,	Vol_1500, 1},
						{DOVDD,	Vol_1800, 1},
						{AFVDD, Vol_2800, 5},
						{PDN,	Vol_Low,  1},
						{RST,	Vol_Low,  1},
						{SensorMCLK,Vol_High, 0},
						{PDN,	Vol_High, 0},
						{RST,	Vol_High, 0}
					},
				},
#endif
#if defined(HM5040_MIPI_RAW)
				{SENSOR_DRVNAME_HM5040_MIPI_RAW,
					{
						{DOVDD, Vol_1800, 1},
						{AVDD,	Vol_2800, 1},
						{AFVDD, Vol_2800, 5},
						{PDN,	Vol_Low,  1},
						{RST,	Vol_Low,  1},
						{SensorMCLK,Vol_High, 0},
						{PDN,	Vol_High, 0},
						{RST,	Vol_High, 0}
					},
				},
#endif
#if defined(HM8040_MIPI_RAW)
				{SENSOR_DRVNAME_HM8040_MIPI_RAW,
					{
						{DOVDD, Vol_1800, 1},
						{AVDD,	Vol_2800, 1},
						{DVDD,	Vol_1200, 1},
						{AFVDD, Vol_2800, 5},
						{PDN,	Vol_Low,  1},
						{RST,	Vol_Low,  1},
						{SensorMCLK,Vol_High, 0},
						{PDN,	Vol_High, 0},
						{RST,	Vol_High, 0}
					},
				},
#endif
#if defined(GC2755_MIPI_RAW)
		{SENSOR_DRVNAME_GC2755_MIPI_RAW,
			{
				{DOVDD,	Vol_1800, 1},
				{AVDD,	Vol_2800, 1},
				{DVDD,	Vol_1500, 1},//no used
				{AFVDD, Vol_2800, 5},
				{PDN,	Vol_High, 1},
				{RST,   Vol_Low,  1},
				{SensorMCLK,Vol_High, 0},
				{PDN,	Vol_Low, 0},
				{RST,	Vol_High, 0}
			},
		},
#endif
#if defined(SP5409_MIPI_RAW)
		{SENSOR_DRVNAME_SP5409_MIPI_RAW,
			{
				{AVDD, Vol_2800, 1},
				{DOVDD,	Vol_1800, 1},
#if defined(CONFIG_T93A_PROJ)
				{DVDD,	Vol_1800, 1},  // this voltage need check with fae , every module may different
#else
				{DVDD,	Vol_1500, 1},  // this voltage need check with fae , every module may different
#endif
				{SensorMCLK,Vol_High, 0},
				{AFVDD, Vol_2800, 5},
				{PDN,	Vol_Low,  1},
				{RST,	Vol_Low,  1},
				{PDN,	Vol_High, 1},
				{PDN,	Vol_Low,  0},
				{RST,	Vol_High, 0}
			},
		},
#endif
#if defined(IMX219_MIPI_RAW)
		{SENSOR_DRVNAME_IMX219_MIPI_RAW,
			{
				{AVDD, Vol_2800, 1},
				{DVDD,	Vol_1200, 1},
				{DOVDD,	Vol_1800, 1},
				{SensorMCLK,Vol_High, 1},
				{AFVDD, Vol_2800, 5},
				{PDN,	Vol_Low,  1},
				{RST,	Vol_Low,  1},
				{PDN,	Vol_High, 1},
				{RST,	Vol_High, 1}
			},
		},
#endif
#if defined(IMX219_MIPI_RAW_SUB)
		{SENSOR_DRVNAME_IMX219_MIPI_RAW_SUB,
			{
				{AVDD, Vol_2800, 1},
				{DVDD,	Vol_1200, 1},
				{DOVDD,	Vol_1800, 1},
				{SensorMCLK,Vol_High, 1},
				{AFVDD, Vol_2800, 5},
				{PDN,	Vol_Low,  1},
				{RST,	Vol_Low,  1},
				{PDN,	Vol_High, 1},
				{RST,	Vol_High, 1}
			},
		},
#endif
#if defined(IMX258_MIPI_RAW)
		{SENSOR_DRVNAME_IMX258_MIPI_RAW,
			{
				{AVDD, Vol_2800, 1},
				{DVDD,	Vol_1200, 1},
				{DOVDD,	Vol_1800, 1},
				{SensorMCLK,Vol_High, 1},
				{AFVDD, Vol_2800, 5},
				{PDN,	Vol_Low,  1},
				{RST,	Vol_Low,  1},
				{PDN,	Vol_High, 1},
				{RST,	Vol_High, 1}
			},
		},
#endif
#if defined(IMX214_MIPI_RAW)
		{SENSOR_DRVNAME_IMX214_MIPI_RAW,
			{
				{AVDD, Vol_2800, 1},
				{DOVDD,	Vol_1800, 1},

				{SensorMCLK,Vol_High, 1},
				{AFVDD, Vol_2800, 5},
				{PDN,	Vol_Low,  1},
				{RST,	Vol_Low,  1},
				{PDN,	Vol_High, 1},
				{RST,	Vol_High, 1},
				{DVDD,	Vol_1200, 1}

			},
		},
#endif
#if defined(OV5670_MIPI_RAW)
		{SENSOR_DRVNAME_OV5670_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{DOVDD, Vol_1800, 1},
				{AVDD,  Vol_2800, 1},
				{DVDD,  Vol_1200, 1},
				{AFVDD, Vol_2800, 5},
				{PDN,   Vol_Low,  1},
				{RST,   Vol_Low,  1},
				{PDN,   Vol_High, 0},
				{RST,   Vol_High, 0}
			},
		},
#endif
#if defined(SP0A19_YUV)
		{SENSOR_DRVNAME_SP0A19_YUV,
			{
				{PDN,	Vol_High, 1},
				{RST,   Vol_Low,  1},
				{DOVDD,	Vol_1800, 1},
				{DVDD,	Vol_1500, 1},//no used
				{AVDD,	Vol_2800, 1},
				{SensorMCLK,Vol_High, 0},
				{PDN,	Vol_Low, 0},
				{RST,	Vol_High, 0}
			},
		},
#endif
#if defined(HI551_MIPI_RAW)
		{SENSOR_DRVNAME_HI551_MIPI_RAW,
			{
				{PDN, Vol_Low, 1},
				{RST, Vol_Low, 1},
				{DOVDD, Vol_1800, 1},
				{AVDD, Vol_2800, 1},
				{DVDD, Vol_1800, 1},
				{AFVDD, Vol_2800, 5},
				{PDN, Vol_High, 0},
				{SensorMCLK, Vol_High, 0},
				{RST, Vol_High, 0}
			},
		},
#endif
#if defined(S5K4H5YC_MIPI_RAW)
		{SENSOR_DRVNAME_S5K4H5YC_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{DOVDD, Vol_1800, 0},
				{AVDD,  Vol_2800, 0},
				{DVDD,  Vol_1200, 0},
				{AFVDD, Vol_2800, 5},
				{PDN,   Vol_Low,  4},
				{PDN,   Vol_High, 0},
				{RST,   Vol_Low,  1},
				{RST,   Vol_High, 0},
			},
		},
#endif
#if defined(S5K3L2_MIPI_RAW)
		{SENSOR_DRVNAME_S5K3L2_MIPI_RAW,
			{
				{SensorMCLK,Vol_High, 0},
				{DOVDD, Vol_1800, 0},
				{AVDD,  Vol_2800, 0},
				{DVDD,  Vol_1200, 0},
				{AFVDD, Vol_2800, 5},
				{PDN,   Vol_Low,  4},
				{RST,   Vol_Low,  1},
				{PDN,   Vol_High, 0},
				{RST,   Vol_High, 0},
			},
		},
#endif
#if defined(MN34152_MIPI_RAW)
                {SENSOR_DRVNAME_MN34152_MIPI_RAW,
                        {
				{PDN,   Vol_Low,  1},
				{RST,   Vol_Low,  1},
                                {SensorMCLK,Vol_High, 0},
				{DOVDD, Vol_1800, 1},
				{AVDD,  Vol_2800, 1},
				{DVDD,  Vol_1100, 5},
				{AFVDD, Vol_2800, 1},
				{PDN,   Vol_High, 1},
				{RST,   Vol_High, 20}
                        },
                },
#endif
#if defined(S5K4H8_MIPI_RAW)
	   {SENSOR_DRVNAME_S5K4H8_MIPI_RAW,
	  {
	  {PDN, Vol_Low, 4},
	  {SensorMCLK, Vol_High, 0},
	  {RST, Vol_Low, 1},
	  {AVDD, Vol_2800, 1},
	  {DOVDD, Vol_1800, 1},
	  {DVDD, Vol_1200, 5},
	  {AFVDD, Vol_2800, 1},
	  {PDN, Vol_High, 2},
	  {RST, Vol_High, 20},
	  },
	 },
#endif

		//add new sensor before this line
		{NULL,},
	}
};



static int cntVCAMD = 0;
static int cntVCAMA = 0;
static int cntVCAMIO = 0;
static int cntVCAMAF = 0;
static int cntVCAMD_SUB = 0;

static DEFINE_SPINLOCK(kdsensor_pw_cnt_lock);


bool _hwPowerOnCnt(KD_REGULATOR_TYPE_T powerId, int powerVolt)
{

	if (_hwPowerOn(powerId, powerVolt)) {
		spin_lock(&kdsensor_pw_cnt_lock);
		if (powerId == VCAMD)
			cntVCAMD += 1;
		else if (powerId == VCAMA)
			cntVCAMA += 1;
		else if (powerId == VCAMIO)
			cntVCAMIO += 1;
		else if (powerId == VCAMAF)
			cntVCAMAF += 1;
		else if (powerId == SUB_VCAMD)
			cntVCAMD_SUB += 1;
		spin_unlock(&kdsensor_pw_cnt_lock);
		return true;
	}
	return false;
}

bool _hwPowerDownCnt(KD_REGULATOR_TYPE_T powerId)
{

	if (_hwPowerDown(powerId)) {
		spin_lock(&kdsensor_pw_cnt_lock);
		if (powerId == VCAMD)
			cntVCAMD -= 1;
		else if (powerId == VCAMA)
			cntVCAMA -= 1;
		else if (powerId == VCAMIO)
			cntVCAMIO -= 1;
		else if (powerId == VCAMAF)
			cntVCAMAF -= 1;
		else if (powerId == SUB_VCAMD)
			cntVCAMD_SUB -= 1;
		spin_unlock(&kdsensor_pw_cnt_lock);
		return true;
	}
	return false;
}

void checkPowerBeforClose(char *mode_name)
{

	int i = 0;

	PK_DBG
	    ("[checkPowerBeforClose]cntVCAMD:%d, cntVCAMA:%d,cntVCAMIO:%d, cntVCAMAF:%d, cntVCAMD_SUB:%d,\n",
	     cntVCAMD, cntVCAMA, cntVCAMIO, cntVCAMAF, cntVCAMD_SUB);


	for (i = 0; i < cntVCAMD; i++)
		_hwPowerDown(VCAMD);
	for (i = 0; i < cntVCAMA; i++)
		_hwPowerDown(VCAMA);
	for (i = 0; i < cntVCAMIO; i++)
		_hwPowerDown(VCAMIO);
	for (i = 0; i < cntVCAMAF; i++)
		_hwPowerDown(VCAMAF);
	for (i = 0; i < cntVCAMD_SUB; i++)
		_hwPowerDown(SUB_VCAMD);

	cntVCAMD = 0;
	cntVCAMA = 0;
	cntVCAMIO = 0;
	cntVCAMAF = 0;
	cntVCAMD_SUB = 0;

}


static void config_sensor_rst_pwd(char *currSensorName)
{
	int pwListIdx,pwIdx;
	for(pwListIdx=0 ; pwListIdx<16; pwListIdx++)
	{
		//currSensorName
		if(currSensorName && (PowerOnList.PowerSeq[pwListIdx].SensorName!=NULL) && (0 == strcmp(PowerOnList.PowerSeq[pwListIdx].SensorName,currSensorName)))
		{
			for(pwIdx=0;pwIdx<12;pwIdx++)
			{  
				if(PowerOnList.PowerSeq[pwListIdx].PowerInfo[pwIdx].PowerType == RST)
				{
					if(pinSetIdx==0)
						g_main_rst_output_when_sensor_disable = PowerOnList.PowerSeq[pwListIdx].PowerInfo[pwIdx].Voltage;
					else
						g_sub_rst_output_when_sensor_disable = PowerOnList.PowerSeq[pwListIdx].PowerInfo[pwIdx].Voltage;
					break;
				}
			}
			for(pwIdx=0;pwIdx<12;pwIdx++)
			{  
				if(PowerOnList.PowerSeq[pwListIdx].PowerInfo[pwIdx].PowerType == PDN)
				{
					if(pinSetIdx==0)
						g_main_pwd_output_when_sensor_disable = PowerOnList.PowerSeq[pwListIdx].PowerInfo[pwIdx].Voltage;
					else
						g_sub_pwd_output_when_sensor_disable = PowerOnList.PowerSeq[pwListIdx].PowerInfo[pwIdx].Voltage;
					break;
				}
			}
		}
	}

	for(pwListIdx=0 ; pwListIdx<16; pwListIdx++)
	{
		//g_MainSensorName
		if( (PowerOnList.PowerSeq[pwListIdx].SensorName!=NULL) && (0 == strcmp(PowerOnList.PowerSeq[pwListIdx].SensorName,g_MainSensorName)))
		{
			for(pwIdx=0;pwIdx<12;pwIdx++)
			{  
				if(PowerOnList.PowerSeq[pwListIdx].PowerInfo[pwIdx].PowerType == RST)
				{
					g_main_rst_output_when_sensor_disable = PowerOnList.PowerSeq[pwListIdx].PowerInfo[pwIdx].Voltage;
					break;
				}
			}
			for(pwIdx=0;pwIdx<12;pwIdx++)
			{  
				if(PowerOnList.PowerSeq[pwListIdx].PowerInfo[pwIdx].PowerType == PDN)
				{
					g_main_pwd_output_when_sensor_disable = PowerOnList.PowerSeq[pwListIdx].PowerInfo[pwIdx].Voltage;
					break;
				}
			}
		}
		//g_SubSensorName
		if( (PowerOnList.PowerSeq[pwListIdx].SensorName!=NULL) && (0 == strcmp(PowerOnList.PowerSeq[pwListIdx].SensorName,g_SubSensorName)))
		{
			for(pwIdx=0;pwIdx<12;pwIdx++)
			{  
				if(PowerOnList.PowerSeq[pwListIdx].PowerInfo[pwIdx].PowerType == RST)
				{
					g_sub_rst_output_when_sensor_disable = PowerOnList.PowerSeq[pwListIdx].PowerInfo[pwIdx].Voltage;
					break;
				}
			}
			for(pwIdx=0;pwIdx<12;pwIdx++)
			{  
				if(PowerOnList.PowerSeq[pwListIdx].PowerInfo[pwIdx].PowerType == PDN)
				{
					g_sub_pwd_output_when_sensor_disable = PowerOnList.PowerSeq[pwListIdx].PowerInfo[pwIdx].Voltage;
					break;
				}
			}
		}
	}
	PK_DBG("[imgsensor_poweron] config_sensor_rst_pwd mainrst=%d, mainpwd=%d, subrst=%d, subpwd=%d\n",g_main_rst_output_when_sensor_disable, 
        g_main_pwd_output_when_sensor_disable, g_sub_rst_output_when_sensor_disable, g_sub_pwd_output_when_sensor_disable); 
	
      mtkcam_gpio_set(0, CAMRST,g_main_rst_output_when_sensor_disable);
      mtkcam_gpio_set(0, CAMPDN,g_main_pwd_output_when_sensor_disable);
  
      mtkcam_gpio_set(1, CAMRST,g_sub_rst_output_when_sensor_disable);
      mtkcam_gpio_set(1, CAMPDN,g_sub_pwd_output_when_sensor_disable);
  


}

static void config_subsensor_rst_pwd_when_nosub(char *currSensorName)
{
	int pwListIdx,pwIdx;
	if((0==strcmp(g_SubSensorName,KDIMGSENSOR_NOSENSOR)) && (pinSetIdx==0))
	{
	       
		for(pwListIdx=0 ; pwListIdx<16; pwListIdx++)
		{
			//currSensorName
			if(currSensorName && (PowerOnList.PowerSeq[pwListIdx].SensorName!=NULL) && (0 == strcmp(PowerOnList.PowerSeq[pwListIdx].SensorName,currSensorName)))
			{
				for(pwIdx=0;pwIdx<12;pwIdx++)
				{  
					if(PowerOnList.PowerSeq[pwListIdx].PowerInfo[pwIdx].PowerType == RST)
					{
						g_sub_rst_output_when_sensor_disable = PowerOnList.PowerSeq[pwListIdx].PowerInfo[pwIdx].Voltage;
						break;
					}
				}
				for(pwIdx=0;pwIdx<12;pwIdx++)
				{  
					if(PowerOnList.PowerSeq[pwListIdx].PowerInfo[pwIdx].PowerType == PDN)
					{
						g_sub_pwd_output_when_sensor_disable = PowerOnList.PowerSeq[pwListIdx].PowerInfo[pwIdx].Voltage;
						break;
					}
				}
			}
		
		}
		PK_DBG("[imgsensor_poweron] config_subsensor_rst_pwd_when_nosub subrst=%d, subpwd=%d\n",g_sub_rst_output_when_sensor_disable, g_sub_pwd_output_when_sensor_disable);	

              mtkcam_gpio_set(1, CAMRST,g_sub_rst_output_when_sensor_disable);
              mtkcam_gpio_set(1, CAMPDN,g_sub_pwd_output_when_sensor_disable);


	}
}

BOOL hwpoweron(PowerInformation pwInfo, char* mode_name)
{
	if(pwInfo.PowerType == AVDD)
	{
		if(PowerCustList.PowerCustInfo[0].Gpio_Pin == GPIO_UNSUPPORTED)
		{

			{
				PK_DBG("[CAMERA SENSOR] camera VAM_A power on\n");
				if(TRUE != _hwPowerOnCnt(VCAMA, pwInfo.Voltage))
	       	 	{
         	            		PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
         	            		return FALSE;
       	        	}
              	}
		}
		else
            {
                        PK_DBG("ERR::::[CAMERA SENSOR] camera VAM_A power on us GPIO methold  not support yet!!!!\n");
		}			
	}
	else if(pwInfo.PowerType == DVDD)
	{
		if(PowerCustList.PowerCustInfo[1].Gpio_Pin == GPIO_UNSUPPORTED)
		{


			if(pinSetIdx == 1)
			{
				PK_DBG("[CAMERA SENSOR] Sub camera VCAM_D power on\n");
				if(TRUE != _hwPowerOnCnt(SUB_VCAMD, pwInfo.Voltage))
       	 		{
                        		PK_DBG("[CAMERA SENSOR] Fail to enable SUB_VCAMD power\n");
                        		return FALSE;
                		}
			}	
			else
			{
				PK_DBG("[CAMERA SENSOR] Main camera VAM_D power on\n");
				if(TRUE != _hwPowerOnCnt(VCAMD, pwInfo.Voltage))
	       	 	{
        	            		PK_DBG("[CAMERA SENSOR] Fail to enable VCAMD power\n");
        	            		return FALSE;
        	        	}
                     }
    		}
		else{
                  PK_DBG("[CAMERA LENS] set gpio failed!! \n");
			}			
	}
	else if(pwInfo.PowerType == DOVDD)
	{
		if(PowerCustList.PowerCustInfo[2].Gpio_Pin == GPIO_UNSUPPORTED)
		{
                if(TRUE != _hwPowerOnCnt(VCAMIO, pwInfo.Voltage))
       	 	{
                		PK_DBG("[CAMERA SENSOR] Fail to enable VCAMIO power\n");
                		return FALSE;
                	}
		}
				
	}
#ifndef CONFIG_HCT_AF_POWER_ON_NOISE
	else if(pwInfo.PowerType == AFVDD)
	{
		if(PowerCustList.PowerCustInfo[3].Gpio_Pin == GPIO_UNSUPPORTED)
		{
                    if(TRUE != _hwPowerOnCnt(VCAMAF, pwInfo.Voltage))
       	 	{
            		PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
            		return FALSE;
        	      }
		}
		else{
                    printk("Error: %s,%d, need check\n",__FUNCTION__,__LINE__);
		}			
	}
#endif
	else if(pwInfo.PowerType==PDN)
	{
		PK_DBG("%s hwPowerOn: PDN %d \n",pinSetIdx?"SubCam-":"MainCam", pwInfo.Voltage);

             mtkcam_gpio_set(pinSetIdx, CAMPDN ,pwInfo.Voltage);

	}
	else if(pwInfo.PowerType==RST)
	{
		PK_DBG("%s hwPowerOn: RST %d \n",pinSetIdx?"SubCam-":"MainCam",pwInfo.Voltage);

             mtkcam_gpio_set(pinSetIdx, CAMRST,pwInfo.Voltage);
		
	}
	else if(pwInfo.PowerType==SensorMCLK)
	{
		if(pinSetIdx==0)
		{
			PK_DBG("Sensor MCLK1 On");
			ISP_MCLK1_EN(TRUE);
		}
		else if(pinSetIdx==1)
		{
			PK_DBG("Sensor MCLK1 On");
			ISP_MCLK2_EN(TRUE);
		}
	}
	else{}
	if(pwInfo.Delay>0)
		mdelay(pwInfo.Delay);
	return TRUE;
}
	


BOOL hwpowerdown(PowerInformation pwInfo, char* mode_name)
{
	if(pwInfo.PowerType == AVDD)
	{
		if(PowerCustList.PowerCustInfo[0].Gpio_Pin == GPIO_UNSUPPORTED)
		{

                        if(TRUE != _hwPowerDownCnt(VCAMA))
               	 	{
                    		PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
                    		return FALSE;
                    	}
		}
			
	}
	else if(pwInfo.PowerType == DVDD)
	{
    		if(PowerCustList.PowerCustInfo[1].Gpio_Pin == GPIO_UNSUPPORTED)
    		{


                    if(pinSetIdx == 1)
                    {
                        PK_DBG("[CAMERA SENSOR] Sub camera VCAM_D power on\n");
                        if(TRUE != _hwPowerDownCnt(SUB_VCAMD))
                        {
                                PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
                                return FALSE;
                        }

                    }   
                    else
                    {
                        PK_DBG("[CAMERA SENSOR] Main camera VAM_D power on");
                        if(TRUE != _hwPowerDownCnt(VCAMD))
                        {
                                PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
                                return FALSE;
                        }

                    
        		}
              }
				
	}
	else if(pwInfo.PowerType == DOVDD)
	{
		if(PowerCustList.PowerCustInfo[2].Gpio_Pin == GPIO_UNSUPPORTED)
		{
                if(TRUE != _hwPowerDownCnt(VCAMIO))
                {
                    PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
                    return FALSE;
                }
		}

	}
#ifndef CONFIG_HCT_AF_POWER_ON_NOISE
	else if(pwInfo.PowerType == AFVDD)
	{
		if(PowerCustList.PowerCustInfo[3].Gpio_Pin == GPIO_UNSUPPORTED)
		{
                if(TRUE != _hwPowerDownCnt(VCAMAF))
           	 	{
                		PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
                		return FALSE;
                	}
		}
			
	}
#endif
	else if(pwInfo.PowerType==PDN)
	{
		PK_DBG("hwPowerDown: PDN %d \n",pwInfo.Voltage);
            mtkcam_gpio_set(pinSetIdx, CAMPDN ,pwInfo.Voltage);
        
		msleep(1);
	}
	else if(pwInfo.PowerType==RST)
	{
		PK_DBG("hwPowerDown: RST %d \n",pwInfo.Voltage);
             mtkcam_gpio_set(pinSetIdx, CAMRST,pwInfo.Voltage);

	}
	else if(pwInfo.PowerType==SensorMCLK)
	{
		if(pinSetIdx==0)
		{
			ISP_MCLK1_EN(FALSE);
		}
		else if(pinSetIdx==1)
		{
			ISP_MCLK2_EN(FALSE);
		}
	}
	else{}
	return TRUE;
}




int kdCISModulePowerOn(CAMERA_DUAL_CAMERA_SENSOR_ENUM SensorIdx, char *currSensorName, BOOL On, char* mode_name)
{

	int pwListIdx,pwIdx;
    BOOL sensorInPowerList = KAL_FALSE;

    if (DUAL_CAMERA_MAIN_SENSOR == SensorIdx){
        pinSetIdx = 0;
    }
    else if (DUAL_CAMERA_SUB_SENSOR == SensorIdx) {
        pinSetIdx = 1;
    }
    else if (DUAL_CAMERA_MAIN_2_SENSOR == SensorIdx) {
        pinSetIdx = 2;
    }
	
    //power ON
    if (On) {
		PK_DBG("kdCISModulePowerOn -on:currSensorName=%s\n",currSensorName);
		PK_DBG("kdCISModulePowerOn -on:pinSetIdx=%d\n",pinSetIdx);
#if 0
             if(pinSetIdx==0 && (0== ))
       show_stack(NULL, NULL);
#endif

        
		config_subsensor_rst_pwd_when_nosub(currSensorName);

		for(pwListIdx=0 ; pwListIdx<16; pwListIdx++)
		{
			if(currSensorName && (PowerOnList.PowerSeq[pwListIdx].SensorName!=NULL) && (0 == strcmp(PowerOnList.PowerSeq[pwListIdx].SensorName,currSensorName)))
			{
				PK_DBG("kdCISModulePowerOn  powerOn get in--- \n");
				PK_DBG("--in--sensorIdx:%d \n",pwListIdx);

                         sensorInPowerList = KAL_TRUE;

				for(pwIdx=0;pwIdx<12;pwIdx++)
				{  
					if(PowerOnList.PowerSeq[pwListIdx].PowerInfo[pwIdx].PowerType != VDD_None)
					{
						if(hwpoweron(PowerOnList.PowerSeq[pwListIdx].PowerInfo[pwIdx],mode_name)==FALSE)
							goto _kdCISModulePowerOn_exit_;
					}					
					else
					{
						PK_DBG("pwIdx=%d \n",pwIdx);
						break;
					}
				}
				break;
			}
			else if(PowerOnList.PowerSeq[pwListIdx].SensorName == NULL)
			{	
				break;
			}
			else{}
		}

        // Temp solution: default power on/off sequence
        if(KAL_FALSE == sensorInPowerList)
        {
            PK_DBG("Default power on sequence\n");
            
            if(pinSetIdx == 0 ) {
                ISP_MCLK1_EN(1);
            }
            else if (pinSetIdx == 1) {
                ISP_MCLK2_EN(1);
            }

            //First Power Pin low and Reset Pin Low

            mtkcam_gpio_set(pinSetIdx, CAMPDN ,pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF]);
            mtkcam_gpio_set(pinSetIdx, CAMRST ,pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF]);


           
            //VCAM_IO
            if(TRUE != _hwPowerOnCnt(VCAMIO,Vol_1800))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable digital power (VCAM_IO),  \n" );
                goto _kdCISModulePowerOn_exit_;
            }

            //VCAM_A
            if(TRUE != _hwPowerOnCnt(VCAMA,Vol_1800))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable analog power (VCAM_A), \n");
                goto _kdCISModulePowerOn_exit_;
            }

            if(TRUE != _hwPowerOnCnt(VCAMD,Vol_1500))
            {
                 PK_DBG("[CAMERA SENSOR] Fail to enable digital power (VCAM_D) \n");
                 goto _kdCISModulePowerOn_exit_;
            }
#ifndef CONFIG_HCT_AF_POWER_ON_NOISE
             //AF_VCC
            if(TRUE != _hwPowerOnCnt(VCAMAF, Vol_2800))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable analog power (VCAM_AF)\n");
                goto _kdCISModulePowerOn_exit_;
            }

            mdelay(5);
#endif
            //enable active sensor

            mtkcam_gpio_set(pinSetIdx, CAMPDN,pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON]);
            mdelay(1);
            mtkcam_gpio_set(pinSetIdx, CAMRST,pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON]);

        }
        }
    else {//power OFF
	for(pwListIdx=0 ; pwListIdx<16; pwListIdx++)
		{
			if(currSensorName && (PowerOnList.PowerSeq[pwListIdx].SensorName!=NULL) && (0 == strcmp(PowerOnList.PowerSeq[pwListIdx].SensorName,currSensorName)))
			{
				PK_DBG("kdCISModulePowerOn PowerOff get in--- \n");
				PK_DBG("sensorIdx:%d \n",pwListIdx);

                        sensorInPowerList = KAL_TRUE;

				for(pwIdx=12;pwIdx>=0;pwIdx--)
				{  
					if(PowerOnList.PowerSeq[pwListIdx].PowerInfo[pwIdx].PowerType != VDD_None)
					{
						if(hwpowerdown(PowerOnList.PowerSeq[pwListIdx].PowerInfo[pwIdx],mode_name)==FALSE)
							goto _kdCISModulePowerOn_exit_;
						if(pwIdx>0)
						{
							if(PowerOnList.PowerSeq[pwListIdx].PowerInfo[pwIdx-1].Delay > 0)
								mdelay(PowerOnList.PowerSeq[pwListIdx].PowerInfo[pwIdx-1].Delay);
						}
					}					
					else
					{
						PK_DBG("pwIdx=%d \n",pwIdx);
					}
				}
			}
			else if(PowerOnList.PowerSeq[pwListIdx].SensorName == NULL)
			{	
				break;
			}
			else{}
		}

        // Temp solution: default power on/off sequence
        if(KAL_FALSE == sensorInPowerList)
        {
            PK_DBG("Default power off sequence");
            
            if(pinSetIdx == 0 ) {
                ISP_MCLK1_EN(0);
            }
            else if (pinSetIdx == 1) {
                ISP_MCLK2_EN(0);
            }

            //Set Power Pin low and Reset Pin Low

            
            mtkcam_gpio_set(pinSetIdx, CAMPDN,pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF]);
            
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
                mtkcam_gpio_set(pinSetIdx, CAMRST,pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF]);
            }
            
            if(TRUE != _hwPowerDownCnt(VCAMD))
            {
                 PK_DBG("[CAMERA SENSOR] Fail to OFF core power (VCAM_D), \n");
                 goto _kdCISModulePowerOn_exit_;
            }

            //VCAM_A
            if(TRUE != _hwPowerDownCnt(VCAMA))
			{
                PK_DBG("[CAMERA SENSOR] Fail to OFF analog power (VCAM_A),  \n");
                //return -EIO;
                goto _kdCISModulePowerOn_exit_;
            }

            //VCAM_IO
            if(TRUE != _hwPowerDownCnt(VCAMIO)){
                PK_DBG("[CAMERA SENSOR] Fail to OFF digital power (VCAM_IO) \n");
                //return -EIO;
                goto _kdCISModulePowerOn_exit_;
            }
#ifndef CONFIG_HCT_AF_POWER_ON_NOISE
            //AF_VCC
            if(TRUE != _hwPowerDownCnt(VCAMAF))
            {
                PK_DBG("[CAMERA SENSOR] Fail to OFF AF power (VCAM_AF)\n");
                //return -EIO;
                goto _kdCISModulePowerOn_exit_;
            }
#endif
        }
	config_sensor_rst_pwd(currSensorName);
    }//

	return 0;

_kdCISModulePowerOn_exit_:

    
    PK_DBG("[CAMERA SENSOR] error happened pls check ----\n");
    return -EIO;
}

#else
#error "macro defined error, in Android M ,CONFIG_MTK_LEGACY shold not defined"
#endif
EXPORT_SYMBOL(kdCISModulePowerOn);

/* !-- */
/*  */


