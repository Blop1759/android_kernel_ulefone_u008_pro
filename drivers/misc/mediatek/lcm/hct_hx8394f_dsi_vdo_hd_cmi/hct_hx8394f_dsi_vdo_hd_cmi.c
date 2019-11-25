/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/kernel.h>
#endif

#include "lcm_drv.h"

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1280)

#define LCM_ID                      								(0x0F)

#define REGFLAG_DELAY             								(0XFE)
#define REGFLAG_END_OF_TABLE      								(0x100)	// END OF REGISTERS MARKER

#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    									(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 										(lcm_util.udelay(n))
#define MDELAY(n) 										(lcm_util.mdelay(n))

//static unsigned int lcm_esd_test = FALSE;      ///only for ESD test

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------
//#define dsi_set_cmdq_V3(para_tbl,size,force_update)        lcm_util.dsi_set_cmdq_V3(para_tbl,size,force_update)
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
/* #define read_reg lcm_util.dsi_read_reg() */
#define read_reg_v2(cmd, buffer, buffer_size)				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

 struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};


static struct LCM_setting_table lcm_initialization_setting[] = {
// Set EXTC
  {0xB9,3,{0xFF,0x83,0x94}},

  {0xBA,6,{0x62,0x03,0x68,0x6B,0xB2,0xC0}},
 
  {0xB1,10,{0x50,0x12,0x72,0x09,0x33,0x54,0x71,0x31,0x70,0x2F}},

  {0xB2, 6,{0x00,0x80,0x64,0x0E,0x0D,0x2F}},
            
  {0xB4,21,{0x64,0x65,0x64,0x65,0x64,0x65,0x01,0x01,0x7E,0x75,
            0x00,0x3F,0x64,0x65,0x64,0x65,0x64,0x65,0x01,0x01,0x7E}},

  {0xB6,2,{0x6b,0x6b}},//VCOM  
  
  

  {0xD3,33,{0x00,0x00,0x07,0x07,0x40,0x07,0x10,0x00,0x08,0x10,
            0x08,0x00,0x08,0x54,0x15,0x0e,0x05,0x0e,0x02,0x15,
            0x06,0x05,0x06,0x47,0x44,0x0a,0x0a,0x4b,0x10,0x07,0x07,0x0e,0x40}},
            
  {0xD5,44,{0x1a,0x1a,0x1b,0x1b,0x00,0x01,0x02,0x03,0x04,0x05,
            0x06,0x07,0x08,0x09,0x0a,0x0b,0x24,0x25,0x18,0x18,
            0x26,0x27,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
            0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x20,0x21,0x18,0x18,0x18,0x18}}, 
              
  {0xD6,44,{0x1a,0x1a,0x1b,0x1b,0x0b,0x0a,0x09,0x08,0x07,0x06,
            0x05,0x04,0x03,0x02,0x01,0x00,0x21,0x20,0x18,0x18,
            0x27,0x26,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
            0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x25,0x24,0x18,0x18,0x18,0x18}},   
             
         
             
  //{0xE0,58,{0x00,0x03,0x0C,0x12,0x16,0x19,0x1C,0x1B,0x3A,0x4B,
  //          0x60,0x62,0x6F,0x86,0x8F,0x96,0xA5,0xA8,0xA5,0xB3,
  //          0xC2,0x5F,0x5E,0x61,0x66,0x69,0x73,0x77,0x7F,0x00,
   //         0x03,0x0C,0x12,0x16,0x19,0x1C,0x1B,0x3A,0x4B,0x60,
  //          0x62,0x6F,0x86,0x8F,0x96,0xA5,0xA8,0xA5,0xB3,0xC2,
  //          0x5F,0x5E,0x61,0x66,0x69,0x73,0x77,0x7F}},
  {0xE0,58,	{0x00,0x0C,0x19,0x20,0x23,0x28,0x2e,0x2e,0x5f,0x71,
  		0x83,0x81,0x89,0x9c,0xa1,0xa3,0xac,0xaa,0xa3,0xAc,
  		0xB8,0x5a,0x58,0x5b,0x5f,0x61,0x67,0x6a,0x7F,
		0x00,0x0C,0x19,0x20,0x23,0x28,0x2e,0x2e,0x5f,0x71,
  		0x83,0x81,0x89,0x9c,0xa1,0xa3,0xac,0xaa,0xa3,0xAc,
  		0xB8,0x5a,0x58,0x5b,0x5f,0x61,0x67,0x6a,0x7F}}, 
  
            
  {0xC0,2,{0x1F,0x73}},
             
  {0xCC,1,{0x0b}},	 //03
  
