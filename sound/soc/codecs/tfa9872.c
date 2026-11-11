/* SPDX-License-Identifier: GPL-2.0-only
 *
 * Based of tfa9895.c
 * Register definitions taken from tfa98xx kernel driver:
 * Copyright (C) 2014-2020 NXP Semiconductors, All Rights Reserved.
 */

#include <linux/bitfield.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/regmap.h>
#include <sound/soc.h>

#define TFA987X_SYS_CTRL0		0x00
#define TFA987X_SYS_CTRL0_PWDN_MSK	BIT(0)
#define TFA987X_SYS_CTRL0_I2CR_MSK	BIT(1)
#define TFA987X_SYS_CTRL0_AMPE_MSK	BIT(3)
#define TFA987X_SYS_CTRL0_DCDC_MSK	BIT(4)

#define TFA987X_SYS_CTRL1		0x01
#define TFA987X_SYS_CTRL1_MANSCONF_MSK	BIT(2)
#define TFA987X_SYS_CTRL1_MANSAOOSC_MSK	BIT(4)

#define TFA987X_SYS_CTRL2		0x02
#define TFA987X_SYS_CTRL2_AUDFS_MSK	GENMASK(3, 0)
#define TFA987X_SYS_CTRL2_FRACTDEL_MSK	GENMASK(10, 5)

#define TFA987X_REV			0x03
#define TFA987X_CLK_GATING_CTRL		0x05

#define TFA987X_TDM_CFG0		0x20
#define TFA987X_TDM_CFG0_FSBCLKS_MSK	GENMASK(15, 12)
#define TFA987X_TDM_CFG1		0x21
#define TFA987X_TDM_CFG1_NSLOTS_MSK	GENMASK(3,  0)
#define TFA987X_TDM_CFG1_SLOTBITS_MSK	GENMASK(8,  4)
#define TFA987X_TDM_CFG2		0x22
#define TFA987X_TDM_CFG2_SWIDTH_MSK	GENMASK(6,  2)
#define TFA987X_TDM_CFG3		0x23
#define TFA987X_TDM_CFG3_SPKE_MSK	BIT(0)
#define TFA987X_TDM_CFG3_DCE_MSK	BIT(1)
#define TFA987X_TDM_CFG3_CSE_MSK	BIT(3)
#define TFA987X_TDM_CFG3_VSE_MSK	BIT(4)
#define TFA987X_TDM_CFG6		0x26
#define TFA987X_TDM_CFG6_SPKS_MSK	GENMASK(3,  0)
#define TFA987X_TDM_CFG6_DCS_MSK	GENMASK(4,  7)
#define TFA987X_TDM_CFG6_CSS_MSK	GENMASK(15, 12)
#define TFA987X_TDM_CFG7		0x27
#define TFA987X_TDM_CFG7_VSS_MSK	GENMASK(3,  0)

#define TFA987X_AUDIO_CTRL		0x51
#define TFA987X_AUDIO_CTRL_BSSS_MSK	BIT(0)
#define TFA9872_AUDIO_CTRL_INTSMUTE_MSK	BIT(1)
#define TFA987X_AUDIO_CTRL_HPFBYP_MSK	BIT(5)
#define TFA987X_AUDIO_CTRL_DPSA_MSK     BIT(7)

#define TFA987X_AMP_CFG			0x52
#define TFA987X_AMP_CFG_CLIPCTRL_MSK	GENMASK(4, 2)
#define TFA987X_AMP_CFG_GAIN_MSK	GENMASK(12, 5)
#define TFA987X_AMP_CFG_SLOPEE_MSK	BIT(13)
#define TFA987X_AMP_CFG_SLOPESET_MSK	BIT(14)

#define TFA987X_KEY1_PWM_CFG		0x58
#define TFA987X_TDM_CFG8		0x61
#define TFA987X_TDM_CFG8_DCG_MSK	GENMASK(5, 2)
#define TFA987X_TDM_CFG8_SPKG_MSK	GENMASK(9, 6)

