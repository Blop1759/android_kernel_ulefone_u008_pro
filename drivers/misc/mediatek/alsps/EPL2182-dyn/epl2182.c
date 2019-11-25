/* drivers/hwmon/mt6516/amit/epl2182.c - EPL2182 ALS/PS driver
 *
 * Author: MingHsien Hsieh <minghsien.hsieh@mediatek.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>
#include <linux/platform_device.h>
#include <linux/atomic.h>
#include <linux/io.h>
#include "epl2182.h"
#include <linux/input/mt.h>
#include <alsps.h>
#include "upmu_sw.h"
#include "upmu_common.h"
#include <linux/gpio.h>
#include <linux/of_irq.h>

#include <linux/wakelock.h>
#include <linux/sched.h>
//#define CONFIG_HAS_EARLYSUSPEND  1
#if defined(CONFIG_HAS_EARLYSUSPEND)
#include <linux/earlysuspend.h>
#endif
#include <alsps.h>
//#define CUSTOM_KERNEL_SENSORHUB  1
#ifdef CUSTOM_KERNEL_SENSORHUB
#include <SCP_sensorHub.h>
#endif
#include <linux/hct_board_config.h>
#define COMPATIABLE_NAME "mediatek,epl2182"
static long long int_top_time;
#if 0
#ifdef CONFIG_MTK_AUTO_DETECT_ALSPS
#define MTK_LTE         1   //4G
#else
#define MTK_LTE         0   //3G
#endif
#else
#define MTK_LTE         1   //4G
#endif
//#define MT6735

#if MTK_LTE
#include <alsps.h>
#endif
//#if defined(MT6735) || defined(MT6752)
#define POWER_NONE_MACRO MT65XX_POWER_NONE
//#endif
/*******************************************************************************/
#define LUX_PER_COUNT		1100              // 1100 = 1.1 * 1000
#define PS_DRIVE				EPL_DRIVE_120MA
#define POLLING_MODE_HS		0
#define EINT_API    1
#define PS_GES 0
#define ELAN_WRITE_CALI  1
#define QUEUE_RUN        0 //1: epl2182_polling_work always run
#define ALS_FACTORY_MODE  0 //for special customer
#define DYN_ENABLE      1
#define WAKE_LOCK_PATCH     0
#define PS_FIRST_REPORT     1

#if WAKE_LOCK_PATCH
int suspend_idx = 0;
#endif

//static int PS_INTT 				= EPL_INTT_PS_80; //4;

#if defined(__HCT_EPL2182_PS_INIT_VALUE__)
static int PS_INTT = __HCT_EPL2182_PS_INIT_VALUE__;
#else
static int PS_INTT = 4;
#endif
#if defined(__HCT_EPL2182_ALS_INIT_VALUE__)
static int ALS_INTT = __HCT_EPL2182_ALS_INIT_VALUE__;
#else
static int ALS_INTT = 7;
#endif

//static int HS_INTT 				= 0; // reset when enable
//#define HS_INTT_CENTER			EPL_INTT_PS_80 //EPL_INTT_PS_48
//static int HS_INTT 				= HS_INTT_CENTER;


#define PS_DELAY 			15
#define ALS_DELAY 			55
//#define HS_DELAY 			30

#define KEYCODE_LEFT			KEY_LEFTALT

#if DYN_ENABLE
#define DYN_H_OFFSET 	 	900
#define DYN_L_OFFSET		600
#define DYN_PS_CONDITION	30000
#endif

/******************************************************************************
*******************************************************************************/

#define TXBYTES 				2
#define RXBYTES 				2

#define PACKAGE_SIZE 		2
#define I2C_RETRY_COUNT 	10
static struct mutex sensor_mutex;

#if ELAN_WRITE_CALI
struct _epl_ps_als_factory
{
    bool cal_file_exist;
    bool cal_finished;
    u16 ps_cal_h;
    u16 ps_cal_l;
    char s1[16];
    char s2[16];
};
#endif

#if ALS_FACTORY_MODE
bool    als_factory_flag = false;
#define ALS_FACTORY_DELAY 			25 //2 cycle, 21.026ms
#endif

#define EPL2182_DEV_NAME     "EPL2182"
#define DRIVER_VERSION       "v2.06"
// for heart rate
//static bool change_int_time = false;
//static int hs_count=0;
//static int hs_idx=0;
//static int show_hs_raws_flag=0;
//static int hs_als_flag=0;

typedef struct _epl2182_raw_data
{
    u8 raw_bytes[PACKAGE_SIZE];
    u16 renvo;
    u16 ps_state;
#if DYN_ENABLE
	u16 ps_min_raw;
	u16 ps_sta;
	u16 ps_dyn_high;
	u16 ps_dyn_low;
	bool ps_dny_ini_lock;
#endif
    u16 ps_raw;
    u16 ps_ch0_raw;
    u16 als_ch0_raw;
    u16 als_ch1_raw;
    u16 als_lux;
    //u16 hs_data[200];
    bool ps_suspend_flag;
#if ELAN_WRITE_CALI
    struct _epl_ps_als_factory ps_als_factory;
#endif
} epl2182_raw_data;

#if ELAN_WRITE_CALI
#define PS_CAL_FILE_PATH	"/data/data/com.eminent.ps.calibration/xtalk_cal"  //PS Calbration file path
static int PS_h_offset = 3000;
static int PS_l_offset = 2000;
static int PS_MAX_XTALK = 50000;
#endif

#if PS_FIRST_REPORT
static bool ps_first_flag = true;
#define PS_MAX_CT  10000
#endif
/*----------------------------------------------------------------------------*/
#define APS_TAG                  "[ALS/PS] "
#define APS_FUN(f)               printk(KERN_INFO APS_TAG"%s\n", __FUNCTION__)
#define APS_ERR(fmt, args...)    printk(KERN_ERR  APS_TAG"%s %d : "fmt, __FUNCTION__, __LINE__, ##args)
#define APS_LOG(fmt, args...)    printk(KERN_INFO APS_TAG fmt, ##args)
#define APS_DBG(fmt, args...)    printk(KERN_INFO fmt, ##args)
#define FTM_CUST_ALSPS "/data/epl2182"

/*----------------------------------------------------------------------------*/
static struct i2c_client *epl2182_i2c_client = NULL;
static struct alsps_hw alsps_cust;
static struct alsps_hw *hw = &alsps_cust;

/*----------------------------------------------------------------------------*/
static const struct i2c_device_id epl2182_i2c_id[] = {{"EPL2182",0},{}};
//static struct i2c_board_info __initdata i2c_EPL2182= { I2C_BOARD_INFO("EPL2182", (0X92>>1))};
/*the adapter id & i2c address will be available in customization*/
//static unsigned short epl2182_force[] = {0x00, 0x92, I2C_CLIENT_END, I2C_CLIENT_END};
//static const unsigned short *const epl2182_forces[] = { epl2182_force, NULL };
//static struct i2c_client_address_data epl2182_addr_data = { .forces = epl2182_forces,};


/*----------------------------------------------------------------------------*/
static int epl2182_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int epl2182_i2c_remove(struct i2c_client *client);
//static int epl2182_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info);


/*----------------------------------------------------------------------------*/
static int epl2182_i2c_suspend(struct i2c_client *client, pm_message_t msg);
static int epl2182_i2c_resume(struct i2c_client *client);

static irqreturn_t epl2182_eint_func(int irq, void *desc);
static int set_psensor_intr_threshold(uint16_t low_thd, uint16_t high_thd);
void epl2182_restart_polling(void);
int epl2182_ps_operate(void* self, uint32_t command, void* buff_in, int size_in,
                       void* buff_out, int size_out, int* actualout);

static struct epl2182_priv *g_epl2182_ptr = NULL;


/*----------------------------------------------------------------------------*/
typedef enum
{
    CMC_TRC_ALS_DATA 	= 0x0001,
    CMC_TRC_PS_DATA 	= 0X0002,
    CMC_TRC_EINT    		= 0x0004,
    CMC_TRC_IOCTL   		= 0x0008,
    CMC_TRC_I2C     		= 0x0010,
    CMC_TRC_CVT_ALS 	= 0x0020,
    CMC_TRC_CVT_PS  		= 0x0040,
    CMC_TRC_DEBUG   		= 0x0800,
} CMC_TRC;

/*----------------------------------------------------------------------------*/
typedef enum
{
    CMC_BIT_ALS    = 1,
    CMC_BIT_PS     = 2,
    //CMC_BIT_HS  		= 8,
} CMC_BIT;

/*----------------------------------------------------------------------------*/
struct epl2182_i2c_addr      /*define a series of i2c slave address*/
{
    u8  write_addr;
    u8  ps_thd;     /*PS INT threshold*/
};

/*----------------------------------------------------------------------------*/
struct epl2182_priv
{
    struct alsps_hw  *hw;
    struct i2c_client *client;
    struct delayed_work  eint_work;
    struct delayed_work  polling_work;
    struct input_dev *input_dev;
#if EINT_API
    struct device_node *irq_node;
    int		irq;
#endif
    /*i2c address group*/
    struct epl2182_i2c_addr  addr;

    //int 		polling_mode_hs;
    int		ir_type;

    /*misc*/
    atomic_t    trace;
    atomic_t    als_suspend;
    atomic_t    ps_suspend;
    //atomic_t	hs_suspend;

    /*data*/
    u16		lux_per_count;
    ulong       enable;         /*record HAL enalbe status*/
    ulong       pending_intr;   /*pending interrupt*/

    /*data*/
    u16         als_level_num;
    u16         als_value_num;
    u32         als_level[C_CUST_ALS_LEVEL-1];
    u32         als_value[C_CUST_ALS_LEVEL];

    /*early suspend*/
#if defined(CONFIG_HAS_EARLYSUSPEND)
    struct early_suspend    early_drv;
#endif
};


#ifdef CONFIG_OF
static const struct of_device_id alsps_of_match[] = {
        {.compatible = "mediatek,epl259x"},
        {.compatible = "mediatek,epl2182"},        
	{},
};
#endif
/*----------------------------------------------------------------------------*/
static struct i2c_driver epl2182_i2c_driver =
{
    .probe      	= epl2182_i2c_probe,
    .remove     = epl2182_i2c_remove,
    //.detect     	= epl2182_i2c_detect,
    .suspend    = epl2182_i2c_suspend,
    .resume     = epl2182_i2c_resume,
    .id_table   	= epl2182_i2c_id,
    //.address_data = &epl2182_addr_data,
    .driver = {
        //.owner          = THIS_MODULE,
        .name           = EPL2182_DEV_NAME,
       #ifdef CONFIG_OF
			.of_match_table = alsps_of_match,
	#endif
    },
};


static struct epl2182_priv *epl2182_obj = NULL;
#if !MTK_LTE
static struct platform_driver epl2182_alsps_driver;
#endif
static struct wake_lock ps_lock;
static epl2182_raw_data	gRawData;

#if MTK_LTE
static int alsps_init_flag =-1; // 0<==>OK -1 <==> fail
static int alsps_local_init(void);
static int alsps_remove(void);
static struct alsps_init_info epl2182_init_info = {
		.name = EPL2182_DEV_NAME,
		.init = alsps_local_init,
		.uninit = alsps_remove,

};
#endif

/*
//====================I2C write operation===============//
//regaddr: ELAN epl2182 Register Address.
//bytecount: How many bytes to be written to epl2182 register via i2c bus.
//txbyte: I2C bus transmit byte(s). Single byte(0X01) transmit only slave address.
//data: setting value.
//
// Example: If you want to write single byte to 0x1D register address, show below
//	      elan_epl2182_I2C_Write(client,0x1D,0x01,0X02,0xff);
//
*/
static int elan_epl2182_I2C_Write(struct i2c_client *client, uint8_t regaddr, uint8_t bytecount, uint8_t txbyte, uint8_t data)
{
    uint8_t buffer[2];
    int ret = 0;
    int retry;

    buffer[0] = (regaddr<<3) | bytecount ;
    buffer[1] = data;

    for(retry = 0; retry < I2C_RETRY_COUNT; retry++)
    {
        ret = i2c_master_send(client, buffer, txbyte);

        if (ret == txbyte)
        {
            break;
        }

        APS_ERR("i2c write error,TXBYTES %d\n",ret);
        mdelay(10);
    }


    if(retry>=I2C_RETRY_COUNT)
    {
        APS_ERR("i2c write retry over %d\n", I2C_RETRY_COUNT);
        return -EINVAL;
    }

    return ret;
}




