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

#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
	#include <string.h>
#elif defined(BUILD_UBOOT)
	#include <asm/arch/mt_gpio.h>
#else
//	#include <mach/mt_gpio.h>
#endif
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH                                         (1080)
#define FRAME_HEIGHT                                        (1920)
#define LCM_ID                       (0x9885)

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
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[128];
};


static struct LCM_setting_table lcm_initialization_setting[] = {


/*//============ILI9885+AUO5.46 SZ initial Code========

{0xB0,3,{0x98,0x85,0x0A}},
{REGFLAG_DELAY,20,{}}, 

{0xC4,7,{0x70,0x19,0x23,0x00,0x0F,0x0F,0x00}}, //set SDT
{0xD0,6,{0x55,0x05,0x34,0x6B,0xEC,0xC0}}, //VGH=3AVDD-AVEE=16V(single), VGL=2AVEE-AVDD=-14V(dual)
{0xD3,9,{0x33,0x33,0x05,0x03,0x59,0x59,0x22,0x26,0x22}}, //GVDDP=4.3V GVDDN=-4.3V VGHO=15 VGLO=-12 AVDDR=4.7V AVEER=-4.7V
{0xD5,10,{0x8B,0x00,0x00,0x00,0x01,0x8A,0x01,0x8A,0x00,0xFF }},//set Vcom




{0xEC,07,{0x76,0x1E,0x32,0x00,0x46,0x00,0x02}}, //black display while video stop
{0xEF,01,{0x8F}}, //power saving
//set LVD sequence
{0xEB,35,{0xA3,0xC7,0x73,0x00,0x58,0x55,0x55,0x55,0x55,0x50,0x00,0x00,0x00,0x00,0x00,0x25,0xCD,0x0F,0xFF,0xFF,0xFF,0xFF,0xFF,0x55,0x55,0x55,0x55,0x32,0x77,0x55,0x43,0x45,0x5E,0xFF,0x55}}, //VCOM tie GND during sleep in & source GND for 2 frames


//GIP setting
{0xE5,73,{0x36,0x36,0xA1,0xF6,0xF6,0x47,0x07,0x55,0x15,0x63,0x23,0x71,0x31,0x3E,0x37,0x85,0x36,0x36,0x36,0x36,0x36,0x36,0xA8,0xF6,0xF6,0x4E,0x0E,0x5C,0x1C,0x6A,0x2A,0x78,0x38,0x3E,0x37,0x8C,0x36,0x36,0x36,0x36,0x18,0x70,0x61,0x00,0x4E,0xBB,0x70,0x80,0x00,0x4E,0xBB,0xF7,0x00,0x4E,0xBB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07}},
{0xEA,66,{0x51,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x70,0x01,0x10,0x00,0x40,0x80,0xC0,0x00,0x00,0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF,0xCC,0xCC,0x22,0x33,0x33,0x00,0x11,0x00,0x11,0x00,0x11,0x00,0x11,0xCC,0xCC,0x22,0xCC,0xCC,0xCC,0xCC}},
{0xED,23,{0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40}}, //set reg_dispon_wait_vdo                                                                                                                                                                                    



//gamma setting
{0xC7,122,{0x00,0x19,0x00,0x68,0x00,0x96,0x00,0xB0,0x00,0xD0,0x00,0xE2,0x00,0xF2,0x01,0x04,0x01,0x14,0x01,0x43,0x01,0x66,0x01,0x9F,0x01,0xCB,0x02,0x10,0x02,0x45,0x02,0x47,0x02,0x7A,0x02,0xB1,0x02,0xD5,0x03,0x06,0x03,0x24,0x03,0x48,0x03,0x58,0x03,0x61,0x03,0x73,0x03,0x7D,0x03,0x93,0x03,0xA8,0x03,0xC8,0x03,0xE8,0x00,0x19,0x00,0x68,0x00,0x96,0x00,0xB0,0x00,0xD0,0x00,0xE2,0x00,0xF2,0x01,0x04,0x01,0x14,0x01,0x43,0x01,0x66,0x01,0x9F,0x01,0xCB,0x02,0x10,0x02,0x45,0x02,0x47,0x02,0x7A,0x02,0xB1,0x02,0xD5,0x03,0x06,0x03,0x24,0x03,0x48,0x03,0x58,0x03,0x61,0x03,0x73,0x03,0x7D,0x03,0x93,0x03,0xA8,0x03,0xC8,0x03,0xE8,0x01,0x01}},

{0xD1,03,{0x09,0x09,0xC2}},   

{0xEE,14,{0x22,0x10,0x02,0x02,0x0F,0x40,0x00,0xA7,0x00,0x04,0x00,0x00,0x40,0xB9}},   
*/

