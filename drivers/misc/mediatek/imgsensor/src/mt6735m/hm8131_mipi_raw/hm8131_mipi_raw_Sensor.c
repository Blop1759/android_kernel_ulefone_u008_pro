/*******************************************************************************************/
	  

/*******************************************************************************************/

#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>	
#include <linux/fs.h>
#include <asm/atomic.h>
//#include <asm/system.h>
#include <linux/types.h>

#include "kd_camera_typedef.h"
#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"

#include "hm8131_mipi_raw_Sensor.h"


#ifdef CONFIG_MTK_CAM_CAL
extern int read_hm8131_eeprom_mtk_fmt(void);
#endif


/****************************Modify following Strings for debug****************************/
#define PFX "hm8131_camera_sensor"
//#define LOG_1 LOG_INF("hm8131,MIPI 2LANE\n")
//#define LOG_2 LOG_INF("preview 1280*960@30fps,864Mbps/lane; video 1280*960@30fps,864Mbps/lane; capture 5M@30fps,864Mbps/lane\n")

#define LOG_INF(format, args...)	pr_debug(PFX "[%s] " format, __FUNCTION__, ##args)

static DEFINE_SPINLOCK(imgsensor_drv_lock);

/* FIXME: old factors and DIDNOT use now. s*/
static imgsensor_info_struct imgsensor_info = {
	.sensor_id = HM8131MIPI_SENSOR_ID,		//record sensor id defined in Kd_imgsensor.h

	.checksum_value = 0xd6d43c1f,		//checksum value for Camera Auto Test
/* FIXME: old factors and DIDNOT use now. e*/

	.pre = {
		.pclk = 34000000,				//record different mode's pclk
		.linelength = 0x378,				//record different mode's linelength
		.framelength = 0x51a,			//record different mode's framelength
		.startx = 2,					//record different mode's startx of grabwindow
		.starty = 2,					//record different mode's starty of grabwindow
		.grabwindow_width = 1632-16,		//record different mode's width of grabwindow
		.grabwindow_height = 1224-12,		//record different mode's height of grabwindow
		.mipi_data_lp2hs_settle_dc = 14,//unit , ns
		.max_framerate = 300,
	},
	.cap = {
		.pclk = 34000000,
		.linelength = 0x378,
		.framelength = 0x9e6,
		.startx = 4,
		.starty = 4,
		.grabwindow_width = 3264-32,
		.grabwindow_height = 2448-24,
		.mipi_data_lp2hs_settle_dc = 14,//unit , ns
		.max_framerate = 150,
	},
	.cap1 = {							//capture for PIP 24fps relative information, capture1 mode must use same framelength, linelength with Capture mode for shutter calculate
		.pclk = 34000000,
		.linelength = 0x378,
		.framelength = 0x9e6,
		.startx = 4,
		.starty = 4,
		.grabwindow_width = 3264-32,
		.grabwindow_height = 2448-24,
		.mipi_data_lp2hs_settle_dc = 14,//unit , ns
		.max_framerate = 150,
	},
	.normal_video = {
		.pclk = 34000000,
		.linelength = 0x378,
		.framelength = 0x9e6,
		.startx = 4,
		.starty = 4,
		.grabwindow_width  = 3264-32,
		.grabwindow_height = 2448-24,
		.mipi_data_lp2hs_settle_dc = 14,
		.max_framerate = 150,
	},
	.hs_video = {
		.pclk = 34000000,
		.linelength = 0x378,
		.framelength = 0x9e6,
		.startx = 4,
		.starty = 4,
		.grabwindow_width = 3264,
		.grabwindow_height = 2448,
		.mipi_data_lp2hs_settle_dc = 14,
		.max_framerate = 300,
	},
	.slim_video = {
		.pclk = 34000000,
		.linelength = 0x378,
		.framelength = 0x9e6,
		.startx = 4,
		.starty = 4,
		.grabwindow_width = 1280,
		.grabwindow_height = 720,
		.mipi_data_lp2hs_settle_dc = 14,
		.max_framerate = 300,	

	},
	.custom1 = {
		.pclk = 34000000,				//record different mode's pclk
		.linelength = 0x378,				//record different mode's linelength
		.framelength = 0x51a,			//record different mode's framelength
		.startx = 2,					//record different mode's startx of grabwindow
		.starty = 2,					//record different mode's starty of grabwindow
		.grabwindow_width = 1632,		//record different mode's width of grabwindow
		.grabwindow_height = 1224,		//record different mode's height of grabwindow
		.mipi_data_lp2hs_settle_dc = 85,//unit , ns
		.max_framerate = 300,
	},
	.custom2 = {
		.pclk = 34000000,				//record different mode's pclk
		.linelength = 0x378,				//record different mode's linelength
		.framelength = 0x51a,			//record different mode's framelength
		.startx = 2,					//record different mode's startx of grabwindow
		.starty = 2,					//record different mode's starty of grabwindow
		.grabwindow_width = 1632,		//record different mode's width of grabwindow
		.grabwindow_height = 1224,		//record different mode's height of grabwindow
		.mipi_data_lp2hs_settle_dc = 85,//unit , ns
		.max_framerate = 300,
	},
	.custom3 = {
		.pclk = 34000000,				//record different mode's pclk
		.linelength = 0x378,				//record different mode's linelength
		.framelength = 0x51a,			//record different mode's framelength
		.startx = 2,					//record different mode's startx of grabwindow
		.starty = 2,					//record different mode's starty of grabwindow
		.grabwindow_width = 1632,		//record different mode's width of grabwindow
		.grabwindow_height = 1224,		//record different mode's height of grabwindow
		.mipi_data_lp2hs_settle_dc = 85,//unit , ns
		.max_framerate = 300,
	},
	.custom4 = {
		.pclk = 34000000,				//record different mode's pclk
		.linelength = 0x378,				//record different mode's linelength
		.framelength = 0x51a,			//record different mode's framelength
		.startx = 2,					//record different mode's startx of grabwindow
		.starty = 2,					//record different mode's starty of grabwindow
		.grabwindow_width = 1632,		//record different mode's width of grabwindow
		.grabwindow_height = 1224,		//record different mode's height of grabwindow
		.mipi_data_lp2hs_settle_dc = 85,//unit , ns
		.max_framerate = 300,
	},
	.custom5 = {
		.pclk = 34000000,				//record different mode's pclk
		.linelength = 0x378,				//record different mode's linelength
		.framelength = 0x51a,			//record different mode's framelength
		.startx = 2,					//record different mode's startx of grabwindow
		.starty = 2,					//record different mode's starty of grabwindow
		.grabwindow_width = 1632,		//record different mode's width of grabwindow
		.grabwindow_height = 1224,		//record different mode's height of grabwindow
		.mipi_data_lp2hs_settle_dc = 85,//unit , ns
		.max_framerate = 300,
	},
	.margin = 10,			//sensor framelength & shutter margin
	.min_shutter = 4,		//min shutter
	.max_frame_length = 0xffff,//max framelength by sensor register's limitation
	.ae_shut_delay_frame = 0,	//shutter delay frame for AE cycle, 2 frame with ispGain_delay-shut_delay=2-0=2
	.ae_sensor_gain_delay_frame = 0,//sensor gain delay frame for AE cycle,2 frame with ispGain_delay-sensor_gain_delay=2-0=2
	.ae_ispGain_delay_frame = 2,//isp gain delay frame for AE cycle
	.ihdr_support = 0,	  //1, support; 0,not support
	.ihdr_le_firstline = 0,  //1,le first ; 0, se first
	.sensor_mode_num = 3,	  //support sensor mode num
	.cap_delay_frame = 6,		//enter capture delay frame num
	.pre_delay_frame = 3, 		//enter preview delay frame num
	.video_delay_frame = 3,		//enter video delay frame num
	.hs_video_delay_frame = 3,	//enter high speed video  delay frame num
	.slim_video_delay_frame = 3,//enter slim video delay frame num
	.custom1_delay_frame = 3,
	.custom2_delay_frame = 3,
	.custom3_delay_frame = 3,
	.custom4_delay_frame = 3,
	.custom5_delay_frame = 3,
	.isp_driving_current = ISP_DRIVING_8MA, //mclk driving current
	.sensor_interface_type = SENSOR_INTERFACE_TYPE_MIPI,//sensor_interface_type
	.mipi_sensor_type = MIPI_OPHY_NCSI2, //0,MIPI_OPHY_NCSI2;  1,MIPI_OPHY_CSI2
	.mipi_settle_delay_mode = 1,//0,MIPI_SETTLEDELAY_AUTO; 1,MIPI_SETTLEDELAY_MANNUAL
	.sensor_output_dataformat = SENSOR_OUTPUT_FORMAT_RAW_B,//sensor output first pixel color
	.mclk = 24,//mclk value, suggest 24 or 26 for 24Mhz or 26Mhz
	.mipi_lane_num = SENSOR_MIPI_2_LANE,//mipi lane num
	.i2c_addr_table = {0x48, 0xff},//record sensor support all write id addr, only supprt 4must end with 0xff
};
static imgsensor_struct imgsensor = {
	.mirror = IMAGE_NORMAL,				//mirrorflip information
	.sensor_mode = 00,			//current shutter
	.gain = 0x40,						//current gain
	.dummy_pixel = 0,					//current dummypixel
	.dummy_line = 0,					//current dummyline
	.current_fps = 300,  //full size current fps : 24fps for PIP, 30fps for Normal or ZSD
	.autoflicker_en = KAL_FALSE,  //auto flicker enable: KAL_FALSE for disable auto flicker, KAL_TRUE for enable auto flicker
	.test_pattern = KAL_FALSE,		//test pattern mode or not. KAL_FALSE for in test pattern mode, KAL_TRUE for normal output
	.current_scenario_id = MSDK_SCENARIO_ID_CAMERA_PREVIEW,//current scenario id
	.ihdr_en = 0, //sensor need support LE, SE with HDR feature
	.i2c_write_id = 0x48,//record current sensor's i2c write id
};