  {0xD4,1,{0x02}},  

{0xbd,1,{0x01}}, 
{0xb1,1,{0x60}}, 
{0xbd,1,{0x00}}, 
                                   

{0xbd,1,{0x02}}, 
{0xd8,12,{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}}, 
{0xbd,1,{0x00}}, 
   
  {0xBF,7,{0x40,0x81,0x50,0x00,0x1A,0xFC,0x01}},                                   
  {0x11,1,{0x00}},		// Sleep-Out
  {REGFLAG_DELAY, 120,  {}},          
  {0x29,1,{0x00}},    //Display on            
  {REGFLAG_DELAY, 10,  {}},

	// Setting ending by predefined flag
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


#if 0
static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},
  	{0xBF,7,{0x40,0x81,0x50,0x02,0x1A,0xFC,0x02}}, 
    // Display ON
	{0x29, 1, {0x00}},
	{REGFLAG_DELAY, 10, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif

static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Display off sequence
	{0x28, 1, {0x00}},
    	{REGFLAG_DELAY, 10, {}},

	// Sleep Mode On
	{0x10, 1, {0x00}},
	{REGFLAG_DELAY, 150, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#if 0
static struct LCM_setting_table lcm_compare_id_setting[] = {
	// Display off sequence
	{0xf0, 5, {0x55, 0xaa, 0x52, 0x08, 0x01}},
	{REGFLAG_DELAY, 10, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_backlight_level_setting[] = {
	{0x51, 1, {0xFF}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

    for(i = 0; i < count; i++) {
		
        unsigned cmd;
        cmd = table[i].cmd;
		
        switch (cmd) {
			
            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;
				
            case REGFLAG_END_OF_TABLE :
                break;
				
            default:
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
       	}
    }
	
}


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}

static void lcm_get_params(LCM_PARAMS *params)
{
	memset(params, 0, sizeof(LCM_PARAMS));
	params->type   = LCM_TYPE_DSI;
	params->width  = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;

	// enable tearing-free
	params->dbi.te_mode 			= LCM_DBI_TE_MODE_DISABLED;
    //params->dbi.te_edge_polarity          = LCM_POLARITY_RISING;

	params->dsi.mode   =SYNC_EVENT_VDO_MODE;

	// DSI
	/* Command mode setting */
	params->dsi.LANE_NUM			= LCM_THREE_LANE;
	//The following defined the fomat for data coming from LCD engine. 
	params->dsi.data_format.color_order 	= LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq   	= LCM_DSI_TRANS_SEQ_MSB_FIRST; 
	params->dsi.data_format.padding     	= LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format      	= LCM_DSI_FORMAT_RGB888;
	// Highly depends on LCD driver capability.
	// Not support in MT6573
	params->dsi.packet_size=256;
	// Video mode setting		
	params->dsi.intermediat_buffer_num 	= 2;
	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
        //params->dsi.word_count=720 * 3;
	params->dsi.vertical_sync_active				= 4;//5
	params->dsi.vertical_backporch					= 12;
	params->dsi.vertical_frontporch					= 15;
	params->dsi.vertical_active_line				= FRAME_HEIGHT; 

	params->dsi.horizontal_sync_active				= 10;//37;//10;
	params->dsi.horizontal_backporch				= 20;//60;//20;
	params->dsi.horizontal_frontporch				= 40;//60;//40;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	//params->dsi.ssc_disable=1;
	params->dsi.cont_clock=0;
	params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable      = 1;
 
	params->dsi.lcm_esd_check_table[0].cmd          = 0xd9;
	params->dsi.lcm_esd_check_table[0].count        = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x80;
 /*
	params->dsi.lcm_esd_check_table[1].cmd          = 0x09;
	params->dsi.lcm_esd_check_table[1].count        = 3;
	params->dsi.lcm_esd_check_table[1].para_list[0] = 0x80;
	params->dsi.lcm_esd_check_table[1].para_list[1] = 0x73;
	params->dsi.lcm_esd_check_table[1].para_list[2] = 0x06;
 

	params->dsi.lcm_esd_check_table[2].cmd          = 0x45;
	params->dsi.lcm_esd_check_table[2].count        = 2;
	params->dsi.lcm_esd_check_table[2].para_list[0] = 0x0f;
	params->dsi.lcm_esd_check_table[2].para_list[1] = 0xf0;
*/
//    	params->dsi.PLL_CLOCK=241;//250 ;
#if FRAME_WIDTH == 480	
	params->dsi.PLL_CLOCK=210;//254//247
#elif FRAME_WIDTH == 540
	params->dsi.PLL_CLOCK=230;
#elif FRAME_WIDTH == 720
	params->dsi.PLL_CLOCK=230;
#elif FRAME_WIDTH == 1080
	params->dsi.PLL_CLOCK=410;
#else
	params->dsi.PLL_CLOCK=230;
#endif
 		//params->dsi.cont_clock=1;
} 
#if 0
static unsigned int lcm_init_resgister(void)
{
     unsigned int data_array[16];
  
    data_array[0] = 0x00043902;
    data_array[1] = 0x9483ffb9;
    dsi_set_cmdq(data_array, 2, 1);
    MDELAY(5);
    data_array[0] = 0x00073902;
    data_array[1] = 0x680361ba;//0x680363ba
    data_array[2] = 0x00c0b26b;
    dsi_set_cmdq(data_array, 3, 1);
    MDELAY(5);
    data_array[0] = 0x000B3902;
    data_array[1] = 0x721250b1;
    data_array[2] = 0x71543309;
    data_array[3] = 0x002f7031;
    dsi_set_cmdq(data_array, 4, 1);
    MDELAY(5);
    data_array[0] = 0x00073902;
    data_array[1] = 0x648000b2;
    data_array[2] = 0x002F0D0E;
    dsi_set_cmdq(data_array, 3, 1);
    MDELAY(5);
    data_array[0] = 0x00163902;
    data_array[1] = 0x6C6D6Cb4;
    data_array[2] = 0x016D6C6D;
    data_array[3] = 0x0075FF01;
    data_array[4] = 0x6C6D6C3F;
       data_array[5] = 0x016D6C6D;
           data_array[6] = 0x0000FF01;
    dsi_set_cmdq(data_array, 7, 1);
    MDELAY(5);
   
        data_array[0] = 0x00033902;
    data_array[1] = 0x007575b6;
    dsi_set_cmdq(data_array, 2, 1);
    MDELAY(5);
    
    data_array[0] = 0x00223902;
    data_array[1] = 0x070000d3;
    data_array[2] = 0x10074007;
    data_array[3] = 0x08100800;
    data_array[4] = 0x15540800;
    data_array[5] = 0x020E050E;
    data_array[6] = 0x06050615;
    data_array[7] = 0x0A0A4447;
    data_array[8] = 0x0707104B;
    data_array[9] = 0x0000400E;
    dsi_set_cmdq(data_array, 10, 1);
    MDELAY(5);
   
    data_array[0] = 0x002d3902;
    data_array[1] = 0x1B1A1Ad5;
    data_array[2] = 0x0201001B;
    data_array[3] = 0x06050403;
    data_array[4] = 0x0A090807;
    data_array[5] = 0x1825240B;
    data_array[6] = 0x18272618;
    data_array[7] = 0x18181818;
    data_array[8] = 0x18181818;
    data_array[9] = 0x18181818;
    data_array[10] = 0x20181818;
    data_array[11] = 0x18181821;
    data_array[12] = 0x00000018;
    dsi_set_cmdq(data_array, 13, 1);
    MDELAY(5);
   data_array[0] = 0x002d3902;
    data_array[1] = 0x1B1A1Ad6;
    data_array[2] = 0x090A0B1B;
    data_array[3] = 0x05060708;
    data_array[4] = 0x01020304;
    data_array[5] = 0x18202100;
    data_array[6] = 0x18262718;
    data_array[7] = 0x18181818;
    data_array[8] = 0x18181818;
    data_array[9] = 0x18181818;
    data_array[10] = 0x25181818;
    data_array[11] = 0x18181824;
    data_array[12] = 0x00000018;
    dsi_set_cmdq(data_array, 13, 1);
    MDELAY(5);
/*
    data_array[0] = 0x003B3902;
    data_array[1] = 0x190C00e0;
    data_array[2] = 0x29262320;
    data_array[3] = 0x70615128;
    data_array[4] = 0x8986766F;
    data_array[5] = 0x959A998D;
    data_array[6] = 0x5557B0A1;
    data_array[7] = 0x645E5C58;
    data_array[8] = 0x0C007F6B;
    data_array[9] = 0x26232018;
    data_array[10] = 0x61512829;
    data_array[11] = 0x86766F70;
    data_array[12] = 0x9A998D89;
    data_array[13] = 0x57B0A195;
    data_array[14] = 0x5E5C5855;
    data_array[15] = 0x007F6B64;
    dsi_set_cmdq(data_array, 16, 1);
    MDELAY(5);
 */  
    data_array[0] = 0x003B3902; 
    data_array[1] = 0x1B0D00e0; 
    data_array[2] = 0x2F2B2723; 
    data_array[3] = 0x806F5E2E; 
    data_array[4] = 0x9C98867F; 
    data_array[5] = 0xA9AEAA9E; 
    data_array[6] = 0x5F61C6B8; 
    data_array[7] = 0x6C666563; 
    data_array[8] = 0x0D00FF7F; 
    data_array[9] = 0x2B27231B; 
    data_array[10] = 0x6F5E2E2F; 
    data_array[11] = 0x98867F80; 
    data_array[12] = 0xAEAA9E9C; 
    data_array[13] = 0x61C6B8A9; 
    data_array[14] = 0x6665635F; 
    data_array[15] = 0x00FF7F6C; 
    dsi_set_cmdq(data_array, 16, 1); 
    MDELAY(5); 
      
    data_array[0] = 0x00033902;
    data_array[1] = 0x00731Fc0;
    dsi_set_cmdq(data_array, 2, 1);
    MDELAY(5);    
      
    data_array[0] = 0x00023902;
    data_array[1] = 0x000003cc; //0b
    dsi_set_cmdq(data_array, 2, 1);
    MDELAY(5);
    data_array[0] = 0x00023902;
    data_array[1] = 0x000002D4;
    dsi_set_cmdq(data_array, 2, 1);
    MDELAY(5);

	data_array[0] = 0x00023902;
	data_array[1] = 0x000002BD;
	dsi_set_cmdq(data_array, 2, 1);
	MDELAY(5);
	data_array[0] = 0x000D3902;
	data_array[1] = 0xFFFFFFD8;
	data_array[2] = 0xFFFFFFFF;
	data_array[3] = 0xFFFFFFFF;
	data_array[4] = 0x000000FF;
	dsi_set_cmdq(data_array, 5, 1);
	MDELAY(5);
	data_array[0] = 0x00023902;
	data_array[1] = 0x000000BD;
	dsi_set_cmdq(data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x000001BD;
dsi_set_cmdq(data_array, 2, 1);


data_array[0] = 0x00023902;
data_array[1] = 0x000060B1;
dsi_set_cmdq(data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x000000BD;
dsi_set_cmdq(data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x000002D4;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(5);

data_array[0] = 0x00083902;
data_array[1] = 0x508140BF;
data_array[2] = 0x01FC1A00;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(5);

    data_array[0] = 0x00110500;
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(120);

   
    
    data_array[0] = 0x00290500;
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(20);
    ///////////////////////////
};
#endif

static void lcm_init(void)
{
    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(20);
    SET_RESET_PIN(1);
    MDELAY(120);

    //lcm_init_resgister();
   // dsi_set_cmdq_V3(lcm_initialization_setting_V3, sizeof(lcm_initialization_setting_V3) / sizeof(LCM_setting_table_V3), 1);
  //	dsi_set_cmdq_V3(lcm_initialization_setting,sizeof(lcm_initialization_setting)/sizeof(lcm_initialization_setting[0]),1);
    push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_suspend(void)
{
#ifndef BUILD_LK
	SET_RESET_PIN(1);  //NOTE:should reset LCM firstly
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(30);
	SET_RESET_PIN(1);
	MDELAY(120);
	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);//wqtao. enable
#endif
}


static void lcm_resume(void)
{
#ifndef BUILD_LK
//        lcm_compare_id();
	lcm_init();
#endif
}



	
static unsigned int lcm_compare_id(void)
{
	unsigned int id=0;
	unsigned char buffer[2];
	unsigned int array[16];  

	SET_RESET_PIN(1);
	MDELAY(1);
	SET_RESET_PIN(0);
	MDELAY(1);
	SET_RESET_PIN(1);
	MDELAY(120);//Must over 6 ms

	array[0]=0x00043902;
	array[1]=0x9483FFB9;// page enable
	dsi_set_cmdq(array, 2, 1);
	MDELAY(10);

	array[0]=0x00073902; 
	array[1]=0x680362BA;// page enable //9341
	array[2]=0x00c0b26b; 
	dsi_set_cmdq(array, 3, 1); 
	MDELAY(10); 
//{0xBA,6,{0x62,0x03,0x68,0x6B,0xB2,0xC0}},
	array[0] = 0x00023700;// return byte number
	dsi_set_cmdq(array, 1, 1);
	MDELAY(10);

	read_reg_v2(0xdc, buffer, 2);
	//id = buffer[0]; 
        id = buffer[0];      // 0x0f
#ifdef BUILD_LK
	printf("[HX8394F]%s,  id = 0x%x\n", __func__, id);
#else
	printk("[HX8394F]%s,  id = 0x%x\n", __func__, id);
#endif

    return (LCM_ID == id)?1:0;
}

// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER hct_hx8394f_dsi_vdo_hd_cmi = 
{
	.name			  = "hct_hx8394f_dsi_vdo_hd_cmi",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,	
	.compare_id     = lcm_compare_id,	
//	.esd_check   = lcm_esd_check,	
//	.esd_recover   = lcm_esd_recover,	
//	.update         = lcm_update,
};

