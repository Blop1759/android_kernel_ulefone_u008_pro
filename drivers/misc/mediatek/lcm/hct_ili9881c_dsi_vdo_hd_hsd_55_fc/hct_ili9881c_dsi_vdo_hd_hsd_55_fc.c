/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

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

#define FRAME_WIDTH                                         (720)
#define FRAME_HEIGHT                                        (1280)
#define LCM_ID                       (0x9881)

#define REGFLAG_DELAY               (0XFE)
#define REGFLAG_END_OF_TABLE        (0x100) // END OF REGISTERS MARKER


#define LCM_DSI_CMD_MODE                                    0



// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util;

#define SET_RESET_PIN(v)                                    (lcm_util.set_reset_pin((v)))

#define UDELAY(n)                                           (lcm_util.udelay(n))
#define MDELAY(n)                                           (lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------
#define dsi_set_cmdq_V3(para_tbl,size,force_update)        lcm_util.dsi_set_cmdq_V3(para_tbl,size,force_update)
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)    lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)       lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)                                      lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)                  lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)               lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

#define   LCM_DSI_CMD_MODE							0
 struct LCM_setting_table {
    unsigned char cmd;
    unsigned char count;
    unsigned char para_list[64];
};


static struct LCM_setting_table lcm_initialization_setting[] = {
    
