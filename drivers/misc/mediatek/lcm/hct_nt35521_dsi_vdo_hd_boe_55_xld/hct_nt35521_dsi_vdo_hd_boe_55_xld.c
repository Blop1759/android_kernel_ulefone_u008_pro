
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

#define FRAME_WIDTH                                          (720)
#define FRAME_HEIGHT                                         (1280)

#define REGFLAG_DELAY                                         0XFE
#define REGFLAG_END_OF_TABLE                                  0xFF   // END OF REGISTERS MARKER

#define LCM_ID_NT35521  0x5521
#define LCM_DSI_CMD_MODE                                    0

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)    lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)        lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)                                    lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)                lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg                                            lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)                   lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size) 

struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};

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
		params->dbi.te_mode 				= LCM_DBI_TE_MODE_DISABLED;
		//params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

		params->dsi.mode   = SYNC_PULSE_VDO_MODE; //SYNC_PULSE_VDO_MODE;//BURST_VDO_MODE;

		// DSI
		/* Command mode setting */
		//1 Three lane or Four lane
		params->dsi.LANE_NUM				= LCM_FOUR_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		// Highly depends on LCD driver capability.
		// Not support in MT6573
		params->dsi.packet_size=256;

		// Video mode setting
		params->dsi.intermediat_buffer_num = 0;//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage

		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
		params->dsi.word_count=720*3;


		params->dsi.vertical_sync_active				= 8;// 3    2
		params->dsi.vertical_backporch					= 20;// 20   1
		params->dsi.vertical_frontporch					= 20; // 1  12
		params->dsi.vertical_active_line				= FRAME_HEIGHT;

		params->dsi.horizontal_sync_active				= 10;// 50  2
		params->dsi.horizontal_backporch				= 110;
		params->dsi.horizontal_frontporch				= 110;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

            params->dsi.clk_lp_per_line_enable = 1;  
            params->dsi.esd_check_enable = 1;
            params->dsi.customization_esd_check_enable = 1;   
            params->dsi.lcm_esd_check_table[0].cmd          = 0x0A;    
            params->dsi.lcm_esd_check_table[0].count        = 1;     
            params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9c;
            params->dsi.noncont_clock = 1;   
            params->dsi.noncont_clock_period = 2;
            params->dsi.ssc_disable = 1;
	    //params->dsi.LPX=8;

		// Bit rate calculation
		//1 Every lane speed
        	//params->dsi.pll_select=1;
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
#endif //this value must be in MTK suggested table
    params->dsi.compatibility_for_nvk = 1;
}

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

static struct LCM_setting_table lcm_initialization_setting[] = {

{0xFF,4,{0xAA,0x55,0x25,0x01}},
	{0xFC,1,{0x08}},
	{REGFLAG_DELAY, 5, {}},
	{0xFC,1,{0x00}},
        
	{REGFLAG_DELAY, 5, {}},
        
	{0x6F,1,{0x21}},
	{0xF7,1,{0x01}},
	{REGFLAG_DELAY, 5, {}},
	{0x6F,1,{0x21}},
	{0xF7,1,{0x00}},
	{REGFLAG_DELAY, 5, {}},
              
	{0x6F,1,{0x1A}},
	{0xF7,1,{0x05}},
	{REGFLAG_DELAY, 5, {}},
       
	     
	{0xFF,4,{0xAA,0x55,0x25,0x00}},
   
	//Page   0
	{0xF0,4,{0x55,0xAA,0x52,0x08,0x00}},
	{0xBD,4,{0x01,0xA0,0x0c,0x08,0x01}},
	{0x6f,1,{0x02}},
	{0xB8,1,{0x01}},
	    
	{0xBB,2,{0x11,0x11}},
	{0xBC,2,{0x00,0x00}},
	{0xB1,2,{0x68,0x25}},  //NEW ADD
	{0xB6,2,{0x06}},
         
         
	//Page  1
	{0xF0,5,{0x55,0xAA,0x52,0x08,0x01}},
	{0xB0,2,{0x09,0x09}},
	{0xB1,2,{0x09,0x09}},
       
	{0xBC,2,{0x78,0x00}},
	{0xBD,2,{0x78,0x00}},
       
	{0xCA,1,{0x00}},
	{0xC0,1,{0x04}},
	{0xB5,2,{0x03,0x03 }},
	{0xBE,1,{0x6E}},
	{0xB3,2,{0x19,0x19}},
	{0xB4,2,{0x19,0x19}},
	{0xB9,2,{0x26,0x26}},
	{0xBA,2,{0x24,0x24}},
       
