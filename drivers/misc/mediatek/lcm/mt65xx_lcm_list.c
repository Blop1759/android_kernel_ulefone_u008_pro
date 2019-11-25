#include "mt65xx_lcm_list.h"
#include <lcm_drv.h>
#ifdef BUILD_LK
#include <platform/disp_drv_platform.h>
#else
#include <linux/delay.h>
/* #include <mach/mt_gpio.h> */
#endif

/* used to identify float ID PIN status */
#define LCD_HW_ID_STATUS_LOW      0
#define LCD_HW_ID_STATUS_HIGH     1
#define LCD_HW_ID_STATUS_FLOAT 0x02
#define LCD_HW_ID_STATUS_ERROR  0x03

#ifdef BUILD_LK
#define LCD_DEBUG(fmt)  dprintf(CRITICAL, fmt)
#else
#define LCD_DEBUG(fmt, args...)  pr_debug("[KERNEL/LCM]"fmt, ##args)
#endif
extern LCM_DRIVER hct_ili9881_dsi_vdo_hd_cmi_50_hlt;
extern LCM_DRIVER hct_ili9885_dsi_vdo_fhd_auo_55_hf;
extern LCM_DRIVER hct_ili9881c_dsi_vdo_hd_ivo_50_xt;
extern LCM_DRIVER hct_nt35521_dsi_vdo_hd_boe_50_xld;
extern LCM_DRIVER hct_rm68200_dsi_vdo_hd_cmi_50_ll;
extern LCM_DRIVER hct_ili9881c_dsi_vdo_hd_auo_55_hz;
extern LCM_DRIVER hct_nt35521_dsi_vdo_hd_auo_50_rx;
extern LCM_DRIVER hct_fl11281_dsi_vdo_hd_ivo_50_hycj;
extern LCM_DRIVER hct_ili9881c_dsi_vdo_hd_auo_546_fsd;
extern LCM_DRIVER hct_rm68200_dsi_vdo_hd_auo_55_ry;
extern LCM_DRIVER hct_ili9881c_dsi_vdo_hd_cmo_55_sh;
extern LCM_DRIVER hct_ili9881c_dsi_vdo_hd_cmi_55_cm;
extern LCM_DRIVER hct_ili9881c_dsi_vdo_hd_cmi_50_cm;
extern LCM_DRIVER hct_ili9881c_dsi_vdo_hd_hsd_499_hx;
extern LCM_DRIVER hct_fl11281_dsi_vdo_hd_auo_50_wcl;
extern LCM_DRIVER hct_r69339_dsi_vdo_hd_sharp_55_ln;
extern LCM_DRIVER hct_nt35521sh_dsi_vdo_hd_cmi_50_rx;
extern LCM_DRIVER hct_gds8102_dsi_vdo_hd_gvo_55_gx;
extern LCM_DRIVER hct_rm67120_dsi_vdo_hd_gvo_55_gx;
extern LCM_DRIVER hct_hx8394f_dsi_vdo_hd_auo_55_np;
extern LCM_DRIVER hct_otm1282a_dsi_vdo_hd_auo_50_ll;
extern LCM_DRIVER hct_nt35521_dsi_vdo_hd_lg_50_rx;
extern LCM_DRIVER hct_otm1285a_dsi_vdo_hd_boe_50_hx;
extern LCM_DRIVER hct_nt35521_dsi_vdo_hd_auo_50_lg;
extern LCM_DRIVER hct_otm1285a_dsi_vdo_hd_boe_50_ykl;
extern LCM_DRIVER hct_rm69052_dsi_vdo_hd_auo_50_rx;
extern LCM_DRIVER hct_rm68200_dsi_vdo_hd_tm_50_ll;
extern LCM_DRIVER hct_rm68200_dsi_vdo_hd_auo_50_tzgd;
extern LCM_DRIVER hct_otm1284a_dsi_vdo_hd_boe_55_ykl;
extern LCM_DRIVER hct_ili9881c_dsi_vdo_hd_auo_50_hz;
extern LCM_DRIVER hct_jd9365aa_dsi_vdo_hd_cmo_55_sh;
extern LCM_DRIVER hct_jd9365aa_dsi_vdo_hd_ivo_55_hycj;
extern LCM_DRIVER hct_jd9365aa_dsi_vdo_hd_ivo_55_xzx;
extern LCM_DRIVER hct_td4291_dsi_vdo_hd_jdi_50_xyl;
extern LCM_DRIVER hct_ili9881c_dsi_vdo_hd_cmi_55_hf;
extern LCM_DRIVER hct_ili9881c_dsi_vdo_hd_cmi_50_tz;
extern LCM_DRIVER hct_ili9881c_dsi_vdo_hd_auo_55_xzx;
extern LCM_DRIVER hct_nt35521_dsi_vdo_hd_boe_55_hr;
extern LCM_DRIVER hct_ili9881c_dsi_vdo_hd_cmi_55_hr;
extern LCM_DRIVER hct_rm67120_dsi_vdo_hd_auo_55_amoled;
extern LCM_DRIVER hct_otm1282a_dsi_vdo_hd_auo_50_kl;
extern LCM_DRIVER hct_jd9365_dsi_vdo_hd_qm_55_hz;
extern LCM_DRIVER hct_ili9881c_dsi_vdo_hd_cpt_50_cw;
extern LCM_DRIVER hct_jd9361_dsi_vdo_hd_ivo_50_bx;
extern LCM_DRIVER hct_nt35596_dsi_vdo_hd_jdi_60_ykl;
extern LCM_DRIVER hct_nt35532_dsi_vdo_fhd_sharp_55_sh;
extern LCM_DRIVER hct_nt35532_dsi_vdo_fhd_sharp_50_sh;
extern LCM_DRIVER hct_r61322a_dsi_vdo_fhd_55_ot;
extern LCM_DRIVER hct_ili9881c_dsi_vdo_hd_hsd_499_ykl;
extern LCM_DRIVER hct_rm68200_dsi_vdo_hd_cmi_50_ykl;
extern LCM_DRIVER hct_rm68200_dsi_vdo_hd_hsd_50_xsj;
extern LCM_DRIVER hct_nt35521_dsi_vdo_hd_boe_55_xld;
extern LCM_DRIVER hct_nt35521_dsi_vdo_hd_tm_55_xld;
extern LCM_DRIVER hct_rm68200_dsi_vdo_hd_auo_50_ykl;
extern LCM_DRIVER hct_ili9881_dsi_vdo_hd_cpt;
extern LCM_DRIVER hct_hx8394f_dsi_vdo_hd_cmi;
extern LCM_DRIVER hct_jd9365_dsi_vdo_hd_cpt_50_xq;
extern LCM_DRIVER hct_st7701_dsi_vdo_fwvga_cmi_50_xq;
extern LCM_DRIVER hct_ili9881c_dsi_vdo_hd_auo_50_cw;
extern LCM_DRIVER hct_rm68200_dsi_vdo_hd_tm_55_xld;
extern LCM_DRIVER hct_fl11281_dsi_vdo_hd_auo_50_gt;
extern LCM_DRIVER hct_fl10802_dsi_vdo_fwvga_ivo_50_gt;
extern LCM_DRIVER hct_st7701_dsi_vdo_fwvga_ivo_50_bx;
extern LCM_DRIVER hct_jd9161_dsi_vdo_fwvga_boe_50_gt;
extern LCM_DRIVER hct_nt35521_dsi_vdo_hd_boe_50_xld_5Dai;
extern LCM_DRIVER hct_sh1386_dsi_vdo_hd_hx_55_hx;
extern LCM_DRIVER hct_ili9881c_dsi_vdo_hd_hsd_55_fc;
extern LCM_DRIVER hct_ili9881c_dsi_vdo_hd_cpt_55_ot;