 //ili9885+auo5.46  ILI  code
//============9885 initial Code========
{0xB0,3,{0x98,0x85,0x0A}},
{REGFLAG_DELAY,20,{}}, 

{0xC4,7,{0x70,0x19,0x23,0x00,0x0F,0x0F,0x00}}, //set SDT
{0xD0,6,{0x55,0x05,0x34,0x6B,0xEC,0xC0}}, //VGH=3AVDD-AVEE=16V(single), VGL=2AVEE-AVDD=-14V(dual)
{0xD3,9,{0x33,0x33,0x05,0x03,0x59,0x59,0x22,0x26,0x22}}, //GVDDP=4.3V GVDDN=-4.3V VGHO=15 VGLO=-12 AVDDR=4.7V AVEER=-4.7V
{0xD5,10,{0x8B,0x00,0x00,0x00,0x01,0x86,0x01,0x86,0x00,0xFF }},//set Vcom



{0xEC,07,{0x76,0x1E,0x32,0x00,0x46,0x00,0x02}}, //black display while video stop


{0xEF,8,{0x8F,0x05,0x52,0x13,0xE1,0x33,0x5b,0x09}}, //power saving

{0xD6,12,{0x00,0x00,0x08,0x17,0x23,0x65,0x77,0x44,0x87,0x00,0x00,0x09}},

//set LVD sequence
{0xEB,35,{0xA3,0xC7,0x73,0x00,0x58,0x55,0x55,0x55,0x55,0x50,0x00,0x00,0x00,0x00,0x00,0x25,0xCD,0x0F,0xFF,0xFF,0xFF,0xFF,0xFF,0x55,0x55,0x55,0x55,0x32,0x77,0x55,0x43,0x45,0x5E,0xFF,0x55}}, //VCOM tie GND during sleep in & source GND for 2 frames


//GIP setting
{0xE5,73,{0x36,0x36,0xA1,0xF6,0xF6,0x47,0x07,0x55,0x15,0x63,0x23,0x71,0x31,0x3E,0x37,0x85,0x36,0x36,0x36,0x36,0x36,0x36,0xA8,0xF6,0xF6,0x4E,0x0E,0x5C,0x1C,0x6A,0x2A,0x78,0x38,0x3E,0x37,0x8C,0x36,0x36,0x36,0x36,0x18,0x70,0x61,0x00,0x4E,0xBB,0x70,0x80,0x00,0x4E,0xBB,0xF7,0x00,0x4E,0xBB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07}},
{0xEA,66,{0x51,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x70,0x01,0x10,0x00,0x40,0x80,0xC0,0x00,0x00,0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF,0xCC,0xCC,0x22,0x33,0x33,0x00,0x11,0x00,0x11,0x00,0x11,0x00,0x11,0xCC,0xCC,0x22,0xCC,0xCC,0xCC,0xCC}},
{0xED,23,{0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40}}, //set reg_dispon_wait_vdo                                                                                                                                                                                    



//gamma setting
//{0xC7,122,{0x00,0x19,0x00,0x68,0x00,0x96,0x00,0xB0,0x00,0xD0,0x00,0xE2,0x00,0xF2,0x01,0x04,0x01,0x14,0x01,0x43,0x01,0x66,0x01,0x9F,0x01,0xCB,0x02,0x10,0x02,0x45,0x02,0x47,0x02,0x7A,0x02,0xB1,0x02,0xD5,0x03,0x06,0x03,0x24,0x03,0x48,0x03,0x58,0x03,0x61,0x03,0x73,0x03,0x7D,0x03,0x93,0x03,0xA8,0x03,0xC8,0x03,0xE8,0x00,0x19,0x00,0x68,0x00,0x96,0x00,0xB0,0x00,0xD0,0x00,0xE2,0x00,0xF2,0x01,0x04,0x01,0x14,0x01,0x43,0x01,0x66,0x01,0x9F,0x01,0xCB,0x02,0x10,0x02,0x45,0x02,0x47,0x02,0x7A,0x02,0xB1,0x02,0xD5,0x03,0x06,0x03,0x24,0x03,0x48,0x03,0x58,0x03,0x61,0x03,0x73,0x03,0x7D,0x03,0x93,0x03,0xA8,0x03,0xC8,0x03,0xE8,0x01,0x01}},

{0xC7,122,{0x00,0x3A,0x00,0x68,0x00,0x7F,0x00,0x95,0x00,0xAB,0x00,0xC1,0x00,0xD5,0x00,0xEB,0x01,0x00,0x01,0x2C,0x01,0x52,0x01,0x8C,0x01,0xBA,0x02,0x05,0x02,0x3F,0x02,0x42,0x02,0x75,0x02,0xAF,0x02,0xD4,0x03,0x05,0x03,0x25,0x03,0x4C,0x03,0x57,0x03,0x66,0x03,0x78,0x03,0x88,0x03,0x98,0x03,0xB0,0x03,0xC8,0x03,0xE8,0x00,0x3A,0x00,0x68,0x00,0x7F,0x00,0x95,0x00,0xAB,0x00,0xC1,0x00,0xD5,0x00,0xEB,0x01,0x00,0x01,0x2C,0x01,0x52,0x01,0x8C,0x01,0xBA,0x02,0x05,0x02,0x3F,0x02,0x42,0x02,0x75,0x02,0xAF,0x02,0xD4,0x03,0x05,0x03,0x25,0x03,0x4C,0x03,0x57,0x03,0x66,0x03,0x78,0x03,0x88,0x03,0x98,0x03,0xB0,0x03,0xC8,0x03,0xE8,0x01,0x01}},
    
{0x35,1,{0x00}},
{0x36,1,{0x00}},


	
	{0x11,1,{0x00}},
	{REGFLAG_DELAY,120,{}},	
	{0x29,1,{0x00}},
	{REGFLAG_DELAY,20,{}},	
	{REGFLAG_END_OF_TABLE, 0x00, {}}
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