/*
//====================I2C read operation===============//
*/
static int elan_epl2182_I2C_Read(struct i2c_client *client)
{
    uint8_t buffer[RXBYTES];
    int ret = 0, i =0;
    int retry;

    for(retry = 0; retry < I2C_RETRY_COUNT; retry++)
    {
        ret = i2c_master_recv(client, buffer, RXBYTES);

        if (ret == RXBYTES)
            break;

        APS_ERR("i2c read error,RXBYTES %d\r\n",ret);
        mdelay(10);
    }

    if(retry>=I2C_RETRY_COUNT)
    {
        APS_ERR("i2c read retry over %d\n", I2C_RETRY_COUNT);
        return -EINVAL;
    }

    for(i=0; i<PACKAGE_SIZE; i++)
        gRawData.raw_bytes[i] = buffer[i];

    return ret;
}
/*
static int elan_epl2182_I2C_Read_long(struct i2c_client *client, int bytecount)
{
    uint8_t buffer[bytecount];
    int ret = 0, i =0;
    int retry;

    for(retry = 0; retry < I2C_RETRY_COUNT; retry++)
    {
        ret = i2c_master_recv(client, buffer, bytecount);

        if (ret == bytecount)
            break;

        APS_ERR("i2c read error,RXBYTES %d\r\n",ret);
        mdelay(10);
    }

    if(retry>=I2C_RETRY_COUNT)
    {
        APS_ERR("i2c read retry over %d\n", I2C_RETRY_COUNT);
        return -EINVAL;
    }

    for(i=0; i<bytecount; i++)
        gRawData.raw_bytes[i] = buffer[i];

    return ret;
}

static int elan_calibration_atoi(char* s)
{
    int num=0,flag=0;
    int i=0;
    //printk("[ELAN] %s\n", __func__);
    for(i=0; i<=strlen(s); i++)
    {
        if(s[i] >= '0' && s[i] <= '9')
            num = num * 10 + s[i] -'0';
        else if(s[0] == '-' && i==0)
            flag =1;
        else
            break;
    }
    if(flag == 1)
        num = num * -1;
    return num;
}
*/
#if ELAN_WRITE_CALI
static int write_factory_calibration(struct epl2182_priv *epl_data, char* ps_data, int ps_cal_len)
{
    struct file *fp_cal;

	mm_segment_t fs;
	loff_t pos;

	APS_FUN();
    pos = 0;

	fp_cal = filp_open(PS_CAL_FILE_PATH, O_CREAT|O_RDWR|O_TRUNC, 0777);
	if (IS_ERR(fp_cal))
	{
		APS_ERR("[ELAN]create file error\n");
		return -1;
	}

    fs = get_fs();
	set_fs(KERNEL_DS);

	vfs_write(fp_cal, ps_data, ps_cal_len, &pos);

    filp_close(fp_cal, NULL);

	set_fs(fs);

	return 0;
}

static bool read_factory_calibration(struct epl2182_priv *epl_data)//struct epl2182_priv *epl_data
{
	//struct i2c_client *client = epl_data->client;
	struct file *fp;
	mm_segment_t fs;
	loff_t pos;
	char buffer[100]= {0};
	if(gRawData.ps_als_factory.cal_file_exist == 1)
	{
		fp = filp_open(PS_CAL_FILE_PATH, O_RDWR, S_IRUSR);

		if (IS_ERR(fp))
		{
			APS_ERR("NO PS calibration file(%d)\n", (int)IS_ERR(fp));
			gRawData.ps_als_factory.cal_file_exist =  0;
			return -EINVAL;
		}
		else
		{
		    int ps_hthr = 0, ps_lthr = 0;
			pos = 0;
			fs = get_fs();
			set_fs(KERNEL_DS);
			vfs_read(fp, buffer, sizeof(buffer), &pos);
			filp_close(fp, NULL);

			sscanf(buffer, "%d,%d", &ps_hthr, &ps_lthr);
			gRawData.ps_als_factory.ps_cal_h = ps_hthr;
			gRawData.ps_als_factory.ps_cal_l = ps_lthr;
			set_fs(fs);

			epl_data->hw->ps_threshold_high = gRawData.ps_als_factory.ps_cal_h;
		    epl_data->hw->ps_threshold_low = gRawData.ps_als_factory.ps_cal_l;

		    //atomic_set(&epl_data->ps_thd_val_high, epl_data->hw->ps_threshold_high);
	        //atomic_set(&epl_data->ps_thd_val_low, epl_data->hw->ps_threshold_low);
		}

		gRawData.ps_als_factory.cal_finished = 1;
	}
	return 0;
}

static int elan_epl2182_psensor_enable(struct epl2182_priv *epl_data, int enable);

static int elan_run_calibration(struct epl2182_priv *epl_data)
{

    struct epl2182_priv *obj = epl_data;
    u16 ch1;
    u32 ch1_all=0;
    int count =5;
    int i;
    //uint8_t read_data[2];
    int ps_hthr=0, ps_lthr=0;
    int ps_cal_len = 0;
    char ps_calibration[20];
    bool enable_ps = test_bit(CMC_BIT_PS, &obj->enable) && atomic_read(&obj->ps_suspend)==0;

    APS_FUN();

    if(!epl_data)
    {
        APS_ERR("epl2182_obj is null!!\n");
        return -EINVAL;
    }

    if(PS_MAX_XTALK < 0)
    {
        APS_ERR("Failed: PS_MAX_XTALK < 0 \r\n");
        return -EINVAL;
    }

    if(enable_ps == 0)
    {
        set_bit(CMC_BIT_PS, &obj->enable);
        epl2182_restart_polling();
        msleep(ALS_DELAY+2*PS_DELAY+50);
    }

    for(i=0; i<count; i++)
    {
        u16 ps_cali_raw;
        msleep(PS_DELAY);


        ps_cali_raw = gRawData.ps_raw;
	    APS_LOG("[%s]: gRawData.ps_raw=%d \r\n", __func__, gRawData.ps_raw);

		ch1_all = ch1_all+ ps_cali_raw;
    }

    ch1 = (u16)ch1_all/count;
    if(ch1 > PS_MAX_XTALK)
    {
        APS_ERR("Failed: ch1 > max_xtalk(%d) \r\n", ch1);
        return -EINVAL;
    }
    else if(ch1 <= 0)
    {
        APS_ERR("Failed: ch1 = 0\r\n");
        return -EINVAL;
    }

    ps_hthr = ch1 + PS_h_offset;
    ps_lthr = ch1 + PS_l_offset;

    ps_cal_len = sprintf(ps_calibration, "%d,%d", ps_hthr, ps_lthr);

    if(write_factory_calibration(obj, ps_calibration, ps_cal_len) < 0)
    {
        APS_ERR("[%s] create file error \n", __func__);
        return -EINVAL;
    }

    gRawData.ps_als_factory.ps_cal_h = ps_hthr;
    gRawData.ps_als_factory.ps_cal_l = ps_lthr;
    epl_data->hw->ps_threshold_high = ps_hthr;
    epl_data->hw->ps_threshold_low = ps_lthr;
    //atomic_set(&epl_data->ps_thd_val_high, epl_data->hw->ps_threshold_high);
    //atomic_set(&epl_data->ps_thd_val_low, epl_data->hw->ps_threshold_low);

    set_psensor_intr_threshold(epl_data->hw->ps_threshold_low,epl_data->hw->ps_threshold_high);

	APS_LOG("[%s]: ch1 = %d\n", __func__, ch1);

	return ch1;
}

#endif
#if PS_GES
static void epl2182_notify_event(void)
{
    struct input_dev *idev = epl2182_obj->input_dev;

    APS_LOG("  --> LEFT\n\n");
    input_report_key(idev, KEYCODE_LEFT, 1);
    input_report_key(idev,  KEYCODE_LEFT, 0);
    input_sync(idev);
}
#endif
/*
static void epl2182_hs_enable(struct epl2182_priv *epld, bool interrupt, bool full_enable)
{
    int ret;
    uint8_t regdata = 0;
    struct i2c_client *client = epld->client;

    if(full_enable)
    {

        regdata = PS_DRIVE | (interrupt? EPL_INT_FRAME_ENABLE : EPL_INT_DISABLE);
        ret = elan_epl2182_I2C_Write(client,REG_9,W_SINGLE_BYTE,0x02, regdata);

        regdata = EPL_SENSING_1_TIME | EPL_PS_MODE | EPL_L_GAIN | EPL_S_SENSING_MODE;
        ret = elan_epl2182_I2C_Write(client,REG_0,W_SINGLE_BYTE,0X02,regdata);

        regdata = HS_INTT<<4 | EPL_PST_1_TIME | EPL_12BIT_ADC;
        ret = elan_epl2182_I2C_Write(client,REG_1,W_SINGLE_BYTE,0X02,regdata);

        ret = elan_epl2182_I2C_Write(client,REG_7,W_SINGLE_BYTE,0X02,EPL_C_RESET);


    }

    ret = elan_epl2182_I2C_Write(client,REG_7,W_SINGLE_BYTE,0x02,EPL_C_START_RUN);

    if(epld->polling_mode_hs == 1){
        msleep(HS_DELAY);
    }

}
*/
#if DYN_ENABLE
static void dyn_ps_cal(struct epl2182_priv *epl_data)
{
	if((gRawData.ps_raw < gRawData.ps_min_raw)
	&& (gRawData.ps_sta != 1)
	&& (gRawData.ps_ch0_raw <= DYN_PS_CONDITION))
	{
		gRawData.ps_min_raw = gRawData.ps_raw;
		epl_data->hw ->ps_threshold_low = gRawData.ps_raw + DYN_L_OFFSET;
		epl_data->hw ->ps_threshold_high = gRawData.ps_raw + DYN_H_OFFSET;
		set_psensor_intr_threshold(epl_data->hw ->ps_threshold_low,epl_data->hw ->ps_threshold_high);
		APS_LOG("dyn ps raw = %d, min = %d, ch0 = %d\n dyn h_thre = %d, l_thre = %d, ps_state = %d",
		gRawData.ps_raw, gRawData.ps_min_raw, gRawData.ps_ch0_raw,epl_data->hw ->ps_threshold_high,epl_data->hw ->ps_threshold_low, gRawData.ps_state);
	}
}
#endif

static int elan_epl2182_psensor_enable(struct epl2182_priv *epl_data, int enable)
{
    int ret = 0;
    int ps_state = 0;
    uint8_t regdata;
    struct i2c_client *client = epl_data->client;
#if !MTK_LTE
    hwm_sensor_data sensor_data;
#endif
    u8 ps_state_tmp;

    APS_LOG("[ELAN epl2182] %s enable = %d\n", __func__, enable);

    ret = elan_epl2182_I2C_Write(client,REG_9,W_SINGLE_BYTE,0x02,EPL_INT_DISABLE | PS_DRIVE);

    if(enable)
    {
        regdata = EPL_SENSING_2_TIME | EPL_PS_MODE | EPL_L_GAIN ;
        regdata = regdata | (epl_data->hw->polling_mode_ps == 0 ? EPL_C_SENSING_MODE : EPL_S_SENSING_MODE);
        ret = elan_epl2182_I2C_Write(client,REG_0,W_SINGLE_BYTE,0X02,regdata);

        regdata = PS_INTT<<4 | EPL_PST_1_TIME | EPL_10BIT_ADC;
        ret = elan_epl2182_I2C_Write(client,REG_1,W_SINGLE_BYTE,0X02,regdata);
#if ELAN_WRITE_CALI
        if(gRawData.ps_als_factory.cal_finished == 0 &&  gRawData.ps_als_factory.cal_file_exist ==1)
		    ret=read_factory_calibration(epl_data);

        APS_LOG("[ELAN epl2182] %s cal_finished = %d\n, cal_file_exist = %d\n", __func__, gRawData.ps_als_factory.cal_finished , gRawData.ps_als_factory.cal_file_exist);
#endif
#if !DYN_ENABLE
        set_psensor_intr_threshold(epl_data->hw ->ps_threshold_low,epl_data->hw ->ps_threshold_high);
#endif
        ret = elan_epl2182_I2C_Write(client,REG_7,W_SINGLE_BYTE,0X02,EPL_C_RESET);
        ret = elan_epl2182_I2C_Write(client,REG_7,W_SINGLE_BYTE,0x02,EPL_C_START_RUN);

        msleep(PS_DELAY);

        elan_epl2182_I2C_Write(client,REG_13,R_SINGLE_BYTE,0x01,0);
        elan_epl2182_I2C_Read(client);
        ps_state_tmp = !((gRawData.raw_bytes[0]&0x04)>>2);
        APS_LOG("[%s]:real ps_state = %d\n", __func__, ps_state_tmp);
#if DYN_ENABLE
        gRawData.ps_sta = ((gRawData.raw_bytes[0]&0x02)>>1);
#endif
        elan_epl2182_I2C_Write(client,REG_14,R_TWO_BYTE,0x01,0x00);
	    elan_epl2182_I2C_Read(client);
		gRawData.ps_ch0_raw = ((gRawData.raw_bytes[1]<<8) | gRawData.raw_bytes[0]);

	    elan_epl2182_I2C_Write(client,REG_16,R_TWO_BYTE,0x01,0x00);
	    elan_epl2182_I2C_Read(client);
		gRawData.ps_raw = ((gRawData.raw_bytes[1]<<8) | gRawData.raw_bytes[0]);
#if DYN_ENABLE

        dyn_ps_cal(epl_data);
		APS_LOG("dyn k ps raw = %d, ch0 = %d\n, ps_state = %d",	gRawData.ps_raw, gRawData.ps_ch0_raw, ps_state);
#endif
        APS_LOG("[%s]:gRawData.ps_raw=%d \r\n", __func__, gRawData.ps_raw);
        if(epl_data->hw->polling_mode_ps == 0)
        {
            if(test_bit(CMC_BIT_ALS, &epl_data->enable))
            {
                APS_LOG("[%s]: ALS+PS mode \r\n", __func__);
                if((ps_state_tmp==0 && gRawData.ps_raw > epl_data->hw->ps_threshold_high) ||
                    (ps_state_tmp==1 && gRawData.ps_raw < epl_data->hw->ps_threshold_low))
                {
                    APS_LOG("change ps_state(ps_state_tmp=%d, gRawData.ps_state=%d) \r\n", ps_state_tmp, gRawData.ps_state);
                    ps_state = ps_state_tmp;
                }
                else
                {
                    ps_state = gRawData.ps_state;
                }
            }
            else
            {
                ps_state = ps_state_tmp;
                APS_LOG("[%s]: PS only \r\n", __func__);
            }

#if PS_FIRST_REPORT
            if((gRawData.ps_state != ps_state) || ps_first_flag == true)
#else
            if(gRawData.ps_state != ps_state)
#endif
            {
                gRawData.ps_state = ps_state;
#if PS_FIRST_REPORT
                if(ps_first_flag == true)
                {
                    ps_first_flag = false;
                    if(gRawData.ps_raw > PS_MAX_CT)
                        ps_state = 0;
                    else
                        ps_state = 1;
                    APS_LOG("[%s]: PS_FIRST_REPORT, ps_state=%d, ps_raw=%d \r\n", __func__, ps_state, gRawData.ps_raw);
                }
#endif
#if PS_GES
                if( gRawData.ps_state==0)
                    epl2182_notify_event();
#endif
#if MTK_LTE
                gRawData.ps_state = ps_state;
                ps_report_interrupt_data(ps_state);
#else
                sensor_data.values[0] = ps_state;
                sensor_data.value_divide = 1;
                sensor_data.status = SENSOR_STATUS_ACCURACY_MEDIUM;
                //let up layer to know
                hwmsen_get_interrupt_data(ID_PROXIMITY, &sensor_data);
#endif
                elan_epl2182_I2C_Write(client,REG_9,W_SINGLE_BYTE,0x02,EPL_INT_ACTIVE_LOW|PS_DRIVE);
                APS_LOG("[%s]: Driect report ps status .............\r\n", __func__);
                //APS_LOG("[%s]: EPL_INT_FRAME_ENABLE .............\r\n", __func__);
            }
            else
            {
                elan_epl2182_I2C_Write(client,REG_9,W_SINGLE_BYTE,0x02,EPL_INT_ACTIVE_LOW|PS_DRIVE);
                APS_LOG("[%s]: EPL_INT_ACTIVE_LOW .............\r\n", __func__);
            }
        }

    }
    else
    {
        regdata = EPL_SENSING_2_TIME | EPL_PS_MODE | EPL_L_GAIN | EPL_S_SENSING_MODE;
        ret = elan_epl2182_I2C_Write(client,REG_0,W_SINGLE_BYTE,0X02,regdata);
    }

    if(ret<0)
    {
        APS_ERR("[ELAN epl2182 error]%s: ps enable %d fail\n",__func__,ret);
    }
    else
    {
        ret = 0;
    }

    return ret;
}


