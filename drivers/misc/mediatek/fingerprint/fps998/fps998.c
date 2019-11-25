#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/ioport.h>
#include <linux/errno.h>
#include <linux/spi/spi.h>
#include <linux/workqueue.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/irqreturn.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/wakelock.h>
#include <mach/irqs.h>
#include <linux/kthread.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <mt-plat/mt_gpio.h>
#include <mach/emi_mpu.h>
#include <linux/spi/spidev.h>
#include <linux/semaphore.h>
#include <linux/poll.h>
#include <linux/fcntl.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/input.h>
#include <linux/signal.h>
#include <linux/spi/spi.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/gpio.h>
#include <linux/rtpm_prio.h>
#include <linux/platform_data/spi-mt65xx.h>
#include <linux/regulator/consumer.h>
#include <linux/spi/spidev.h>
#include "../../../spi/mediatek/mt6735/mt_spi.h"
//#include "../../../spi/mediatek/mt6755/mt_spi.h" 

u8 fps1098_debug = 0x01;
#define FPS1098_FOR_HCT
#define FPS1098_DBG(fmt, args...) \
	do{ \
		if(fps1098_debug & 0x01) \
			printk( "[DBG][fps1098]:%5d: <%s>" fmt, __LINE__,__func__,##args ); \
	}while(0)
#define FPS1098_FUNCTION(fmt, args...) \
	do{ \
		if(fps1098_debug & 0x02) \
			printk( "[DBG][fps1098]:%5d: <%s>" fmt, __LINE__,__func__,##args ); \
	}while(0)
#define FPS1098_ERR(fmt, args...) \
    do{ \
		printk( "[DBG][fps1098]:%5d: <%s>" fmt, __LINE__,__func__,##args ); \
    }while(0)


#define SPI_DRV_NAME                    "fpsdev0"
#define FPS1098_HEIGHT                  96
#define FPS1098_WIDTH                   112
#define FPS1098_IMAGE_SIZE              (FPS1098_HEIGHT*FPS1098_WIDTH)
#define FPS1098_RD_PIX_SIZE             (15*1024)
#define FPS1098_SPI_CLOCK_SPEED         8*1000*1000
#define SPI_CALIBRATION

#define FPS1098_IOCTL_MAGIC_NO          0xFB
#define FPS1098_INIT                    _IOW(FPS1098_IOCTL_MAGIC_NO, 0, uint8_t)
#define FPS1098_GETIMAGE                _IOW(FPS1098_IOCTL_MAGIC_NO, 1, uint8_t)
#define FPS1098_INITERRUPT_MODE	        _IOW(FPS1098_IOCTL_MAGIC_NO, 2, uint8_t)
#define FPS1098_INITERRUPT_KEYMODE      _IOW(FPS1098_IOCTL_MAGIC_NO, 3, uint8_t)
#define FPS1098_INITERRUPT_FINGERUPMODE _IOW(FPS1098_IOCTL_MAGIC_NO, 4, uint8_t)
#define FPS1098_RELEASE_WAKELOCK        _IO(FPS1098_IOCTL_MAGIC_NO, 5)
#define FPS1098_CHECK_INETRRUPT         _IO(FPS1098_IOCTL_MAGIC_NO, 6)
#define FPS1098_SET_SPI_SPEED           _IOW(FPS1098_IOCTL_MAGIC_NO, 7, uint8_t)

#define KEY_INTERRUPT                   KEY_F11

enum work_mode {
	FPS1098_INTERRUPT_MODE,
	FPS1098_KEY_MODE,
	FPS1098_FINGER_UP_MODE
};

enum spi_speed {
	FPS1098_SPI_4M1 = 1,
	FPS1098_SPI_4M4,
	FPS1098_SPI_4M7,
	FPS1098_SPI_5M1,
	FPS1098_SPI_5M5,
	FPS1098_SPI_6M1,
	FPS1098_SPI_6M7,
	FPS1098_SPI_7M4,
	FPS1098_SPI_8M
};


static struct fps1098_data {
	struct spi_device *spi;
	struct class *class;
	struct device *device;
	struct cdev wg_cdev;
	dev_t devno;
	spinlock_t spi_lock;
	struct mutex buf_lock;
	struct list_head device_entry;
	unsigned users;

	u8 *imagetxcmd;
	u8 *imagerxpix;
	u8 *imagebuf;

	unsigned int irq;
	int irq_enabled;
#ifndef FPS1098_FOR_HCT
	u32 power_always_on;
	struct pinctrl *fps_pinctrl;
	struct pinctrl_state *fps_reset_high;
	struct pinctrl_state *fps_reset_low;
	struct pinctrl_state *fps_power_on;
	struct pinctrl_state *fps_power_off;
	struct pinctrl_state *fps1098_spi_miso;
	struct pinctrl_state *fps1098_spi_mosi;
	struct pinctrl_state *fps1098_spi_sck;
	struct pinctrl_state *fps1098_spi_cs;
#endif
	int thread_wakeup;
	volatile int process_interrupt;
	int key_report;
	enum work_mode device_mode;
	uint8_t int_count;
	struct timer_list int_timer;
	struct input_dev *fps1098_inputdev;
	struct wake_lock fps1098_lock;
	struct task_struct *fps1098_thread;
	struct task_struct *fps1098_spi_calibration_thread;
	struct fasync_struct *async_queue;
	uint8_t reg_eint_flag;
	u8 last_transfer;
}*g_fps1098;

static DEFINE_MUTEX(device_list_lock);
static LIST_HEAD(device_list);
static DECLARE_WAIT_QUEUE_HEAD(waiter);
extern int hct_finger_set_power(int cmd);
extern int hct_finger_set_reset(int cmd);
extern int hct_finger_set_spi_mode(int cmd);
extern int hct_finger_set_eint(int cmd);
extern void hct_waite_for_finger_dts_paser(void);

static struct mt_chip_conf spi_conf = {
	.setuptime = 7,
	.holdtime = 7,
	.high_time = 13,
	.low_time = 13,
	.cs_idletime = 6,
	.cpol = 0,
	.cpha = 0,
	.rx_mlsb = 1,
	.tx_mlsb = 1,
	.tx_endian = 0,
	.rx_endian = 0,
	.com_mod = DMA_TRANSFER,
	.pause = 1,
	.finish_intr = 1,
	.deassert = 0,
	.ulthigh = 0,
	.tckdly = 0,
};

static void fps1098_disable_irq(struct fps1098_data *fps1098)
{
	if(fps1098->irq_enabled == 1)
	{
		disable_irq(fps1098->irq);
		fps1098->irq_enabled = 0;
	}
}

static void fps1098_enable_irq(struct fps1098_data *fps1098)
{
	if(fps1098->irq_enabled == 0)
	{
		enable_irq(fps1098->irq);
		fps1098->irq_enabled =1;
	}
}

static int fps1098_parse_dts(struct fps1098_data *fps1098)
{
	int ret = -1;

#ifdef FPS1098_FOR_HCT
	fps1098->spi->dev.of_node = of_find_compatible_node(NULL,NULL,"mediatek,hct_finger");
	if(!(fps1098->spi->dev.of_node)){
		FPS1098_ERR("of node not exist!\n");
		goto parse_err;
	}

	fps1098->irq = irq_of_parse_and_map(fps1098->spi->dev.of_node, 0);
	if(fps1098->irq < 0)
	{
		FPS1098_ERR("parse irq failed! irq[%d]\n",fps1098->irq);
		goto parse_err;
	}
	FPS1098_DBG("irq[%d]\n", fps1098->irq);

	return 0;
#else

	if(fps1098->spi == NULL)
	{
		FPS1098_ERR("spi is NULL !\n");
		goto parse_err;
	}

    fps1098->spi->dev.of_node = of_find_compatible_node(NULL,NULL,"cdfinger,fps1098");
	if(!(fps1098->spi->dev.of_node)){
		FPS1098_ERR("of node not exist!\n");
		goto parse_err;
	}

	fps1098->irq = irq_of_parse_and_map(fps1098->spi->dev.of_node, 0);
	if(fps1098->irq < 0)
	{
		FPS1098_ERR("parse irq failed! irq[%d]\n",fps1098->irq);
		goto parse_err;
	}

	if(of_property_read_u32(fps1098->spi->dev.of_node,"power_always_on",&fps1098->power_always_on))
	{
		FPS1098_ERR("get power failed!\n");
		goto parse_err;
	}
	FPS1098_DBG("irq[%d], power always on[%d]\n", fps1098->irq, fps1098->power_always_on);

	fps1098->fps_pinctrl = devm_pinctrl_get(&fps1098->spi->dev);
	if(fps1098->fps_pinctrl == NULL)
	{
		FPS1098_ERR(" fps1098->fps_pinctrl = NULL \n");
		goto parse_err;
	}
	if (IS_ERR(fps1098->fps_pinctrl)) {
		ret = PTR_ERR(fps1098->fps_pinctrl);
		FPS1098_ERR("Cannot find fingerprint fps1098->fps_pinctrl! ret=%d\n", ret);
		goto parse_err;
	}

	fps1098->fps_reset_low = pinctrl_lookup_state(fps1098->fps_pinctrl,"fingerprint_reset_low");
	if (IS_ERR(fps1098->fps_reset_low))
	{
		ret = PTR_ERR(fps1098->fps_reset_low);
		FPS1098_ERR("fps1098->fps_reset_low ret = %d\n",ret);
		goto parse_err;
	}	
	fps1098->fps_reset_high = pinctrl_lookup_state(fps1098->fps_pinctrl,"fingerprint_reset_high");
	if (IS_ERR(fps1098->fps_reset_high))
	{
		ret = PTR_ERR(fps1098->fps_reset_high);
		FPS1098_ERR("fps1098->fps_reset_high ret = %d\n",ret);
		goto parse_err;
	}

	fps1098->fps1098_spi_miso = pinctrl_lookup_state(fps1098->fps_pinctrl,"fingerprint_spi_miso");
	if (IS_ERR(fps1098->fps1098_spi_miso))
	{
		ret = PTR_ERR(fps1098->fps1098_spi_miso);
		FPS1098_ERR("fps1098->fps1098_spi_miso ret = %d\n",ret);
		goto parse_err;
	}
	fps1098->fps1098_spi_mosi = pinctrl_lookup_state(fps1098->fps_pinctrl,"fingerprint_spi_mosi");
	if (IS_ERR(fps1098->fps1098_spi_mosi))
	{
		ret = PTR_ERR(fps1098->fps1098_spi_mosi);
		FPS1098_ERR("fps1098->fps1098_spi_mosi ret = %d\n",ret);
		goto parse_err;
	}
	fps1098->fps1098_spi_sck = pinctrl_lookup_state(fps1098->fps_pinctrl,"fingerprint_spi_sck");
	if (IS_ERR(fps1098->fps1098_spi_sck))
	{
		ret = PTR_ERR(fps1098->fps1098_spi_sck);
		FPS1098_ERR("fps1098->fps1098_spi_sck ret = %d\n",ret);
		goto parse_err;
	}
	fps1098->fps1098_spi_cs = pinctrl_lookup_state(fps1098->fps_pinctrl,"fingerprint_spi_cs");
	if (IS_ERR(fps1098->fps1098_spi_cs))
	{
		ret = PTR_ERR(fps1098->fps1098_spi_cs);
		FPS1098_ERR("fps1098->fps1098_spi_cs ret = %d\n",ret);
		goto parse_err;
	}

	if(fps1098->power_always_on == 0)
	{
		fps1098->fps_power_on = pinctrl_lookup_state(fps1098->fps_pinctrl,"fingerprint_power_high");
		if (IS_ERR(fps1098->fps_power_on))
		{
			ret = PTR_ERR(fps1098->fps_power_on);
			FPS1098_ERR("fps1098->fps_power_on ret = %d\n",ret);
			goto parse_err;
		}

		fps1098->fps_power_off = pinctrl_lookup_state(fps1098->fps_pinctrl,"fingerprint_power_low");
		if (IS_ERR(fps1098->fps_power_off))
		{
			ret = PTR_ERR(fps1098->fps_power_off);
			FPS1098_ERR("fps1098->fps_power_off ret = %d\n",ret);
			goto parse_err;
		}
	}

	return 0;
#endif
parse_err:
	FPS1098_ERR("parse dts failed!\n");

	return ret;
}

static int spi_send_cmd(struct fps1098_data *fps1098,  u8 * tx, u8 * rx, u16 spilen)
{
	int ret = 0;
	struct spi_message m;
	struct mt_chip_conf *spiconf = &spi_conf;
	struct spi_transfer t = {
		.cs_change = 0,
		.delay_usecs = 5,
		.speed_hz = FPS1098_SPI_CLOCK_SPEED,
		.tx_buf = tx,
		.rx_buf = rx,
		.len = spilen,
	};

	if(spiconf->com_mod != DMA_TRANSFER)
	{
		spiconf->com_mod = DMA_TRANSFER;
		spi_setup(fps1098->spi);
	}

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	ret = spi_sync(fps1098->spi, &m);

	return ret;
}

static int spi_send_cmd_fifo(struct fps1098_data *fps1098,  u8 * tx, u8 * rx, u16 spilen)
{
	int ret = 0;
	struct spi_message m;
	struct mt_chip_conf *spiconf = &spi_conf;
	struct spi_transfer t = {
		.cs_change = 0,
		.delay_usecs = 5,
		.speed_hz = FPS1098_SPI_CLOCK_SPEED,
		.tx_buf = tx,
		.rx_buf = rx,
		.len =   spilen,
	};

	if(tx[0] == 0x14 && tx[0] == fps1098->last_transfer)
	{
		FPS1098_DBG("Waring: transfer is same as last transfer.now[0x%x], last[0x%x]\n", tx[0], fps1098->last_transfer);
		return 0;
	}
	fps1098->last_transfer = tx[0];

	if(spiconf->com_mod != FIFO_TRANSFER)
	{
		spiconf->com_mod = FIFO_TRANSFER;
		spi_setup(fps1098->spi);
	}

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	ret = spi_sync(fps1098->spi, &m);
	udelay(100);

	return ret;
}

static void finger_power_on(struct fps1098_data *fps1098)
{
#ifdef FPS1098_FOR_HCT
	hct_finger_set_spi_mode(1);
	hct_finger_set_power(1);
	//hct_finger_set_18v_power(1);
#else
	//init spi pin
	pinctrl_select_state(fps1098->fps_pinctrl, fps1098->fps1098_spi_miso);
	pinctrl_select_state(fps1098->fps_pinctrl, fps1098->fps1098_spi_mosi);
	pinctrl_select_state(fps1098->fps_pinctrl, fps1098->fps1098_spi_sck);
	pinctrl_select_state(fps1098->fps_pinctrl, fps1098->fps1098_spi_cs);

	if(fps1098->power_always_on == 0)
		pinctrl_select_state(fps1098->fps_pinctrl, fps1098->fps_power_on);
#endif
}

static int fps1098_reset(uint8_t ms)
{
#ifdef FPS1098_FOR_HCT
	hct_finger_set_reset(0);
	mdelay(1);
	hct_finger_set_reset(1);
	udelay(500);
#else
	struct fps1098_data *fps1098 = g_fps1098;

	pinctrl_select_state(fps1098->fps_pinctrl, fps1098->fps_reset_low);
    mdelay(ms);
	pinctrl_select_state(fps1098->fps_pinctrl, fps1098->fps_reset_high);
#endif

	return 0;
}

static void fps1098_release_wakelock(struct fps1098_data *fps1098)
{
	FPS1098_FUNCTION("enter\n");
	wake_unlock(&fps1098->fps1098_lock);
	FPS1098_FUNCTION("exit\n");
}

static int fps1098_set_power(struct fps1098_data *fps1098)
{
	u8 powerdown_cmd = 0x00, read;
	spi_send_cmd_fifo(fps1098,&powerdown_cmd,&read,1);

	return 0;
}

static int fps1098_dev_init(struct fps1098_data *fps1098, u8 arg)
{
	static u8 reg27_tx[5] = {0x27,0x66,0x66,0x31,0x81};
	static u8 reg21_tx[7] = {0x21,0x66,0x66,0xa5,0x00,0x00,0x70};
	u8 reg27_rx[5], reg21_rx[7];
	int ret =0, count = 0;

	FPS1098_FUNCTION("enter\n");

	fps1098_disable_irq(fps1098);
	reg21_tx[5] = arg;
	for(count=0; count<2; count++)
	{
		fps1098_reset(1);
		ret = spi_send_cmd_fifo(fps1098, reg27_tx,reg27_rx, 5);
		if (ret !=0){
			goto out;
		}
		ret = spi_send_cmd_fifo(fps1098, reg21_tx, reg21_rx, 7);
		if (ret !=0){
			goto out;
		}

		FPS1098_DBG("agc = 0x%x reg[3] = 0x%x reg[4] = 0x%x reg[5] = 0x%x reg[6] = 0x%x\n",reg21_tx[5], reg21_rx[3], reg21_rx[4], reg21_rx[5], reg21_rx[6]);

		if (reg21_rx[6] != 0x70)
		{
			continue;
		}
		break;
	} ;

	if (reg21_rx[6] != 0x70)
		goto out;

	fps1098->device_mode = FPS1098_INTERRUPT_MODE;
	fps1098->key_report = 0;
	fps1098->process_interrupt = 0;

	FPS1098_FUNCTION("exit\n");

	return 0;
out:
	FPS1098_ERR("spi transfer failed! ret = %d\n", ret);

	return -1;
}

static int fps1098_config_interrupt(struct fps1098_data *fps1098, u8 arg)
{
	static u8 interrupt = 0x14;
	static u8 reg27_tx[5] = {0x27,0x66,0x66,0x30,0x05};
	static u8 reg22_tx[7] = {0x22,0x66,0x66,0xa8,0x54,0x01,0x30};
	static u8 reg21_tx[7] = {0x21,0x66,0x66,0xa5,0x00,0x00,0x70};
	u8 read, reg27_rx[5], reg21_rx[7], reg22_rx[7];
	int ret =0, count = 0;

	FPS1098_FUNCTION("enter\n");

	reg21_tx[5] = arg;
	for(count=0; count<2; count++)
	{
		fps1098_reset(1);
		ret = spi_send_cmd_fifo(fps1098, reg27_tx,reg27_rx, 5);
		if (ret !=0){
			goto out;
		}
		ret = spi_send_cmd_fifo(fps1098, reg22_tx,reg22_rx, 7);
		if (ret !=0){
			goto out;
		}
		ret = spi_send_cmd_fifo(fps1098, reg21_tx, reg21_rx, 7);
		if (ret !=0){
			goto out;
		}

		FPS1098_DBG("interrupt = 0x%x reg22_rx[3] = 0x%x reg22_rx[4] = 0x%x reg21_rx[5] = 0x%x reg21_rx[6] = 0x%x\n",reg21_tx[5], reg22_rx[3], reg22_rx[4], reg21_rx[5], reg21_rx[6]);

		if (reg21_rx[6] != 0x70)
		{
			continue;
		}
		break;
	} ;

	if (reg21_rx[6] != 0x70)
		goto out;

	if(spi_send_cmd_fifo(fps1098, &interrupt, &read, 1))
		goto out;

	FPS1098_FUNCTION("exit\n");

	return 0;
out:
	FPS1098_ERR("spi transfer failed! ret = %d\n", ret);

	return -1;
}

static int fps1098_check_interrupt(struct fps1098_data *fps1098)
{
	u8 regval, interrupt = 0;
	int timeout = 40;

	FPS1098_FUNCTION("enter\n");
	fps1098->device_mode = FPS1098_INTERRUPT_MODE;
	fps1098->key_report = 0;
	fps1098->process_interrupt = 0;
	fps1098_enable_irq(fps1098);

	for (regval = 0xf0;regval >= 0xa0;regval-=16)
	{
		fps1098->reg_eint_flag = 0;
		if(fps1098_config_interrupt(fps1098,regval))
			return -1;
		msleep(50);
		if (fps1098->reg_eint_flag == 0)
		{
			interrupt = regval;
			break;
		}
	}

	if(interrupt == 0)
	{
		FPS1098_ERR("cannot find interrupt value!\n");
		return 0;
	}

	FPS1098_DBG("interrupt = 0x%x\n", interrupt);

	for (regval=interrupt+15; regval>interrupt; regval--)
	{
		fps1098->reg_eint_flag = 0;
		if(fps1098_config_interrupt(fps1098,regval))
			return -1;
		msleep(50);
		if (fps1098->reg_eint_flag == 0)
		{
			FPS1098_DBG("interrupt value:0x%x\n",regval);
			break;
		}
	}

	FPS1098_DBG("regval = 0x%x\n", regval);

	for(; regval>=0xa0; regval--)
	{
		fps1098->reg_eint_flag = 0;
		if(fps1098_config_interrupt(fps1098,regval))
				return -1;
		timeout = 40;
		while(fps1098->reg_eint_flag == 0 && timeout > 0)
		{
			msleep(50);
			timeout -- ;
		}
		if (fps1098->reg_eint_flag == 0)
		{
			FPS1098_DBG("interrupt value:0x%x\n",regval);
			return regval;
		}
	}
	
	FPS1098_FUNCTION("exit\n");

	return 0;
}

static int fps1098_mod_init(struct fps1098_data *fps1098, uint8_t arg, enum work_mode mode)
{
	FPS1098_FUNCTION("enter mode=%d\n", mode);
	FPS1098_DBG("mode=%d\n", mode);

	fps1098_disable_irq(fps1098);
	if(fps1098_config_interrupt(fps1098,arg))
	{
		FPS1098_ERR("interrupt config failed! mode=%d\n", mode);
		return -1;
	}
	fps1098->process_interrupt = 1;
	fps1098->device_mode = mode;
	fps1098->key_report = 0;
	fps1098_enable_irq(fps1098);
	FPS1098_FUNCTION("exit\n");

	return 0;
}

static int fps1098_set_spi_speed(struct fps1098_data *fps1098, uint8_t arg)
{
	struct mt_chip_conf *spi_par = &spi_conf;
	enum spi_speed speed = arg;

	switch(speed){
		case FPS1098_SPI_4M1:
			spi_par->high_time = 16;
			spi_par->low_time = 16;
			break;
		case FPS1098_SPI_4M4:
			spi_par->high_time = 15;
			spi_par->low_time = 15;
			break;
		case FPS1098_SPI_4M7:
			spi_par->high_time = 14;
			spi_par->low_time = 14;
			break;
		case FPS1098_SPI_5M1:
			spi_par->high_time = 13;
			spi_par->low_time = 13;
			break;
		case FPS1098_SPI_5M5:
			spi_par->high_time = 12;
			spi_par->low_time = 12;
			break;
		case FPS1098_SPI_6M1:
			spi_par->high_time = 11;
			spi_par->low_time = 11;
			break;
		case FPS1098_SPI_6M7:
			spi_par->high_time = 10;
			spi_par->low_time = 10;
			break;
		case FPS1098_SPI_7M4:
			spi_par->high_time = 9;
			spi_par->low_time = 9;
			break;
		case FPS1098_SPI_8M:
			spi_par->high_time = 8;
			spi_par->low_time = 8;
			break;
		default:
			return -ENOTTY;
	}

	FPS1098_DBG("spi high_time[%d],low_time[%d]\n",spi_par->high_time,spi_par->low_time);

	return spi_setup(fps1098->spi);

}

static int fps1098_read_image(struct fps1098_data *fps1098, uint32_t timeout)
{
	int pix_count = 0, ret = 0;
	u8  linenum = 0;

	FPS1098_FUNCTION("enter\n");
	memset(fps1098->imagerxpix, 0x00, FPS1098_RD_PIX_SIZE);
	fps1098->imagetxcmd[0] = 0x90;
	memset(&fps1098->imagetxcmd[1], 0x66, (FPS1098_RD_PIX_SIZE - 1));
	memset(fps1098->imagebuf, 0x00, FPS1098_IMAGE_SIZE);   
	ret = spi_send_cmd(fps1098, fps1098->imagetxcmd, fps1098->imagerxpix, FPS1098_RD_PIX_SIZE);
	if (ret != 0){
		FPS1098_ERR("read image fail, spi transfer fail\n");
		return -1;
	}
	for (pix_count=0;pix_count<(FPS1098_RD_PIX_SIZE-FPS1098_WIDTH-2);pix_count++){
		if(fps1098->imagerxpix[pix_count] == 0xaa){
			++pix_count;
			linenum = fps1098->imagerxpix[pix_count];
			memcpy((fps1098->imagebuf+linenum*FPS1098_WIDTH),(fps1098->imagerxpix+(++pix_count)),FPS1098_WIDTH);
			pix_count+=FPS1098_WIDTH;
			if (linenum == FPS1098_HEIGHT-1){
				if(pix_count < FPS1098_IMAGE_SIZE)
					goto out;
				FPS1098_DBG("line:%d, pix count index:%d\n",linenum, pix_count);
				return 0;//read imagepix ok
			}
		}
	}
out:
	FPS1098_ERR("read image falied! line:%d, pix count index:%d\n",linenum, pix_count);
	return -2;
}

#ifdef SPI_CALIBRATION
static int fps1098_spi_cal_read_image(struct fps1098_data *fps1098, uint32_t timeout)
{
	int pix_count = 0, ret = 0;
	u8  linenum = 0;

	FPS1098_FUNCTION("enter\n");
	memset(fps1098->imagerxpix, 0x00, FPS1098_RD_PIX_SIZE);
	fps1098->imagetxcmd[0] = 0x90;
	memset(&fps1098->imagetxcmd[1], 0x66, (FPS1098_RD_PIX_SIZE - 1));
	memset(fps1098->imagebuf, 0x00, FPS1098_IMAGE_SIZE);   
	ret = spi_send_cmd(fps1098, fps1098->imagetxcmd, fps1098->imagerxpix, FPS1098_RD_PIX_SIZE);
	if (ret != 0){
		FPS1098_ERR("read image fail, spi transfer fail\n");
		return -1;
	}
	for (pix_count=0;pix_count<(FPS1098_RD_PIX_SIZE-FPS1098_WIDTH-2);pix_count++){
		if(fps1098->imagerxpix[pix_count] == 0xaa){
			++pix_count;
			linenum = fps1098->imagerxpix[pix_count];
			memcpy((fps1098->imagebuf+linenum*FPS1098_WIDTH),(fps1098->imagerxpix+(++pix_count)),FPS1098_WIDTH);
			pix_count+=FPS1098_WIDTH;
			if (linenum == FPS1098_HEIGHT-1){
				if(pix_count < FPS1098_IMAGE_SIZE)
					goto out;
				FPS1098_DBG("line:%d, pix count index:%d\n",linenum, pix_count);
				return pix_count;//read imagepix ok
			}
		}
	}
out:
	FPS1098_ERR("read image falied! line:%d, pix count index:%d\n",linenum, pix_count);
	return -2;
}
#endif
static long fps1098_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct fps1098_data *fps1098 = filp->private_data;
	int error = 0;

	FPS1098_FUNCTION("enter\n");
	if(fps1098 == NULL)
	{
		FPS1098_ERR("%s: fingerprint please open device first!\n", __func__);
		return -EIO;
	}

	mutex_lock(&fps1098->buf_lock);
	switch (cmd) {
		case FPS1098_INIT:
			error = fps1098_dev_init(fps1098, arg);
			break;
		case FPS1098_GETIMAGE:
			error = fps1098_read_image(fps1098, arg);
			break;
		case FPS1098_INITERRUPT_MODE:
			error = fps1098_mod_init(fps1098,arg,FPS1098_INTERRUPT_MODE);
			break;
		case FPS1098_INITERRUPT_FINGERUPMODE:
			error = fps1098_mod_init(fps1098,arg,FPS1098_FINGER_UP_MODE);
			break;
		case FPS1098_RELEASE_WAKELOCK:
			fps1098_release_wakelock(fps1098);
			break;
		case FPS1098_INITERRUPT_KEYMODE:
			error = fps1098_mod_init(fps1098,arg,FPS1098_KEY_MODE);
			break;
		case FPS1098_CHECK_INETRRUPT:
			error = fps1098_check_interrupt(fps1098);
			break;
		case FPS1098_SET_SPI_SPEED:
			error = fps1098_set_spi_speed(fps1098,arg);
			break;
		default:
			error = -ENOTTY;
			break;
	}
	mutex_unlock(&fps1098->buf_lock);
	//FPS1098_DBG("return velue: %d\n", error);
	FPS1098_FUNCTION("exit\n");

	return error;
}

static int fps1098_open(struct inode *inode, struct file *file)
{
	struct fps1098_data *fps1098;
	int status = -ENXIO;

	FPS1098_FUNCTION("enter\n");
	mutex_lock(&device_list_lock);
	list_for_each_entry(fps1098, &device_list, device_entry) {
		if (fps1098->devno == inode->i_rdev) {
			status = 0;
			break;
		}
	}
	if (status == 0) {
		fps1098->users++;
		file->private_data = fps1098;
		nonseekable_open(inode, file);
	} else {
		FPS1098_ERR("spidev: nothing for minor %d\n", iminor(inode));
	}
	mutex_unlock(&device_list_lock);
	FPS1098_FUNCTION("exit\n");

	return status;
}

static ssize_t fps1098_write(struct file *file, const char *buff, size_t count, loff_t * ppos)
{
	return -ENOMEM;
}

static int fps1098_async_fasync(int fd, struct file *filp, int mode)
{
	struct fps1098_data *fps1098 = g_fps1098;

	FPS1098_FUNCTION("enter\n");
	return fasync_helper(fd, filp, mode, &fps1098->async_queue);
}

static ssize_t fps1098_read(struct file *file, char *buff, size_t count, loff_t * ppos)
{
	int ret = 0;
	struct fps1098_data *fps1098 = file->private_data;
	ssize_t status = 0;
	struct spi_device *spi;

	FPS1098_FUNCTION("enter\n");
	if(fps1098 == NULL)
	{
		FPS1098_ERR("%s: fingerprint please open device first!\n", __func__);
		return -EIO;
	}
	mutex_lock(&fps1098->buf_lock);
	spi = spi_dev_get(fps1098->spi);
	ret = copy_to_user(buff, fps1098->imagebuf, count);
	if (ret) {
		status = -EFAULT;
	}
	mutex_unlock(&fps1098->buf_lock);
	FPS1098_FUNCTION("exit\n");

	return status;
}

static int fps1098_release(struct inode *inode, struct file *file)
{
	struct fps1098_data *fps1098 = file->private_data;
	int status = 0;

	FPS1098_FUNCTION("enter\n");
	if(fps1098 == NULL)
	{
		FPS1098_ERR("%s: fingerprint please open device first!\n", __func__);
		return -EIO;
	}
	mutex_lock(&device_list_lock);
	file->private_data = NULL;
	fps1098->users--;
	if (!fps1098->users) {
		int dofree;
		spin_lock_irq(&fps1098->spi_lock);
		dofree = (fps1098->spi == NULL);
		spin_unlock_irq(&fps1098->spi_lock);
		if (dofree)
			kfree(fps1098);
	}
	mutex_unlock(&device_list_lock);
	FPS1098_FUNCTION("exit\n");

	return status;
}

static const struct file_operations fps1098_fops = {
	.owner = THIS_MODULE,
	.open = fps1098_open,
	.write = fps1098_write,
	.read = fps1098_read,
	.release = fps1098_release,
	.fasync = fps1098_async_fasync,
	.unlocked_ioctl = fps1098_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = fps1098_ioctl,
#endif
};

//clear interrupt
static int fps1098_clear_interrupt(struct fps1098_data *fps1098)
{
	u8 start = 0x18,clc_cmd = 0xA3,read;
	int retry = 50;

	FPS1098_FUNCTION("enter\n");

	for(; retry>0; retry--)
	{
		if(spi_send_cmd_fifo(fps1098, &start, &read, 1) < 0)
		{
			FPS1098_ERR("spi transfer failed!\n");
			msleep(10);
			continue;
		}
		if(spi_send_cmd_fifo(fps1098, &clc_cmd, &read, 1) < 0)
		{
			FPS1098_ERR("spi transfer failed!\n");
			msleep(10);
			continue;
		}
		break;
	}
	FPS1098_DBG("retry = %d\n", 50-retry);
	FPS1098_FUNCTION("exit\n");

	return 0;
}

static void fps1098_async_Report(void)
{
	struct fps1098_data *fps1098 = g_fps1098;

	FPS1098_FUNCTION("enter\n");
	kill_fasync(&fps1098->async_queue, SIGIO, POLL_IN);
	FPS1098_FUNCTION("exit\n");
}

static void int_timer_handle(unsigned long arg)
{
	struct fps1098_data *fps1098 = g_fps1098;

	fps1098->int_count = 0;
	FPS1098_DBG("enter\n");
	if ((fps1098->device_mode == FPS1098_KEY_MODE) && (fps1098->key_report == 1)) {
		input_report_key(fps1098->fps1098_inputdev, KEY_INTERRUPT, 0);
		input_sync(fps1098->fps1098_inputdev);
		fps1098->key_report = 0;
	}

	if (fps1098->device_mode == FPS1098_FINGER_UP_MODE){
		fps1098->process_interrupt = 0;
		fps1098_async_Report();
	}
	FPS1098_DBG("exit\n");
}

//Thread processing function
static int fps1098_thread_func(void *arg)
{
//	struct sched_param param = {.sched_priority = REG_RT_PRIO(1) };
	struct fps1098_data *fps1098 = (struct fps1098_data *)arg;
	static u8 int_cmd = 0x14,read;

	FPS1098_DBG("enter\n");
//	sched_setscheduler(current, SCHED_RR, &param);
	
	do {
		wait_event_interruptible(waiter, fps1098->thread_wakeup != 0);
        FPS1098_DBG("fps1098:%s,thread wakeup\n",__func__);
		fps1098->thread_wakeup = 0;
		set_current_state(TASK_RUNNING);
		fps1098->int_count++;
		wake_lock_timeout(&fps1098->fps1098_lock, 3*HZ);
		fps1098_clear_interrupt(fps1098);
		if (fps1098->int_count >= 2) {
			fps1098->int_count = 0;
			if (fps1098->device_mode == FPS1098_INTERRUPT_MODE) {
				fps1098->process_interrupt = 0;
				fps1098_async_Report();
				del_timer_sync(&fps1098->int_timer);
				continue;
			} else if ((fps1098->device_mode == FPS1098_KEY_MODE) && (fps1098->key_report == 0)) {
				input_report_key(fps1098->fps1098_inputdev, KEY_INTERRUPT, 1);
				input_sync(fps1098->fps1098_inputdev);
				fps1098->key_report = 1;
			}
		}
		spi_send_cmd_fifo(fps1098, &int_cmd, &read, 1);
	} while (!kthread_should_stop());
	FPS1098_DBG("exit\n");

	return 0;
}


// Interrupt handler function
static irqreturn_t fps1098_interrupt_handler(unsigned irq, void *arg)
{
	struct fps1098_data *fps1098 = (struct fps1098_data *)arg;

	fps1098->reg_eint_flag = 1;
	if (fps1098->process_interrupt == 0x01) {
		mod_timer(&fps1098->int_timer, jiffies + HZ / 10);
		fps1098->thread_wakeup = 1;
		wake_up_interruptible(&waiter);
	} else {
		fps1098->thread_wakeup = 0;
	}

	return IRQ_HANDLED;
}

static ssize_t fps1098_debug_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "fps1098_debug = %x\n", fps1098_debug);
}


