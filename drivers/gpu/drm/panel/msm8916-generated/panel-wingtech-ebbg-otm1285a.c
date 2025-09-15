// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2025 FIXME
// Generated with linux-mdss-dsi-panel-driver-generator from vendor device tree:
//   Copyright (c) 2013, The Linux Foundation. All rights reserved. (FIXME)

#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/regulator/consumer.h>

#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>
#include <drm/drm_probe_helper.h>

struct otm1285a_otp {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct regulator *supply;
	struct gpio_desc *reset_gpio;
};

static inline struct otm1285a_otp *to_otm1285a_otp(struct drm_panel *panel)
{
	return container_of(panel, struct otm1285a_otp, panel);
}

static void otm1285a_otp_reset(struct otm1285a_otp *ctx)
{
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(5000, 6000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	usleep_range(5000, 6000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(10000, 11000);
}

static int otm1285a_otp_on(struct otm1285a_otp *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	ctx->dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x00, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xff, 0x12, 0x85, 0x01);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x00, 0x80);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xff, 0x12, 0x85);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x00, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x11);
	mipi_dsi_msleep(&dsi_ctx, 120);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x00, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x29);
	mipi_dsi_usleep_range(&dsi_ctx, 10000, 11000);

	return dsi_ctx.accum_err;
}

static int otm1285a_otp_off(struct otm1285a_otp *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	ctx->dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	mipi_dsi_dcs_set_display_off_multi(&dsi_ctx);
	mipi_dsi_dcs_enter_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 120);

	return dsi_ctx.accum_err;
}

static int otm1285a_otp_prepare(struct drm_panel *panel)
{
	struct otm1285a_otp *ctx = to_otm1285a_otp(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = regulator_enable(ctx->supply);
	if (ret < 0) {
		dev_err(dev, "Failed to enable regulator: %d\n", ret);
		return ret;
	}

	otm1285a_otp_reset(ctx);

	ret = otm1285a_otp_on(ctx);
	if (ret < 0) {
		dev_err(dev, "Failed to initialize panel: %d\n", ret);
		gpiod_set_value_cansleep(ctx->reset_gpio, 1);
		regulator_disable(ctx->supply);
		return ret;
	}

	return 0;
}

static int otm1285a_otp_unprepare(struct drm_panel *panel)
{
	struct otm1285a_otp *ctx = to_otm1285a_otp(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = otm1285a_otp_off(ctx);
	if (ret < 0)
		dev_err(dev, "Failed to un-initialize panel: %d\n", ret);

	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	regulator_disable(ctx->supply);

	return 0;
}

static const struct drm_display_mode otm1285a_otp_mode = {
	.clock = (720 + 28 + 2 + 28) * (1280 + 30 + 2 + 30) * 59 / 1000,
	.hdisplay = 720,
	.hsync_start = 720 + 28,
	.hsync_end = 720 + 28 + 2,
	.htotal = 720 + 28 + 2 + 28,
	.vdisplay = 1280,
	.vsync_start = 1280 + 30,
	.vsync_end = 1280 + 30 + 2,
	.vtotal = 1280 + 30 + 2 + 30,
	.width_mm = 58,
	.height_mm = 103,
	.type = DRM_MODE_TYPE_DRIVER,
};

static int otm1285a_otp_get_modes(struct drm_panel *panel,
				  struct drm_connector *connector)
{
	return drm_connector_helper_get_modes_fixed(connector, &otm1285a_otp_mode);
}

static const struct drm_panel_funcs otm1285a_otp_panel_funcs = {
	.prepare = otm1285a_otp_prepare,
	.unprepare = otm1285a_otp_unprepare,
	.get_modes = otm1285a_otp_get_modes,
};

static int otm1285a_otp_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct otm1285a_otp *ctx;
	int ret;

	ctx = devm_drm_panel_alloc(dev, struct otm1285a_otp, panel,
				   &otm1285a_otp_panel_funcs,
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

	dsi->lanes = 3;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_HSE |
			  MIPI_DSI_MODE_NO_EOT_PACKET |
			  MIPI_DSI_CLOCK_NON_CONTINUOUS;

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

static void otm1285a_otp_remove(struct mipi_dsi_device *dsi)
{
	struct otm1285a_otp *ctx = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "Failed to detach from DSI host: %d\n", ret);

	drm_panel_remove(&ctx->panel);
}

static const struct of_device_id otm1285a_otp_of_match[] = {
	{ .compatible = "wingtech,ebbg-otm1285a" }, // FIXME
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, otm1285a_otp_of_match);

static struct mipi_dsi_driver otm1285a_otp_driver = {
	.probe = otm1285a_otp_probe,
	.remove = otm1285a_otp_remove,
	.driver = {
		.name = "panel-wingtech-ebbg-otm1285a",
		.of_match_table = otm1285a_otp_of_match,
	},
};
module_mipi_dsi_driver(otm1285a_otp_driver);

MODULE_AUTHOR("linux-mdss-dsi-panel-driver-generator <fix@me>"); // FIXME
MODULE_DESCRIPTION("DRM driver for otm1285a_otp_720p_video_EBBG");
MODULE_LICENSE("GPL");