  {0xF0,5,{0x55,0xAA,0x52,0x08,0x02}},
	{0xEE,1,{0x01}},
       
	     
  {0xB0,16,{0x00,0x05,0x00,0x2C,0x00,0x57,0x00,0x7C,0x00,0x95,0x00,0xBB,0x00,0xDD,0x01,0x0E}},
  {0xB1,16,{0x01,0x33,0x01,0x6C,0x01,0x98,0x01,0xE1,0x02,0x19,0x02,0x1A,0x02,0x4F,0x02,0x88}},
  {0xB2,16,{0x02,0xAA,0x02,0xD8,0x02,0xF7,0x03,0x21,0x03,0x3E,0x03,0x66,0x03,0x80,0x03,0xA1}},
  {0xB3,4,{0x03,0xD6,0x03,0xE4}},


	//Page 6
	{ 0xF0,5,{0x55,0xAA,0x52,0x08,0x06}},
	{ 0xB0,2,{0x10,0x12}},
	{ 0xB1,2,{0x14,0x16}},
	{ 0xB2,2,{0x00,0x02}},
	{ 0xB3,2,{0x31,0x31}},
	{ 0xB4,2,{0x31,0x34}},
	{ 0xB5,2,{0x34,0x34}},
	{ 0xB6,2,{0x34,0x31}},
	{ 0xB7,2,{0x31,0x31}},
	{ 0xB8,2,{0x31,0x31}},
	{ 0xB9,2,{0x2D,0x2E}},
	{ 0xBA,2,{0x2E,0x2D}},
	{ 0xBB,2,{0x31,0x31}},
	{ 0xBC,2,{0x31,0x31}},
	{ 0xBD,2,{0x31,0x34}},
	{ 0xBE,2,{0x34,0x34}},
	{ 0xBF,2,{0x34,0x31}},
	{ 0xC0,2,{0x31,0x31}},
	{ 0xC1,2,{0x03,0x01}},
	{ 0xC2,2,{0x17,0x15}},
	{ 0xC3,2,{0x13,0x11}},
	{ 0xE5,2,{0x31,0x31}},
	{ 0xC4,2,{0x17,0x15}},
	{ 0xC5,2,{0x13,0x11}},
	{ 0xC6,2,{0x03,0x01}},
	{ 0xC7,2,{0x31,0x31}},
	{ 0xC8,2,{0x31,0x34}},
	{ 0xC9,2,{0x34,0x34}},
	{ 0xCA,2,{0x34,0x31}},
	{ 0xCB,2,{0x31,0x31}},
	{ 0xCC,2,{0x31,0x31}},
	{ 0xCD,2,{0x2E,0x2D}},
	{ 0xCE,2,{0x2D,0x2E}},
	{ 0xCF,2,{0x31,0x31}},
	{ 0xD0,2,{0x31,0x31}},
	{ 0xD1,2,{0x31,0x34}},
	{ 0xD2,2,{0x34,0x34}},
	{ 0xD3,2,{0x34,0x31}},
	{ 0xD4,2,{0x31,0x31}},
	{ 0xD5,2,{0x00,0x02}},
	{ 0xD6,2,{0x10,0x12}},
	{ 0xD7,2,{0x14,0x16}},
	{ 0xE6,2,{0x32,0x32}},
	{ 0xD8,5,{0x00,0x00,0x00,0x00,0x00}},
	{ 0xD9,5,{0x00,0x00,0x00,0x00,0x00}},
	{ 0xE7,1,{0x00}},
         
	//Page 
	{ 0xF0,5,{0x55,0xAA,0x52,0x08,0x05}},
	{ 0xED,1,{0x30}},
	{ 0xB0,2,{0x17,0x06}},
	{ 0xB8,1,{0x00}},
	{ 0xC0,1,{0x0D}},
	{ 0xC1,1,{0x0B}},
	{ 0xC2,1,{0x23}},
	{ 0xC3,1,{0x40}},
	{ 0xC4,1,{0x84}},
	{ 0xC5,1,{0x82}},
	{ 0xC6,1,{0x82}},
	{ 0xC7,1,{0x80}},
	{ 0xC8,2,{0x0B,0x30}},
	{ 0xC9,2,{0x05,0x10}},
	{ 0xCA,2,{0x01,0x10}},
	{ 0xCB,2,{0x01,0x10}},
	{ 0xD1,5,{0x03,0x05,0x05,0x07,0x00}},
	{ 0xD2,5,{0x03,0x05,0x09,0x03,0x00}},
	{ 0xD3,5,{0x00,0x00,0x6A,0x07,0x10}},
	{ 0xD4,5,{0x30,0x00,0x6A,0x07,0x10}},
         
