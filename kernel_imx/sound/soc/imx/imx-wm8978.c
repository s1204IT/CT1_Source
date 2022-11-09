/*
 * imx-wm8978.c
 *
 * Copyright (C) 2012-2013 Freescale Semiconductor, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/bitops.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/fsl_devices.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/switch.h>
#include <linux/kthread.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/initval.h>
#include <sound/jack.h>
#include <mach/dma.h>
#include <mach/clock.h>
#include <mach/audmux.h>
#include <mach/gpio.h>
#include <asm/mach-types.h>

#include "imx-ssi.h"
#include "../codecs/wm8978.h"

struct imx_priv {
	int sysclk;         /*mclk from the outside*/
	int codec_sysclk;
	int dai_hifi;
	int hp_irq;
	int hp_status;
	int hp_status_now;
	int mic_irq;
	int mic_status;
	int mic_status_now;
	struct platform_device *pdev;
	struct switch_dev sdev;
};
unsigned int sample_format = SNDRV_PCM_FMTBIT_S16_LE;
static struct imx_priv card_priv;
static struct snd_soc_card snd_soc_card_imx;
static struct snd_soc_codec *gcodec;

static int mic_irq_enable;

static struct snd_soc_jack imx_hp_jack;
static struct snd_soc_jack_pin imx_hp_jack_pins[] = {
	{
		.pin = "Headphone Jack",
		.mask = SND_JACK_HEADPHONE,
	},
};
static struct snd_soc_jack_gpio imx_hp_jack_gpio = {
	.name = "headphone detect",
	.report = SND_JACK_HEADPHONE,
	.debounce_time = 150,
	.invert = 0,
};

static struct snd_soc_jack imx_mic_jack;
static struct snd_soc_jack_pin imx_mic_jack_pins[] = {
//	{
//		.pin = "HPMIC",
//		.mask = SND_JACK_MICROPHONE,
//	},
//	{
//		.pin = "Headphone Jack",
//		.mask = SND_JACK_HEADPHONE,
//	},
};
static struct snd_soc_jack_gpio imx_mic_jack_gpio = {
	.name = "micphone detect",
	.report = SND_JACK_HEADSET,
	.debounce_time = 150,
	.invert = 0,
};

