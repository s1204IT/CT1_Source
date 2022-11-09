/*
 * Copyright (C) 2012 Freescale Semiconductor, Inc. All Rights Reserved.
 *
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

#include <linux/types.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/nodemask.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/fsl_devices.h>
#include <linux/spi/spi.h>
#include <linux/spi/flash.h>
#include <linux/i2c.h>
#include <linux/i2c/pca953x.h>
//#include <linux/ata.h>
//#include <linux/mtd/mtd.h>
//#include <linux/mtd/map.h>
//#include <linux/mtd/partitions.h>
#include <linux/regulator/consumer.h>
#include <linux/pmic_external.h>
#include <linux/pmic_status.h>
#include <linux/ipu.h>
#include <linux/mxcfb.h>
#include <linux/pwm_backlight.h>
#include <linux/fec.h>
#include <linux/memblock.h>
#include <linux/gpio.h>
#include <linux/ion.h>
#include <linux/etherdevice.h>
#include <linux/power/sabresd_battery.h>
#include <linux/regulator/anatop-regulator.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/fixed.h>
#include <linux/mfd/max17135.h>
#include <linux/mfd/wm8994/pdata.h>
#include <linux/mfd/wm8994/gpio.h>
#include <sound/wm8962.h>
#include <linux/mfd/mxc-hdmi-core.h>

#include <mach/common.h>
#include <mach/hardware.h>
#include <mach/mxc_dvfs.h>
#include <mach/memory.h>
#include <mach/iomux-mx6q.h>
#include <mach/imx-uart.h>
#include <mach/viv_gpu.h>
#include <mach/ahci_sata.h>
#include <mach/ipu-v3.h>
#include <mach/mxc_hdmi.h>
#include <mach/mxc_asrc.h>
#include <mach/mipi_dsi.h>

#include <asm/irq.h>
#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/time.h>

#include "usb.h"
#include "devices-imx6q.h"
#include "crm_regs.h"
#include "cpu_op-mx6.h"

//WS1Šî”Â‚Å‚Í—LŒø‚É‚·‚éB
//#define WS1_BOARD

#include "board-mx6q_bee01.h"
#include "board-mx6dl_bee01.h"



#ifndef WS1_BOARD
	#define BEE01_USR_DEF_RED_LED			IMX_GPIO_NR(1, 2)
	#define BEE01_TP225						IMX_GPIO_NR(1, 3)
	#define BEE01_KEY_VOL_UP				IMX_GPIO_NR(1, 4)
	#define BEE01_KEY_VOL_DN				IMX_GPIO_NR(1, 5)
	#define BEE01_TP227						IMX_GPIO_NR(1, 6)
	#define BEE01_TP223						IMX_GPIO_NR(1, 7)
	#define BEE01_MICROPHONE_DET			IMX_GPIO_NR(1, 9)
	#define BEE01_CSI0_PWN					IMX_GPIO_NR(1, 16)
	#define BEE01_CSI0_RST_B				IMX_GPIO_NR(1, 17)
	#define BEE01_ACCL_INT1_IN				IMX_GPIO_NR(1, 18)
	#define BEE01_TP253						IMX_GPIO_NR(1, 19)
	#define BEE01_ACCL_INT2_IN				IMX_GPIO_NR(1, 20)
	#define BEE01_BEJ_PWM					IMX_GPIO_NR(1, 21)
	#define BEE01_TP180						IMX_GPIO_NR(1, 22)
	#define BEE01_TP182						IMX_GPIO_NR(1, 23)
	#define BEE01_TP181						IMX_GPIO_NR(1, 25)
	#define BEE01_TP234						IMX_GPIO_NR(1, 26)
	#define BEE01_UOK_B						IMX_GPIO_NR(1, 27)
	#define BEE01_TP226						IMX_GPIO_NR(1, 28)
	#define BEE01_TP166						IMX_GPIO_NR(1, 29)
	#define BEE01_DISP0_PWR_EN				IMX_GPIO_NR(1, 30)
	#define BEE01_TP179						IMX_GPIO_NR(1, 31)

	#define BEE01_SD3_CD_B					IMX_GPIO_NR(2, 0)
	#define BEE01_SD3_WP					IMX_GPIO_NR(2, 1)
	#define BEE01_LED_Flashing_off			IMX_GPIO_NR(2, 2)
	#define BEE01_LED_OSIRASE_C				IMX_GPIO_NR(2, 3)
	#define BEE01_HOME						IMX_GPIO_NR(2, 4)
	#define BEE01_SENSOR_PWR_EN				IMX_GPIO_NR(2, 5)
//	#define BEE01_TP240						IMX_GPIO_NR(2, 6)
	#define BEE01_PCB_VER					IMX_GPIO_NR(2, 6)
//	#define BEE01_TP241						IMX_GPIO_NR(2, 7)
	#define BEE01_PCB_VER2					IMX_GPIO_NR(2, 7)
	#define BEE01_BOOT_CFG3_6				IMX_GPIO_NR(2, 16)
	#define BEE01_BOOT_CFG3_5				IMX_GPIO_NR(2, 17)
	#define BEE01_BOOT_CFG3_4				IMX_GPIO_NR(2, 18)
	#define BEE01_BOOT_CFG3_3				IMX_GPIO_NR(2, 19)
	#define BEE01_BOOT_CFG3_2				IMX_GPIO_NR(2, 20)
	#define BEE01_BOOT_CFG3_1				IMX_GPIO_NR(2, 21)
	#define BEE01_BOOT_CFG3_0				IMX_GPIO_NR(2, 22)
	#define BEE01_TP212						IMX_GPIO_NR(2, 23)
	#define BEE01_DOK_B						IMX_GPIO_NR(2, 24)
	#define BEE01_TP263						IMX_GPIO_NR(2, 25)
	#define BEE01_BOOT_CFG4_5				IMX_GPIO_NR(2, 26)
	#define BEE01_BOOT_CFG4_2				IMX_GPIO_NR(2, 27)
	#define BEE01_BOOT_CFG4_3				IMX_GPIO_NR(2, 28)
	#define BEE01_BOOT_CFG4_4				IMX_GPIO_NR(2, 29)
	#define BEE01_BOOT_CFG4_6				IMX_GPIO_NR(2, 30)
	#define BEE01_BOOT_CFG4_7				IMX_GPIO_NR(2, 31)

	#define BEE01_BOOT_CFG1_0				IMX_GPIO_NR(3, 0)
	#define BEE01_BOOT_CFG1_1				IMX_GPIO_NR(3, 1)
	#define BEE01_BOOT_CFG1_2				IMX_GPIO_NR(3, 2)
	#define BEE01_BOOT_CFG1_3				IMX_GPIO_NR(3, 3)
	#define BEE01_BOOT_CFG1_4				IMX_GPIO_NR(3, 4)
	#define BEE01_BOOT_CFG1_5				IMX_GPIO_NR(3, 5)
	#define BEE01_BOOT_CFG1_6				IMX_GPIO_NR(3, 6)
	#define BEE01_BOOT_CFG1_7				IMX_GPIO_NR(3, 7)
	#define BEE01_BOOT_CFG2_0				IMX_GPIO_NR(3, 8)
	#define BEE01_BOOT_CFG2_1				IMX_GPIO_NR(3, 9)
	#define BEE01_BOOT_CFG2_2				IMX_GPIO_NR(3, 10)
	#define BEE01_BOOT_CFG2_3				IMX_GPIO_NR(3, 11)
	#define BEE01_BOOT_CFG2_4				IMX_GPIO_NR(3, 12)
	#define BEE01_BOOT_CFG2_5				IMX_GPIO_NR(3, 13)
	#define BEE01_BOOT_CFG2_6				IMX_GPIO_NR(3, 14)
	#define BEE01_BOOT_CFG2_7				IMX_GPIO_NR(3, 15)
	#define BEE01_EIM_D16_OPEN				IMX_GPIO_NR(3, 16)
	#define BEE01_TP242						IMX_GPIO_NR(3, 17)
	#define BEE01_EIM_D18_OPEN				IMX_GPIO_NR(3, 18)
	#define BEE01_TP211						IMX_GPIO_NR(3, 19)
	#define BEE01_TP243						IMX_GPIO_NR(3, 20)
	#define BEE01_USB_OTG_PWR				IMX_GPIO_NR(3, 22)
	#define BEE01_CHG_STATUS1_B				IMX_GPIO_NR(3, 23)
	#define BEE01_TS_PWR_EN					IMX_GPIO_NR(3, 24)
	#define BEE01_PWR_BTN_SNS				IMX_GPIO_NR(3, 25)
	#define BEE01_TP170						IMX_GPIO_NR(3, 30)
	#define BEE01_TP244						IMX_GPIO_NR(3, 31)

	#define BEE01_BATT_ALRT					IMX_GPIO_NR(4, 5)
	#define BEE01_CODEC_PWR_EN				IMX_GPIO_NR(4, 10)
	#define BEE01_KEY_ROW2_OPEN				IMX_GPIO_NR(4, 11)
	#define BEE01_KEY_MENU					IMX_GPIO_NR(4, 14)
	#define BEE01_KEY_BACK					IMX_GPIO_NR(4, 15)
	#define BEE01_TP279						IMX_GPIO_NR(4, 16)
	#define BEE01_DI0_PIN15_OPEN			IMX_GPIO_NR(4, 17)
	#define BEE01_DI0_PIN2_OPEN				IMX_GPIO_NR(4, 18)
	#define BEE01_DI0_PIN3_OPEN				IMX_GPIO_NR(4, 19)
	#define BEE01_DI0_PIN4_OPEN				IMX_GPIO_NR(4, 20)
	#define BEE01_DSP0_DAT0_OPEN			IMX_GPIO_NR(4, 21)
	#define BEE01_DSP0_DAT1_OPEN			IMX_GPIO_NR(4, 22)
	#define BEE01_DSP0_DAT2_OPEN			IMX_GPIO_NR(4, 23)
	#define BEE01_DSP0_DAT3_OPEN			IMX_GPIO_NR(4, 24)
	#define BEE01_DSP0_DAT4_OPEN			IMX_GPIO_NR(4, 25)
	#define BEE01_DSP0_DAT5_OPEN			IMX_GPIO_NR(4, 26)
	#define BEE01_DSP0_DAT6_OPEN			IMX_GPIO_NR(4, 27)
	#define BEE01_DSP0_DAT7_OPEN			IMX_GPIO_NR(4, 28)
	#define BEE01_DSP0_DAT8_OPEN			IMX_GPIO_NR(4, 29)
	#define BEE01_DISP0_DAT9_OPEN			IMX_GPIO_NR(4, 30)
	#define BEE01_DISP0_DAT10_OPEN			IMX_GPIO_NR(4, 31)

	#define BEE01_BOOT_CFG4_1				IMX_GPIO_NR(5, 0)
	#define BEE01_CHG_FLT1_B				IMX_GPIO_NR(5, 2)
	#define BEE01_BOOT_CFG4_0				IMX_GPIO_NR(5, 4)
	#define BEE01_DISP0_DAT11_OPEN			IMX_GPIO_NR(5, 5)
	#define BEE01_DISP0_DAT12_OPEN			IMX_GPIO_NR(5, 6)
	#define BEE01_DISP0_DAT13_OPEN			IMX_GPIO_NR(5, 7)
	#define BEE01_DISP0_DAT14_OPEN			IMX_GPIO_NR(5, 8)
	#define BEE01_DISP0_DAT15_OPEN			IMX_GPIO_NR(5, 9)
	#define BEE01_DISP0_DAT16_OPEN			IMX_GPIO_NR(5, 10)
	#define BEE01_DISP0_DAT17_OPEN			IMX_GPIO_NR(5, 11)
	#define BEE01_DISP0_DAT18_OPEN			IMX_GPIO_NR(5, 12)
	#define BEE01_DISP0_DAT19_OPEN			IMX_GPIO_NR(5, 13)
	#define BEE01_DISP0_DAT20_OPEN			IMX_GPIO_NR(5, 14)
	#define BEE01_DISP0_DAT21_OPEN			IMX_GPIO_NR(5, 15)
	#define BEE01_DISP0_DAT22_OPEN			IMX_GPIO_NR(5, 16)
	#define BEE01_DISP0_DAT23_OPEN			IMX_GPIO_NR(5, 17)
	#define BEE01_TP218						IMX_GPIO_NR(5, 20)

	#define BEE01_BOOT_CFG3_7				IMX_GPIO_NR(6, 6)
	#define BEE01_AMP_EN					IMX_GPIO_NR(6, 7)
	#define BEE01_CAME_EN_B					IMX_GPIO_NR(6, 8)
	#define BEE01_LCD_BL_EN					IMX_GPIO_NR(6, 9)
	#define BEE01_AUX_5V_EN					IMX_GPIO_NR(6, 10)
	#define BEE01_WIFI_32K_EN				IMX_GPIO_NR(6, 11)
	#define BEE01_WLAN_RST					IMX_GPIO_NR(6, 14)
	#define BEE01_WiFi_BT_PWR_EN			IMX_GPIO_NR(6, 15)
	#define BEE01_AUDIO_CLK_EN_B			IMX_GPIO_NR(6, 16)
	#define BEE01_TP252						IMX_GPIO_NR(6, 17)
	#define BEE01_TP251						IMX_GPIO_NR(6, 18)
	#define BEE01_RGMII_TXC_OPEN			IMX_GPIO_NR(6, 19)
	#define BEE01_RGMII_TD0_OPEN			IMX_GPIO_NR(6, 20)
	#define BEE01_RGMII_TD1_OPEN			IMX_GPIO_NR(6, 21)
	#define BEE01_RGMII_TD2_OPEN			IMX_GPIO_NR(6, 22)
	#define BEE01_RGMII_TD3_OPEN			IMX_GPIO_NR(6, 23)
	#define BEE01_RGMII_RX_CTL_OPEN			IMX_GPIO_NR(6, 24)
	#define BEE01_RGMII_RD0_OPEN			IMX_GPIO_NR(6, 25)
	#define BEE01_RGMII_TX_CTL_OPEN			IMX_GPIO_NR(6, 26)
	#define BEE01_RGMII_RD1_OPEN			IMX_GPIO_NR(6, 27)
	#define BEE01_RGMII_RD2_OPEN			IMX_GPIO_NR(6, 28)
	#define BEE01_RGMII_RD3_OPEN			IMX_GPIO_NR(6, 29)
	#define BEE01_RGMII_RXC_OPEN			IMX_GPIO_NR(6, 30)
	#define BEE01_TP213						IMX_GPIO_NR(6, 31)

	#define BEE01_TP250						IMX_GPIO_NR(7, 0)
	#define BEE01_TS_INT					IMX_GPIO_NR(7, 1)
	#define BEE01_HEADPHONE_DET				IMX_GPIO_NR(7, 8)
	#define BEE01_CHG_LED_B					IMX_GPIO_NR(7, 11)
	#define BEE01_BAT_CEN_B					IMX_GPIO_NR(7, 12)
	#define BEE01_PMIC_INT_B				IMX_GPIO_NR(7, 13)
#else
	#define BEE01_USR_DEF_RED_LED			IMX_GPIO_NR(1, 2)
	#define BEE01_TP225						IMX_GPIO_NR(1, 3)
	#define BEE01_KEY_VOL_UP				IMX_GPIO_NR(1, 4)
	#define BEE01_KEY_VOL_DN				IMX_GPIO_NR(1, 5)
	#define BEE01_TP227						IMX_GPIO_NR(1, 6)
	#define BEE01_TP223						IMX_GPIO_NR(1, 7)
	#define BEE01_MICROPHONE_DET			IMX_GPIO_NR(1, 9)
	#define BEE01_CSI0_PWN					IMX_GPIO_NR(1, 16)
	#define BEE01_CSI0_RST_B				IMX_GPIO_NR(1, 17)
	#define BEE01_ACCL_INT1_IN				IMX_GPIO_NR(1, 18)
	#define BEE01_TP253						IMX_GPIO_NR(1, 19)
	#define BEE01_ACCL_INT2_IN				IMX_GPIO_NR(1, 20)
	#define BEE01_TP180						IMX_GPIO_NR(1, 22)
	#define BEE01_TP182						IMX_GPIO_NR(1, 23)
	#define BEE01_TP181						IMX_GPIO_NR(1, 25)
	#define BEE01_TP234						IMX_GPIO_NR(1, 26)
	#define BEE01_UOK_B						IMX_GPIO_NR(1, 27)
	#define BEE01_TP226						IMX_GPIO_NR(1, 28)
	#define BEE01_USB_H1_PWR_EN				IMX_GPIO_NR(1, 29)
	#define BEE01_DISP0_PWR_EN				IMX_GPIO_NR(1, 30)
	#define BEE01_TP179						IMX_GPIO_NR(1, 31)

	#define BEE01_SD3_CD_B					IMX_GPIO_NR(2, 0)
	#define BEE01_SD3_WP					IMX_GPIO_NR(2, 1)
	#define BEE01_LED_Flashing_off			IMX_GPIO_NR(2, 2)
	#define BEE01_LED_GREEN					IMX_GPIO_NR(2, 3)
	#define BEE01_HOME						IMX_GPIO_NR(2, 4)
	#define BEE01_SENSOR_PWR_EN				IMX_GPIO_NR(2, 5)
//	#define BEE01_TP240						IMX_GPIO_NR(2, 6)
	#define BEE01_PCB_VER					IMX_GPIO_NR(2, 6)
//	#define BEE01_TP241						IMX_GPIO_NR(2, 7)
	#define BEE01_PCB_VER2					IMX_GPIO_NR(2, 7)
	#define BEE01_BOOT_CFG3_6				IMX_GPIO_NR(2, 16)
	#define BEE01_BOOT_CFG3_5				IMX_GPIO_NR(2, 17)
	#define BEE01_BOOT_CFG3_4				IMX_GPIO_NR(2, 18)
	#define BEE01_BOOT_CFG3_3				IMX_GPIO_NR(2, 19)
	#define BEE01_BOOT_CFG3_2				IMX_GPIO_NR(2, 20)
	#define BEE01_BOOT_CFG3_1				IMX_GPIO_NR(2, 21)
	#define BEE01_BOOT_CFG3_0				IMX_GPIO_NR(2, 22)
	#define BEE01_TP212						IMX_GPIO_NR(2, 23)
	#define BEE01_DOK_B						IMX_GPIO_NR(2, 24)
	#define BEE01_TP263						IMX_GPIO_NR(2, 25)
	#define BEE01_BOOT_CFG4_5				IMX_GPIO_NR(2, 26)
	#define BEE01_BOOT_CFG4_2				IMX_GPIO_NR(2, 27)
	#define BEE01_BOOT_CFG4_3				IMX_GPIO_NR(2, 28)
	#define BEE01_BOOT_CFG4_4				IMX_GPIO_NR(2, 29)
	#define BEE01_BOOT_CFG4_6				IMX_GPIO_NR(2, 30)
	#define BEE01_BOOT_CFG4_7				IMX_GPIO_NR(2, 31)

	#define BEE01_BOOT_CFG1_0				IMX_GPIO_NR(3, 0)
	#define BEE01_BOOT_CFG1_1				IMX_GPIO_NR(3, 1)
	#define BEE01_BOOT_CFG1_2				IMX_GPIO_NR(3, 2)
	#define BEE01_BOOT_CFG1_3				IMX_GPIO_NR(3, 3)
	#define BEE01_BOOT_CFG1_4				IMX_GPIO_NR(3, 4)
	#define BEE01_BOOT_CFG1_5				IMX_GPIO_NR(3, 5)
	#define BEE01_BOOT_CFG1_6				IMX_GPIO_NR(3, 6)
	#define BEE01_BOOT_CFG1_7				IMX_GPIO_NR(3, 7)
	#define BEE01_BOOT_CFG2_0				IMX_GPIO_NR(3, 8)
	#define BEE01_BOOT_CFG2_1				IMX_GPIO_NR(3, 9)
	#define BEE01_BOOT_CFG2_2				IMX_GPIO_NR(3, 10)
	#define BEE01_BOOT_CFG2_3				IMX_GPIO_NR(3, 11)
	#define BEE01_BOOT_CFG2_4				IMX_GPIO_NR(3, 12)
	#define BEE01_BOOT_CFG2_5				IMX_GPIO_NR(3, 13)
	#define BEE01_BOOT_CFG2_6				IMX_GPIO_NR(3, 14)
	#define BEE01_BOOT_CFG2_7				IMX_GPIO_NR(3, 15)
	#define BEE01_EIM_D16_OPEN				IMX_GPIO_NR(3, 16)
	#define BEE01_TP242						IMX_GPIO_NR(3, 17)
	#define BEE01_EIM_D18_OPEN				IMX_GPIO_NR(3, 18)
	#define BEE01_TP211						IMX_GPIO_NR(3, 19)
	#define BEE01_TP243						IMX_GPIO_NR(3, 20)
	#define BEE01_USB_OTG_PWR				IMX_GPIO_NR(3, 22)
	#define BEE01_CHG_STATUS1_B				IMX_GPIO_NR(3, 23)
	#define BEE01_TS_INT					IMX_GPIO_NR(3, 24)
	#define BEE01_PWR_BTN_SNS				IMX_GPIO_NR(3, 25)
	#define BEE01_TP244						IMX_GPIO_NR(3, 31)

	#define BEE01_BATT_ALRT					IMX_GPIO_NR(4, 5)
	#define BEE01_CODEC_PWR_EN				IMX_GPIO_NR(4, 10)
	#define BEE01_KEY_ROW2_OPEN				IMX_GPIO_NR(4, 11)
	#define BEE01_TP219						IMX_GPIO_NR(4, 14)
	#define BEE01_TP220						IMX_GPIO_NR(4, 15)
	#define BEE01_TP279						IMX_GPIO_NR(4, 16)
	#define BEE01_TP278						IMX_GPIO_NR(4, 17)
	#define BEE01_TP275						IMX_GPIO_NR(4, 18)
	#define BEE01_TP276						IMX_GPIO_NR(4, 19)
	#define BEE01_TP277						IMX_GPIO_NR(4, 20)
	#define BEE01_TP267						IMX_GPIO_NR(4, 21)
	#define BEE01_TP268						IMX_GPIO_NR(4, 22)
	#define BEE01_TP269						IMX_GPIO_NR(4, 23)
	#define BEE01_TP270						IMX_GPIO_NR(4, 24)
	#define BEE01_TP271						IMX_GPIO_NR(4, 25)
	#define BEE01_TP272						IMX_GPIO_NR(4, 26)
	#define BEE01_TP273						IMX_GPIO_NR(4, 27)
	#define BEE01_TP274						IMX_GPIO_NR(4, 28)
	#define BEE01_TP153						IMX_GPIO_NR(4, 29)
	#define BEE01_DISP0_DAT9_OPEN			IMX_GPIO_NR(4, 30)
	#define BEE01_DISP0_DAT10_OPEN			IMX_GPIO_NR(4, 31)

	#define BEE01_BOOT_CFG4_1				IMX_GPIO_NR(5, 0)
	#define BEE01_CHG_FLT1_B				IMX_GPIO_NR(5, 2)
	#define BEE01_BOOT_CFG4_0				IMX_GPIO_NR(5, 4)
	#define BEE01_DISP0_DAT11_OPEN			IMX_GPIO_NR(5, 5)
	#define BEE01_DISP0_DAT12_OPEN			IMX_GPIO_NR(5, 6)
	#define BEE01_DISP0_DAT13_OPEN			IMX_GPIO_NR(5, 7)
	#define BEE01_DISP0_DAT14_OPEN			IMX_GPIO_NR(5, 8)
	#define BEE01_DISP0_DAT15_OPEN			IMX_GPIO_NR(5, 9)
	#define BEE01_DISP0_DAT16_OPEN			IMX_GPIO_NR(5, 10)
	#define BEE01_DISP0_DAT17_OPEN			IMX_GPIO_NR(5, 11)
	#define BEE01_DISP0_DAT18_OPEN			IMX_GPIO_NR(5, 12)
	#define BEE01_DISP0_DAT19_OPEN			IMX_GPIO_NR(5, 13)
	#define BEE01_DISP0_DAT20_OPEN			IMX_GPIO_NR(5, 14)
	#define BEE01_DISP0_DAT21_OPEN			IMX_GPIO_NR(5, 15)
	#define BEE01_DISP0_DAT22_OPEN			IMX_GPIO_NR(5, 16)
	#define BEE01_DISP0_DAT23_OPEN			IMX_GPIO_NR(5, 17)
	#define BEE01_TP218						IMX_GPIO_NR(5, 20)

	#define BEE01_BOOT_CFG3_7				IMX_GPIO_NR(6, 6)
	#define BEE01_AMP_EN					IMX_GPIO_NR(6, 7)
	#define BEE01_CAME_EN_B					IMX_GPIO_NR(6, 8)
	#define BEE01_LCD_BL_EN					IMX_GPIO_NR(6, 9)
	#define BEE01_AUX_5V_EN					IMX_GPIO_NR(6, 10)
	#define BEE01_WIFI_32K_EN				IMX_GPIO_NR(6, 11)
	#define BEE01_WLAN_RST					IMX_GPIO_NR(6, 14)
	#define BEE01_WiFi_BT_PWR_EN			IMX_GPIO_NR(6, 15)
	#define BEE01_AUDIO_CLK_EN_B			IMX_GPIO_NR(6, 16)
	#define BEE01_TP252						IMX_GPIO_NR(6, 17)
	#define BEE01_TP251						IMX_GPIO_NR(6, 18)
	#define BEE01_RGMII_TXC_OPEN			IMX_GPIO_NR(6, 19)
	#define BEE01_RGMII_TD0_OPEN			IMX_GPIO_NR(6, 20)
	#define BEE01_RGMII_TD1_OPEN			IMX_GPIO_NR(6, 21)
	#define BEE01_RGMII_TD2_OPEN			IMX_GPIO_NR(6, 22)
	#define BEE01_RGMII_TD3_OPEN			IMX_GPIO_NR(6, 23)
	#define BEE01_RGMII_RX_CTL_OPEN			IMX_GPIO_NR(6, 24)
	#define BEE01_RGMII_RD0_OPEN			IMX_GPIO_NR(6, 25)
	#define BEE01_RGMII_TX_CTL_OPEN			IMX_GPIO_NR(6, 26)
	#define BEE01_RGMII_RD1_OPEN			IMX_GPIO_NR(6, 27)
	#define BEE01_RGMII_RD2_OPEN			IMX_GPIO_NR(6, 28)
	#define BEE01_RGMII_RD3_OPEN			IMX_GPIO_NR(6, 29)
	#define BEE01_RGMII_RXC_OPEN			IMX_GPIO_NR(6, 30)
	#define BEE01_TP213						IMX_GPIO_NR(6, 31)

	#define BEE01_TP250						IMX_GPIO_NR(7, 0)
	#define BEE01_TP249						IMX_GPIO_NR(7, 1)
	#define BEE01_HEADPHONE_DET				IMX_GPIO_NR(7, 8)
	#define BEE01_GPIO_16_OPEN				IMX_GPIO_NR(7, 11)
	#define BEE01_TP224						IMX_GPIO_NR(7, 12)
	#define BEE01_PMIC_INT_B				IMX_GPIO_NR(7, 13)
#endif //WS1_BOARD


static int mma8452_position = 4;
static int caam_enabled;

extern char *gp_reg_id;
extern char *soc_reg_id;
extern char *pu_reg_id;

int mx6_bee01_get_PCB_version(void)
{
#if 0
	if(gpio_get_value(BEE01_PCB_VER))
	{
		return 2;
	}
	else
	{
		return 1;
	}
#else
	int ver;

	if(gpio_get_value(BEE01_PCB_VER2))
	{
		if(gpio_get_value(BEE01_PCB_VER))
		{
			ver = 2;
		}
		else
		{
			ver = 1;
		}
	}
	else
	{
		if(gpio_get_value(BEE01_PCB_VER))
		{
			ver = 3;
		}
		else
		{
			ver = 4;
		}
	}

	return ver;
#endif
}

/* WiFi */
static const struct esdhc_platform_data mx6q_bee01_sd2_data __initconst = {
	.always_present = 0,
	.keep_power_at_suspend = 1,
	.support_8bit = 0,
	.delay_line = 0,
	.cd_type = ESDHC_CD_GPIO,
	.cd_gpio = BEE01_TP180,
};