static int elan_epl2182_lsensor_enable(struct epl2182_priv *epl_data, int enable)
{
    int ret = 0;
    uint8_t regdata;
    //int mode;
    struct i2c_client *client = epl_data->client;

    APS_LOG("[ELAN epl2182] %s enable = %d\n", __func__, enable);

    if(enable)
    {
        regdata = EPL_INT_DISABLE;
        ret = elan_epl2182_I2C_Write(client,REG_9,W_SINGLE_BYTE,0x02, regdata);

#if ALS_FACTORY_MODE
        if(als_factory_flag == true)
        {
            regdata = EPL_S_SENSING_MODE | EPL_SENSING_2_TIME | EPL_ALS_MODE | EPL_AUTO_GAIN;
        }
        else
        {
            regdata = EPL_S_SENSING_MODE | EPL_SENSING_8_TIME | EPL_ALS_MODE | EPL_AUTO_GAIN;
        }
#else
        regdata = EPL_S_SENSING_MODE | EPL_SENSING_8_TIME | EPL_ALS_MODE | EPL_AUTO_GAIN;
#endif
        ret = elan_epl2182_I2C_Write(client,REG_0,W_SINGLE_BYTE,0X02,regdata);

        regdata = ALS_INTT<<4 | EPL_PST_1_TIME | EPL_10BIT_ADC;
        ret = elan_epl2182_I2C_Write(client,REG_1,W_SINGLE_BYTE,0X02,regdata);

        ret = elan_epl2182_I2C_Write(client,REG_10,W_SINGLE_BYTE,0X02,EPL_GO_MID);
        ret = elan_epl2182_I2C_Write(client,REG_11,W_SINGLE_BYTE,0x02,EPL_GO_LOW);

        ret = elan_epl2182_I2C_Write(client,REG_7,W_SINGLE_BYTE,0X02,EPL_C_RESET);
        ret = elan_epl2182_I2C_Write(client,REG_7,W_SINGLE_BYTE,0x02,EPL_C_START_RUN);
#if ALS_FACTORY_MODE
        if(als_factory_flag == true)
        {
            msleep(ALS_FACTORY_DELAY);
        }
        else
#endif
        {
            msleep(ALS_DELAY);
        }
    }

    if(ret<0)
    {
        APS_ERR("[ELAN epl2182 error]%s: als_enable %d fail\n",__func__,ret);
    }
    else
    {
        ret = 0;
    }

    return ret;
}
/*
static void epl2182_read_hs(void)
{
    mutex_lock(&sensor_mutex);
    struct epl2182_priv *epld = epl2182_obj;
    struct i2c_client *client = epld->client;
    int max_frame = 200;
    int idx = hs_idx+hs_count;
    u16 data;


    elan_epl2182_I2C_Write(client,REG_16,R_TWO_BYTE,0x01,0x00);
    elan_epl2182_I2C_Read_long(client, 2);
    data=(gRawData.raw_bytes[1]<<8)|gRawData.raw_bytes[0];


    if(data>60800&& HS_INTT>HS_INTT_CENTER-5)
    {
        HS_INTT--;
        change_int_time=true;
    }
    else if(data>6400 && data <25600 && HS_INTT<HS_INTT_CENTER+5)
    {
        HS_INTT++;
        change_int_time=true;
    }
    else
    {
        change_int_time=false;

        if(idx>=max_frame)
            idx-=max_frame;

        gRawData.hs_data[idx] = data;

        if(hs_count>=max_frame)
        {
            hs_idx++;
            if(hs_idx>=max_frame)
                hs_idx=0;
        }

        hs_count++;
        if(hs_count>=max_frame)
            hs_count=max_frame;
    }
    mutex_unlock(&sensor_mutex);

}
*/
static int epl2182_get_als_value(struct epl2182_priv *obj, u16 als)
{
    int idx;
    int report_type = 1; //0:table, 1:lux
    int lux = 0;

    lux = (als * obj->lux_per_count)/1000;
    for(idx = 0; idx < obj->als_level_num; idx++)
    {
        if(lux < obj->hw->als_level[idx])
        {
            break;
        }
    }

    if(idx >= obj->als_value_num)
    {
        APS_ERR("exceed range\n");
        idx = obj->als_value_num - 1;
    }

    if(report_type == 0)
    {
        gRawData.als_lux = obj->hw->als_value[idx];
        APS_LOG("ALS: %05d => %05d\n", als, obj->hw->als_value[idx]);
        return gRawData.als_lux;
    }
    else
    {
        gRawData.als_lux = lux;
        APS_LOG("ALS: %05d => %05d (lux=%d)\n", als, obj->hw->als_value[idx], lux);
        return gRawData.als_lux;
    }
}


static int set_psensor_intr_threshold(uint16_t low_thd, uint16_t high_thd)
{
    int ret = 0;
    struct epl2182_priv *epld = epl2182_obj;
    struct i2c_client *client = epld->client;

    uint8_t high_msb ,high_lsb, low_msb, low_lsb;

    APS_LOG("%s\n", __func__);

    high_msb = (uint8_t) (high_thd >> 8);
    high_lsb = (uint8_t) (high_thd & 0x00ff);
    low_msb  = (uint8_t) (low_thd >> 8);
    low_lsb  = (uint8_t) (low_thd & 0x00ff);

    APS_LOG("%s: low_thd = %d, high_thd = %d \n",__func__, low_thd, high_thd);

    elan_epl2182_I2C_Write(client,REG_2,W_SINGLE_BYTE,0x02,high_lsb);
    elan_epl2182_I2C_Write(client,REG_3,W_SINGLE_BYTE,0x02,high_msb);
    elan_epl2182_I2C_Write(client,REG_4,W_SINGLE_BYTE,0x02,low_lsb);
    elan_epl2182_I2C_Write(client,REG_5,W_SINGLE_BYTE,0x02,low_msb);

    return ret;
}



/*----------------------------------------------------------------------------*/
static void epl2182_dumpReg(struct i2c_client *client)
{
    APS_LOG("chip id REG 0x00 value = %8x\n", i2c_smbus_read_byte_data(client, 0x00));
    APS_LOG("chip id REG 0x01 value = %8x\n", i2c_smbus_read_byte_data(client, 0x08));
    APS_LOG("chip id REG 0x02 value = %8x\n", i2c_smbus_read_byte_data(client, 0x10));
    APS_LOG("chip id REG 0x03 value = %8x\n", i2c_smbus_read_byte_data(client, 0x18));
    APS_LOG("chip id REG 0x04 value = %8x\n", i2c_smbus_read_byte_data(client, 0x20));
    APS_LOG("chip id REG 0x05 value = %8x\n", i2c_smbus_read_byte_data(client, 0x28));
    APS_LOG("chip id REG 0x06 value = %8x\n", i2c_smbus_read_byte_data(client, 0x30));
    APS_LOG("chip id REG 0x07 value = %8x\n", i2c_smbus_read_byte_data(client, 0x38));
    APS_LOG("chip id REG 0x09 value = %8x\n", i2c_smbus_read_byte_data(client, 0x48));
    APS_LOG("chip id REG 0x0D value = %8x\n", i2c_smbus_read_byte_data(client, 0x68));
    APS_LOG("chip id REG 0x0E value = %8x\n", i2c_smbus_read_byte_data(client, 0x70));
    APS_LOG("chip id REG 0x0F value = %8x\n", i2c_smbus_read_byte_data(client, 0x71));
    APS_LOG("chip id REG 0x10 value = %8x\n", i2c_smbus_read_byte_data(client, 0x80));
    APS_LOG("chip id REG 0x11 value = %8x\n", i2c_smbus_read_byte_data(client, 0x88));
    APS_LOG("chip id REG 0x13 value = %8x\n", i2c_smbus_read_byte_data(client, 0x98));

}


/*----------------------------------------------------------------------------*/
int hw8k_init_device(struct i2c_client *client)
{
    APS_LOG("hw8k_init_device.........\r\n");

    epl2182_i2c_client=client;

    APS_LOG(" I2C Addr==[0x%x],line=%d\n",epl2182_i2c_client->addr,__LINE__);

    return 0;
}

/*----------------------------------------------------------------------------*/
int epl2182_get_addr(struct alsps_hw *hw, struct epl2182_i2c_addr *addr)
{
    if(!hw || !addr)
    {
        return -EFAULT;
    }
    addr->write_addr= hw->i2c_addr[0];
    return 0;
}


/*----------------------------------------------------------------------------*/
static void epl2182_power(struct alsps_hw *hw, unsigned int on)
{
    static unsigned int power_on = 0;

    //APS_LOG("power %s\n", on ? "on" : "off");
//#ifndef MT6582
    if(hw->power_id != POWER_NONE_MACRO)
    {
        if(power_on == on)
        {
            APS_LOG("ignore power control: %d\n", on);
        }
        else if(on)
        {
            if(!hwPowerOn(hw->power_id, hw->power_vol, "EPL2182"))
            {
                APS_ERR("power on fails!!\n");
            }
        }
        else
        {
            if(!hwPowerDown(hw->power_id, "EPL2182"))
            {
                APS_ERR("power off fail!!\n");
            }
        }
    }
    power_on = on;
//#endif
}



/*----------------------------------------------------------------------------*/
static int epl2182_check_intr(struct i2c_client *client)
{
    struct epl2182_priv *obj = i2c_get_clientdata(client);
    int mode;

    //APS_LOG("int pin = %d\n", mt_get_gpio_in(GPIO_ALS_EINT_PIN));

    //if (mt_get_gpio_in(GPIO_ALS_EINT_PIN) == 1) /*skip if no interrupt*/
    //   return 0;

    elan_epl2182_I2C_Write(obj->client,REG_13,R_SINGLE_BYTE,0x01,0);
    elan_epl2182_I2C_Read(obj->client);
    mode = gRawData.raw_bytes[0]&(3<<4);
    APS_LOG("mode %d\n", mode);

    if(mode==0x10)// PS
    {
        set_bit(CMC_BIT_PS, &obj->pending_intr);
    }
    else
    {
        clear_bit(CMC_BIT_PS, &obj->pending_intr);
    }

#if 0
    if(atomic_read(&obj->trace) & CMC_TRC_DEBUG)
    {
        APS_LOG("check intr: 0x%08x\n", obj->pending_intr);
    }
#endif
    return 0;

}



/*----------------------------------------------------------------------------*/

int epl2182_read_als(struct i2c_client *client)
{
    struct epl2182_priv *obj = i2c_get_clientdata(client);
    uint8_t setting;
    u16 ch1;

    if(client == NULL)
    {
        APS_DBG("CLIENT CANN'T EQUL NULL\n");
        return -1;
    }

    elan_epl2182_I2C_Write(client,REG_13,R_SINGLE_BYTE,0x01,0);
    elan_epl2182_I2C_Read(client);
    setting = gRawData.raw_bytes[0];
    if((setting&(3<<4))!=0x00)
    {
        APS_ERR("read als data in wrong mode\n");
    }

    elan_epl2182_I2C_Write(obj->client,REG_16,R_TWO_BYTE,0x01,0x00);
    elan_epl2182_I2C_Read(obj->client);
    ch1 = (gRawData.raw_bytes[1]<<8) | gRawData.raw_bytes[0];

    // FIX: mid gain and low gain cannot report ff in auton gain
    if(setting>>7 ==0&& ch1==65535)
    {
        APS_LOG("setting %d, gain %x, als %d\n", setting, setting>>7,  ch1);
        APS_LOG("skip FF in auto gain\n\n");
    }
    else
    {
        gRawData.als_ch1_raw = ch1;
        APS_LOG("read als raw data = %d\n", gRawData.als_ch1_raw);
    }

    return 0;
}


