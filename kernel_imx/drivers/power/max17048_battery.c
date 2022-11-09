/*
 *  max17048_battery.c
 *  ModelGauge IC for 1 cell Li+ battery
 *
 *  Copyright (C) 2013 Sanyo Ltd..
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/power_supply.h>
#include <linux/slab.h>
#include <linux/interrupt.h>

//#define DEBUG
//#define DEBUG2

#define MAX17048_VCELL_REG	0x02
#define MAX17048_SOC_REG	0x04
#define MAX17048_MODE_REG	0x06
#define MAX17048_VERSION_REG	0x08
#define MAX17048_CONFIG_REG	0x0C
#define MAX17048_OCV_REG	0x0E
#define MAX17048_STATUS_REG	0x1A
#define MAX17048_MODEL_REG	0x40

#define MAX17048_DELAY		1000

#define TEMP_DATA_MAX 166
static int temp_data[TEMP_DATA_MAX][2] =
{
	{-40,2795},
	{-39,2787},
	{-38,2779},
	{-37,2771},
	{-36,2762},
	{-35,2753},
	{-34,2744},
	{-33,2734},
	{-32,2723},
	{-31,2713},
	{-30,2702},
	{-29,2690},
	{-28,2678},
	{-27,2666},
	{-26,2653},
	{-25,2640},
	{-24,2626},
	{-23,2612},
	{-22,2597},
	{-21,2582},
	{-20,2567},
	{-19,2551},
	{-18,2534},
	{-17,2517},
	{-16,2500},
	{-15,2482},
	{-14,2464},
	{-13,2445},
	{-12,2426},
	{-11,2406},
	{-10,2386},
	{ -9,2365},
	{ -8,2344},
	{ -7,2322},
	{ -6,2300},
	{ -5,2277},
	{ -4,2254},
	{ -3,2231},
	{ -2,2207},
	{ -1,2183},
	{  0,2159},
	{  1,2134},
	{  2,2109},
	{  3,2084},
	{  4,2058},
	{  5,2032},
	{  6,2006},
	{  7,1979},
	{  8,1953},
	{  9,1926},
	{ 10,1899},
	{ 11,1871},
	{ 12,1844},
	{ 13,1816},
	{ 14,1789},
	{ 15,1761},
	{ 16,1733},
	{ 17,1705},
	{ 18,1677},
	{ 19,1649},
	{ 20,1622},
	{ 21,1594},
	{ 22,1566},
	{ 23,1539},
	{ 24,1511},
	{ 25,1484},
	{ 26,1456},
	{ 27,1429},
	{ 28,1402},
	{ 29,1375},
	{ 30,1348},
	{ 31,1322},
	{ 32,1296},
	{ 33,1270},
	{ 34,1244},
	{ 35,1219},
	{ 36,1194},
	{ 37,1169},
	{ 38,1144},
	{ 39,1120},
	{ 40,1096},
	{ 41,1073},
	{ 42,1049},
	{ 43,1027},
	{ 44,1004},
	{ 45,982 },
	{ 46,960 },
	{ 47,938 },
	{ 48,917 },
	{ 49,896 },
	{ 50,876 },
	{ 51,856 },
	{ 52,836 },
	{ 53,817 },
	{ 54,798 },
	{ 55,779 },
	{ 56,761 },
	{ 57,743 },
	{ 58,725 },
	{ 59,708 },
	{ 60,691 },
	{ 61,675 },
	{ 62,659 },
	{ 63,644 },
	{ 64,628 },
	{ 65,614 },
	{ 66,599 },
	{ 67,585 },
	{ 68,571 },
	{ 69,558 },
	{ 70,544 },
	{ 71,531 },
	{ 72,519 },
	{ 73,506 },
	{ 74,494 },
	{ 75,482 },
	{ 76,471 },
	{ 77,460 },
	{ 78,449 },
	{ 79,438 },
	{ 80,428 },
	{ 81,417 },
	{ 82,408 },
	{ 83,398 },
	{ 84,389 },
	{ 85,379 },
	{ 86,370 },
	{ 87,362 },
	{ 88,353 },
	{ 89,345 },
	{ 90,337 },
	{ 91,329 },
	{ 92,321 },
	{ 93,314 },
	{ 94,306 },
	{ 95,299 },
	{ 96,292 },
	{ 97,285 },
	{ 98,278 },
	{ 99,272 },
	{100,266 },
	{101,260 },
	{102,254 },
	{103,248 },
	{104,242 },
	{105,237 },
	{106,231 },
	{107,226 },
	{108,221 },
	{109,216 },
	{110,211 },
	{111,206 },
	{112,202 },
	{113,197 },
	{114,193 },
	{115,189 },
	{116,184 },
	{117,180 },
	{118,176 },
	{119,172 },
	{120,169 },
	{121,165 },
	{122,161 },
	{123,158 },
	{124,154 },
	{125,151 },
};

struct max17048_chip {
	struct i2c_client		*client;
	struct delayed_work		work;
	int vcell;
	int soc;
	u16 ocv;
	int init;
	int temp;
	int fail;
};

static struct max17048_chip chip_st;

static u8 model_data[64] =
{
	0xAB,0x80,0xB7,0xA0,0xB8,0xB0,0xB9,0xC0,
	0xBA,0xD0,0xBB,0xE0,0xBC,0xC0,0xBD,0xA0,
	0xBE,0x80,0xBF,0x70,0xC1,0xA0,0xC3,0xE0,
	0xC8,0x60,0xCA,0xB0,0xD0,0x60,0xD1,0xB0,
	0x00,0x80,0x34,0x00,0x34,0x20,0x34,0x00,
	0x34,0x20,0x3E,0x40,0x3E,0x60,0x3E,0x40,
	0x3A,0x20,0x14,0x40,0x13,0xC0,0x13,0xC0,
	0x13,0x40,0x17,0x40,0x1E,0xC0,0x1E,0xC0
};

//#define ADJUST_SOC

#ifdef ADJUST_SOC
static int deltaV_cnt=0;
static int arrayDeltaV[5];
static int TempSOC=0;
static int maxDeltaV=0;
static int adjust_flg=0;
#endif

#define OCVtest 54960
#define SOCCheckA 247
#define SOCCheckB 249
#define RCOMP0 204
#define TempCoUp -25
#define TempCoDown -275
#ifdef ADJUST_SOC
#define ALART_SETTING (0x1e | 0x40) /*1% 19bit*/
#else
#define ALART_SETTING (0x1e) /*1% 19bit*/
#endif
#define ALART_INT (1 << 5)
#ifdef ADJUST_SOC
#define KEMPTY (15)
#endif
extern void bee01_max8903_chg_enable(int en);
extern void do_switch_loadmodel_operation(int value);
extern int get_loadmodel_operation_flg(void);
extern int imx6q_bee01_AC_status(void);