/* SD card */
static const struct esdhc_platform_data mx6q_bee01_sd3_data __initconst = {
	.cd_gpio = BEE01_SD3_CD_B,
	.wp_gpio = BEE01_SD3_WP,
	.keep_power_at_suspend = 0,
	.support_8bit = 0,
	.delay_line = 0,
	.cd_type = ESDHC_CD_CONTROLLER,
};

/* eMMC */
static const struct esdhc_platform_data mx6q_bee01_sd4_data __initconst = {
	.always_present = 1,
	.keep_power_at_suspend = 1,
	.support_8bit = 1,
	.delay_line = 0,
	.cd_type = ESDHC_CD_PERMANENT,
};

static const struct anatop_thermal_platform_data
	mx6q_bee01_anatop_thermal_data __initconst = {
		.name = "anatop_thermal",
};

static inline void mx6q_bee01_init_uart(void)
{
//	imx6q_add_imx_uart(2, NULL);
//	imx6q_add_imx_uart(1, NULL);
	imx6q_add_imx_uart(0, NULL);
}

static void mx6q_csi0_cam_powerdown(int powerdown)
{
	if (powerdown)
		gpio_set_value(BEE01_CSI0_PWN, 1);
	else
		gpio_set_value(BEE01_CSI0_PWN, 0);

	msleep(2);
}

