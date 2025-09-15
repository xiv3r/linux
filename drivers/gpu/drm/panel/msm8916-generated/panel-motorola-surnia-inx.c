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

struct inx_450_v1 {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct regulator *supply;
	struct gpio_desc *reset_gpio;
};

static inline struct inx_450_v1 *to_inx_450_v1(struct drm_panel *panel)
{
	return container_of(panel, struct inx_450_v1, panel);
}

static void inx_450_v1_reset(struct inx_450_v1 *ctx)
{
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(2000, 3000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	usleep_range(1000, 2000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	msleep(21);
}

static int inx_450_v1_on(struct inx_450_v1 *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_WRITE_CONTROL_DISPLAY,
				     0x2c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_WRITE_POWER_SAVE, 0x02);
	mipi_dsi_dcs_exit_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 120);
	mipi_dsi_dcs_set_display_brightness_multi(&dsi_ctx, 0x00ff);
	mipi_dsi_dcs_set_display_on_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 40);

	return dsi_ctx.accum_err;
}

static int inx_450_v1_off(struct inx_450_v1 *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	mipi_dsi_dcs_set_display_off_multi(&dsi_ctx);
	mipi_dsi_usleep_range(&dsi_ctx, 10000, 11000);
	mipi_dsi_dcs_enter_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 120);

	return dsi_ctx.accum_err;
}

static int inx_450_v1_prepare(struct drm_panel *panel)
{
	struct inx_450_v1 *ctx = to_inx_450_v1(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = regulator_enable(ctx->supply);
	if (ret < 0) {
		dev_err(dev, "Failed to enable regulator: %d\n", ret);
		return ret;
	}

	inx_450_v1_reset(ctx);

	ret = inx_450_v1_on(ctx);
	if (ret < 0) {
		dev_err(dev, "Failed to initialize panel: %d\n", ret);
		gpiod_set_value_cansleep(ctx->reset_gpio, 1);
		regulator_disable(ctx->supply);
		return ret;
	}

	return 0;
}

static int inx_450_v1_unprepare(struct drm_panel *panel)
{
	struct inx_450_v1 *ctx = to_inx_450_v1(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = inx_450_v1_off(ctx);
	if (ret < 0)
		dev_err(dev, "Failed to un-initialize panel: %d\n", ret);

	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	regulator_disable(ctx->supply);

	return 0;
}

static const struct drm_display_mode inx_450_v1_mode = {
	.clock = (540 + 24 + 4 + 40) * (960 + 16 + 2 + 19) * 60 / 1000,
	.hdisplay = 540,
	.hsync_start = 540 + 24,
	.hsync_end = 540 + 24 + 4,
	.htotal = 540 + 24 + 4 + 40,
	.vdisplay = 960,
	.vsync_start = 960 + 16,
	.vsync_end = 960 + 16 + 2,
	.vtotal = 960 + 16 + 2 + 19,
	.width_mm = 55,
	.height_mm = 99,
	.type = DRM_MODE_TYPE_DRIVER,
};

static int inx_450_v1_get_modes(struct drm_panel *panel,
				struct drm_connector *connector)
{
	return drm_connector_helper_get_modes_fixed(connector, &inx_450_v1_mode);
}

static const struct drm_panel_funcs inx_450_v1_panel_funcs = {
	.prepare = inx_450_v1_prepare,
	.unprepare = inx_450_v1_unprepare,
	.get_modes = inx_450_v1_get_modes,
};

static int inx_450_v1_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct inx_450_v1 *ctx;
	int ret;

	ctx = devm_drm_panel_alloc(dev, struct inx_450_v1, panel,
				   &inx_450_v1_panel_funcs,
				   DRM_MODE_CONNECTOR_DSI);
	if (IS_ERR(ctx))
		return PTR_ERR(ctx);

	ctx->supply = devm_regulator_get(dev, "power");
	if (IS_ERR(ctx->supply))
		return dev_err_probe(dev, PTR_ERR(ctx->supply),
				     "Failed to get power regulator\n");

	ctx->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->reset_gpio))
		return dev_err_probe(dev, PTR_ERR(ctx->reset_gpio),
				     "Failed to get reset-gpios\n");

	ctx->dsi = dsi;
	mipi_dsi_set_drvdata(dsi, ctx);

	dsi->lanes = 2;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_CLOCK_NON_CONTINUOUS |
			  MIPI_DSI_MODE_LPM;

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

static void inx_450_v1_remove(struct mipi_dsi_device *dsi)
{
	struct inx_450_v1 *ctx = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "Failed to detach from DSI host: %d\n", ret);

	drm_panel_remove(&ctx->panel);
}

static const struct of_device_id inx_450_v1_of_match[] = {
	{ .compatible = "motorola,surnia-panel-inx" }, // FIXME
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, inx_450_v1_of_match);

static struct mipi_dsi_driver inx_450_v1_driver = {
	.probe = inx_450_v1_probe,
	.remove = inx_450_v1_remove,
	.driver = {
		.name = "panel-motorola-surnia-inx",
		.of_match_table = inx_450_v1_of_match,
	},
};
module_mipi_dsi_driver(inx_450_v1_driver);

MODULE_AUTHOR("linux-mdss-dsi-panel-driver-generator <fix@me>"); // FIXME
MODULE_DESCRIPTION("DRM driver for mipi_mot_video_inx_qhd_450_v1");
MODULE_LICENSE("GPL");