/*----------------------------------------------------------------------------*/
long epl2182_read_ps(struct i2c_client *client, u16 *data)
{
    struct epl2182_priv *obj = i2c_get_clientdata(client);
    uint8_t setting;
    u16 new_ps_state;
    u16 ps_state;


    if(client == NULL)
    {
        APS_DBG("CLIENT CANN'T EQUL NULL\n");
        return -1;
    }

    elan_epl2182_I2C_Write(obj->client,REG_13,R_SINGLE_BYTE,0x01,0);
    elan_epl2182_I2C_Read(obj->client);
    setting = gRawData.raw_bytes[0];
    if((setting&(3<<4))!=0x10)
    {
        APS_ERR("read ps data in wrong mode\n");
    }
    new_ps_state= !((gRawData.raw_bytes[0]&0x04)>>2);
    APS_LOG("[%s]:real ps_state = %d\n", __func__, new_ps_state);
#if PS_GES
    if( new_ps_state==0 && gRawData.ps_state==1)
        epl2182_notify_event();
#endif

    elan_epl2182_I2C_Write(obj->client,REG_16,R_TWO_BYTE,0x01,0x00);
    elan_epl2182_I2C_Read(obj->client);
    gRawData.ps_raw = (gRawData.raw_bytes[1]<<8) | gRawData.raw_bytes[0];

    if(test_bit(CMC_BIT_ALS, &obj->enable))
    {
        APS_LOG("[%s]: ALS+PS mode \r\n", __func__);
        if((new_ps_state==0 && gRawData.ps_raw > obj->hw->ps_threshold_high) ||
            (new_ps_state==1 && gRawData.ps_raw < obj->hw->ps_threshold_low))
        {
            APS_LOG("[%s]:change ps_state(new_ps_state=%d, gRawData.ps_state=%d) \r\n", __func__, new_ps_state, gRawData.ps_state);
            ps_state = new_ps_state;
        }
        else
        {
            ps_state = gRawData.ps_state;
        }
    }
    else
    {
        ps_state= new_ps_state;
        APS_LOG("[%s]: PS only \r\n", __func__);
    }

    gRawData.ps_state = ps_state;




    *data = gRawData.ps_raw ;
    APS_LOG("read ps raw data = %d\n", gRawData.ps_raw);
    APS_LOG("read ps binary data = %d\n", gRawData.ps_state);

    return 0;
}

void epl2182_restart_polling(void)
{
    int queue_flag;
    struct epl2182_priv *obj = epl2182_obj;
    cancel_delayed_work(&obj->polling_work);
    queue_flag = work_busy(&((obj->polling_work).work));
    APS_LOG("[%s]: queue_flag=%d \r\n", __func__, queue_flag);
    if(queue_flag == 0)
    {
        schedule_delayed_work(&obj->polling_work, msecs_to_jiffies(50));
    }
    else
    {
        schedule_delayed_work(&obj->polling_work, msecs_to_jiffies(ALS_DELAY+2*PS_DELAY+50));
    }

}


void epl2182_polling_work(struct work_struct *work)
{
    struct epl2182_priv *obj = epl2182_obj;
    struct i2c_client *client = obj->client;

    bool enable_ps = test_bit(CMC_BIT_PS, &obj->enable) && atomic_read(&obj->ps_suspend)==0;
    bool enable_als = test_bit(CMC_BIT_ALS, &obj->enable) && atomic_read(&obj->als_suspend)==0;
    //bool enable_hs = test_bit(CMC_BIT_HS, &obj->enable) && atomic_read(&obj->hs_suspend)==0;
    APS_LOG("als / ps enable: %d / %d\n", enable_als, enable_ps);

    cancel_delayed_work(&obj->polling_work);
#if ALS_FACTORY_MODE
    if(enable_als && als_factory_flag == true)
    {
        schedule_delayed_work(&obj->polling_work, msecs_to_jiffies(ALS_FACTORY_DELAY+20));
        APS_LOG("[%s]: ALS_FACTORY_MODE............... \r\n", __func__);
    }
    else if((enable_ps&& obj->hw->polling_mode_ps == 1) || (enable_als==true && enable_ps==true) || (enable_als==true && enable_ps==false))
#else

#if QUEUE_RUN
    if((enable_ps==true) || (enable_als==true && enable_ps==true) || (enable_als==true && enable_ps==false))
#else
    if((enable_ps&& obj->hw->polling_mode_ps == 1) || (enable_als==true && enable_ps==true) || (enable_als==true && enable_ps==false) || (enable_ps==true && DYN_ENABLE))
#endif
#endif
    {
        schedule_delayed_work(&obj->polling_work, msecs_to_jiffies(ALS_DELAY+2*PS_DELAY+30));
    }


    if(enable_als)
    {
        elan_epl2182_lsensor_enable(obj, 1);
        epl2182_read_als(client);
    }
/*
    if 0
    {
        if (obj->polling_mode_hs==0)
        {
            epl2182_hs_enable(obj, true, true);
        }
        else
        {
            epl2182_read_hs();
            epl2182_hs_enable(obj, false, true);
            schedule_delayed_work(&obj->polling_work, msecs_to_jiffies(5));//HS_DELAY
        }
    }
	*/
    if(enable_ps)
    {
        elan_epl2182_psensor_enable(obj, 1);
        if(obj->hw->polling_mode_ps == 1)
        {
            epl2182_read_ps(client, &gRawData.ps_raw);
        }
    }
#if 0
    if(gRawData.ps_suspend_flag)
    {
        cancel_delayed_work(&obj->polling_work);
    }
#endif
    if(enable_als==false && enable_ps==false)
    {
        APS_LOG("disable sensor\n");
        elan_epl2182_lsensor_enable(obj, 1);
        cancel_delayed_work(&obj->polling_work);
        elan_epl2182_I2C_Write(client,REG_9,W_SINGLE_BYTE,0x02,EPL_INT_DISABLE);
        elan_epl2182_I2C_Write(client,REG_0,W_SINGLE_BYTE,0X02,EPL_S_SENSING_MODE);

    }

}
/*----------------------------------------------------------------------------*/
static irqreturn_t epl2182_eint_func(int irq, void *desc)
{
    struct epl2182_priv *obj = g_epl2182_ptr;

    // APS_LOG(" interrupt fuc\n");

    int_top_time = sched_clock();

    if(!obj)
    {
        return IRQ_HANDLED;
    }

    disable_irq_nosync(epl2182_obj->irq);

    schedule_delayed_work(&obj->eint_work, 0);

    return IRQ_HANDLED;
}



/*----------------------------------------------------------------------------*/
static void epl2182_eint_work(struct work_struct *work)
{
    struct epl2182_priv *epld = g_epl2182_ptr;
    int err;
#if !MTK_LTE
    hwm_sensor_data sensor_data;
#endif
    u8 ps_state;

/*
    if(test_bit(CMC_BIT_HS, &epld->enable) && atomic_read(&epld->hs_suspend)==0)
    {
        epl2182_read_hs();
        epl2182_hs_enable(epld, true, change_int_time);
    }
    */

    if(test_bit(CMC_BIT_PS, &epld->enable))
    {
        APS_LOG("xxxxx eint work\n");

        if((err = epl2182_check_intr(epld->client)))
        {
            APS_ERR("check intrs: %d\n", err);
        }

        if(epld->pending_intr)
        {
            elan_epl2182_I2C_Write(epld->client,REG_13,R_SINGLE_BYTE,0x01,0);
            elan_epl2182_I2C_Read(epld->client);
            ps_state = !((gRawData.raw_bytes[0]&0x04)>>2);
            APS_LOG("real ps_state = %d\n", ps_state);
            //gRawData.ps_state= !((gRawData.raw_bytes[0]&0x04)>>2);
            //APS_LOG("ps state = %d\n", gRawData.ps_state);

            elan_epl2182_I2C_Write(epld->client,REG_16,R_TWO_BYTE,0x01,0x00);
            elan_epl2182_I2C_Read(epld->client);
            gRawData.ps_raw = (gRawData.raw_bytes[1]<<8) | gRawData.raw_bytes[0];
            APS_LOG("ps raw_data = %d\n", gRawData.ps_raw);

            if(test_bit(CMC_BIT_ALS, &epld->enable))
            {
                APS_LOG("ALS+PS mode \r\n");
                if((ps_state==0 && gRawData.ps_raw > epld->hw->ps_threshold_high) ||
                    (ps_state==1 && gRawData.ps_raw < epld->hw->ps_threshold_low))
                {
                    APS_LOG("change ps_state(ps_state=%d, gRawData.ps_state=%d) \r\n", ps_state, gRawData.ps_state);
                    gRawData.ps_state = ps_state;
                }
            }
            else
            {
                gRawData.ps_state = ps_state;
                APS_LOG("PS only \r\n");
            }
#if MTK_LTE
#if PS_GES
            if( gRawData.ps_state==0)
                epl2182_notify_event();
#endif
            err = ps_report_interrupt_data(gRawData.ps_state);
            if(err != 0)
            {
                APS_ERR("epl2182_eint_work err: %d\n", err);
            }
#else
            sensor_data.values[0] = gRawData.ps_state;
            sensor_data.value_divide = 1;
            sensor_data.status = SENSOR_STATUS_ACCURACY_MEDIUM;
#if PS_GES
            if( gRawData.ps_state==0)
                epl2182_notify_event();
#endif
            //let up layer to know
            if((err = hwmsen_get_interrupt_data(ID_PROXIMITY, &sensor_data)))
            {
                APS_ERR("get interrupt data failed\n");
                APS_ERR("call hwmsen_get_interrupt_data fail = %d\n", err);
            }
#endif
        }

        elan_epl2182_I2C_Write(epld->client,REG_9,W_SINGLE_BYTE,0x02,EPL_INT_ACTIVE_LOW | PS_DRIVE);
        elan_epl2182_I2C_Write(epld->client,REG_7,W_SINGLE_BYTE,0x02,EPL_DATA_UNLOCK);
    }

#if EINT_API
#if defined(CONFIG_OF)
	enable_irq(epld->irq);
#endif
#endif
}



/*----------------------------------------------------------------------------*/
int epl2182_setup_eint(struct i2c_client *client)
{
   #ifdef CUSTOM_KERNEL_SENSORHUB
	int err = 0;

	err = SCP_sensorHub_rsp_registration(ID_PROXIMITY, alsps_irq_handler);
    #else				/* #ifdef CUSTOM_KERNEL_SENSORHUB */
	struct epl2182_priv *obj = i2c_get_clientdata(client);
	int ret;
	u32 ints[2] = { 0, 0 };
	struct pinctrl *pinctrl;
	struct pinctrl_state *pins_default;
	struct pinctrl_state *pins_cfg;
	struct platform_device *alsps_pdev = get_alsps_platformdev();

	APS_LOG("epl2182_setup_eint\n");

       g_epl2182_ptr = obj;
	   /*configure to GPIO function, external interrupt */
	 #ifndef FPGA_EARLY_PORTING
/* gpio setting */
	pinctrl = devm_pinctrl_get(&alsps_pdev->dev);
	if (IS_ERR(pinctrl)) {
		ret = PTR_ERR(pinctrl);
		APS_ERR("Cannot find alsps pinctrl!\n");
	}
	pins_default = pinctrl_lookup_state(pinctrl, "pin_default");
	if (IS_ERR(pins_default)) {
		ret = PTR_ERR(pins_default);
		APS_ERR("Cannot find alsps pinctrl default!\n");
	}

	pins_cfg = pinctrl_lookup_state(pinctrl, "pin_cfg");
	if (IS_ERR(pins_cfg)) {
		ret = PTR_ERR(pins_cfg);
		APS_ERR("Cannot find alsps pinctrl pin_cfg!\n");

	}
	pinctrl_select_state(pinctrl, pins_cfg);

	if (epl2182_obj->irq_node) {
		of_property_read_u32_array(epl2182_obj->irq_node, "debounce", ints,
					   ARRAY_SIZE(ints));
		gpio_request(ints[0], "p-sensor");
		gpio_set_debounce(ints[0], ints[1]);
		APS_LOG("ints[0] = %d, ints[1] = %d!!\n", ints[0], ints[1]);

		epl2182_obj->irq = irq_of_parse_and_map(epl2182_obj->irq_node, 0);
		APS_LOG("epl2182_obj->irq = %d\n", epl2182_obj->irq);
		if (!epl2182_obj->irq) {
			APS_ERR("irq_of_parse_and_map fail!!\n");
			return -EINVAL;
		}
		if (request_irq
		    (epl2182_obj->irq, epl2182_eint_func, IRQF_TRIGGER_NONE, "ALS-eint", NULL)) {
			APS_ERR("IRQ LINE NOT AVAILABLE!!\n");
			return -EINVAL;
		}
		enable_irq(epl2182_obj->irq);
	}
	else
	{
		APS_ERR("null irq node!!\n");
		return -EINVAL;
	}

	enable_irq(epl2182_obj->irq);
#endif				/* #ifndef FPGA_EARLY_PORTING */
#endif				/* #ifdef CUSTOM_KERNEL_SENSORHUB */

	return 0;
}