#ifdef ADJUST_SOC
static int getMaxDeltaV(void)
{
	int i;
	int max=0;
	
	for(i=0;i<5;i++){
		if(max < arrayDeltaV[i])
			max = arrayDeltaV[i];
	}
	
	return max;
}
#endif

static int max17048_write_buf_data(struct i2c_client *client, u8 reg, u8 *buf,int size)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct i2c_msg i2cmsg;
	int ret = 0;
	u8 data_buf[65];

	if(size > 64)
	{
		return -1;
	}

	data_buf[0] = reg;
	memcpy(&data_buf[1] ,buf ,size);

	i2cmsg.addr  = client->addr;
	i2cmsg.len   = size + 1;
	i2cmsg.buf   = data_buf;
	i2cmsg.flags = 0;

	/* Start the i2c transfer by calling host i2c driver function */
	ret = i2c_transfer(adapter, &i2cmsg, 1);
	if (ret < 0) {
		dev_info(&client->dev,
			 "%s: i2c_transfer failed!!!\n", __func__);
		return ret;
	}

	return 0;
}

static int max17048_write_reg(struct i2c_client *client, u8 reg, u16 value)
{
	s32 ret;
	u8 buf[2];

	buf[0] = value >> 8;
	buf[1] = value & 0x00ff;

#ifdef DEBUG
	printk("write reg %02x,data %02x%02x\n",reg,buf[0],buf[1]);
#endif
	ret = i2c_smbus_write_i2c_block_data(client, reg, 2 ,buf);

	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	return ret;
}

static int max17048_read_reg(struct i2c_client *client, u8 reg , u16 *value)
{
	s32 ret;
	u8 buf[2];

	ret = i2c_smbus_read_i2c_block_data(client, reg, 2, buf);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	*value = ((u16)buf[0] << 8) | buf[1];

	return ret;
}