static int imx_hifi_hw_params(struct snd_pcm_substream *substream,
				     struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct imx_priv *priv = &card_priv;
	unsigned int channels = params_channels(params);
	int ret = 0;
	u32 dai_format;

	snd_soc_dai_set_sysclk(codec_dai, WM8978_PLL, priv->sysclk, 0);

	dai_format = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
		SND_SOC_DAIFMT_CBM_CFM;

	/* set codec DAI configuration */
	ret = snd_soc_dai_set_fmt(codec_dai, dai_format);
	if (ret < 0)
		return ret;

	/* set i.MX active slot mask */
	snd_soc_dai_set_tdm_slot(cpu_dai,
				 channels == 1 ? 0xfffffffe : 0xfffffffc,
				 channels == 1 ? 0xfffffffe : 0xfffffffc,
				 2, 32);

	dai_format = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_IF |
		SND_SOC_DAIFMT_CBM_CFM;

	/* set cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai, dai_format);
	if (ret < 0)
		return ret;

	return 0;
}


static void imx_resume_event(struct work_struct *wor)
{
	struct imx_priv *priv = &card_priv;
	struct platform_device *pdev = priv->pdev;
	struct mxc_audio_platform_data *plat = pdev->dev.platform_data;
	struct snd_soc_jack *jack;
	int enable;
	int report;

	if (plat->hp_gpio != -1) {
		jack = imx_hp_jack_gpio.jack;

		enable = gpio_get_value_cansleep(imx_hp_jack_gpio.gpio);
		if (imx_hp_jack_gpio.invert)
			enable = !enable;

		if (enable)
			report = imx_hp_jack_gpio.report;
		else
			report = 0;

		priv->hp_status_now = -1;
		report = imx_hp_jack_gpio.jack_status_check();

		snd_soc_jack_report(jack, report, imx_hp_jack_gpio.report);
	}

#if 0
	if (plat->mic_gpio != -1) {
		jack = imx_mic_jack_gpio.jack;

		enable = gpio_get_value_cansleep(imx_mic_jack_gpio.gpio);
		if (imx_mic_jack_gpio.invert)
			enable = !enable;

		if (enable)
			report = imx_mic_jack_gpio.report;
		else
			report = 0;

		snd_soc_jack_report(jack, report, imx_mic_jack_gpio.report);
	}
#endif

	return;
}

static int hp_jack_status_check(void)
{
	struct imx_priv *priv = &card_priv;
	struct platform_device *pdev = priv->pdev;
	struct mxc_audio_platform_data *plat = pdev->dev.platform_data;
	char *envp[3];
	char *buf;
	int  ret = 0;

	if (plat->hp_gpio != -1) {
		priv->hp_status = gpio_get_value(plat->hp_gpio);
		
		if( priv->hp_status == priv->hp_status_now )
		{
			return imx_hp_jack_gpio.jack->status;
		}
		priv->hp_status_now = priv->hp_status;
		
		/* if headphone is inserted, disable speaker */
		if (priv->hp_status != plat->hp_active_low)
			snd_soc_dapm_nc_pin(&gcodec->dapm, "Ext Spk");
		else
			snd_soc_dapm_enable_pin(&gcodec->dapm, "Ext Spk");

//		snd_soc_dapm_sync(&gcodec->dapm);

		buf = kmalloc(32, GFP_ATOMIC);
		if (!buf) {
			pr_err("%s kmalloc failed\n", __func__);
			return -ENOMEM;
		}

		if (priv->hp_status != plat->hp_active_low) {
			if (plat->mic_gpio != -1) {
				priv->mic_status = gpio_get_value(plat->mic_gpio);

				/* if headset is inserted, disable mic */
				if (priv->mic_status != plat->mic_active_low)
				{
					snd_soc_dapm_nc_pin(&gcodec->dapm, "MIC");
					if( priv->sdev.state != 0 )
					{
						switch_set_state(&priv->sdev, 0);
					}
					switch_set_state(&priv->sdev, 1);
					snprintf(buf, 32, "STATE=%d", 1);
					snd_soc_dapm_enable_pin(&gcodec->dapm, "HPMIC");
					ret = imx_hp_jack_gpio.report;
				}
				else
				{
					snd_soc_dapm_nc_pin(&gcodec->dapm, "HPMIC");
					snd_soc_dapm_enable_pin(&gcodec->dapm, "MIC");
					if( priv->sdev.state != 0 )
					{
						switch_set_state(&priv->sdev, 0);
					}
					switch_set_state(&priv->sdev, 2);
					snprintf(buf, 32, "STATE=%d", 2);
					ret = imx_hp_jack_gpio.report;
				}
//				snd_soc_dapm_sync(&gcodec->dapm);
				if (plat->mic_gpio != -1) {
					if( mic_irq_enable == 0 )
					{
						enable_irq(gpio_to_irq(plat->mic_gpio));
						mic_irq_enable = 1;
					}
				}
				priv->mic_status_now = priv->mic_status;
			}
		} else {
			snd_soc_dapm_enable_pin(&gcodec->dapm, "MIC");
//			snd_soc_dapm_sync(&gcodec->dapm);
			switch_set_state(&priv->sdev, 0);
			snprintf(buf, 32, "STATE=%d", 0);
			if (plat->mic_gpio != -1) {
				if( mic_irq_enable == 1 )
				{
					disable_irq_nosync(gpio_to_irq(plat->mic_gpio));
					mic_irq_enable = 0;
				}
			}
			priv->mic_status_now = plat->mic_active_low;
		}

		envp[0] = "NAME=headphone";
		envp[1] = buf;
		envp[2] = NULL;
		kobject_uevent_env(&pdev->dev.kobj, KOBJ_CHANGE, envp);
		kfree(buf);
	}

	return ret;
}