/* Sensor output window information */
static SENSOR_WINSIZE_INFO_STRUCT imgsensor_winsize_info[10] =
{{ 3280, 2464,	8, 	8, 3271, 2455, 1632, 1244, 0, 0, 1632, 1224,		2,	2, 1632-16,  1224-12}, // Preview
 { 3280, 2464,	8,	8, 3271, 2455, 3264, 2448, 0, 0, 3264, 2464,	4,	4, 3264, 2448-24}, // capture
 { 3280, 2464,	8,	8, 3271, 2455, 1632, 1244, 0, 0, 1632, 1224,	2,	2, 1632-16, 1224-12},  // video
 { 3280, 2464,	8,	8, 3271, 2455, 1632, 1244, 0, 0, 1632, 1224,	2,	2, 1632-16, 1224-12}, //hight speed video
 { 3280, 2464,	8,  8, 3271, 2455, 1640,  926, 0, 0, 1640,  926,	0,	0, 1280,  720},// slim video
 { 3280, 2464,	8,	8, 3271, 2455, 1632, 1224, 0, 0, 1632, 1224,	2,	2, 1632,  1224},
 { 3280, 2464,	8,	8, 3271, 2455, 1632, 1224, 0, 0, 1632, 1224,	2,	2, 1632,  1224},
 { 3280, 2464,	8,	8, 3271, 2455, 1632, 1224, 0, 0, 1632, 1224,	2,	2, 1632,  1224},
 { 3280, 2464,	8,	8, 3271, 2455, 1632, 1224, 0, 0, 1632, 1224,	2,	2, 1632,  1224},
 { 3280, 2464,	8,	8, 3271, 2455, 1632, 1224, 0, 0, 1632, 1224,	2,	2, 1632,  1224}};

#define HM8131MIPI_MaxGainIndex (64)
kal_uint32 HM8131MIPI_sensorGainMapping[HM8131MIPI_MaxGainIndex][2] ={
    {64 ,0x00},
    {68 ,0x01},
    {72 ,0x02},
    {76 ,0x03},
    {80 ,0x04},
    {84 ,0x05},
    {88 ,0x06},
    {92 ,0x07},
    {96 ,0x08},
    {100,0x09},
    {104,0x0A},
    {108,0x0B},
    {112,0x0C},
    {116,0x0D},
    {120,0x0E},
    {124,0x0F},
    {128,0x10},
    {136,0x12},
    {144,0x14},
    {152,0x16},
    {160,0x18},
    {168,0x1A},
    {176,0x1C},
    {184,0x1E},
    {192,0x20},
    {200,0x22},
    {208,0x24},
    {216,0x26},
    {224,0x28},
    {232,0x2A},
    {240,0x2C},
    {248,0x2E},
    {256,0x30},
    {272,0x34},
    {288,0x38},
    {304,0x3C},
    {320,0x40},
    {336,0x44},
    {352,0x48},
    {368,0x4C},
    {384,0x50},
    {400,0x54},
    {416,0x58},
    {432,0x5C},
    {448,0x60},
    {464,0x64},
    {480,0x68},
    {496,0x6C},
    {512,0x70},
    {544,0x78},
    {576,0x80},
    {608,0x88},
    {640,0x90},
    {672,0x98},
    {704,0xA0},
    {736,0xA8},
    {768,0xB0},
    {800,0xB8},
    {832,0xC0},
    {864,0xC8},
    {896,0xD0},
    {928,0xD8},
    {960,0xE0},
    {992,0xE8},
};
static kal_uint16 read_cmos_sensor(kal_uint32 addr)
{
	kal_uint16 get_byte=0;
	char pu_send_cmd[2] = {(char)(addr >> 8), (char)(addr & 0xFF) };
        kdSetI2CSpeed(imgsensor_info.i2c_speed);
	iReadRegI2C(pu_send_cmd, 2, (u8*)&get_byte, 1, imgsensor.i2c_write_id);
	return get_byte;
}
static void write_cmos_sensor(kal_uint32 addr, kal_uint32 para)
{
	char pu_send_cmd[3] = {(char)(addr >> 8), (char)(addr & 0xFF), (char)(para & 0xFF)};
	
	iWriteRegI2C(pu_send_cmd, 3, imgsensor.i2c_write_id);
}

static void set_dummy(void)

{
	LOG_INF("dummyline = %d, dummypixels = %d \n", imgsensor.dummy_line, imgsensor.dummy_pixel);

	/*write_cmos_sensor( 0x0340, (imgsensor.frame_length >>8) & 0xFF);

    write_cmos_sensor(0x0341, imgsensor.frame_length & 0xFF);

*/
}	/*	set_dummy  */

static kal_uint32 return_sensor_id(void)
////////////////////////////////////////////////////////////////////////////////////////////////////
{
	return ((read_cmos_sensor(0x0000) << 8) | read_cmos_sensor(0x0001));
	//int sensorid;
	
	//LOG_INF( ">>> HM8131MIPI_Set_Dummy(): iPixels=%d,iLines=%d\n",iPixels ,iLines);

}
static void set_max_framerate(UINT16 framerate,kal_bool min_framelength_en)
{
	kal_uint32 frame_length = imgsensor.frame_length;
	// Set total line length

	LOG_INF("framerate = %d, min framelength should enable = %d\n", framerate,min_framelength_en);

	frame_length = imgsensor.pclk / framerate * 10 / imgsensor.line_length;
	spin_lock(&imgsensor_drv_lock);
	imgsensor.frame_length = (frame_length > imgsensor.min_frame_length) ? frame_length : imgsensor.min_frame_length;
	imgsensor.dummy_line = imgsensor.frame_length - imgsensor.min_frame_length;
	if (imgsensor.frame_length > imgsensor_info.max_frame_length)
	{
		imgsensor.frame_length = imgsensor_info.max_frame_length;
		imgsensor.dummy_line = imgsensor.frame_length - imgsensor.min_frame_length;
	}
	if (min_framelength_en)
		imgsensor.min_frame_length = imgsensor.frame_length;
	spin_unlock(&imgsensor_drv_lock);
	set_dummy();
}	/*	set_max_framerate  */


	// TODO:
		
	


#if 0
static void write_shutter(kal_uint16 shutter)
{
	kal_uint16 realtime_fps = 0;
	kal_uint32 frame_length = 0;
	
	//This is Special case for HM8131SensorDriver, It Need Write FrameLength when shutter >FrameHeight
	//In here,Don't Increase this avariable HM8131SensorDriver.FrameHeight
	spin_lock(&imgsensor_drv_lock);
	if (shutter > imgsensor.min_frame_length - imgsensor_info.margin)
		imgsensor.frame_length = shutter + imgsensor_info.margin;
	else
		imgsensor.frame_length = imgsensor.min_frame_length;
	if (imgsensor.frame_length > imgsensor_info.max_frame_length)
		imgsensor.frame_length = imgsensor_info.max_frame_length;
	spin_unlock(&imgsensor_drv_lock);
	shutter = (shutter < imgsensor_info.min_shutter) ? imgsensor_info.min_shutter : shutter;
	shutter = (shutter > (imgsensor_info.max_frame_length - imgsensor_info.margin)) ? (imgsensor_info.max_frame_length - imgsensor_info.margin) : shutter;
    shutter = (shutter >> 1) << 1;
    imgsensor.frame_length = (imgsensor.frame_length >> 1) << 1;
	if (imgsensor.autoflicker_en) {
		realtime_fps = imgsensor.pclk / imgsensor.line_length * 10 / imgsensor.frame_length;
		if(realtime_fps >= 297 && realtime_fps <= 305)
			set_max_framerate(296,0);
		else if(realtime_fps >= 147 && realtime_fps <= 150)
			set_max_framerate(146,0);
		else {
		write_cmos_sensor(0x0340, imgsensor.frame_length >> 8);
		write_cmos_sensor(0x0341, imgsensor.frame_length & 0xFF);
		}
	} else {
		write_cmos_sensor(0x0340, imgsensor.frame_length >> 8);
		write_cmos_sensor(0x0341, imgsensor.frame_length & 0xFF);
	}


	// Update Shutter
	
	// Set Shutter. (Coarse integration time, uint: lines.)
	write_cmos_sensor(0x0202, (shutter >> 8) & 0xFF);
	write_cmos_sensor(0x0203, (shutter ) & 0xFF);

	// TODO:
	write_cmos_sensor(0x0104, 0x00);	
	LOG_INF("Exit! shutter =%d, framelength =%d\n", shutter,imgsensor.frame_length);
	
	//LOG_INF("frame_length = %d ", frame_length);
}	/*	write_shutter  */
#endif