int max17048_get_vcell_external(void)
{
	u16 reg;
	int ret;

	if(chip_st.fail == 1)
	{
		return 4000;
	}
	
	if(chip_st.init == 0)
	{
		return 4000;
	}

	ret = max17048_read_reg(chip_st.client, MAX17048_VCELL_REG, &reg);
	if(ret < 0)
	{
		chip_st.init = 0;
		return 4000;
	}

	chip_st.vcell = (u32)reg * 78125 / 1000 / 1000;

#ifdef DEBUG2
	printk("vcell %04x %dmv\n",reg,chip_st.vcell);
#endif

	return chip_st.vcell;
}

int max17048_get_soc_external(void)
{
	u16 reg;
	int ret;
#ifdef ADJUST_SOC
	int adjust_soc;
#endif

	if(chip_st.fail == 1)
	{
		return 0;
	}

	if(chip_st.init == 0)
	{
		return 100;
	}

	ret = max17048_read_reg(chip_st.client, MAX17048_SOC_REG, &reg);
	if(ret < 0)
	{
		chip_st.init = 0;
		return 100;
	}

	chip_st.soc = reg / 512;

#ifdef DEBUG2
	printk("soc %04x %d%%\n",reg ,chip_st.soc);
#endif

#ifdef ADJUST_SOC
	adjust_soc = chip_st.soc - (100 - chip_st.soc) * maxDeltaV * KEMPTY / 100000;
	if(adjust_soc < 0)
		adjust_soc = 0;
	if(1 == imx6q_bee01_AC_status())
		adjust_soc = chip_st.soc;
#ifdef DEBUG2
	printk("adjust soc %d%% ## maxDeltaV %d\n",adjust_soc,maxDeltaV);
#endif
	
	return adjust_soc;
#else
	return chip_st.soc;
#endif
}

int max17048_get_temp_data(u16 data)
{
	int i;

	for(i = 0 ; i < TEMP_DATA_MAX; i++)
	{
		if((int)data >= temp_data[i][1])
		{
			return temp_data[i][0];
		}
	}

	return temp_data[TEMP_DATA_MAX -1][0];
}

static void rcomp_update(struct i2c_client *client,int temp)
{
	int rcomp,ret;

	if(chip_st.init == 0)
	{
		return;
	}

	if(temp > 20)
	{
		rcomp = RCOMP0 + ((temp - 20) * TempCoUp / 1000);
	}
	else if(temp < 20)
	{
		rcomp = RCOMP0 + ((temp - 20) * TempCoDown / 1000);
	}
	else
	{
		rcomp = RCOMP0;
	}

	if(rcomp > 0xff)
	{
		rcomp = 0xff;
	}
	else if(rcomp < 0)
	{
		rcomp = 0;
	}

	ret = max17048_write_reg(client, MAX17048_CONFIG_REG, rcomp << 8 | ALART_SETTING);
	if(ret < 0)
	{
		chip_st.init = 0;
	}
}

extern u16 rohm_bu21025gul_read_adc(void);
extern int mx6_bee01_get_PCB_version(void);

static void max17048_work(struct work_struct *work)
{
	struct max17048_chip *chip;
	u16 data;
	int temp;

	chip = container_of(work, struct max17048_chip, work.work);

	if(mx6_bee01_get_PCB_version())
	{
		data = rohm_bu21025gul_read_adc();
	}
	else
	{
		data = 1622;//20
	}
	temp = max17048_get_temp_data(data);
#ifdef DEBUG2
	printk("temp = %d\n",temp);
#endif
	if(chip->temp != temp)
	{
		rcomp_update(chip->client,temp);
		chip->temp = temp;
	}

	schedule_delayed_work(&chip->work, MAX17048_DELAY);
}

int max17048_get_temp_external(void)
{
	return chip_st.temp;
}