static int mic_jack_status_check(void)
{
	struct imx_priv *priv = &card_priv;
	struct platform_device *pdev = priv->pdev;
	struct mxc_audio_platform_data *plat = pdev->dev.platform_data;
	char *envp[3];
	char *buf;
	int  ret = 0;

	if (plat->hp_gpio != -1) {
		priv->hp_status = gpio_get_value(plat->hp_gpio);

		/* if headphone is inserted, disable speaker */
		if (priv->hp_status != plat->hp_active_low)
			snd_soc_dapm_nc_pin(&gcodec->dapm, "Ext Spk");
		else
			snd_soc_dapm_enable_pin(&gcodec->dapm, "Ext Spk");

//		snd_soc_dapm_sync(&gcodec->dapm);

		buf = kmalloc(32, GFP_ATOMIC);
		if (!buf) {
			pr_err("%s kmalloc failed\n", __func__);
			return -ENOMEM;
		}

		if (priv->hp_status != plat->hp_active_low) {
			if (plat->mic_gpio != -1) {
				priv->mic_status = gpio_get_value(plat->mic_gpio);
				
				if( priv->mic_status == priv->mic_status_now )
				{
					kfree(buf);
					return ret;
				}
				priv->mic_status_now = priv->mic_status;

				/* if headset is inserted, disable mic */
				if (priv->mic_status != plat->mic_active_low)
				{
					snd_soc_dapm_nc_pin(&gcodec->dapm, "MIC");
					if( priv->sdev.state != 0 )
					{
						switch_set_state(&priv->sdev, 0);
					}
					switch_set_state(&priv->sdev, 1);
					snprintf(buf, 32, "STATE=%d", 1);
					snd_soc_dapm_enable_pin(&gcodec->dapm, "HPMIC");
//					ret = imx_hp_jack_gpio.report;
				}
				else
				{
					snd_soc_dapm_nc_pin(&gcodec->dapm, "HPMIC");
					snd_soc_dapm_enable_pin(&gcodec->dapm, "MIC");
					if( priv->sdev.state != 0 )
					{
						switch_set_state(&priv->sdev, 0);
					}
					switch_set_state(&priv->sdev, 2);
					snprintf(buf, 32, "STATE=%d", 2);
//					ret = imx_hp_jack_gpio.report;
				}
//				snd_soc_dapm_sync(&gcodec->dapm);
			}
		}
/*
		else {
			snd_soc_dapm_enable_pin(&gcodec->dapm, "MIC");
			snd_soc_dapm_sync(&gcodec->dapm);
			switch_set_state(&priv->sdev, 0);
			snprintf(buf, 32, "STATE=%d", 0);
		}
*/
		envp[0] = "NAME=headphone";
		envp[1] = buf;
		envp[2] = NULL;
		kobject_uevent_env(&pdev->dev.kobj, KOBJ_CHANGE, envp);
		kfree(buf);
	}

	return ret;
}
/* imx card dapm widgets */
static const struct snd_soc_dapm_widget imx_dapm_widgets[] = {
	SND_SOC_DAPM_HP("Headphone Jack", NULL),
	SND_SOC_DAPM_SPK("Ext Spk", NULL),
	SND_SOC_DAPM_MIC("MIC", NULL),
	SND_SOC_DAPM_MIC("HPMIC", NULL),
};

/* imx machine connections to the codec pins */
static const struct snd_soc_dapm_route audio_map[] = {
	{ "Headphone Jack", NULL, "LHP" },
	{ "Headphone Jack", NULL, "RHP" },

	{ "Ext Spk", NULL, "LSPK" },
	{ "Ext Spk", NULL, "RSPK" },

	{ "RMICN", NULL, "MIC" },
	{ "RMICP", NULL, "MIC" },
	{ "MIC", NULL, "Mic Bias" },

	{ "HPMIC", NULL, "Mic Bias" },
	{ "LMICN", NULL, "HPMIC" },
	{ "LMICP", NULL, "HPMIC" },

};