/*************************************************************************
* FUNCTION
*   HM8131MIPI_SetShutter
*
* DESCRIPTION
*   This function set e-Shutter of HM8131MIPI to change exposure time.
*
* PARAMETERS
*   iShutter : exposured lines
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static void set_shutter(kal_uint16 shutter)
{
	unsigned long flags;
	kal_uint16 realtime_fps = 0;
	spin_lock_irqsave(&imgsensor_drv_lock, flags);
	imgsensor.shutter = shutter;
	spin_unlock_irqrestore(&imgsensor_drv_lock, flags);
	
	//if( HM8131SensorDriver.Shutter == iShutter )
	//	return;
	
	spin_lock(&imgsensor_drv_lock);
	if (shutter > imgsensor.min_frame_length - imgsensor_info.margin)
		imgsensor.frame_length = shutter + imgsensor_info.margin;
	else
		imgsensor.frame_length = imgsensor.min_frame_length;
	if (imgsensor.frame_length > imgsensor_info.max_frame_length)
		imgsensor.frame_length = imgsensor_info.max_frame_length;
	spin_unlock(&imgsensor_drv_lock);
	shutter = (shutter < imgsensor_info.min_shutter) ? imgsensor_info.min_shutter : shutter;
	shutter = (shutter > (imgsensor_info.max_frame_length - imgsensor_info.margin)) ? (imgsensor_info.max_frame_length - imgsensor_info.margin) : shutter;
	
	if (imgsensor.autoflicker_en) {
		realtime_fps = imgsensor.pclk / imgsensor.line_length * 10 / imgsensor.frame_length;
		if(realtime_fps >= 297 && realtime_fps <= 305)
			set_max_framerate(296,0);
		else if(realtime_fps >= 147 && realtime_fps <= 150)
			set_max_framerate(146,0);
		else {
		// Extend frame length
		write_cmos_sensor(0x0340, imgsensor.frame_length >> 8);
		write_cmos_sensor(0x0341, imgsensor.frame_length & 0xFF);
}
	} else {
		// Extend frame length
		write_cmos_sensor(0x0340, imgsensor.frame_length >> 8);
		write_cmos_sensor(0x0341, imgsensor.frame_length & 0xFF);
	}

	// Update Shutter
	write_cmos_sensor(0x0202, (shutter >> 8) & 0xFF);
	write_cmos_sensor(0x0203, (shutter ) & 0xFF);
	write_cmos_sensor(0x0104, 0x00);		
	LOG_INF("Exit! shutter =%d, framelength =%d\n", shutter,imgsensor.frame_length);

	
}

/*******************************************************************************
*
********************************************************************************/
static kal_uint16 gain2reg(const kal_uint16 gain)
{
	kal_uint8 iI;
    LOG_INF("[hm8131MIPI]enter hm8131MIPIGain2Reg function\n");
    for (iI = 0; iI < (HM8131MIPI_MaxGainIndex-1); iI++)
	// Range: 1x to 32x
	// Gain = (GAIN[7] + 1) * (GAIN[6] + 1) * (GAIN[5] + 1) * (GAIN[4] + 1) * (1 + GAIN[3:0] / 16)

/*******************************************************************************
*
********************************************************************************/
{

        if(gain <HM8131MIPI_sensorGainMapping[iI][0])
		{
            break;
        }
		if(gain < HM8131MIPI_sensorGainMapping[iI][0])
		{
			return HM8131MIPI_sensorGainMapping[iI][1];
		}


    }
    if(gain != HM8131MIPI_sensorGainMapping[iI][0])



    {
         LOG_INF("Gain mapping don't correctly:%d %d \n", gain, HM8131MIPI_sensorGainMapping[iI][0]);
    }
	LOG_INF("exit hm8131MIPIGain2Reg function\n");
    return HM8131MIPI_sensorGainMapping[iI-1][1];

}

/*************************************************************************
* FUNCTION
*	HM8131MIPI_SetGain
*
* DESCRIPTION
*	This function is to set global gain to sensor.
*
* PARAMETERS
*	gain : sensor global gain(base: 0x40)
*
* RETURNS
*	the actually gain set to sensor.
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT16 iPreGain = 0;
static kal_uint16 set_gain(kal_uint16 gain)
{
	kal_uint16 reg_gain;
 if (gain < BASEGAIN || gain > 32 * BASEGAIN) {
        LOG_INF("Error gain setting");

        if (gain < BASEGAIN)
            gain = BASEGAIN;
        else if (gain > 32 * BASEGAIN)
            gain = 32 * BASEGAIN;
    }
		reg_gain = gain2reg(gain);
		spin_lock(&imgsensor_drv_lock);
		imgsensor.gain = reg_gain;
	//HM8131SensorDriver.Gain2Reg = HM8131MIPIGain2Reg( iGain );
		spin_unlock(&imgsensor_drv_lock);
		LOG_INF("gain = %d , reg_gain = 0x%x\n ", gain, reg_gain);
   // reg_gain=0x70;  //fix gain 2016

    write_cmos_sensor(0x0205, reg_gain);
    write_cmos_sensor(0x0104, 0x00);
		LOG_INF("gain = %d , reg_gain = 0x%x\n ", gain, reg_gain);

	return gain;
}

static void ihdr_write_shutter_gain(kal_uint16 le, kal_uint16 se, kal_uint16 gain)
{
	LOG_INF("le:0x%x, se:0x%x, gain:0x%x\n",le,se,gain);
	#if 0
	if (imgsensor.ihdr_en) {
	
	
		spin_lock(&imgsensor_drv_lock);
			if (le > imgsensor.min_frame_length - imgsensor_info.margin)
				imgsensor.frame_length = le + imgsensor_info.margin;
			else
				imgsensor.frame_length = imgsensor.min_frame_length;
			if (imgsensor.frame_length > imgsensor_info.max_frame_length)
				imgsensor.frame_length = imgsensor_info.max_frame_length;
			spin_unlock(&imgsensor_drv_lock);
			if (le < imgsensor_info.min_shutter) le = imgsensor_info.min_shutter;
			if (se < imgsensor_info.min_shutter) se = imgsensor_info.min_shutter;
	

				write_cmos_sensor(0x380e, imgsensor.frame_length >> 8);
				write_cmos_sensor(0x380f, imgsensor.frame_length & 0xFF);
		write_cmos_sensor(0x3502, (le << 4) & 0xFF);
		write_cmos_sensor(0x3501, (le >> 4) & 0xFF);
		write_cmos_sensor(0x3500, (le >> 12) & 0x0F);
		write_cmos_sensor(0x3508, (se << 4) & 0xFF);
		write_cmos_sensor(0x3507, (se >> 4) & 0xFF);
		write_cmos_sensor(0x3506, (se >> 12) & 0x0F);
		set_gain(gain);
	}
	#endif
}

#if 0
static void set_mirror_flip(kal_uint8 image_mirror)
{
	LOG_INF("image_mirror = %d\n", image_mirror);
//	
//	for( i = 0; 0xFFFFFFFF != HM8131SensorReg[i].Addr; i++ )
//	{
//		HM8131MIPI_write_cmos_sensor( HM8131SensorReg[i].Addr, HM8131SensorReg[i].Para );
//	}
//	for( i = HM8131MIPI_ENGINEER_START_ADDR; 0xFFFFFFFF != HM8131SensorReg[i].Addr; i++ )
//	{
//		HM8131MIPI_write_cmos_sensor( HM8131SensorReg[i].Addr, HM8131SensorReg[i].Para );
//	}
//	for( i = HM8131MIPI_FACTORY_START_ADDR; i < HM8131MIPI_FACTORY_END_ADDR; i++ )
//	{
//		HM8131MIPI_write_cmos_sensor( HM8131SensorCCT[i].Addr, HM8131SensorCCT[i].Para );
//	}

/*************************************************************************
* FUNCTION
*	HM8131MIPI_sensor_to_camera_para
*
* DESCRIPTION
*	// update camera_para from sensor register
*
* PARAMETERS
*	None
*
* RETURNS
*	gain : sensor global gain(base: 0x40)
*
* GLOBALS AFFECTED
*
*************************************************************************/
	kal_uint8  iTemp;
	LOG_INF("set_mirror_flip function\n");
    iTemp = read_cmos_sensor(0x0172) & 0x03;	//Clear the mirror and flip bits.
    switch (image_mirror)
{
        case IMAGE_NORMAL:
            break;
        case IMAGE_V_MIRROR:
//	{
            break;
        case IMAGE_H_MIRROR:
//		HM8131SensorReg[i].Para = temp_data;
            break;
        case IMAGE_HV_MIRROR:
            break;
    }
	LOG_INF("Error image_mirror setting\n");
//	{
//		temp_data = HM8131MIPI_read_cmos_sensor( HM8131SensorReg[i].Addr );
//		spin_lock( &HM8131mipi_drv_lock );
//		HM8131SensorReg[i].Para = temp_data;
//		spin_unlock( &HM8131mipi_drv_lock );
//	}
}
#endif
/*************************************************************************
* FUNCTION
*	HM8131MIPI_get_sensor_group_count
*
* DESCRIPTION
*	//
*
* PARAMETERS
*	None
*
* RETURNS
*	gain : sensor global gain(base: 0x40)
*
* GLOBALS AFFECTED
*
*************************************************************************/
static void night_mode(kal_bool enable)
{
/*No Need to implement this function*/
}

static void sensor_init(void)
{
	LOG_INF("E\n");
	//write_cmos_sensor(0x0100,0x01);  //wake up
	}