static void mx6q_csi0_cam_reset(void)
{
	gpio_set_value(BEE01_CSI0_RST_B, 0);
	msleep(10);
	gpio_set_value(BEE01_CSI0_RST_B, 1);
}

static void mx6q_csi0_io_init(void)
{
	if (cpu_is_mx6q())
		mxc_iomux_v3_setup_multiple_pads(mx6q_bee01_csi0_sensor_pads,
			ARRAY_SIZE(mx6q_bee01_csi0_sensor_pads));
	else if (cpu_is_mx6dl())
		mxc_iomux_v3_setup_multiple_pads(mx6dl_bee01_csi0_sensor_pads,
			ARRAY_SIZE(mx6dl_bee01_csi0_sensor_pads));

	/* Camera reset */
	gpio_request(BEE01_CSI0_RST_B, "cam-reset");
	gpio_direction_output(BEE01_CSI0_RST_B, 0);

	/* Camera power down */
	gpio_request(BEE01_CSI0_PWN, "cam-pwdn");
	gpio_direction_output(BEE01_CSI0_PWN, 1);
	msleep(5);
	gpio_set_value(BEE01_CSI0_PWN, 0);
	msleep(5);
	gpio_set_value(BEE01_CSI0_RST_B, 0);
	msleep(1);
	gpio_set_value(BEE01_CSI0_RST_B, 1);

	/* For MX6Q:
	 * GPR1 bit19 and bit20 meaning:
	 * Bit19:       0 - Enable mipi to IPU1 CSI0
	 *                      virtual channel is fixed to 0
	 *              1 - Enable parallel interface to IPU1 CSI0
	 * Bit20:       0 - Enable mipi to IPU2 CSI1
	 *                      virtual channel is fixed to 3
	 *              1 - Enable parallel interface to IPU2 CSI1
	 * IPU1 CSI1 directly connect to mipi csi2,
	 *      virtual channel is fixed to 1
	 * IPU2 CSI0 directly connect to mipi csi2,
	 *      virtual channel is fixed to 2
	 *
	 * For MX6DL:
	 * GPR13 bit 0-2 IPU_CSI0_MUX
	 *   000 MIPI_CSI0
	 *   100 IPU CSI0
	 */
	if (cpu_is_mx6q())
		mxc_iomux_set_gpr_register(1, 19, 1, 1);
	else if (cpu_is_mx6dl())
		mxc_iomux_set_gpr_register(13, 0, 3, 4);
}

static struct fsl_mxc_camera_platform_data camera_data = {
	.mclk = 24000000,
	.mclk_source = 0,
	.csi = 0,
//	.io_init = mx6q_csi0_io_init,
	.pwdn = mx6q_csi0_cam_powerdown,
	.reset = mx6q_csi0_cam_reset,
};

void bee01_max8903_chg_enable(int en)
{
	if(en)
	{
		gpio_set_value(BEE01_BAT_CEN_B, 0);
	}
	else
	{
		gpio_set_value(BEE01_BAT_CEN_B, 1);
	}
}

static struct imxi2c_platform_data mx6q_bee01_i2c_data = {
	.bitrate = 300000,
};


static struct i2c_board_info mxc_i2c0_board_info[] __initdata = {
	{
		I2C_BOARD_INFO("wm8978", 0x1a),
	},
	{
		I2C_BOARD_INFO("sr200pc20", 0x20),
		.platform_data = (void *)&camera_data,
	},
	{
		I2C_BOARD_INFO("mma8451", 0x1c),
		.platform_data = (void *)&mma8452_position,
	},
	{
		I2C_BOARD_INFO("max17048", 0x36),
		.irq = gpio_to_irq(BEE01_BATT_ALRT),
	},
};

static struct i2c_board_info mxc_i2c1_board_info[] __initdata = {
	{
		I2C_BOARD_INFO("bu21025", 0x48),
		.irq = gpio_to_irq(BEE01_TS_INT),
	},
	/* MMPF0100 */
};

void imx6q_bee01_ts_pwr_on(int on)
{
#ifndef WS1_BOARD
	if (on)
		gpio_set_value(BEE01_TS_PWR_EN, 1);
	else
		gpio_set_value(BEE01_TS_PWR_EN, 0);
#endif
}

static void imx6q_bee01_usbotg_vbus(bool on)
{
	if (on)
		gpio_set_value(BEE01_USB_OTG_PWR, 1);
	else
		gpio_set_value(BEE01_USB_OTG_PWR, 0);
}

static void imx6q_bee01_host1_vbus(bool on)
{
#ifdef WS1_BOARD
	if (on)
		gpio_set_value(BEE01_USB_H1_PWR_EN, 1);
	else
		gpio_set_value(BEE01_USB_H1_PWR_EN, 0);
#endif //WS1_BOARD
}

static void __init imx6q_bee01_init_usb(void)
{
	int ret = 0;

	imx_otg_base = MX6_IO_ADDRESS(MX6Q_USB_OTG_BASE_ADDR);
	/* disable external charger detect,
	 * or it will affect signal quality at dp .
	 */
	ret = gpio_request(BEE01_USB_OTG_PWR, "usb-pwr");
	if (ret) {
		pr_err("failed to get GPIO SABRESD_USB_OTG_PWR: %d\n",
			ret);
		return;
	}
	gpio_direction_output(BEE01_USB_OTG_PWR, 0);

#ifdef WS1_BOARD
	/* keep USB host1 VBUS always on */
	ret = gpio_request(BEE01_USB_H1_PWR_EN, "usb-h1-pwr");
	if (ret) {
		pr_err("failed to get GPIO BEE01_USB_H1_PWR_EN: %d\n",
			ret);
		return;
	}
	gpio_direction_output(BEE01_USB_H1_PWR_EN, 0);
#endif //WS1_BOARD

//	if (board_is_mx6_reva())
//		mxc_iomux_set_gpr_register(1, 13, 1, 1);
//	else
//		mxc_iomux_set_gpr_register(1, 13, 1, 0);

	mx6_set_otghost_vbus_func(imx6q_bee01_usbotg_vbus);
#ifdef WS1_BOARD
	mx6_set_host1_vbus_func(imx6q_bee01_host1_vbus);
#endif
}