/* Total gain depending on INPLEV setting (TFA9873_BF_TDMSPKG) */
#define TFA987X_TDM_SPKG		0x5f
#define TFA987X_TDM_SPKG_GAIN_MSK	GENMASK(9, 6)

#define TFA987X_LOW_NOISE_GAIN1		0x62
#define TFA987X_LOW_NOISE_GAIN2		0x63
#define TFA987X_MODE1_DET1		0x64
#define TFA987X_MODE1_DET1_LPM1MODE_MSK	GENMASK(15, 14)
#define TFA987X_MODE1_DETECTOR2		0x65
#define TFA987X_TDM_SRC			0x68
#define TFA987X_CURSENSE_COMP		0x6f
#define TFA987X_DCDC_CTRL0		0x70
#define TFA9872_DCDC_CTRL0_DCVOS_MSK	GENMASK(2,  0)
#define TFA987X_DCDC_CTRL0_MCC_MSK	GENMASK(6,  3)
#define TFA987X_DCDC_CTRL0_DCDIS_MSK	BIT(7)
#define TFA987X_DCDC_CTRL0_DCIE_MSK	BIT(9)

/* TFA9873 (Bolero) DCDC_CTRL0 layout */
#define TFA9873_DCDC_CTRL0_MCC_MSK	GENMASK(3, 0)
#define TFA9873_DCDC_CTRL0_DCIE_MSK	BIT(6)
#define TFA9873_DCDC_CTRL0_DCSR_MSK	BIT(7)
#define TFA9873_DCDC_CTRL0_DCDIS_MSK	BIT(14)
#define TFA9873_DCDC_CTRL0_DCPWM_MSK	BIT(15)

/* TFA9874 (TFA2) DCDC_CTRL0 layout */
#define TFA9874_DCDC_CTRL0_MCC_MSK	GENMASK(6, 3)
#define TFA9874_DCDC_CTRL0_DCIE_MSK	BIT(9)
#define TFA9874_DCDC_CTRL0_DCSR_MSK	BIT(10)
#define TFA9874_DCDC_CTRL0_DCDIS_MSK	BIT(14)
#define TFA9874_DCDC_CTRL0_DCPWM_MSK	BIT(15)

#define TFA987X_DCDC_CTRL1		0x71
#define TFA987X_DCDC_CTRL4		0x74
#define TFA9872_DCDC_CTRL4_DCVOF_MSK	GENMASK(2,  0)
#define TFA987X_DCDC_CTRL4_DCTRIP_MSK	GENMASK(8,  4)
#define TFA987X_DCDC_CTRL4_DCINT_MSK	BIT(14)
#define TFA987X_DCDC_CTRL5		0x75
#define TFA987X_DCDC_CTRL5_DCTRIP2_MSK	GENMASK(7,  3)
#define TFA987X_DCDC_CTRL5_DCTRIPT_MSK	GENMASK(12, 8)
#define TFA987X_DCDC_CTRL6		0x76
#define TFA987X_DCDC_CTRL6_DCVOF_MSK	GENMASK(8,  3)
#define TFA987X_DCDC_CTRL6_DCVOS_MSK	GENMASK(14, 9)

struct tfa987x_priv {
	struct gpio_desc *spk_sw;
	bool amp_enabled;
};

static int tfa987x_digital_mute(struct snd_soc_dai *codec_dai, int mute, int stream)
{
	struct snd_soc_component *component = codec_dai->component;
	struct tfa987x_priv *priv = dev_get_drvdata(component->dev);

	if (stream != SNDRV_PCM_STREAM_PLAYBACK)
		return 0;

	/*
	 * A disabled amp (bottom speaker in the earpiece profile) must be
	 * put into powerdown, matching the container's powerdown.standby.
	 * The DAPM PWUP widget may have deasserted PWDN just before this.
	 */
	if (!priv || !priv->amp_enabled) {
		snd_soc_component_update_bits(component, TFA987X_SYS_CTRL0,
					      TFA987X_SYS_CTRL0_AMPE_MSK, 0);
		snd_soc_component_update_bits(component, TFA987X_SYS_CTRL0,
					      TFA987X_SYS_CTRL0_PWDN_MSK,
					      TFA987X_SYS_CTRL0_PWDN_MSK);
		return 0;
	}

	snd_soc_component_update_bits(component, TFA987X_SYS_CTRL0,
				      TFA987X_SYS_CTRL0_PWDN_MSK, 0);
	snd_soc_component_update_bits(component, TFA987X_SYS_CTRL0,
				      TFA987X_SYS_CTRL0_AMPE_MSK,
				      mute ? 0 : TFA987X_SYS_CTRL0_AMPE_MSK);

	return 0;
}

