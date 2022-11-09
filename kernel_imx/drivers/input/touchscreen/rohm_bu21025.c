/*
 * Driver for Rohm BU21025GUL - A Resistive touch screen controller with
 * i2c interface
 *
 * Based on mcs5000_ts.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/hrtimer.h>
#include <linux/semaphore.h>
#include <linux/earlysuspend.h>
#include <linux/gpio.h>
#include <linux/rtc.h>		//異物混入対応

//#define DEBUG
//#define DEBUG2

/* OPECODE */
#define POWER_SETTING 0x00
#define AUX_MEASURE   0x02
#define RESET         0x05
#define X_SET         0x08
#define Y_SET         0x09
#define Z_SET         0x0a
#define SETUP         0x0b
#define X_MEASURE     0x0c
#define Y_MEASURE     0x0d
#define Z1_MEASURE    0x0e
#define Z2_MEASURE    0x0f

#define ADC_MAX			0x1000
#define MAX_X			0xfff
#define MAX_Y			0xfff

/* OPERAND*/
/*type 0*/
#define PD_OFF (1 << 2)
#define PD_ON 0

#define X_CALIB_MAX 1024
#define Y_CALIB_MAX 768

#define READ_TH 100

#define Z1_DIFF_MAX 250

//#define RX_VALUE 563	// Xパネルの実測抵抗値
//#define RY_VALUE 319	// Yパネルの実測抵抗値
//#define RX_VALUE 500	// XパネルMIN
//#define RY_VALUE 300	// YパネルMIN
//#define RX_VALUE 585	// 平均
//#define RY_VALUE 360	// 平均
//#define RX_VALUE 610	// XパネルMAX
//#define RY_VALUE 400	// YパネルMAX
#define RX_VALUE 610
#define RY_VALUE 380

#define X_INV 0
#define Y_INV 1
#define XY_SWAP 0

#define CMD_PRM_READ	0x01
#define CMD_PRM_WRITE	0x03
#define TS_NAME			"ts"
static int TS_MAJOR;
//#define SHOW_REG

struct rohm_bu21025gul_data {
	struct i2c_client		*client;
	struct input_dev		*input_dev;
	struct work_struct  work;
	struct hrtimer timer;
	struct mutex lock;
	volatile bool touch_stopped;
	bool int_enable;
#ifdef CONFIG_EARLYSUSPEND
	struct early_suspend		es_handler;
#endif
};

static struct workqueue_struct *wq;

static struct rohm_bu21025gul_data *rohm_bu21025gul_data;

//異物混入対応-s
struct touch_info {
	int flag;
	int x_val;
	int y_val;
	struct timespec uptime;
};

static struct touch_info t_info;

static int first_touch_flag = 0;

static void touch_info_set(int x_data, int y_data)
{
	int x,y,swap_data;

	first_touch_flag = 1;
	
	t_info.flag = 1;
	
	x = x_data;
	y = y_data;

	if(X_INV)
	{
		x = MAX_X - x;
	}

	if(Y_INV)
	{
		y = MAX_Y - y;
	}

	if(XY_SWAP)
	{
		swap_data = x;
		x = y;
		y = swap_data;
	}
	
	t_info.x_val = x;
	t_info.y_val = y;

	do_posix_clock_monotonic_gettime(&t_info.uptime);
	monotonic_to_bootbased(&t_info.uptime);
//printk("set:::flag:%d time_sec:%ld uptime_nsec:%09ld x_val:%d y_val:%d  \n", t_info.flag, t_info.uptime.tv_sec, t_info.uptime.tv_nsec, t_info.x_val, t_info.y_val );
}

static void touch_info_clear(void)
{
	struct touch_info clear_data = { 0, 0, 0, { 0, 0 } };
	
	first_touch_flag = 0;
	t_info = clear_data;
//printk("clear:::flag:%d time_sec:%ld uptime_nsec:%09ld x_val:%d y_val:%d  \n", t_info.flag, t_info.uptime.tv_sec, t_info.uptime.tv_nsec, t_info.x_val, t_info.y_val );
}
//異物混入対応-e

