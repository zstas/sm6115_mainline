// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2026 Stanislav Zaikin

#include <linux/backlight.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/regulator/consumer.h>

#include <video/mipi_display.h>

#include <drm/display/drm_dsc.h>
#include <drm/display/drm_dsc_helper.h>
#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>

struct xiaomi_taoyao_csot_nt36672c {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct drm_dsc_config dsc;
	struct regulator_bulk_data *supplies;
	struct gpio_desc *reset_gpio;
};

static const struct regulator_bulk_data xiaomi_taoyao_csot_nt36672c_supplies[] = {
	{ .supply = "vdd" },
	{ .supply = "vddio" },
	{ .supply = "dvdd" },
};

static inline struct xiaomi_taoyao_csot_nt36672c *
to_xiaomi_taoyao_csot_nt36672c(struct drm_panel *panel)
{
	return container_of_const(panel, struct xiaomi_taoyao_csot_nt36672c,
				  panel);
}

static void
xiaomi_taoyao_csot_nt36672c_reset(struct xiaomi_taoyao_csot_nt36672c *ctx)
{
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(11000, 12000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	usleep_range(1000, 2000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(11000, 12000);
}

static int
xiaomi_taoyao_csot_nt36672c_on(struct xiaomi_taoyao_csot_nt36672c *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf0, 0x55, 0xaa, 0x52, 0x08,
				     0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe9, 0x00, 0x00, 0x00, 0x16);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6f, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb9, 0x05, 0xdb, 0x00, 0x7a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0xaa, 0x55, 0xa5, 0x80);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6f, 0x1d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf2, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf0, 0x55, 0xaa, 0x52, 0x08,
				     0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6f, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xca, 0xaa);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6f, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc0, 0xb3);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6f, 0x11);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc0, 0x30);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6f, 0x1c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc0, 0x03, 0x12);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3b, 0x00, 0x14, 0x00, 0x12);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x90, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x91, 0xab, 0x28, 0x00, 0x0c,
				     0xc2, 0x00, 0x03, 0x1c, 0x01, 0x7e, 0x00,
				     0x0f, 0x08, 0xbb, 0x04, 0x3d, 0x10, 0xf0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x03, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_WRITE_MEMORY_START,
				     0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf0, 0x55, 0xaa, 0x52, 0x08,
				     0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc0, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc1, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc2, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc3, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc4, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc5, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc6, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc7, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc8, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcd, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xce, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcf, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd0, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_WRITE_CONTROL_DISPLAY,
				     0x20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf0, 0x55, 0xaa, 0x52, 0x08,
				     0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6f, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb2, 0x08, 0x08);
	mipi_dsi_dcs_set_tear_on_multi(&dsi_ctx, MIPI_DSI_DCS_TEAR_MODE_VBLANK);
	mipi_dsi_dcs_set_column_address_multi(&dsi_ctx, 0x0000, 0x0437);
	mipi_dsi_dcs_set_page_address_multi(&dsi_ctx, 0x0000, 0x095f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf0, 0x55, 0xaa, 0x52, 0x08,
				     0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc3, 0x94, 0x01, 0x8c, 0xd0,
				     0x22, 0x02, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2f, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5f, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf0, 0x55, 0xaa, 0x52, 0x08,
				     0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6f, 0x09);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc0, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf0, 0x55, 0xaa, 0x52, 0x08,
				     0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcc, 0x30);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xce, 0x01);
	mipi_dsi_msleep(&dsi_ctx, 20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcc, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0xaa, 0x55, 0xa5, 0x80);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6f, 0x61);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf3, 0x80);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf0, 0x55, 0xaa, 0x52, 0x08,
				     0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6f, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd2, 0x00, 0x00, 0x11);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6f, 0x06);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd2, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6f, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd2, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6f, 0x09);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd2, 0x00, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf0, 0x55, 0xaa, 0x52, 0x08,
				     0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xce, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x11, 0x00);
	mipi_dsi_msleep(&dsi_ctx, 120);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x29, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf0, 0x55, 0xaa, 0x52, 0x08,
				     0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xce, 0x00);

	return dsi_ctx.accum_err;
}

static int
xiaomi_taoyao_csot_nt36672c_off(struct xiaomi_taoyao_csot_nt36672c *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x28, 0x00);
	mipi_dsi_usleep_range(&dsi_ctx, 10000, 11000);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x10, 0x00);
	mipi_dsi_msleep(&dsi_ctx, 120);

	return dsi_ctx.accum_err;
}

