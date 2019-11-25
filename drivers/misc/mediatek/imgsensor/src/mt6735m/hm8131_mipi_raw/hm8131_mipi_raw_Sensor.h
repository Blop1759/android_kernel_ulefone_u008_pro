/*******************************************************************************************/


/*******************************************************************************************/
#ifndef _HM8131MIPI_SENSOR_H
#define _HM8131MIPI_SENSOR_H







typedef enum {
	IMGSENSOR_MODE_INIT,
	IMGSENSOR_MODE_PREVIEW,
	IMGSENSOR_MODE_CAPTURE,
	IMGSENSOR_MODE_VIDEO,
	IMGSENSOR_MODE_HIGH_SPEED_VIDEO,
	IMGSENSOR_MODE_SLIM_VIDEO,
    IMGSENSOR_MODE_CUSTOM1,
    IMGSENSOR_MODE_CUSTOM2,
    IMGSENSOR_MODE_CUSTOM3,
    IMGSENSOR_MODE_CUSTOM4,
    IMGSENSOR_MODE_CUSTOM5,
} IMGSENSOR_MODE;

typedef struct imgsensor_mode_struct {
	kal_uint32 pclk;				//record different mode's pclk
	kal_uint32 linelength;			//record different mode's linelength

	kal_uint32 framelength;			//record different mode's framelength

	kal_uint8 startx;				//record different mode's startx of grabwindow
	kal_uint8 starty;				//record different mode's startx of grabwindow
	
	kal_uint16 grabwindow_width;	//record different mode's width of grabwindow
	kal_uint16 grabwindow_height;	//record different mode's height of grabwindow

	/*	 following for MIPIDataLowPwr2HighSpeedSettleDelayCount by different scenario	*/
	kal_uint8 mipi_data_lp2hs_settle_dc;

	/*	 following for GetDefaultFramerateByScenario()	*/
	kal_uint16 max_framerate;

	
} imgsensor_mode_struct;

typedef struct imgsensor_struct {
	kal_uint8 mirror;				//mirrorflip information
	

	kal_uint8 sensor_mode;			//record IMGSENSOR_MODE enum value
/* Sensor Clock Info,UT: KHz */


/*First Bayer Color*/

/*First Point Info For Output Image*/

/*Image Size For Output*/

/* Defualt Frame Info  Width & Height*/


// TODO:



/* Slave address */
//#define		HM8131MIPI_WRITE_ID_0				(0x48)
//#define		HM8131MIPI_READ_ID_0					(0x49)

/* Sensor ID, Move to kd_imgsensor.h*/
//#define	HM8131MIPI_SENSOR_ID		0x8131	



/*Sensor Settings*/
	kal_uint32 shutter;				//current shutter
	kal_uint16 gain;				//current gain

//#define MIPI_4_lane_enable 1

	
	kal_uint32 pclk;				//current pclk
	
	kal_uint32 frame_length;		//current framelength
	kal_uint32 line_length;			//current linelength
	
	
	
	
				  
	
	
		//mipi 2-lane 1216 version
		//{0x0303,0x01}, //2 1 ; PLL adc1 - enable PLL -> was 002A
	
	//TEST OK: 2A_3B  31_45 3F_58 46_62
		//{0x0307,0x46}, //2 1 ; PLL M, pclk_raw=68mhz
	

	kal_uint32 min_frame_length;	//current min  framelength to max framerate
	kal_uint16 dummy_pixel;			//current dummypixel
	kal_uint16 dummy_line;			//current dummline
				 
				 
				   
				   
				   
				   
				   
				   
				   
	 
	 //===========
	 
				   
	 //analog
	 
	
	kal_uint16 current_fps;			//current max fps
	kal_bool   autoflicker_en;		//record autoflicker enable or disable
	kal_bool test_pattern;			//record test pattern mode or not
	MSDK_SCENARIO_ID_ENUM current_scenario_id;//current scenario id
	kal_uint8  ihdr_en;				//ihdr enable or disable
	
	kal_uint8 i2c_write_id;			//record current sensor's i2c write id
} imgsensor_struct;
	
//#else
	
//#endif    
	   //{0x4B03,0x05}, //2 1 ; hs_zero_width
	
	
	
	   //{0x4B3F,0x08}, //2 1 ; [3:0] mipi_req_dly
	
	
	
//#if for_module 
//#else          
//	   {0x0101,0x03}, //flip+mirror
//#endif
	
	
	
	
	
	
	