void rohm_bu21025gul_lock(struct rohm_bu21025gul_data *data)
{
	mutex_lock(&data->lock);
}

void rohm_bu21025gul_unlock(struct rohm_bu21025gul_data *data)
{
	mutex_unlock(&data->lock);
}

static int rohm_bu21025gul_write_reg(struct i2c_client *client, int data)
{
	struct i2c_msg i2cmsg;
	unsigned char buf = 0;
	int ret = 0;

	buf = data;

	i2cmsg.addr  = client->addr;
	i2cmsg.len   = 1;
	i2cmsg.buf   = &buf;

	i2cmsg.flags = 0;
	ret = i2c_transfer(client->adapter, &i2cmsg, 1);

	if (ret < 0) {
		dev_info(&client->dev,\
		"_%s:master_xfer Failed!!\n", __func__);
		return ret;
	}

	return 0;
}

static int rohm_bu21025gul_read_data(struct i2c_client *client)
{
	unsigned char buf[2] = {0, 0};
	struct i2c_msg i2cmsg;
	int ret = 0;

	i2cmsg.addr  = client->addr ;
	i2cmsg.len   = 2;
	i2cmsg.buf   = &buf[0];

	/*To read the data on I2C set flag to I2C_M_RD */
	i2cmsg.flags = I2C_M_RD;

	/* Start the i2c transfer by calling host i2c driver function */
	ret = i2c_transfer(client->adapter, &i2cmsg, 1);
	if (ret < 0) {
		dev_info(&client->dev,\
		"_%s:master_xfer Failed!!\n", __func__);
		return ret;
	}

	return (buf[0] << 4) | (buf[1] >> 4);
}

static u16 rohm_bu21025gul_aux(void)
{
	struct i2c_client *client = rohm_bu21025gul_data->client;
	u16 sample_data = 0;
	int err;

	rohm_bu21025gul_lock(rohm_bu21025gul_data);

	err = rohm_bu21025gul_write_reg(client, AUX_MEASURE << 4);
	if (err) {
		dev_err(&client->dev, "work error\n");
		goto err_proc;
	}
	udelay(150);
	sample_data = rohm_bu21025gul_read_data(client);
	sample_data = (sample_data*3150)/4096;

err_proc:
	rohm_bu21025gul_unlock(rohm_bu21025gul_data);

	return sample_data;
}

u16 rohm_bu21025gul_read_adc(void)
{
	u16 adc_data;

	adc_data = rohm_bu21025gul_aux();

	return adc_data;
}
EXPORT_SYMBOL_GPL(rohm_bu21025gul_read_adc);

static int calibration[9];
module_param_array(calibration, int, NULL, S_IRUGO | S_IWUSR);

static int calibration_pointer(int *x_orig, int *y_orig)
{
	int x,y,swap_data;

	x = *x_orig;
	y = *y_orig;

	if(X_INV)
	{
		x = MAX_X - x;
	}

	if(Y_INV)
	{
		y = MAX_Y - y;
	}

	if(XY_SWAP)
	{
		swap_data = x;
		x = y;
		y = swap_data;
	}

	/*本来は、4096系で座標を上げて、キャリブレーションすべきだが、
	  Androidが画面サイズにイベントを補正してしまうので、
	  画面サイズに変換しておく。
	*/
	x = x * X_CALIB_MAX / ADC_MAX;
	y = y * Y_CALIB_MAX / ADC_MAX;

	/*calibration*/
	*x_orig = x;
	*y_orig = y;

	if (calibration[6] == 0)
		return -1;

	x = calibration[0] * (*x_orig) +
		calibration[1] * (*y_orig) +
		calibration[2];
	x /= calibration[6];
	if (x < 0)
		x = 0;
	y = calibration[3] * (*x_orig) +
		calibration[4] * (*y_orig) +
		calibration[5];
	y /= calibration[6];
	if (y < 0)
		y = 0;

	/**/

	*x_orig = x;
	*y_orig = y;

	return 0;
}