LCM_DRIVER *lcm_driver_list[] = {
#if defined(MTK_LCM_DEVICE_TREE_SUPPORT)
	&lcm_common_drv,
#else
#if defined(HCT_ILI9881_DSI_VDO_HD_CMI_50_HLT)
        &hct_ili9881_dsi_vdo_hd_cmi_50_hlt,
#endif
#if defined(HCT_ILI9885_DSI_VDO_FHD_AUO_55_HF)
	&hct_ili9885_dsi_vdo_fhd_auo_55_hf,
#endif
#if defined(HCT_ILI9881C_DSI_VDO_HD_IVO_50_XT)
	&hct_ili9881c_dsi_vdo_hd_ivo_50_xt,
#endif
#if defined(HCT_NT35521_DSI_VDO_HD_BOE_50_XLD)
	&hct_nt35521_dsi_vdo_hd_boe_50_xld,
#endif
#if defined(HCT_RM68200_DSI_VDO_HD_CMI_50_LL)
	&hct_rm68200_dsi_vdo_hd_cmi_50_ll,
#endif
#if defined(HCT_ILI9881C_DSI_VDO_HD_AUO_55_HZ)
	&hct_ili9881c_dsi_vdo_hd_auo_55_hz,
#endif
#if defined(HCT_ILI9881C_DSI_VDO_HD_CMI_55_HR)
	&hct_ili9881c_dsi_vdo_hd_cmi_55_hr,
#endif
#if defined(HCT_NT35521_DSI_VDO_HD_AUO_50_RX)
	&hct_nt35521_dsi_vdo_hd_auo_50_rx,
#endif
#if defined(HCT_FL11281_DSI_VDO_HD_IVO_50_HYCJ)
	&hct_fl11281_dsi_vdo_hd_ivo_50_hycj,
#endif
#if defined(HCT_ILI9881C_DSI_VDO_HD_AUO_546_FSD)
	&hct_ili9881c_dsi_vdo_hd_auo_546_fsd,
#endif
#if defined(HCT_RM68200_DSI_VDO_HD_AUO_55_RY)
	&hct_rm68200_dsi_vdo_hd_auo_55_ry,
#endif
#if defined(HCT_ILI9881C_DSI_VDO_HD_CMO_55_SH)
	&hct_ili9881c_dsi_vdo_hd_cmo_55_sh,
#endif
#if defined(HCT_ILI9881C_DSI_VDO_HD_CMI_55_CM)
	&hct_ili9881c_dsi_vdo_hd_cmi_55_cm,
#endif
#if defined(HCT_ILI9881C_DSI_VDO_HD_CMI_50_CM)
	&hct_ili9881c_dsi_vdo_hd_cmi_50_cm,
#endif
#if defined(HCT_ILI9881C_DSI_VDO_HD_HSD_499_HX)
	&hct_ili9881c_dsi_vdo_hd_hsd_499_hx,
#endif
#if defined(HCT_FL11281_DSI_VDO_HD_AUO_50_WCL)
	&hct_fl11281_dsi_vdo_hd_auo_50_wcl,
#endif

#if defined(HCT_R69339_DSI_VDO_HD_SHARP_55_LN)
	&hct_r69339_dsi_vdo_hd_sharp_55_ln,
#endif
#if defined(HCT_NT35521SH_DSI_VDO_HD_CMI_50_RX)
	&hct_nt35521sh_dsi_vdo_hd_cmi_50_rx,
#endif
#if defined(HCT_GDS8102_DSI_VDO_HD_GVO_55_GX) 
        &hct_gds8102_dsi_vdo_hd_gvo_55_gx,
#endif
#if defined(HCT_RM67120_DSI_VDO_HD_GVO_55_GX)
	&hct_rm67120_dsi_vdo_hd_gvo_55_gx,
#endif
#if defined(HCT_HX8394F_DSI_VDO_HD_AUO_55_NP)
	&hct_hx8394f_dsi_vdo_hd_auo_55_np,
#endif
#if defined(HCT_OTM1282A_DSI_VDO_HD_AUO_50_LL)
	&hct_otm1282a_dsi_vdo_hd_auo_50_ll,
#endif
#if defined(HCT_NT35521_DSI_VDO_HD_LG_50_RX)
	&hct_nt35521_dsi_vdo_hd_lg_50_rx,
#endif
#if defined(HCT_OTM1285A_DSI_VDO_HD_BOE_50_HX)
	&hct_otm1285a_dsi_vdo_hd_boe_50_hx,
#endif
#if defined(HCT_NT35521_DSI_VDO_HD_AUO_50_LG)
	&hct_nt35521_dsi_vdo_hd_auo_50_lg,
#endif
#if defined(HCT_OTM1285A_DSI_VDO_HD_BOE_50_YKL)
	&hct_otm1285a_dsi_vdo_hd_boe_50_ykl,
#endif
#if defined(HCT_RM69052_DSI_VDO_HD_AUO_50_RX)
	&hct_rm69052_dsi_vdo_hd_auo_50_rx,
#endif
#if defined(HCT_RM68200_DSI_VDO_HD_TM_50_LL)
	&hct_rm68200_dsi_vdo_hd_tm_50_ll,
#endif
#if defined(HCT_RM68200_DSI_VDO_HD_AUO_50_TZGD)
	&hct_rm68200_dsi_vdo_hd_auo_50_tzgd,
#endif
#if defined(HCT_OTM1284A_DSI_VDO_HD_BOE_55_YKL)
	&hct_otm1284a_dsi_vdo_hd_boe_55_ykl,
#endif
#if defined(HCT_ILI9881C_DSI_VDO_HD_AUO_50_HZ)
	&hct_ili9881c_dsi_vdo_hd_auo_50_hz,
#endif
#if defined(HCT_JD9365AA_DSI_VDO_HD_CMO_55_SH)
	&hct_jd9365aa_dsi_vdo_hd_cmo_55_sh,
#endif
#if defined(HCT_JD9365AA_DSI_VDO_HD_IVO_55_HYCJ)
	&hct_jd9365aa_dsi_vdo_hd_ivo_55_hycj,
#endif
#if defined(HCT_JD9365AA_DSI_VDO_HD_IVO_55_XZX)
	&hct_jd9365aa_dsi_vdo_hd_ivo_55_xzx,
#endif
#if defined(HCT_TD4291_DSI_VDO_HD_JDI_50_XYL)
	&hct_td4291_dsi_vdo_hd_jdi_50_xyl,
#endif
#if defined(HCT_ILI9881C_DSI_VDO_HD_CMI_55_HF)
	&hct_ili9881c_dsi_vdo_hd_cmi_55_hf,
#endif
#if defined(HCT_ILI9881C_DSI_VDO_HD_CMI_50_TZ)
	&hct_ili9881c_dsi_vdo_hd_cmi_50_tz,
#endif
#if defined(HCT_ILI9881C_DSI_VDO_HD_AUO_55_XZX)
	&hct_ili9881c_dsi_vdo_hd_auo_55_xzx,
#endif
#if defined(HCT_NT35521_DSI_VDO_HD_BOE_55_HR)
	&hct_nt35521_dsi_vdo_hd_boe_55_hr,
#endif
#if defined(HCT_RM67120_DSI_VDO_HD_AUO_55_AMOLED)
	&hct_rm67120_dsi_vdo_hd_auo_55_amoled,
#endif
#if defined(HCT_OTM1282A_DSI_VDO_HD_AUO_50_KL)
	&hct_otm1282a_dsi_vdo_hd_auo_50_kl,
#endif
#if defined(HCT_JD9365_DSI_VDO_HD_QM_55_HZ)
	&hct_jd9365_dsi_vdo_hd_qm_55_hz,
#endif
#if defined(HCT_ILI9881C_DSI_VDO_HD_CPT_50_CW)
	&hct_ili9881c_dsi_vdo_hd_cpt_50_cw,
#endif
#if defined(HCT_JD9361_DSI_VDO_HD_IVO_50_BX)
	&hct_jd9361_dsi_vdo_hd_ivo_50_bx,
#endif
#if defined(HCT_NT35596_DSI_VDO_HD_JDI_60_YKL)
	&hct_nt35596_dsi_vdo_hd_jdi_60_ykl,
#endif
#if defined(HCT_NT35532_DSI_VDO_FHD_SHARP_55_SH)
	&hct_nt35532_dsi_vdo_fhd_sharp_55_sh,
#endif
#if defined(HCT_NT35532_DSI_VDO_FHD_SHARP_50_SH)
	&hct_nt35532_dsi_vdo_fhd_sharp_50_sh,
#endif
#if defined(HCT_R61322A_DSI_VDO_FHD_55_OT)
	&hct_r61322a_dsi_vdo_fhd_55_ot,
#endif
#if defined(HCT_ILI9881C_DSI_VDO_HD_HSD_499_YKL)
	&hct_ili9881c_dsi_vdo_hd_hsd_499_ykl,
#endif
#if defined(HCT_RM68200_DSI_VDO_HD_CMI_50_YKL)
	&hct_rm68200_dsi_vdo_hd_cmi_50_ykl,
#endif
#if defined(HCT_RM68200_DSI_VDO_HD_HSD_50_XSJ)
	&hct_rm68200_dsi_vdo_hd_hsd_50_xsj,
#endif
#if defined(HCT_NT35521_DSI_VDO_HD_BOE_55_XLD)
	&hct_nt35521_dsi_vdo_hd_boe_55_xld,
#endif
#if defined(HCT_NT35521_DSI_VDO_HD_TM_55_XLD)
	&hct_nt35521_dsi_vdo_hd_tm_55_xld,
#endif
#if defined(HCT_RM68200_DSI_VDO_HD_AUO_50_YKL)
	&hct_rm68200_dsi_vdo_hd_auo_50_ykl,
#endif
#if defined(HCT_HX8394F_DSI_VDO_HD_CMI)
	&hct_hx8394f_dsi_vdo_hd_cmi,
#endif
#if defined(HCT_ILI9881_DSI_VDO_HD_CPT)
        &hct_ili9881_dsi_vdo_hd_cpt,
#endif

#if defined(HCT_JD9365_DSI_VDO_HD_CPT_50_XQ)
	&hct_jd9365_dsi_vdo_hd_cpt_50_xq,
#endif
#if defined(HCT_ST7701_DSI_VDO_FWVGA_CMI_50_XQ)
	&hct_st7701_dsi_vdo_fwvga_cmi_50_xq,
#endif
#if defined(HCT_ILI9881C_DSI_VDO_HD_AUO_50_CW)
	&hct_ili9881c_dsi_vdo_hd_auo_50_cw,
#endif
#if defined(HCT_RM68200_DSI_VDO_HD_TM_55_XLD)
	&hct_rm68200_dsi_vdo_hd_tm_55_xld,
#endif
#if defined(HCT_FL11281_DSI_VDO_HD_AUO_50_GT)
	&hct_fl11281_dsi_vdo_hd_auo_50_gt,
#endif
#if defined(HCT_FL10802_DSI_VDO_FWVGA_IVO_50_GT)
	&hct_fl10802_dsi_vdo_fwvga_ivo_50_gt,
#endif
#if defined(HCT_ST7701_DSI_VDO_FWVGA_IVO_50_BX)
	&hct_st7701_dsi_vdo_fwvga_ivo_50_bx,
#endif
#if defined(HCT_JD9161_DSI_VDO_FWVGA_BOE_50_GT)
	&hct_jd9161_dsi_vdo_fwvga_boe_50_gt,
#endif
#if defined(HCT_NT35521_DSI_VDO_HD_BOE_50_XLD_5DAI)
	&hct_nt35521_dsi_vdo_hd_boe_50_xld_5Dai,
#endif
#if defined(HCT_SH1386_DSI_VDO_HD_HX_55_HX)
	&hct_sh1386_dsi_vdo_hd_hx_55_hx,
#endif
#if defined(HCT_ILI9881C_DSI_VDO_HD_HSD_55_FC)
	&hct_ili9881c_dsi_vdo_hd_hsd_55_fc,
#endif
#if defined(HCT_ILI9881C_DSI_VDO_HD_CPT_55_OT)
	&hct_ili9881c_dsi_vdo_hd_cpt_55_ot,
#endif
#endif
};

