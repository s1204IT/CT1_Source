/*
 * Copyright (c) 2013 STST Co., Ltd.
 */

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
//--------------------------------------------------------------------------
//
//SR200PC20用
//
//--------------------------------------------------------------------------
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/ctype.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/regulator/consumer.h>
#include <linux/fsl_devices.h>
#include <media/v4l2-chip-ident.h>
#include <media/v4l2-int-device.h>
#include "mxc_v4l2_capture.h"
#include "sr200pc20.h"

#define SR200PC20_VOLTAGE_ANALOG               2800000
#define SR200PC20_VOLTAGE_DIGITAL_CORE         1800000
#define SR200PC20_VOLTAGE_DIGITAL_IO           1800000

#define MIN_FPS 15
#define MAX_FPS 30
#define DEFAULT_FPS 30

#define SR200PC20_XCLK_MIN 6000000
#define SR200PC20_XCLK_MAX 48000000

#define CMD_REG_BYTE_READ		0x01
#define CMD_REG_WRITE			0x03

#define CAM_NAME					"cam"
static int CAM_MAJOR;

static struct regulator *io_regulator;
static struct regulator *core_regulator;
static struct regulator *analog_regulator;
static struct regulator *gpo_regulator;
static struct fsl_mxc_camera_platform_data *camera_plat;

//--- Sanyo CE add
static bool Cam_Init = false;
//--- Sanyo CE add

static int sr200pc20_probe(struct i2c_client *adapter,
				const struct i2c_device_id *device_id);
static int sr200pc20_remove(struct i2c_client *client);

static s32 sr200pc20_read_reg(u8 reg, u8 *val);
static s32 sr200pc20_write_reg(u8 reg, u8 val);

static const struct i2c_device_id sr200pc20_id[] = {
	{"sr200pc20", 0},
	{},
};

MODULE_DEVICE_TABLE(i2c, sr200pc20_id);

static struct i2c_driver sr200pc20_i2c_driver = {
	.driver = {
		  .owner = THIS_MODULE,
		  .name  = "sr200pc20",
		  },
	.probe  = sr200pc20_probe,
	.remove = sr200pc20_remove,
	.id_table = sr200pc20_id,
};

extern void gpio_sensor_active(unsigned int csi_index);
extern void gpio_sensor_inactive(unsigned int csi);

#ifdef CONFIG_MACH_MX53_BEJ2
extern void clear_bej2_suspend_state(void);
extern void set_bej2_suspend_state(void);
#endif

static s32 sr200pc20_write_reg(u8 reg, u8 val)
{
	u8 au8Buf[2] = {0};

	au8Buf[0] = reg;
	au8Buf[1] = val;

	if (i2c_master_send(sr200pc20_data.i2c_client, au8Buf, 2) < 0) {
		printk("%s:write reg error:reg=%x,val=%x\n",
			__func__, reg, val);
		return -1;
	}

	return 0;
}

static s32 sr200pc20_read_reg(u8 reg, u8 *val)
{
	u8 au8RegBuf[2] = {0};
	u8 u8RdVal = 0;

	au8RegBuf[0] = reg;

	if (1 != i2c_master_send(sr200pc20_data.i2c_client, au8RegBuf, 1)) {
		printk("%s:write reg error:reg=%x\n",
				__func__, reg);
		return -1;
	}

	if (1 != i2c_master_recv(sr200pc20_data.i2c_client, &u8RdVal, 1)) {
		printk("%s:read reg error:reg=%x,val=%x\n",
				__func__, reg, u8RdVal);
		return -1;
	}

	*val = u8RdVal;

	return u8RdVal;
}

static int sr200pc20_set_rotate_mode(struct reg_value *rotate_mode)
{
	int retval = 0;
#if 0
	s32 i = 0;
	s32 iModeSettingArySize = 2;
	register u32 Delay_ms = 0;
	register u8 RegAddr = 0;
	register u8 Mask = 0;
	register u8 Val = 0;
	u8 RegVal = 0;
	for (i = 0; i < iModeSettingArySize; ++i, ++rotate_mode) {
		Delay_ms = rotate_mode->u32Delay_ms;
		RegAddr = rotate_mode->u8RegAddr;
		Val = rotate_mode->u8Val;
		Mask = rotate_mode->u8Mask;
		if (Mask) {
			retval = sr200pc20_read_reg(RegAddr, &RegVal);
			if (retval < 0)
				goto err;

			RegVal &= ~(u8)Mask;
			Val &= Mask;
			Val |= RegVal;
		}

		retval = sr200pc20_write_reg(RegAddr, Val);
		if (retval < 0)
			goto err;

		if (Delay_ms)
			msleep(Delay_ms);
	}
err:
#endif
	return retval;
}

static int sr200pc20_set_mode(struct reg_value *Reg  , int  iModeSettingArySize )
{
	s32 i = 0;
	register u32 Delay_ms = 0;
	register u8 RegAddr = 0;
	register u8 Mask = 0;
	register u8 Val = 0;
	u8 RegVal = 0;
	int retval = 0;

	if( iModeSettingArySize == 0 ) {
		retval = -EPERM;
		goto err;
	}

	for (i = 0; i < iModeSettingArySize; ++i, ++Reg) {
		Delay_ms = Reg->u32Delay_ms;
		RegAddr = Reg->u8RegAddr;
		Val = Reg->u8Val;
		Mask = Reg->u8Mask;
		if (Mask) {
			retval = sr200pc20_read_reg(RegAddr, &RegVal);
			if (retval < 0)
				goto err;

			RegVal &= ~(u8)Mask;
			Val &= Mask;
			Val |= RegVal;
		}

		retval = sr200pc20_write_reg(RegAddr, Val);
		if (retval < 0)
			goto err;

		if (Delay_ms)
			msleep(Delay_ms);
	}
err:

	return retval;
}