/*----------------------------------------------------------------------------*/
static int epl2182_init_client(struct i2c_client *client)
{
    struct epl2182_priv *obj = i2c_get_clientdata(client);
    int err=0;

    APS_LOG("[Agold spl] I2C Addr==[0x%x],line=%d\n",epl2182_i2c_client->addr,__LINE__);

    /*  interrupt mode */


    APS_FUN();

    if(obj->hw->polling_mode_ps == 0)
    {
	#if !EINT_API
		#if defined(MT6582) || defined(MT6592) || defined(MT6752) || defined(MT6735)
        	mt_eint_mask(CUST_EINT_ALS_NUM);
		#else
		mt65xx_eint_mask(CUST_EINT_ALS_NUM);
		#endif
	#endif
        if((err = epl2182_setup_eint(client)))
        {
            APS_ERR("setup eint: %d\n", err);
            return err;
        }
        APS_LOG("epl2182 interrupt setup\n");
    }


    if((err = hw8k_init_device(client)) != 0)
    {
        APS_ERR("init dev: %d\n", err);
        return err;
    }


    if((err = epl2182_check_intr(client)))
    {
        APS_ERR("check/clear intr: %d\n", err);
        return err;
    }


    /*  interrupt mode */
//if(obj->hw->polling_mode_ps == 0)
    //     mt65xx_eint_unmask(CUST_EINT_ALS_NUM);

    return err;
}


/*----------------------------------------------------------------------------*/
static ssize_t epl2182_show_reg(struct device_driver *ddri, char *buf)
{
    ssize_t len = 0;
    struct i2c_client *client = NULL;

    if(!epl2182_obj)
    {
        APS_ERR("epl2182_obj is null!!\n");
        return 0;
    }

   client = epl2182_obj->client;

    len += snprintf(buf+len, PAGE_SIZE-len, "chip id REG 0x00 value = %8x\n", i2c_smbus_read_byte_data(client, 0x00));
    len += snprintf(buf+len, PAGE_SIZE-len, "chip id REG 0x01 value = %8x\n", i2c_smbus_read_byte_data(client, 0x08));
    len += snprintf(buf+len, PAGE_SIZE-len, "chip id REG 0x02 value = %8x\n", i2c_smbus_read_byte_data(client, 0x10));
    len += snprintf(buf+len, PAGE_SIZE-len, "chip id REG 0x03 value = %8x\n", i2c_smbus_read_byte_data(client, 0x18));
    len += snprintf(buf+len, PAGE_SIZE-len, "chip id REG 0x04 value = %8x\n", i2c_smbus_read_byte_data(client, 0x20));
    len += snprintf(buf+len, PAGE_SIZE-len, "chip id REG 0x05 value = %8x\n", i2c_smbus_read_byte_data(client, 0x28));
    len += snprintf(buf+len, PAGE_SIZE-len, "chip id REG 0x06 value = %8x\n", i2c_smbus_read_byte_data(client, 0x30));
    len += snprintf(buf+len, PAGE_SIZE-len, "chip id REG 0x07 value = %8x\n", i2c_smbus_read_byte_data(client, 0x38));
    len += snprintf(buf+len, PAGE_SIZE-len, "chip id REG 0x09 value = %8x\n", i2c_smbus_read_byte_data(client, 0x48));
    len += snprintf(buf+len, PAGE_SIZE-len, "chip id REG 0x0D value = %8x\n", i2c_smbus_read_byte_data(client, 0x68));
    len += snprintf(buf+len, PAGE_SIZE-len, "chip id REG 0x0E value = %8x\n", i2c_smbus_read_byte_data(client, 0x70));
    len += snprintf(buf+len, PAGE_SIZE-len, "chip id REG 0x0F value = %8x\n", i2c_smbus_read_byte_data(client, 0x71));
    len += snprintf(buf+len, PAGE_SIZE-len, "chip id REG 0x10 value = %8x\n", i2c_smbus_read_byte_data(client, 0x80));
    len += snprintf(buf+len, PAGE_SIZE-len, "chip id REG 0x11 value = %8x\n", i2c_smbus_read_byte_data(client, 0x88));
    len += snprintf(buf+len, PAGE_SIZE-len, "chip id REG 0x13 value = %8x\n", i2c_smbus_read_byte_data(client, 0x98));

    return len;

}

/*----------------------------------------------------------------------------*/
static ssize_t epl2182_show_status(struct device_driver *ddri, char *buf)
{
    ssize_t len = 0;
    struct epl2182_priv *epld = epl2182_obj;
    //u16 ch0, ch1;

    if(!epl2182_obj)
    {
        APS_ERR("epl2182_obj is null!!\n");
        return 0;
    }
#if 0
    elan_epl2182_I2C_Write(epld->client,REG_7,W_SINGLE_BYTE,0x02,EPL_DATA_LOCK);

    elan_epl2182_I2C_Write(epld->client,REG_14,R_TWO_BYTE,0x01,0x00);
    elan_epl2182_I2C_Read(epld->client);
    ch0 = (gRawData.raw_bytes[1]<<8) | gRawData.raw_bytes[0];
    elan_epl2182_I2C_Write(epld->client,REG_16,R_TWO_BYTE,0x01,0x00);
    elan_epl2182_I2C_Read(epld->client);
    ch1 = (gRawData.raw_bytes[1]<<8) | gRawData.raw_bytes[0];

    elan_epl2182_I2C_Write(epld->client,REG_7,W_SINGLE_BYTE,0x02,EPL_DATA_UNLOCK);
#endif
    len += snprintf(buf+len, PAGE_SIZE-len, "chip is %s, ver is %s \n", EPL2182_DEV_NAME, DRIVER_VERSION);
    len += snprintf(buf+len, PAGE_SIZE-len, "als/ps int time is %d-%d\n",ALS_INTT, PS_INTT);
    len += snprintf(buf+len, PAGE_SIZE-len, "ch0 ch1 raw is %d-%d\n",gRawData.ps_ch0_raw, gRawData.ps_raw);
    len += snprintf(buf+len, PAGE_SIZE-len, "threshold is %d/%d\n",epld->hw->ps_threshold_low, epld->hw->ps_threshold_high);
    //len += snprintf(buf+len, PAGE_SIZE-len, "heart int time: %d\n", HS_INTT);
#if DYN_ENABLE
    len += snprintf(buf+len, PAGE_SIZE-len, "ps dyn K: ch0=%d, ps raw=%d, ps_state=%d \n", gRawData.ps_ch0_raw, gRawData.ps_raw, gRawData.ps_state);
    len += snprintf(buf+len, PAGE_SIZE-len, "ps dyn K: threshold=%d/%d \n", epld->hw->ps_threshold_low, epld->hw->ps_threshold_high);
#endif
    return len;
}

/*----------------------------------------------------------------------------*/
static ssize_t epl2182_show_renvo(struct device_driver *ddri, char *buf)
{
    ssize_t len = 0;

    APS_FUN();
    APS_LOG("gRawData.renvo=0x%x \r\n", gRawData.renvo);

    len += snprintf(buf+len, PAGE_SIZE-len, "%x",gRawData.renvo);

    return len;
}

/*----------------------------------------------------------------------------*/
static ssize_t epl2182_store_als_int_time(struct device_driver *ddri, const char *buf, size_t count)
{
    if(!epl2182_obj)
    {
        APS_ERR("epl2182_obj is null!!\n");
        return 0;
    }

    sscanf(buf, "%d", &ALS_INTT);
    APS_LOG("als int time is %d\n", ALS_INTT);
    return count;
}


/*----------------------------------------------------------------------------*/
static ssize_t epl2182_show_ps_cal_raw(struct device_driver *ddri, char *buf)
{
    struct epl2182_priv *obj = NULL;
    u16 ch1;
    u32 ch1_all=0;
    int count =1;
    int i;
    //uint8_t read_data[2];
    ssize_t len = 0;

    bool enable_ps = test_bit(CMC_BIT_PS, &obj->enable) && atomic_read(&obj->ps_suspend)==0;
    bool enable_als = test_bit(CMC_BIT_ALS, &obj->enable) && atomic_read(&obj->als_suspend)==0;
    APS_FUN();
    if(!epl2182_obj)
    {
        APS_ERR("epl2182_obj is null!!\n");
        return 0;
    }

	obj = epl2182_obj;

    if(enable_ps == 0 || enable_als == 0)
    {
        set_bit(CMC_BIT_ALS, &obj->enable);
        set_bit(CMC_BIT_PS, &obj->enable);
        epl2182_restart_polling();
        msleep(ALS_DELAY+2*PS_DELAY+30+50);
    }

    for(i=0; i<count; i++)
    {
        //elan_epl2182_psensor_enable(obj, 1);
        msleep(PS_DELAY);
        APS_LOG("epl2182_show_ps_cal_raw: gRawData.ps_raw=%d \r\n", gRawData.ps_raw);


		ch1_all = ch1_all+ gRawData.ps_raw;

    }

    ch1 = (u16)ch1_all/count;
	APS_LOG("epl2182_show_ps_cal_raw =  %d\n", ch1);

    len += snprintf(buf+len, PAGE_SIZE-len, "%d \r\n", ch1);

	return len;
}

static ssize_t epl2182_show_ps_threshold(struct device_driver *ddri, char *buf)
{
    ssize_t len = 0;
    //struct epl2182_priv *obj = epl2182_obj;
#if ELAN_WRITE_CALI
    len += snprintf(buf+len, PAGE_SIZE-len, "gRawData.ps_als_factory(H/L): %d/%d \r\n", gRawData.ps_als_factory.ps_cal_h, gRawData.ps_als_factory.ps_cal_l);
#endif
    len += snprintf(buf+len, PAGE_SIZE-len, "ps_threshold(H/L): %d/%d \r\n", epl2182_obj->hw->ps_threshold_high, epl2182_obj->hw->ps_threshold_low);
    return len;
}



/*----------------------------------------------------------------------------*/
static ssize_t epl2182_store_ps_int_time(struct device_driver *ddri, const char *buf, size_t count)
{
    if(!epl2182_obj)
    {
        APS_ERR("epl2182_obj is null!!\n");
        return 0;
    }
    sscanf(buf, "%d", &PS_INTT);
    APS_LOG("ps int time is %d\n", PS_INTT);
    return count;
}

/*----------------------------------------------------------------------------*/
static ssize_t epl2182_store_ps_threshold(struct device_driver *ddri, const char *buf, size_t count)
{
    if(!epl2182_obj)
    {
        APS_ERR("epl2182_obj is null!!\n");
        return 0;
    }
    sscanf(buf, "%d,%d", &epl2182_obj->hw->ps_threshold_low, &epl2182_obj->hw->ps_threshold_high);
#if ELAN_WRITE_CALI
    gRawData.ps_als_factory.ps_cal_h = epl2182_obj->hw->ps_threshold_high;
    gRawData.ps_als_factory.ps_cal_l = epl2182_obj->hw->ps_threshold_low;
#endif
    epl2182_restart_polling();
    return count;
}
/*
static ssize_t epl2182_store_hs_enable(struct device_driver *ddri, const char *buf, size_t count)
{
    uint16_t mode=0;
    struct epl2182_priv *obj = epl2182_obj;
    bool enable_als = test_bit(CMC_BIT_ALS, &obj->enable) && atomic_read(&obj->als_suspend)==0;
    APS_FUN();

    sscanf(buf, "%hu",&mode);


    if(mode){
        if(enable_als == true){
            atomic_set(&obj->als_suspend, 1);
            hs_als_flag = 1;

        }
        set_bit(CMC_BIT_HS, &obj->enable);
    }
    else{
        clear_bit(CMC_BIT_HS, &obj->enable);

        if(hs_als_flag == 1){
            atomic_set(&obj->als_suspend, 0);
            hs_als_flag = 0;

        }

    }
    if(mode)
    {
        //HS_INTT = EPL_INTT_PS_272;
        hs_idx=0;
        hs_count=0;
    }

    epl2182_restart_polling();
    return count;
}

static ssize_t epl2182_show_hs_raws(struct device_driver *ddri, char *buf)
{

    mutex_lock(&sensor_mutex);

    u16 *tmp = (u16*)buf;
    u16 length= hs_count;
    int byte_count=2+length*2;
    int i=0;
    int start = hs_idx;
    APS_FUN();
    tmp[0]= length;
    if(length == 0){
        tmp[0] = 1;
        length = 1;
        show_hs_raws_flag = 1;
    }
    for(i=start; i<length; i++){
        if(show_hs_raws_flag == 1){
            tmp[i+1] = 0;
            show_hs_raws_flag = 0;
        }
        else{
            tmp[i+1] = gRawData.hs_data[i];
        }

    }

    hs_count=0;
    hs_idx=0;
    mutex_unlock(&sensor_mutex);

    return byte_count;
}

static ssize_t epl2182_store_hs_polling(struct device_driver *ddri, const char *buf, size_t count)
{
    struct epl2182_priv *obj = epl2182_obj;
    APS_FUN();

    sscanf(buf, "%d",&obj->polling_mode_hs);

    APS_LOG("HS polling mode: %d\n", obj->polling_mode_hs);

    return count;
}
*/
#if ELAN_WRITE_CALI
static ssize_t epl2182_store_ps_w_calfile(struct device_driver *ddri, const char *buf, size_t count)
{
	struct epl2182_priv *obj = epl2182_obj;
    int ps_hthr=0, ps_lthr=0;
    int ps_cal_len = 0;
    char ps_calibration[20];
	APS_FUN();

	if(!epl2182_obj)
    {
        APS_ERR("epl2182_obj is null!!\n");
        return 0;
    }
    sscanf(buf, "%d,%d", &ps_hthr, &ps_lthr);

    ps_cal_len = sprintf(ps_calibration, "%d,%d", ps_hthr, ps_lthr);

    write_factory_calibration(obj, ps_calibration, ps_cal_len);
	return count;
}