static int xiaomi_taoyao_csot_nt36672c_prepare(struct drm_panel *panel)
{
	struct xiaomi_taoyao_csot_nt36672c *ctx =
		to_xiaomi_taoyao_csot_nt36672c(panel);
	struct device *dev = &ctx->dsi->dev;
	struct drm_dsc_picture_parameter_set pps;
	int ret;

	ret = regulator_bulk_enable(
		ARRAY_SIZE(xiaomi_taoyao_csot_nt36672c_supplies),
		ctx->supplies);
	if (ret < 0) {
		dev_err(dev, "Failed to enable regulators: %d\n", ret);
		return ret;
	}

	xiaomi_taoyao_csot_nt36672c_reset(ctx);

	ret = xiaomi_taoyao_csot_nt36672c_on(ctx);
	if (ret < 0) {
		dev_err(dev, "Failed to initialize panel: %d\n", ret);
		gpiod_set_value_cansleep(ctx->reset_gpio, 1);
		regulator_bulk_disable(
			ARRAY_SIZE(xiaomi_taoyao_csot_nt36672c_supplies),
			ctx->supplies);
		return ret;
	}

	drm_dsc_pps_payload_pack(&pps, &ctx->dsc);

	ret = mipi_dsi_picture_parameter_set(ctx->dsi, &pps);
	if (ret < 0) {
		dev_err(panel->dev, "failed to transmit PPS: %d\n", ret);
		return ret;
	}

	ret = mipi_dsi_compression_mode(ctx->dsi, true);
	if (ret < 0) {
		dev_err(dev, "failed to enable compression mode: %d\n", ret);
		return ret;
	}

	msleep(28); /* wait for the panel to apply the compression settings */

	return 0;
}

static int xiaomi_taoyao_csot_nt36672c_unprepare(struct drm_panel *panel)
{
	struct xiaomi_taoyao_csot_nt36672c *ctx =
		to_xiaomi_taoyao_csot_nt36672c(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = xiaomi_taoyao_csot_nt36672c_off(ctx);
	if (ret < 0)
		dev_err(dev, "Failed to un-initialize panel: %d\n", ret);

	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	regulator_bulk_disable(ARRAY_SIZE(xiaomi_taoyao_csot_nt36672c_supplies),
			       ctx->supplies);

	return 0;
}

static const struct drm_display_mode xiaomi_taoyao_csot_nt36672c_modes[] = {
	{
		/* 1080x2400 @ 60Hz */
		.clock = (1080 + 32 + 32 + 32) * (2400 + 1335 + 32 + 1300) *
			 60 / 1000,
		.hdisplay = 1080,
		.hsync_start = 1080 + 32,
		.hsync_end = 1080 + 32 + 32,
		.htotal = 1080 + 32 + 32 + 32,
		.vdisplay = 2400,
		.vsync_start = 2400 + 1335,
		.vsync_end = 2400 + 1335 + 32,
		.vtotal = 2400 + 1335 + 32 + 1300,
		.width_mm = 68,
		.height_mm = 152,
		.type = DRM_MODE_TYPE_DRIVER,
	},
	{
		/* 1080x2400 @ 120Hz */
		.clock = (1080 + 32 + 16 + 32) * (2400 + 114 + 32 + 80) * 120 /
			 1000,
		.hdisplay = 1080,
		.hsync_start = 1080 + 32,
		.hsync_end = 1080 + 32 + 16,
		.htotal = 1080 + 32 + 16 + 32,
		.vdisplay = 2400,
		.vsync_start = 2400 + 114,
		.vsync_end = 2400 + 114 + 32,
		.vtotal = 2400 + 114 + 32 + 80,
		.width_mm = 68,
		.height_mm = 152,
		.type = DRM_MODE_TYPE_DRIVER,
	},
};

static int
xiaomi_taoyao_csot_nt36672c_get_modes(struct drm_panel *panel,
				      struct drm_connector *connector)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(xiaomi_taoyao_csot_nt36672c_modes); i++) {
		struct drm_display_mode *mode;

		mode = drm_mode_duplicate(connector->dev,
					  &xiaomi_taoyao_csot_nt36672c_modes[i]);
		if (!mode)
			return -ENOMEM;

		mode->type = DRM_MODE_TYPE_DRIVER;
		if (i == 0)
			mode->type |= DRM_MODE_TYPE_PREFERRED;

		drm_mode_set_name(mode);
		drm_mode_probed_add(connector, mode);
	}

	return i;
}

static const struct drm_panel_funcs xiaomi_taoyao_csot_nt36672c_panel_funcs = {
	.prepare = xiaomi_taoyao_csot_nt36672c_prepare,
	.unprepare = xiaomi_taoyao_csot_nt36672c_unprepare,
	.get_modes = xiaomi_taoyao_csot_nt36672c_get_modes,
};

static int
xiaomi_taoyao_csot_nt36672c_bl_update_status(struct backlight_device *bl)
{
	struct mipi_dsi_device *dsi = bl_get_data(bl);
	u16 brightness = backlight_get_brightness(bl);
	int ret;

	ret = mipi_dsi_dcs_set_display_brightness_large(dsi, brightness);
	if (ret < 0)
		return ret;

	return 0;
}