typedef struct imgsensor_info_struct { 
	kal_uint32 sensor_id;			//record sensor id defined in Kd_imgsensor.h
	kal_uint32 checksum_value;		//checksum value for Camera Auto Test
	imgsensor_mode_struct pre;		//preview scenario relative information
	imgsensor_mode_struct cap;		//capture scenario relative information
	imgsensor_mode_struct cap1;		//capture for PIP 24fps relative information, capture1 mode must use same framelength, linelength with Capture mode for shutter calculate
	imgsensor_mode_struct normal_video;//normal video  scenario relative information
	imgsensor_mode_struct hs_video;	//high speed video scenario relative information
	imgsensor_mode_struct slim_video;	//slim video for VT scenario relative information
    imgsensor_mode_struct custom1;      //custom1 scenario relative information
    imgsensor_mode_struct custom2;      //custom2 scenario relative information
    imgsensor_mode_struct custom3;      //custom3 scenario relative information
    imgsensor_mode_struct custom4;      //custom4 scenario relative information
    imgsensor_mode_struct custom5;      //custom5 scenario relative information
	
	kal_uint8  ae_shut_delay_frame;	//shutter delay frame for AE cycle
	kal_uint8  ae_sensor_gain_delay_frame;	//sensor gain delay frame for AE cycle
	kal_uint8  ae_ispGain_delay_frame;	//isp gain delay frame for AE cycle
	kal_uint8  ihdr_support;		//1, support; 0,not support
	kal_uint8  ihdr_le_firstline;	//1,le first ; 0, se first
	kal_uint8  sensor_mode_num;		//support sensor mode num
	
	kal_uint8  cap_delay_frame;		//enter capture delay frame num
	kal_uint8  pre_delay_frame;		//enter preview delay frame num
	kal_uint8  video_delay_frame;	//enter video delay frame num
	kal_uint8  hs_video_delay_frame;	//enter high speed video  delay frame num
	kal_uint8  slim_video_delay_frame;	//enter slim video delay frame num
    kal_uint8  custom1_delay_frame;     //enter custom1 delay frame num
    kal_uint8  custom2_delay_frame;     //enter custom1 delay frame num
    kal_uint8  custom3_delay_frame;     //enter custom1 delay frame num
    kal_uint8  custom4_delay_frame;     //enter custom1 delay frame num
    kal_uint8  custom5_delay_frame;     //enter custom1 delay frame num
		
		

	
	//{0x030F,0x31}, //2 1 ; PLL M, pkt_clk=196mhz
	 
		//{0x030D, 0x0C},//2 1 ; PLL N,
		//{0x0309, 0x02},
		//{0x400D, 0x04},
		//{0x0383, 0x03},
		//{0x0387, 0x03},
		//{0x0390, 0x01},//01
		//{0x0390,0x11}, //1028, for 2-lan

	 //analog
				 
				 
	  //{0x427D, 0x00},
	  //{0x427E, 0x03},
	  //{0x427F, 0x00},
	  //{0x4380, 0xA6},
	  //{0x4381, 0x7B},
	  //{0x4382, 0x00},
	  //{0x4388, 0x9F},
	  //{0x4389, 0x15},
	  
	kal_uint8  margin;				//sensor framelength & shutter margin 
	kal_uint32 min_shutter;			//min shutter
	kal_uint32 max_frame_length;	//max framelength by sensor register's limitation
	  
				   
				   
				   
				   
				   
				   
				   
	 
	 //===========
	 
				   
	 //analog
	 
	
	
	   //{0x4274,0x33}, //2 1 ; [5] mask out bad frame due to mode (flip/mirror) change
	   //{0x4002,0x23}, //2 1 ; output BPC
	   
	
//#if MIPI_4_lane_enable
		//{0x0111,0x01},
	
//#else
	
//#endif    
	
	
	
	   
	
	
	
//#if for_module 
//#else          
//	   {0x0101,0x03}, //flip+mirror
//#endif
	
	//{0x0350,0x33},
	
	//{0x3110,0x01},
	//{0x3111,0x01},				
	//{0x3130,0x01},
	//{0x3131,0x26},

	kal_uint8  isp_driving_current;	//mclk driving current
	kal_uint8  sensor_interface_type;//sensor_interface_type
	kal_uint8  mipi_sensor_type; //0,MIPI_OPHY_NCSI2; 1,MIPI_OPHY_CSI2, default is NCSI2, don't modify this para
	kal_uint8  mipi_settle_delay_mode; //0, high speed signal auto detect; 1, use settle delay,unit is ns, default is auto detect, don't modify this para
	kal_uint8  sensor_output_dataformat;//sensor output first pixel color
	kal_uint8  mclk;				//mclk value, suggest 24 or 26 for 24Mhz or 26Mhz
//{0x0349,0xCD},//3277
//{0x034A,0x09},
//{0x034B,0x9D},//2461

//{0x034C,0x06},
//{0x034D,0x68},//1632
//{0x034E,0x04},
//{0x034F,0xD0},//1224


	   //{0x0340,0x05},
	   //{0x0341,0x1A},







	
	kal_uint8  mipi_lane_num;		//mipi lane num
	kal_uint8  i2c_addr_table[5];	//record sensor support all write id addr, only supprt 4must end with 0xff
    kal_uint32  i2c_speed;     //i2c speed
} imgsensor_info_struct;
	
	
		
	
	
	   


		//analog();

//#define IMGSENSOR_WRITE_ID_2 (0x20)
//#define IMGSENSOR_READ_ID_2  (0x21)
					
					
					  
					  
					  
					  
					  
					  
					  
		
		//===========
		
					  

extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);
extern void kdSetI2CSpeed(u16 i2cSpeed);
	   //{0x4274,0x33}, //2 1 ; [5] mask out bad frame due to mode (flip/mirror) change
	   

//#if MIPI_4_lane_enable
		//{0x0111,0x01},
//#else	
//#endif    
		//{0x4B03,0x0E},
		

	
//#if for_module
//#else
//		{0x0101,0x03}, //flip+mirror
//#endif
	
	
	
	//{0x0350,0x33},
	
	//{0x3110,0x23},
	//{0x3111,0x00},
	//{0x3130,0x01},
	//{0x3131,0x80},

//Soso modify
//{0x0344,0x00}, 
//{0x0345,0x00},	 
//{0x0346,0x00},
//{0x0347,0x00},
//{0x0348,0x0C},
//{0x0349,0xCF},//3279
//{0x034A,0x09},
//{0x034B,0x9F},//2463

//{0x034C,0x0C},
//{0x034D,0xC0},//3264
//{0x034E,0x09},
//{0x034F,0x90},//2448


	




////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

//export functions

#endif /* _HM8131MIPI_SENSOR_H_ */