static int sr200pc20_init_mode(enum sr200pc20_frame_rate frame_rate,
			    enum sr200pc20_mode mode)
{
	struct reg_value *pModeSetting = NULL;
	s32 i = 0;
	s32 j = 0;
	s32 iModeSettingArySize = 0;
	register u32 Delay_ms = 0;
	register u8 RegAddr = 0;
	register u8 Mask = 0;
	register u8 Val = 0;
	u8 RegVal = 0;
	int retval = 0;

	if (mode > (sr200pc20_mode_MAX+1) || mode < sr200pc20_mode_MIN) {
		printk("Wrong sr200pc20 mode detected! max:%d mode:%d\n",sr200pc20_mode_MAX,mode);
		return -1;
	}


	if(mode <= sr200pc20_mode_MAX) {	//撮影サイズデータ
		pModeSetting = sr200pc20_mode_info_data[frame_rate][mode].init_data_ptr;
		iModeSettingArySize =
		sr200pc20_mode_info_data[frame_rate][mode].init_data_size;

		sr200pc20_data.pix.width = sr200pc20_mode_info_data[frame_rate][mode].width;
		sr200pc20_data.pix.height = sr200pc20_mode_info_data[frame_rate][mode].height;
	} else {													//初期化データ
		pModeSetting = sr200pc20_mode_init[0].init_data_ptr;
		iModeSettingArySize =
		sr200pc20_mode_init[0].init_data_size;

		sr200pc20_data.pix.width = sr200pc20_mode_init[0].width;
		sr200pc20_data.pix.height = sr200pc20_mode_init[0].height;
	}
		printk("sr200pc20_init_mode width %d height %d\n",sr200pc20_data.pix.width,sr200pc20_data.pix.height);

		if (sr200pc20_data.pix.width == 0 || sr200pc20_data.pix.height == 0 ||
	    	pModeSetting == NULL || iModeSettingArySize == 0)
			return -EINVAL;

		printk("sr200pc20_init_mode frame_rate %d mode %d\n",frame_rate,mode);

		for (i = 0; i < iModeSettingArySize; ++i, ++pModeSetting) {
			Delay_ms = pModeSetting->u32Delay_ms;
			RegAddr = pModeSetting->u8RegAddr;
			Val = pModeSetting->u8Val;
			Mask = pModeSetting->u8Mask;

			if (Mask) {
				for(j=0;j<2;j++) {
					retval = sr200pc20_read_reg(RegAddr, &RegVal);
					if (retval < 0) {
						printk("sr200pc20_init_mode reg read error:%d Addr:%02x \n",retval,RegAddr);
						if( j == 1 ) goto err;		//Retry Error
					} else {
						RegVal &= ~(u8)Mask;
						Val &= Mask;
						Val |= RegVal;
						break;
					}
				}
			}

			for(j=0; j<2; j++ ) {
				retval = sr200pc20_write_reg(RegAddr, Val);
				if (retval < 0) {
					printk("sr200pc20_init_mode reg read error:%d Addr:0x%02x Val:%02x\n",retval,RegAddr,Val);
					if( j == 1 ) goto err;		//Retry Error
				} else {
					break;
				}
			}
			if (Delay_ms)
				msleep(Delay_ms);
		}

#if 0
	for (i = 0; i < 0xff; i++) {
		retval = sr200pc20_read_reg(i, &RegVal);
		if (retval < 0)
			goto err;
		printk("RegAddr %02x Val %02x\n",i,RegVal);
	}
#endif
err:
	return retval;
}

/* --------------- IOCTL functions from v4l2_int_ioctl_desc --------------- */

static int ioctl_g_ifparm(struct v4l2_int_device *s, struct v4l2_ifparm *p)
{
	if (s == NULL) {
		printk("   ERROR!! no slave device set!\n");
		return -1;
	}

	memset(p, 0, sizeof(*p));
	p->u.bt656.clock_curr = sr200pc20_data.mclk;
	pr_debug("   clock_curr=mclk=%d\n", sr200pc20_data.mclk);
	p->if_type = V4L2_IF_TYPE_BT656;
	p->u.bt656.mode = V4L2_IF_TYPE_BT656_MODE_NOBT_8BIT;
	p->u.bt656.clock_min = SR200PC20_XCLK_MIN;
	p->u.bt656.clock_max = SR200PC20_XCLK_MAX;
	p->u.bt656.bt_sync_correct = 1;  /* Indicate external vsync */
	p->u.bt656.nobt_vs_inv = 1;
	return 0;
}

/*サスペンド対応*/
#ifdef CONFIG_MACH_MX53_BEJ2
static int suspend_flag = 0;

void set_sr200pc20_suspend_flag(int on)
{
	suspend_flag = on;
}
#endif