static ssize_t fps1098_debug_store(struct device *dev, 
				struct device_attribute *attr, const char *buf, size_t size)
{
	int data;

	if (buf != NULL) 
		sscanf(buf, "%x", &data);

	fps1098_debug = (u8)data;
	
	return size;	
}

static ssize_t fps1098_spi_freq_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct mt_chip_conf *spi_par = &spi_conf;
	int freq = 0;

	freq = 134300/(spi_par->high_time + spi_par->low_time);

	return sprintf(buf, "spi frequence[%d], hight_time[%d], low_time[%d]\n", freq, spi_par->high_time, spi_par->low_time);
}

static ssize_t fps1098_spi_freq_store(struct device *dev, 
				struct device_attribute *attr, const char *buf, size_t size)
{
	struct mt_chip_conf *spi_par;
	struct fps1098_data *fps1098 = g_fps1098;
	u32 time = 0;

	if (buf != NULL) 
		sscanf(buf, "%d", &time);
	spi_par = &spi_conf;
	if (!spi_par) {
		return -1;
	}
	FPS1098_DBG("freq time: %d\n",time);

	spi_par->high_time = time;
	spi_par->low_time = time;
	spi_setup(fps1098->spi);

	return size;
}
static DEVICE_ATTR(debug, S_IRUGO|S_IWUSR, fps1098_debug_show, fps1098_debug_store);
static DEVICE_ATTR(spi_freq, S_IRUGO|S_IWUSR,fps1098_spi_freq_show, fps1098_spi_freq_store);