static void preview_setting(void)
{

	
	 mdelay(30);//START


write_cmos_sensor(0x3519,0x00);
write_cmos_sensor(0x351A,0x05);
write_cmos_sensor(0x351B,0x1E);
write_cmos_sensor(0x351C,0x90);
write_cmos_sensor(0x351E,0x15);
write_cmos_sensor(0x351D,0x15);
write_cmos_sensor(0x4001,0x80);
	 mdelay(10);

write_cmos_sensor(0xBA93,0x01);
 mdelay(10);


write_cmos_sensor(0x412A,0x8A);
write_cmos_sensor(0xBAA2,0xC0);
write_cmos_sensor(0xBAA2,0xC0);
write_cmos_sensor(0xBAA2,0xC0);
write_cmos_sensor(0xBAA2,0x40);
write_cmos_sensor(0x412A,0x8A);
write_cmos_sensor(0x412A,0x8A);
write_cmos_sensor(0x412A,0x0A);

 mdelay(10);

write_cmos_sensor(0xBA93,0x03);
write_cmos_sensor(0xBA93,0x02);
write_cmos_sensor(0xBA90,0x01);
write_cmos_sensor(0x4001,0x00);

 mdelay(10);


write_cmos_sensor(0x0303,0x02); //1028, for 2-lane

write_cmos_sensor(0x0305,0x0C); //2 1 ; PLL N, mclk 24mhz
write_cmos_sensor(0x0309,0x02); //1028, for 2-lane

write_cmos_sensor(0x0307,0x44); //1028, for 2-lane

write_cmos_sensor(0x030D, 0x0C);
write_cmos_sensor(0x030F,0x62); //1028, for 2-lan
 mdelay(10);
write_cmos_sensor(0x414A, 0x02);
write_cmos_sensor(0x4147, 0x03);
write_cmos_sensor(0x4144, 0x03);
write_cmos_sensor(0x4145, 0x31);
write_cmos_sensor(0x4146, 0x51);
write_cmos_sensor(0x4149, 0x57);
write_cmos_sensor(0x4260, 0x00);
write_cmos_sensor(0x4261, 0x00);
write_cmos_sensor(0x426A, 0x01);
write_cmos_sensor(0x4270, 0x08);
write_cmos_sensor(0x4271, 0xBF);
write_cmos_sensor(0x4272, 0x00);
write_cmos_sensor(0x4383, 0x98);
write_cmos_sensor(0x4387, 0x17);
write_cmos_sensor(0x4386, 0x32);
write_cmos_sensor(0x438A, 0x00);

write_cmos_sensor(0x427D, 0x00);
write_cmos_sensor(0x427E, 0x03);
write_cmos_sensor(0x427F, 0x00);
write_cmos_sensor(0x4380, 0xA6);
write_cmos_sensor(0x4381, 0x7B);
write_cmos_sensor(0x4382, 0x00);
write_cmos_sensor(0x4388, 0x9F);
write_cmos_sensor(0x4389, 0x15);

write_cmos_sensor(0x438C, 0x0F);
write_cmos_sensor(0x4384, 0x14);
write_cmos_sensor(0x438B, 0x00);
write_cmos_sensor(0x4385, 0xA5);
write_cmos_sensor(0x438F, 0x00);
write_cmos_sensor(0x438D, 0xA0);
write_cmos_sensor(0x4B11, 0x1F);
write_cmos_sensor(0x4B44, 0x00);//mipi enable phy to LDO: 1.5=>1.2
write_cmos_sensor(0x4B46, 0x03);
write_cmos_sensor(0x4B47, 0xC9);
write_cmos_sensor(0x44B0, 0x03);
write_cmos_sensor(0x44B1, 0x01);
write_cmos_sensor(0x44B2, 0x00);
write_cmos_sensor(0x44B3, 0x04);
write_cmos_sensor(0x44B4, 0x14);
write_cmos_sensor(0x44B5, 0x24);
write_cmos_sensor(0x44B8, 0x03);
write_cmos_sensor(0x44B9, 0x01);
write_cmos_sensor(0x44BA, 0x05);
write_cmos_sensor(0x44BB, 0x15);
write_cmos_sensor(0x44BC, 0x25);
write_cmos_sensor(0x44BD, 0x35);

write_cmos_sensor(0x44D0, 0xC0);
write_cmos_sensor(0x44D1, 0x80);
write_cmos_sensor(0x44D2, 0x40);
write_cmos_sensor(0x44D3, 0x40);
write_cmos_sensor(0x44D4, 0x40);
write_cmos_sensor(0x44D5, 0x40);
write_cmos_sensor(0x4B07, 0xF0);
write_cmos_sensor(0x4131, 0x01);
write_cmos_sensor(0x060B, 0x01);
 mdelay(10);
write_cmos_sensor(0x4274, 0x33);
write_cmos_sensor(0x400D, 0x04);
write_cmos_sensor(0x3110, 0x03);
write_cmos_sensor(0x3111, 0x01);
write_cmos_sensor(0x3130, 0x01);
write_cmos_sensor(0x3131, 0x26);
write_cmos_sensor(0x0383, 0x03);
write_cmos_sensor(0x0387, 0x03);
write_cmos_sensor(0x0390, 0x11);
 mdelay(30);
write_cmos_sensor(0x0348,0x0C);
write_cmos_sensor(0x0349,0xCD);//3277
write_cmos_sensor(0x034A,0x09);
write_cmos_sensor(0x034B,0x9D);//2461
write_cmos_sensor(0x0340,0x05);
write_cmos_sensor(0x0341,0x1A);
write_cmos_sensor(0x4B31,0x06); //NEW!! 
write_cmos_sensor(0x034C,0x06);
write_cmos_sensor(0x034D,0x68);//1632
write_cmos_sensor(0x034E,0x04);
write_cmos_sensor(0x034F,0xD0);//1224
write_cmos_sensor(0x4B18,0x18); //1028, for 2-lane
write_cmos_sensor(0x0111,0x00); //2 1 ; B5: 1'b0:2lane, 1'b1:4lane (org 4B19)
write_cmos_sensor(0x4B20,0xDE); //2 1 ; clock always on(9E) / clock always on while sending packet(BE)
write_cmos_sensor(0x4B0E,0x01);
write_cmos_sensor(0x4B42,0x02);
write_cmos_sensor(0x4B04,0x02); //2 1 ; hs_trail_width
write_cmos_sensor(0x4B06,0x03); //2 1 ; clk_trail_width
write_cmos_sensor(0x4B31,0x06); //2 1 ; clk_trail_width
write_cmos_sensor(0x4B3F,0x10); //1028, for 2-lane
write_cmos_sensor(0x4B42,0x02); //2 1 ; HS_PREPARE_WIDTH
write_cmos_sensor(0x4B43,0x02); //2 1 ; CLK_PREPARE_WIDTH
write_cmos_sensor(0x4024,0x40); //2 1 ; enabled MIPI -> was 0024
write_cmos_sensor(0x0101,0x00); //flip+mirror
write_cmos_sensor(0x4800,0x00);
write_cmos_sensor(0x0104,0x01);
write_cmos_sensor(0x0104,0x00);
write_cmos_sensor(0x4801,0x00);
write_cmos_sensor(0x0000,0x00);
write_cmos_sensor(0xBA94,0x01);
write_cmos_sensor(0xBA94,0x00);
write_cmos_sensor(0xBA93,0x02);
 mdelay(10);
			}
			
			//temp_gain = (temp_para/HM8131SensorDriver.sensorBaseGain) * 1000;
			
static void capture_setting(kal_uint16 currefps)
			{
	LOG_INF("E! currefps:%d\n",currefps);
	write_cmos_sensor(0x3519,0x00);
write_cmos_sensor(0x351A,0x05);
write_cmos_sensor(0x351B,0x1E);
write_cmos_sensor(0x351C,0x90);
write_cmos_sensor(0x351E,0x15);
write_cmos_sensor(0x351D,0x15);
write_cmos_sensor(0x4001,0x80);
 mdelay(10);
write_cmos_sensor(0xBA93,0x01);
 mdelay(10);
write_cmos_sensor(0x412A,0x8A);
write_cmos_sensor(0xBAA2,0xC0);
write_cmos_sensor(0xBAA2,0xC0);
write_cmos_sensor(0xBAA2,0xC0);
write_cmos_sensor(0xBAA2,0x40);
write_cmos_sensor(0x412A,0x8A);
write_cmos_sensor(0x412A,0x8A);
write_cmos_sensor(0x412A,0x0A);
 mdelay(10);
write_cmos_sensor(0xBA93,0x03);
write_cmos_sensor(0xBA93,0x02);
write_cmos_sensor(0xBA90,0x01);
write_cmos_sensor(0x4001,0x00);
 mdelay(10);
write_cmos_sensor(0x0303,0x02); //1028, for 2-lane
write_cmos_sensor(0x0305,0x0C); //2 1 ; PLL N, mclk 24mhz
write_cmos_sensor(0x0307, 0x44); //2 1 ; PLL M, pclk_raw=68mhz
write_cmos_sensor(0x030D,0x0C); //2 1 ; PLL N,
write_cmos_sensor(0x030F,0x5E); //2 1 ; PLL M, pkt_clk=94mhz
write_cmos_sensor(0x414A, 0x02);
write_cmos_sensor(0x4147, 0x03);
write_cmos_sensor(0x4144, 0x03);
write_cmos_sensor(0x4145, 0x31);
write_cmos_sensor(0x4146, 0x51);
write_cmos_sensor(0x4149, 0x57);
write_cmos_sensor(0x4260, 0x00);
write_cmos_sensor(0x4261, 0x00);
write_cmos_sensor(0x426A, 0x01);
write_cmos_sensor(0x4270, 0x08);
write_cmos_sensor(0x4271, 0xBF);
write_cmos_sensor(0x4272, 0x00);
write_cmos_sensor(0x427D, 0x00);
write_cmos_sensor(0x427E, 0x03);
write_cmos_sensor(0x427F, 0x00);
write_cmos_sensor(0x4380, 0xA6);
write_cmos_sensor(0x4381, 0x7B);
write_cmos_sensor(0x4383, 0x98);
write_cmos_sensor(0x4387, 0x17);
write_cmos_sensor(0x4386, 0x32);
write_cmos_sensor(0x4382, 0x00);
write_cmos_sensor(0x4388, 0x9F);
write_cmos_sensor(0x4389, 0x15);
write_cmos_sensor(0x438A, 0x00);
write_cmos_sensor(0x438C, 0x0F);
write_cmos_sensor(0x4384, 0x14);
write_cmos_sensor(0x438B, 0x00);
write_cmos_sensor(0x4385, 0xA5);
write_cmos_sensor(0x438F, 0x00);
write_cmos_sensor(0x438D, 0xA0);
write_cmos_sensor(0x4B11, 0x1F);
write_cmos_sensor(0x4B44, 0x00);//mipi enable phy to LDO: 1.5=>1.2
write_cmos_sensor(0x4B46, 0x03);
write_cmos_sensor(0x4B47, 0xC9);		  
write_cmos_sensor(0x44B0, 0x03);
write_cmos_sensor(0x44B1, 0x01);
write_cmos_sensor(0x44B2, 0x00);
write_cmos_sensor(0x44B3, 0x04);
write_cmos_sensor(0x44B4, 0x14);
write_cmos_sensor(0x44B5, 0x24);		  
write_cmos_sensor(0x44B8, 0x03);
write_cmos_sensor(0x44B9, 0x01);
write_cmos_sensor(0x44BA, 0x05);
write_cmos_sensor(0x44BB, 0x15);
write_cmos_sensor(0x44BC, 0x25);
write_cmos_sensor(0x44BD, 0x35);
write_cmos_sensor(0x44D0, 0xC0);
write_cmos_sensor(0x44D1, 0x80);
write_cmos_sensor(0x44D2, 0x40);
write_cmos_sensor(0x44D3, 0x40);
write_cmos_sensor(0x44D4, 0x40);
write_cmos_sensor(0x44D5, 0x40);
write_cmos_sensor(0x4B07, 0xF0);
write_cmos_sensor(0x4131, 0x01);
write_cmos_sensor(0x060B, 0x01);
 mdelay(30);
write_cmos_sensor(0x4274,0x33);
write_cmos_sensor(0x3110,0x21);
write_cmos_sensor(0x3111,0x00);
write_cmos_sensor(0x3130,0x01);
write_cmos_sensor(0x3131,0x80);
write_cmos_sensor(0x4B31,0x06); //new
write_cmos_sensor(0x4002,0x23); //2 1 ; output BPC
write_cmos_sensor(0x034C,0x0C);
write_cmos_sensor(0x034D,0xC0);
write_cmos_sensor(0x034E,0x09);
write_cmos_sensor(0x034F,0xA0);
write_cmos_sensor(0x4B18,0x28); //1028, for 2-lane
write_cmos_sensor(0x0111,0x00); //2 1 ; B5: 1'b0:2lane, 1'b1:4lane (org 4B19)
write_cmos_sensor(0x4B20,0xDE); //2 1 ; clock always on(9E) / clock always on while sending packet(BE)
write_cmos_sensor(0x4B0E,0x03);
write_cmos_sensor(0x4B42,0x05);
write_cmos_sensor(0x4B03,0x0E);
write_cmos_sensor(0x4B04,0x05);
write_cmos_sensor(0x4B06,0x06);
write_cmos_sensor(0x4B3F,0x18); //1028, for 2-lane
write_cmos_sensor(0x4024,0x40); //2 1 ; enabled MIPI -> was 0024
write_cmos_sensor(0x0101,0x00); //flip+mirror


write_cmos_sensor(0x4800,0x00);
write_cmos_sensor(0x0104,0x01);
write_cmos_sensor(0x0104,0x00);
write_cmos_sensor(0x4801,0x00);
write_cmos_sensor(0x0000,0x00);
					
write_cmos_sensor(0xBA94,0x01);
write_cmos_sensor(0xBA94,0x00);
write_cmos_sensor(0xBA93,0x02);

 mdelay(30);
					}

