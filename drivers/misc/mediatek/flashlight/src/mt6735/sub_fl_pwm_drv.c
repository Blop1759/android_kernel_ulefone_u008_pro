#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/time.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <mach/upmu_sw.h>
#include <linux/of.h>
#include <sub_fl_pwm_drv.h>
#if  __HCT_SUB_FLASHLIGHT_PWM_SUPPORT__
static kal_bool active_flag = KAL_FALSE;



U16 sub_flash_duty;
U16 sub_torch_duty;

struct pwm_spec_config sub_FL_pwm_config = {
//    .pwm_no = PWM_NO,
    .mode = PWM_MODE_OLD,
    .clk_div = CLK_DIV32,//CLK_DIV16: 147.7kHz    CLK_DIV32: 73.8kHz
    .clk_src = PWM_CLK_OLD_MODE_BLOCK,
    .pmic_pad = false,
    .PWM_MODE_OLD_REGS.IDLE_VALUE = IDLE_FALSE,
    .PWM_MODE_OLD_REGS.GUARD_VALUE = GUARD_FALSE,
    .PWM_MODE_OLD_REGS.GDURATION = 0,
    .PWM_MODE_OLD_REGS.WAVE_NUM = 0,
    .PWM_MODE_OLD_REGS.DATA_WIDTH = 10,
    .PWM_MODE_OLD_REGS.THRESH = 5,
};

int sub_FL_shutdown(void)
{
		int ret = 0;
	if(active_flag == KAL_FALSE)
		return ret;
	mt_pwm_disable(sub_FL_pwm_config.pwm_no, false);
	flashlight_gpio_output(FL_SUB_PMW_PIN, 0);
	mdelay(5);

	active_flag = KAL_FALSE;
	return ret;
}
int sub_FL_set_torch_mode(U16 duty)
{
	int ret = 0;

	if(active_flag == KAL_TRUE)
		return ret;
	flashlight_gpio_as_pwm(FL_SUB_PMW_PIN, 0);

	sub_FL_pwm_config.PWM_MODE_OLD_REGS.THRESH = duty;
	ret = pwm_set_spec_config(&sub_FL_pwm_config);	

	active_flag = KAL_TRUE;
	return ret;
}
int sub_FL_set_flash_mode(U16 duty)
{
	int ret = 0;

	if(active_flag == KAL_TRUE)
		return ret;
	flashlight_gpio_as_pwm(FL_SUB_PMW_PIN, 0);

	sub_FL_pwm_config.PWM_MODE_OLD_REGS.THRESH = duty;
	ret = pwm_set_spec_config(&sub_FL_pwm_config);	

	active_flag = KAL_TRUE;
	return ret;
}

int sub_FL_ioctr(U16 mode , U16 duty)
{
	PK_DBG("mode: %d , duty = %d\n",mode,duty);
	if( mode < MODE_MIN || mode > MODE_MAX)
		{
			PK_DBG("mode is err\n");
			return 1;
		}
	if (duty<0||duty>10)
		{
			PK_DBG("duty is err\n");
			return 1;
		}
	switch(mode){
		case SUB_FLASH_M:
			sub_FL_set_flash_mode(duty);
			break;
		case SUB_TORCH_M:
			sub_FL_set_torch_mode(duty);
			break;
		case SUB_PWD_M:
			sub_FL_shutdown();
			break;
		}
	return 0;
}

void sub_FL_get_dts_info(const struct of_device_id *maches )
{
        struct device_node *node1 = NULL;
        u32   pwm_number;
        node1 = of_find_matching_node(node1, maches);
    
          if (node1) 
          {
              of_property_read_u32(node1, "flashlight_sub_pwmnum", &pwm_number);
              sub_FL_pwm_config.pwm_no=pwm_number; 
              PK_DBG(" PWM_NO=%d!!!\n",pwm_number);
          }else
          {
              sub_FL_pwm_config.pwm_no=PWM3;
              PK_DBG("get_dts_info, error, pwm_no is error pls check\n");
          }
          sub_flash_duty= SUB_F_DUTY;
          sub_flash_duty= SUB_T_DUTY;
}

void sub_FL_as_pwm_Enable(int duty)
{
	if(duty > 0)//flashlight mode
		sub_FL_ioctr(SUB_FLASH_M, sub_flash_duty);
	else //torch mode
		sub_FL_ioctr(SUB_TORCH_M, sub_flash_duty);
}

void sub_FL_as_pwm_Disable(void)
{
	sub_FL_ioctr(SUB_PWD_M, 0);
}
#endif
