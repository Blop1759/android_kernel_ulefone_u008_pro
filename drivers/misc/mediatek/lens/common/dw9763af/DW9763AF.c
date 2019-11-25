/*
 * DW9763AF voice coil motor driver     
 *
 *
 */

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/fs.h>

#include "lens_info.h"   
#include "mt_boot_common.h" //miaowei motify for factory slow 

#define AF_DRVNAME "DW9763AF_DRV"
#define AF_I2C_SLAVE_ADDR        0x18

#define AF_DEBUG
#ifdef AF_DEBUG
#define LOG_INF(format, args...) pr_debug(AF_DRVNAME " [%s] " format, __func__, ##args)
#else
#define LOG_INF(format, args...)
#endif


static struct i2c_client *g_pstAF_I2Cclient;
static int *g_pAF_Opened;
static spinlock_t *g_pAF_SpinLock;


static long g_i4MotorStatus = 0;
static long g_i4Dir = 0;
static unsigned long g_u4AF_INF ;
static unsigned long g_u4AF_MACRO = 1023;
static unsigned long g_u4TargetPosition;
static unsigned long g_u4CurrPosition;

static int g_sr = 3; 

static int i2c_read(u8 a_u2Addr, u8 *a_puBuff)
{
	int i4RetValue = 0;
	char puReadCmd[1] = { (char)(a_u2Addr) };
	i4RetValue = i2c_master_send(g_pstAF_I2Cclient, puReadCmd, 1);
	if (i4RetValue != 2) {
		LOG_INF(" I2C write failed!!\n");
		return -1;
	}
	/*  */
	i4RetValue = i2c_master_recv(g_pstAF_I2Cclient, (char *)a_puBuff, 1);
	if (i4RetValue != 1) {
		LOG_INF(" I2C read failed!!\n");
		return -1;
	}

	return 0;
}

u8 read_data(u8 addr)
{
	u8 get_byte=0;
    i2c_read( addr ,&get_byte);
    LOG_INF("[DW9763AF]  get_byte %d \n",  get_byte);
    return get_byte;
}


static int s4AF_ReadReg(unsigned short *a_pu2Result)
{
    int  i4RetValue = 0;
    char pBuff[2];

    i4RetValue = i2c_master_recv(g_pstAF_I2Cclient, pBuff , 2);

    if (i4RetValue < 0)
    {
        LOG_INF("I2C read failed!! \n");
        return -1;
    }

  //  *a_pu2Result = (((u16)pBuff[0]) << 4) + (pBuff[1] >> 4);
 *a_pu2Result = (read_data(0x03) << 8) + (read_data(0x04)&0xff);

    return 0;
}

static int s4AF_WriteReg(u16 a_u2Data)
{
    int  i4RetValue = 0;

    char puSendCmd[2] = {(char)(0x03) , (char)(a_u2Data >> 8)};
    char puSendCmd1[2] = {(char)(0x04) , (char)(a_u2Data & 0xFF)};
    i4RetValue = i2c_master_send(g_pstAF_I2Cclient, puSendCmd, 2);
    if (i4RetValue < 0) 
    {
        printk("s4AF_WriteReg() failed.\n");
        return -1;
    }
    
    i4RetValue = i2c_master_send(g_pstAF_I2Cclient, puSendCmd1, 2);
    
    if (i4RetValue < 0) 
    {
        printk("s4AF_WriteReg() failed.\n");
        return -1;
    }

    return 0;
}



static int AF_setconfig(void)
{
  
    int  Ret = 0;
    char puSendCmd2[2] = {(char)(0x02) , (char)(0x02)};
    char puSendCmd3[2] = {(char)(0x06) , (char)(0x61)};
    char puSendCmd4[2] = {(char)(0x07) , (char)(0x39)};

    LOG_INF("Start \n");

    Ret = i2c_master_send(g_pstAF_I2Cclient, puSendCmd2, 2);
    if (Ret < 0)
    {
        LOG_INF("puSendCmd2 send failed.\n");
    }
  
    Ret = i2c_master_send(g_pstAF_I2Cclient, puSendCmd3, 2);
    if (Ret < 0)
    {
        LOG_INF("puSendCmd3 send failed.\n");
    }
    
    Ret = i2c_master_send(g_pstAF_I2Cclient, puSendCmd4, 2);
    if (Ret < 0)
    {
        LOG_INF("puSendCmd4 send failed.\n");
    }

    LOG_INF("End \n");

    return 0;
}

