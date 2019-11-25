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

#define LCM_ID                      								(0x94)

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


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg                                            lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

 struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};

static struct LCM_setting_table lcm_initialization_setting[] = {
	// AUO5.46	
	// Set EXTC 
	{0xB9, 3, {0xFF,0x83,0x94}},
	// Set MIPI 
{0xBA,6,{0x63,0x03,0x68,0x6B,0xB2,0xC0}},
{0xB1,10,{0x48,0x12,0x72,0x09,0x32,0x44,0x71,0x31,0x4d,0x2f}},
{0xB2,5,{0x00,0x80,0x64,0x05,0x07}},
{0xB4,21,{0x26,0x76,0x26,0x76,0x26,0x26,0x05,0x10,0x86,0x75,
          0x00,0x3F,0x26,0x76,0x26,0x76,0x26,0x26,0x05,0x10,
          0x86,}},
{0xD3,33,{0x00,0x00,0x06,0x06,0x01,0x01,0x10,0x10,0x32,0x10,
          0x00,0x00,0x00,0x32,0x15,0x04,0x05,0x04,0x32,0x15,
          0x14,0x05,0x14,0x37,0x33,0x04,0x04,0x37,0x00,0x00,
          0x47,0x05,0x40}},
{0xD5,44,{0x18,0x18,0x25,0x24,0x27,0x26,0x11,0x10,0x15,0x14,
          0x13,0x12,0x17,0x16,0x01,0x00,0x18,0x18,0x18,0x18,
          0x18,0x18,0x18,0x18,0x18,0x18,0x05,0x04,0x03,0x02,
          0x07,0x06,0x18,0x18,0x18,0x18,0x21,0x20,0x23,0x22,
          0x18,0x18,0x18,0x18}},
{0xD6,44,{0x18,0x18,0x22,0x23,0x20,0x21,0x12,0x13,0x16,0x17,
          0x10,0x11,0x14,0x15,0x06,0x07,0x18,0x18,0x18,0x18,
          0x18,0x18,0x18,0x18,0x18,0x18,0x02,0x03,0x04,0x05,
          0x00,0x01,0x18,0x18,0x18,0x18,0x26,0x27,0x24,0x25,
          0x18,0x18,0x18,0x18}},
//Gamma2.5 调试
{0xE0,58,{0x00,0x08,0x11,0x18,0x1c,0x22,0x27,0x28,0x54,0x64,
          0x77,0x7b,0x88,0x9d,0xa3,0xa9,0xb7,0xbb,0xb6,0xc3,
          0xce,0x67,0x65,0x66,0x6a,0x6e,0x76,0x7a,0x7F,
          0x00,0x08,0x11,0x18,0x1c,0x22,0x27,0x28,0x54,0x64,
          0x77,0x7b,0x88,0x9d,0xa3,0xa9,0xb7,0xbb,0xb6,0xc3,
          0xce,0x67,0x65,0x66,0x6a,0x6e,0x76,0x7a,0x7F,}},
{0xB6,2,{0x3f,0x3f}},
{0xCC,1,{0x0b}},
{0xC0,2,{0x1F,0x31}},
{0xD4,1,{0x02}},
{0xBD,1,{0x01}},
{0xB1,1,{0x60}},
{0xBD,1,{0x00}},
{0xBF,7,{0x40,0x81,0x50,0x00,0x1A,0xFC,0x01}},
	{0x11,	0,	{0x00}},	//Sleep Out 
	{REGFLAG_DELAY, 200, {0}}, 
	{0x29,	0,	{0x00}}, //Display On					  
	{REGFLAG_DELAY, 20, {0}},

