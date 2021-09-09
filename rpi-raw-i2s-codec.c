// SPDX-License-Identifier: GPL-2.0
// rpi-raw-i2s-codec.c
//
// Copyright (C) 2021 Joel Holdsworth
// Joel Holdsworth <joel@airwebreathe.org.uk>

#include <linux/module.h>
#include <sound/soc.h>

/*
 * Implements a virtual raw I2S codec
 */

static struct snd_soc_dai_driver rpi_raw_i2s_codec_dai = {
	.name = "rpi-raw-i2s-hifi",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 2,
		.channels_max = 2,
		.rates = SNDRV_PCM_RATE_8000_192000,
		.formats = SNDRV_PCM_FMTBIT_S16_LE |
			SNDRV_PCM_FMTBIT_S24_LE |
			SNDRV_PCM_FMTBIT_S32_LE,
	},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 2,
		.channels_max = 2,
		.rates = SNDRV_PCM_RATE_8000_192000,
		.formats = SNDRV_PCM_FMTBIT_S16_LE |
			SNDRV_PCM_FMTBIT_S24_LE |
			SNDRV_PCM_FMTBIT_S32_LE,
	},
	.symmetric_rates = 1,
};

static const struct snd_soc_component_driver soc_component_dev_rpi_raw_i2s_codec = {
	.idle_bias_on		= 1,
	.use_pmdown_time	= 1,
	.endianness		= 1,
	.non_legacy_dai_naming	= 1,
};

static int rpi_raw_i2s_codec_soc_probe(struct platform_device *pdev)
{
	return devm_snd_soc_register_component(&pdev->dev,
				      &soc_component_dev_rpi_raw_i2s_codec,
				      &rpi_raw_i2s_codec_dai, 1);
}

static const struct of_device_id rpi_raw_i2s_codec_of_match[] = {
	{ .compatible = "rpi,rpi-raw-i2s-codec" },
	{},
};
MODULE_DEVICE_TABLE(of, rpi_raw_i2s_codec_of_match);

static struct platform_driver rpi_raw_i2s_codec_driver = {
	.driver = {
		.name = "rpi-raw-i2s-codec",
		.of_match_table = rpi_raw_i2s_codec_of_match,
	},
	.probe	= rpi_raw_i2s_codec_soc_probe,
};
module_platform_driver(rpi_raw_i2s_codec_driver);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Rasperry Pi Raw I2S I/O Virtual Codec");
MODULE_AUTHOR("Joel Holdsworth <joel@airwebreathe.org.uk>");