static struct viv_gpu_platform_data imx6q_gpu_pdata __initdata = {
	.reserved_mem_size = SZ_128M + SZ_64M - SZ_16M,
};

static struct imx_asrc_platform_data imx_asrc_data = {
	.channel_bits = 4,
	.clk_map_ver = 2,
};

static struct ipuv3_fb_platform_data bee01_fb_data[] = {
	{ /*fb0*/
	.disp_dev = "ldb",
#ifndef WS1_BOARD
	.interface_pix_fmt = IPU_PIX_FMT_RGB24,
#else
	.interface_pix_fmt = IPU_PIX_FMT_RGB666,
#endif
		.mode_str = "LDB-XGA",
	.default_bpp = 16,
	.int_clk = false,
	.late_init = true,
	}
};

static struct fsl_mxc_ldb_platform_data ldb_data = {
	.ipu_id = 0,
#ifndef WS1_BOARD
	.disp_id = 1,
#else
	.disp_id = 0,
#endif
	.ext_ref = 1,
#ifndef WS1_BOARD
	.mode = LDB_SIN1,
#else
	.mode = LDB_SIN0,
#endif
};

int imx6q_bee01_AC_status(void)
{
	if((gpio_get_value(BEE01_DOK_B) == 0)
	//	|| (gpio_get_value(BEE01_UOK_B) == 0)
	)
	{
		return 1;
	}
	else
		return 0;
}

static struct max8903_pdata charger1_data = {
	.dok = BEE01_DOK_B,
	.uok = BEE01_UOK_B,
	.chg = BEE01_CHG_STATUS1_B,
	.flt = BEE01_CHG_FLT1_B,
	.dcm_always_high = true,
	.dc_valid = true,
	.usb_valid = true,
};

static struct platform_device bee01_max8903_charger_1 = {
	.name	= "max8903-charger",
	.id	= 1,
	.dev	= {
		.platform_data = &charger1_data,
	},
};

static struct imx_ipuv3_platform_data ipu_data[] = {
	{
	.rev = 4,
	.csi_clk[0] = "clko_clk",
	.bypass_reset = true,
	}, {
	.rev = 4,
	.csi_clk[0] = "clko_clk",
	.bypass_reset = true,
	},
};

static struct ion_platform_data imx_ion_data = {
	.nr = 1,
	.heaps = {
		{
		.id = 0,
		.type = ION_HEAP_TYPE_CARVEOUT,
		.name = "vpu_ion",
		.size = SZ_16M,
		.cacheable = 1,
		},
	},
};

static struct fsl_mxc_capture_platform_data capture_data[] = {
	{
		.csi = 0,
		.ipu = 0,
		.mclk_source = 0,
		.is_mipi = 0,
	}, 
};


static void bee01_suspend_enter(void)
{
	/* suspend preparation */
	/* Disable AUX 5V */
	gpio_set_value(BEE01_AUX_5V_EN, 0);
}

static void bee01_suspend_exit(void)
{
	/* resume restore */
	/* Enable AUX 5V */
	gpio_set_value(BEE01_AUX_5V_EN, 1);
}

static const struct pm_platform_data mx6q_bee01_pm_data __initconst = {
	.name = "imx_pm",
	.suspend_enter = bee01_suspend_enter,
	.suspend_exit = bee01_suspend_exit,
};

static struct regulator_consumer_supply bee01_vmmc_consumers[] = {
	REGULATOR_SUPPLY("vmmc", "sdhci-esdhc-imx.1"),
	REGULATOR_SUPPLY("vmmc", "sdhci-esdhc-imx.2"),
	REGULATOR_SUPPLY("vmmc", "sdhci-esdhc-imx.3"),
};

static struct regulator_init_data bee01_vmmc_init = {
	.num_consumer_supplies = ARRAY_SIZE(bee01_vmmc_consumers),
	.consumer_supplies = bee01_vmmc_consumers,
};

static struct fixed_voltage_config bee01_vmmc_reg_config = {
	.supply_name		= "vmmc",
	.microvolts		= 3300000,
	.gpio			= -1,
	.init_data		= &bee01_vmmc_init,
};

static struct platform_device bee01_vmmc_reg_devices = {
	.name	= "reg-fixed-voltage",
	.id	= 3,
	.dev	= {
		.platform_data = &bee01_vmmc_reg_config,
	},
};

static struct platform_device mx6_sabresd_audio_wm8978_device = {
	.name = "imx-wm8978",
};

static struct mxc_audio_platform_data wm8978_data = {
	.ssi_num = 1,
	.src_port = 2,
	.ext_port = 3,
	.hp_gpio = BEE01_HEADPHONE_DET,
	.hp_active_low = 1,
	.mic_gpio = BEE01_MICROPHONE_DET,
	.mic_active_low = 0,
};

static struct imx_ssi_platform_data mx6_bee01_ssi_pdata = {
	.flags = IMX_SSI_DMA | IMX_SSI_SYN,
};

static int __init imx6q_init_audio(int rate)
{
	wm8978_data.sysclk = rate;
	mxc_register_device(&mx6_sabresd_audio_wm8978_device,
				    &wm8978_data);
	imx6q_add_imx_ssi(1, &mx6_bee01_ssi_pdata);

	return 0;
}

#if defined(CONFIG_LEDS_TRIGGER) || defined(CONFIG_LEDS_GPIO)

#define GPIO_LED(gpio_led, name_led, act_low, state_suspend, trigger)	\
{									\
	.gpio			= gpio_led,				\
	.name			= name_led,				\
	.active_low		= act_low,				\
	.retain_state_suspended = state_suspend,			\
	.default_state		= 0,					\
	.default_trigger	= "max8903-"trigger,		\
}

/* use to show a external power source is connected
 * GPIO_LED(SABRESD_CHARGE_DONE, "chg_detect", 0, 1, "ac-online"),
 */
static struct gpio_led imx6q_gpio_leds[] = {
	GPIO_LED(BEE01_USR_DEF_RED_LED, "chg_now_led", 0, 1, "charger-charging"),
/* For the latest B4 board, this GPIO_1 is connected to POR_B,
which will reset the whole board if this pin's level is changed,
so, for the latest board, we have to avoid using this pin as
GPIO.
	GPIO_LED(SABRESD_CHARGE_DONE, "chg_done_led", 0, 1,
			"charger-full"),
*/
};

static struct gpio_led_platform_data imx6q_gpio_leds_data = {
	.leds		= imx6q_gpio_leds,
	.num_leds	= ARRAY_SIZE(imx6q_gpio_leds),
};

static struct platform_device imx6q_gpio_led_device = {
	.name		= "leds-gpio",
	.id		= -1,
	.num_resources  = 0,
	.dev		= {
		.platform_data = &imx6q_gpio_leds_data,
	}
};

static void __init imx6q_add_device_gpio_leds(void)
{
	platform_device_register(&imx6q_gpio_led_device);
}
#else
static void __init imx6q_add_device_gpio_leds(void) {}
#endif

#if defined(CONFIG_KEYBOARD_GPIO) || defined(CONFIG_KEYBOARD_GPIO_MODULE)
#define GPIO_BUTTON(gpio_num, ev_code, act_low, descr, wake, debounce)	\
{								\
	.gpio		= gpio_num,				\
	.type		= EV_KEY,				\
	.code		= ev_code,				\
	.active_low	= act_low,				\
	.desc		= "btn " descr,				\
	.wakeup		= wake,					\
	.debounce_interval = debounce,				\
}

static struct gpio_keys_button new_bee01_buttons[] = {
	GPIO_BUTTON(BEE01_KEY_VOL_UP, KEY_VOLUMEUP, 1, "volume-up", 0, 1),
	GPIO_BUTTON(BEE01_KEY_VOL_DN, KEY_VOLUMEDOWN, 1, "volume-down", 0, 1),
	GPIO_BUTTON(BEE01_HOME, KEY_HOMEPAGE, 1, "home", 0, 1),
	GPIO_BUTTON(BEE01_PWR_BTN_SNS, KEY_POWER, 1, "power-key", 1, 1),
#ifndef WS1_BOARD
	GPIO_BUTTON(BEE01_KEY_MENU, KEY_MENU, 1, "menu-key", 0, 1),
	GPIO_BUTTON(BEE01_KEY_BACK, KEY_BACK, 1, "back-key", 0, 1),
#endif //WS1_BOARD
};

static struct gpio_keys_platform_data new_bee01_button_data = {
	.buttons	= new_bee01_buttons,
	.nbuttons	= ARRAY_SIZE(new_bee01_buttons),
};

static struct platform_device bee01_button_device = {
	.name		= "gpio-keys",
	.id		= -1,
	.num_resources  = 0,
};

static void __init imx6q_add_device_buttons(void)
{
		platform_device_add_data(&bee01_button_device,
				&new_bee01_button_data,
				sizeof(new_bee01_button_data));

	platform_device_register(&bee01_button_device);
}
#else
static void __init imx6q_add_device_buttons(void) {}
#endif

static int pwm_port_enable = -1;

void bee01_disp_enable(int on)
{
	if(on)
	{
		gpio_set_value(BEE01_DISP0_PWR_EN, 1);
	}
	else
	{
		gpio_set_value(BEE01_DISP0_PWR_EN, 0);
	}
}

void bee01_backlight_enable(void)
{
	pwm_port_enable = 0;
	gpio_set_value(BEE01_LCD_BL_EN, 1);
}

static void enable_pwm_port(void)
{
	if(pwm_port_enable == 0)
	{
		pwm_port_enable = 1;
		gpio_free(BEE01_BEJ_PWM);
		mxc_iomux_v3_setup_pad(MX6DL_PAD_SD1_DAT3__PWM1_PWMO);
		msleep(150);
		gpio_set_value(BEE01_LCD_BL_EN, 1);
	}
}

static void disable_pwm_port(void)
{
	if(pwm_port_enable == 1)
	{
		pwm_port_enable = 0;
		gpio_set_value(BEE01_LCD_BL_EN, 0);
		mxc_iomux_v3_setup_pad(MX6DL_PAD_SD1_DAT3__GPIO_1_21);
		gpio_request(BEE01_BEJ_PWM, "pwm");
		gpio_direction_output(BEE01_BEJ_PWM, 1);
	}
}

static struct mxc_pwm_platform_data mxc_pwm_data = {
	.pwmo_invert = 1,
	.enable_pwm_pad = enable_pwm_port,
	.disable_pwm_pad = disable_pwm_port,
};

static struct platform_pwm_backlight_data mx6_bee01_pwm_backlight_data = {
	.pwm_id = 0,
	.max_brightness = 248,
	.dft_brightness = 136,
	.pwm_period_ns = 50000,
};

static struct mxc_dvfs_platform_data bee01_dvfscore_data = {
	.reg_id = "VDDCORE",
	.soc_id	= "VDDSOC",
	.clk1_id = "cpu_clk",
	.clk2_id = "gpc_dvfs_clk",
	.gpc_cntr_offset = MXC_GPC_CNTR_OFFSET,
	.ccm_cdcr_offset = MXC_CCM_CDCR_OFFSET,
	.ccm_cacrr_offset = MXC_CCM_CACRR_OFFSET,
	.ccm_cdhipr_offset = MXC_CCM_CDHIPR_OFFSET,
	.prediv_mask = 0x1F800,
	.prediv_offset = 11,
	.prediv_val = 3,
	.div3ck_mask = 0xE0000000,
	.div3ck_offset = 29,
	.div3ck_val = 2,
	.emac_val = 0x08,
	.upthr_val = 25,
	.dnthr_val = 9,
	.pncthr_val = 33,
	.upcnt_val = 10,
	.dncnt_val = 10,
	.delay_time = 80,
};