static enum hrtimer_restart rohm_bu21025gul_timer_func(struct hrtimer *timer)
{
	struct rohm_bu21025gul_data *data = container_of(timer, struct rohm_bu21025gul_data, timer);

	if( data->touch_stopped != true )
	{
		if (wq)
			queue_work(wq, &data->work);
	}
	return HRTIMER_NORESTART;
}

static irqreturn_t rohm_bu21025gul_interrupt(int irq, void *dev_id)
{
	struct rohm_bu21025gul_data *data = dev_id;

	if( data->touch_stopped != true )
	{
		data->int_enable = false;
		disable_irq_nosync(irq);
		if (wq)
			queue_work(wq,&data->work);
	}

	return IRQ_HANDLED;
}

static void rohm_bu21025gul_work(struct work_struct *work)
{
	struct rohm_bu21025gul_data *data = container_of(work, struct rohm_bu21025gul_data, work);
	struct i2c_client *client = data->client;
	int err;
	int read_x,read_y,read_z1;
	unsigned int touch = 0 ;
	static int back_z1 = -1;
	int touch_value;
	static int keep_x = -1,keep_y = -1,keep_z1 = -1;
	static int send_x = -1,send_y = -1,send_z1 = -1;
	static unsigned int send_cnt = 0;

	rohm_bu21025gul_lock(data);

	err = rohm_bu21025gul_write_reg(client, (X_SET << 4) | 0x00);
	if (err) {
		dev_err(&client->dev, "work error\n");
		goto err_proc;
	}

//	msleep(1);
//	udelay(600);
	udelay(1500);

	err = rohm_bu21025gul_write_reg(client, (X_MEASURE << 4) | PD_OFF);
	if (err) {
		dev_err(&client->dev, "work error\n");
		goto err_proc;
	}
//	udelay(150);

	read_x = rohm_bu21025gul_read_data(client);

	err = rohm_bu21025gul_write_reg(client, (Y_SET << 4) | 0x00);
	if (err) {
		dev_err(&client->dev, "work error\n");
		goto err_proc;
	}

//	msleep(1);
//	udelay(600);
	udelay(800);

	err = rohm_bu21025gul_write_reg(client, (Y_MEASURE << 4) | PD_OFF);
	if (err) {
		dev_err(&client->dev, "work error\n");
		goto err_proc;
	}
//	udelay(150);

	read_y = rohm_bu21025gul_read_data(client);

	err = rohm_bu21025gul_write_reg(client, (Z_SET << 4) | 0x00);
	if (err) {
		dev_err(&client->dev, "work error\n");
		goto err_proc;
	}

//	msleep(1);
	udelay(600);

	err = rohm_bu21025gul_write_reg(client, (Z1_MEASURE << 4) | PD_OFF);
	if (err) {
		dev_err(&client->dev, "work error\n");
		goto err_proc;
	}
//	udelay(150);

	read_z1 = rohm_bu21025gul_read_data(client);

#ifdef DEBUG2
printk("before_change: x %d,y %d,z1 %d\n",read_x,read_y,read_z1);
#endif

//異物混入対応-s
	if( first_touch_flag == 0 )
	{
		touch_info_set(read_x, read_y);
	}
//異物混入対応-e

	if( read_z1 == 0 )
	{
		touch_value = 0;
	}
	else
	{
		touch_value = ((RX_VALUE * read_x) / 4096) * (((4096 * 1000) / read_z1) - 1000) - ((1000 - ((read_y * 1000) / 4096)) * RY_VALUE);
	}
#ifdef DEBUG2
printk("touch_value %d,\n",touch_value);
#endif

	if( back_z1 == -1 && read_z1 != 0 )
	{
		touch = 2;
	}
	else if( read_z1 == 0 )
	{
		touch = 0;
	}
	else if( abs( back_z1 - read_z1 ) > Z1_DIFF_MAX )
	{
		touch = 2;
	}
//	else if( touch_value < 130000 || touch_value > 1300000 )
//	else if( touch_value > 900000 )
	else if( touch_value > 1300000 )
	{
#ifdef DEBUG2
printk("touch_value::NG %d\n",touch_value);
#endif
		touch = 2;
	}
	else
	{
		touch = 1;
	}

	if( read_z1 == 0 )
	{
		back_z1 = -1;
	}
	else
	{
		back_z1 = read_z1;
	}

	send_x = keep_x;
	send_y = keep_y;
	send_z1 = keep_z1;
	if( touch == 1 )
	{
		keep_x = read_x;
		keep_y = read_y;
		keep_z1 = read_z1;
	}
	else
	{
		keep_x = -1;
		keep_y = -1;
		keep_z1 = -1;
	}

#ifdef DEBUG2
printk("keep_x %d,keep_y %d,keep_z1 %d,send_x %d,send_y %d,send_z1 %d\n",keep_x,keep_y,keep_z1,send_x,send_y,send_z1);
#endif

	if((touch == 0) || (read_z1 == 0) || (read_z1 == 4095))
	{
		if( send_cnt == 0 && send_z1 > READ_TH )
		{
			read_x = send_x;
			read_y = send_y;
			calibration_pointer(&read_x, &read_y);
#ifdef DEBUG2
printk("release_one-send x %d,y %d,z1 %d\n",read_x,read_y,read_z1);
#endif
			input_report_abs(data->input_dev, ABS_X, read_x);
			input_report_abs(data->input_dev, ABS_Y, read_y);
			input_report_abs(data->input_dev, ABS_PRESSURE, 1);
			input_event(data->input_dev, EV_KEY, BTN_TOUCH, 1);
			input_sync(data->input_dev);
		}

#ifdef DEBUG
		printk("release \n");
#endif
#ifdef DEBUG2
printk("ozaki-test:release  x %d,y %d,z1 %d, back_z1 %d\n",read_x,read_y,read_z1,back_z1);
#endif
		send_cnt = 0;
		
		input_report_abs(data->input_dev, ABS_PRESSURE, 0);
		input_event(data->input_dev, EV_KEY, BTN_TOUCH, 0);
		input_sync(data->input_dev);

		err = rohm_bu21025gul_write_reg(client, (POWER_SETTING << 4) | 0x00);
		if (err) {
			dev_err(&client->dev, "work error\n");
			goto err_proc;
		}
		touch_info_clear();		//異物混入対応
		
		msleep(1);/*これがないとペンアップ時に割り込みが入る*/
		data->int_enable = true;
		enable_irq(client->irq);
	}
	else if( touch == 1 )
	{
		read_x = send_x;
		read_y = send_y;
		read_z1 = send_z1;

#ifdef DEBUG
		printk("x %d,y %d,z1 %d\n",read_x,read_y,read_z1);
#endif
#ifdef DEBUG2
		printk("x %d,y %d,z1 %d,   send_cnt %u\n",read_x,read_y,read_z1,send_cnt);
#endif
		/*z1の値の低いものはここではじく*/
		if(read_z1 > READ_TH)
		{
			send_cnt++;
			
			calibration_pointer(&read_x, &read_y);
#ifdef DEBUG2
		printk("x %d,y %d,z1 %d\n",read_x,read_y,read_z1);
#endif
			input_report_abs(data->input_dev, ABS_X, read_x);
			input_report_abs(data->input_dev, ABS_Y, read_y);
			input_report_abs(data->input_dev, ABS_PRESSURE, 1);
			input_event(data->input_dev, EV_KEY, BTN_TOUCH, 1);
			input_sync(data->input_dev);
		}
		hrtimer_start(&data->timer, ktime_set(0, 10000000), HRTIMER_MODE_REL);
	}
	else
	{
#ifdef DEBUG2
printk("ozaki-test: touch %d,x %d,y %d,z1 %d,back_z1 %d\n",touch,read_x,read_y,read_z1,back_z1);
#endif
		hrtimer_start(&data->timer, ktime_set(0, 10000000), HRTIMER_MODE_REL);
	}

	rohm_bu21025gul_unlock(data);

	return;

err_proc:
	touch_info_clear();		//異物混入対応
	
	rohm_bu21025gul_write_reg(client, (RESET << 4) | 0x00);/*reset*/
	rohm_bu21025gul_unlock(data);
	data->int_enable = true;
	enable_irq(client->irq);

	return;
}

