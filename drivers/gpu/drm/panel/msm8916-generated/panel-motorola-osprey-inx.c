// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2025 FIXME
// Generated with linux-mdss-dsi-panel-driver-generator from vendor device tree:
//   Copyright (c) 2013, The Linux Foundation. All rights reserved. (FIXME)

#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/regulator/consumer.h>

#include <video/mipi_display.h>

#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>
#include <drm/drm_probe_helper.h>

struct inx_500_v0 {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct regulator_bulk_data *supplies;
	struct gpio_desc *reset_gpio;
};

static const struct regulator_bulk_data inx_500_v0_supplies[] = {
	{ .supply = "vsp" },
	{ .supply = "vsn" },
	{ .supply = "vddio" },
};

static inline struct inx_500_v0 *to_inx_500_v0(struct drm_panel *panel)
{
	return container_of(panel, struct inx_500_v0, panel);
}

static void inx_500_v0_reset(struct inx_500_v0 *ctx)
{
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	usleep_range(10000, 11000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(10000, 11000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	usleep_range(10000, 11000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	msleep(20);
}

static int inx_500_v0_on(struct inx_500_v0 *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xf0,
					 0x55, 0xaa, 0x52, 0x08, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xe8,
					 0xf3, 0xd9, 0xcc, 0xc0, 0xb8, 0xb0,
					 0xa0, 0xa0, 0xa0, 0x9c);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xd9, 0x03, 0x06);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xea, 0x09);
	mipi_dsi_dcs_exit_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 120);
	mipi_dsi_dcs_set_display_brightness_multi(&dsi_ctx, 0x00cf);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_WRITE_CONTROL_DISPLAY,
				     0x2c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_WRITE_POWER_SAVE, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_SET_ADDRESS_MODE, 0x03);
	mipi_dsi_dcs_set_display_on_multi(&dsi_ctx);

	return dsi_ctx.accum_err;
}

static int inx_500_v0_off(struct inx_500_v0 *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	mipi_dsi_dcs_set_display_off_multi(&dsi_ctx);
	mipi_dsi_usleep_range(&dsi_ctx, 17000, 18000);
	mipi_dsi_dcs_enter_sleep_mode_multi(&dsi_ctx);

	return dsi_ctx.accum_err;
}

static int inx_500_v0_prepare(struct drm_panel *panel)
{
	struct inx_500_v0 *ctx = to_inx_500_v0(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = regulator_bulk_enable(ARRAY_SIZE(inx_500_v0_supplies), ctx->supplies);
	if (ret < 0) {
		dev_err(dev, "Failed to enable regulators: %d\n", ret);
		return ret;
	}

	inx_500_v0_reset(ctx);

	ret = inx_500_v0_on(ctx);
	if (ret < 0) {
		dev_err(dev, "Failed to initialize panel: %d\n", ret);
		gpiod_set_value_cansleep(ctx->reset_gpio, 1);
		regulator_bulk_disable(ARRAY_SIZE(inx_500_v0_supplies), ctx->supplies);
		return ret;
	}

	return 0;
}

static int inx_500_v0_unprepare(struct drm_panel *panel)
{
	struct inx_500_v0 *ctx = to_inx_500_v0(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = inx_500_v0_off(ctx);
	if (ret < 0)
		dev_err(dev, "Failed to un-initialize panel: %d\n", ret);

	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	regulator_bulk_disable(ARRAY_SIZE(inx_500_v0_supplies), ctx->supplies);

	return 0;
}

static const struct drm_display_mode inx_500_v0_mode = {
	.clock = (720 + 48 + 8 + 68) * (1280 + 52 + 6 + 50) * 60 / 1000,
	.hdisplay = 720,
	.hsync_start = 720 + 48,
	.hsync_end = 720 + 48 + 8,
	.htotal = 720 + 48 + 8 + 68,
	.vdisplay = 1280,
	.vsync_start = 1280 + 52,
	.vsync_end = 1280 + 52 + 6,
	.vtotal = 1280 + 52 + 6 + 50,
	.width_mm = 62,
	.height_mm = 110,
	.type = DRM_MODE_TYPE_DRIVER,
};

static int inx_500_v0_get_modes(struct drm_panel *panel,
				struct drm_connector *connector)
{
	return drm_connector_helper_get_modes_fixed(connector, &inx_500_v0_mode);
}

static const struct drm_panel_funcs inx_500_v0_panel_funcs = {
	.prepare = inx_500_v0_prepare,
	.unprepare = inx_500_v0_unprepare,
	.get_modes = inx_500_v0_get_modes,
};

static int inx_500_v0_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct inx_500_v0 *ctx;
	int ret;

	ctx = devm_drm_panel_alloc(dev, struct inx_500_v0, panel,
				   &inx_500_v0_panel_funcs,
				   DRM_MODE_CONNECTOR_DSI);
	if (IS_ERR(ctx))
		return PTR_ERR(ctx);

	ret = devm_regulator_bulk_get_const(dev,
					    ARRAY_SIZE(inx_500_v0_supplies),
					    inx_500_v0_supplies,
					    &ctx->supplies);
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
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_BURST |
			  MIPI_DSI_MODE_VIDEO_HSE |
			  MIPI_DSI_CLOCK_NON_CONTINUOUS | MIPI_DSI_MODE_LPM;

	ctx->panel.prepare_prev_first = true;

	ret = drm_panel_of_backlight(&ctx->panel);
	if (ret)
		return dev_err_probe(dev, ret, "Failed to get backlight\n");

	drm_panel_add(&ctx->panel);

	ret = mipi_dsi_attach(dsi);
	if (ret < 0) {
		drm_panel_remove(&ctx->panel);
		return dev_err_probe(dev, ret, "Failed to attach to DSI host\n");
	}

	return 0;
}

static void inx_500_v0_remove(struct mipi_dsi_device *dsi)
{
	struct inx_500_v0 *ctx = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "Failed to detach from DSI host: %d\n", ret);

	drm_panel_remove(&ctx->panel);
}

static const struct of_device_id inx_500_v0_of_match[] = {
	{ .compatible = "motorola,osprey-panel-inx" }, // FIXME
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, inx_500_v0_of_match);

static struct mipi_dsi_driver inx_500_v0_driver = {
	.probe = inx_500_v0_probe,
	.remove = inx_500_v0_remove,
	.driver = {
		.name = "panel-motorola-osprey-inx",
		.of_match_table = inx_500_v0_of_match,
	},
};
module_mipi_dsi_driver(inx_500_v0_driver);

MODULE_AUTHOR("linux-mdss-dsi-panel-driver-generator <fix@me>"); // FIXME
MODULE_DESCRIPTION("DRM driver for mipi_mot_video_inx_720p_500_v0");
MODULE_LICENSE("GPL");