static void __init fixup_mxc_board(struct machine_desc *desc, struct tag *tags,
				   char **cmdline, struct meminfo *mi)
{
	char *str;
	struct tag *t;
	int i = 0;
	struct ipuv3_fb_platform_data *pdata_fb = bee01_fb_data;

	for_each_tag(t, tags) {
		if (t->hdr.tag == ATAG_CMDLINE) {
			str = t->u.cmdline.cmdline;
			str = strstr(str, "fbmem=");
			if (str != NULL) {
				str += 6;
				pdata_fb[i++].res_size[0] = memparse(str, &str);
				while (*str == ',' &&
					i < ARRAY_SIZE(bee01_fb_data)) {
					str++;
					pdata_fb[i++].res_size[0] = memparse(str, &str);
				}
			}
			/* ION reserved memory */
			str = t->u.cmdline.cmdline;
			str = strstr(str, "ionmem=");
			if (str != NULL) {
				str += 7;
				imx_ion_data.heaps[0].size = memparse(str, &str);
			}
			/* Primary framebuffer base address */
			str = t->u.cmdline.cmdline;
			str = strstr(str, "fb0base=");
			if (str != NULL) {
				str += 8;
				pdata_fb[0].res_base[0] =
						simple_strtol(str, &str, 16);
			}
			/* GPU reserved memory */
			str = t->u.cmdline.cmdline;
			str = strstr(str, "gpumem=");
			if (str != NULL) {
				str += 7;
				imx6q_gpu_pdata.reserved_mem_size = memparse(str, &str);
			}
			break;
		}
	}
}

static int __init caam_setup(char *__unused)
{
	caam_enabled = 1;
	return 1;
}
early_param("caam", caam_setup);

#define SNVS_LPCR 0x38
static void mx6_snvs_poweroff(void)
{

	void __iomem *mx6_snvs_base =  MX6_IO_ADDRESS(MX6Q_SNVS_BASE_ADDR);
	u32 value;
	value = readl(mx6_snvs_base + SNVS_LPCR);
	/*set TOP and DP_EN bit*/
	writel(value | 0x60, mx6_snvs_base + SNVS_LPCR);
}

#ifdef CONFIG_ANDROID_RAM_CONSOLE
static struct resource ram_console_resource = {
	.name = "android ram console",
	.flags = IORESOURCE_MEM,
};

static struct platform_device android_ram_console = {
	.name = "ram_console",
	.num_resources = 1,
	.resource = &ram_console_resource,
};

static int __init imx6x_add_ram_console(void)
{
	return platform_device_register(&android_ram_console);
}
#else
#define imx6x_add_ram_console() do {} while (0)
#endif

static void mx6_bee01_sensor_power(bool on)
{
	if (on)
		gpio_set_value(BEE01_SENSOR_PWR_EN, 1);
	else
		gpio_set_value(BEE01_SENSOR_PWR_EN, 0);
}

static __init void mx6_bee01_wifi_init(void)
{
	gpio_set_value(BEE01_WIFI_32K_EN, 1);
	gpio_set_value(BEE01_WLAN_RST, 1);
	msleep(310);
	gpio_set_value(BEE01_WiFi_BT_PWR_EN, 1);
	gpio_set_value(BEE01_WLAN_RST, 0);
	msleep(1);
	gpio_set_value(BEE01_WLAN_RST, 1);
}

void mx6_bee01_wifi_ctl(bool on)
{
	if (on)
	{
		gpio_set_value(BEE01_WLAN_RST, 1);
		mxc_iomux_v3_setup_pad(MX6DL_PAD_ENET_MDIO__GPIO_1_22_PD);
	}
	else
	{
		gpio_set_value(BEE01_WLAN_RST, 0);
		mxc_iomux_v3_setup_pad(MX6DL_PAD_ENET_MDIO__GPIO_1_22_PU);
	}
	msleep(1);
}
EXPORT_SYMBOL(mx6_bee01_wifi_ctl);

static struct platform_device bee01_wifi_reset = {
	.name	= "wifi_reset",
};