static ssize_t epl2182_show_ps_run_cali(struct device_driver *ddri, char *buf)
{
	struct epl2182_priv *obj = epl2182_obj;
	ssize_t len = 0;
    int ret;

    APS_FUN();

    ret = elan_run_calibration(obj);

    len += snprintf(buf+len, PAGE_SIZE-len, "ret = %d\r\n", ret);

	return len;
}

#endif

static ssize_t epl2182_store_ps_polling(struct device_driver *ddri, const char *buf, size_t count)
{
	struct epl2182_priv *obj = epl2182_obj;
#if !MTK_LTE
    struct hwmsen_object obj_ps; //Arima alvinchen 20140922 added for ALS/PS cali (From Elan IceFang) +++
#endif
    if(!epl2182_obj)
    {
        APS_ERR("epl2182_obj is null!!\n");
        return 0;
    }
#if !MTK_LTE
	//Arima alvinchen 20140922 added for ALS/PS cali (From Elan IceFang) +++
    hwmsen_detach(ID_PROXIMITY);

    sscanf(buf, "%d", &obj->hw->polling_mode_ps);
    APS_LOG("ps polling mode is %d\n", obj->hw->polling_mode_ps);

    obj_ps.self = epl2182_obj;
    obj_ps.polling = obj->hw->polling_mode_ps;
    obj_ps.sensor_operate = epl2182_ps_operate;

    if(hwmsen_attach(ID_PROXIMITY, &obj_ps))    {
        APS_ERR("[%s]: attach fail !\n", __FUNCTION__);
    }
	//Arima alvinchen 20140922 added for ALS/PS cali (From Elan IceFang) ---
#else
    sscanf(buf, "%d", &obj->hw->polling_mode_ps);
    APS_LOG("ps polling mode is %d\n", obj->hw->polling_mode_ps);
#endif
    epl2182_restart_polling();
    return count;
}

//Arima alvinchen 20140922 added for ALS/PS cali (From Elan IceFang) +++
static ssize_t epl2182_store_ps_enable(struct device_driver *ddri, const char *buf, size_t count)
{
    int mode=0;
    struct epl2182_priv *obj = epl2182_obj;

    APS_FUN();

    sscanf(buf, "%d", &mode);

    if(mode)
        set_bit(CMC_BIT_PS, &obj->enable);
    else
        clear_bit(CMC_BIT_PS, &obj->enable);

    epl2182_restart_polling();


    return count;
}
static ssize_t epl2182_store_als_enable(struct device_driver *ddri, const char *buf, size_t count)
{
    int mode=0;
    struct epl2182_priv *obj = epl2182_obj;

    APS_FUN();

    sscanf(buf, "%d", &mode);

    if(mode)
    {
#if ALS_FACTORY_MODE
        als_factory_flag = false;
#endif
        set_bit(CMC_BIT_ALS, &obj->enable);
    }
    else
        clear_bit(CMC_BIT_ALS, &obj->enable);

    epl2182_restart_polling();


    return count;
}
static ssize_t epl_show_als_data(struct device_driver *ddri, char *buf)
{
    struct epl2182_priv *obj = epl2182_obj;
    ssize_t len = 0;
    bool enable_als = test_bit(CMC_BIT_ALS, &obj->enable) && atomic_read(&obj->als_suspend)==0;
    APS_FUN();
#if ALS_FACTORY_MODE
    als_factory_flag = true;
    clear_bit(CMC_BIT_PS, &obj->enable);    //disable PS
#endif
    if(enable_als == 0)
    {
        set_bit(CMC_BIT_ALS, &obj->enable);
        epl2182_restart_polling();
#if ALS_FACTORY_MODE
        msleep(ALS_FACTORY_DELAY+20);
#else
        msleep(ALS_DELAY+2*PS_DELAY+30+50);
#endif
    }

    APS_LOG("[%s]: ALS RAW = %d\n", __func__, gRawData.als_ch1_raw);
    len += snprintf(buf + len, PAGE_SIZE - len, "%d", gRawData.als_ch1_raw);
    return len;

}
static ssize_t epl_show_ps_data(struct device_driver *ddri, char *buf)
{
    struct epl2182_priv *obj = epl2182_obj;
    ssize_t len = 0;
    bool enable_ps = test_bit(CMC_BIT_PS, &obj->enable) && atomic_read(&obj->ps_suspend)==0;
    APS_FUN();
    if(enable_ps == 0)
    {
        set_bit(CMC_BIT_PS, &obj->enable);
        epl2182_restart_polling();
#if ALS_FACTORY_MODE
        msleep(ALS_FACTORY_DELAY+20);
#else
        msleep(ALS_DELAY+2*PS_DELAY+30+50);
#endif
    }

    APS_LOG("[%s]: PS RAW = %d\n", __func__, gRawData.ps_raw);
    len += snprintf(buf + len, PAGE_SIZE - len, "%d", gRawData.ps_raw);
    return len;

}
/*CTS --> S_IWUSR | S_IRUGO*/
/*----------------------------------------------------------------------------*/
static DRIVER_ATTR(elan_status, S_IWUSR | S_IRUGO, epl2182_show_status, NULL);
static DRIVER_ATTR(elan_reg, S_IWUSR | S_IRUGO, epl2182_show_reg, NULL);
static DRIVER_ATTR(elan_renvo, S_IWUSR | S_IRUGO, epl2182_show_renvo, NULL);
static DRIVER_ATTR(als_int_time, S_IWUSR | S_IRUGO, NULL, epl2182_store_als_int_time);
static DRIVER_ATTR(ps_cal_raw, S_IWUSR | S_IRUGO, epl2182_show_ps_cal_raw, NULL);
static DRIVER_ATTR(ps_int_time, S_IWUSR | S_IRUGO, NULL, epl2182_store_ps_int_time);
static DRIVER_ATTR(ps_threshold, S_IWUSR | S_IRUGO, epl2182_show_ps_threshold, epl2182_store_ps_threshold);
//static DRIVER_ATTR(hs_enable, S_IWUSR | S_IRUGO, NULL, epl2182_store_hs_enable);
//static DRIVER_ATTR(hs_raws,	S_IWUSR | S_IRUGO, epl2182_show_hs_raws, NULL);
//static DRIVER_ATTR(hs_polling, S_IWUSR | S_IRUGO, NULL, epl2182_store_hs_polling);
#if ELAN_WRITE_CALI
static DRIVER_ATTR(ps_calfile, S_IWUSR | S_IRUGO, NULL, epl2182_store_ps_w_calfile);
static DRIVER_ATTR(ps_run_cali, S_IWUSR | S_IRUGO, epl2182_show_ps_run_cali, NULL);
#endif
static DRIVER_ATTR(ps_polling, S_IWUSR | S_IRUGO, NULL, epl2182_store_ps_polling);
static DRIVER_ATTR(ps_enable, S_IWUSR | S_IRUGO, NULL, epl2182_store_ps_enable);
static DRIVER_ATTR(als_enable, S_IWUSR | S_IRUGO, NULL, epl2182_store_als_enable);
static DRIVER_ATTR(als_data,	 S_IWUSR | S_IRUGO, epl_show_als_data, NULL);
static DRIVER_ATTR(ps_data,	 S_IWUSR | S_IRUGO, epl_show_ps_data, NULL);

/*----------------------------------------------------------------------------*/
static struct driver_attribute * epl2182_attr_list[] =
{
    &driver_attr_elan_status,
    &driver_attr_elan_reg,
    &driver_attr_elan_renvo,
    &driver_attr_als_int_time,
    &driver_attr_ps_cal_raw,
    &driver_attr_ps_int_time,
    &driver_attr_ps_threshold,
    //&driver_attr_hs_enable,
    //&driver_attr_hs_raws,
    //&driver_attr_hs_polling,
#if ELAN_WRITE_CALI
    &driver_attr_ps_calfile,
    &driver_attr_ps_run_cali,
#endif
    &driver_attr_ps_polling,
    &driver_attr_ps_enable,
    &driver_attr_als_enable,
    &driver_attr_als_data,
    &driver_attr_ps_data,
};

/*----------------------------------------------------------------------------*/
static int epl2182_create_attr(struct device_driver *driver)
{
    int idx, err = 0;
    int num = (int)(sizeof(epl2182_attr_list)/sizeof(epl2182_attr_list[0]));
    if (driver == NULL)
    {
        return -EINVAL;
    }

    for(idx = 0; idx < num; idx++)
    {
        if((err = driver_create_file(driver, epl2182_attr_list[idx])))
        {
            APS_ERR("driver_create_file (%s) = %d\n", epl2182_attr_list[idx]->attr.name, err);
            break;
        }
    }
    return err;
}



/*----------------------------------------------------------------------------*/
static int epl2182_delete_attr(struct device_driver *driver)
{
    int idx ,err = 0;
    int num = (int)(sizeof(epl2182_attr_list)/sizeof(epl2182_attr_list[0]));

    if (!driver)
        return -EINVAL;

    for (idx = 0; idx < num; idx++)
    {
        driver_remove_file(driver, epl2182_attr_list[idx]);
    }

    return err;
}



/******************************************************************************
 * Function Configuration
******************************************************************************/
static int epl2182_open(struct inode *inode, struct file *file)
{
    file->private_data = epl2182_i2c_client;

    APS_FUN();

    if (!file->private_data)
    {
        APS_ERR("null pointer!!\n");
        return -EINVAL;
    }

    return nonseekable_open(inode, file);
}

/*----------------------------------------------------------------------------*/
static int epl2182_release(struct inode *inode, struct file *file)
{
    APS_FUN();
    file->private_data = NULL;
    return 0;
}

/*----------------------------------------------------------------------------*/
static long epl2182_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct i2c_client *client = (struct i2c_client*)file->private_data;
    struct epl2182_priv *obj = i2c_get_clientdata(client);
    int err = 0;
    void __user *ptr = (void __user*) arg;
    int dat;
    uint32_t enable;
    bool enable_ps = test_bit(CMC_BIT_PS, &obj->enable) && atomic_read(&obj->ps_suspend)==0;
    bool enable_als = test_bit(CMC_BIT_ALS, &obj->enable) && atomic_read(&obj->als_suspend)==0;


    APS_LOG("[%s], cmd = 0x%x........\r\n", __func__, cmd);
    switch (cmd)
    {
        case ALSPS_SET_PS_MODE:

            if(copy_from_user(&enable, ptr, sizeof(enable)))
            {
                err = -EFAULT;
                goto err_out;
            }

            APS_LOG("[%s]:ALSPS_SET_PS_MODE => enable=%d \r\n", __func__, enable);
            if(enable)
            {
#if DYN_ENABLE
		        gRawData.ps_min_raw=0xffff;
#endif
#if PS_FIRST_REPORT
                ps_first_flag = true;
#endif
                set_bit(CMC_BIT_PS, &obj->enable);
            }
            else
            {
                clear_bit(CMC_BIT_PS, &obj->enable);
            }
            epl2182_restart_polling();
            break;


        case ALSPS_GET_PS_MODE:

            enable=test_bit(CMC_BIT_PS, &obj->enable);
            if(copy_to_user(ptr, &enable, sizeof(enable)))
            {
                err = -EFAULT;
                goto err_out;
            }
            break;


        case ALSPS_GET_PS_DATA:

            if(enable_ps == 0 || enable_als == 0)
            {
                set_bit(CMC_BIT_PS, &obj->enable);
                set_bit(CMC_BIT_ALS, &obj->enable);
                epl2182_restart_polling();
            }
            dat = gRawData.ps_state;

            APS_LOG("ioctl ps state value = %d \n", dat);

            if(copy_to_user(ptr, &dat, sizeof(dat)))
            {
                err = -EFAULT;
                goto err_out;
            }

            break;


        case ALSPS_GET_PS_RAW_DATA:

            if(enable_ps == 0 || enable_als == 0)
            {
                set_bit(CMC_BIT_PS, &obj->enable);
                set_bit(CMC_BIT_ALS, &obj->enable);
                epl2182_restart_polling();
            }
            msleep(ALS_DELAY+2*PS_DELAY+50); //if getting ps rate is too fast, it is the same raw. it can reslove this issue.

            dat = gRawData.ps_raw;

            APS_LOG("ioctl ps raw value = %d \n", dat);

            if(copy_to_user(ptr, &dat, sizeof(dat)))
            {
                err = -EFAULT;
                goto err_out;
            }

            break;


        case ALSPS_SET_ALS_MODE:
            if(copy_from_user(&enable, ptr, sizeof(enable)))
            {
                err = -EFAULT;
                goto err_out;
            }
            if(enable)
                set_bit(CMC_BIT_ALS, &obj->enable);
            else
                clear_bit(CMC_BIT_ALS, &obj->enable);

            break;



        case ALSPS_GET_ALS_MODE:
            enable=test_bit(CMC_BIT_ALS, &obj->enable);
            if(copy_to_user(ptr, &enable, sizeof(enable)))
            {
                err = -EFAULT;
                goto err_out;
            }
            break;



        case ALSPS_GET_ALS_DATA:

            if(enable_als == 0)
            {
                set_bit(CMC_BIT_ALS, &obj->enable);
                epl2182_restart_polling();
            }

            dat = epl2182_get_als_value(obj, gRawData.als_ch1_raw);
            APS_LOG("ioctl get als data = %d\n", dat);

            if(copy_to_user(ptr, &dat, sizeof(dat)))
            {
                err = -EFAULT;
                goto err_out;
            }

            break;


        case ALSPS_GET_ALS_RAW_DATA:

            if(enable_als == 0)
            {
                set_bit(CMC_BIT_ALS, &obj->enable);
                epl2182_restart_polling();
            }

            dat = gRawData.als_ch1_raw;
            APS_LOG("ioctl get als raw data = %d\n", dat);

            if(copy_to_user(ptr, &dat, sizeof(dat)))
            {
                err = -EFAULT;
                goto err_out;
            }
            break;


        default:
            APS_ERR("%s not supported = 0x%04x \r\n", __FUNCTION__, cmd);
            err = -ENOIOCTLCMD;
            break;
    }