    /*
    Note :

    Data ID will depends on the following rule.
    
        count of parameters > 1 => Data ID = 0x39
        count of parameters = 1 => Data ID = 0x15
        count of parameters = 0 => Data ID = 0x05

    Structure Format :

    {DCS command, count of parameters, {parameter list}}
    {REGFLAG_DELAY, milliseconds of time, {)},

    ...

    Setting ending by predefined flag
    
    {REGFLAG_END_OF_TABLE, 0x00, {}}
    */


{0xFF,0x03,{0x98,0x81,0x03}},
{0x01,0x01,{0x00}},
{0x02,0x01,{0x00}},
{0x03,0x01,{0x72}},
{0x04,0x01,{0x00}},
{0x05,0x01,{0x00}},
{0x06,0x01,{0x09}},
{0x07,0x01,{0x00}},
{0x08,0x01,{0x00}},
{0x09,0x01,{0x01}},
{0x0a,0x01,{0x00}},
{0x0b,0x01,{0x00}},
{0x0c,0x01,{0x01}},
{0x0d,0x01,{0x00}},
{0x0e,0x01,{0x00}},
{0x0f,0x01,{0x00}},
{0x10,0x01,{0x00}},
{0x11,0x01,{0x00}},
{0x12,0x01,{0x00}},
{0x13,0x01,{0x00}},
{0x14,0x01,{0x00}},
{0x15,0x01,{0x00}},
{0x16,0x01,{0x00}},
{0x17,0x01,{0x00}},
{0x18,0x01,{0x00}},
{0x19,0x01,{0x00}},
{0x1a,0x01,{0x00}},
{0x1b,0x01,{0x00}},
{0x1c,0x01,{0x00}},
{0x1d,0x01,{0x00}},
{0x1e,0x01,{0x40}},
{0x1f,0x01,{0x80}},
{0x20,0x01,{0x05}},
{0x21,0x01,{0x02}},
{0x22,0x01,{0x00}},
{0x23,0x01,{0x00}},
{0x24,0x01,{0x00}},
{0x25,0x01,{0x00}},
{0x26,0x01,{0x00}},
{0x27,0x01,{0x00}},
{0x28,0x01,{0x33}},
{0x29,0x01,{0x02}},
{0x2a,0x01,{0x00}},
{0x2b,0x01,{0x00}},
{0x2c,0x01,{0x00}},
{0x2d,0x01,{0x00}},
{0x2e,0x01,{0x00}},
{0x2f,0x01,{0x00}},
{0x30,0x01,{0x00}},
{0x31,0x01,{0x00}},
{0x32,0x01,{0x00}},
{0x33,0x01,{0x00}},
{0x34,0x01,{0x04}},
{0x35,0x01,{0x00}},
{0x36,0x01,{0x00}},
{0x37,0x01,{0x00}},
{0x38,0x01,{0x3C}},
{0x39,0x01,{0x00}},
{0x3a,0x01,{0x40}},
{0x3b,0x01,{0x40}},
{0x3c,0x01,{0x00}},
{0x3d,0x01,{0x00}},
{0x3e,0x01,{0x00}},
{0x3f,0x01,{0x00}},
{0x40,0x01,{0x00}},
{0x41,0x01,{0x00}},
{0x42,0x01,{0x00}},
{0x43,0x01,{0x00}},
{0x44,0x01,{0x00}},
{0x50,0x01,{0x01}},
{0x51,0x01,{0x23}},
{0x52,0x01,{0x45}},
{0x53,0x01,{0x67}},
{0x54,0x01,{0x89}},
{0x55,0x01,{0xab}},
{0x56,0x01,{0x01}},
{0x57,0x01,{0x23}},
{0x58,0x01,{0x45}},
{0x59,0x01,{0x67}},
{0x5a,0x01,{0x89}},
{0x5b,0x01,{0xab}},
{0x5c,0x01,{0xcd}},
{0x5d,0x01,{0xef}},
{0x5e,0x01,{0x11}},
{0x5f,0x01,{0x01}},
{0x60,0x01,{0x00}},
{0x61,0x01,{0x15}},
{0x62,0x01,{0x14}},
{0x63,0x01,{0x0E}},
{0x64,0x01,{0x0F}},
{0x65,0x01,{0x0C}},
{0x66,0x01,{0x0D}},
{0x67,0x01,{0x06}},
{0x68,0x01,{0x02}},
{0x69,0x01,{0x02}},
{0x6a,0x01,{0x02}},
{0x6b,0x01,{0x02}},
{0x6c,0x01,{0x02}},
{0x6d,0x01,{0x02}},
{0x6e,0x01,{0x07}},
{0x6f,0x01,{0x02}},
{0x70,0x01,{0x02}},
{0x71,0x01,{0x02}},
{0x72,0x01,{0x02}},
{0x73,0x01,{0x02}},
{0x74,0x01,{0x02}},
{0x75,0x01,{0x01}},
{0x76,0x01,{0x00}},
{0x77,0x01,{0x14}},
{0x78,0x01,{0x15}},
{0x79,0x01,{0x0E}},
{0x7a,0x01,{0x0F}},
{0x7b,0x01,{0x0C}},
{0x7c,0x01,{0x0D}},
{0x7D,0x01,{0x06}},
{0x7E,0x01,{0x02}},
{0x7F,0x01,{0x02}},
{0x80,0x01,{0x02}},
{0x81,0x01,{0x02}},
{0x82,0x01,{0x02}},
{0x83,0x01,{0x02}},
{0x84,0x01,{0x07}},
{0x85,0x01,{0x02}},
{0x86,0x01,{0x02}},
{0x87,0x01,{0x02}},
{0x88,0x01,{0x02}},
{0x89,0x01,{0x02}},
{0x8A,0x01,{0x02}},
{0xFF,0x03,{0x98,0x81,0x04}},
{0x6C,0x01,{0x15}},
{0x6E,0x01,{0x2A}},
{0x6F,0x01,{0x33}},
{0x3A,0x01,{0x94}},
{0x8D,0x01,{0x1A}},
{0x87,0x01,{0xBA}},
{0x26,0x01,{0x76}},
{0xB2,0x01,{0xD1}},
{0xB5,0x01,{0x06}},
{0xFF,0x03,{0x98,0x81,0x01}},
{0x22,0x01,{0x0A}},
{0x31,0x01,{0x00}},
{0x53,0x01,{0x84}},
{0x55,0x01,{0x8F}},
{0x50,0x01,{0xAE}},
{0x51,0x01,{0xAE}},
{0x60,0x01,{0x28}},
{0xA0,0x01,{0x0F}},
{0xA1,0x01,{0x1B}},
{0xA2,0x01,{0x28}},
{0xA3,0x01,{0x12}},
{0xA4,0x01,{0x15}},
{0xA5,0x01,{0x28}},
{0xA6,0x01,{0x1B}},
{0xA7,0x01,{0x1E}},
{0xA8,0x01,{0x79}},
{0xA9,0x01,{0x1B}},
{0xAA,0x01,{0x27}},
{0xAB,0x01,{0x69}},
{0xAC,0x01,{0x19}},
{0xAD,0x01,{0x18}},
{0xAE,0x01,{0x4C}},
{0xAF,0x01,{0x21}},
{0xB0,0x01,{0x28}},
{0xB1,0x01,{0x52}},
{0xB2,0x01,{0x65}},
{0xB3,0x01,{0x3F}},
{0xC0,0x01,{0x04}},
{0xC1,0x01,{0x1B}},
{0xC2,0x01,{0x27}},
{0xC3,0x01,{0x13}},
{0xC4,0x01,{0x15}},
{0xC5,0x01,{0x28}},
{0xC6,0x01,{0x1C}},
{0xC7,0x01,{0x1E}},
{0xC8,0x01,{0x79}},
{0xC9,0x01,{0x1A}},
{0xCA,0x01,{0x27}},
{0xCB,0x01,{0x69}},
{0xCC,0x01,{0x1A}},
{0xCD,0x01,{0x18}},
{0xCE,0x01,{0x4C}},
{0xCF,0x01,{0x21}},
{0xD0,0x01,{0x27}},
{0xD1,0x01,{0x52}},
{0xD2,0x01,{0x65}},
{0xD3,0x01,{0x3F}},
{0xFF,0x03,{0x98,0x81,0x00}},
{0x35,0x01,{0x00}},

{0x11,0x01,{0x00}}, // Sleep Out
{REGFLAG_DELAY,120,{}},                      
{0x29,0x01,{0x00}}, //Display on 
{REGFLAG_DELAY,20,{}},

};