static __init void mx6_bee01_board_io_init(void)
{
	/* enable sensor 3v3 and 1v8 */
	gpio_request(BEE01_SENSOR_PWR_EN, "sensor-en");
	gpio_direction_output(BEE01_SENSOR_PWR_EN, 0);

	/* BEE01_LED_Flashing_off		GPIO_2_2 */
	gpio_request(BEE01_LED_Flashing_off, "led_flashing_off");
	gpio_direction_output(BEE01_LED_Flashing_off, 0);

#ifndef WS1_BOARD
	/* BEE01_LED_OSIRASE_C				GPIO_2_3 */
	gpio_request(BEE01_LED_OSIRASE_C, "led_osirase_c");
	gpio_direction_output(BEE01_LED_OSIRASE_C, 0);
#else
	/* BEE01_LED_GREEN				GPIO_2_3 */
	gpio_request(BEE01_LED_GREEN, "led_green");
	gpio_direction_output(BEE01_LED_GREEN, 0);
#endif //WS1_BOARD

	/* BEE01_BOOT_CFG4_2			GPIO_2_27 */
	gpio_request(BEE01_BOOT_CFG4_2, "boot_cfg4_2");
	gpio_direction_output(BEE01_BOOT_CFG4_2, 0);

	/* BEE01_BOOT_CFG4_5			GPIO_2_26 */
	gpio_request(BEE01_BOOT_CFG4_5, "boot_cfg4_5");
	gpio_direction_output(BEE01_BOOT_CFG4_5, 0);

	/* BEE01_BOOT_CFG3_0			GPIO_2_22 */
	gpio_request(BEE01_BOOT_CFG3_0, "boot_cfg3_2");
	gpio_direction_output(BEE01_BOOT_CFG3_0, 0);

	/* BEE01_BOOT_CFG3_1			GPIO_2_21 */
	gpio_request(BEE01_BOOT_CFG3_1, "boot_cfg3_1");
	gpio_direction_output(BEE01_BOOT_CFG3_1, 0);

	/* BEE01_BOOT_CFG3_2			GPIO_2_20 */
	gpio_request(BEE01_BOOT_CFG3_2, "boot_cfg3_0");
	gpio_direction_output(BEE01_BOOT_CFG3_2, 0);

	/* BEE01_BOOT_CFG3_3			GPIO_2_19 */
	gpio_request(BEE01_BOOT_CFG3_3, "boot_cfg3_3");
	gpio_direction_output(BEE01_BOOT_CFG3_3, 0);

	/* BEE01_BOOT_CFG3_4			GPIO_2_18 */
	gpio_request(BEE01_BOOT_CFG3_4, "boot_cfg3_4");
	gpio_direction_output(BEE01_BOOT_CFG3_4, 0);

	/* BEE01_BOOT_CFG3_5			GPIO_2_17 */
	gpio_request(BEE01_BOOT_CFG3_5, "boot_cfg3_5");
	gpio_direction_output(BEE01_BOOT_CFG3_5, 0);

	/* BEE01_BOOT_CFG3_6			GPIO_2_16 */
	gpio_request(BEE01_BOOT_CFG3_6, "boot_cfg3_6");
	gpio_direction_output(BEE01_BOOT_CFG3_6, 0);

	/* BEE01_BOOT_CFG3_7			GPIO_6_6 */
	gpio_request(BEE01_BOOT_CFG3_7, "boot_cfg3_7");
	gpio_direction_output(BEE01_BOOT_CFG3_7, 0);

	/* BEE01_BOOT_CFG4_0			GPIO_5_4 */
	gpio_request(BEE01_BOOT_CFG4_0, "boot_cfg4_0");
	gpio_direction_output(BEE01_BOOT_CFG4_0, 0);

	/* BEE01_TS_INT					GPIO_3_24 WS1*/
	/* BEE01_TS_INT					GPIO_7_1 WS2-*/
	gpio_request(BEE01_TS_INT, "ts_int");
	gpio_direction_input(BEE01_TS_INT);

#if 0
	/* BEE01_PWR_BTN_SNS			GPIO_3_25 */
	gpio_request(BEE01_PWR_BTN_SNS, "pwr_btn_sns");
	gpio_direction_input(BEE01_PWR_BTN_SNS);
#endif

	/* BEE01_BOOT_CFG4_6			GPIO_2_30 */
	gpio_request(BEE01_BOOT_CFG4_6, "boot_cfg4_6");
	gpio_direction_output(BEE01_BOOT_CFG4_6, 0);

	/* BEE01_BOOT_CFG4_7			GPIO_2_31 */
	gpio_request(BEE01_BOOT_CFG4_7, "boot_cfg4_7");
	gpio_direction_output(BEE01_BOOT_CFG4_7, 0);

	/* BEE01_BOOT_CFG4_1			GPIO_5_0 */
	gpio_request(BEE01_BOOT_CFG4_1, "boot_cfg4_1");
	gpio_direction_output(BEE01_BOOT_CFG4_1, 0);

	/* BEE01_BOOT_CFG4_3			GPIO_2_28 */
	gpio_request(BEE01_BOOT_CFG4_3, "boot_cfg4_3");
	gpio_direction_output(BEE01_BOOT_CFG4_3, 0);

	/* BEE01_BOOT_CFG4_4			GPIO_2_29 */
	gpio_request(BEE01_BOOT_CFG4_4, "boot_cfg4_4");
	gpio_direction_output(BEE01_BOOT_CFG4_4, 0);

	/* BEE01_BOOT_CFG1_0			GPIO_3_0 */
	gpio_request(BEE01_BOOT_CFG1_0, "boot_cfg1_0");
	gpio_direction_output(BEE01_BOOT_CFG1_0, 0);

	/* BEE01_BOOT_CFG1_1			GPIO_3_1 */
	gpio_request(BEE01_BOOT_CFG1_1, "boot_cfg1_1");
	gpio_direction_output(BEE01_BOOT_CFG1_1, 0);

	/* BEE01_BOOT_CFG1_2			GPIO_3_2 */
	gpio_request(BEE01_BOOT_CFG1_2, "boot_cfg1_2");
	gpio_direction_output(BEE01_BOOT_CFG1_2, 0);

	/* BEE01_BOOT_CFG1_3			GPIO_3_3 */
	gpio_request(BEE01_BOOT_CFG1_3, "boot_cfg1_3");
	gpio_direction_output(BEE01_BOOT_CFG1_3, 0);

	/* BEE01_BOOT_CFG1_4			GPIO_3_4 */
	gpio_request(BEE01_BOOT_CFG1_4, "boot_cfg1_4");
	gpio_direction_output(BEE01_BOOT_CFG1_4, 0);

	/* BEE01_BOOT_CFG1_5			GPIO_3_5 */
	gpio_request(BEE01_BOOT_CFG1_5, "boot_cfg1_5");
	gpio_direction_output(BEE01_BOOT_CFG1_5, 0);

	/* BEE01_BOOT_CFG1_6			GPIO_3_6 */
	gpio_request(BEE01_BOOT_CFG1_6, "boot_cfg1_6");
	gpio_direction_output(BEE01_BOOT_CFG1_6, 0);

	/* BEE01_BOOT_CFG1_7			GPIO_3_7 */
	gpio_request(BEE01_BOOT_CFG1_7, "boot_cfg1_7");
	gpio_direction_output(BEE01_BOOT_CFG1_7, 0);

	/* BEE01_BOOT_CFG2_0			GPIO_3_8 */
	gpio_request(BEE01_BOOT_CFG2_0, "boot_cfg2_0");
	gpio_direction_output(BEE01_BOOT_CFG2_0, 0);

	/* BEE01_BOOT_CFG2_1			GPIO_3_9 */
	gpio_request(BEE01_BOOT_CFG2_1, "boot_cfg2_1");
	gpio_direction_output(BEE01_BOOT_CFG2_1, 0);

	/* BEE01_BOOT_CFG2_2			GPIO_3_10 */
	gpio_request(BEE01_BOOT_CFG2_2, "boot_cfg2_2");
	gpio_direction_output(BEE01_BOOT_CFG2_2, 0);

	/* BEE01_BOOT_CFG2_3			GPIO_3_11 */
	gpio_request(BEE01_BOOT_CFG2_3, "boot_cfg2_3");
	gpio_direction_output(BEE01_BOOT_CFG2_3, 0);

	/* BEE01_BOOT_CFG2_4			GPIO_3_12 */
	gpio_request(BEE01_BOOT_CFG2_4, "boot_cfg2_4");
	gpio_direction_output(BEE01_BOOT_CFG2_4, 0);

	/* BEE01_BOOT_CFG2_5			GPIO_3_13 */
	gpio_request(BEE01_BOOT_CFG2_5, "boot_cfg2_5");
	gpio_direction_output(BEE01_BOOT_CFG2_5, 0);

	/* BEE01_BOOT_CFG2_6			GPIO_3_14 */
	gpio_request(BEE01_BOOT_CFG2_6, "boot_cfg2_6");
	gpio_direction_output(BEE01_BOOT_CFG2_6, 0);

	/* BEE01_BOOT_CFG2_7			GPIO_3_15 */
	gpio_request(BEE01_BOOT_CFG2_7, "boot_cfg2_7");
	gpio_direction_output(BEE01_BOOT_CFG2_7, 0);

	/* BEE01_ACCL_INT1_IN			GPIO_1_18 */
	gpio_request(BEE01_ACCL_INT1_IN, "accl_int1_in");
	gpio_direction_input(BEE01_ACCL_INT1_IN);

	/* BEE01_ACCL_INT2_IN			GPIO_1_20 */
	gpio_request(BEE01_ACCL_INT2_IN, "accl_int2_in");
	gpio_direction_input(BEE01_ACCL_INT2_IN);

	/* BEE01_DISP0_PWR_EN			GPIO_1_30 */
	gpio_request(BEE01_DISP0_PWR_EN, "disp0_pwr_en");
	gpio_direction_output(BEE01_DISP0_PWR_EN, 1);

	/* BEE01_BATT_ALRT				GPIO_4_5 */
	gpio_request(BEE01_BATT_ALRT, "batt_alrt");
	gpio_direction_input(BEE01_BATT_ALRT);

	/* BEE01_WIFI_32K_EN			GPIO_6_11 */
	gpio_request(BEE01_WIFI_32K_EN, "wifi_32k_en");
	gpio_direction_output(BEE01_WIFI_32K_EN, 1);

	/* BEE01_WLAN_RST				GPIO_6_14 */
	gpio_request(BEE01_WLAN_RST, "wlan_rst");
	gpio_direction_output(BEE01_WLAN_RST, 0);

	/* BEE01_WiFi_BT_PWR_EN			GPIO_6_15 */
	gpio_request(BEE01_WiFi_BT_PWR_EN, "wifi_bt_pwr_en");
	gpio_direction_output(BEE01_WiFi_BT_PWR_EN, 0);

	/* BEE01_AUDIO_CLK_EN_B			GPIO_6_16 */
	gpio_request(BEE01_AUDIO_CLK_EN_B, "audio_clk_en_b");
	gpio_direction_output(BEE01_AUDIO_CLK_EN_B, 0);

	/* BEE01_CAME_EN_B				GPIO_6_8 */
	gpio_request(BEE01_CAME_EN_B, "came_en_b");
	gpio_direction_output(BEE01_CAME_EN_B, 0);

	/* BEE01_AMP_EN					GPIO_6_7 */
	gpio_request(BEE01_AMP_EN, "amp_en");
	gpio_direction_output(BEE01_AMP_EN, 0);

	/* BEE01_LCD_BL_EN				GPIO_6_9 */
	gpio_request(BEE01_LCD_BL_EN, "lcd_bl_en");
//	gpio_direction_output(BEE01_LCD_BL_EN, 0);
	gpio_direction_output(BEE01_LCD_BL_EN, 1);

#ifndef WS1_BOARD
	/* BEE01_CHG_LED_B				GPIO_7_11 */
	gpio_request(BEE01_CHG_LED_B, "chg_led_b");
	gpio_direction_output(BEE01_CHG_LED_B, 0);

	/* BEE01_BAT_CEN_B				GPIO_7_12 */
	gpio_request(BEE01_BAT_CEN_B, "bat_cen_b");
	gpio_direction_output(BEE01_BAT_CEN_B, 0);
#endif //WS1_BOARD

	/* BEE01_TP225					GPIO_1_3 */
	gpio_request(BEE01_TP225, "tp225");
	gpio_direction_output(BEE01_TP225, 0);

	/* BEE01_TP227					GPIO_1_6 */
	gpio_request(BEE01_TP227, "tp227");
	gpio_direction_output(BEE01_TP227, 0);

	/* BEE01_TP223					GPIO_1_7 */
	gpio_request(BEE01_TP223, "tp223");
	gpio_direction_output(BEE01_TP223, 0);

	/* BEE01_TP253					GPIO_1_19 */
	gpio_request(BEE01_TP253, "tp253");
	gpio_direction_output(BEE01_TP253, 0);

#ifndef WS1_BOARD
	/* BEE01_TP166					GPIO_1_29 */
	gpio_request(BEE01_TP166, "tp166");
	gpio_direction_output(BEE01_TP166, 0);
#endif //WS1_BOARD

	/* BEE01_TP179					GPIO_1_31 */
	gpio_request(BEE01_TP179, "tp179");
	gpio_direction_output(BEE01_TP179, 0);

#if 0
	/* BEE01_TP180					GPIO_1_22 */
	gpio_request(BEE01_TP180, "tp180");
	gpio_direction_output(BEE01_TP180, 0);
#endif

	/* BEE01_TP181					GPIO_1_25 */
	gpio_request(BEE01_TP181, "tp181");
	gpio_direction_output(BEE01_TP181, 0);

	/* BEE01_TP182					GPIO_1_23 */
	gpio_request(BEE01_TP182, "tp182");
	gpio_direction_output(BEE01_TP182, 0);

	/* BEE01_TP226					GPIO_1_28 */
	gpio_request(BEE01_TP226, "tp226");
	gpio_direction_output(BEE01_TP226, 0);

	/* BEE01_TP234					GPIO_1_26 */
	gpio_request(BEE01_TP234, "tp234");
	gpio_direction_output(BEE01_TP234, 0);

#if 0
	/* BEE01_TP240					GPIO_2_6 */
	gpio_request(BEE01_TP240, "tp240");
	gpio_direction_output(BEE01_TP240, 0);
#else
	/* BEE01_PCB_VER					GPIO_2_6 */
	gpio_request(BEE01_PCB_VER, "pcb_ver");
	gpio_direction_input(BEE01_PCB_VER);
#endif

#if 0
	/* BEE01_TP241					GPIO_2_7 */
	gpio_request(BEE01_TP241, "tp241");
	gpio_direction_output(BEE01_TP241, 0);
#else
	/* BEE01_PCB_VER2				GPIO_2_7 */
	gpio_request(BEE01_PCB_VER2, "pcb_ver2");
	gpio_direction_input(BEE01_PCB_VER2);
#endif

	/* BEE01_TP212					GPIO_2_23 */
	gpio_request(BEE01_TP212, "tp212");
	gpio_direction_output(BEE01_TP212, 0);

	/* BEE01_TP263					GPIO_2_25 */
	gpio_request(BEE01_TP263, "tp263");
	gpio_direction_output(BEE01_TP263, 0);

	/* BEE01_TP242					GPIO_3_17 */
	gpio_request(BEE01_TP242, "tp242");
	gpio_direction_output(BEE01_TP242, 0);

	/* BEE01_TP211					GPIO_3_19 */
	gpio_request(BEE01_TP211, "tp211");
	gpio_direction_output(BEE01_TP211, 0);

	/* BEE01_TP243					GPIO_3_20 */
	gpio_request(BEE01_TP243, "tp243");
	gpio_direction_output(BEE01_TP243, 0);

#ifndef WS1_BOARD
	/* BEE01_TP170					GPIO_3_30 */
	gpio_request(BEE01_TP170, "tp170");
	gpio_direction_output(BEE01_TP170, 0);
#endif //WS1_BOARD

	/* BEE01_TP244					GPIO_3_31 */
	gpio_request(BEE01_TP244, "tp244");
	gpio_direction_output(BEE01_TP244, 0);

#ifdef WS1_BOARD
	/* BEE01_TP219					GPIO_4_14 */
	gpio_request(BEE01_TP219, "tp219");
	gpio_direction_output(BEE01_TP219, 0);

	/* BEE01_TP220					GPIO_4_15 */
	gpio_request(BEE01_TP220, "tp220");
	gpio_direction_output(BEE01_TP220, 0);
#endif //WS1_BOARD

	/* BEE01_TP279					GPIO_4_16 */
	gpio_request(BEE01_TP279, "tp279");
	gpio_direction_output(BEE01_TP279, 0);

#ifdef WS1_BOARD
	/* BEE01_TP275					GPIO_4_18 */
	gpio_request(BEE01_TP275, "tp275");
	gpio_direction_output(BEE01_TP275, 0);

	/* BEE01_TP276					GPIO_4_19 */
	gpio_request(BEE01_TP276, "tp276");
	gpio_direction_output(BEE01_TP276, 0);

	/* BEE01_TP277					GPIO_4_20 */
	gpio_request(BEE01_TP277, "tp277");
	gpio_direction_output(BEE01_TP277, 0);

	/* BEE01_TP278					GPIO_4_17 */
	gpio_request(BEE01_TP278, "tp278");
	gpio_direction_output(BEE01_TP278, 0);

	/* BEE01_TP267					GPIO_4_21 */
	gpio_request(BEE01_TP267, "tp267");
	gpio_direction_output(BEE01_TP267, 0);

	/* BEE01_TP268					GPIO_4_22 */
	gpio_request(BEE01_TP268, "tp268");
	gpio_direction_output(BEE01_TP268, 0);

	/* BEE01_TP269					GPIO_4_23 */
	gpio_request(BEE01_TP269, "tp269");
	gpio_direction_output(BEE01_TP269, 0);

	/* BEE01_TP270					GPIO_4_24 */
	gpio_request(BEE01_TP270, "tp270");
	gpio_direction_output(BEE01_TP270, 0);

	/* BEE01_TP271					GPIO_4_25 */
	gpio_request(BEE01_TP271, "tp271");
	gpio_direction_output(BEE01_TP271, 0);

	/* BEE01_TP272					GPIO_4_26 */
	gpio_request(BEE01_TP272, "tp272");
	gpio_direction_output(BEE01_TP272, 0);

	/* BEE01_TP273					GPIO_4_27 */
	gpio_request(BEE01_TP273, "tp273");
	gpio_direction_output(BEE01_TP273, 0);

	/* BEE01_TP274					GPIO_4_28 */
	gpio_request(BEE01_TP274, "tp274");
	gpio_direction_output(BEE01_TP274, 0);

	/* BEE01_TP153					GPIO_4_29 */
	gpio_request(BEE01_TP153, "tp153");
	gpio_direction_output(BEE01_TP153, 0);
#endif //WS1_BOARD

	/* BEE01_TP218					GPIO_5_20 */
	gpio_request(BEE01_TP218, "tp218");
	gpio_direction_output(BEE01_TP218, 0);

	/* BEE01_TP251					GPIO_6_18 */
	gpio_request(BEE01_TP251, "tp251");
	gpio_direction_output(BEE01_TP251, 0);

	/* BEE01_TP252					GPIO_6_17 */
	gpio_request(BEE01_TP252, "tp252");
	gpio_direction_output(BEE01_TP252, 0);

	/* BEE01_TP213					GPIO_6_31 */
	gpio_request(BEE01_TP213, "tp213");
	gpio_direction_output(BEE01_TP213, 0);

#ifdef WS1_BOARD
	/* BEE01_TP249					GPIO_7_1 */
	gpio_request(BEE01_TP249, "tp249");
	gpio_direction_output(BEE01_TP249, 0);
#else
	/* BEE01_TS_PWR_EN					GPIO_3_24 */
	gpio_request(BEE01_TS_PWR_EN, "ts_pwr_en");
	gpio_direction_output(BEE01_TS_PWR_EN, 1);
#endif

	/* BEE01_TP250					GPIO_7_0 */
	gpio_request(BEE01_TP250, "tp250");
	gpio_direction_output(BEE01_TP250, 0);

#ifdef WS1_BOARD
	/* BEE01_TP224					GPIO_7_12 */
	gpio_request(BEE01_TP224, "tp224");
	gpio_direction_output(BEE01_TP224, 0);
#endif //WS1_BOARD


	/* BEE01_EIM_D16_OPEN			GPIO_3_16 */
	gpio_request(BEE01_EIM_D16_OPEN, "eim_d16_open");
	gpio_direction_output(BEE01_EIM_D16_OPEN, 0);

	/* BEE01_EIM_D18_OPEN			GPIO_3_18 */
	gpio_request(BEE01_EIM_D18_OPEN, "eim_d18_open");
	gpio_direction_output(BEE01_EIM_D18_OPEN, 0);

	/* BEE01_KEY_ROW2_OPEN			GPIO_4_11 */
	gpio_request(BEE01_KEY_ROW2_OPEN, "key_row2_open");
	gpio_direction_output(BEE01_KEY_ROW2_OPEN, 0);

#ifndef WS1_BOARD
	/* BEE01_DI0_PIN15_OPEN			GPIO_4_17 */
	gpio_request(BEE01_DI0_PIN15_OPEN, "di0_pin15_open");
	gpio_direction_output(BEE01_DI0_PIN15_OPEN, 0);

	/* BEE01_DI0_PIN2_OPEN			GPIO_4_18 */
	gpio_request(BEE01_DI0_PIN2_OPEN, "di0_pin2_open");
	gpio_direction_output(BEE01_DI0_PIN2_OPEN, 0);

	/* BEE01_DI0_PIN3_OPEN			GPIO_4_19 */
	gpio_request(BEE01_DI0_PIN3_OPEN, "di0_pin3_open");
	gpio_direction_output(BEE01_DI0_PIN3_OPEN, 0);

	/* BEE01_DI0_PIN4_OPEN			GPIO_4_20 */
	gpio_request(BEE01_DI0_PIN4_OPEN, "di0_pin4_open");
	gpio_direction_output(BEE01_DI0_PIN4_OPEN, 0);

	/* BEE01_DSP0_DAT0_OPEN			GPIO_4_21 */
	gpio_request(BEE01_DSP0_DAT0_OPEN, "dsp0_dat0_open");
	gpio_direction_output(BEE01_DSP0_DAT0_OPEN, 0);

	/* BEE01_DSP0_DAT1_OPEN			GPIO_4_22 */
	gpio_request(BEE01_DSP0_DAT1_OPEN, "dsp0_dat1_open");
	gpio_direction_output(BEE01_DSP0_DAT1_OPEN, 0);

	/* BEE01_DSP0_DAT2_OPEN			GPIO_4_23 */
	gpio_request(BEE01_DSP0_DAT2_OPEN, "dsp0_dat2_open");
	gpio_direction_output(BEE01_DSP0_DAT2_OPEN, 0);

	/* BEE01_DSP0_DAT3_OPEN			GPIO_4_24 */
	gpio_request(BEE01_DSP0_DAT3_OPEN, "dsp0_dat3_open");
	gpio_direction_output(BEE01_DSP0_DAT3_OPEN, 0);

	/* BEE01_DSP0_DAT4_OPEN			GPIO_4_25 */
	gpio_request(BEE01_DSP0_DAT4_OPEN, "dsp0_dat4_open");
	gpio_direction_output(BEE01_DSP0_DAT4_OPEN, 0);

	/* BEE01_DSP0_DAT5_OPEN			GPIO_4_26 */
	gpio_request(BEE01_DSP0_DAT5_OPEN, "dsp0_dat5_open");
	gpio_direction_output(BEE01_DSP0_DAT5_OPEN, 0);

	/* BEE01_DSP0_DAT6_OPEN			GPIO_4_27 */
	gpio_request(BEE01_DSP0_DAT6_OPEN, "dsp0_dat6_open");
	gpio_direction_output(BEE01_DSP0_DAT6_OPEN, 0);

	/* BEE01_DSP0_DAT7_OPEN			GPIO_4_28 */
	gpio_request(BEE01_DSP0_DAT7_OPEN, "dsp0_dat7_open");
	gpio_direction_output(BEE01_DSP0_DAT7_OPEN, 0);

	/* BEE01_DSP0_DAT8_OPEN			GPIO_4_29 */
	gpio_request(BEE01_DSP0_DAT8_OPEN, "dsp0_dat8_open");
	gpio_direction_output(BEE01_DSP0_DAT8_OPEN, 0);
#endif //WS1_BOARD

	/* BEE01_DISP0_DAT9_OPEN		GPIO_4_30 */
	gpio_request(BEE01_DISP0_DAT9_OPEN, "disp0_dat9_open");
	gpio_direction_output(BEE01_DISP0_DAT9_OPEN, 0);

	/* BEE01_DISP0_DAT10_OPEN		GPIO_4_31 */
	gpio_request(BEE01_DISP0_DAT10_OPEN, "disp0_dat10_open");
	gpio_direction_output(BEE01_DISP0_DAT10_OPEN, 0);

	/* BEE01_DISP0_DAT11_OPEN		GPIO_5_5 */
	gpio_request(BEE01_DISP0_DAT11_OPEN, "disp0_dat11_open");
	gpio_direction_output(BEE01_DISP0_DAT11_OPEN, 0);

	/* BEE01_DISP0_DAT12_OPEN		GPIO_5_6 */
	gpio_request(BEE01_DISP0_DAT12_OPEN, "disp0_dat12_open");
	gpio_direction_output(BEE01_DISP0_DAT12_OPEN, 0);

	/* BEE01_DISP0_DAT13_OPEN		GPIO_5_7 */
	gpio_request(BEE01_DISP0_DAT13_OPEN, "disp0_dat13_open");
	gpio_direction_output(BEE01_DISP0_DAT13_OPEN, 0);

	/* BEE01_DISP0_DAT14_OPEN		GPIO_5_8 */
	gpio_request(BEE01_DISP0_DAT14_OPEN, "disp0_dat14_open");
	gpio_direction_output(BEE01_DISP0_DAT14_OPEN, 0);

	/* BEE01_DISP0_DAT15_OPEN		GPIO_5_9 */
	gpio_request(BEE01_DISP0_DAT15_OPEN, "disp0_dat15_open");
	gpio_direction_output(BEE01_DISP0_DAT15_OPEN, 0);

	/* BEE01_DISP0_DAT16_OPEN		GPIO_5_10 */
	gpio_request(BEE01_DISP0_DAT16_OPEN, "disp0_dat16_open");
	gpio_direction_output(BEE01_DISP0_DAT16_OPEN, 0);

	/* BEE01_DISP0_DAT17_OPEN		GPIO_5_11 */
	gpio_request(BEE01_DISP0_DAT17_OPEN, "disp0_dat17_open");
	gpio_direction_output(BEE01_DISP0_DAT17_OPEN, 0);

	/* BEE01_DISP0_DAT18_OPEN		GPIO_5_12 */
	gpio_request(BEE01_DISP0_DAT18_OPEN, "disp0_dat18_open");
	gpio_direction_output(BEE01_DISP0_DAT18_OPEN, 0);

	/* BEE01_DISP0_DAT19_OPEN		GPIO_5_13 */
	gpio_request(BEE01_DISP0_DAT19_OPEN, "disp0_dat19_open");
	gpio_direction_output(BEE01_DISP0_DAT19_OPEN, 0);

	/* BEE01_DISP0_DAT20_OPEN		GPIO_5_14 */
	gpio_request(BEE01_DISP0_DAT20_OPEN, "disp0_dat20_open");
	gpio_direction_output(BEE01_DISP0_DAT20_OPEN, 0);

	/* BEE01_DISP0_DAT21_OPEN		GPIO_5_15 */
	gpio_request(BEE01_DISP0_DAT21_OPEN, "disp0_dat21_open");
	gpio_direction_output(BEE01_DISP0_DAT21_OPEN, 0);

	/* BEE01_DISP0_DAT22_OPEN		GPIO_5_16 */
	gpio_request(BEE01_DISP0_DAT22_OPEN, "disp0_dat22_open");
	gpio_direction_output(BEE01_DISP0_DAT22_OPEN, 0);

	/* BEE01_DISP0_DAT23_OPEN		GPIO_5_17 */
	gpio_request(BEE01_DISP0_DAT23_OPEN, "disp0_dat23_open");
	gpio_direction_output(BEE01_DISP0_DAT23_OPEN, 0);

	/* BEE01_RGMII_TXC_OPEN			GPIO_6_19 */
	gpio_request(BEE01_RGMII_TXC_OPEN, "rgmii_txc_open");
	gpio_direction_output(BEE01_RGMII_TXC_OPEN, 0);

	/* BEE01_RGMII_TD0_OPEN			GPIO_6_20 */
	gpio_request(BEE01_RGMII_TD0_OPEN, "rgmii_td0_open");
	gpio_direction_output(BEE01_RGMII_TD0_OPEN, 0);

	/* BEE01_RGMII_TD1_OPEN			GPIO_6_21 */
	gpio_request(BEE01_RGMII_TD1_OPEN, "rgmii_td1_open");
	gpio_direction_output(BEE01_RGMII_TD1_OPEN, 0);

	/* BEE01_RGMII_TD2_OPEN			GPIO_6_22 */
	gpio_request(BEE01_RGMII_TD2_OPEN, "rgmii_td2_open");
	gpio_direction_output(BEE01_RGMII_TD2_OPEN, 0);

	/* BEE01_RGMII_TD3_OPEN			GPIO_6_23 */
	gpio_request(BEE01_RGMII_TD3_OPEN, "rgmii_td3_open");
	gpio_direction_output(BEE01_RGMII_TD3_OPEN, 0);

	/* BEE01_RGMII_TX_CTL_OPEN		GPIO_6_26 */
	gpio_request(BEE01_RGMII_TX_CTL_OPEN, "rgmii_tx_ctl_open");
	gpio_direction_output(BEE01_RGMII_TX_CTL_OPEN, 0);

	/* BEE01_RGMII_RXC_OPEN			GPIO_6_30 */
	gpio_request(BEE01_RGMII_RXC_OPEN, "rgmii_rxc_open");
	gpio_direction_output(BEE01_RGMII_RXC_OPEN, 0);

	/* BEE01_RGMII_RD0_OPEN			GPIO_6_25 */
	gpio_request(BEE01_RGMII_RD0_OPEN, "rgmii_rd0_open");
	gpio_direction_output(BEE01_RGMII_RD0_OPEN, 0);

	/* BEE01_RGMII_RD1_OPEN			GPIO_6_27 */
	gpio_request(BEE01_RGMII_RD1_OPEN, "rgmii_rd1_open");
	gpio_direction_output(BEE01_RGMII_RD1_OPEN, 0);

	/* BEE01_RGMII_RD2_OPEN			GPIO_6_28 */
	gpio_request(BEE01_RGMII_RD2_OPEN, "rgmii_rd2_open");
	gpio_direction_output(BEE01_RGMII_RD2_OPEN, 0);

	/* BEE01_RGMII_RD3OPEN			GPIO_6_29 */
	gpio_request(BEE01_RGMII_RD3_OPEN, "rgmii_rd3_open");
	gpio_direction_output(BEE01_RGMII_RD3_OPEN, 0);

	/* BEE01_RGMII_RX_CTL_OPEN		GPIO_6_24 */
	gpio_request(BEE01_RGMII_RX_CTL_OPEN, "rgmii_rx_ctl_open");
	gpio_direction_output(BEE01_RGMII_RX_CTL_OPEN, 0);

#ifdef WS1_BOARD
	/* BEE01_GPIO_16_OPEN			GPIO_7_11 */
	gpio_request(BEE01_GPIO_16_OPEN, "rgmii_gpio_16_open");
	gpio_direction_output(BEE01_GPIO_16_OPEN, 0);
#endif //WS1_BOARD
	
	/* set DOK gpio input */
	gpio_request(BEE01_DOK_B, "max8903-DOK");
	gpio_direction_input(BEE01_DOK_B);

	/* set UOK gpio input */
	gpio_request(BEE01_UOK_B, "max8903-UOK");
	gpio_direction_input(BEE01_UOK_B);
}