	//Page 
	{ 0xF0,5,{0x55,0xAA,0x52,0x08,0x03}},
	{ 0xB0,2,{0x00,0x00}},
	{ 0xB1,2,{0x00,0x00}},
	{ 0xB2,5,{0x05,0x00,0x0A,0x00,0x00}},
	{ 0xB3,5,{0x05,0x00,0x0A,0x00,0x00}},
	{ 0xB4,5,{0x05,0x00,0x0A,0x00,0x00}},
	{ 0xB5,5,{0x05,0x00,0x0A,0x00,0x00}},
	{ 0xB6,5,{0x02,0x00,0x0A,0x00,0x00}},
	{ 0xB7,5,{0x02,0x00,0x0A,0x00,0x00}},
	{ 0xB8,5,{0x02,0x00,0x0A,0x00,0x00}},
	{ 0xB9,5,{0x02,0x00,0x0A,0x00,0x00}},
	{ 0xBA,5,{0x53,0x00,0x0A,0x00,0x00}},
	{ 0xBB,5,{0x53,0x00,0x0A,0x00,0x00}},
	{ 0xBC,5,{0x53,0x00,0x0A,0x00,0x00}},
	{ 0xBD,5,{0x53,0x00,0x0A,0x00,0x00}},
	{ 0xC4,1,{0x60}},
	{ 0xC5,1,{0x40}},
	{ 0xC6,1,{0x64}},
	{ 0xC7,1,{0x44}},
{0x11, 1, {0x00}},
{REGFLAG_DELAY, 150, {}},
{0x29, 1, {0x00}},
{REGFLAG_DELAY, 50, {}},

{REGFLAG_END_OF_TABLE, 0x00, {}}
	// Note
	// Strongly recommend not to set Sleep out / Display On here. That will cause messed frame to be shown as later the backlight is on.


	// Setting ending by predefined flag
};

static void lcm_init(void)
{
    
    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(20);
    SET_RESET_PIN(1);
    MDELAY(50);
   // lcm_initialization();
    push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_suspend(void)
{
    unsigned int data_array[16];
    data_array[0]=0x00280500;
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(10);
    data_array[0]=0x00100500;
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(100);
}


static void lcm_resume(void)
{/*
    unsigned int data_array[16];
    data_array[0]=0x00110500;
    dsi_set_cmdq(data_array,1,1);
    MDELAY(100);
    data_array[0]=0x00290500;
    dsi_set_cmdq(data_array,1,1);
    MDELAY(10);
*/
   lcm_init();
}


static unsigned int lcm_compare_id(void)
{
    unsigned int id=0;
    unsigned char buffer[3];
    unsigned int array[16]; 
    unsigned int data_array[16];

    SET_RESET_PIN(1);
        MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(50);
   
    SET_RESET_PIN(1);
    MDELAY(120);

    data_array[0] = 0x00063902;
    data_array[1] = 0x52AA55F0; 
    data_array[2] = 0x00000108;               
    dsi_set_cmdq(data_array, 3, 1);

    array[0] = 0x00033700;// read id return two byte,version and id
    dsi_set_cmdq(array, 1, 1);
   
    read_reg_v2(0xC5, buffer, 3);
    id = buffer[1]; //we only need ID
    #ifdef BUILD_LK
        printf("%s, LK nt35590 debug: nt35590 id = 0x%08x buffer[0]=0x%08x,buffer[1]=0x%08x,buffer[2]=0x%08x\n", __func__, id,buffer[0],buffer[1],buffer[2]);
    #else
        printk("%s, LK nt35590 debug: nt35590 id = 0x%08x buffer[0]=0x%08x,buffer[1]=0x%08x,buffer[2]=0x%08x\n", __func__, id,buffer[0],buffer[1],buffer[2]);
    #endif

   // if(id == LCM_ID_NT35521)
    if(buffer[0]==0x55 && buffer[1]==0x21)
        return 1;
    else
        return 0;


}









// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER hct_nt35521_dsi_vdo_hd_boe_55_xld= 
{
    .name			= "hct_nt35521_dsi_vdo_hd_boe_55_xld",
    .set_util_funcs = lcm_set_util_funcs,
    .get_params     = lcm_get_params,
    .init           = lcm_init,
    .suspend        = lcm_suspend,
    .resume         = lcm_resume,
    .compare_id    = lcm_compare_id,
#if 0//defined(LCM_DSI_CMD_MODE)
//    .set_backlight	= lcm_setbacklight,
    //.set_pwm        = lcm_setpwm,
    //.get_pwm        = lcm_getpwm,
    .update         = lcm_update
#endif
};