//static int vcom=0x40;
static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
    unsigned int i;

    for(i = 0; i < count; i++) {
        
        unsigned cmd;
        cmd = table[i].cmd;
        
        switch (cmd) {
			/*case 0xd9:
			table[i].para_list[0]=vcom;
			dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
            vcom+=2;
			break;
			*/
            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;
                
            case REGFLAG_END_OF_TABLE :
                break;
                
            default:
                dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
                MDELAY(1);
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
    params->dbi.te_mode             = LCM_DBI_TE_MODE_DISABLED;
    params->dbi.te_edge_polarity        = LCM_POLARITY_RISING;



    params->dsi.mode   = SYNC_EVENT_VDO_MODE;


    // DSI
    /* Command mode setting */
    params->dsi.LANE_NUM                = LCM_FOUR_LANE;
    //The following defined the fomat for data coming from LCD engine. 
    params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
    params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST; 
    params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
    params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;
    // Highly depends on LCD driver capability.
    // Not support in MT6573
    params->dsi.packet_size=256;
    // Video mode setting       
    params->dsi.intermediat_buffer_num = 2;
    params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
        params->physical_width = 62;
     	params->physical_height = 110;
	params->dsi.vertical_sync_active				= 6; //8;	//2;
	params->dsi.vertical_backporch					= 15; //18;	//14;
	params->dsi.vertical_frontporch					= 19; //20;	//16;
	params->dsi.vertical_active_line				= FRAME_HEIGHT; 

	params->dsi.horizontal_sync_active				= 40;	//2;
	params->dsi.horizontal_backporch				= 120;//120;	//60;	//42;
	params->dsi.horizontal_frontporch				= 80;//100;	//60;	//44;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

// Bit rate calculation
//1 Every lane speed
//params->dsi.pll_div1=0;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
//params->dsi.pll_div2=0;		// div2=0,1,2,3;div1_real=1,2,4,4	
//params->dsi.fbk_div =0x12;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)	

// zhangxiaofei add for test
params->dsi.PLL_CLOCK = 230;//208;
params->dsi.ssc_disable=1; 	
    params->dsi.clk_lp_per_line_enable = 1; // 1 clk line by line return LP11 0 clk frame by frame return LP11 
    params->dsi.esd_check_enable = 1; 
    params->dsi.customization_esd_check_enable = 1; // 0 ESD check via TE 1 ESD check via register 
    params->dsi.lcm_esd_check_table[0].cmd = 0x0A; 
    params->dsi.lcm_esd_check_table[0].count = 1; 
    params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C; 
    params->dsi.noncont_clock = 1; // 1 clk noncontinue 非连续 0 continue 连续 
    params->dsi.noncont_clock_period = 2;
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
    MDELAY(100);
    SET_RESET_PIN(1);
    MDELAY(120);
    SET_RESET_PIN(0);
	MDELAY(1); // 1ms
	SET_RESET_PIN(1);
	MDELAY(120); 
}


    //for LGE backlight IC mapping table


    // Refresh value of backlight level.


static void lcm_resume(void)
{
     lcm_init();

    /// please notice: the max return packet size is 1
    /// if you want to change it, you can refer to the following marked code
    /// but read_reg currently only support read no more than 4 bytes....
    /// if you need to read more, please let BinHan knows.
    /*
            unsigned int data_array[16];
            unsigned int max_return_size = 1;
            
            data_array[0]= 0x00003700 | (max_return_size << 16);    
            
            dsi_set_cmdq(&data_array, 1, 1);
    


	unsigned int data_array[16];

    MDELAY(100);
    data_array[0]=0x00290500;
    dsi_set_cmdq(data_array,1,1);
    MDELAY(10);
*/
}


static unsigned int lcm_compare_id(void)
{
int   array[4];
		char  buffer[5];
		unsigned int id_high;
		unsigned int id_low;
		unsigned int id=0;
        //Do reset here
        SET_RESET_PIN(1);
        SET_RESET_PIN(0);
		MDELAY(10);
        SET_RESET_PIN(1);
		MDELAY(30);
       
        array[0]=0x00043902;
	array[1]=0x018198ff;
	dsi_set_cmdq(array, 2, 1);
        MDELAY(10);
        array[0]=0x00023700;
        dsi_set_cmdq(array, 1, 1);
    
        read_reg_v2(0x00, buffer,1);
	id_high = buffer[0]; ///////////////////////0x98
        read_reg_v2(0x01, buffer,1);
	id_low = buffer[0]; ///////////////////////0x06
       // id = (id_midd &lt;&lt; 8) | id_low;
	id = (id_high << 8) | id_low;

		#if defined(BUILD_LK)
		printf("ILI9881 %s id_high = 0x%04x, id_low = 0x%04x\n,id=0x%x\n", __func__, id_high, id_low,id);
#else
		printk("ILI9881 %s id_high = 0x%04x, id_low = 0x%04x\n,id=0x%x\n", __func__, id_high, id_low,id);
#endif
	//return 1;	
	 return (LCM_ID == id)?1:0;

}

// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER hct_ili9881c_dsi_vdo_hd_hsd_55_fc = 
{
    .name           = "hct_ili9881c_dsi_vdo_hd_hsd_55_fc",
    .set_util_funcs = lcm_set_util_funcs,
    .get_params     = lcm_get_params,
    .init           = lcm_init,
    .suspend        = lcm_suspend,
    .resume         = lcm_resume,   
    .compare_id    = lcm_compare_id,    
#if 0//defined(LCM_DSI_CMD_MODE)
    //.set_backlight    = lcm_setbacklight,
    //.esd_check   = lcm_esd_check, 
    //.esd_recover   = lcm_esd_recover, 
    .update         = lcm_update,
#endif  //wqtao
};