extern unsigned int system_rev;
void mx6_bee01_set_revision(int version)
{
	if(!version)
		return;

	if(version > 15)
		return;

	system_rev &= ~0x0f00;
	system_rev |= (version - 1) << 8;
	printk("system_rev %08x\n",system_rev);
}

/*!
 * Board specific initialization.
 */
static void __init mx6_bee01_board_init(void)
{
	int i;
	int ret;
	struct clk *clko, *clko2;
	struct clk *new_parent;
	int rate;
	int version;

	if (cpu_is_mx6q())
		mxc_iomux_v3_setup_multiple_pads(mx6q_bee01_pads,
			ARRAY_SIZE(mx6q_bee01_pads));
	else if (cpu_is_mx6dl()) {
		mxc_iomux_v3_setup_multiple_pads(mx6dl_bee01_pads,
			ARRAY_SIZE(mx6dl_bee01_pads));
	}

	clko2 = clk_get(NULL, "clko2_clk");
	if (IS_ERR(clko2))
		pr_err("can't get CLKO2 clock.\n");

	new_parent = clk_get(NULL, "osc_clk");
	if (!IS_ERR(new_parent)) {
		clk_set_parent(clko2, new_parent);
		clk_put(new_parent);
	}
	rate = clk_round_rate(clko2, 24000000);

	clk_set_rate(clko2, rate);
	clk_enable(clko2);

	/* Camera and audio use osc clock */
	clko = clk_get(NULL, "clko_clk");
	if (!IS_ERR(clko))
		clk_set_parent(clko, clko2);

	clk_enable(clko);

	mx6_bee01_board_io_init();

	version = mx6_bee01_get_PCB_version();
	mx6_bee01_set_revision(version);

	gpio_request(BEE01_CODEC_PWR_EN, "codec_pwr_en");
	gpio_direction_output(BEE01_CODEC_PWR_EN, 1);
	mx6_bee01_sensor_power(1);
	mx6_bee01_wifi_init();

	/* Enable Aux_5V */
	gpio_request(BEE01_AUX_5V_EN, "aux_5v_en");
	gpio_direction_output(BEE01_AUX_5V_EN, 1);
	gpio_set_value(BEE01_AUX_5V_EN, 1);

	gpio_set_value(BEE01_DISP0_PWR_EN, 1);

/*test*/
	gpio_set_value(BEE01_AMP_EN, 1);
/****/

	gp_reg_id = bee01_dvfscore_data.reg_id;
	soc_reg_id = bee01_dvfscore_data.soc_id;
	mx6q_bee01_init_uart();
	imx6x_add_ram_console();

	imx6q_add_ipuv3(0, &ipu_data[0]);
	if (cpu_is_mx6q()) {
		imx6q_add_ipuv3(1, &ipu_data[1]);
		for (i = 0; i < 4 && i < ARRAY_SIZE(bee01_fb_data); i++)
			imx6q_add_ipuv3fb(i, &bee01_fb_data[i]);
	} else
		for (i = 0; i < 2 && i < ARRAY_SIZE(bee01_fb_data); i++)
			imx6q_add_ipuv3fb(i, &bee01_fb_data[i]);

	imx6q_add_vdoa();
	imx6q_add_ldb(&ldb_data);
	imx6q_add_v4l2_output(0);
	imx6q_add_v4l2_capture(0, &capture_data[0]);
	imx6q_add_imx_snvs_rtc();

//	if (1 == caam_enabled)
		imx6q_add_imx_caam();

	mx6q_csi0_io_init();

//	imx6q_add_device_gpio_leds();

	imx6q_add_imx_i2c(0, &mx6q_bee01_i2c_data);
	imx6q_add_imx_i2c(1, &mx6q_bee01_i2c_data);
	i2c_register_board_info(1, mxc_i2c1_board_info,
			ARRAY_SIZE(mxc_i2c1_board_info));
	i2c_register_board_info(0, mxc_i2c0_board_info,
			ARRAY_SIZE(mxc_i2c0_board_info));
	ret = gpio_request(BEE01_PMIC_INT_B, "pmic-int");
	if (ret) {
		printk(KERN_ERR"request pmic-int error!!\n");
		return;
	} else {
		gpio_direction_input(BEE01_PMIC_INT_B);
		mx6q_bee01_init_pfuze100(BEE01_PMIC_INT_B);
	}
	imx6q_add_anatop_thermal_imx(1, &mx6q_bee01_anatop_thermal_data);
	imx6q_add_pm_imx(0, &mx6q_bee01_pm_data);

	/* Move sd4 to first because sd4 connect to emmc.
	   Mfgtools want emmc is mmcblk0 and other sd card is mmcblk1.
	*/
	imx6q_add_sdhci_usdhc_imx(3, &mx6q_bee01_sd4_data);
	imx6q_add_sdhci_usdhc_imx(1, &mx6q_bee01_sd2_data);
	imx6q_add_sdhci_usdhc_imx(2, &mx6q_bee01_sd3_data);
	imx_add_viv_gpu(&imx6_gpu_data, &imx6q_gpu_pdata);
	imx6q_bee01_init_usb();
	imx6q_add_vpu();
	imx6q_init_audio(rate);
	platform_device_register(&bee01_vmmc_reg_devices);
	imx_asrc_data.asrc_core_clk = clk_get(NULL, "asrc_clk");
	imx_asrc_data.asrc_audio_clk = clk_get(NULL, "asrc_serial_clk");
	imx6q_add_asrc(&imx_asrc_data);

	imx6q_add_mxc_pwm_pdata(0, &mxc_pwm_data);
	imx6q_add_mxc_pwm_backlight(0, &mx6_bee01_pwm_backlight_data);

	imx6q_add_otp();
	imx6q_add_viim();
	imx6q_add_imx2_wdt(0, NULL);
	imx6q_add_dma();

	imx6q_add_dvfs_core(&bee01_dvfscore_data);

	if (imx_ion_data.heaps[0].size)
		imx6q_add_ion(0, &imx_ion_data,
			sizeof(imx_ion_data) + sizeof(struct ion_platform_heap));

	imx6q_add_device_buttons();

	if (cpu_is_mx6dl()) {
		imx6dl_add_imx_pxp();
		imx6dl_add_imx_pxp_client();
	}

	/* Register charger chips */
	platform_device_register(&bee01_max8903_charger_1);
	pm_power_off = mx6_snvs_poweroff;
	imx6q_add_busfreq();

	imx6_add_armpmu();
	imx6q_add_perfmon(0);
	imx6q_add_perfmon(1);
	imx6q_add_perfmon(2);

	platform_device_register(&bee01_wifi_reset);
}