static struct attribute *fps1098_attrs [] =
{
	&dev_attr_debug.attr,
	&dev_attr_spi_freq.attr,
	NULL
};

static struct attribute_group fps1098_attribute_group = {
	.name = "cdfinger",
	.attrs = fps1098_attrs,
};

static int fps1098_create_inputdev(struct fps1098_data *fps1098)
{
	FPS1098_DBG("enter\n");
	fps1098->fps1098_inputdev = input_allocate_device();
	if (!fps1098->fps1098_inputdev) {
		FPS1098_ERR("fps1098->fps1098_inputdev create faile!\n");
		return -ENOMEM;
	}
	__set_bit(EV_KEY, fps1098->fps1098_inputdev->evbit);
	__set_bit(KEY_INTERRUPT, fps1098->fps1098_inputdev->keybit);
	__set_bit(KEY_POWER, fps1098->fps1098_inputdev->keybit);
	fps1098->fps1098_inputdev->id.bustype = BUS_HOST;
	fps1098->fps1098_inputdev->name = "fps1098_inputdev";
	if (input_register_device(fps1098->fps1098_inputdev)) {
		FPS1098_ERR("register inputdev failed\n");
		input_free_device(fps1098->fps1098_inputdev);
		return -ENOMEM;
	}

	FPS1098_DBG("exit\n");

	return sysfs_create_group(&fps1098->fps1098_inputdev->dev.kobj, &fps1098_attribute_group);
}