static void slim_video_setting(void)
					{
	
					}
//
kal_uint8  test_pattern_flag=0;
static kal_uint32 set_test_pattern_mode(kal_bool enable)
					{
	LOG_INF("enable: %d\n", enable);
	if(imgsensor.current_scenario_id != MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG)
					{
					
		   if(enable)
			{
			   //1640 x 1232
			 
			}
		   else
			{
			  

	}
}
	   else
{
		   if(enable)
	
	{
			
			

//				temp_para = (temp_gain * HM8131SensorDriver.sensorBaseGain + BASEGAIN/2) / BASEGAIN;
			}
			else
			
			{
			
					//no need to apply this item for driving current
			}
			}

	   return ERROR_NONE;

	spin_lock(&imgsensor_drv_lock);
	imgsensor.test_pattern = enable;
	spin_unlock(&imgsensor_drv_lock);
	return ERROR_NONE;

}

/*************************************************************************
* FUNCTION
*   HM8131MIPIOpen
*
* DESCRIPTION
*   This function initialize the registers of CMOS sensor
*
* PARAMETERS
*   None
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 get_imgsensor_id(UINT32 *sensor_id)
{
	kal_uint8 retry = 3;
	//int i = 0;
	//kal_uint16 sensor_id = 0;
	

	do {
			*sensor_id = ((read_cmos_sensor(0x0000) << 8) | read_cmos_sensor(0x0001));
			LOG_INF("hm8131_get_imgsensor_id: 0x%x\n", *sensor_id);
			if (*sensor_id != imgsensor_info.sensor_id) {
				*sensor_id =0xffffffff;
				LOG_INF("read HM8131 R1A R2A bate fail\n");	  
				return ERROR_SENSOR_CONNECT_FAIL;
				}
			 else
		{
			*sensor_id=imgsensor_info.sensor_id;
		        break;
		}
			//SIOSENSOR( "Slave Addr=%x, Sensor ID=0x%x\n", HM8131SensorDriver.SlaveAddr,HM8131SensorDriver.SensorID );
	
			LOG_INF("Read sensor id ok, id: 0x%x\n", imgsensor.i2c_write_id);
			retry--;
		} while( retry > 0 );


	//Check Sensor ID Fail

	//Sensor Init Setting
	
	
	




	return ERROR_NONE;
}

/*************************************************************************
* FUNCTION
*   HM8131MIPIGetSensorID
*
* DESCRIPTION
*   This function get the sensor ID
*
* PARAMETERS
*   *sensorID : return the sensor ID
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
	kal_uint32 sensor_id;
static kal_uint32 open(void)
{
	int  retry = 1;
	
	//sensor have two i2c address 0x6c 0x6d & 0x21 0x20, we should detect the module used i2c address
	
	// check if sensor ID correct
	do {
			sensor_id = return_sensor_id();
			LOG_INF("gaochao_8131_get_imgsensor_id: 0x%x\n", sensor_id);
			if (sensor_id != imgsensor_info.sensor_id) {
				sensor_id = 0xffffffff;
				LOG_INF("read HM8131 R1A R2A bate fail\n");	  
				return ERROR_SENSOR_CONNECT_FAIL;
				}
			 else
		{
			sensor_id=imgsensor_info.sensor_id;
		        break;
		}
			
			LOG_INF("Read sensor id ok, id: 0x%x\n", imgsensor.i2c_write_id);
		//SIOSENSOR( "Slave Addr=%x, Sensor ID=0x%x\n", HM8131SensorDriver.SlaveAddr,HM8131SensorDriver.SensorID );

		retry--;
	} while( retry > 0 );

	sensor_init();

	iPreGain = 0;

	spin_lock(&imgsensor_drv_lock);

	imgsensor.autoflicker_en= KAL_FALSE;
	imgsensor.sensor_mode = IMGSENSOR_MODE_INIT;
	imgsensor.pclk = imgsensor_info.pre.pclk;
	imgsensor.frame_length = imgsensor_info.pre.framelength;
	imgsensor.line_length = imgsensor_info.pre.linelength;
	imgsensor.min_frame_length = imgsensor_info.pre.framelength;
	imgsensor.dummy_pixel = 0;
	imgsensor.dummy_line = 0;
	imgsensor.ihdr_en = 0;
	imgsensor.test_pattern = KAL_FALSE;
	imgsensor.current_fps = imgsensor_info.pre.max_framerate;
	spin_unlock(&imgsensor_drv_lock);

		//SIOSENSOR_ERR("GetSensorID Success!!!\n");	
		return ERROR_NONE;
	}



/*************************************************************************
* FUNCTION
*   HM8131MIPIClose
*
* DESCRIPTION
*   This function is to turn off sensor module power.
*
* PARAMETERS
*   None
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 close(void)
{
	LOG_INF("E\n");

	//CISModulePowerOn( FALSE );
	//DRV_I2CClose( HM8131MIPIhDrvI2C );

	return ERROR_NONE;
}

/*************************************************************************
* FUNCTION
*   HM8131MIPIPreview
*
* DESCRIPTION
*   This function start the sensor preview.
*
* PARAMETERS
*   *image_window : address pointer of pixel numbers in one period of HSYNC
*  *sensor_config_data : address pointer of line numbers in one period of VSYNC
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");
	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_PREVIEW;
	imgsensor.pclk = imgsensor_info.pre.pclk;
	imgsensor.line_length = imgsensor_info.pre.linelength;
	imgsensor.frame_length = imgsensor_info.pre.framelength;
	imgsensor.min_frame_length = imgsensor_info.pre.framelength;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	

	write_cmos_sensor(0x0100,0x00);
	    mdelay(72);
	write_cmos_sensor(0x0103,0x00);
	write_cmos_sensor(0x0100,0x02);
		mdelay(10);
	preview_setting();
	write_cmos_sensor(0x0100,0x01);
	LOG_INF("Add 0100 by Ethan in preview \n");
	mdelay(10);
	return ERROR_NONE;
	}

static kal_uint32 capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
						  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");
	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CAPTURE;
	if (imgsensor.current_fps == imgsensor_info.cap1.max_framerate) {//PIP capture: 24fps for less than 13M, 20fps for 16M,15fps for 20M
		imgsensor.pclk = imgsensor_info.cap1.pclk;
		imgsensor.line_length = imgsensor_info.cap1.linelength;
		imgsensor.frame_length = imgsensor_info.cap1.framelength;
		imgsensor.min_frame_length = imgsensor_info.cap1.framelength;
		imgsensor.autoflicker_en = KAL_FALSE;
	} else {
		if (imgsensor.current_fps != imgsensor_info.cap.max_framerate)
			LOG_INF("Warning: current_fps %d fps is not support, so use cap1's setting: %d fps!\n",imgsensor.current_fps,imgsensor_info.cap1.max_framerate/10);
		imgsensor.pclk = imgsensor_info.cap.pclk;
		imgsensor.line_length = imgsensor_info.cap.linelength;
		imgsensor.frame_length = imgsensor_info.cap.framelength;
		imgsensor.min_frame_length = imgsensor_info.cap.framelength;
		imgsensor.autoflicker_en = KAL_FALSE;
	}

	spin_unlock(&imgsensor_drv_lock);
write_cmos_sensor(0x0100,0x00);
	mdelay(72);
	write_cmos_sensor(0x0103,0x00);
	write_cmos_sensor(0x0100,0x02);
	mdelay(10);
	capture_setting(imgsensor.current_fps);
write_cmos_sensor(0x0100,0x01);

	mdelay(10);

	return ERROR_NONE;
}

static kal_uint32 normal_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	//kal_uint16 dummy_line = 0;
	
	LOG_INF("E\n");
	
	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_VIDEO;
	imgsensor.pclk = imgsensor_info.normal_video.pclk;
	imgsensor.line_length = imgsensor_info.normal_video.linelength;
	imgsensor.frame_length = imgsensor_info.normal_video.framelength;
	imgsensor.min_frame_length = imgsensor_info.normal_video.framelength;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	write_cmos_sensor(0x0100,0x00);
	    mdelay(72);
	write_cmos_sensor(0x0103,0x00);
	write_cmos_sensor(0x0100,0x02);
		//HM8131SetSensorSetting(HM8131_VideoSettings);
		mdelay(10);
	preview_setting();
	write_cmos_sensor(0x0100,0x01);
	LOG_INF("Add 0100 by Ethan in Video \n");
	mdelay(10);
	mdelay(10);
	return ERROR_NONE;
	}
static kal_uint32 hs_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");
	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_HIGH_SPEED_VIDEO;
	imgsensor.pclk = imgsensor_info.hs_video.pclk;
	imgsensor.line_length = imgsensor_info.hs_video.linelength;
	imgsensor.frame_length = imgsensor_info.hs_video.framelength;
	imgsensor.min_frame_length = imgsensor_info.hs_video.framelength;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	return ERROR_NONE;
}	/*	hs_video   */