static int tfa987x_earpiece_get(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct tfa987x_priv *priv = dev_get_drvdata(component->dev);

	if (!priv || !priv->spk_sw)
		return 0;

	ucontrol->value.integer.value[0] =
		gpiod_get_value_cansleep(priv->spk_sw);

	return 0;
}

static int tfa987x_earpiece_set(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct tfa987x_priv *priv = dev_get_drvdata(component->dev);

	if (!priv || !priv->spk_sw)
		return 0;

	gpiod_set_value_cansleep(priv->spk_sw,
				 !!ucontrol->value.integer.value[0]);

	/*
	 * Vendor taoyao profiles:
	 *   handset: TDMSPKG=0x0b, TDMDCE=0, DCVOS=0x22
	 *   left/stereo: TDMSPKG=0x06, TDMDCE=1, DCVOS=0x2e
	 * These are the receiver tuning values from tfa98xx.cnt.
	 */
	if (ucontrol->value.integer.value[0]) {
		snd_soc_component_update_bits(component, TFA987X_TDM_SPKG,
					      TFA987X_TDM_SPKG_GAIN_MSK,
					      FIELD_PREP(TFA987X_TDM_SPKG_GAIN_MSK,
							 0x0b));
		snd_soc_component_update_bits(component, TFA987X_TDM_CFG3,
					      TFA987X_TDM_CFG3_DCE_MSK, 0);
		snd_soc_component_update_bits(component, TFA987X_DCDC_CTRL6,
					      TFA987X_DCDC_CTRL6_DCVOS_MSK,
					      FIELD_PREP(TFA987X_DCDC_CTRL6_DCVOS_MSK,
							 0x22));
	} else {
		snd_soc_component_update_bits(component, TFA987X_TDM_SPKG,
					      TFA987X_TDM_SPKG_GAIN_MSK,
					      FIELD_PREP(TFA987X_TDM_SPKG_GAIN_MSK,
							 0x06));
		snd_soc_component_update_bits(component, TFA987X_TDM_CFG3,
					      TFA987X_TDM_CFG3_DCE_MSK,
					      TFA987X_TDM_CFG3_DCE_MSK);
		snd_soc_component_update_bits(component, TFA987X_DCDC_CTRL6,
					      TFA987X_DCDC_CTRL6_DCVOS_MSK,
					      FIELD_PREP(TFA987X_DCDC_CTRL6_DCVOS_MSK,
							 0x2e));
	}

	return 0;
}

static int tfa987x_amp_get(struct snd_kcontrol *kcontrol,
			   struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct tfa987x_priv *priv = dev_get_drvdata(component->dev);

	if (priv)
		ucontrol->value.integer.value[0] = priv->amp_enabled;

	return 0;
}

static int tfa987x_amp_set(struct snd_kcontrol *kcontrol,
			   struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct tfa987x_priv *priv = dev_get_drvdata(component->dev);

	if (!priv)
		return 0;

	priv->amp_enabled = !!ucontrol->value.integer.value[0];
	if (priv->amp_enabled) {
		snd_soc_component_update_bits(component, TFA987X_SYS_CTRL0,
					      TFA987X_SYS_CTRL0_PWDN_MSK, 0);
		snd_soc_component_update_bits(component, TFA987X_SYS_CTRL0,
					      TFA987X_SYS_CTRL0_AMPE_MSK,
					      TFA987X_SYS_CTRL0_AMPE_MSK);
	} else {
		snd_soc_component_update_bits(component, TFA987X_SYS_CTRL0,
					      TFA987X_SYS_CTRL0_AMPE_MSK, 0);
		snd_soc_component_update_bits(component, TFA987X_SYS_CTRL0,
					      TFA987X_SYS_CTRL0_PWDN_MSK,
					      TFA987X_SYS_CTRL0_PWDN_MSK);
	}

	return 0;
}