#if defined(MTK_LCM_DEVICE_TREE_SUPPORT)
unsigned char lcm_name_list[][128] = {
#if defined(HX8392A_DSI_CMD)
	"hx8392a_dsi_cmd",
#endif

#if defined(HX8392A_DSI_VDO)
	"hx8392a_vdo_cmd",
#endif

#if defined(HX8392A_DSI_CMD_FWVGA)
	"hx8392a_dsi_cmd_fwvga",
#endif

#if defined(OTM9608_QHD_DSI_CMD)
	"otm9608a_qhd_dsi_cmd",
#endif

#if defined(OTM9608_QHD_DSI_VDO)
	"otm9608a_qhd_dsi_vdo",
#endif

#if defined(R63417_FHD_DSI_CMD_TRULY_NT50358)
	"r63417_fhd_dsi_cmd_truly_nt50358_drv",
#endif

#if defined(R63417_FHD_DSI_CMD_TRULY_NT50358_QHD)
	"r63417_fhd_dsi_cmd_truly_nt50358_qhd_drv",
#endif

#if defined(R63417_FHD_DSI_VDO_TRULY_NT50358)
	"r63417_fhd_dsi_vdo_truly_nt50358_drv",
#endif


};
#endif

#define LCM_COMPILE_ASSERT(condition) LCM_COMPILE_ASSERT_X(condition, __LINE__)
#define LCM_COMPILE_ASSERT_X(condition, line) LCM_COMPILE_ASSERT_XX(condition, line)
#define LCM_COMPILE_ASSERT_XX(condition, line) char assertion_failed_at_line_##line[(condition) ? 1 : -1]