static kal_uint32 slim_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");
	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_SLIM_VIDEO;
	imgsensor.pclk = imgsensor_info.slim_video.pclk;
	imgsensor.line_length = imgsensor_info.slim_video.linelength;
	imgsensor.frame_length = imgsensor_info.slim_video.framelength;
	imgsensor.min_frame_length = imgsensor_info.slim_video.framelength;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	slim_video_setting();
    return ERROR_NONE;
}  /* slim_video */

/*************************************************************************
* FUNCTION
* Custom1
	
*************************************************************************/
static kal_uint32 Custom1(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                      MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    LOG_INF("E\n");
    spin_lock(&imgsensor_drv_lock);
    imgsensor.sensor_mode = IMGSENSOR_MODE_CUSTOM1;
    imgsensor.pclk = imgsensor_info.custom1.pclk;
    imgsensor.line_length = imgsensor_info.custom1.linelength;
    imgsensor.frame_length = imgsensor_info.custom1.framelength;
    imgsensor.min_frame_length = imgsensor_info.custom1.framelength;
    imgsensor.autoflicker_en = KAL_FALSE;
    spin_unlock(&imgsensor_drv_lock);
    preview_setting();
    return ERROR_NONE;
}   /*  Custom1   */

static kal_uint32 Custom2(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                      MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    LOG_INF("E\n");
    spin_lock(&imgsensor_drv_lock);
    imgsensor.sensor_mode = IMGSENSOR_MODE_CUSTOM2;
    imgsensor.pclk = imgsensor_info.custom2.pclk;
    imgsensor.line_length = imgsensor_info.custom2.linelength;
    imgsensor.frame_length = imgsensor_info.custom2.framelength;
    imgsensor.min_frame_length = imgsensor_info.custom2.framelength;
    imgsensor.autoflicker_en = KAL_FALSE;
    spin_unlock(&imgsensor_drv_lock);
    preview_setting();
    return ERROR_NONE;
}   /*  Custom2   */
static kal_uint32 Custom3(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                      MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    LOG_INF("E\n");
	
    spin_lock(&imgsensor_drv_lock);
    imgsensor.sensor_mode = IMGSENSOR_MODE_CUSTOM3;
    imgsensor.pclk = imgsensor_info.custom3.pclk;
    //imgsensor.video_mode = KAL_FALSE;
    imgsensor.line_length = imgsensor_info.custom3.linelength;
    imgsensor.frame_length = imgsensor_info.custom3.framelength;
    imgsensor.min_frame_length = imgsensor_info.custom3.framelength;
    imgsensor.autoflicker_en = KAL_FALSE;
    spin_unlock(&imgsensor_drv_lock);
    preview_setting();
    return ERROR_NONE;
	}
static kal_uint32 Custom4(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                      MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    LOG_INF("E\n");
    spin_lock(&imgsensor_drv_lock);
    imgsensor.sensor_mode = IMGSENSOR_MODE_CUSTOM4;
    imgsensor.pclk = imgsensor_info.custom4.pclk;
    //imgsensor.video_mode = KAL_FALSE;
    imgsensor.line_length = imgsensor_info.custom4.linelength;
    imgsensor.frame_length = imgsensor_info.custom4.framelength;
    imgsensor.min_frame_length = imgsensor_info.custom4.framelength;
    imgsensor.autoflicker_en = KAL_FALSE;
    spin_unlock(&imgsensor_drv_lock);
    preview_setting();
    return ERROR_NONE;
}   /*  Custom4   */


static kal_uint32 Custom5(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                      MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    LOG_INF("E\n");
    spin_lock(&imgsensor_drv_lock);
    imgsensor.sensor_mode = IMGSENSOR_MODE_CUSTOM5;
    imgsensor.pclk = imgsensor_info.custom5.pclk;
    imgsensor.line_length = imgsensor_info.custom5.linelength;
    imgsensor.frame_length = imgsensor_info.custom5.framelength;
    imgsensor.min_frame_length = imgsensor_info.custom5.framelength;
    imgsensor.autoflicker_en = KAL_FALSE;
    spin_unlock(&imgsensor_drv_lock);
    preview_setting();
	return ERROR_NONE;
}

static kal_uint32 get_resolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *sensor_resolution)
{
	LOG_INF("E\n");
	sensor_resolution->SensorFullWidth = imgsensor_info.cap.grabwindow_width;
	sensor_resolution->SensorFullHeight = imgsensor_info.cap.grabwindow_height;
	
	sensor_resolution->SensorPreviewWidth = imgsensor_info.pre.grabwindow_width;
	sensor_resolution->SensorPreviewHeight = imgsensor_info.pre.grabwindow_height;
	
	sensor_resolution->SensorVideoWidth = imgsensor_info.normal_video.grabwindow_width;
	sensor_resolution->SensorVideoHeight = imgsensor_info.normal_video.grabwindow_height;
	

	sensor_resolution->SensorHighSpeedVideoWidth	 = imgsensor_info.hs_video.grabwindow_width;
	sensor_resolution->SensorHighSpeedVideoHeight	 = imgsensor_info.hs_video.grabwindow_height;

	sensor_resolution->SensorSlimVideoWidth	 = imgsensor_info.slim_video.grabwindow_width;
	sensor_resolution->SensorSlimVideoHeight	 = imgsensor_info.slim_video.grabwindow_height;
    sensor_resolution->SensorHighSpeedVideoWidth     = imgsensor_info.hs_video.grabwindow_width;
    sensor_resolution->SensorHighSpeedVideoHeight    = imgsensor_info.hs_video.grabwindow_height;
    sensor_resolution->SensorSlimVideoWidth  = imgsensor_info.slim_video.grabwindow_width;
    sensor_resolution->SensorSlimVideoHeight     = imgsensor_info.slim_video.grabwindow_height;
    sensor_resolution->SensorCustom1Width  = imgsensor_info.custom1.grabwindow_width;
    sensor_resolution->SensorCustom1Height     = imgsensor_info.custom1.grabwindow_height;
    sensor_resolution->SensorCustom2Width  = imgsensor_info.custom2.grabwindow_width;
    sensor_resolution->SensorCustom2Height     = imgsensor_info.custom2.grabwindow_height;
    sensor_resolution->SensorCustom3Width  = imgsensor_info.custom3.grabwindow_width;
    sensor_resolution->SensorCustom3Height     = imgsensor_info.custom3.grabwindow_height;
    sensor_resolution->SensorCustom4Width  = imgsensor_info.custom4.grabwindow_width;
    sensor_resolution->SensorCustom4Height     = imgsensor_info.custom4.grabwindow_height;
    sensor_resolution->SensorCustom5Width  = imgsensor_info.custom5.grabwindow_width;
    sensor_resolution->SensorCustom5Height     = imgsensor_info.custom5.grabwindow_height;
	return ERROR_NONE;
}   /* HM8131MIPIGetResolution() */