static irqreturn_t max17048_irq_thread(int irq, void *data)
{
	struct max17048_chip *chip = data;
	u16 reg;
#ifdef ADJUST_SOC
	int RepSOC;
	int ocv;
	int vcell;
#endif

#ifdef DEBUG
	max17048_read_reg(chip->client, MAX17048_CONFIG_REG, &reg);
	printk("MAX17048_CONFIG_REG %04x\n",reg);
#endif

#ifdef DEBUG
	max17048_read_reg(chip->client, MAX17048_STATUS_REG, &reg);
	printk("status %04x\n",reg);
#endif

#ifdef ADJUST_SOC
	if(reg & 0x2000){
		max17048_read_reg(chip_st.client, MAX17048_SOC_REG, &reg);
		RepSOC = reg / 512;
		
		if(TempSOC != RepSOC){
			TempSOC = RepSOC;
			
			/* Unlock Model Access */
			max17048_write_reg(chip_st.client, 0x3E,0x4a57);
			/* Read OCV */
			max17048_read_reg(chip_st.client, MAX17048_OCV_REG,&reg);
			
			max17048_write_reg(chip_st.client, 0x3E, 0x0000); //Send lock command
			
			ocv = (u32)reg * 78125 / 1000 / 1000;
#ifdef DEBUG2
			printk("max17048 ocv   %04x -> %dmv\n",reg,ocv);
#endif
			max17048_read_reg(chip_st.client, MAX17048_VCELL_REG, &reg);
			
			vcell = (u32)reg * 78125 / 1000 / 1000;
#ifdef DEBUG2
			printk("max17048 vcell %04x -> %dmv\n",reg,vcell);
#endif
			arrayDeltaV[deltaV_cnt] = ocv - vcell;
			deltaV_cnt++;
			if(deltaV_cnt == 5){
				deltaV_cnt = 0;
				adjust_flg = 1;
			}
			
			if(adjust_flg)
				maxDeltaV = getMaxDeltaV();
		}
	}
#endif
	
	max17048_write_reg(chip->client, MAX17048_STATUS_REG, 0x00ff); //Reset Status

	return IRQ_RETVAL(0);
}

static ssize_t loadmodel_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{

	struct i2c_adapter *adapter = to_i2c_adapter(chip_st.client->dev.parent);
	u16 test_ocv,test_soc;
	int ret;
	u16 reg;
	
	chip_st.init = 0;

	if(&chip_st.work)
	{
		cancel_delayed_work(&chip_st.work);
	}

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_I2C_BLOCK))
		return count;
	
	i2c_set_clientdata(chip_st.client, &chip_st);
	
	ret = max17048_read_reg(chip_st.client, MAX17048_VERSION_REG ,&reg);
	if(ret < 0)
	{
		return count;
	}
	printk("max17048 version %04x\n",reg);

	ret = get_loadmodel_operation_flg();
	if(ret){
		max17048_write_reg(chip_st.client, 0xFE, 0x5400); //Send POR command
		msleep(100);
	}
#ifdef DEBUG
	max17048_get_soc(chip_st.client);

	ret = max17048_read_reg(chip_st.client, MAX17048_OCV_REG,&(chip_st.ocv));
	if(ret < 0)
	{
		return count;
	}
	printk("max17048 ocv %04x\n",chip_st.ocv);
#endif
	/* Unlock Model Access */
	ret = max17048_write_reg(chip_st.client, 0x3E,0x4a57);
	/* Read OCV */
	ret = max17048_read_reg(chip_st.client, MAX17048_OCV_REG,&(chip_st.ocv));
	if(ret < 0)
	{
		return count;
	}
#ifdef DEBUG
	printk("max17048 ocv %04x\n",chip_st.ocv);
#endif

	msleep(150);
	
	do_switch_loadmodel_operation(1);
	
	/* Write OCV */
	ret = max17048_write_reg(chip_st.client, MAX17048_OCV_REG,OCVtest);
	if(ret < 0)
	{
		do_switch_loadmodel_operation(0);
		return count;
	}
	ret = max17048_write_reg(chip_st.client, 0x3E, 0x0000); //Send lock command
	if(ret < 0)
	{
		do_switch_loadmodel_operation(0);
		return count;
	}

	msleep(150);

	ret = max17048_read_reg(chip_st.client, MAX17048_SOC_REG,&test_soc);
	if(ret < 0)
	{
		do_switch_loadmodel_operation(0);
		return count;
	}

#ifdef DEBUG
	printk("test_soc %04x\n",test_soc);