static int rohm_bu21025gul_chip_init(struct i2c_client *client)
{
	int error = 0;

	error = rohm_bu21025gul_write_reg(client, (RESET << 4) | 0x00);
	if (error) {
		dev_err(&client->dev, "Failed to init RESET\n");
		return error;
	}
	error = rohm_bu21025gul_write_reg(client, (SETUP << 4) | 0x00);
	if (error) {
		dev_err(&client->dev, "Failed to init SETUP\n");
	}

	return error;
}

static void bu21025_early_suspend(struct early_suspend *h);
static void bu21025_later_resume(struct early_suspend *h);

//異物混入対応-s
ssize_t ts_info_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	return sprintf(buf, "%d %ld%09ld %d %d\n", t_info.flag, t_info.uptime.tv_sec, t_info.uptime.tv_nsec, t_info.x_val, t_info.y_val );
}

ssize_t ts_info_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	return count;
}

DEVICE_ATTR(ts_info, 0644, ts_info_show, ts_info_store);

static struct attribute *tsinfo_attributes[] = {
	&dev_attr_ts_info.attr,
	NULL,
};

static struct attribute_group tsinfo_attr_group = {
	.attrs = tsinfo_attributes,
};
//異物混入対応-e

static int __devinit rohm_bu21025gul_probe(struct i2c_client *client,
				       const struct i2c_device_id *id)
{
	struct rohm_bu21025gul_data *data;
	struct input_dev *input_dev;
	int error;

	data = kzalloc(sizeof(struct rohm_bu21025gul_data), GFP_KERNEL);
	input_dev = input_allocate_device();
	if (!data || !input_dev) {
		dev_err(&client->dev, "Failed to allocate memory\n");
		error = -ENOMEM;
		goto err_free_mem;
	}
	data->client = client;
	data->input_dev = input_dev;
	INIT_WORK(&data->work, rohm_bu21025gul_work);
	data->touch_stopped = false;

	memset(calibration, 0, sizeof(calibration));

	touch_info_clear();		//異物混入対応

	mutex_init(&data->lock);

	input_dev->name = "rohm_bu21025";
	input_dev->id.bustype = BUS_I2C;
	input_dev->dev.parent = &client->dev;

	input_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);
	input_dev->evbit[0] = (BIT_MASK(EV_SYN) | BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS));

	input_set_abs_params(input_dev, ABS_X, 0, X_CALIB_MAX, 0, 0);
	input_set_abs_params(input_dev, ABS_Y, 0, Y_CALIB_MAX, 0, 0);
	input_set_drvdata(input_dev, data);

	wq = create_singlethread_workqueue("rohm_bu21025gul_wq");
	//timer init
	hrtimer_init(&data->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	data->timer.function = rohm_bu21025gul_timer_func;

	//init
	rohm_bu21025gul_data = data;

	error = rohm_bu21025gul_chip_init(client);
	if (error) {
		goto err_free_mem;
	}

	error = request_threaded_irq(client->irq, NULL, rohm_bu21025gul_interrupt,
				     IRQF_TRIGGER_LOW | IRQF_ONESHOT,
				     "rohm_bu21025gul", data);
	if (error) {
		dev_err(&client->dev, "Failed to register interrupt\n");
		goto err_free_mem;
	}
	data->int_enable = true;

	error = input_register_device(data->input_dev);
	if (error)
		goto err_free_irq;

	i2c_set_clientdata(client, data);

#ifdef CONFIG_EARLYSUSPEND
	data->es_handler.level = EARLY_SUSPEND_LEVEL_DISABLE_FB;
	data->es_handler.suspend = bu21025_early_suspend;
	data->es_handler.resume = bu21025_later_resume;
	data->es_handler.data = (void *)client;
	register_early_suspend(&data->es_handler);
#endif

//異物混入対応-s
	error = sysfs_create_group(&client->dev.kobj, &tsinfo_attr_group);
	if (error)
		goto err_free_irq;
//異物混入対応-e

	return 0;

err_free_irq:
	free_irq(client->irq, data);
err_free_mem:
	input_free_device(input_dev);
	kfree(data);
	return error;
}