err_out:
    return err;
}


/*----------------------------------------------------------------------------*/
static struct file_operations epl2182_fops =
{
    .owner = THIS_MODULE,
    .open = epl2182_open,
    .release = epl2182_release,
    .unlocked_ioctl = epl2182_unlocked_ioctl,
};


/*----------------------------------------------------------------------------*/
static struct miscdevice epl2182_device =
{
    .minor = MISC_DYNAMIC_MINOR,
    .name = "als_ps",
    .fops = &epl2182_fops,
};


/*----------------------------------------------------------------------------*/
static int epl2182_i2c_suspend(struct i2c_client *client, pm_message_t msg)
{
    //struct epl2182_priv *obj = i2c_get_clientdata(client);
    //int err;
    APS_FUN();
#if WAKE_LOCK_PATCH
    if(suspend_idx == 0)
	{
	    APS_LOG("[%s]: wake_lock! \r\n", __func__);
        wake_lock(&ps_lock);
	}

	if(test_bit(CMC_BIT_PS, &obj->enable) && suspend_idx == 0)
	{
		cancel_delayed_work(&obj->polling_work);
		msleep(PS_DELAY * 2 + ALS_DELAY + 50);
		APS_LOG("[%s]:cancel_delayed_work\n", __func__);
	}
	else
	{
        APS_LOG("[%s]: nothing \n", __func__);
	}

	if(suspend_idx == 0)
	{
	    APS_LOG("[%s]: wake_unlock! \r\n", __func__);
        wake_unlock(&ps_lock);
	}
    suspend_idx++;
#endif
#if 0
    if(msg.event == PM_EVENT_SUSPEND)
    {
        if(!obj)
        {
            APS_ERR("null pointer!!\n");
            return -EINVAL;
        }

        atomic_set(&obj->als_suspend, 1);
        atomic_set(&obj->ps_suspend, 1);
        atomic_set(&obj->hs_suspend, 1);

        if(test_bit(CMC_BIT_PS,  &obj->enable) && obj->hw->polling_mode_ps==0)
            epl2182_restart_polling();

        epl2182_power(obj->hw, 0);
    }

    if(test_bit(CMC_BIT_PS, &obj->enable) == 0)
    {
        elan_epl2182_lsensor_enable(obj, 1);
    }
#endif
    return 0;

}



/*----------------------------------------------------------------------------*/
static int epl2182_i2c_resume(struct i2c_client *client)
{
    //struct epl2182_priv *obj = i2c_get_clientdata(client);
#if !MTK_LTE
	hwm_sensor_data sensor_data;
#endif
    //int err;
    APS_FUN();
#if WAKE_LOCK_PATCH

	if(gRawData.ps_state)
	{
	    wake_lock_timeout(&ps_lock, 2*HZ);
	    APS_LOG("[%s]: ps_state (%d) \r\n", __func__, gRawData.ps_state);
#if MTK_LTE
        ps_report_interrupt_data(gRawData.ps_state);
#else
        sensor_data.values[0] = gRawData.ps_state;
        sensor_data.value_divide = 1;
        sensor_data.status = SENSOR_STATUS_ACCURACY_MEDIUM;
        //let up layer to know
        hwmsen_get_interrupt_data(ID_PROXIMITY, &sensor_data);
#endif
	}

#endif
#if 0
    if(!obj)
    {
        APS_ERR("null pointer!!\n");
        return -EINVAL;
    }

    epl2182_power(obj->hw, 1);

    msleep(50);

    atomic_set(&obj->ps_suspend, 0);

    if(err = epl2182_init_client(client))
    {
        APS_ERR("initialize client fail!!\n");
        return err;
    }

    if(obj->hw->polling_mode_ps == 0)
        epl2182_setup_eint(client);


    if(test_bit(CMC_BIT_PS,  &obj->enable))
        epl2182_restart_polling();
#endif
    return 0;
}


#if defined(CONFIG_HAS_EARLYSUSPEND)
static void epl2182_early_suspend(struct early_suspend *h)
{
    /*early_suspend is only applied for ALS*/
    struct epl2182_priv *obj = container_of(h, struct epl2182_priv, early_drv);
    int err;
    APS_FUN();

    if(!obj)
    {
        APS_ERR("null pointer!!\n");
        return;
    }
#if WAKE_LOCK_PATCH
    suspend_idx = 0;
#endif
    atomic_set(&obj->als_suspend, 1);
#if 0
    if(obj->hw->polling_mode_ps == 0)
		gRawData.ps_suspend_flag = true;
#endif
    if(test_bit(CMC_BIT_PS, &obj->enable) == 0)
    {
        elan_epl2182_lsensor_enable(obj, 1);
    }
}
static void epl2182_late_resume(struct early_suspend *h)
{
    /*late_resume is only applied for ALS*/
    struct epl2182_priv *obj = container_of(h, struct epl2182_priv, early_drv);
    int err;
    APS_FUN();

    if(!obj)
    {
        APS_ERR("null pointer!!\n");
        return;
    }

    atomic_set(&obj->als_suspend, 0);
    atomic_set(&obj->ps_suspend, 0);
    atomic_set(&obj->hs_suspend, 0);
#if 0
    if(obj->hw->polling_mode_ps == 0)
		gRawData.ps_suspend_flag = false;
#endif
#if WAKE_LOCK_PATCH
    if(suspend_idx != 0 || (test_bit(CMC_BIT_ALS, &obj->enable)==1))
    {
        APS_LOG("[%s]: restart polling suspend_idx=%d............ \r\n", __func__, suspend_idx);
        epl2182_restart_polling();
    }
#else
    if(test_bit(CMC_BIT_ALS, &obj->enable)==1)
        epl2182_restart_polling();
#endif
}
#endif

#if MTK_LTE /*MTK_LTE start .................*/
/*--------------------------------------------------------------------------------*/
static int als_open_report_data(int open)
{
	//should queuq work to report event if  is_report_input_direct=true
	return 0;
}
/*--------------------------------------------------------------------------------*/
// if use  this typ of enable , Gsensor only enabled but not report inputEvent to HAL
static int als_enable_nodata(int en)
{
    struct epl2182_priv *obj = epl2182_obj;
    //bool enable_ps = test_bit(CMC_BIT_PS, &obj->enable) && atomic_read(&obj->ps_suspend)==0;
    bool enable_als = test_bit(CMC_BIT_ALS, &obj->enable) && atomic_read(&obj->als_suspend)==0;
	if(!obj)
	{
		APS_ERR("obj is null!!\n");
		return -1;
	}
	APS_LOG("[%s] als enable en = %d\n", __func__, en);

    if(enable_als != en)
    {
        if(en)
        {
#if ALS_FACTORY_MODE
            als_factory_flag = false;
#endif
            set_bit(CMC_BIT_ALS, &obj->enable);
            epl2182_restart_polling();
        }
        else
        {
            clear_bit(CMC_BIT_ALS, &obj->enable);
        }
    }
	return 0;
}
/*--------------------------------------------------------------------------------*/
static int als_set_delay(u64 ns)
{
	return 0;
}
/*--------------------------------------------------------------------------------*/
static int als_get_data(int* value, int* status)
{
	int err = 0;
	struct epl2182_priv *obj = epl2182_obj;
	if(!obj)
	{
		APS_ERR("obj is null!!\n");
		return -1;
	}

    *value = epl2182_get_als_value(obj, gRawData.als_ch1_raw);
    *status = SENSOR_STATUS_ACCURACY_MEDIUM;
    APS_LOG("[%s]:*value = %d\n", __func__, *value);

	return err;
}
/*--------------------------------------------------------------------------------*/
// if use  this typ of enable , Gsensor should report inputEvent(x, y, z ,stats, div) to HAL
static int ps_open_report_data(int open)
{
	//should queuq work to report event if  is_report_input_direct=true
	return 0;
}
/*--------------------------------------------------------------------------------*/
// if use  this typ of enable , Gsensor only enabled but not report inputEvent to HAL
static int ps_enable_nodata(int en)
{
	struct epl2182_priv *obj = epl2182_obj;
    //struct i2c_client *client = obj->client;
    bool enable_ps = test_bit(CMC_BIT_PS, &obj->enable) && atomic_read(&obj->ps_suspend)==0;

    APS_LOG("ps enable = %d\n", en);
    if(enable_ps != en)
    {
        if(en)
        {
#if !WAKE_LOCK_PATCH
            wake_lock(&ps_lock);
#endif
            if(obj->hw->polling_mode_ps==0)
                gRawData.ps_state = 2; //if ps enable, report PS status for psensor_enable function
            set_bit(CMC_BIT_PS, &obj->enable);
#if DYN_ENABLE
		    gRawData.ps_min_raw=0xffff;
#endif
#if PS_FIRST_REPORT
            ps_first_flag = true;
#endif
        }
        else
        {
            clear_bit(CMC_BIT_PS, &obj->enable);
#if !WAKE_LOCK_PATCH
            wake_unlock(&ps_lock);
#endif
        }
        epl2182_restart_polling();
    }

	return 0;

}
/*--------------------------------------------------------------------------------*/
static int ps_set_delay(u64 ns)
{
	return 0;
}
/*--------------------------------------------------------------------------------*/
static int ps_get_data(int* value, int* status)
{

    int err = 0;
    //struct epl2182_priv *obj = epl2182_obj;
    //struct i2c_client *client = obj->client;

    APS_LOG("---SENSOR_GET_DATA---\n\n");

    *value = gRawData.ps_state;
    *status = SENSOR_STATUS_ACCURACY_MEDIUM;

    APS_LOG("[%s]:*value = %d\n", __func__, *value);

	return err;
}
/*----------------------------------------------------------------------------*/

#else /*MTK_LTE*/

/*----------------------------------------------------------------------------*/
int epl2182_ps_operate(void* self, uint32_t command, void* buff_in, int size_in,
                       void* buff_out, int size_out, int* actualout)
{
    int err = 0;
    int value;
    hwm_sensor_data* sensor_data;
    struct epl2182_priv *obj = (struct epl2182_priv *)self;

    APS_LOG("epl2182_ps_operate command = %x\n",command);
    switch (command)
    {
        case SENSOR_DELAY:
            if((buff_in == NULL) || (size_in < sizeof(int)))
            {
                APS_ERR("Set delay parameter error!\n");
                err = -EINVAL;
            }
            break;


        case SENSOR_ENABLE:
            if((buff_in == NULL) || (size_in < sizeof(int)))
            {
                APS_ERR("Enable sensor parameter error!\n");
                err = -EINVAL;
            }
            else
            {
                value = *(int *)buff_in;
                APS_LOG("ps enable = %d\n", value);


                if(value)
                {
#if !WAKE_LOCK_PATCH
                    wake_lock(&ps_lock);
#endif
                    if(obj->hw->polling_mode_ps==0)
                        gRawData.ps_state=2;    //if ps enable, report PS status for psensor_enable function
#if DYN_ENABLE
		            gRawData.ps_min_raw=0xffff;
#endif
#if PS_FIRST_REPORT
                    ps_first_flag = true;
#endif
                    set_bit(CMC_BIT_PS, &obj->enable);
                }
                else
                {
                    clear_bit(CMC_BIT_PS, &obj->enable);

                    //elan_epl2182_lsensor_enable(obj, 1);
#if !WAKE_LOCK_PATCH
                    wake_unlock(&ps_lock);
#endif

                }
                epl2182_restart_polling();
            }

            break;



        case SENSOR_GET_DATA:
            APS_LOG(" get ps data !!!!!!\n");
            if((buff_out == NULL) || (size_out< sizeof(hwm_sensor_data)))
            {
                APS_ERR("get sensor data parameter error!\n");
                err = -EINVAL;
            }
            else
            {
                APS_LOG("---SENSOR_GET_DATA---\n\n");

                sensor_data = (hwm_sensor_data *)buff_out;
                sensor_data->values[0] =gRawData.ps_state;
                sensor_data->values[1] =gRawData.ps_raw;
                sensor_data->value_divide = 1;
                sensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;
            }
            break;


        default:
            APS_ERR("proxmy sensor operate function no this parameter %d!\n", command);
            err = -1;
            break;



    }

    return err;

}