#endif
	if((test_soc >> 8) >= SOCCheckA && (test_soc >> 8) <= SOCCheckB)
	{
		printk("model was loaded\n");
		chip_st.init = 1;
		
		ret = max17048_write_reg(chip_st.client, 0x3E, 0x4a57); //Send Unlock command
		/* Write OCV */
		ret = max17048_write_reg(chip_st.client, MAX17048_OCV_REG,chip_st.ocv);
		
		max17048_write_reg(chip_st.client, MAX17048_STATUS_REG, 0x0000); //Reset Status
		
		ret = max17048_write_reg(chip_st.client, MAX17048_CONFIG_REG, RCOMP0 << 8 | ALART_SETTING);
		
		ret = max17048_write_reg(chip_st.client, 0x3E, 0x0000); //Send lock command
		
		msleep(150);
	}
	else
	{
		printk("model was NOT loaded\n");
		
		if(0 == imx6q_bee01_AC_status()){
			chip_st.fail = 1;
			do_switch_loadmodel_operation(0);
			return ret;
		}
		else
			chip_st.fail = 0;

		bee01_max8903_chg_enable(0);
		chip_st.init = 1;

#ifdef DEBUG2
		printk("vcell_before %d\n",max17048_get_vcell_external());
#endif
		msleep(2000);
#ifdef DEBUG2
		printk("vcell_after %d\n",max17048_get_vcell_external());
#endif
		do
		{
			max17048_write_reg(chip_st.client, 0xFE, 0x5400); //Send POR command
			msleep(100);
			ret = max17048_write_reg(chip_st.client, 0x3E, 0x4a57); //Send Unlock command
			if(ret < 0)
			{
				chip_st.init = 0;
				do_switch_loadmodel_operation(0);
				return count;
			}
			ret = max17048_read_reg(chip_st.client, MAX17048_OCV_REG,&test_ocv);
			if(ret < 0)
			{
				chip_st.init = 0;
				do_switch_loadmodel_operation(0);
				return count;
			}
		} while(test_ocv == 0xffff);

#ifdef DEBUG
		printk("max17048 test_ocv %04x\n",test_ocv);
#endif

		/* Write the Model */
		ret = max17048_write_buf_data(chip_st.client,MAX17048_MODEL_REG , model_data ,sizeof(model_data));

		msleep(150);

		/* Write OCV */
		ret = max17048_write_reg(chip_st.client, MAX17048_OCV_REG,OCVtest);
		ret = max17048_write_reg(chip_st.client, 0x3E, 0x0000); //Send lock command

		msleep(150);

		ret = max17048_read_reg(chip_st.client, MAX17048_SOC_REG,&test_soc);
		if(ret < 0)
		{
			chip_st.init = 0;
			do_switch_loadmodel_operation(0);
			return count;
		}

#ifdef DEBUG
		printk("test_soc %04x\n",test_soc);
#endif
		if((test_soc >> 8) >= SOCCheckA && (test_soc >> 8) <= SOCCheckB)
		{
			if(0 == imx6q_bee01_AC_status()){
				printk("model was NOT loaded successfully\n");
				chip_st.fail = 1;
				return count;
			}
			else
				printk("model was loaded successfully\n");
		}
		else
		{
			printk("model was NOT loaded successfully\n");
			chip_st.fail = 1;
			return count;
		}
		ret = max17048_write_reg(chip_st.client, 0x3E, 0x4a57); //Send Unlock command
		/* Write OCV */
		ret = max17048_write_reg(chip_st.client, MAX17048_OCV_REG,test_ocv);
		
		max17048_write_reg(chip_st.client, MAX17048_STATUS_REG, 0x0000); //Reset Status
		
		ret = max17048_write_reg(chip_st.client, MAX17048_CONFIG_REG, RCOMP0 << 8 | ALART_SETTING);
		
		ret = max17048_write_reg(chip_st.client, 0x3E, 0x0000); //Send lock command
		
		msleep(150);
		bee01_max8903_chg_enable(1);
		chip_st.temp = max17048_get_temp_data(rohm_bu21025gul_read_adc());
		rcomp_update(chip_st.client,chip_st.temp);
	}
	
	do_switch_loadmodel_operation(0);

	if(&chip_st.work)
	{
		schedule_delayed_work(&chip_st.work, MAX17048_DELAY);
	}
	
	return count;
}

static struct kobj_attribute loadmodel_attribute = __ATTR(loadmodel, 0200, NULL, loadmodel_store);

static struct attribute *attrs[] = {
	&loadmodel_attribute.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = attrs,
};