static __devexit int rohm_bu21025gul_remove(struct i2c_client *client)
{
	struct rohm_bu21025gul_data *data = i2c_get_clientdata(client);

#ifdef CONFIG_EARLYSUSPEND
	unregister_early_suspend(&data->es_handler);
#endif

	free_irq(client->irq, data);

	if (wq)
		destroy_workqueue(wq);

	hrtimer_cancel(&data->timer);
	mutex_destroy(&data->lock);

	input_unregister_device(data->input_dev);
	kfree(data);

	return 0;
}

static long rohm_bu21025gul_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int 		ret = 0;
	int		*ibuf = (int *)arg;
	int i;
	
#ifdef SHOW_REG
	printk("rohm_ts_command() Call\n");
#endif
	switch( cmd ) {

		case CMD_PRM_READ:
#ifdef SHOW_REG
				printk("Commnd:Parm Read\n");
#endif
			for( i=0;  i<7; i++) {
				ibuf[i] = calibration[i];
			}
			break;

		case CMD_PRM_WRITE:
#ifdef SHOW_REG
				printk("Commnd:Parm Write\n");
#endif
				for( i=0;  i<7; i++) {
					calibration[i] = ibuf[i];
				}
#ifdef SHOW_REG
				for( i=0;  i<7; i++) {
					printk("Commnd:Parm Write i:%d param src: %d dis:%d\n",i,ibuf[i],calibration[i] );
				}
#endif

			break;
		default:
			ret = -EIO;				//
			break;
	}