static const struct snd_kcontrol_new tfa987x_snd_controls[] = {
	/* 1 = earpiece (receiver), 0 = external top speaker */
	SOC_SINGLE_EXT("Earpiece Switch", SND_SOC_NOPM, 0, 1, 0,
		       tfa987x_earpiece_get, tfa987x_earpiece_set),
	/* per-amp enable, used to mute the bottom amp for the earpiece */
	SOC_SINGLE_EXT("Amp Switch", SND_SOC_NOPM, 0, 1, 0,
		       tfa987x_amp_get, tfa987x_amp_set),
};

static const struct snd_soc_dapm_widget tfa987x_dapm_widgets[] = {
	SND_SOC_DAPM_OUTPUT("Speaker"),
	SND_SOC_DAPM_OUT_DRV_E("PWUP", TFA987X_SYS_CTRL0, 0, 1, NULL, 0, NULL, 0),
};

static const struct snd_soc_dapm_route tfa987x_dapm_routes[] = {
	{"PWUP", NULL, "HiFi Playback"},
	{"Speaker", NULL, "PWUP"},
};

static const struct snd_soc_component_driver tfa987x_component = {
	.controls		= tfa987x_snd_controls,
	.num_controls		= ARRAY_SIZE(tfa987x_snd_controls),
	.dapm_widgets		= tfa987x_dapm_widgets,
	.num_dapm_widgets	= ARRAY_SIZE(tfa987x_dapm_widgets),
	.dapm_routes		= tfa987x_dapm_routes,
	.num_dapm_routes	= ARRAY_SIZE(tfa987x_dapm_routes),
	.idle_bias_on		= 1,
	.use_pmdown_time	= 1,
	.endianness		= 1,
};

static int tfa987x_hw_params(struct snd_pcm_substream *substream,
			     struct snd_pcm_hw_params *params,
			     struct snd_soc_dai *dai)
{
	struct snd_soc_component *component = dai->component;
	unsigned int fssel;

	switch (params_rate(params)) {
	case 8000:   fssel = 0; break;
	case 11025:  fssel = 1; break;
	case 12000:  fssel = 2; break;
	case 16000:  fssel = 3; break;
	case 22050:  fssel = 4; break;
	case 24000:  fssel = 5; break;
	case 32000:  fssel = 6; break;
	case 44100:  fssel = 7; break;
	case 48000:  fssel = 8; break;
	case 96000:  fssel = 9; break;
	default:
		return -EINVAL;
	}

	snd_soc_component_update_bits(component, TFA987X_SYS_CTRL2,
				      TFA987X_SYS_CTRL2_AUDFS_MSK, fssel);

	return 0;
}

static const struct snd_soc_dai_ops tfa987x_dai_ops = {
	.hw_params = tfa987x_hw_params,
	.mute_stream = tfa987x_digital_mute,
};

static struct snd_soc_dai_driver tfa987x_dai = {
	.name = "tfa987x-hifi",
	.playback = {
		.stream_name	= "HiFi Playback",
		.formats	= SNDRV_PCM_FMTBIT_S16_LE,
		.rates		= SNDRV_PCM_RATE_48000,
		.rate_min	= 48000,
		.rate_max	= 48000,
		.channels_min	= 1,
		.channels_max	= 2,
	},
	.ops = &tfa987x_dai_ops,
};

static const struct regmap_config tfa987x_regmap_config = {
	.reg_bits = 8,
	.val_bits = 16,
	.max_register = 0xff,
};

static inline bool tfa987x_field_fit(u16 mask, u32 val)
{
	return val <= (mask >> __ffs(mask));
}