	params->physical_width = 68;
	params->physical_height = 122;

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
	params->dsi.vertical_sync_active				= 8; //8;	//2;
	params->dsi.vertical_backporch					= 16; //18;	//14;
	params->dsi.vertical_frontporch					= 24; //20;	//16;
	params->dsi.vertical_active_line				= FRAME_HEIGHT; 

	params->dsi.horizontal_sync_active				= 8;	//2;
	params->dsi.horizontal_backporch				= 80;//120;	//60;	//42;
	params->dsi.horizontal_frontporch				= 64;//100;	//60;	//44;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;


    params->dsi.clk_lp_per_line_enable = 1; // 1 clk line by line return LP11 0 clk frame by frame return LP11
    params->dsi.esd_check_enable = 1;
    params->dsi.customization_esd_check_enable = 0; // 0 ESD check via TE 1 ESD check via register
    params->dsi.lcm_esd_check_table[0].cmd = 0x0A;
    params->dsi.lcm_esd_check_table[0].count = 1;
    params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;
    params->dsi.noncont_clock = 1; // 1 clk noncontinue 非连续 0 continue 连续
    params->dsi.noncont_clock_period = 2;


// Bit rate calculation
//1 Every lane speed
//params->dsi.pll_div1=0;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
//params->dsi.pll_div2=0;		// div2=0,1,2,3;div1_real=1,2,4,4	
//params->dsi.fbk_div =0x12;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)	

// zhangxiaofei add for test
params->dsi.PLL_CLOCK = 470;//208;	
}

static void lcm_init(void)
{
    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(10);

#ifdef GPIO_LCM_LDO_1V8_EN_PIN
    lcm_util.set_gpio_mode(GPIO_LCM_LDO_1V8_EN_PIN, GPIO_MODE_00);
    lcm_util.set_gpio_dir(GPIO_LCM_LDO_1V8_EN_PIN, GPIO_DIR_OUT);
    lcm_util.set_gpio_out(GPIO_LCM_LDO_1V8_EN_PIN, GPIO_OUT_ONE);  //shm
#else   

    mt_dsi_pinctrl_set(LCM_POWER_DM_NO, 1);
#endif
    MDELAY(10);

#ifdef GPIO_LCM_LDO_2V8_EN_PIN
    lcm_util.set_gpio_mode(GPIO_LCM_LDO_2V8_EN_PIN, GPIO_MODE_00);
    lcm_util.set_gpio_dir(GPIO_LCM_LDO_2V8_EN_PIN, GPIO_DIR_OUT);
    lcm_util.set_gpio_out(GPIO_LCM_LDO_2V8_EN_PIN, GPIO_OUT_ONE);
#else   
    mt_dsi_pinctrl_set(LCM_POWER_DP_NO, 1);
#endif
    MDELAY(10);


    SET_RESET_PIN(1);
    MDELAY(10);

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
	MDELAY(50);
    SET_RESET_PIN(1);
    MDELAY(20);
    SET_RESET_PIN(0);
	MDELAY(10); // 1ms

#ifdef GPIO_LCM_LDO_2V8_EN_PIN
    lcm_util.set_gpio_mode(GPIO_LCM_LDO_2V8_EN_PIN, GPIO_MODE_00);
    lcm_util.set_gpio_dir(GPIO_LCM_LDO_2V8_EN_PIN, GPIO_DIR_OUT);
    lcm_util.set_gpio_out(GPIO_LCM_LDO_2V8_EN_PIN, GPIO_OUT_ZERO);
#else   

    mt_dsi_pinctrl_set(LCM_POWER_DP_NO, 0);    // 2V8 off
#endif
    MDELAY(5);

#ifdef GPIO_LCM_LDO_1V8_EN_PIN
    lcm_util.set_gpio_mode(GPIO_LCM_LDO_1V8_EN_PIN, GPIO_MODE_00);
    lcm_util.set_gpio_dir(GPIO_LCM_LDO_1V8_EN_PIN, GPIO_DIR_OUT);
    lcm_util.set_gpio_out(GPIO_LCM_LDO_1V8_EN_PIN, GPIO_OUT_ZERO);  //shm
#else   
    mt_dsi_pinctrl_set(LCM_POWER_DM_NO, 0);
#endif
    MDELAY(5);
}


    //for LGE backlight IC mapping table


    // Refresh value of backlight level.