#ifdef SHOW_REG
	printk("rohm_ts_command() Call End Return Status:%d\n",ret);
#endif
	return ret;
}
static const struct file_operations ts_fops = {
	.owner			= THIS_MODULE,
	.unlocked_ioctl	= rohm_bu21025gul_ioctl,
};
static struct class *ts_dev_class;

#ifdef CONFIG_PM

#ifdef CONFIG_MACH_MX6Q_BEE01
extern void imx6q_bee01_ts_pwr_on(int on);
#endif

/**
 * bu21025_suspend() - suspend the touch screen controller
 * @dev: pointer to device structure
 *
 * This function is used to suspend the
 * touch panel controller and returns integer
 */
static int bu21025_suspend(struct device *dev)
{
#ifdef CONFIG_MACH_MX6Q_BEE01
	imx6q_bee01_ts_pwr_on(0);
#endif


	return 0;
}

/**
 * bu21025_resume() - resume the touch screen controller
 * @dev: pointer to device structure
 *
 * This function is used to resume the touch panel
 * controller and returns integer.
 */
static int bu21025_resume(struct device *dev)
{
#ifdef CONFIG_MACH_MX6Q_BEE01
	imx6q_bee01_ts_pwr_on(1);
	mdelay(5);
#endif
	return 0;
}

static const struct dev_pm_ops bu21025_dev_pm_ops = {
	.suspend = bu21025_suspend,
	.resume  = bu21025_resume,
};
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
static void bu21025_early_suspend(struct early_suspend *h)
{
	struct i2c_client *client = rohm_bu21025gul_data->client;
	struct workqueue_struct *wq_bak;

	rohm_bu21025gul_data->touch_stopped = true;
	wq_bak = wq;
	wq = 0;
	destroy_workqueue(wq_bak);
	rohm_bu21025gul_lock(rohm_bu21025gul_data);
	hrtimer_cancel(&rohm_bu21025gul_data->timer);
	if(rohm_bu21025gul_data->int_enable == true)
	{
		rohm_bu21025gul_data->int_enable = false;
		disable_irq_nosync(client->irq);
	}
	
	touch_info_clear();		//異物混入対応
	
	rohm_bu21025gul_unlock(rohm_bu21025gul_data);
}