static int fps1098_check_id(struct fps1098_data *fps1098)
{
	u8 reset = 0x0c, start = 0x18, read, id_rx[7]={0};
	u8 id_cmd[7] = {0x21,0x66,0x66,0xb5,0x00,0x43,0x44};

	fps1098_reset(1);
	spi_send_cmd_fifo(fps1098,&reset,&read,1);
	spi_send_cmd_fifo(fps1098,&reset,&read,1);
	spi_send_cmd_fifo(fps1098,&start,&read,1);
	spi_send_cmd_fifo(fps1098,id_cmd,id_rx,7);
	FPS1098_DBG("reg[5] = 0x%x reg[6] = 0x%x\n",id_rx[5], id_rx[6]);
	if((id_rx[5]!=0x70)||(id_rx[6]!=0x70))
		return -1;

	return 0;
}

#ifdef SPI_CALIBRATION
static int fps1098_calibration_spi(void *arg)
{
	struct fps1098_data *fps1098 = (struct fps1098_data *)arg;
	enum spi_speed speed=FPS1098_SPI_8M;
	int ret=-1, i=0;

	mutex_lock(&fps1098->buf_lock);
	for(; speed>=FPS1098_SPI_4M1; speed--)
	{
		FPS1098_DBG("ioctl enum %d\n", speed);
		fps1098_set_spi_speed(fps1098, speed);
		
		ret = fps1098_dev_init(fps1098, 0xe0);
		if (ret < 0)
		{
			FPS1098_ERR("fps1098_dev_init failed, ret = %d\n", ret);
			continue;
		}
		ret = fps1098_spi_cal_read_image(fps1098,1);
		if (ret < 0)
		{
			FPS1098_ERR("fps1098_spi_cal_read_image failed, ret = %d\n", ret);
			continue;
		}
		FPS1098_DBG("read image buffer size: %d\n", ret);
		
		if(ret<13500 && ret>11000)
		{
			for(i=0; i<5; i++)
			{
				ret = fps1098_dev_init(fps1098, 0xe0);
				if (ret < 0)
				{
					FPS1098_ERR("fps1098_dev_init failed, ret = %d\n", ret);
					break;
				}
				ret = fps1098_spi_cal_read_image(fps1098,1);
				if (ret == -2)
				{
					FPS1098_ERR("fps1098_spi_cal_read_image failed, ret = %d\n", ret);
					break;
				}
				if(ret > 13500)
					break;
			}
			if(ret < 0 || ret > 13500)
				continue;
			FPS1098_DBG("find spi speed[%d] buffer[%d]\n",speed, ret);
			break;
		}
		if(speed == FPS1098_SPI_4M1)
			break;
	}

	fps1098_set_power(fps1098);
	mutex_unlock(&fps1098->buf_lock);

	return 0;
}
#endif