	//{0x21,	0,	{0x00}},
};





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
	params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

	params->dsi.mode   = SYNC_PULSE_VDO_MODE;	//SYNC_PULSE_VDO_MODE;

	// DSI
	/* Command mode setting */
	params->dsi.LANE_NUM			= LCM_FOUR_LANE;
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
        params->dsi.word_count=720 * 3;
	params->dsi.vertical_sync_active				= 2;//5
	params->dsi.vertical_backporch					= 5;
	params->dsi.vertical_frontporch					= 9;
	params->dsi.vertical_active_line				= FRAME_HEIGHT; 
	params->dsi.horizontal_sync_active				= 85;//38//15
	params->dsi.horizontal_backporch				= 85;//38//70
	params->dsi.horizontal_frontporch				= 85;//38//65
	//params->dsi.horizontal_blanking_pixel		       		= 60;
	params->dsi.horizontal_active_pixel		       		= FRAME_WIDTH;
	// Bit rate calculation

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
          params->dsi.esd_check_enable = 0;
          params->dsi.customization_esd_check_enable = 0;
          params->dsi.cont_clock=0; //1;
          params->dsi.lcm_esd_check_table[0].cmd = 0x09;//09 ,45,d9
          params->dsi.lcm_esd_check_table[0].count = 3;
          params->dsi.lcm_esd_check_table[0].para_list[0] = 0x80;
          params->dsi.lcm_esd_check_table[0].para_list[1] = 0x73;
          params->dsi.lcm_esd_check_table[0].para_list[2] = 0x04;

          params->dsi.lcm_esd_check_table[1].cmd = 0xd9;
          params->dsi.lcm_esd_check_table[1].count = 1;
          params->dsi.lcm_esd_check_table[1].para_list[0] = 0x80;

          params->dsi.lcm_esd_check_table[2].cmd = 0x45;
          params->dsi.lcm_esd_check_table[2].count = 2;
          params->dsi.lcm_esd_check_table[2].para_list[0] = 0x05;
          params->dsi.lcm_esd_check_table[2].para_list[1] = 0x0e;
}

static void lcm_init(void)
{
    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(120);

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
        SET_RESET_PIN(1);
        MDELAY(10);
  	 	SET_RESET_PIN(0);
    	MDELAY(50);
}

static unsigned int lcm_compare_id(void);

static void lcm_resume(void)
{
	lcm_init();
}



#if 0
extern int IMM_GetOneChannelValue(int dwChannel, int data[4], int* rawdata);
  #define AUX_IN0_LCD_ID  12
  #define ADC_MIN_VALUE   0x800
#endif
  //0                             
//47 / 147 * 4096 = 0x51D               0.575V
  //100/ 147 * 4096 = 0xAE2               1.22V
	
static unsigned int lcm_compare_id(void)
{
	unsigned int id1 = 0;
	unsigned int id2 = 0;
	unsigned char buffer[2];
	unsigned int array[16];  
       //  int adcdata[4] = {0};
       //  int rawdata = 0;
        //  int ret = 0;

    //      ret = IMM_GetOneChannelValue(AUX_IN0_LCD_ID, adcdata, &rawdata);
// #if defined(BUILD_LK)
//         printf("hct_8394-auo adc = %x adcdata= %x %x, ret=%d, ADC_MIN_VALUE=%x\r\n",rawdata, adcdata[0], adcdata[1],ret, ADC_MIN_VALUE);
//  #else
//          printk("hct_8394-auo adc = %x adcdata= %x %x, ret=%d, ADC_MIN_VALUE=%x\r\n",rawdata, adcdata[0], adcdata[1],ret, ADC_MIN_VALUE);
//  #endif

	if (1)//rawdata<0x600) //0x63d

       {

	SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(120);//Must over 6 ms

	array[0] = 0x00043902;
    	array[1] = 0x9483ffb9;
    	dsi_set_cmdq(array, 2, 1);
	MDELAY(10); 

	array[0] = 0x00023700;// return byte number
	dsi_set_cmdq(array, 1, 1);
	MDELAY(10);

	read_reg_v2(0xF4, buffer, 2);
	id1 = buffer[0]; //we only need ID
	id2 = buffer[1]; 

#ifdef BUILD_LK
	printf("[HX8394D]%s,  id = 0x%x\n", __func__, id1);
#else
	printk("[HX8394D]%s,  id = 0x%x\n", __func__, id1);
#endif

    return (LCM_ID == id1)?1:0;
      return 1;
	}
	else
	{
	   return 0;	
	}

}

// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER hct_hx8394f_dsi_vdo_hd_auo_55_np = 
{
	.name			  = "hct_hx8394f_dsi_vdo_hd_auo_55_np",
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

