// SPDX-License-Identifier: GPL-2.0
/*
 * rpi-simple-soundcard.c -- ALSA SoC Raspberry Pi soundcard.
 *
 * Copyright (C) 2021 Joel Holdsworth
 *
 * Authors: Joel Holdsworth <joel@airwebreathe.org.uk>
 *
 * Based on code:
 * rpi-simple-soundcard.c
 * by Tim Gover <tim.gover@raspberrypi.org>
 *
 * hifiberry_amp.c, hifiberry_dac.c, rpi-dac.c
 * by Florian Meier <florian.meier@koalo.de>
 *
 * googlevoicehat-soundcard.c
 * by Peter Malkin <petermalkin@google.com>
 *
 * adau1977-adc.c
 * by Andrey Grodzovsky <andrey2805@gmail.com>
 *
 * merus-amp.c
 * by Ariel Muszkat <ariel.muszkat@gmail.com>
 *		Jorgen Kragh Jakobsen <jorgen.kraghjakobsen@infineon.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>

/* Parameters for generic RPI functions */
struct snd_rpi_simple_mod_drvdata {
	struct snd_soc_dai_link *dai;
	const char* card_name;
	unsigned int fixed_bclk_ratio;
};

static struct snd_soc_card snd_rpi_simple_mod = {
	.driver_name  = "rpi-simple-mod",
	.owner        = THIS_MODULE,
	.dai_link     = NULL,
	.num_links    = 1, /* Only a single DAI supported at the moment */
};

static int snd_rpi_simple_mod_init(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_rpi_simple_mod_drvdata *drvdata =
		snd_soc_card_get_drvdata(rtd->card);
	struct snd_soc_dai *cpu_dai = asoc_rtd_to_cpu(rtd, 0);

	if (drvdata->fixed_bclk_ratio > 0)
		return snd_soc_dai_set_bclk_ratio(cpu_dai,
				drvdata->fixed_bclk_ratio);

	return 0;
}

static int snd_rpi_simple_mod_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = asoc_rtd_to_cpu(rtd, 0);
	struct snd_rpi_simple_mod_drvdata *drvdata;
	unsigned int sample_bits;

	drvdata = snd_soc_card_get_drvdata(rtd->card);

	if (drvdata->fixed_bclk_ratio > 0)
		return 0; // BCLK is configured in .init

	/* The simple drivers just set the bclk_ratio to sample_bits * 2 so
	 * hard-code this for now. More complex drivers could just replace
	 * the hw_params routine.
	 */
	sample_bits = snd_pcm_format_physical_width(params_format(params));
	return snd_soc_dai_set_bclk_ratio(cpu_dai, sample_bits * 2);
}

static struct snd_soc_ops snd_rpi_simple_mod_ops = {
	.hw_params = snd_rpi_simple_mod_hw_params,
};

SND_SOC_DAILINK_DEFS(rpi_raw_i2s_codec,
	DAILINK_COMP_ARRAY(COMP_EMPTY()),
	DAILINK_COMP_ARRAY(COMP_CODEC("rpi-raw-i2s-codec", "rpi-raw-i2s-hifi")),
	DAILINK_COMP_ARRAY(COMP_EMPTY()));

static struct snd_soc_dai_link snd_rpi_raw_i2s_codec_dai[] = {
	{
		.name           = "Raspberry Pi Raw I2S Codec",
		.stream_name    = "Raspberry Pi Raw I2S Codec HiFi",
		.dai_fmt        = SND_SOC_DAIFMT_I2S |
					SND_SOC_DAIFMT_NB_NF |
					SND_SOC_DAIFMT_CBS_CFS,
		SND_SOC_DAILINK_REG(rpi_raw_i2s_codec),
	},
};

static struct snd_rpi_simple_mod_drvdata drvdata_rpi_raw_i2s_codec = {
	.card_name = "snd_rpi_raw_i2s_codec",
	.dai       = snd_rpi_raw_i2s_codec_dai,
};

static const struct of_device_id snd_rpi_simple_mod_of_match[] = {
	{ .compatible = "rpi,rpi-simple-soundcard-mod",
		.data = (void *) &drvdata_rpi_raw_i2s_codec },
	{},
};

static int snd_rpi_simple_mod_probe(struct platform_device *pdev)
{
	int ret = 0;
	const struct of_device_id *of_id;

	snd_rpi_simple_mod.dev = &pdev->dev;
	of_id = of_match_node(snd_rpi_simple_mod_of_match, pdev->dev.of_node);

	if (pdev->dev.of_node && of_id->data) {
		struct device_node *i2s_node;
		struct snd_rpi_simple_mod_drvdata *drvdata =
			(struct snd_rpi_simple_mod_drvdata *) of_id->data;
		struct snd_soc_dai_link *dai = drvdata->dai;

		snd_soc_card_set_drvdata(&snd_rpi_simple_mod, drvdata);

		/* More complex drivers might override individual functions */
		if (!dai->init)
			dai->init = snd_rpi_simple_mod_init;
		if (!dai->ops)
			dai->ops = &snd_rpi_simple_mod_ops;

		snd_rpi_simple_mod.name = drvdata->card_name;

		snd_rpi_simple_mod.dai_link = dai;
		i2s_node = of_parse_phandle(pdev->dev.of_node,
				"i2s-controller", 0);
		if (!i2s_node) {
			pr_err("Failed to find i2s-controller DT node\n");
			return -ENODEV;
		}

		dai->cpus->of_node = i2s_node;
		dai->platforms->of_node = i2s_node;
	}

	ret = devm_snd_soc_register_card(&pdev->dev, &snd_rpi_simple_mod);
	if (ret && ret != -EPROBE_DEFER)
		dev_err(&pdev->dev, "Failed to register card %d\n", ret);

	return ret;
}

static struct platform_driver snd_rpi_simple_mod_driver = {
	.driver = {
		.name   = "snd-rpi-simple-soundcard-mod",
		.owner  = THIS_MODULE,
		.of_match_table = snd_rpi_simple_mod_of_match,
	},
	.probe          = snd_rpi_simple_mod_probe,
};
MODULE_DEVICE_TABLE(of, snd_rpi_simple_mod_of_match);

module_platform_driver(snd_rpi_simple_mod_driver);

MODULE_AUTHOR("Joel Holdsworth <joel@airwebreathe.org.uk>");
MODULE_DESCRIPTION("Modified ASoC Raspberry Pi simple soundcard driver");
MODULE_LICENSE("GPL v2");
