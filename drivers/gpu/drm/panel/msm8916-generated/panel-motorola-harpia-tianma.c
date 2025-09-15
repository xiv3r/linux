// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2025 FIXME
// Generated with linux-mdss-dsi-panel-driver-generator from vendor device tree:
//   Copyright (c) 2013, The Linux Foundation. All rights reserved. (FIXME)

#include <linux/backlight.h>
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

struct tianma_499_v2 {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct regulator_bulk_data *supplies;
	struct gpio_desc *reset_gpio;
	struct gpio_desc *backlight_gpio;
};

static const struct regulator_bulk_data tianma_499_v2_supplies[] = {
	{ .supply = "vsp" },
	{ .supply = "vsn" },
};

static inline struct tianma_499_v2 *to_tianma_499_v2(struct drm_panel *panel)
{
	return container_of(panel, struct tianma_499_v2, panel);
}

static void tianma_499_v2_reset(struct tianma_499_v2 *ctx)
{
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(1000, 2000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	usleep_range(10000, 11000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	msleep(20);
}

static int tianma_499_v2_on(struct tianma_499_v2 *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x98, 0x81, 0x00);
	mipi_dsi_dcs_set_display_brightness_multi(&dsi_ctx, 0x0000);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_WRITE_CONTROL_DISPLAY,
				     0x2c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_WRITE_POWER_SAVE, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x68, 0x06);
	mipi_dsi_dcs_exit_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 120);
	mipi_dsi_dcs_set_display_on_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 20);

	return dsi_ctx.accum_err;
}

static int tianma_499_v2_off(struct tianma_499_v2 *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	mipi_dsi_dcs_set_display_off_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 20);
	mipi_dsi_dcs_enter_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 120);

	return dsi_ctx.accum_err;
}