static inline int getAFInfo(__user stAF_MotorInfo *pstMotorInfo)
{
	stAF_MotorInfo stMotorInfo;

	stMotorInfo.u4MacroPosition = g_u4AF_MACRO;
	stMotorInfo.u4InfPosition = g_u4AF_INF;
	stMotorInfo.u4CurrentPosition = g_u4CurrPosition;
	stMotorInfo.bIsSupportSR = 1;

    if (g_i4MotorStatus == 1)    {stMotorInfo.bIsMotorMoving = 1;}
    else                        {stMotorInfo.bIsMotorMoving = 0;}

    if (*g_pAF_Opened >= 1)    {stMotorInfo.bIsMotorOpen = 1;}
    else                        {stMotorInfo.bIsMotorOpen = 0;}

    if(copy_to_user(pstMotorInfo , &stMotorInfo , sizeof(stAF_MotorInfo)))
    {
        LOG_INF("copy to user failed when getting motor information \n");
    }

    return 0;
}

#ifdef LensdrvCM3
inline static int getAFMETA(__user stAF_MotorMETAInfo * pstMotorMETAInfo)
{
    stAF_MotorMETAInfo stMotorMETAInfo;
    stMotorMETAInfo.Aperture=2.8;       //fn
    stMotorMETAInfo.Facing=1;
    stMotorMETAInfo.FilterDensity=1;   //X
    stMotorMETAInfo.FocalDistance=1.0;    //diopters
    stMotorMETAInfo.FocalLength=34.0;  //mm
    stMotorMETAInfo.FocusRange=1.0;    //diopters
    stMotorMETAInfo.InfoAvalibleApertures=2.8;
    stMotorMETAInfo.InfoAvalibleFilterDensity=1;
    stMotorMETAInfo.InfoAvalibleFocalLength=34.0;
    stMotorMETAInfo.InfoAvalibleHypeDistance=1.0;
    stMotorMETAInfo.InfoAvalibleMinFocusDistance=1.0;
    stMotorMETAInfo.InfoAvalibleOptStabilization=0;
    stMotorMETAInfo.OpticalAxisAng[0]=0.0;
    stMotorMETAInfo.OpticalAxisAng[1]=0.0;
    stMotorMETAInfo.Position[0]=0.0;
    stMotorMETAInfo.Position[1]=0.0;
    stMotorMETAInfo.Position[2]=0.0;
    stMotorMETAInfo.State=0;
    stMotorMETAInfo.u4OIS_Mode=0;

    if(copy_to_user(pstMotorMETAInfo , &stMotorMETAInfo , sizeof(stAF_MotorMETAInfo)))
    {
        LOG_INF("copy to user failed when getting motor information \n");
    }

    return 0;
}
#endif