extern void __iomem *twd_base;
static void __init mx6_bee01_timer_init(void)
{
	struct clk *uart_clk;
#ifdef CONFIG_LOCAL_TIMERS
	twd_base = ioremap(LOCAL_TWD_ADDR, SZ_256);
	BUG_ON(!twd_base);
#endif
	mx6_clocks_init(32768, 24000000, 0, 0);

	uart_clk = clk_get_sys("imx-uart.0", NULL);
	early_console_setup(UART1_BASE_ADDR, uart_clk);
}

static struct sys_timer mx6_bee01_timer = {
	.init   = mx6_bee01_timer_init,
};

static void __init mx6q_bee01_reserve(void)
{
	phys_addr_t phys;
	int i, fb0_reserved = 0, fb_array_size;

	/*
	 * Reserve primary framebuffer memory if its base address
	 * is set by kernel command line.
	 */
	fb_array_size = ARRAY_SIZE(bee01_fb_data);
	if (fb_array_size > 0 && bee01_fb_data[0].res_base[0] &&
	    bee01_fb_data[0].res_size[0]) {
		if (bee01_fb_data[0].res_base[0] > SZ_2G)
			printk(KERN_INFO"UI Performance downgrade with FB phys address %x!\n",
			    bee01_fb_data[0].res_base[0]);
		memblock_reserve(bee01_fb_data[0].res_base[0],
				 bee01_fb_data[0].res_size[0]);
		memblock_remove(bee01_fb_data[0].res_base[0],
				bee01_fb_data[0].res_size[0]);
		bee01_fb_data[0].late_init = true;
		ipu_data[ldb_data.ipu_id].bypass_reset = true;
//		bee01_fb_data[0].late_init = false;
//		ipu_data[ldb_data.ipu_id].bypass_reset = false;
		fb0_reserved = 1;
	}
	for (i = fb0_reserved; i < fb_array_size; i++)
		if (bee01_fb_data[i].res_size[0]) {
			/* Reserve for other background buffer. */
			phys = memblock_alloc_base(bee01_fb_data[i].res_size[0],
						SZ_4K, SZ_2G);
			memblock_remove(phys, bee01_fb_data[i].res_size[0]);
			bee01_fb_data[i].res_base[0] = phys;
		}

#ifdef CONFIG_ANDROID_RAM_CONSOLE
	phys = memblock_alloc_base(SZ_1M, SZ_4K, SZ_1G);
	memblock_remove(phys, SZ_1M);
	memblock_free(phys, SZ_1M);
	ram_console_resource.start = phys;
	ram_console_resource.end   = phys + SZ_1M - 1;
#endif

#if defined(CONFIG_MXC_GPU_VIV) || defined(CONFIG_MXC_GPU_VIV_MODULE)
	if (imx6q_gpu_pdata.reserved_mem_size) {
		phys = memblock_alloc_base(imx6q_gpu_pdata.reserved_mem_size,
					   SZ_4K, SZ_2G);
		memblock_remove(phys, imx6q_gpu_pdata.reserved_mem_size);
		imx6q_gpu_pdata.reserved_mem_base = phys;
	}
#endif

#if defined(CONFIG_ION)
	if (imx_ion_data.heaps[0].size) {
		phys = memblock_alloc(imx_ion_data.heaps[0].size, SZ_4K);
		memblock_remove(phys, imx_ion_data.heaps[0].size);
		imx_ion_data.heaps[0].base = phys;
	}
#endif
}

/*
 * initialize __mach_desc_MX6Q_BEE01 data structure.
 */
MACHINE_START(MX6Q_BEE01, "Freescale i.MX 6Quad/DualLite BEE01 Board")
	/* Maintainer: Freescale Semiconductor, Inc. */
	.boot_params = MX6_PHYS_OFFSET + 0x100,
	.fixup = fixup_mxc_board,
	.map_io = mx6_map_io,
	.init_irq = mx6_init_irq,
	.init_machine = mx6_bee01_board_init,
	.timer = &mx6_bee01_timer,
	.reserve = mx6q_bee01_reserve,
MACHINE_END