//--- STS Add
// Camera Sleep On/OFF
static int sr200pc_sleep_set(int on) {
	u8	reg;
	u8	val;
	int ret;


		printk("sr200pc_sleep_set Set Sleep Mode:%d\n",on );

		if( (on != 0) && ( on != 1)) {
			printk("sr200pc_sleep_set Set Sleep Mode Error Mode:%d\n",on);
			
			goto errend;
		}

		reg = 0x03;
		val = 0x00;
		if( (ret = sr200pc20_write_reg(reg, val)) < 0 ) {
			printk("sr200pc_sleep_set Page 0 Set Error:%d\n",ret );
			goto errend;
		}

		reg = 0x01;
		val = 0x00;
		if( (ret = sr200pc20_read_reg(reg, &val)) < 0 ) {
			printk("sr200pc_sleep_set Page mode 1 Read Error:%d\n",ret );
			goto errend;
		}

		printk("sr200pc_sleep_set Page mode 1 Read Val:%x\n",val );

		val = ( val & 0xFE ) | (u8)on;
		
		printk("sr200pc_sleep_set Page mode 1 Write Val:%x\n",val );
		
		if( (ret = sr200pc20_write_reg(reg, val)) < 0 ) {
			printk("sr200pc_sleep_set Page 0 Set Error:%d\n",ret );
			goto errend;
		}

errend:
	return ret;

}

/*!
 * ioctl_s_power - V4L2 sensor interface handler for VIDIOC_S_POWER ioctl
 * @s: pointer to standard V4L2 device structure
 * @on: indicates power mode (on or off)
 *
 * Turns the power on or off, depending on the value of on and returns the
 * appropriate error code.
 */