static inline u16 tfa987x_field_prep(u16 mask, u32 val)
{
	return (val << __ffs(mask)) & mask;
}

struct tfa987x_dcdc_cfg {
	u32 mcc;
	u32 dcvof;
	u32 dcvos;
	u32 dctrip;
	u32 dctrip2;
	u32 dctript;
	bool new_boost;
};

static bool tfa987x_setup_dcdc(struct device *dev, struct regmap *rmap, u16 rev)
{
	struct tfa987x_dcdc_cfg cfg = {};
	u16 mcc_msk, dcie_msk, dcsr_msk = 0, dcddis_msk = 0, dcpwm_msk = 0;
	u16 dcvof_msk, dcvos_msk;
	u16 dcvof_reg = TFA987X_DCDC_CTRL6;
	u16 dcvos_reg = TFA987X_DCDC_CTRL6;
	int i;
	const struct {
		const char *name;
		u32 *value;
	} props[] = {
		{ "max-coil-current", &cfg.mcc },
		{ "first-boost-voltage", &cfg.dcvof },
		{ "second-boost-voltage", &cfg.dcvos },
		{ "first-boost-trip-lvl", &cfg.dctrip },
		{ "second-boost-trip-lvl", &cfg.dctrip2 },
		{ "track-boost-trip-lvl", &cfg.dctript },
	};

	switch (rev & 0xff) {
	case 0x72:
		mcc_msk = TFA987X_DCDC_CTRL0_MCC_MSK;
		dcie_msk = TFA987X_DCDC_CTRL0_DCIE_MSK;
		dcddis_msk = TFA987X_DCDC_CTRL0_DCDIS_MSK;
		dcvof_msk = TFA9872_DCDC_CTRL4_DCVOF_MSK;
		dcvof_reg = TFA987X_DCDC_CTRL4;
		dcvos_msk = TFA9872_DCDC_CTRL0_DCVOS_MSK;
		dcvos_reg = TFA987X_DCDC_CTRL0;
		break;
	case 0x73:
		mcc_msk = TFA9873_DCDC_CTRL0_MCC_MSK;
		dcie_msk = TFA9873_DCDC_CTRL0_DCIE_MSK;
		dcsr_msk = TFA9873_DCDC_CTRL0_DCSR_MSK;
		dcddis_msk = TFA9873_DCDC_CTRL0_DCDIS_MSK;
		dcpwm_msk = TFA9873_DCDC_CTRL0_DCPWM_MSK;
		dcvof_msk = TFA987X_DCDC_CTRL6_DCVOF_MSK;
		dcvos_msk = TFA987X_DCDC_CTRL6_DCVOS_MSK;
		/* taoyao / vendor "RCV" device init */
		cfg.mcc = 9;
		cfg.dctrip = 0x0a;
		cfg.dctript = 0x10;
		cfg.dcvos = 0x2e;
		cfg.new_boost = true;
		break;
	case 0x74:
		mcc_msk = TFA9874_DCDC_CTRL0_MCC_MSK;
		dcie_msk = TFA9874_DCDC_CTRL0_DCIE_MSK;
		dcsr_msk = TFA9874_DCDC_CTRL0_DCSR_MSK;
		dcddis_msk = TFA9874_DCDC_CTRL0_DCDIS_MSK;
		dcpwm_msk = TFA9874_DCDC_CTRL0_DCPWM_MSK;
		dcvof_msk = TFA987X_DCDC_CTRL6_DCVOF_MSK;
		dcvos_msk = TFA987X_DCDC_CTRL6_DCVOS_MSK;
		/* taoyao / vendor "SPK" device init */
		cfg.mcc = 12;
		cfg.dctrip = 0x09;
		cfg.dctript = 0x0d;
		cfg.dcvos = 0x3f;
		cfg.new_boost = true;
		break;
	default:
		return false;
	}

	for (i = 0; i < ARRAY_SIZE(props); i++) {
		int ret;

		ret = of_property_read_u32(dev->of_node, props[i].name,
					   props[i].value);
		if (ret < 0 && ret != -EINVAL)
			return false;
	}

	if (!tfa987x_field_fit(mcc_msk, cfg.mcc) ||
	    !tfa987x_field_fit(dcvof_msk, cfg.dcvof) ||
	    !tfa987x_field_fit(dcvos_msk, cfg.dcvos) ||
	    !tfa987x_field_fit(TFA987X_DCDC_CTRL4_DCTRIP_MSK, cfg.dctrip) ||
	    !tfa987x_field_fit(TFA987X_DCDC_CTRL5_DCTRIP2_MSK, cfg.dctrip2))
		return false;

	if (cfg.dcvof)
		regmap_update_bits(rmap, dcvof_reg, dcvof_msk,
				   tfa987x_field_prep(dcvof_msk, cfg.dcvof));
	if (cfg.dcvos)
		regmap_update_bits(rmap, dcvos_reg, dcvos_msk,
				   tfa987x_field_prep(dcvos_msk, cfg.dcvos));
	if (cfg.dctrip)
		regmap_update_bits(rmap, TFA987X_DCDC_CTRL4,
				   TFA987X_DCDC_CTRL4_DCTRIP_MSK,
				   tfa987x_field_prep(TFA987X_DCDC_CTRL4_DCTRIP_MSK,
						      cfg.dctrip));
	if (cfg.dctrip2)
		regmap_update_bits(rmap, TFA987X_DCDC_CTRL5,
				   TFA987X_DCDC_CTRL5_DCTRIP2_MSK,
				   tfa987x_field_prep(TFA987X_DCDC_CTRL5_DCTRIP2_MSK,
						      cfg.dctrip2));

	if (cfg.new_boost) {
		/*
		 * Vendor device init: DCINT = 0, DCPWM = 0, DCDIS = 0,
		 * DCSR = 1.  DCMCC/DCIE are at different bit positions on
		 * TFA9873 and TFA9874.
		 */
		regmap_update_bits(rmap, TFA987X_DCDC_CTRL4,
				   TFA987X_DCDC_CTRL4_DCINT_MSK, 0);
		regmap_update_bits(rmap, TFA987X_DCDC_CTRL5,
				   TFA987X_DCDC_CTRL5_DCTRIPT_MSK,
				   tfa987x_field_prep(TFA987X_DCDC_CTRL5_DCTRIPT_MSK,
						      cfg.dctript));
	}

	if (cfg.mcc || cfg.new_boost)
		regmap_update_bits(rmap, TFA987X_DCDC_CTRL0,
				   mcc_msk | dcie_msk | dcsr_msk |
				   dcddis_msk | dcpwm_msk,
				   tfa987x_field_prep(mcc_msk, cfg.mcc) |
				   dcie_msk | dcsr_msk);
	return true;
}