static kal_uint32 get_info(MSDK_SCENARIO_ID_ENUM scenario_id,
					  MSDK_SENSOR_INFO_STRUCT *sensor_info,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("scenario_id = %d\n", scenario_id);

	//Mask????
	//sensor_info->SensorVideoFrameRate = imgsensor_info.normal_video.max_framerate/10; /* not use */
	//sensor_info->SensorStillCaptureFrameRate= imgsensor_info.cap.max_framerate/10; /* not use */
	//imgsensor_info->SensorWebCamCaptureFrameRate= imgsensor_info.v.max_framerate; /* not use */
	
	sensor_info->SensorClockPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorClockFallingPolarity = SENSOR_CLOCK_POLARITY_LOW; /* not use */
	sensor_info->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW; // inverse with datasheet
	sensor_info->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	
	sensor_info->SensorInterruptDelayLines = 4; /* not use */
	sensor_info->SensorResetActiveHigh = FALSE; /* not use */
	
	sensor_info->SensorResetDelayCount = 5; /* not use */

	sensor_info->SensroInterfaceType = imgsensor_info.sensor_interface_type;
	sensor_info->MIPIsensorType = imgsensor_info.mipi_sensor_type;
	sensor_info->SettleDelayMode = imgsensor_info.mipi_settle_delay_mode;
	sensor_info->SensorOutputDataFormat = imgsensor_info.sensor_output_dataformat;
	
	sensor_info->CaptureDelayFrame = imgsensor_info.cap_delay_frame;
	sensor_info->PreviewDelayFrame = imgsensor_info.pre_delay_frame;
	sensor_info->VideoDelayFrame = imgsensor_info.video_delay_frame;
	sensor_info->HighSpeedVideoDelayFrame = imgsensor_info.hs_video_delay_frame;
	sensor_info->SlimVideoDelayFrame = imgsensor_info.slim_video_delay_frame;
    sensor_info->Custom1DelayFrame = imgsensor_info.custom1_delay_frame;
    sensor_info->Custom2DelayFrame = imgsensor_info.custom2_delay_frame;
    sensor_info->Custom3DelayFrame = imgsensor_info.custom3_delay_frame;
    sensor_info->Custom4DelayFrame = imgsensor_info.custom4_delay_frame;
    sensor_info->Custom5DelayFrame = imgsensor_info.custom5_delay_frame;

	sensor_info->SensorMasterClockSwitch = 0; /* not use */
	sensor_info->SensorDrivingCurrent = imgsensor_info.isp_driving_current;

	sensor_info->AEShutDelayFrame = imgsensor_info.ae_shut_delay_frame; 		 /* The frame of setting shutter default 0 for TG int */
	sensor_info->AESensorGainDelayFrame = imgsensor_info.ae_sensor_gain_delay_frame;	/* The frame of setting sensor gain */
	sensor_info->AEISPGainDelayFrame = imgsensor_info.ae_ispGain_delay_frame;
	sensor_info->IHDR_Support = imgsensor_info.ihdr_support;
	sensor_info->IHDR_LE_FirstLine = imgsensor_info.ihdr_le_firstline;
	sensor_info->SensorModeNum = imgsensor_info.sensor_mode_num;

	sensor_info->SensorMIPILaneNumber = imgsensor_info.mipi_lane_num;
	sensor_info->SensorClockFreq = imgsensor_info.mclk;
	sensor_info->SensorClockDividCount = 3; /* not use */
	sensor_info->SensorClockRisingCount = 0;
	sensor_info->SensorClockFallingCount = 2; /* not use */
	sensor_info->SensorPixelClockCount = 3; /* not use */
	sensor_info->SensorDataLatchCount = 2; /* not use */

	sensor_info->MIPIDataLowPwr2HighSpeedTermDelayCount = 0;
	sensor_info->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
	sensor_info->SensorWidthSampling = 0;  // 0 is default 1x
	sensor_info->SensorHightSampling = 0;	// 0 is default 1x
	sensor_info->SensorPacketECCOrder = 1;


 //add 
	switch (scenario_id) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			sensor_info->SensorGrabStartX = imgsensor_info.pre.startx;
			sensor_info->SensorGrabStartY = imgsensor_info.pre.starty;
			
			
			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.pre.mipi_data_lp2hs_settle_dc;
			break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			sensor_info->SensorGrabStartX = imgsensor_info.cap.startx;
			sensor_info->SensorGrabStartY = imgsensor_info.cap.starty;
			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.cap.mipi_data_lp2hs_settle_dc;

			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:

			sensor_info->SensorGrabStartX = imgsensor_info.normal_video.startx;
			sensor_info->SensorGrabStartY = imgsensor_info.normal_video.starty;
			
			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.normal_video.mipi_data_lp2hs_settle_dc;

			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			sensor_info->SensorGrabStartX = imgsensor_info.hs_video.startx;
			sensor_info->SensorGrabStartY = imgsensor_info.hs_video.starty;
			
			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.hs_video.mipi_data_lp2hs_settle_dc;
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			sensor_info->SensorGrabStartX = imgsensor_info.slim_video.startx;
			sensor_info->SensorGrabStartY = imgsensor_info.slim_video.starty;
			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.slim_video.mipi_data_lp2hs_settle_dc;
			break;
        case MSDK_SCENARIO_ID_CUSTOM1:
            sensor_info->SensorGrabStartX = imgsensor_info.custom1.startx;
            sensor_info->SensorGrabStartY = imgsensor_info.custom1.starty;
            sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.custom1.mipi_data_lp2hs_settle_dc;
            break;
        case MSDK_SCENARIO_ID_CUSTOM2:
            sensor_info->SensorGrabStartX = imgsensor_info.custom2.startx;
            sensor_info->SensorGrabStartY = imgsensor_info.custom2.starty;
            sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.custom1.mipi_data_lp2hs_settle_dc;
			break;
        case MSDK_SCENARIO_ID_CUSTOM3:
            sensor_info->SensorGrabStartX = imgsensor_info.custom3.startx;
            sensor_info->SensorGrabStartY = imgsensor_info.custom3.starty;
            sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.custom1.mipi_data_lp2hs_settle_dc;
            break;
        case MSDK_SCENARIO_ID_CUSTOM4:
            sensor_info->SensorGrabStartX = imgsensor_info.custom4.startx;
            sensor_info->SensorGrabStartY = imgsensor_info.custom4.starty;
			
			
            sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.custom1.mipi_data_lp2hs_settle_dc;
            break;
        case MSDK_SCENARIO_ID_CUSTOM5:
            sensor_info->SensorGrabStartX = imgsensor_info.custom5.startx;
            sensor_info->SensorGrabStartY = imgsensor_info.custom5.starty;
            sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.custom1.mipi_data_lp2hs_settle_dc;
			break;
		default:
            sensor_info->SensorGrabStartX = imgsensor_info.pre.startx;
            sensor_info->SensorGrabStartY = imgsensor_info.pre.starty;
			
			
            sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.pre.mipi_data_lp2hs_settle_dc;
			break;
	}
	

	return ERROR_NONE;
}

static kal_uint32 control(MSDK_SCENARIO_ID_ENUM scenario_id, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{

	LOG_INF("scenario_id = %d\n", scenario_id);
	spin_lock(&imgsensor_drv_lock);
	imgsensor.current_scenario_id = scenario_id;
	spin_unlock(&imgsensor_drv_lock);
	switch (scenario_id) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			preview(image_window, sensor_config_data);
			break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			capture(image_window, sensor_config_data);
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			normal_video(image_window, sensor_config_data);
			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			hs_video(image_window, sensor_config_data);
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			slim_video(image_window, sensor_config_data);
			break;
        case MSDK_SCENARIO_ID_CUSTOM1:
            Custom1(image_window, sensor_config_data); // Custom1
			break;
        case MSDK_SCENARIO_ID_CUSTOM2:
            Custom2(image_window, sensor_config_data); // Custom1
            break;
        case MSDK_SCENARIO_ID_CUSTOM3:
            Custom3(image_window, sensor_config_data); // Custom1
			break;
        case MSDK_SCENARIO_ID_CUSTOM4:
            Custom4(image_window, sensor_config_data); // Custom1
            break;
        case MSDK_SCENARIO_ID_CUSTOM5:
            Custom5(image_window, sensor_config_data); // Custom1
            break;
		default:
			LOG_INF("Error ScenarioId setting");
			preview(image_window, sensor_config_data);
			return ERROR_INVALID_SCENARIO_ID;
	}
		
	return ERROR_NONE;
}


static kal_uint32 set_video_mode(UINT16 framerate)
{
	
	LOG_INF("framerate = %d\n ", framerate);

	if (framerate == 0)
		// Dynamic frame rate
		return ERROR_NONE;
	spin_lock(&imgsensor_drv_lock);
	if ((framerate == 300) && (imgsensor.autoflicker_en == KAL_TRUE))
		imgsensor.current_fps = 296;
	else if ((framerate == 150) && (imgsensor.autoflicker_en == KAL_TRUE))
		imgsensor.current_fps = 146;






	else
		imgsensor.current_fps = framerate;
	spin_unlock(&imgsensor_drv_lock);
	set_max_framerate(imgsensor.current_fps,1);

	
	return ERROR_NONE;
}

static kal_uint32 set_auto_flicker_mode(kal_bool enable, UINT16 framerate)
{

	LOG_INF("enable = %d, framerate = %d \n", enable, framerate);
	spin_lock(&imgsensor_drv_lock);
	if (enable) //enable auto flicker
		imgsensor.autoflicker_en = KAL_TRUE;
	else //Cancel Auto flick
		imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	return ERROR_NONE;
	}
	

static kal_uint32 set_max_framerate_by_scenario(MSDK_SCENARIO_ID_ENUM scenario_id, MUINT32 framerate)
	{		
	kal_uint32 frame_length;

	LOG_INF("scenario_id = %d, framerate = %d\n", scenario_id, framerate);
	switch (scenario_id) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			frame_length = imgsensor_info.pre.pclk / framerate * 10 / imgsensor_info.pre.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.pre.framelength) ? (frame_length - imgsensor_info.pre.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.pre.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			set_dummy();
				break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			if(framerate == 0)
				return ERROR_NONE;
			frame_length = imgsensor_info.normal_video.pclk / framerate * 10 / imgsensor_info.normal_video.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.normal_video.framelength) ? (frame_length - imgsensor_info.normal_video.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.normal_video.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			set_dummy();
				break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			frame_length = imgsensor_info.cap.pclk / framerate * 10 / imgsensor_info.cap.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.cap.framelength) ? (frame_length - imgsensor_info.cap.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.cap.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			set_dummy();
				break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			frame_length = imgsensor_info.hs_video.pclk / framerate * 10 / imgsensor_info.hs_video.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.hs_video.framelength) ? (frame_length - imgsensor_info.hs_video.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.hs_video.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			set_dummy();
				break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			frame_length = imgsensor_info.slim_video.pclk / framerate * 10 / imgsensor_info.slim_video.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.slim_video.framelength) ? (frame_length - imgsensor_info.slim_video.framelength): 0;
			imgsensor.frame_length = imgsensor_info.slim_video.framelength + imgsensor.dummy_line;

			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			set_dummy();
        case MSDK_SCENARIO_ID_CUSTOM1:
            frame_length = imgsensor_info.custom1.pclk / framerate * 10 / imgsensor_info.custom1.linelength;
            spin_lock(&imgsensor_drv_lock);
            imgsensor.dummy_line = (frame_length > imgsensor_info.custom1.framelength) ? (frame_length - imgsensor_info.custom1.framelength) : 0;
            if (imgsensor.dummy_line < 0)
                imgsensor.dummy_line = 0;
            imgsensor.frame_length = imgsensor_info.custom1.framelength + imgsensor.dummy_line;


	

	

            imgsensor.min_frame_length = imgsensor.frame_length;
            spin_unlock(&imgsensor_drv_lock);
            set_dummy();
            break;
	
        case MSDK_SCENARIO_ID_CUSTOM2:
            frame_length = imgsensor_info.custom2.pclk / framerate * 10 / imgsensor_info.custom2.linelength;
            spin_lock(&imgsensor_drv_lock);
            imgsensor.dummy_line = (frame_length > imgsensor_info.custom2.framelength) ? (frame_length - imgsensor_info.custom2.framelength) : 0;
            if (imgsensor.dummy_line < 0)
                imgsensor.dummy_line = 0;
            imgsensor.frame_length = imgsensor_info.custom2.framelength + imgsensor.dummy_line;
            imgsensor.min_frame_length = imgsensor.frame_length;
            spin_unlock(&imgsensor_drv_lock);
            set_dummy();
			break;
        case MSDK_SCENARIO_ID_CUSTOM3:
            frame_length = imgsensor_info.custom3.pclk / framerate * 10 / imgsensor_info.custom3.linelength;
            spin_lock(&imgsensor_drv_lock);
            imgsensor.dummy_line = (frame_length > imgsensor_info.custom3.framelength) ? (frame_length - imgsensor_info.custom3.framelength) : 0;
            if (imgsensor.dummy_line < 0)
                imgsensor.dummy_line = 0;
            imgsensor.frame_length = imgsensor_info.custom3.framelength + imgsensor.dummy_line;
            imgsensor.min_frame_length = imgsensor.frame_length;
            spin_unlock(&imgsensor_drv_lock);
            set_dummy();
			break;
        case MSDK_SCENARIO_ID_CUSTOM4:
            frame_length = imgsensor_info.custom4.pclk / framerate * 10 / imgsensor_info.custom4.linelength;
            spin_lock(&imgsensor_drv_lock);
            imgsensor.dummy_line = (frame_length > imgsensor_info.custom4.framelength) ? (frame_length - imgsensor_info.custom4.framelength) : 0;
            if (imgsensor.dummy_line < 0)
                imgsensor.dummy_line = 0;
            imgsensor.frame_length = imgsensor_info.custom4.framelength + imgsensor.dummy_line;
            imgsensor.min_frame_length = imgsensor.frame_length;
            spin_unlock(&imgsensor_drv_lock);
            set_dummy();
            break;
        case MSDK_SCENARIO_ID_CUSTOM5:
            frame_length = imgsensor_info.custom5.pclk / framerate * 10 / imgsensor_info.custom5.linelength;
            spin_lock(&imgsensor_drv_lock);
            imgsensor.dummy_line = (frame_length > imgsensor_info.custom5.framelength) ? (frame_length - imgsensor_info.custom5.framelength) : 0;
            if (imgsensor.dummy_line < 0)
                imgsensor.dummy_line = 0;
            imgsensor.frame_length = imgsensor_info.custom1.framelength + imgsensor.dummy_line;
            imgsensor.min_frame_length = imgsensor.frame_length;
            spin_unlock(&imgsensor_drv_lock);
            set_dummy();
			break;
		default:
			frame_length = imgsensor_info.pre.pclk / framerate * 10 / imgsensor_info.pre.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.pre.framelength) ? (frame_length - imgsensor_info.pre.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.pre.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			set_dummy();
			LOG_INF("error scenario_id = %d, we use preview scenario \n", scenario_id);
			break;
	}	

	
	


	return ERROR_NONE;
}