static int ioctl_s_power(struct v4l2_int_device *s, int on)
{
	struct sensor *sensor = s->priv;

//printk("ioctl_s_power %d in\n",on);
//--- STS
//printk("ioctl_s_power Now on:%d Set on:%d\n",sr200pc20_data.on,on);
//--- STS

	if( on > 1 ) {
		if( on == 2 )	sr200pc_sleep_set(1);		//sleep on
		else 				sr200pc_sleep_set(0);		//sleep off
		return 0;
	}

#if 0
	if (on && !sensor->on) {
#else
	if (on && !sr200pc20_data.on) {
#endif
//printk("ioctl_s_power on\n");
		gpio_sensor_active(sr200pc20_data.csi);
		if (io_regulator)
			if (regulator_enable(io_regulator) != 0)
				return -EIO;
		if (core_regulator)
			if (regulator_enable(core_regulator) != 0)
				return -EIO;
		if (gpo_regulator)
			if (regulator_enable(gpo_regulator) != 0)
				return -EIO;
		if (analog_regulator)
			if (regulator_enable(analog_regulator) != 0)
				return -EIO;
		/* Make sure power on */
		if (camera_plat->pwdn)
		{
			camera_plat->pwdn(0);
			msleep(100);
			camera_plat->reset();
			msleep(10);
//--- STS
//printk("ioctl_s_power on Camera Init \n");
			Cam_Init = false;
//-- STS
		}
//-- STS
#if 0
	} else if (!on && sensor->on) {
#else
	} else if (!on && sr200pc20_data.on) {
#endif
//---
		if (camera_plat->pwdn)
			camera_plat->pwdn(1);
//--- STS
//printk("ioctl_s_power power off\n");
//-- STS
#ifdef CONFIG_MACH_MX53_BEJ2
		if(suspend_flag)
		{
			clear_bej2_suspend_state();
		}
#endif
		if (analog_regulator)
			regulator_disable(analog_regulator);
		if (core_regulator)
			regulator_disable(core_regulator);
		if (io_regulator)
			regulator_disable(io_regulator);
		if (gpo_regulator)
			regulator_disable(gpo_regulator);

		gpio_sensor_inactive(sr200pc20_data.csi);

#ifdef CONFIG_MACH_MX53_BEJ2
		if(suspend_flag)
		{
			set_bej2_suspend_state();
		}
#endif
	}

	sensor->on = on;
//--- STS
	sr200pc20_data.on = on;
//printk("ioctl_s_power %d out\n",sr200pc20_data.on);
//--- STS
//printk("ioctl_s_power %d out\n",on);

	return 0;
}

/*!
 * ioctl_g_parm - V4L2 sensor interface handler for VIDIOC_G_PARM ioctl
 * @s: pointer to standard V4L2 device structure
 * @a: pointer to standard V4L2 VIDIOC_G_PARM ioctl structure
 *
 * Returns the sensor's video CAPTURE parameters.
 */
static int ioctl_g_parm(struct v4l2_int_device *s, struct v4l2_streamparm *a)
{
	struct sensor *sensor = s->priv;
	struct v4l2_captureparm *cparm = &a->parm.capture;
	int ret = 0;

	switch (a->type) {
	/* This is the only case currently handled. */
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		memset(a, 0, sizeof(*a));
		a->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		cparm->capability = sensor->streamcap.capability;
		cparm->timeperframe = sensor->streamcap.timeperframe;
		cparm->capturemode = sensor->streamcap.capturemode;
		ret = 0;
		break;

	/* These are all the possible cases. */
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
	case V4L2_BUF_TYPE_VIDEO_OVERLAY:
	case V4L2_BUF_TYPE_VBI_CAPTURE:
	case V4L2_BUF_TYPE_VBI_OUTPUT:
	case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
	case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT:
		ret = -EINVAL;
		break;

	default:
		pr_debug("   type is unknown - %d\n", a->type);
		ret = -EINVAL;
		break;
	}

	return ret;
}

/*!
 * ioctl_s_parm - V4L2 sensor interface handler for VIDIOC_S_PARM ioctl
 * @s: pointer to standard V4L2 device structure
 * @a: pointer to standard V4L2 VIDIOC_S_PARM ioctl structure
 *
 * Configures the sensor to use the input parameters, if possible.  If
 * not possible, reverts to the old parameters and returns the
 * appropriate error code.
 */
static int ioctl_s_parm(struct v4l2_int_device *s, struct v4l2_streamparm *a)
{
	struct sensor *sensor = s->priv;
	struct v4l2_fract *timeperframe = &a->parm.capture.timeperframe;
	u32 tgt_fps;	/* target frames per secound */
	enum sr200pc20_frame_rate frame_rate;
	int ret = 0;

//--- STS
//printk("ioctl_s_parm  Now on:%d\n",sr200pc20_data.on);
//--- STS

	/* Make sure power on */
//--- STS
#if 0
	if (camera_plat->pwdn && !sensor->on)
#else
	if (camera_plat->pwdn && !sr200pc20_data.on)
#endif
//--- STS
	{
		camera_plat->pwdn(0);
		msleep(100);
		camera_plat->reset();
		msleep(10);
//--- STS
//printk("ioctl_s_parm Power on Camera Init on\n");
		Cam_Init = false;
		sr200pc20_data.on = true;
//-- STS
	}

//printk("ioctl_s_parm:%d \n",a->type);

	switch (a->type) {
	/* This is the only case currently handled. */
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		/* Check that the new frame rate is allowed. */
		if ((timeperframe->numerator == 0) ||
		    (timeperframe->denominator == 0)) {
			timeperframe->denominator = DEFAULT_FPS;
			timeperframe->numerator = 1;
		}

		tgt_fps = timeperframe->denominator /
			  timeperframe->numerator;

		if (tgt_fps > MAX_FPS) {
			timeperframe->denominator = MAX_FPS;
			timeperframe->numerator = 1;
		} else if (tgt_fps < MIN_FPS) {
			timeperframe->denominator = MIN_FPS;
			timeperframe->numerator = 1;
		}

		/* Actual frame rate we use */
		tgt_fps = timeperframe->denominator /
			  timeperframe->numerator;

		if (tgt_fps == 15)
			frame_rate = sr200pc20_15_fps;
		else if (tgt_fps == 30)
			frame_rate = sr200pc20_30_fps;
		else {
			printk(" The camera frame rate is not supported!\n");
			return -EINVAL;
		}
//--- Sanyo CE Camera Init parameter Output
		if( !Cam_Init  ) {
			ret = sr200pc20_init_mode(0,sr200pc20_mode_INIT_800_600);
			Cam_Init = true;
		}
//--- Sanyo CE Camera Init parameter Output

		sensor->streamcap.timeperframe = *timeperframe;
		sensor->streamcap.capturemode =
				(u32)a->parm.capture.capturemode;

		ret = sr200pc20_init_mode(frame_rate,
			       sensor->streamcap.capturemode);
		break;

	/* These are all the possible cases. */
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
	case V4L2_BUF_TYPE_VIDEO_OVERLAY:
	case V4L2_BUF_TYPE_VBI_CAPTURE:
	case V4L2_BUF_TYPE_VBI_OUTPUT:
	case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
	case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT:
		pr_debug("   type is not " \
			"V4L2_BUF_TYPE_VIDEO_CAPTURE but %d\n",
			a->type);
		ret = -EINVAL;
		break;

	default:
		pr_debug("   type is unknown - %d\n", a->type);
		ret = -EINVAL;
		break;
	}

	return ret;
}

//---- シリアル通信が成功するか確認
static int ioctl_s_fmt_cap(struct v4l2_int_device *s, struct v4l2_format *f)
{
	u8 val;
	int ret = 0;

	printk("\n***  Camera I2C Init Check Start ***\n");
	if( (ret = sr200pc20_write_reg(0x03,0x00)) < 0 ) {	//P0セット
		printk("***  Camera I2C Init Check Write Error ***\n");
	} else {
		if( (ret = sr200pc20_read_reg(0x04,&val)) < 0 ) {
			printk("***  Camera I2C Init Check Read Error ***\n");
		} else {
			printk("***  Camera I2C Init Check Read Chip ID:%02x ***\n",val);
		}
	}
	printk("***  Camera I2C Init Check End ***\n\n");

	return ret;
}
//----

/*!
 * ioctl_g_fmt_cap - V4L2 sensor interface handler for ioctl_g_fmt_cap
 * @s: pointer to standard V4L2 device structure
 * @f: pointer to standard V4L2 v4l2_format structure
 *
 * Returns the sensor's current pixel format in the v4l2_format
 * parameter.
 */
static int ioctl_g_fmt_cap(struct v4l2_int_device *s, struct v4l2_format *f)
{
	struct sensor *sensor = s->priv;

	f->fmt.pix = sensor->pix;

	return 0;
}

/*!
 * ioctl_g_ctrl - V4L2 sensor interface handler for VIDIOC_G_CTRL ioctl
 * @s: pointer to standard V4L2 device structure
 * @vc: standard V4L2 VIDIOC_G_CTRL ioctl structure
 *
 * If the requested control is supported, returns the control's current
 * value from the video_control[] array.  Otherwise, returns -EINVAL
 * if the control is not supported.
 */
static int ioctl_g_ctrl(struct v4l2_int_device *s, struct v4l2_control *vc)
{
	int ret = 0;

	switch (vc->id) {
	case V4L2_CID_BRIGHTNESS:
		vc->value = sr200pc20_data.brightness;
		break;
	case V4L2_CID_HUE:
		vc->value = sr200pc20_data.hue;
		break;
	case V4L2_CID_CONTRAST:
		vc->value = sr200pc20_data.contrast;
		break;
	case V4L2_CID_SATURATION:
		vc->value = sr200pc20_data.saturation;
		break;
	case V4L2_CID_RED_BALANCE:
		vc->value = sr200pc20_data.red;
		break;
	case V4L2_CID_BLUE_BALANCE:
		vc->value = sr200pc20_data.blue;
		break;
	case V4L2_CID_EXPOSURE:
		vc->value = sr200pc20_data.ae_mode;
		break;
	default:
		ret = -EINVAL;
	}

	return ret;
}

/*!
 * ioctl_s_ctrl - V4L2 sensor interface handler for VIDIOC_S_CTRL ioctl
 * @s: pointer to standard V4L2 device structure
 * @vc: standard V4L2 VIDIOC_S_CTRL ioctl structure
 *
 * If the requested control is supported, sets the control's current
 * value in HW (and updates the video_control[] array).  Otherwise,
 * returns -EINVAL if the control is not supported.
 */
static int ioctl_s_ctrl(struct v4l2_int_device *s, struct v4l2_control *vc)
{
	int retval = 0;
	struct reg_value *pModeSetting = NULL;
	s32 iModeSettingArySize = 0;
	int scene;
	int type;

	pr_debug("In sr200pc20:ioctl_s_ctrl %d\n",
		 vc->id);

//	printk("In sr200pc20:ioctl_s_ctrl ID:%x Val:%d\n",vc->id,vc->value);

	switch (vc->id) {
	case V4L2_CID_BRIGHTNESS:
		break;
	case V4L2_CID_CONTRAST:
		break;
	case V4L2_CID_SATURATION:
		break;
	case V4L2_CID_HUE:
		break;
	case V4L2_CID_AUTO_WHITE_BALANCE:

#ifdef DEBUG
		printk("WhiteBalance data Set mode:%04d Get mode:%04d\n",vc->value,sr200pc20_whitebalance_data[vc->value].mode);
#endif
		if( vc->value <= sr200pc20_wb_mode_MAX ) {
			pModeSetting = sr200pc20_whitebalance_data[vc->value].init_data_ptr;
			iModeSettingArySize = sr200pc20_whitebalance_data[vc->value].init_data_size;
			retval = sr200pc20_set_mode(pModeSetting,iModeSettingArySize);

		}
		else 
			retval = -EPERM;
		break;
	case V4L2_CID_DO_WHITE_BALANCE:
		break;
	case V4L2_CID_RED_BALANCE:
		break;
	case V4L2_CID_BLUE_BALANCE:
		break;
	case V4L2_CID_GAMMA:
		break;
	case V4L2_CID_EXPOSURE:
		scene = vc->value / 10;
		type  = vc->value % 10;		//このカメラでは撮影サイズでのシーン設定の差異がない

#ifdef DEBUG
		printk("Scen data Set mode:%04d Get mode:%04d\n",vc->value,sr200pc20_scene_data[scene].mode);
#endif
		if( scene <= sr200pc20_scene_mode_MAX ) {
			pModeSetting = sr200pc20_scene_data[scene].init_data_ptr;
			iModeSettingArySize = sr200pc20_scene_data[scene].init_data_size;
			retval = sr200pc20_set_mode(pModeSetting,iModeSettingArySize);

		} else {
			retval = -EPERM;
		}
		break;
	case V4L2_CID_AUTOGAIN:
		break;
	case V4L2_CID_GAIN:
		break;
	case V4L2_CID_HFLIP:
		break;
	case V4L2_CID_VFLIP:
		break;
	case V4L2_CID_MXC_ROT:
	case V4L2_CID_MXC_VF_ROT:
		switch (vc->value) {
		case V4L2_MXC_CAM_ROTATE_NONE:
			if (sr200pc20_set_rotate_mode(sr200pc20_rotate_none))
				retval = -EPERM;
			break;
		case V4L2_MXC_CAM_ROTATE_VERT_FLIP:
			if (sr200pc20_set_rotate_mode(sr200pc20_rotate_vert_flip))
				retval = -EPERM;
			break;
		case V4L2_MXC_CAM_ROTATE_HORIZ_FLIP:
			if (sr200pc20_set_rotate_mode(sr200pc20_rotate_horiz_flip))
				retval = -EPERM;
			break;
		case V4L2_MXC_CAM_ROTATE_180:
			if (sr200pc20_set_rotate_mode(sr200pc20_rotate_180))
				retval = -EPERM;
			break;
		default:
			retval = -EPERM;
			break;
		}
		break;
	case V4L2_CID_COLORFX:
#ifdef DEBUG
		printk("Color effect data Set mode:%04d Get mode:%04d\n",vc->value,sr200pc20_effect_data[vc->value].mode);
#endif

		if( vc->value <= sr200pc20_effect_mode_MAX ) {
			pModeSetting = sr200pc20_effect_data[vc->value].init_data_ptr;
			iModeSettingArySize = sr200pc20_effect_data[vc->value].init_data_size;
			retval = sr200pc20_set_mode(pModeSetting,iModeSettingArySize);
//printk("sr200pc20 Color effect  retval:%d\n",retval);
		}
		else 
			retval = -EPERM;
		break;

	case V4L2_CID_ZOOM_RELATIVE:
		scene = vc->value / 10;
		type  = vc->value % 10;

#ifdef DEBUG
		printk("Zoom data Set mode:%04d Get mode:%04x\n",vc->value,sr200pc20_zoom_data[scene][type].mode);
#endif
//-- zoom未サポートのため削除
#if 0
		if(( scene <= 5 ) && ( type <= 4 ) ){
			pModeSetting = sr200pc20_zoom_data[scene][type].init_data_ptr;
			iModeSettingArySize = sr200pc20_zoom_data[scene][type].init_data_size;
			retval = sr200pc20_set_mode(pModeSetting,iModeSettingArySize);
		} else {
			retval = -EPERM;
		}
#endif
//-- zoom未サポートのため削除
		break;
	default:
		retval = -EPERM;
		break;
	}

	return retval;
}

/*!
 * ioctl_enum_framesizes - V4L2 sensor interface handler for
 *			   VIDIOC_ENUM_FRAMESIZES ioctl
 * @s: pointer to standard V4L2 device structure
 * @fsize: standard V4L2 VIDIOC_ENUM_FRAMESIZES ioctl structure
 *
 * Return 0 if successful, otherwise -EINVAL.
 */
static int ioctl_enum_framesizes(struct v4l2_int_device *s,
				 struct v4l2_frmsizeenum *fsize)
{
	if (fsize->index > sr200pc20_mode_MAX)
		return -EINVAL;

	fsize->discrete.width =
			max(sr200pc20_mode_info_data[0][fsize->index].width,
			    sr200pc20_mode_info_data[1][fsize->index].width);
	fsize->discrete.height =
			max(sr200pc20_mode_info_data[0][fsize->index].height,
			    sr200pc20_mode_info_data[1][fsize->index].height);
	fsize->pixel_format = sr200pc20_data.pix.pixelformat;

//	printk("ioctl_enum_framesizes index:%d width %d height%d\n",fsize->index,fsize->discrete.width,fsize->discrete.height);

	return 0;
}

/*!
 * ioctl_g_chip_ident - V4L2 sensor interface handler for
 *			VIDIOC_DBG_G_CHIP_IDENT ioctl
 * @s: pointer to standard V4L2 device structure
 * @id: pointer to int
 *
 * Return 0.
 */
static int ioctl_g_chip_ident(struct v4l2_int_device *s, int *id)
{
	((struct v4l2_dbg_chip_ident *)id)->match.type =
					V4L2_CHIP_MATCH_I2C_DRIVER;
	strcpy(((struct v4l2_dbg_chip_ident *)id)->match.name, "sr200pc20_camera");

	return 0;
}

/*!
 * ioctl_init - V4L2 sensor interface handler for VIDIOC_INT_INIT
 * @s: pointer to standard V4L2 device structure
 */
static int ioctl_init(struct v4l2_int_device *s)
{
	return 0;
}

/*!
 * ioctl_enum_fmt_cap - V4L2 sensor interface handler for VIDIOC_ENUM_FMT
 * @s: pointer to standard V4L2 device structure
 * @fmt: pointer to standard V4L2 fmt description structure
 *
 * Return 0.
 */
static int ioctl_enum_fmt_cap(struct v4l2_int_device *s,
			      struct v4l2_fmtdesc *fmt)
{
	if (fmt->index > sr200pc20_mode_MAX)
		return -EINVAL;

	fmt->pixelformat = sr200pc20_data.pix.pixelformat;

	return 0;
}

/*!
 * ioctl_dev_init - V4L2 sensor interface handler for vidioc_int_dev_init_num
 * @s: pointer to standard V4L2 device structure
 *
 * Initialise the device when slave attaches to the master.
 */
static int ioctl_dev_init(struct v4l2_int_device *s)
{
	int retval = 0;
	struct sensor *sensor = s->priv;
	u32 tgt_xclk;	/* target xclk */
	u32 tgt_fps;	/* target frames per secound */
	enum sr200pc20_frame_rate frame_rate;

	gpio_sensor_active(sr200pc20_data.csi);
//--- Sanyo CE 起動時は電源OFF
	sr200pc20_data.on = false;
//--- Sanyo CE 起動時は電源OFF

	/* mclk */
	tgt_xclk = sr200pc20_data.mclk;
	tgt_xclk = min(tgt_xclk, (u32)SR200PC20_XCLK_MAX);
	tgt_xclk = max(tgt_xclk, (u32)SR200PC20_XCLK_MIN);
	sr200pc20_data.mclk = tgt_xclk;

	pr_debug("   Setting mclk to %d MHz\n", tgt_xclk / 1000000);
	set_mclk_rate(&sr200pc20_data.mclk, sr200pc20_data.csi);

	/* Default camera frame rate is set in probe */
	tgt_fps = sensor->streamcap.timeperframe.denominator /
		  sensor->streamcap.timeperframe.numerator;

	if (tgt_fps == 15)
		frame_rate = sr200pc20_15_fps;
	else if (tgt_fps == 30)
		frame_rate = sr200pc20_30_fps;
	else
		return -EINVAL; /* Only support 15fps or 30fps now. */



	return retval;
}

/*!
 * ioctl_dev_exit - V4L2 sensor interface handler for vidioc_int_dev_exit_num
 * @s: pointer to standard V4L2 device structure
 *
 * Delinitialise the device when slave detaches to the master.
 */
static int ioctl_dev_exit(struct v4l2_int_device *s)
{
	gpio_sensor_inactive(sr200pc20_data.csi);

	return 0;
}

static int sr200pc20_cam_ioctl(struct file *file, unsigned int cmd, void *arg)
{
	int 		ret = 0;
	u8		*buf = arg;
	s32		result=0;	
//	printk("sr200pc20_camera_command() Call\n");

	switch( cmd ) {
		case CMD_REG_BYTE_READ:
//				printk("Commnd:BYTE Read Reg:0x%02x\n",buf[0]);
			if((result = sr200pc20_read_reg(buf[0], &buf[1])) < 0 ) 
				ret = result;
			else {
				buf[0] = buf[1];
				printk("Commnd:BYTE Read Value:0x%02x\n",buf[0]);
			}
			break;

		case CMD_REG_WRITE:
//			printk("Commnd:Write Reg:0x%02x Value:0x%02x\n",buf[0],buf[1]);
			if( (result = sr200pc20_write_reg(buf[0], buf[1])) < 0 )
				ret = result;
			break;

		default:
			ret = -EIO;				//
	}

//	printk("sr200pc20_camera_command() Call End Return Status:%d\n",ret);
	return ret;
}

static const struct file_operations cam_fops = {
	.owner					= THIS_MODULE,
	.unlocked_ioctl	= sr200pc20_cam_ioctl,
};

/*!
 * This structure defines all the ioctls for this module and links them to the
 * enumeration.
 */
static struct v4l2_int_ioctl_desc sr200pc20_ioctl_desc[] = {
	{vidioc_int_dev_init_num, (v4l2_int_ioctl_func*)ioctl_dev_init},
	{vidioc_int_dev_exit_num, ioctl_dev_exit},
	{vidioc_int_s_power_num, (v4l2_int_ioctl_func*)ioctl_s_power},
	{vidioc_int_g_ifparm_num, (v4l2_int_ioctl_func*)ioctl_g_ifparm},
/*	{vidioc_int_g_needs_reset_num,
				(v4l2_int_ioctl_func *)ioctl_g_needs_reset}, */
/*	{vidioc_int_reset_num, (v4l2_int_ioctl_func *)ioctl_reset}, */
	{vidioc_int_init_num, (v4l2_int_ioctl_func*)ioctl_init},
	{vidioc_int_enum_fmt_cap_num,
				(v4l2_int_ioctl_func *)ioctl_enum_fmt_cap},
/*	{vidioc_int_try_fmt_cap_num,
				(v4l2_int_ioctl_func *)ioctl_try_fmt_cap}, */
	{vidioc_int_g_fmt_cap_num, (v4l2_int_ioctl_func*)ioctl_g_fmt_cap},
	{vidioc_int_s_fmt_cap_num, (v4l2_int_ioctl_func *)ioctl_s_fmt_cap}, //I2C通信エラー確認
	{vidioc_int_g_parm_num, (v4l2_int_ioctl_func*)ioctl_g_parm},
	{vidioc_int_s_parm_num, (v4l2_int_ioctl_func*)ioctl_s_parm},
/*	{vidioc_int_queryctrl_num, (v4l2_int_ioctl_func *)ioctl_queryctrl}, */
	{vidioc_int_g_ctrl_num, (v4l2_int_ioctl_func*)ioctl_g_ctrl},
	{vidioc_int_s_ctrl_num, (v4l2_int_ioctl_func*)ioctl_s_ctrl},
	{vidioc_int_enum_framesizes_num,
				(v4l2_int_ioctl_func *)ioctl_enum_framesizes},
	{vidioc_int_g_chip_ident_num,
				(v4l2_int_ioctl_func *)ioctl_g_chip_ident},
};

static struct v4l2_int_slave sr200pc20_slave = {
	.ioctls = sr200pc20_ioctl_desc,
	.num_ioctls = ARRAY_SIZE(sr200pc20_ioctl_desc),
};

static struct v4l2_int_device sr200pc20_int_device = {
	.module = THIS_MODULE,
	.name = "sr200pc20",
	.type = v4l2_int_type_slave,
	.u = {
		.slave = &sr200pc20_slave,
	},
};

/*!
 * sr200pc20 I2C probe function
 *
 * @param adapter            struct i2c_adapter *
 * @return  Error code indicating success or failure
 */
static int sr200pc20_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	int retval;
	struct fsl_mxc_camera_platform_data *plat_data = client->dev.platform_data;

	/* Set initial values for the sensor struct. */
	memset(&sr200pc20_data, 0, sizeof(sr200pc20_data));
	sr200pc20_data.mclk = 24000000; /* 6 - 48 MHz, typical 24MHz */
	sr200pc20_data.mclk = plat_data->mclk;
	sr200pc20_data.csi = plat_data->csi;

	sr200pc20_data.i2c_client = client;
	sr200pc20_data.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	sr200pc20_data.pix.width = 640;
	sr200pc20_data.pix.height = 480;
	sr200pc20_data.streamcap.capability = V4L2_MODE_HIGHQUALITY |
					   V4L2_CAP_TIMEPERFRAME;
	sr200pc20_data.streamcap.capturemode = -1;
	sr200pc20_data.streamcap.timeperframe.denominator = DEFAULT_FPS;
	sr200pc20_data.streamcap.timeperframe.numerator = 1;

#if 0
	if (plat_data->io_regulator) {
		io_regulator = regulator_get(&client->dev,
					     plat_data->io_regulator);
		if (!IS_ERR(io_regulator)) {
			regulator_set_voltage(io_regulator,
					      SR200PC20_VOLTAGE_DIGITAL_IO,
					      SR200PC20_VOLTAGE_DIGITAL_IO);
			if (regulator_enable(io_regulator) != 0) {
				printk("%s:io set voltage error\n", __func__);
				goto err1;
			} else {
				dev_dbg(&client->dev,
					"%s:io set voltage ok\n", __func__);
			}
		} else
			io_regulator = NULL;
	}
#endif

	if (plat_data->core_regulator) {
		core_regulator = regulator_get(&client->dev,
					       plat_data->core_regulator);
		if (!IS_ERR(core_regulator)) {
			regulator_set_voltage(core_regulator,
					      SR200PC20_VOLTAGE_DIGITAL_CORE,
					      SR200PC20_VOLTAGE_DIGITAL_CORE);
			if (regulator_enable(core_regulator) != 0) {
				printk("%s:core set voltage error\n", __func__);
				goto err2;
			} else {
				dev_dbg(&client->dev,
					"%s:core set voltage ok\n", __func__);
			}
		} else {
			printk("Not get core_regulator\n");
			core_regulator = NULL;
		}
	}

	if (plat_data->analog_regulator) {
		analog_regulator = regulator_get(&client->dev,
						 plat_data->analog_regulator);
		if (!IS_ERR(analog_regulator)) {
			regulator_set_voltage(analog_regulator,
					      SR200PC20_VOLTAGE_ANALOG,
					      SR200PC20_VOLTAGE_ANALOG);
			if (regulator_enable(analog_regulator) != 0) {
				printk("%s:analog set voltage error\n",
					__func__);
				goto err3;
			} else {
				dev_dbg(&client->dev,
					"%s:analog set voltage ok\n", __func__);
			}
		} else {
			printk("Not get analog_regulator\n");
			analog_regulator = NULL;
		}
	}

	if (plat_data->pwdn)
	{
		plat_data->pwdn(0);
		msleep(100);
		plat_data->reset();
		msleep(10);
	}

	if( (retval = sr200pc20_write_reg(0x03,0x00)) == 0 )
	{

		camera_plat = plat_data;

		sr200pc20_int_device.priv = &sr200pc20_data;
		retval = v4l2_int_device_register(&sr200pc20_int_device);
		return retval;
	}

err3:
	if (core_regulator) {
		regulator_disable(core_regulator);
		regulator_put(core_regulator);
	}
err2:
#if 0
	if (io_regulator) {
		regulator_disable(io_regulator);
		regulator_put(io_regulator);
	}
#endif
err1:
	return -ENODEV;
}