static int fps1098_probe(struct spi_device *spi)
{
	struct fps1098_data *fps1098 = NULL;
	int status = -ENODEV;
#ifdef FPS1098_FOR_HCT
	hct_waite_for_finger_dts_paser();
#endif
       printk("liwandong probe \r\n");	
	FPS1098_DBG("enter\n");
	fps1098 = kzalloc(sizeof(struct fps1098_data), GFP_KERNEL);
	if (!fps1098) {
		return -ENOMEM;
	}
	fps1098->imagebuf = (char*)kzalloc(FPS1098_IMAGE_SIZE*sizeof(char),GFP_KERNEL);
	if (!fps1098->imagebuf)
	{
		FPS1098_ERR("%s: imagebuf malloc fail!\n", __func__);
		goto free_fps1098;
	}
	fps1098->imagetxcmd = (char*)kzalloc(FPS1098_RD_PIX_SIZE*sizeof(char),GFP_KERNEL);
	if (!fps1098->imagetxcmd)
	{
		FPS1098_ERR("%s: imagetxcmd malloc fail!\n", __func__);
		goto free_imagebuf;
	}
	fps1098->imagerxpix= (char*)kzalloc(FPS1098_RD_PIX_SIZE*sizeof(char),GFP_KERNEL);
	if (!fps1098->imagerxpix)
	{
		FPS1098_ERR("%s: imagerxpix malloc fail!\n", __func__);
		goto free_imagetxcmd;
	}

	g_fps1098 = fps1098;
	fps1098->spi = spi;

	INIT_LIST_HEAD(&fps1098->device_entry);
	spin_lock_init(&fps1098->spi_lock);

	mutex_init(&fps1098->buf_lock);
	mutex_lock(&device_list_lock);
	wake_lock_init(&fps1098->fps1098_lock, WAKE_LOCK_SUSPEND, "fps1098 wakelock");

	cdev_init(&(fps1098->wg_cdev), &fps1098_fops);
	fps1098->wg_cdev.owner = THIS_MODULE;
	alloc_chrdev_region(&(fps1098->wg_cdev.dev), 10, 1, SPI_DRV_NAME);
	fps1098->devno = fps1098->wg_cdev.dev;
	cdev_add(&(fps1098->wg_cdev), fps1098->devno, 1);
	fps1098->class = class_create(THIS_MODULE, SPI_DRV_NAME);
	fps1098->device = device_create(fps1098->class, NULL, fps1098->devno, NULL, SPI_DRV_NAME);
	list_add(&fps1098->device_entry, &device_list);

	status = IS_ERR(fps1098->device) ? PTR_ERR(fps1098->device) : 0;
	if (status != 0)
	{
		FPS1098_ERR("%s: cdev register failed!\n", __func__);
		goto free_resource;
	}

	spi_set_drvdata(spi, fps1098);
	if(fps1098_parse_dts(fps1098))
	{
		FPS1098_ERR("%s: parse dts failed!\n", __func__);
		goto free_device;
	}

	finger_power_on(fps1098);
	fps1098_reset(5);

	if(fps1098_create_inputdev(fps1098) < 0)
	{
		FPS1098_ERR("%s: inputdev register failed!\n", __func__);
		goto free_device;
	}
	hct_finger_set_eint(1);
	status = request_irq(fps1098->irq, (irq_handler_t)fps1098_interrupt_handler, IRQF_TRIGGER_RISING | IRQF_NO_SUSPEND,"fps1098-irq", fps1098);
	//enable_irq_wake(fps1098->irq);
	if(status){
		FPS1098_ERR("request_irq error\n");
		goto free_inputdev;
	}
	else
		FPS1098_DBG("request_irq sucess fps1098->irq=%d\n ",fps1098->irq);

	fps1098->irq_enabled = 1;
	init_timer(&fps1098->int_timer);
	fps1098->int_timer.function = int_timer_handle;
	add_timer(&fps1098->int_timer);

	spi->bits_per_word = 8;
	spi->mode = SPI_MODE_0;
	spi->max_speed_hz = FPS1098_SPI_CLOCK_SPEED;
	spi->controller_data = (void *)&spi_conf;
	if(spi_setup(spi) != 0)
	{
		FPS1098_ERR("%s: spi setup failed!\n", __func__);
		goto free_inputdev;
	}

	status = fps1098_check_id(fps1098); //cheak id
	if (status != 0)
	{
		FPS1098_ERR("fps1098: check id failed! status=%d\n",status);
		goto free_inputdev;
	}
	//Create and run the thread
	fps1098->fps1098_thread = kthread_run(fps1098_thread_func, fps1098, "fps1098_thread");
	if (IS_ERR(fps1098->fps1098_thread)) {
		FPS1098_ERR("kthread_run is faile\n");
		goto free_inputdev;
	}

#ifdef SPI_CALIBRATION
	fps1098->fps1098_spi_calibration_thread = kthread_run(fps1098_calibration_spi, fps1098, "fps1098_spi_calibration_thread");
#else
	fps1098_set_power(fps1098);
#endif
	mutex_unlock(&device_list_lock);
	FPS1098_DBG("exit\n");

	return 0;

free_inputdev:
	sysfs_remove_group(&fps1098->fps1098_inputdev->dev.kobj, &fps1098_attribute_group);
	input_unregister_device(fps1098->fps1098_inputdev);
	fps1098->fps1098_inputdev = NULL;
	input_free_device(fps1098->fps1098_inputdev);
	del_timer(&fps1098->int_timer);
free_device:
free_resource:
	cdev_del(&fps1098->wg_cdev);
	unregister_chrdev_region(fps1098->wg_cdev.dev, 1);
	device_destroy(fps1098->class, fps1098->devno);
	class_destroy(fps1098->class);
	wake_lock_destroy(&fps1098->fps1098_lock);
	mutex_destroy(&fps1098->buf_lock);
//free_imagerxpix:
	kfree(fps1098->imagerxpix);
free_imagetxcmd:
	kfree(fps1098->imagetxcmd);
free_imagebuf:
	kfree(fps1098->imagebuf);
free_fps1098:
	kfree(fps1098);
	mutex_unlock(&device_list_lock);
	mutex_destroy(&device_list_lock);

	return -1;
}