static kal_uint32 get_default_framerate_by_scenario(MSDK_SCENARIO_ID_ENUM scenario_id, MUINT32 *framerate)
{
	LOG_INF("scenario_id = %d\n", scenario_id);

	switch (scenario_id) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			*framerate = imgsensor_info.pre.max_framerate;
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			*framerate = imgsensor_info.normal_video.max_framerate;
			break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			*framerate = imgsensor_info.cap.max_framerate;
			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			*framerate = imgsensor_info.hs_video.max_framerate;
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			*framerate = imgsensor_info.slim_video.max_framerate;
			break;



	
	



        case MSDK_SCENARIO_ID_CUSTOM1:
            *framerate = imgsensor_info.custom1.max_framerate;
			break;
        case MSDK_SCENARIO_ID_CUSTOM2:
            *framerate = imgsensor_info.custom2.max_framerate;
			break;
        case MSDK_SCENARIO_ID_CUSTOM3:
            *framerate = imgsensor_info.custom3.max_framerate;
			break;
        case MSDK_SCENARIO_ID_CUSTOM4:
            *framerate = imgsensor_info.custom4.max_framerate;
			break;
        case MSDK_SCENARIO_ID_CUSTOM5:
            *framerate = imgsensor_info.custom5.max_framerate;
			break;
		default:
			break;
	}
	return ERROR_NONE;
}



static kal_uint32 feature_control(MSDK_SENSOR_FEATURE_ENUM feature_id,
							 UINT8 *feature_para,UINT32 *feature_para_len)
{
	UINT16 *feature_return_para_16=(UINT16 *) feature_para;
	UINT16 *feature_data_16=(UINT16 *) feature_para;
	UINT32 *feature_return_para_32=(UINT32 *) feature_para;
	UINT32 *feature_data_32=(UINT32 *) feature_para;


//derek
    unsigned long long *feature_data=(unsigned long long *) feature_para;
  //  unsigned long long *feature_return_para=(unsigned long long *) feature_para;

	SENSOR_WINSIZE_INFO_STRUCT *wininfo;
	MSDK_SENSOR_REG_INFO_STRUCT *sensor_reg_data=(MSDK_SENSOR_REG_INFO_STRUCT *) feature_para;



	LOG_INF("feature_id = %d\n", feature_id);
	switch (feature_id) {
		case SENSOR_FEATURE_GET_PERIOD:
			*feature_return_para_16++ = imgsensor.line_length;
			*feature_return_para_16 = imgsensor.frame_length;
			*feature_para_len=4;
			break;
		case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
            LOG_INF("feature_Control imgsensor.pclk = %d,imgsensor.current_fps = %d\n", imgsensor.pclk,imgsensor.current_fps);
			*feature_return_para_32 = imgsensor.pclk;
			*feature_para_len=4;
			break;
		case SENSOR_FEATURE_SET_ESHUTTER:
            set_shutter(*feature_data);
			break;
		case SENSOR_FEATURE_SET_NIGHTMODE:
            night_mode((BOOL) *feature_data);
			break;
		case SENSOR_FEATURE_SET_GAIN:
            set_gain((UINT16) *feature_data);
			break;
		case SENSOR_FEATURE_SET_FLASHLIGHT:
			break;
		case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
			//HM8131MIPI_isp_master_clock = *pFeatureData32;
			break;
		case SENSOR_FEATURE_SET_REGISTER:
			write_cmos_sensor(sensor_reg_data->RegAddr, sensor_reg_data->RegData);
			break;
		case SENSOR_FEATURE_GET_REGISTER:
			sensor_reg_data->RegData = read_cmos_sensor(sensor_reg_data->RegAddr);
			break;
		case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
			// get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE
			// if EEPROM does not exist in camera module.
			*feature_return_para_32=LENS_DRIVER_ID_DO_NOT_CARE;
			*feature_para_len=4;
			break;
		case SENSOR_FEATURE_SET_VIDEO_MODE:
            set_video_mode(*feature_data);
			break;
		case SENSOR_FEATURE_CHECK_SENSOR_ID:
			get_imgsensor_id(feature_return_para_32);
			break;
		case SENSOR_FEATURE_SET_AUTO_FLICKER_MODE:
			set_auto_flicker_mode((BOOL)*feature_data_16,*(feature_data_16+1));
			break;
		case SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO:
            set_max_framerate_by_scenario((MSDK_SCENARIO_ID_ENUM)*feature_data, *(feature_data+1));
			break;
		case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
            get_default_framerate_by_scenario((MSDK_SCENARIO_ID_ENUM)*(feature_data), (MUINT32 *)(uintptr_t)(*(feature_data+1)));
			break;
		case SENSOR_FEATURE_SET_TEST_PATTERN:
            set_test_pattern_mode((BOOL)*feature_data);
			break;
		case SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE: //for factory mode auto testing
			*feature_return_para_32 = imgsensor_info.checksum_value;
			*feature_para_len=4;
			break;
		case SENSOR_FEATURE_SET_FRAMERATE:
            LOG_INF("current fps :%d\n", (UINT32)*feature_data);
			spin_lock(&imgsensor_drv_lock);
            imgsensor.current_fps = *feature_data;
			spin_unlock(&imgsensor_drv_lock);
			break;
		case SENSOR_FEATURE_SET_HDR:
            LOG_INF("ihdr enable :%d\n", (BOOL)*feature_data);
			spin_lock(&imgsensor_drv_lock);
			imgsensor.ihdr_en = *feature_data;
			spin_unlock(&imgsensor_drv_lock);
			break;
		case SENSOR_FEATURE_GET_CROP_INFO:
            LOG_INF("SENSOR_FEATURE_GET_CROP_INFO scenarioId:%d\n", (UINT32)*feature_data);
            wininfo = (SENSOR_WINSIZE_INFO_STRUCT *)(uintptr_t)(*(feature_data+1));

			switch (*feature_data_32) {
				case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
					memcpy((void *)wininfo,(void *)&imgsensor_winsize_info[1],sizeof(SENSOR_WINSIZE_INFO_STRUCT));
			break;
				case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
					memcpy((void *)wininfo,(void *)&imgsensor_winsize_info[2],sizeof(SENSOR_WINSIZE_INFO_STRUCT));
			break;
				case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
					memcpy((void *)wininfo,(void *)&imgsensor_winsize_info[3],sizeof(SENSOR_WINSIZE_INFO_STRUCT));
			break;
				case MSDK_SCENARIO_ID_SLIM_VIDEO:
					memcpy((void *)wininfo,(void *)&imgsensor_winsize_info[4],sizeof(SENSOR_WINSIZE_INFO_STRUCT));
			break;
				case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			//HM8131SetAutoFlickerMode( (BOOL)*pFeatureData16, *(pFeatureData16+1) );
				default:
					memcpy((void *)wininfo,(void *)&imgsensor_winsize_info[0],sizeof(SENSOR_WINSIZE_INFO_STRUCT));
			break;
			
			}
				
		
			case SENSOR_FEATURE_SET_IHDR_SHUTTER_GAIN:
            LOG_INF("SENSOR_SET_SENSOR_IHDR LE=%d, SE=%d, Gain=%d\n",(UINT16)*feature_data,(UINT16)*(feature_data+1),(UINT16)*(feature_data+2));
            ihdr_write_shutter_gain((UINT16)*feature_data,(UINT16)*(feature_data+1),(UINT16)*(feature_data+2));
				break;

		default:
			break;
	}

	
	return ERROR_NONE;
} 

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

static SENSOR_FUNCTION_STRUCT sensor_func = {
	open,
	get_info,
	get_resolution,
	feature_control,
	control,
	close
};

UINT32 HM8131MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
	/* To Do : Check Sensor status here */
	if( pfFunc != NULL )
		*pfFunc=&sensor_func;
	
	return ERROR_NONE;
}   /* SensorInit() */