/*!
 * sr200pc20 I2C detach function
 *
 * @param client            struct i2c_client *
 * @return  Error code indicating success or failure
 */
static int sr200pc20_remove(struct i2c_client *client)
{
	v4l2_int_device_unregister(&sr200pc20_int_device);

	if (gpo_regulator) {
		regulator_disable(gpo_regulator);
		regulator_put(gpo_regulator);
	}

	if (analog_regulator) {
		regulator_disable(analog_regulator);
		regulator_put(analog_regulator);
	}

	if (core_regulator) {
		regulator_disable(core_regulator);
		regulator_put(core_regulator);
	}

	if (io_regulator) {
		regulator_disable(io_regulator);
		regulator_put(io_regulator);
	}

	return 0;
}

static struct class *cam_dev_class;

/*!
 * sr200pc20 init function
 * Called by insmod sr200pc20_camera.ko.
 *
 * @return  Error code indicating success or failure
 */
static __init int sr200pc20_init(void)
{
	int res;
	struct fsl_mxc_camera_platform_data *plat_data;

	printk("sr200pc20_init\n");

	res = i2c_add_driver(&sr200pc20_i2c_driver);
	if( res < 0 )
		goto out; 

 	plat_data = sr200pc20_data.i2c_client->dev.platform_data;

	if(sr200pc20_write_reg(0x03,0x00))
	{
		if (plat_data->pwdn)
		{
			plat_data->pwdn(1);
		}
		goto out_del_driver;
	}

	CAM_MAJOR = register_chrdev(0, CAM_NAME, &cam_fops);
	if (CAM_MAJOR < 0) {
		printk(KERN_ERR "%s: Driver Initialisation failed\n", __FILE__);
		res = CAM_MAJOR;
		goto out;
	}

	printk("/dev/cam  MAJOR:%d\n",CAM_MAJOR);

	cam_dev_class = class_create(THIS_MODULE, "cam-dev");
	if (IS_ERR(cam_dev_class)) {
		res = PTR_ERR(cam_dev_class);
		goto out_unreg_chrdev;
	}

	/* register this i2c device with the driver core */
	if( device_create(cam_dev_class, NULL,
				     MKDEV(CAM_MAJOR, 0), NULL, "cam") == NULL ) {
		res = -1;
		goto out_unreg_class;
	}

	if (plat_data->pwdn)
	{
		plat_data->pwdn(1);
	}

	printk("sr200pc20_init Success \n");
	return res;

out_destroy:
	device_destroy(cam_dev_class, MKDEV(CAM_MAJOR, 0));
out_unreg_class:
	class_destroy(cam_dev_class);
out_unreg_chrdev:
	unregister_chrdev(CAM_MAJOR, CAM_NAME);
out_del_driver:
//	i2c_del_driver(&sr200pc20_i2c_driver);
	i2c_unregister_device(sr200pc20_data.i2c_client);
out:
	printk("sr200pc20_init Fail \n");
	return res;
}

/*!
 * sr200pc20 cleanup function
 * Called on rmmod sr200pc20_camera.ko
 *
 * @return  Error code indicating success or failure
 */
static void __exit sr200pc20_clean(void)
{
printk("sr200pc20_clean\n");
	class_destroy(cam_dev_class);
	unregister_chrdev(CAM_MAJOR, CAM_NAME);
	i2c_del_driver(&sr200pc20_i2c_driver);
}

module_init(sr200pc20_init);
module_exit(sr200pc20_clean);

MODULE_AUTHOR("STST Co., Ltd");
MODULE_DESCRIPTION("sr200pc20 Camera Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
MODULE_ALIAS("CSI");
