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


#ifdef BUILD_LK
#else
#include <linux/string.h>
#endif
#include "lcm_drv.h"

#if defined(BUILD_LK)
#else
#include "tpd.h"
#include <linux/regulator/consumer.h>


#include <linux/proc_fs.h>   //proc file use 
#endif

#ifdef BUILD_LK
	#include <platform/upmu_common.h>
	#include <platform/upmu_hw.h>
	#include <platform/mt_i2c.h> 
	#include <platform/mt_pmic.h>
	#include <platform/mt_gpio.h>
	#include "cust_gpio_usage.h"

#else

#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>

//	#include <mach/mt_gpio.h>
#endif
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1280)

#define REGFLAG_DELAY             							(0XFFFE)
#define REGFLAG_END_OF_TABLE      							(0xFFFF)	// END OF REGISTERS MARKER


#define LCM_DSI_CMD_MODE									0

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

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))

#define LCM_RM68200_ID 		(0x6820)

//static unsigned int lcm_esd_test = FALSE;      ///only for ESD test

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

#ifdef BUILD_LK
#else
//extern void tpd_vgp_reg_regulator_disable(void);
extern void tpd_vgp_reg_regulator_enable(void);
extern void tpd_vgp_reg_regulator_disable(void);
#endif

 struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};


static void init_lcm_registers(void)
{
	unsigned int data_array[16];
	
	data_array[0] = 0x00DE0500; // 
	dsi_set_cmdq(data_array, 1, 1); //enter register mode


	data_array[0] = 0x32B41500; // 
	dsi_set_cmdq(data_array, 1, 1);


	data_array[0] = 0x70B31500; // 
	dsi_set_cmdq(data_array, 1, 1);


	data_array[0] = 0x10211500; // 
	dsi_set_cmdq(data_array, 1, 1);


	data_array[0] = 0x00DF0500; // 
	dsi_set_cmdq(data_array, 1, 1);


	data_array[0] = 0x00110500; // 
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(150);


	data_array[0] = 0x00290500; // Display On
	dsi_set_cmdq(data_array, 1, 1); 
	MDELAY(120);

}

#if 0
static struct LCM_setting_table lcm_sleep_in_setting[] = {
	// Display off sequence
	{0x01, 1, {0x00}},
	{REGFLAG_DELAY, 50, {}},
	
	{0x28, 1, {0x00}},
	{REGFLAG_DELAY, 50, {}},

