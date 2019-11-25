#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/kernel.h>
#endif
#include "lcm_drv.h"

#if defined(BUILD_LK)
#else

#include <linux/proc_fs.h>   //proc file use 
#endif

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (480)
#define FRAME_HEIGHT (854)
#define REGFLAG_DELAY             							0XFE
#define REGFLAG_END_OF_TABLE      							0xFD   // END OF REGISTERS MARKER


#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif
/*
#ifndef BUILD_LK
static unsigned int lcm_esd_test = FALSE;      ///only for ESD test
#endif
*/
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

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

#define   LCM_DSI_CMD_MODE							0
struct LCM_setting_table {
    unsigned char cmd;
    unsigned char count;
    unsigned char para_list[64];
};


static struct LCM_setting_table lcm_initialization_setting[] = {
{0xB9,3,{0xF1,0x08,0x01}},
{0xB1,4,{0x22,0x1e,0x1e,0x87}},
{0xB2,1,{0x22}},
{0xB3,8,{0x01,0x00,0x06,0x06,0x16,0x12,0x37,0x34}},
{0xBA,17,{0x31,0x00,0x44,0x25,0x91,0x0A,0x00,0x00,0xC1,0x00,0x00,0x00,0x0D,0x02,0x4F,0xB9,0xEE}},
{0xE3,5,{0x05,0x05,0x01,0x01,0x00}},
{0xB4,1,{0x00}},
{0xB5,2,{0x0C,0x0C}},
{0xB6,2,{0x0C,0x0C}},
{0xB8,2,{0x64,0x20}},
{0xCC,1,{0x02}},
{0xBC,1,{0x47}},
{0xE9,51,{0x00,0x00,0x0E,0x03,0x68,0x0A,0x08,0x12,0x31,0x23,
	  0x47,0x11,0x58,0x28,0x37,0x00,0x06,0x18,0x00,0x00,
	  0x00,0x10,0x88,0x83,0x11,0x35,0x75,0x78,0x88,0x88,
	  0x88,0x88,0x82,0x00,0x24,0x64,0x68,0x88,0x88,0x88,
	  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
{0xEA,28,{0x90,0x00,0x00,0x00,0x88,0x84,0x60,0x64,0x22,0x08,
	  0x88,0x88,0x88,0x88,0x85,0x71,0x75,0x33,0x18,0x88,
	  0x88,0x88,0x30,0x00,0x00,0xFF,0x00,0x60}},
{0xE0,34,{0x00,0x0D,0x16,0x3F,0x3F,0x3F,0x29,0x42,0x08,0x0F,0x0F,0x12,0x12,0x11,0x12,0x0E,0x1F,
	  0x00,0x0D,0x16,0x3F,0x3F,0x3F,0x29,0x42,0x08,0x0F,0x0F,0x12,0x12,0x11,0x12,0x0E,0x1F}},

{0x11,1,{0x00}},

{REGFLAG_DELAY,150,{}},

{0x29,1,{0x00}},

{REGFLAG_DELAY,50,{}},

{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
    // Display off sequence
    {0xE3,5,{0x05,0x05,0x01,0x01,0xC0}},
    {REGFLAG_DELAY, 10, {}},
    {0x28, 1, {0x00}},
    {REGFLAG_DELAY, 20, {}},

    // Sleep Mode On
    {0x10, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    {REGFLAG_END_OF_TABLE, 0x00, {}}
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

	// physical size
	//params->physical_width = PHYSICAL_WIDTH;
	//params->physical_height = PHYSICAL_HEIGHT;
	// enable tearing-free		
	params->dbi.te_mode 			= LCM_DBI_TE_MODE_VSYNC_ONLY;
	params->dbi.te_edge_polarity = LCM_POLARITY_RISING;
	///////////////////// 	
	//if(params->dsi.lcm_int_te_monitor)	
	params->dsi.cont_clock= TRUE;
	params->dsi.mode	 = SYNC_PULSE_VDO_MODE;  

	// DSI
	/* Command mode setting */
		params->dsi.LANE_NUM				= LCM_TWO_LANE;//LCM_FOUR_LANE;
	
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;
	
	params->dsi.intermediat_buffer_num = 2;  
	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
	

	// Video mode setting
	params->dsi.packet_size=256;

	params->dsi.vertical_sync_active				=  4;//4
	params->dsi.vertical_backporch					= 18;//50;
	params->dsi.vertical_frontporch					= 18;//50;
	params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 20;// 50  2
		params->dsi.horizontal_backporch				= 60;//50
		params->dsi.horizontal_frontporch				=60;//50
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	    //params->dsi.LPX=8; 

		// Bit rate calculation
//		params->dsi.PLL_CLOCK = 208;//220  190   //255
		//1 Every lane speed
		//params->dsi.pll_div1=1;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
		//params->dsi.pll_div2=0;		// div2=0,1,2,3;div1_real=1,2,4,4	
		//params->dsi.fbk_div =17;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)	

#if FRAME_WIDTH == 480	
	params->dsi.PLL_CLOCK=205;//254//247
#elif FRAME_WIDTH == 540
	params->dsi.PLL_CLOCK=230;
#elif FRAME_WIDTH == 720
	params->dsi.PLL_CLOCK=230;
#elif FRAME_WIDTH == 1080
	params->dsi.PLL_CLOCK=410;
#else
	params->dsi.PLL_CLOCK=230;
#endif

	params->dsi.customization_esd_check_enable = 1;
	params->dsi.esd_check_enable = 1;
	params->dsi.lcm_esd_check_table[0].cmd          = 0x0A;
	params->dsi.lcm_esd_check_table[0].count        = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;
	


}

static void lcm_init(void)
{
	SET_RESET_PIN(1);  //NOTE:should reset LCM firstly
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(120);	
	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_suspend(void)
{
    push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);   //wqtao. enable

	SET_RESET_PIN(1);  //NOTE:should reset LCM firstly
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(120);	
/*
	unsigned int array[5];
 	array[0] = 0x00011500;// read id return two byte,version and id
	dsi_set_cmdq(&array, 1, 1);
	MDELAY(100);
	array[0] = 0x00280500;
	dsi_set_cmdq(&array, 1, 1);
	MDELAY(50);
 	array[0] = 0x00100500;// read id return two byte,version and id
	dsi_set_cmdq(&array, 1, 1);
	MDELAY(120);
 	array[0] = 0x014f1500;// read id return two byte,version and id
	dsi_set_cmdq(&array, 1, 1);
	MDELAY(50);
*/
}

//static int vcom = 0x46;
static void lcm_resume(void)
{/*lcm_initialization_setting[13].para_list[0] = vcom;//paralist[]//from left to right {0x39,0xB6,4,{0x00,0x91,0x00,0x91}},0x91 =paralist[1]
 lcm_initialization_setting[13].para_list[1] = vcom;	
 */
          lcm_init();
  /* #if defined(BUILD_LK)
	printf("vcom = %x\n",vcom);
      #else
	printk("vcom = %x\n",vcom);
      #endif
	vcom += 2;
   */	
}
/*
#if defined(LCM_DSI_CMD_MODE)
static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];

	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	dsi_set_cmdq(data_array, 3, 1);
	
	data_array[0]= 0x00053902;
	data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[2]= (y1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

//	data_array[0]= 0x00290508; //HW bug, so need send one HS packet
//	dsi_set_cmdq(data_array, 1, 1);
	
	data_array[0]= 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);

}
#endif
*/
static unsigned int lcm_compare_id(void)
{
	unsigned int id1 = 0, id2 = 0,id;
	unsigned char buffer[6];
	unsigned int array[16];  

	SET_RESET_PIN(1);
        MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(20);
	SET_RESET_PIN(1);
	MDELAY(120); 

	array[0]=0x00043902;
	array[1]=0x0108f1B9;// page enable
	dsi_set_cmdq(array, 2, 1);

	array[0] = 0x00033700;// return byte number
	dsi_set_cmdq(array, 1, 3);
	MDELAY(10);


	read_reg_v2(0x04, buffer, 3);
	id1  =  buffer[0];
	id2 =  buffer[1];
	id = (id1 << 8) | id2; 
	/*
	array[0] = 0x00013700;// return byte number
	dsi_set_cmdq(array, 1, 1);
	MDELAY(10);
	read_reg_v2(0xDB, buffer, 1);
	id1 =  buffer[0];
	*/
#ifdef BUILD_LK
	printk("FL10802 id = 0x%08x id1=%x \n",  id,id1);
#else
	//printk("FL10802 id = 0x%08x  id1=%x \n",id,id1);
#endif
	//return 1;	
	return (0x1080 == id)?1:0;//id1

}
/*add by lifengxiang@wind-mobi.com 20151227 begin
static unsigned int lcm_esd_check(void)
{
	#ifndef BUILD_LK     
		//  return 0; //FALSE  
		unsigned int id  = 0;  
		unsigned char buffer[6];   
		unsigned int data_array[16];   
		//static int lcm_id;  
		UDELAY(600); //guohongjin@wind-mobi.com 20131127 modify
		data_array[0] = 0x00013700;// read id return two byte,version and id  
		dsi_set_cmdq(data_array, 1, 1);  
		//MDELAY(10);
		read_reg_v2(0x0A, buffer, 1);    // A1  
		id = buffer[0]; //we only need ID     
		//printk("ghj ########## ili9806e lcd_id=%x,\r\n",id);
		if(id ==0x9c)     
		{    
			return 0;   
		}      
		else      
		{         
			return 1; //TRUE     
		}
	#endif
}

static unsigned int lcm_esd_recover(void)
{
	lcm_init();
	return TRUE;
}

static unsigned int lcm_check_status(void)
{
	unsigned char buffer[2];

	read_reg_v2(0x0A, buffer, 2);
#ifdef BUILD_LK
	printf("Check LCM Status: 0x%08x\n", buffer[0]);
#else
	printk("Check LCM Status: 0x%08x\n", buffer[0]);
#endif
	return 0;
}add by lifengxiang@wind-mobi.com 20151227 end*/


LCM_DRIVER hct_fl10802_dsi_vdo_fwvga_ivo_50_gt = 
{
    	.name	= "hct_fl10802_dsi_vdo_fwvga_ivo_50_gt",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
        .compare_id    	= lcm_compare_id,
//	.esd_check      = lcm_esd_check,
//	.esd_recover    = lcm_esd_recover,
#if (LCM_DSI_CMD_MODE) 
    .update         = lcm_update,
#endif
    };