static int
xiaomi_taoyao_csot_nt36672c_bl_get_brightness(struct backlight_device *bl)
{
	struct mipi_dsi_device *dsi = bl_get_data(bl);
	u16 brightness;
	int ret;

	ret = mipi_dsi_dcs_get_display_brightness_large(dsi, &brightness);
	if (ret < 0)
		return ret;

	return brightness;
}

static const struct backlight_ops xiaomi_taoyao_csot_nt36672c_bl_ops = {
	.update_status = xiaomi_taoyao_csot_nt36672c_bl_update_status,
	.get_brightness = xiaomi_taoyao_csot_nt36672c_bl_get_brightness,
};

static struct backlight_device *
xiaomi_taoyao_csot_nt36672c_create_backlight(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	const struct backlight_properties props = {
		.type = BACKLIGHT_RAW,
		.brightness = 4095,
		.max_brightness = 4095,
	};

	return devm_backlight_device_register(
		dev, dev_name(dev), dev, dsi,
		&xiaomi_taoyao_csot_nt36672c_bl_ops, &props);
}

static int xiaomi_taoyao_csot_nt36672c_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct xiaomi_taoyao_csot_nt36672c *ctx;
	int ret;

	ctx = devm_drm_panel_alloc(dev, struct xiaomi_taoyao_csot_nt36672c,
				   panel,
				   &xiaomi_taoyao_csot_nt36672c_panel_funcs,
				   DRM_MODE_CONNECTOR_DSI);
	if (IS_ERR(ctx))
		return PTR_ERR(ctx);

	ret = devm_regulator_bulk_get_const(
		dev, ARRAY_SIZE(xiaomi_taoyao_csot_nt36672c_supplies),
		xiaomi_taoyao_csot_nt36672c_supplies, &ctx->supplies);
	if (ret < 0)
		return ret;

	ctx->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->reset_gpio))
		return dev_err_probe(dev, PTR_ERR(ctx->reset_gpio),
				     "Failed to get reset-gpios\n");

	ctx->dsi = dsi;
	mipi_dsi_set_drvdata(dsi, ctx);

	dsi->lanes = 4;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_CLOCK_NON_CONTINUOUS | MIPI_DSI_MODE_LPM;

	ctx->panel.prepare_prev_first = true;

	ctx->panel.backlight =
		xiaomi_taoyao_csot_nt36672c_create_backlight(dsi);
	if (IS_ERR(ctx->panel.backlight))
		return dev_err_probe(dev, PTR_ERR(ctx->panel.backlight),
				     "Failed to create backlight\n");

	drm_panel_add(&ctx->panel);

	/* This panel only supports DSC; unconditionally enable it */
	dsi->dsc = &ctx->dsc;

	ctx->dsc.dsc_version_major = 1;
	ctx->dsc.dsc_version_minor = 1;

	ctx->dsc.slice_height = 12;
	ctx->dsc.slice_width = 1080;
	ctx->dsc.slice_count = 1080 / ctx->dsc.slice_width;
	ctx->dsc.bits_per_component = 8;
	ctx->dsc.bits_per_pixel = 8 << 4; /* 4 fractional bits */
	ctx->dsc.block_pred_enable = true;

	ret = mipi_dsi_attach(dsi);
	if (ret < 0) {
		drm_panel_remove(&ctx->panel);
		return dev_err_probe(dev, ret,
				     "Failed to attach to DSI host\n");
	}

	return 0;
}

static void xiaomi_taoyao_csot_nt36672c_remove(struct mipi_dsi_device *dsi)
{
	struct xiaomi_taoyao_csot_nt36672c *ctx = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "Failed to detach from DSI host: %d\n", ret);

	drm_panel_remove(&ctx->panel);
}

static const struct of_device_id xiaomi_taoyao_csot_nt36672c_of_match[] = {
	{ .compatible = "xiaomi,taoyao-csot-nt36672c" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, xiaomi_taoyao_csot_nt36672c_of_match);

static struct mipi_dsi_driver xiaomi_taoyao_csot_nt36672c_driver = {
	.probe = xiaomi_taoyao_csot_nt36672c_probe,
	.remove = xiaomi_taoyao_csot_nt36672c_remove,
	.driver = {
		.name = "xiaomi,taoyao-csot-nt36672c",
		.of_match_table = xiaomi_taoyao_csot_nt36672c_of_match,
	},
};
module_mipi_dsi_driver(xiaomi_taoyao_csot_nt36672c_driver);

MODULE_AUTHOR("Stanislav Zaikin <zstaseg@gmail.com>");
MODULE_DESCRIPTION("DRM driver for xiaomi,taoyao-csot-nt36672c panel");
MODULE_LICENSE("GPL");