static int __devinit max17048_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct max17048_chip *chip;
	u16 test_ocv,test_soc;
	int ret;
	u16 reg;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_I2C_BLOCK))
		return -EIO;

	chip = &chip_st;
	chip->client = client;
	chip->init = 0;

	i2c_set_clientdata(client, chip);

	ret = max17048_read_reg(client, MAX17048_VERSION_REG ,&reg);
	if(ret < 0)
	{
		return ret;
	}
	printk("max17048 version %04x\n",reg);

	ret = get_loadmodel_operation_flg();
	if(ret){
		max17048_write_reg(client, 0xFE, 0x5400); //Send POR command
		msleep(100);
	}
#ifdef DEBUG
	max17048_get_soc(client);

	ret = max17048_read_reg(client, MAX17048_OCV_REG,&(chip->ocv));
	if(ret < 0)
	{
		return ret;
	}
	printk("max17048 ocv %04x\n",chip->ocv);
#endif
	/* Unlock Model Access */
	ret = max17048_write_reg(client, 0x3E,0x4a57);
	if(ret < 0)
	{
		return ret;
	}
	/* Read OCV */
	ret = max17048_read_reg(client, MAX17048_OCV_REG,&(chip->ocv));
	if(ret < 0)
	{
		return ret;
	}
#ifdef DEBUG
	printk("max17048 ocv %04x\n",chip->ocv);
#endif

	msleep(150);
	
	do_switch_loadmodel_operation(1);

	/* Write OCV */
	ret = max17048_write_reg(client, MAX17048_OCV_REG,OCVtest);
	if(ret < 0)
	{
		do_switch_loadmodel_operation(0);
		return ret;
	}
	ret = max17048_write_reg(client, 0x3E, 0x0000); //Send lock command
	if(ret < 0)
	{
		do_switch_loadmodel_operation(0);
		return ret;
	}

	msleep(150);

	ret = max17048_read_reg(client, MAX17048_SOC_REG,&test_soc);
	if(ret < 0)
	{
		do_switch_loadmodel_operation(0);
		return ret;
	}

#ifdef DEBUG
	printk("test_soc %04x\n",test_soc);