static int fps1098_suspend (struct device *dev)
{
	return 0;
}

static int fps1098_resume (struct device *dev)
{

	return 0;
}

static int fps1098_remove(struct spi_device *spi)
{
	struct fps1098_data *fps1098 = spi_get_drvdata(spi);

	kthread_stop(fps1098->fps1098_thread);
	sysfs_remove_group(&fps1098->fps1098_inputdev->dev.kobj, &fps1098_attribute_group);
	input_unregister_device(fps1098->fps1098_inputdev);
	fps1098->fps1098_inputdev = NULL;
	input_free_device(fps1098->fps1098_inputdev);
	del_timer(&fps1098->int_timer);
	device_destroy(fps1098->class, fps1098->devno);
	class_destroy(fps1098->class);
	cdev_del(&fps1098->wg_cdev);
	unregister_chrdev_region(fps1098->wg_cdev.dev, 1);
	wake_lock_destroy(&fps1098->fps1098_lock);
	mutex_destroy(&fps1098->buf_lock);
	input_unregister_device(fps1098->fps1098_inputdev);
	input_free_device(fps1098->fps1098_inputdev);
	kfree(fps1098->imagerxpix);
	kfree(fps1098->imagetxcmd);
	kfree(fps1098->imagebuf);
	kfree(fps1098);
	g_fps1098 = NULL;

	return 0;
}