static ssize_t show_headphone(struct device_driver *dev, char *buf)
{
	struct imx_priv *priv = &card_priv;
	struct platform_device *pdev = priv->pdev;
	struct mxc_audio_platform_data *plat = pdev->dev.platform_data;

	if (plat->hp_gpio == -1)
		return 0;

	/* determine whether hp is plugged in */
	priv->hp_status = gpio_get_value(plat->hp_gpio);

	if (priv->hp_status != plat->hp_active_low)
		strcpy(buf, "headphone\n");
	else
		strcpy(buf, "speaker\n");

	return strlen(buf);
}

static DRIVER_ATTR(headphone, S_IRUGO | S_IWUSR, show_headphone, NULL);

static ssize_t show_amic(struct device_driver *dev, char *buf)
{
	struct imx_priv *priv = &card_priv;
	struct platform_device *pdev = priv->pdev;
	struct mxc_audio_platform_data *plat = pdev->dev.platform_data;

	/* determine whether amic is plugged in */
	priv->mic_status = gpio_get_value(plat->mic_gpio);

	if (priv->mic_status != plat->mic_active_low)
		strcpy(buf, "mic\n");
	else
		strcpy(buf, "hpmic\n");

	return strlen(buf);
}

static DRIVER_ATTR(amic, S_IRUGO | S_IWUSR, show_amic, NULL);

static DECLARE_DELAYED_WORK(resume_hp_event, imx_resume_event);

int imx_hifi_trigger(struct snd_pcm_substream *substream, int cmd)
{
	struct imx_priv *priv = &card_priv;
	struct platform_device *pdev = priv->pdev;
	struct mxc_audio_platform_data *plat = pdev->dev.platform_data;

	if (SNDRV_PCM_TRIGGER_RESUME == cmd) {
		if ((plat->hp_gpio != -1) || (plat->mic_gpio != -1))
			schedule_delayed_work(&resume_hp_event,
				msecs_to_jiffies(200));
	}

	return 0;
}