unsigned int lcm_count = sizeof(lcm_driver_list) / sizeof(LCM_DRIVER *);
LCM_COMPILE_ASSERT(0 != sizeof(lcm_driver_list) / sizeof(LCM_DRIVER *));
#if defined(NT35520_HD720_DSI_CMD_TM) | defined(NT35520_HD720_DSI_CMD_BOE) | \
	defined(NT35521_HD720_DSI_VDO_BOE) | defined(NT35521_HD720_DSI_VIDEO_TM)
static unsigned char lcd_id_pins_value = 0xFF;

/**
 * Function:       which_lcd_module_triple
 * Description:    read LCD ID PIN status,could identify three status:highlowfloat
 * Input:           none
 * Output:         none
 * Return:         LCD ID1|ID0 value
 * Others:
 */
unsigned char which_lcd_module_triple(void)
{
	unsigned char  high_read0 = 0;
	unsigned char  low_read0 = 0;
	unsigned char  high_read1 = 0;
	unsigned char  low_read1 = 0;
	unsigned char  lcd_id0 = 0;
	unsigned char  lcd_id1 = 0;
	unsigned char  lcd_id = 0;
	/*Solve Coverity scan warning : check return value*/
	unsigned int ret = 0;

	/*only recognise once*/
	if (0xFF != lcd_id_pins_value)
		return lcd_id_pins_value;

	/*Solve Coverity scan warning : check return value*/
	ret = mt_set_gpio_mode(GPIO_DISP_ID0_PIN, GPIO_MODE_00);
	if (0 != ret)
		LCD_DEBUG("ID0 mt_set_gpio_mode fail\n");

	ret = mt_set_gpio_dir(GPIO_DISP_ID0_PIN, GPIO_DIR_IN);
	if (0 != ret)
		LCD_DEBUG("ID0 mt_set_gpio_dir fail\n");

	ret = mt_set_gpio_pull_enable(GPIO_DISP_ID0_PIN, GPIO_PULL_ENABLE);
	if (0 != ret)
		LCD_DEBUG("ID0 mt_set_gpio_pull_enable fail\n");

	ret = mt_set_gpio_mode(GPIO_DISP_ID1_PIN, GPIO_MODE_00);
	if (0 != ret)
		LCD_DEBUG("ID1 mt_set_gpio_mode fail\n");

	ret = mt_set_gpio_dir(GPIO_DISP_ID1_PIN, GPIO_DIR_IN);
	if (0 != ret)
		LCD_DEBUG("ID1 mt_set_gpio_dir fail\n");

	ret = mt_set_gpio_pull_enable(GPIO_DISP_ID1_PIN, GPIO_PULL_ENABLE);
	if (0 != ret)
		LCD_DEBUG("ID1 mt_set_gpio_pull_enable fail\n");

	/*pull down ID0 ID1 PIN*/
	ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN, GPIO_PULL_DOWN);
	if (0 != ret)
		LCD_DEBUG("ID0 mt_set_gpio_pull_select->Down fail\n");

	ret = mt_set_gpio_pull_select(GPIO_DISP_ID1_PIN, GPIO_PULL_DOWN);
	if (0 != ret)
		LCD_DEBUG("ID1 mt_set_gpio_pull_select->Down fail\n");

	/* delay 100ms , for discharging capacitance*/
	mdelay(100);
	/* get ID0 ID1 status*/
	low_read0 = mt_get_gpio_in(GPIO_DISP_ID0_PIN);
	low_read1 = mt_get_gpio_in(GPIO_DISP_ID1_PIN);
	/* pull up ID0 ID1 PIN */
	ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN, GPIO_PULL_UP);
	if (0 != ret)
		LCD_DEBUG("ID0 mt_set_gpio_pull_select->UP fail\n");

	ret = mt_set_gpio_pull_select(GPIO_DISP_ID1_PIN, GPIO_PULL_UP);
	if (0 != ret)
		LCD_DEBUG("ID1 mt_set_gpio_pull_select->UP fail\n");

	/* delay 100ms , for charging capacitance */
	mdelay(100);
	/* get ID0 ID1 status */
	high_read0 = mt_get_gpio_in(GPIO_DISP_ID0_PIN);
	high_read1 = mt_get_gpio_in(GPIO_DISP_ID1_PIN);

	if (low_read0 != high_read0) {
		/*float status , pull down ID0 ,to prevent electric leakage*/
		ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN, GPIO_PULL_DOWN);
		if (0 != ret)
			LCD_DEBUG("ID0 mt_set_gpio_pull_select->Down fail\n");

		lcd_id0 = LCD_HW_ID_STATUS_FLOAT;
	} else if ((LCD_HW_ID_STATUS_LOW == low_read0) && (LCD_HW_ID_STATUS_LOW == high_read0)) {
		/*low status , pull down ID0 ,to prevent electric leakage*/
		ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN, GPIO_PULL_DOWN);
		if (0 != ret)
			LCD_DEBUG("ID0 mt_set_gpio_pull_select->Down fail\n");

		lcd_id0 = LCD_HW_ID_STATUS_LOW;
	} else if ((LCD_HW_ID_STATUS_HIGH == low_read0) && (LCD_HW_ID_STATUS_HIGH == high_read0)) {
		/*high status , pull up ID0 ,to prevent electric leakage*/
		ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN, GPIO_PULL_UP);
		if (0 != ret)
			LCD_DEBUG("ID0 mt_set_gpio_pull_select->UP fail\n");

		lcd_id0 = LCD_HW_ID_STATUS_HIGH;
	} else {
		LCD_DEBUG(" Read LCD_id0 error\n");
		ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN, GPIO_PULL_DISABLE);
		if (0 != ret)
			LCD_DEBUG("ID0 mt_set_gpio_pull_select->Disbale fail\n");

		lcd_id0 = LCD_HW_ID_STATUS_ERROR;
	}


	if (low_read1 != high_read1) {
		/*float status , pull down ID1 ,to prevent electric leakage*/
		ret = mt_set_gpio_pull_select(GPIO_DISP_ID1_PIN, GPIO_PULL_DOWN);
		if (0 != ret)
			LCD_DEBUG("ID1 mt_set_gpio_pull_select->Down fail\n");

		lcd_id1 = LCD_HW_ID_STATUS_FLOAT;
	} else if ((LCD_HW_ID_STATUS_LOW == low_read1) && (LCD_HW_ID_STATUS_LOW == high_read1)) {
		/*low status , pull down ID1 ,to prevent electric leakage*/
		ret = mt_set_gpio_pull_select(GPIO_DISP_ID1_PIN, GPIO_PULL_DOWN);
		if (0 != ret)
			LCD_DEBUG("ID1 mt_set_gpio_pull_select->Down fail\n");

		lcd_id1 = LCD_HW_ID_STATUS_LOW;
	} else if ((LCD_HW_ID_STATUS_HIGH == low_read1) && (LCD_HW_ID_STATUS_HIGH == high_read1)) {
		/*high status , pull up ID1 ,to prevent electric leakage*/
		ret = mt_set_gpio_pull_select(GPIO_DISP_ID1_PIN, GPIO_PULL_UP);
		if (0 != ret)
			LCD_DEBUG("ID1 mt_set_gpio_pull_select->UP fail\n");

		lcd_id1 = LCD_HW_ID_STATUS_HIGH;
	} else {

		LCD_DEBUG(" Read LCD_id1 error\n");
		ret = mt_set_gpio_pull_select(GPIO_DISP_ID1_PIN, GPIO_PULL_DISABLE);
		if (0 != ret)
			LCD_DEBUG("ID1 mt_set_gpio_pull_select->Disable fail\n");

		lcd_id1 = LCD_HW_ID_STATUS_ERROR;
	}
#ifdef BUILD_LK
	dprintf(CRITICAL, "which_lcd_module_triple,lcd_id0:%d\n", lcd_id0);
	dprintf(CRITICAL, "which_lcd_module_triple,lcd_id1:%d\n", lcd_id1);
#else
	LCD_DEBUG("which_lcd_module_triple,lcd_id0:%d\n", lcd_id0);
	LCD_DEBUG("which_lcd_module_triple,lcd_id1:%d\n", lcd_id1);
#endif
	lcd_id =  lcd_id0 | (lcd_id1 << 2);

#ifdef BUILD_LK
	dprintf(CRITICAL, "which_lcd_module_triple,lcd_id:%d\n", lcd_id);
#else
	LCD_DEBUG("which_lcd_module_triple,lcd_id:%d\n", lcd_id);
#endif

	lcd_id_pins_value = lcd_id;
	return lcd_id;
}
#endif