	// Sleep Mode On
	{0x10, 1, {0x00}},
	{REGFLAG_DELAY, 50, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static void push_table(struct LCM_setting_table *table, unsigned int count,
		unsigned char force_update)
{
	unsigned int i;

	for (i = 0; i < count; i++) {

		unsigned cmd;
		cmd = table[i].cmd;

		switch (cmd) {

		case REGFLAG_DELAY:
			MDELAY(table[i].count);
			break;

		case REGFLAG_END_OF_TABLE:
			break;

		default:
			dsi_set_cmdq_V2(cmd, table[i].count,
					table[i].para_list, force_update);
		}
	}

}

#endif

// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS * util)
{
	memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}

static void lcm_get_params(LCM_PARAMS *params)
{
	memset(params, 0, sizeof(LCM_PARAMS));

	params->type   = LCM_TYPE_DSI;

	params->width  = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;

   //1 SSD2075 has no TE Pin
	// enable tearing-free
	//params->dbi.te_mode 				= LCM_DBI_TE_MODE_DISABLED;
//	params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

    #if (LCM_DSI_CMD_MODE)
	params->dsi.mode   = CMD_MODE;
    #else
    params->dsi.mode   = SYNC_PULSE_VDO_MODE;
    //params->dsi.mode   = BURST_VDO_MODE;
    //params->dsi.mode   = SYNC_EVENT_VDO_MODE;
    
    #endif

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
    params->dsi.PLL_CLOCK = 260; //this value must be in MTK suggested table
    
    /* params->dsi.vertical_sync_active             = 3;  //---3 */
    /* params->dsi.vertical_backporch                   = 12; //---14 */
    /* params->dsi.vertical_frontporch                  = 8;  //----8 */
    /* params->dsi.vertical_active_line             = FRAME_HEIGHT; */

	/* params->dsi.horizontal_sync_active				= 2;  //----2 */
	/* params->dsi.horizontal_backporch				= 28; //----28 */
	/* params->dsi.horizontal_frontporch				= 50; //----50 */
	/* params->dsi.horizontal_active_pixel				= FRAME_WIDTH; */
#if 0
	params->dsi.vertical_sync_active				= 3;
	params->dsi.vertical_backporch					= 10;
	params->dsi.vertical_frontporch					= 8;
	params->dsi.vertical_active_line				= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active				= 2;
	params->dsi.horizontal_backporch				= 10;
	params->dsi.horizontal_frontporch				= 10;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
#else
	params->dsi.vertical_sync_active				= 2;
	params->dsi.vertical_backporch					= 10;
	params->dsi.vertical_frontporch 				= 10;
	params->dsi.vertical_active_line				= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active				= 10;
	params->dsi.horizontal_backporch				= 118;
	params->dsi.horizontal_frontporch				= 118;
	params->dsi.horizontal_active_pixel 			= FRAME_WIDTH;

#endif

	params->dsi.HS_PRPR=3;
	params->dsi.CLK_HS_POST=22;
	params->dsi.DA_HS_EXIT=20;


	// Bit rate calculation
	//1 Every lane speed
//	params->dsi.pll_div1=0;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
//	params->dsi.pll_div2=1;		// div2=0,1,2,3;div1_real=1,2,4,4	
//	params->dsi.fbk_div =19;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)	

	params->dsi.clk_lp_per_line_enable = 0;
	params->dsi.esd_check_enable = 0;
	params->dsi.customization_esd_check_enable = 0;
	params->dsi.lcm_esd_check_table[0].cmd          = 0x0a;
	params->dsi.lcm_esd_check_table[0].count        = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9c;

}

static void lcm_init(void)
{
#if defined(BUILD_LK)
	lcm_util.set_gpio_mode(GPIO_LCM_RST, GPIO_MODE_00);
	lcm_util.set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);
	lcm_util.set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);  //shm
	MDELAY(10);

	/*vgp 05: 2.8v
	vgp 07: 3.3v  */

	pmic_set_register_value(PMIC_RG_VGP1_VOSEL,0x07);
	pmic_set_register_value(PMIC_RG_VGP1_EN,1);

	MDELAY(10);

	//#ifdef GPIO_LCM_LDO_2V8_EN_PIN
	lcm_util.set_gpio_mode(GPIO_LCM_LDO_2V8_EN_PIN, GPIO_MODE_00);
	lcm_util.set_gpio_dir(GPIO_LCM_LDO_2V8_EN_PIN, GPIO_DIR_OUT);
	lcm_util.set_gpio_out(GPIO_LCM_LDO_2V8_EN_PIN, GPIO_OUT_ONE);
	MDELAY(20);

	lcm_util.set_gpio_mode(GPIO_LCM_LDO_1V8_EN_PIN, GPIO_MODE_00);
	lcm_util.set_gpio_dir(GPIO_LCM_LDO_1V8_EN_PIN, GPIO_DIR_OUT);
	lcm_util.set_gpio_out(GPIO_LCM_LDO_1V8_EN_PIN, GPIO_OUT_ONE);  //shm

	MDELAY(40);

	//#endif
	//#ifdef  GPIO_LCM_RST
	lcm_util.set_gpio_mode(GPIO_LCM_RST, GPIO_MODE_00);
	lcm_util.set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);
	lcm_util.set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ONE);
	MDELAY(10);
	lcm_util.set_gpio_mode(GPIO_LCM_RST, GPIO_MODE_00);
	lcm_util.set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);
	lcm_util.set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);
	MDELAY(10);
	lcm_util.set_gpio_mode(GPIO_LCM_RST, GPIO_MODE_00);
	lcm_util.set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);
	lcm_util.set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ONE);

	MDELAY(200);
#else
	SET_RESET_PIN(0);
	MDELAY(10);
	tpd_vgp_reg_regulator_enable();
	MDELAY(10);
	mt_dsi_pinctrl_set(LCM_POWER_DM_NO, 1);  // 2v8
	MDELAY(20);
	mt_dsi_pinctrl_set(LCM_POWER_DP_NO, 1);  // 1v8
	MDELAY(40);
	SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(200);