#endif
	if((test_soc >> 8) >= SOCCheckA && (test_soc >> 8) <= SOCCheckB)
	{
		printk("model was loaded\n");
		chip->init = 1;
		
		ret = max17048_write_reg(client, 0x3E, 0x4a57); //Send Unlock command
		/* Write OCV */
		ret = max17048_write_reg(client, MAX17048_OCV_REG,chip->ocv);

		max17048_write_reg(client, MAX17048_STATUS_REG, 0x0000); //Reset Status

		ret = max17048_write_reg(client, MAX17048_CONFIG_REG, RCOMP0 << 8 | ALART_SETTING);

		ret = max17048_write_reg(client, 0x3E, 0x0000); //Send lock command

		msleep(150);
	}
	else
	{
		printk("model was NOT loaded\n");
		
		if(0 == imx6q_bee01_AC_status()){
			chip->fail = 1;
			do_switch_loadmodel_operation(0);
			return ret;
		}
		else
			chip->fail = 0;

		bee01_max8903_chg_enable(0);
		chip->init = 1;

#ifdef DEBUG
		printk("vcell_before %d\n",max17048_get_vcell_external());
#endif
		msleep(2000);
#ifdef DEBUG
		printk("vcell_after %d\n",max17048_get_vcell_external());
#endif
		do
		{
			max17048_write_reg(client, 0xFE, 0x5400); //Send POR command
			msleep(100);
			ret = max17048_write_reg(client, 0x3E, 0x4a57); //Send Unlock command
			if(ret < 0)
			{
				chip->init = 0;
				do_switch_loadmodel_operation(0);
				return ret;
			}
			ret = max17048_read_reg(client, MAX17048_OCV_REG,&test_ocv);
			if(ret < 0)
			{
				chip->init = 0;
				do_switch_loadmodel_operation(0);
				return ret;
			}
		} while(test_ocv == 0xffff);

#ifdef DEBUG
		printk("max17048 test_ocv %04x\n",test_ocv);
#endif

		/* Write the Model */
		ret = max17048_write_buf_data(client,MAX17048_MODEL_REG , model_data ,sizeof(model_data));

		msleep(150);

		/* Write OCV */
		ret = max17048_write_reg(client, MAX17048_OCV_REG,OCVtest);
		ret = max17048_write_reg(client, 0x3E, 0x0000); //Send lock command

		msleep(150);

		ret = max17048_read_reg(client, MAX17048_SOC_REG,&test_soc);
		if(ret < 0)
		{
			chip->init = 0;
			do_switch_loadmodel_operation(0);
			return ret;
		}

#ifdef DEBUG
		printk("test_soc %04x\n",test_soc);
#endif
		if((test_soc >> 8) >= SOCCheckA && (test_soc >> 8) <= SOCCheckB)
		{
			if(0 == imx6q_bee01_AC_status()){
				printk("model was NOT loaded successfully\n");
				chip->fail = 1;
				return ret;
			}
			else
				printk("model was loaded successfully\n");
		}
		else
		{
			printk("model was NOT loaded successfully\n");
			chip->fail = 1;
			return ret;
		}
		ret = max17048_write_reg(client, 0x3E, 0x4a57); //Send Unlock command
		/* Write OCV */
		ret = max17048_write_reg(client, MAX17048_OCV_REG,test_ocv);

		max17048_write_reg(client, MAX17048_STATUS_REG, 0x0000); //Reset Status

		ret = max17048_write_reg(client, MAX17048_CONFIG_REG, RCOMP0 << 8 | ALART_SETTING);

		ret = max17048_write_reg(client, 0x3E, 0x0000); //Send lock command

		msleep(150);
		bee01_max8903_chg_enable(1);
	}

	do_switch_loadmodel_operation(0);

	chip->temp = max17048_get_temp_data(rohm_bu21025gul_read_adc());
	rcomp_update(client,chip->temp);

	INIT_DELAYED_WORK_DEFERRABLE(&chip->work, max17048_work);
	schedule_delayed_work(&chip->work, MAX17048_DELAY);

	if(client->irq)
	{
		ret = request_threaded_irq(client->irq, NULL, max17048_irq_thread,
				     IRQF_TRIGGER_LOW | IRQF_ONESHOT,
				     "max17048", chip);
		if (ret) {
			dev_err(&client->dev, "Failed to register interrupt\n");
		}
	}

	ret = sysfs_create_group(&client->dev.kobj, &attr_group);
	if (ret)
		dev_err(&client->dev, "failed to create sysfs entries\n");

	return 0;
}

static int __devexit max17048_remove(struct i2c_client *client)
{
	struct max17048_chip *chip = i2c_get_clientdata(client);

	if(&chip->work)
	{
		cancel_delayed_work(&chip->work);
	}

	sysfs_remove_group(&client->dev.kobj, &attr_group);

	if(client->irq)
	{
		free_irq(client->irq, chip);
	}
	return 0;
}

#ifdef CONFIG_PM

static int max17048_suspend(struct i2c_client *client,
		pm_message_t state)
{
	struct max17048_chip *chip = i2c_get_clientdata(client);

	if(&chip->work)
	{
		cancel_delayed_work(&chip->work);
	}

	if(client->irq)
	{
		enable_irq_wake(client->irq);
	}

	return 0;
}

static int max17048_resume(struct i2c_client *client)
{
	struct max17048_chip *chip = i2c_get_clientdata(client);

	if(client->irq)
	{
		disable_irq_wake(client->irq);
	}

	if(&chip->work)
	{
		schedule_delayed_work(&chip->work, MAX17048_DELAY);
	}

	return 0;
}

#else

#define max17048_suspend NULL
#define max17048_resume NULL

#endif /* CONFIG_PM */

static const struct i2c_device_id max17048_id[] = {
	{ "max17048", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, max17048_id);

static struct i2c_driver max17048_i2c_driver = {
	.driver	= {
		.name	= "max17048",
	},
	.probe		= max17048_probe,
	.remove		= __devexit_p(max17048_remove),
	.suspend	= max17048_suspend,
	.resume		= max17048_resume,
	.id_table	= max17048_id,
};

static int __init max17048_init(void)
{
	return i2c_add_driver(&max17048_i2c_driver);
}
module_init(max17048_init);

static void __exit max17048_exit(void)
{
	i2c_del_driver(&max17048_i2c_driver);
}
module_exit(max17048_exit);

MODULE_AUTHOR("Sanyo Ltd..");
MODULE_DESCRIPTION("MAX17048 ModelGauge");
MODULE_LICENSE("GPL");