static const struct dev_pm_ops fps1098_pm = {
	.suspend = fps1098_suspend,
	.resume = fps1098_resume
};

static struct spi_board_info spi_board_fps1098[] __initdata = {
	[0] = {
        .modalias = "fps1098",
        .bus_num = 0,
        .chip_select = 0,
        .mode = SPI_MODE_0,
        .max_speed_hz = FPS1098_SPI_CLOCK_SPEED,
        .controller_data = &spi_conf,
	},
};

struct of_device_id fps1098_of_match[] = {
	{ .compatible = "cdfinger,fps1098", },
	{},
};

static const struct spi_device_id fps1098_id[] = {
	{"fps1098", 0},
	{}
};

static struct spi_driver fps1098_driver = {
	.driver = {
		.name = "fps1098",
		.bus = &spi_bus_type,
		.owner = THIS_MODULE,
		.pm = &fps1098_pm,
		//.of_match_table = fps1098_of_match,
	},
	.id_table = fps1098_id,
	.probe = fps1098_probe,
	.remove = fps1098_remove,
};


static int fps1098_spi_init(void)
{
	spi_register_board_info(spi_board_fps1098, ARRAY_SIZE(spi_board_fps1098));
	return spi_register_driver(&fps1098_driver);
}

static void fps1098_spi_exit(void)
{
	spi_unregister_driver(&fps1098_driver);
}

//module_init(fps1098_spi_init);
module_exit(fps1098_spi_exit);
 late_initcall(fps1098_spi_init);
//late_exitcall(fps1098_spi_init);

MODULE_DESCRIPTION("fps1098 spi Driver");
MODULE_AUTHOR("shuaitao@cdfinger.com");
MODULE_LICENSE("GPL");
MODULE_ALIAS("fps1098");