#endif
	init_lcm_registers();
}

static void lcm_suspend(void)
{
 
#if !defined(BUILD_LK)
	tpd_vgp_reg_regulator_disable();

	mt_dsi_pinctrl_set(LCM_POWER_DM_NO, 0);  // 2v8
	MDELAY(20);
	mt_dsi_pinctrl_set(LCM_POWER_DP_NO, 0);  // 1v8

	MDELAY(10);
	SET_RESET_PIN(0);
#endif	

}


static void lcm_resume(void)
{   
#if !defined(BUILD_LK)
	lcm_init();
#endif	

}


static unsigned int lcm_compare_id(void)
{

	int array[4];
	char buffer[5];
	char id_high=0;
	char id_low=0;
	int id1=0;
	int id2=0;
#if defined(BUILD_LK)
	lcm_util.set_gpio_mode(GPIO_LCM_RST, GPIO_MODE_00);
	lcm_util.set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);
	lcm_util.set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);  //shm
	MDELAY(10);

	/*vgp 05: 2.8v
	vgp 07: 3.3v  */

	pmic_set_register_value(PMIC_RG_VGP1_VOSEL,0x07);
	pmic_set_register_value(PMIC_RG_VGP1_EN,1);

	MDELAY(10);

	//#ifdef GPIO_LCM_LDO_2V8_EN_PIN
	lcm_util.set_gpio_mode(GPIO_LCM_LDO_2V8_EN_PIN, GPIO_MODE_00);
	lcm_util.set_gpio_dir(GPIO_LCM_LDO_2V8_EN_PIN, GPIO_DIR_OUT);
	lcm_util.set_gpio_out(GPIO_LCM_LDO_2V8_EN_PIN, GPIO_OUT_ONE);
	MDELAY(20);

	lcm_util.set_gpio_mode(GPIO_LCM_LDO_1V8_EN_PIN, GPIO_MODE_00);
	lcm_util.set_gpio_dir(GPIO_LCM_LDO_1V8_EN_PIN, GPIO_DIR_OUT);
	lcm_util.set_gpio_out(GPIO_LCM_LDO_1V8_EN_PIN, GPIO_OUT_ONE);  //shm

	MDELAY(40);

	//#endif
	//#ifdef  GPIO_LCM_RST
	lcm_util.set_gpio_mode(GPIO_LCM_RST, GPIO_MODE_00);
	lcm_util.set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);
	lcm_util.set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ONE);
	MDELAY(10);
	lcm_util.set_gpio_mode(GPIO_LCM_RST, GPIO_MODE_00);
	lcm_util.set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);
	lcm_util.set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);
	MDELAY(10);
	lcm_util.set_gpio_mode(GPIO_LCM_RST, GPIO_MODE_00);
	lcm_util.set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);
	lcm_util.set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ONE);

	MDELAY(200);
#else
	SET_RESET_PIN(0);
	MDELAY(10);
	tpd_vgp_reg_regulator_enable();
	MDELAY(10);
	mt_dsi_pinctrl_set(LCM_POWER_DM_NO, 1);  // 2v8
	MDELAY(20);
	mt_dsi_pinctrl_set(LCM_POWER_DP_NO, 1);  // 1v8
	MDELAY(40);
	SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(200);

#endif

	array[0]=0x01FE1500;
	dsi_set_cmdq(array,1, 1);

	array[0] = 0x00013700;
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0xde, buffer, 1);

	id_high = buffer[0];
	read_reg_v2(0xdf, buffer, 1);
	id_low = buffer[0];
	id1 = (id_high<<8) | id_low;

#if defined(BUILD_LK)
	printf("rm68200a %s id1 = 0x%04x, id2 = 0x%04x\n", __func__, id1,id2);
#else
	printk("rm68200a %s id1 = 0x%04x, id2 = 0x%04x\n", __func__, id1,id2);
#endif
	return (LCM_RM68200_ID == id1)?1:0;

}

// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER hct_td4291_dsi_vdo_hd_jdi_50_xyl = 
{
	.name			= "hct_td4291_dsi_vdo_hd_jdi_50_xyl",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,	
	.compare_id     = lcm_compare_id,	
#if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
#endif	//wqtao
};