static int tianma_499_v2_prepare(struct drm_panel *panel)
{
	struct tianma_499_v2 *ctx = to_tianma_499_v2(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = regulator_bulk_enable(ARRAY_SIZE(tianma_499_v2_supplies), ctx->supplies);
	if (ret < 0) {
		dev_err(dev, "Failed to enable regulators: %d\n", ret);
		return ret;
	}

	tianma_499_v2_reset(ctx);

	ret = tianma_499_v2_on(ctx);
	if (ret < 0) {
		dev_err(dev, "Failed to initialize panel: %d\n", ret);
		gpiod_set_value_cansleep(ctx->reset_gpio, 1);
		regulator_bulk_disable(ARRAY_SIZE(tianma_499_v2_supplies), ctx->supplies);
		return ret;
	}

	return 0;
}

static int tianma_499_v2_unprepare(struct drm_panel *panel)
{
	struct tianma_499_v2 *ctx = to_tianma_499_v2(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = tianma_499_v2_off(ctx);
	if (ret < 0)
		dev_err(dev, "Failed to un-initialize panel: %d\n", ret);

	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	regulator_bulk_disable(ARRAY_SIZE(tianma_499_v2_supplies), ctx->supplies);

	return 0;
}

static const struct drm_display_mode tianma_499_v2_mode = {
	.clock = (720 + 52 + 20 + 84) * (1280 + 20 + 8 + 20) * 60 / 1000,
	.hdisplay = 720,
	.hsync_start = 720 + 52,
	.hsync_end = 720 + 52 + 20,
	.htotal = 720 + 52 + 20 + 84,
	.vdisplay = 1280,
	.vsync_start = 1280 + 20,
	.vsync_end = 1280 + 20 + 8,
	.vtotal = 1280 + 20 + 8 + 20,
	.width_mm = 62,
	.height_mm = 110,
	.type = DRM_MODE_TYPE_DRIVER,
};

static int tianma_499_v2_get_modes(struct drm_panel *panel,
				   struct drm_connector *connector)
{
	return drm_connector_helper_get_modes_fixed(connector, &tianma_499_v2_mode);
}

static const struct drm_panel_funcs tianma_499_v2_panel_funcs = {
	.prepare = tianma_499_v2_prepare,
	.unprepare = tianma_499_v2_unprepare,
	.get_modes = tianma_499_v2_get_modes,
};

static int tianma_499_v2_bl_update_status(struct backlight_device *bl)
{
	struct mipi_dsi_device *dsi = bl_get_data(bl);
	struct tianma_499_v2 *ctx = mipi_dsi_get_drvdata(dsi);
	u16 brightness = backlight_get_brightness(bl);
	int ret;

	gpiod_set_value_cansleep(ctx->backlight_gpio, !!brightness);

	dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	ret = mipi_dsi_dcs_set_display_brightness_large(dsi, brightness);
	if (ret < 0)
		return ret;

	dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	return 0;
}

static const struct backlight_ops tianma_499_v2_bl_ops = {
	.update_status = tianma_499_v2_bl_update_status,
};

static struct backlight_device *
tianma_499_v2_create_backlight(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	const struct backlight_properties props = {
		.type = BACKLIGHT_RAW,
		.brightness = 4095,
		.max_brightness = 4095,
	};

	return devm_backlight_device_register(dev, dev_name(dev), dev, dsi,
					      &tianma_499_v2_bl_ops, &props);
}

static int tianma_499_v2_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct tianma_499_v2 *ctx;
	int ret;

	ctx = devm_drm_panel_alloc(dev, struct tianma_499_v2, panel,
				   &tianma_499_v2_panel_funcs,
				   DRM_MODE_CONNECTOR_DSI);
	if (IS_ERR(ctx))
		return PTR_ERR(ctx);

	ret = devm_regulator_bulk_get_const(dev,
					    ARRAY_SIZE(tianma_499_v2_supplies),
					    tianma_499_v2_supplies,
					    &ctx->supplies);
	if (ret < 0)
		return ret;

	ctx->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->reset_gpio))
		return dev_err_probe(dev, PTR_ERR(ctx->reset_gpio),
				     "Failed to get reset-gpios\n");

	ctx->backlight_gpio = devm_gpiod_get(dev, "backlight", GPIOD_OUT_LOW);
	if (IS_ERR(ctx->backlight_gpio))
		return dev_err_probe(dev, PTR_ERR(ctx->backlight_gpio),
				     "Failed to get backlight-gpios\n");

	ctx->dsi = dsi;
	mipi_dsi_set_drvdata(dsi, ctx);

	dsi->lanes = 4;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_BURST |
			  MIPI_DSI_MODE_VIDEO_HSE |
			  MIPI_DSI_CLOCK_NON_CONTINUOUS | MIPI_DSI_MODE_LPM;

	ctx->panel.prepare_prev_first = true;

	ctx->panel.backlight = tianma_499_v2_create_backlight(dsi);
	if (IS_ERR(ctx->panel.backlight))
		return dev_err_probe(dev, PTR_ERR(ctx->panel.backlight),
				     "Failed to create backlight\n");

	drm_panel_add(&ctx->panel);

	ret = mipi_dsi_attach(dsi);
	if (ret < 0) {
		drm_panel_remove(&ctx->panel);
		return dev_err_probe(dev, ret, "Failed to attach to DSI host\n");
	}

	return 0;
}

static void tianma_499_v2_remove(struct mipi_dsi_device *dsi)
{
	struct tianma_499_v2 *ctx = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "Failed to detach from DSI host: %d\n", ret);

	drm_panel_remove(&ctx->panel);
}

static const struct of_device_id tianma_499_v2_of_match[] = {
	{ .compatible = "motorola,harpia-panel-tianma" }, // FIXME
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, tianma_499_v2_of_match);

static struct mipi_dsi_driver tianma_499_v2_driver = {
	.probe = tianma_499_v2_probe,
	.remove = tianma_499_v2_remove,
	.driver = {
		.name = "panel-motorola-harpia-tianma",
		.of_match_table = tianma_499_v2_of_match,
	},
};
module_mipi_dsi_driver(tianma_499_v2_driver);

MODULE_AUTHOR("linux-mdss-dsi-panel-driver-generator <fix@me>"); // FIXME
MODULE_DESCRIPTION("DRM driver for mipi_mot_video_tianma_720p_499");
MODULE_LICENSE("GPL");