static void bu21025_later_resume(struct early_suspend *h)
{
	struct i2c_client *client = rohm_bu21025gul_data->client;

	rohm_bu21025gul_lock(rohm_bu21025gul_data);
	
	touch_info_clear();		//異物混入対応
	
	rohm_bu21025gul_chip_init(client);
	if(wq == 0)
	{
		wq = create_singlethread_workqueue("rohm_bu21025gul_wq");
	}
	rohm_bu21025gul_data->touch_stopped = false;
	rohm_bu21025gul_data->int_enable = true;
	enable_irq(client->irq);
	rohm_bu21025gul_unlock(rohm_bu21025gul_data);
}
#else
static void bu21025_early_suspend(struct early_suspend *h)
{
}
static void bu21025_later_resume(struct early_suspend *h)
{
}
#endif

static const struct i2c_device_id rohm_bu21025gul_id[] = {
	{"bu21025", 0},
	{ }
};
MODULE_DEVICE_TABLE(i2c, rohm_bu21025gul_id);

static struct i2c_driver rohm_bu21025gul_driver = {
	.driver = {
		.name	= "bu21025",
		.owner	= THIS_MODULE,
#ifdef CONFIG_PM
		.pm	=	&bu21025_dev_pm_ops,
#endif
	},
	.id_table	= rohm_bu21025gul_id,
	.probe		= rohm_bu21025gul_probe,
	.remove		= __devexit_p(rohm_bu21025gul_remove),
};

static int __init rohm_bu21025gul_init(void)
{
#if 0
	return i2c_add_driver(&rohm_bu21025gul_driver);
#else
#ifdef SHOW_DEBUG
	printk("ROHM BU21025BU rohm_bu21025gul_init \n");
#endif
        int res;

	TS_MAJOR = register_chrdev(0, TS_NAME, &ts_fops);
	if (TS_MAJOR<0) {
		printk(KERN_ERR "%s: Driver Initialisation failed\n", __FILE__);
		res = TS_MAJOR;
		goto out;
	}

	printk("/dev/ts  MAJOR:%d\n",TS_MAJOR);

	ts_dev_class = class_create(THIS_MODULE, "ts-dev");
	if (IS_ERR(ts_dev_class)) {
		res = PTR_ERR(ts_dev_class);
		goto out_unreg_chrdev;
	}

	/* register this i2c device with the driver core */
	if( device_create(ts_dev_class, NULL,
					 MKDEV(TS_MAJOR, 0), NULL, "ts") == NULL ) {
		res = -1;
		goto out_unreg_class; ;
	}

	if( (res = i2c_add_driver(&rohm_bu21025gul_driver)) < 0 ) goto out_destroy; 

	printk("ROHM BU21025BU rohm_bu21025gul_init Success \n");
	return res;

out_destroy:
	device_destroy(ts_dev_class, MKDEV(TS_MAJOR, 0));

out_unreg_class:
	class_destroy(ts_dev_class);
out_unreg_chrdev:
	unregister_chrdev(TS_MAJOR, TS_NAME);
out:
	printk("ROHM BU21025BU rohm_bu21025gul_init Fail \n");
	return res;
#endif
}

static void __exit rohm_bu21025gul_exit(void)
{
	i2c_del_driver(&rohm_bu21025gul_driver);
	class_destroy(ts_dev_class);
	unregister_chrdev(TS_MAJOR, TS_NAME);
}

module_init(rohm_bu21025gul_init);
module_exit(rohm_bu21025gul_exit);

MODULE_AUTHOR("STST");
MODULE_DESCRIPTION("Touchscreen driver for ROHM BU21025GUL controller");
MODULE_LICENSE("GPL");