static int imx_wm8978_init(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_soc_codec *codec = rtd->codec;
	struct imx_priv *priv = &card_priv;
	struct platform_device *pdev = priv->pdev;
	struct mxc_audio_platform_data *plat = pdev->dev.platform_data;
	int ret = 0;

	gcodec = rtd->codec;

	/* Add imx specific widgets */
	snd_soc_dapm_new_controls(&codec->dapm, imx_dapm_widgets,
				  ARRAY_SIZE(imx_dapm_widgets));

	/* Set up imx specific audio path audio_map */
	snd_soc_dapm_add_routes(&codec->dapm, audio_map, ARRAY_SIZE(audio_map));

	snd_soc_dapm_enable_pin(&codec->dapm, "Ext Spk");
	snd_soc_dapm_enable_pin(&codec->dapm, "MIC");

	if (plat->mic_gpio != -1) {
#if 1
		priv->mic_status_now = -1;
		
		imx_mic_jack_gpio.gpio = plat->mic_gpio;
		imx_mic_jack_gpio.jack_status_check = mic_jack_status_check;

		snd_soc_jack_new(codec, "HPMIC", SND_JACK_MICROPHONE,
				&imx_mic_jack);
		snd_soc_jack_add_pins(&imx_mic_jack,
					ARRAY_SIZE(imx_mic_jack_pins),
					imx_mic_jack_pins);
		mic_irq_enable = 1;
		snd_soc_jack_add_gpios(&imx_mic_jack,
					1, &imx_mic_jack_gpio);
//		disable_irq_nosync(gpio_to_irq(plat->mic_gpio));
#endif
		ret = driver_create_file(pdev->dev.driver,
							&driver_attr_amic);
		if (ret < 0) {
			ret = -EINVAL;
			return ret;
		}

//		priv->mic_status = gpio_get_value(plat->mic_gpio);

		/* if headset is inserted, disable mic */
//		if (priv->mic_status != plat->mic_active_low)
//			snd_soc_dapm_nc_pin(&codec->dapm, "MIC");
//		else
//			snd_soc_dapm_enable_pin(&codec->dapm, "MIC");
	} else {
		snd_soc_dapm_nc_pin(&codec->dapm, "HPMIC");
	}

	if (plat->hp_gpio != -1) {
		priv->hp_status_now = -1;
		
		imx_hp_jack_gpio.gpio = plat->hp_gpio;
		imx_hp_jack_gpio.jack_status_check = hp_jack_status_check;

		snd_soc_jack_new(codec, "Headphone Jack", SND_JACK_HEADPHONE,
				&imx_hp_jack);
		snd_soc_jack_add_pins(&imx_hp_jack,
					ARRAY_SIZE(imx_hp_jack_pins),
					imx_hp_jack_pins);
		snd_soc_jack_add_gpios(&imx_hp_jack,
					1, &imx_hp_jack_gpio);

		ret = driver_create_file(pdev->dev.driver,
							&driver_attr_headphone);
		if (ret < 0) {
			ret = -EINVAL;
			return ret;
		}

		priv->hp_status = gpio_get_value(plat->hp_gpio);

		/* if headphone is inserted, disable speaker */
		if (priv->hp_status != plat->hp_active_low)
			snd_soc_dapm_nc_pin(&codec->dapm, "Ext Spk");
		else
			snd_soc_dapm_enable_pin(&codec->dapm, "Ext Spk");
	}

	snd_soc_dapm_sync(&codec->dapm);

	snd_soc_dapm_sync(&codec->dapm);

	return 0;
}

static struct snd_soc_ops imx_hifi_ops = {
	.hw_params = imx_hifi_hw_params,
	.trigger = imx_hifi_trigger,
};

static struct snd_soc_dai_link imx_dai[] = {
	{
		.name = "HiFi",
		.stream_name = "HiFi",
		.codec_dai_name	= "wm8978-hifi",
		.codec_name	= "wm8978.0-001a",
		.cpu_dai_name	= "imx-ssi.1",
		.platform_name	= "imx-pcm-audio.1",
		.init		= imx_wm8978_init,
		.ops		= &imx_hifi_ops,
	},
};

static struct snd_soc_card snd_soc_card_imx = {
	.name		= "wm8978-audio",
	.dai_link	= imx_dai,
	.num_links	= ARRAY_SIZE(imx_dai),
//	.pop_time = 1,		// pop_dbgo—Í—p
};

static int imx_audmux_config(int slave, int master)
{
	unsigned int ptcr, pdcr;
	slave = slave - 1;
	master = master - 1;

	ptcr = MXC_AUDMUX_V2_PTCR_SYN |
		MXC_AUDMUX_V2_PTCR_TFSDIR |
		MXC_AUDMUX_V2_PTCR_TFSEL(master) |
		MXC_AUDMUX_V2_PTCR_TCLKDIR |
		MXC_AUDMUX_V2_PTCR_TCSEL(master);
	pdcr = MXC_AUDMUX_V2_PDCR_RXDSEL(master);
	mxc_audmux_v2_configure_port(slave, ptcr, pdcr);

	ptcr = MXC_AUDMUX_V2_PTCR_SYN;
	pdcr = MXC_AUDMUX_V2_PDCR_RXDSEL(slave);
	mxc_audmux_v2_configure_port(master, ptcr, pdcr);

	return 0;
}

/*
 * This function will register the snd_soc_pcm_link drivers.
 */