static void lcm_resume(void)
{
#ifdef GPIO_LCM_LDO_2V8_EN_PIN
    lcm_util.set_gpio_mode(GPIO_LCM_LDO_2V8_EN_PIN, GPIO_MODE_00);
    lcm_util.set_gpio_dir(GPIO_LCM_LDO_2V8_EN_PIN, GPIO_DIR_OUT);
    lcm_util.set_gpio_out(GPIO_LCM_LDO_2V8_EN_PIN, GPIO_OUT_ZERO);
#else   

    mt_dsi_pinctrl_set(LCM_POWER_DP_NO, 0);    // 2V8 off
#endif
    MDELAY(5);

#ifdef GPIO_LCM_LDO_1V8_EN_PIN
    lcm_util.set_gpio_mode(GPIO_LCM_LDO_1V8_EN_PIN, GPIO_MODE_00);
    lcm_util.set_gpio_dir(GPIO_LCM_LDO_1V8_EN_PIN, GPIO_DIR_OUT);
    lcm_util.set_gpio_out(GPIO_LCM_LDO_1V8_EN_PIN, GPIO_OUT_ZERO);  //shm
#else   
    mt_dsi_pinctrl_set(LCM_POWER_DM_NO, 0);
#endif
    MDELAY(5);
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
 
     char id_high=0;
       unsigned int id = 0; 
        char id_low=0;
	unsigned char buffer[2];
	unsigned int array[16];


#ifdef GPIO_LCM_LDO_1V8_EN_PIN
    lcm_util.set_gpio_mode(GPIO_LCM_LDO_1V8_EN_PIN, GPIO_MODE_00);
    lcm_util.set_gpio_dir(GPIO_LCM_LDO_1V8_EN_PIN, GPIO_DIR_OUT);
    lcm_util.set_gpio_out(GPIO_LCM_LDO_1V8_EN_PIN, GPIO_OUT_ONE);  //shm
#else   

    mt_dsi_pinctrl_set(LCM_POWER_DM_NO, 1);
#endif
    MDELAY(5);

#ifdef GPIO_LCM_LDO_2V8_EN_PIN
    lcm_util.set_gpio_mode(GPIO_LCM_LDO_2V8_EN_PIN, GPIO_MODE_00);
    lcm_util.set_gpio_dir(GPIO_LCM_LDO_2V8_EN_PIN, GPIO_DIR_OUT);
    lcm_util.set_gpio_out(GPIO_LCM_LDO_2V8_EN_PIN, GPIO_OUT_ONE);
#else   
    mt_dsi_pinctrl_set(LCM_POWER_DP_NO, 1);
#endif
    MDELAY(5);
 
        SET_RESET_PIN(1);
        MDELAY(10);
        SET_RESET_PIN(0);
        MDELAY(20);       
	SET_RESET_PIN(1);
        MDELAY(10);

	array[0] = 0x00023700;	// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0xBF, buffer, 2);   //get the manufacturer id,get in spec
	id_high = buffer[0];		//we only need ID
	id_low = buffer[1];
	id =(id_high << 8) | id_low;  
	
#ifdef BUILD_LK
	printf("%s, LK ili9885 txd auo debug: ili9885 0x%08x\n", __func__, id);
#else
	printk("%s, kernel debug:id = 0x%08x\n", __func__, id);
#endif
	//mt_set_gpio_out(GPIO_LCM_PWR_EN,GPIO_OUT_ZERO);
	//mt_set_gpio_out(GPIO_LCM_PWR2_EN,GPIO_OUT_ZERO);
    if(id == LCM_ID)
    	return 1;
    else
        return 0;

}

// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER hct_ili9885_dsi_vdo_fhd_auo_55_hf = 
{
    .name           = "hct_ili9885_dsi_vdo_fhd_auo_55_hf",
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