static inline int moveAF(unsigned long a_u4Position)
{
	int ret = 0;

    if((a_u4Position > g_u4AF_MACRO) || (a_u4Position < g_u4AF_INF))
    {
        LOG_INF("out of range \n");
        return -EINVAL;
    }

    if (*g_pAF_Opened == 1)
    {
        unsigned short InitPos;

         AF_setconfig();
        ret = s4AF_ReadReg(&InitPos);

        if(ret == 0)
        {
            LOG_INF("[hwadd]Init Pos %6d \n", InitPos);
            a_u4Position = 512;
            spin_lock(g_pAF_SpinLock);
            g_u4CurrPosition = (unsigned long)InitPos;
            spin_unlock(g_pAF_SpinLock);
        }
        else
        {
            spin_lock(g_pAF_SpinLock);
            g_u4CurrPosition = 0;
            spin_unlock(g_pAF_SpinLock);
        }

        spin_lock(g_pAF_SpinLock);
        *g_pAF_Opened = 2;
        spin_unlock(g_pAF_SpinLock);
    }

    if (g_u4CurrPosition < a_u4Position)
    {
        spin_lock(g_pAF_SpinLock);
        g_i4Dir = 1;
        spin_unlock(g_pAF_SpinLock);
    }
    else if (g_u4CurrPosition > a_u4Position)
    {
        spin_lock(g_pAF_SpinLock);
        g_i4Dir = -1;
        spin_unlock(g_pAF_SpinLock);
    }
    else                                        {return 0;}

    spin_lock(g_pAF_SpinLock);
    g_u4TargetPosition = a_u4Position;
    spin_unlock(g_pAF_SpinLock);

    //LOG_INF("move [curr] %d [target] %d\n", g_u4CurrPosition, g_u4TargetPosition);

    spin_lock(g_pAF_SpinLock);
    g_sr = 3;
    g_i4MotorStatus = 0;
    spin_unlock(g_pAF_SpinLock);

    if(s4AF_WriteReg((unsigned short)g_u4TargetPosition) == 0)
    {
        spin_lock(g_pAF_SpinLock);
        g_u4CurrPosition = (unsigned long)g_u4TargetPosition;
        spin_unlock(g_pAF_SpinLock);
    }
    else
    {
        LOG_INF("set I2C failed when moving the motor \n");

        spin_lock(g_pAF_SpinLock);
        g_i4MotorStatus = -1;
        spin_unlock(g_pAF_SpinLock);
    }

    return 0;
}

static inline int setAFInf(unsigned long a_u4Position)
{
	spin_lock(g_pAF_SpinLock);
	g_u4AF_INF = a_u4Position;
	spin_unlock(g_pAF_SpinLock);
	return 0;
}

static inline int setAFMacro(unsigned long a_u4Position)
{
	spin_lock(g_pAF_SpinLock);
	g_u4AF_MACRO = a_u4Position;
	spin_unlock(g_pAF_SpinLock);
	return 0;
}

/* ////////////////////////////////////////////////////////////// */
long DW9763AF_Ioctl(struct file *a_pstFile, unsigned int a_u4Command, unsigned long a_u4Param)
{
	long i4RetValue = 0;

	switch (a_u4Command) {
	case AFIOC_G_MOTORINFO:
		i4RetValue = getAFInfo((__user stAF_MotorInfo *) (a_u4Param));
		break;
        #ifdef LensdrvCM3
        case AFIOC_G_MOTORMETAINFO :
            i4RetValue = getAFMETA((__user stAF_MotorMETAInfo *)(a_u4Param));
        break;
        #endif
	case AFIOC_T_MOVETO:
		i4RetValue = moveAF(a_u4Param);
		break;

	case AFIOC_T_SETINFPOS:
		i4RetValue = setAFInf(a_u4Param);
		break;

	case AFIOC_T_SETMACROPOS:
		i4RetValue = setAFMacro(a_u4Param);
		break;

	default:
		LOG_INF("No CMD\n");
		i4RetValue = -EPERM;
		break;
	}

	return i4RetValue;
}

/* Main jobs: */
/* 1.Deallocate anything that "open" allocated in private_data. */
/* 2.Shut down the device on last close. */
/* 3.Only called once on last time. */
/* Q1 : Try release multiple times. */
int DW9763AF_Release(struct inode *a_pstInode, struct file *a_pstFile)
{
	LOG_INF("Start\n");
//miaowei motify for factory slow
if(FACTORY_BOOT != get_boot_mode()){
    if (*g_pAF_Opened == 2)
    {
        g_sr = 5;
        s4AF_WriteReg(200);
        msleep(10);
        s4AF_WriteReg(100);
        msleep(10);  
LOG_INF("miaowei: this boot is not factory_boot\n");  
    }
}

    if (*g_pAF_Opened)
    {
        LOG_INF("Free \n");

        spin_lock(g_pAF_SpinLock);
        *g_pAF_Opened = 0;
        spin_unlock(g_pAF_SpinLock);
    }

	LOG_INF("End\n");

	return 0;
}

void DW9763AF_SetI2Cclient(struct i2c_client *pstAF_I2Cclient, spinlock_t *pAF_SpinLock, int *pAF_Opened)
{
	g_pstAF_I2Cclient = pstAF_I2Cclient;
	g_pAF_SpinLock = pAF_SpinLock;
	g_pAF_Opened = pAF_Opened;
}