static int tfa987x_i2c_probe(struct i2c_client *i2c)
{
	struct device *dev = &i2c->dev;
	struct tfa987x_priv *priv;
	struct regmap *rmap;
	unsigned int rev;
	u32 channel_index = 0;
	int ret;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->amp_enabled = true;
	priv->spk_sw = devm_gpiod_get_optional(dev, "spk-sw", GPIOD_OUT_LOW);
	if (IS_ERR(priv->spk_sw))
		return dev_err_probe(dev, PTR_ERR(priv->spk_sw),
				     "Failed to get spk-sw GPIO\n");

	dev_set_drvdata(dev, priv);

	rmap = devm_regmap_init_i2c(i2c, &tfa987x_regmap_config);
	if (IS_ERR(rmap))
		return PTR_ERR(rmap);

	ret = regmap_read(rmap, TFA987X_REV, &rev);
	if (ret < 0) {
		dev_err(dev, "Failed to read revision register: %d\n", ret);
		return ret;
	}

	switch (rev) {
		case 0x0b73:
		case 0x1b72:
		case 0x2b72:
		case 0x3b72:
		case 0x0c74:
			dev_info(dev, "Chip revision: 0x%04x\n", rev);
			break;
		default:
			dev_err(dev, "Unsupported chip revision: 0x%04x\n", rev);
			return -ENODEV;
	}

	of_property_read_u32(dev->of_node, "sound-channel", &channel_index);

	/* Perform soft reset */
	regmap_write(rmap, TFA987X_SYS_CTRL0, TFA987X_SYS_CTRL0_I2CR_MSK);

	/* Setup DC-DC Converter if we have configuration */
	regmap_update_bits(rmap, TFA987X_SYS_CTRL0, TFA987X_SYS_CTRL0_DCDC_MSK,
			   FIELD_PREP(TFA987X_SYS_CTRL0_DCDC_MSK,
				   (u16) tfa987x_setup_dcdc(dev, rmap, rev)));

	/* Disable DPSA */
	regmap_update_bits(rmap, TFA987X_AUDIO_CTRL,
				 TFA987X_AUDIO_CTRL_DPSA_MSK, 0);

	/* Setup TDM 16 bit 1 slot config */
	regmap_update_bits(rmap, TFA987X_TDM_CFG0,
				 TFA987X_TDM_CFG0_FSBCLKS_MSK,
				 FIELD_PREP(TFA987X_TDM_CFG0_FSBCLKS_MSK, 0));
	regmap_update_bits(rmap, TFA987X_TDM_CFG1,
				 TFA987X_TDM_CFG1_NSLOTS_MSK |
				 TFA987X_TDM_CFG1_SLOTBITS_MSK,
				 FIELD_PREP(TFA987X_TDM_CFG1_NSLOTS_MSK, 1) |
				 FIELD_PREP(TFA987X_TDM_CFG1_SLOTBITS_MSK, 15));
	regmap_update_bits(rmap, TFA987X_TDM_CFG2,
				 TFA987X_TDM_CFG2_SWIDTH_MSK,
				 FIELD_PREP(TFA987X_TDM_CFG2_SWIDTH_MSK, 15));

	/* No current/voltage sense over TDM */
	regmap_update_bits(rmap, TFA987X_TDM_CFG3,
				 TFA987X_TDM_CFG3_SPKE_MSK |
				 TFA987X_TDM_CFG3_DCE_MSK |
				 TFA987X_TDM_CFG3_CSE_MSK |
				 TFA987X_TDM_CFG3_VSE_MSK,
				 TFA987X_TDM_CFG3_SPKE_MSK);
	regmap_update_bits(rmap, TFA987X_TDM_CFG6,
				 TFA987X_TDM_CFG6_SPKS_MSK,
				 FIELD_PREP(TFA987X_TDM_CFG6_SPKS_MSK, channel_index));

	if ((rev & 0xff) == 0x72)
		regmap_update_bits(rmap, TFA987X_MODE1_DET1,
				 TFA987X_MODE1_DET1_LPM1MODE_MSK,
				 FIELD_PREP(TFA987X_MODE1_DET1_LPM1MODE_MSK, 1));

	/* Turn on this thing */
	regmap_update_bits(rmap, TFA987X_SYS_CTRL0,
				 TFA987X_SYS_CTRL0_PWDN_MSK, 0);

	regmap_update_bits(rmap, TFA987X_SYS_CTRL1,
				 TFA987X_SYS_CTRL1_MANSCONF_MSK,
				 TFA987X_SYS_CTRL1_MANSCONF_MSK);

	return devm_snd_soc_register_component(dev, &tfa987x_component,
						    &tfa987x_dai, 1);
}

static const struct of_device_id tfa987x_of_match[] = {
	{ .compatible = "nxp,tfa9872" },
	{ .compatible = "nxp,tfa9874" },
	{ }
};
MODULE_DEVICE_TABLE(of, tfa987x_of_match);

static struct i2c_driver tfa987x_i2c_driver = {
	.driver = {
		.name = "tfa987x",
		.of_match_table = tfa987x_of_match,
	},
	.probe = tfa987x_i2c_probe,
};
module_i2c_driver(tfa987x_i2c_driver);

MODULE_DESCRIPTION("ASoC NXP Semiconductors TFA9872/TFA9874 driver");
MODULE_LICENSE("GPL v2");