static int __devinit imx_wm8978_probe(struct platform_device *pdev)
{

	struct mxc_audio_platform_data *plat = pdev->dev.platform_data;
	struct imx_priv *priv = &card_priv;
	int ret = 0;

	priv->pdev = pdev;

	imx_audmux_config(plat->src_port, plat->ext_port);

	if (plat->init && plat->init()) {
		ret = -EINVAL;
		return ret;
	}

	priv->sysclk = plat->sysclk;

	priv->sdev.name = "h2w";
	ret = switch_dev_register(&priv->sdev);
	if (ret < 0) {
		ret = -EINVAL;
		return ret;
	}

	if (plat->hp_gpio != -1) {
		priv->hp_status = gpio_get_value(plat->hp_gpio);
		if (priv->hp_status != plat->hp_active_low)
		{
			if (plat->mic_gpio != -1)
			{
				priv->mic_status = gpio_get_value(plat->mic_gpio);
				if (priv->mic_status != plat->mic_active_low)
				{
					switch_set_state(&priv->sdev, 1);
				}
				else
				{
					switch_set_state(&priv->sdev, 2);
				}
			}
			else
			{
				switch_set_state(&priv->sdev, 2);
			}
		}
		else
		{
			switch_set_state(&priv->sdev, 0);
		}
	}

	return ret;
}

static int __devexit imx_wm8978_remove(struct platform_device *pdev)
{
	struct mxc_audio_platform_data *plat = pdev->dev.platform_data;
	struct imx_priv *priv = &card_priv;

	plat->clock_enable(0);

	if (plat->finit)
		plat->finit();

	switch_dev_unregister(&priv->sdev);

	if (priv->hp_irq)
		free_irq(priv->hp_irq, priv);
	if (priv->mic_irq)
		free_irq(priv->mic_irq, priv);

	return 0;
}

#ifdef CONFIG_PM
static int imx_wm8978_suspend(struct device *dev)
{
	return 0;
}

static int imx_wm8978_resume(struct device *dev)
{
	struct imx_priv *priv = &card_priv;
	struct platform_device *pdev = priv->pdev;
	struct mxc_audio_platform_data *plat = pdev->dev.platform_data;
	
	if ((plat->hp_gpio != -1) || (plat->mic_gpio != -1))
	{
		schedule_delayed_work(&resume_hp_event, msecs_to_jiffies(200));
	}
	return 0;
}

static const struct dev_pm_ops imx_wm8978_dev_pm_ops = {
	.suspend = imx_wm8978_suspend,
	.resume  = imx_wm8978_resume,
};
#endif

static struct platform_driver imx_wm8978_driver = {
	.probe = imx_wm8978_probe,
	.remove = imx_wm8978_remove,
	.driver = {
		   .name = "imx-wm8978",
		   .owner = THIS_MODULE,
#ifdef CONFIG_PM
		   .pm = &imx_wm8978_dev_pm_ops,
#endif
		   },
};

static struct platform_device *imx_snd_device;

static int __init imx_asoc_init(void)
{
	int ret;

	ret = platform_driver_register(&imx_wm8978_driver);
	if (ret < 0)
		goto exit;

	imx_snd_device = platform_device_alloc("soc-audio", 1);
	if (!imx_snd_device)
		goto err_device_alloc;

	platform_set_drvdata(imx_snd_device, &snd_soc_card_imx);

	ret = platform_device_add(imx_snd_device);

	if (0 == ret)
		goto exit;

	platform_device_put(imx_snd_device);

err_device_alloc:
	platform_driver_unregister(&imx_wm8978_driver);
exit:
	return ret;
}

static void __exit imx_asoc_exit(void)
{
	platform_driver_unregister(&imx_wm8978_driver);
	platform_device_unregister(imx_snd_device);
}

module_init(imx_asoc_init);
module_exit(imx_asoc_exit);

/* Module information */
MODULE_DESCRIPTION("ALSA SoC imx wm8978");
MODULE_LICENSE("GPL");