int epl2182_als_operate(void* self, uint32_t command, void* buff_in, int size_in,
                        void* buff_out, int size_out, int* actualout)
{
    int err = 0;
    int value;
    hwm_sensor_data* sensor_data;
    struct epl2182_priv *obj = (struct epl2182_priv *)self;

    APS_FUN();
    APS_LOG("epl2182_als_operate command = %x\n",command);

    switch (command)
    {
        case SENSOR_DELAY:
            if((buff_in == NULL) || (size_in < sizeof(int)))
            {
                APS_ERR("Set delay parameter error!\n");
                err = -EINVAL;
            }
            break;


        case SENSOR_ENABLE:
            if((buff_in == NULL) || (size_in < sizeof(int)))
            {
                APS_ERR("Enable sensor parameter error!\n");
                err = -EINVAL;
            }
            else
            {
                value = *(int *)buff_in;
                if(value)
                {
#if ALS_FACTORY_MODE
                    als_factory_flag = false;
#endif
                    set_bit(CMC_BIT_ALS, &obj->enable);
                    epl2182_restart_polling();
                }
                else
                {
                    clear_bit(CMC_BIT_ALS, &obj->enable);
                }

            }
            break;


        case SENSOR_GET_DATA:
            APS_LOG("get als data !!!!!!\n");
            if((buff_out == NULL) || (size_out< sizeof(hwm_sensor_data)))
            {
                APS_ERR("get sensor data parameter error!\n");
                err = -EINVAL;
            }
            else
            {
                sensor_data = (hwm_sensor_data *)buff_out;
                sensor_data->values[0] = epl2182_get_als_value(obj, gRawData.als_ch1_raw);
                sensor_data->value_divide = 1;
                sensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;
                APS_LOG("get als data->values[0] = %d\n", sensor_data->values[0]);
            }
            break;

        default:
            APS_ERR("light sensor operate function no this parameter %d!\n", command);
            err = -1;
            break;



    }

    return err;

}
#endif

/*----------------------------------------------------------------------------*/
/*
static int epl2182_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info)
{
    strcpy(info->type, EPL2182_DEV_NAME);
    return 0;
}

*/
/*----------------------------------------------------------------------------*/
static int epl2182_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct epl2182_priv *obj;
#if MTK_LTE
    struct als_control_path als_ctl={0};
	struct als_data_path als_data={0};
	struct ps_control_path ps_ctl={0};
	struct ps_data_path ps_data={0};
#else
struct hwmsen_object obj_ps, obj_als;
#endif
    int err = 0;
    APS_FUN();

    epl2182_dumpReg(client);

    if((err = i2c_smbus_read_byte_data(client, 0x98)) != 0x68){ //check chip
        APS_ERR("elan ALS/PS sensor is failed. \n");
        err=-1;
        goto exit;
    }

    if(!(obj = kzalloc(sizeof(*obj), GFP_KERNEL)))
    {
        err = -ENOMEM;
        goto exit;
    }

    memset(obj, 0, sizeof(*obj));

    epl2182_obj = obj;
    obj->hw =hw;

    epl2182_get_addr(obj->hw, &obj->addr);

    obj->als_level_num = sizeof(obj->hw->als_level)/sizeof(obj->hw->als_level[0]);
    obj->als_value_num = sizeof(obj->hw->als_value)/sizeof(obj->hw->als_value[0]);
    BUG_ON(sizeof(obj->als_level) != sizeof(obj->hw->als_level));
    memcpy(obj->als_level, obj->hw->als_level, sizeof(obj->als_level));
    BUG_ON(sizeof(obj->als_value) != sizeof(obj->hw->als_value));
    memcpy(obj->als_value, obj->hw->als_value, sizeof(obj->als_value));

    INIT_DELAYED_WORK(&obj->eint_work, epl2182_eint_work);
    INIT_DELAYED_WORK(&obj->polling_work, epl2182_polling_work);
    wake_lock_init(&ps_lock, WAKE_LOCK_SUSPEND, "ps wakelock");

    obj->client = client;

    obj->input_dev = input_allocate_device();
    set_bit(EV_KEY, obj->input_dev->evbit);
    set_bit(EV_REL, obj->input_dev->evbit);
    set_bit(EV_ABS, obj->input_dev->evbit);
    obj->input_dev->evbit[0] |= BIT_MASK(EV_REP);
    obj->input_dev->keycodemax = 500;
    obj->input_dev->name ="elan_gesture";
    obj->input_dev->keybit[BIT_WORD(KEYCODE_LEFT)] |= BIT_MASK(KEYCODE_LEFT);
    if (input_register_device(obj->input_dev))
        APS_ERR("register input error\n");

    mutex_init(&sensor_mutex);

    i2c_set_clientdata(client, obj);

    atomic_set(&obj->trace, 0x00);
    atomic_set(&obj->als_suspend, 0);
    atomic_set(&obj->ps_suspend, 0);
    //atomic_set(&obj->hs_suspend, 0);
#if EINT_API
    obj->irq_node = of_find_compatible_node(NULL, NULL, "mediatek, ALS-eint");
#endif
    obj->lux_per_count = LUX_PER_COUNT;
    obj->enable = 0;
    obj->pending_intr = 0;
    //obj->polling_mode_hs = POLLING_MODE_HS;


    epl2182_i2c_client = client;

    elan_epl2182_I2C_Write(client,REG_0,W_SINGLE_BYTE,0x02, EPL_S_SENSING_MODE);
    elan_epl2182_I2C_Write(client,REG_9,W_SINGLE_BYTE,0x02,EPL_INT_DISABLE);

    elan_epl2182_I2C_Write(client,REG_19,R_TWO_BYTE,0x01,0x00);
    elan_epl2182_I2C_Read(client);
    gRawData.renvo = (gRawData.raw_bytes[1]<<8)|gRawData.raw_bytes[0];

    gRawData.ps_suspend_flag = false;

    if((err = epl2182_init_client(client)))
    {
        goto exit_init_failed;
    }


    if((err = misc_register(&epl2182_device)))
    {
        APS_ERR("epl2182_device register failed\n");
        goto exit_misc_device_register_failed;
    }
#if MTK_LTE
    if((err = epl2182_create_attr(&epl2182_init_info.platform_diver_addr->driver)))
    {
        APS_ERR("create attribute err = %d\n", err);
        goto exit_create_attr_failed;
    }
#else
    if(err = epl2182_create_attr(&epl2182_alsps_driver.driver))
    {
        APS_ERR("create attribute err = %d\n", err);
        goto exit_create_attr_failed;
    }
#endif

#if MTK_LTE
    als_ctl.open_report_data= als_open_report_data;
	als_ctl.enable_nodata = als_enable_nodata;
	als_ctl.set_delay  = als_set_delay;
	als_ctl.is_report_input_direct = false;
#ifdef CUSTOM_KERNEL_SENSORHUB
	als_ctl.is_support_batch = true;
#else
    als_ctl.is_support_batch = false;
#endif

	err = als_register_control_path(&als_ctl);
	if(err)
	{
		APS_ERR("register fail = %d\n", err);
		goto exit_create_attr_failed;
	}

	als_data.get_data = als_get_data;
	als_data.vender_div = 100;
	err = als_register_data_path(&als_data);
	if(err)
	{
		APS_ERR("tregister fail = %d\n", err);
		goto exit_create_attr_failed;
	}


	ps_ctl.open_report_data= ps_open_report_data;
	ps_ctl.enable_nodata = ps_enable_nodata;
	ps_ctl.set_delay  = ps_set_delay;
	ps_ctl.is_report_input_direct = obj->hw->polling_mode_ps==0? true:false; //false;
#ifdef CUSTOM_KERNEL_SENSORHUB
	ps_ctl.is_support_batch = true;
#else
    ps_ctl.is_support_batch = false;
#endif

	err = ps_register_control_path(&ps_ctl);
	if(err)
	{
		APS_ERR("register fail = %d\n", err);
		goto exit_create_attr_failed;
	}

	ps_data.get_data = ps_get_data;
	ps_data.vender_div = 100;
	err = ps_register_data_path(&ps_data);
	if(err)
	{
		APS_ERR("tregister fail = %d\n", err);
		goto exit_create_attr_failed;
	}
#else

    obj_ps.self = epl2182_obj;

    if( obj->hw->polling_mode_ps)
    {
        obj_ps.polling = 1;
        APS_LOG("ps_interrupt == false\n");
    }
    else
    {
        obj_ps.polling = 0;//interrupt mode
        APS_LOG("ps_interrupt == true\n");
    }


    obj_ps.sensor_operate = epl2182_ps_operate;



    if(err = hwmsen_attach(ID_PROXIMITY, &obj_ps))
    {
        APS_ERR("attach fail = %d\n", err);
        goto exit_create_attr_failed;
    }


    obj_als.self = epl2182_obj;
    obj_als.polling = 1;
    obj_als.sensor_operate = epl2182_als_operate;
    APS_LOG("als polling mode\n");


    if(err = hwmsen_attach(ID_LIGHT, &obj_als))
    {
        APS_ERR("attach fail = %d\n", err);
        goto exit_create_attr_failed;
    }
#endif


#if ELAN_WRITE_CALI
    gRawData.ps_als_factory.cal_file_exist = 1;
    gRawData.ps_als_factory.cal_finished = 0;
#endif

#if defined(CONFIG_HAS_EARLYSUSPEND)
    obj->early_drv.level    = EARLY_SUSPEND_LEVEL_DISABLE_FB - 1,
    obj->early_drv.suspend  = epl2182_early_suspend,
    obj->early_drv.resume   = epl2182_late_resume,
    register_early_suspend(&obj->early_drv);
#endif

    if(obj->hw->polling_mode_ps == 0 || obj->hw->polling_mode_als == 0 )
        epl2182_setup_eint(client);

#if MTK_LTE
    alsps_init_flag = 0;
#endif
    APS_LOG("%s: OK\n", __func__);
    return 0;

exit_create_attr_failed:
    misc_deregister(&epl2182_device);
exit_misc_device_register_failed:
exit_init_failed:
    //i2c_detach_client(client);
//	exit_kfree:
    kfree(obj);
exit:
    epl2182_i2c_client = NULL;
#if MTK_LTE
    alsps_init_flag = -1;
#endif
#ifdef MT6516
    MT6516_EINTIRQMask(CUST_EINT_ALS_NUM);  /*mask interrupt if fail*/
#endif

    APS_ERR("%s: err = %d\n", __func__, err);
    return err;



}



/*----------------------------------------------------------------------------*/
static int epl2182_i2c_remove(struct i2c_client *client)
{
    int err;
#if MTK_LTE
    if((err = epl2182_delete_attr(&epl2182_init_info.platform_diver_addr->driver)))
    {
        APS_ERR("epl2182_delete_attr fail: %d\n", err);
    }
#else
    if(err = epl2182_delete_attr(&epl2182_i2c_driver.driver))
    {
        APS_ERR("epl2182_delete_attr fail: %d\n", err);
    }
#endif
    if((err = misc_deregister(&epl2182_device)))
    {
        APS_ERR("misc_deregister fail: %d\n", err);
    }

    epl2182_i2c_client = NULL;
    i2c_unregister_device(client);
    kfree(i2c_get_clientdata(client));

    return 0;
}



/*----------------------------------------------------------------------------*/


#if !MTK_LTE
static int epl2182_probe(struct platform_device *pdev)
{
    //struct alsps_hw *hw = get_cust_alsps_hw();

    epl2182_power(hw, 1);

    //epl2182_force[0] = hw->i2c_num;

    if(i2c_add_driver(&epl2182_i2c_driver))
    {
        APS_ERR("add driver error\n");
        return -1;
    }
    return 0;
}



/*----------------------------------------------------------------------------*/
static int epl2182_remove(struct platform_device *pdev)
{
    //struct alsps_hw *hw = get_cust_alsps_hw();
    APS_FUN();
    epl2182_power(hw, 0);

    APS_ERR("EPL2182 remove \n");
    i2c_del_driver(&epl2182_i2c_driver);
    return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id alsps_of_match[] = {
	{ .compatible = "mediatek,als_ps", },
	{},
};
#endif

/*----------------------------------------------------------------------------*/
static struct platform_driver epl2182_alsps_driver =
{
    .probe      = epl2182_probe,
    .remove     = epl2182_remove,
    .driver     = {
        .name  = "als_ps",
        //.owner = THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = alsps_of_match,
#endif
    }
};
#endif

#if MTK_LTE
/*----------------------------------------------------------------------------*/
static int alsps_local_init(void)
{
    //struct alsps_hw *hw = get_cust_alsps_hw();
	//printk("fwq loccal init+++\n");

	epl2182_power(hw, 1);

	if(i2c_add_driver(&epl2182_i2c_driver))
	{
		APS_ERR("add driver error\n");
		return -1;
	}

	if(-1 == alsps_init_flag)
	{
	   return -1;
	}
	//printk("fwq loccal init---\n");
	return 0;
}
/*----------------------------------------------------------------------------*/
static int alsps_remove(void)
{
    //struct alsps_hw *hw = get_cust_alsps_hw();
    APS_FUN();
    epl2182_power(hw, 0);

    APS_ERR("epl2182 remove \n");

    i2c_del_driver(&epl2182_i2c_driver);
    return 0;
}
#endif

/*----------------------------------------------------------------------------*/
static int __init epl2182_init(void)
{
	const char *name = "mediatek,cust_epl2182";
    APS_FUN();

    hw = get_alsps_dts_func(name, hw);

    if (!hw)
    {
    	APS_ERR("get dts info fail\n");
    }

#if MTK_LTE 
	alsps_driver_add(&epl2182_init_info);
#else

    if(platform_driver_register(&epl2182_alsps_driver))
    {
        APS_ERR("failed to register driver");
        return -ENODEV;
    }
#endif 
    return 0;
}
/*----------------------------------------------------------------------------*/
static void __exit epl2182_exit(void)
{
    APS_FUN();
#if !MTK_LTE
    platform_driver_unregister(&epl2182_alsps_driver);
#endif
}
/*----------------------------------------------------------------------------*/
module_init(epl2182_init);
module_exit(epl2182_exit);
/*----------------------------------------------------------------------------*/
MODULE_AUTHOR("renato.pan@eminent-tek.com");
MODULE_DESCRIPTION("EPL2182 ALPsr driver");
MODULE_LICENSE("GPL");




