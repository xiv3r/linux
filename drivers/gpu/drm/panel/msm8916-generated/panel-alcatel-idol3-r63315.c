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

struct r63315 {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct regulator_bulk_data *supplies;
	struct gpio_desc *reset_gpio;
};

static const struct regulator_bulk_data r63315_supplies[] = {
	{ .supply = "enp" },
	{ .supply = "enn" },
};

static inline struct r63315 *to_r63315(struct drm_panel *panel)
{
	return container_of(panel, struct r63315, panel);
}

static void r63315_reset(struct r63315 *ctx)
{
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	msleep(20);
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	msleep(20);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	msleep(20);
}

static int r63315_on(struct r63315 *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	ctx->dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xb0, 0x04);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xb3,
					 0x14, 0x00, 0x00, 0x00, 0x00, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xb6, 0x3a, 0xb3);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xc0, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xc1,
					 0x84, 0x60, 0x10, 0xeb, 0xff, 0x6f,
					 0xce, 0xff, 0xff, 0x17, 0x12, 0x58,
					 0x73, 0xae, 0x31, 0x20, 0xc6, 0xff,
					 0xff, 0x1f, 0xf3, 0xff, 0x5f, 0x10,
					 0x10, 0x10, 0x10, 0x00, 0x62, 0x01,
					 0x22, 0x22, 0x00, 0x01);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xc2,
					 0x31, 0xf7, 0x80, 0x06, 0x08, 0x80,
					 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xc4,
					 0x70, 0x00, 0x00, 0x00, 0x33, 0x00,
					 0x00, 0x33, 0x22, 0x0c, 0x06, 0x00,
					 0x00, 0x00, 0x00, 0x33, 0x00, 0x00,
					 0x33, 0x22, 0x0c, 0x06);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xc6,
					 0x77, 0x00, 0x69, 0x00, 0x69, 0x00,
					 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
					 0x16, 0x0a, 0x77, 0x00, 0x69, 0x00,
					 0x69, 0x00, 0x00, 0x00, 0x00, 0x00,
					 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					 0x00, 0x16, 0x18, 0x08);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xc7,
					 0x00, 0x12, 0x19, 0x22, 0x2e, 0x3a,
					 0x42, 0x51, 0x35, 0x3d, 0x49, 0x57,
					 0x61, 0x6a, 0x72, 0x00, 0x12, 0x19,
					 0x22, 0x2e, 0x3a, 0x42, 0x51, 0x35,
					 0x3d, 0x49, 0x57, 0x61, 0x6a, 0x72);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xc8,
					 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc,
					 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc,
					 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc,
					 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xcb,
					 0x31, 0xfc, 0x3f, 0x8c, 0x00, 0x00,
					 0x00, 0x00, 0xc0);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xcc, 0x0b);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xd0,
					 0x44, 0x81, 0xbb, 0x19, 0xd9, 0x4c,
					 0x19, 0x19, 0x04, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xd3,
					 0x1b, 0x33, 0xbb, 0xbb, 0xb3, 0x33,
					 0x33, 0x33, 0x01, 0x01, 0x00, 0xa0,
					 0xd8, 0xa0, 0x0d, 0x48, 0x48, 0x44,
					 0x3b, 0x22, 0x72, 0x07, 0x3d, 0xbf,
					 0x33);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xd5,
					 0x06, 0x00, 0x00, 0x01, 0x43, 0x01,
					 0x43);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xd6, 0x01);
	mipi_dsi_dcs_set_display_on_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 20);
	mipi_dsi_dcs_exit_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 120);

	return dsi_ctx.accum_err;
}

static int r63315_off(struct r63315 *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	ctx->dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xb0, 0x04);
	mipi_dsi_msleep(&dsi_ctx, 50);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xb1, 0x01);
	mipi_dsi_msleep(&dsi_ctx, 120);

	return dsi_ctx.accum_err;
}

static int r63315_prepare(struct drm_panel *panel)
{
	struct r63315 *ctx = to_r63315(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = regulator_bulk_enable(ARRAY_SIZE(r63315_supplies), ctx->supplies);
	if (ret < 0) {
		dev_err(dev, "Failed to enable regulators: %d\n", ret);
		return ret;
	}

	r63315_reset(ctx);

	ret = r63315_on(ctx);
	if (ret < 0) {
		dev_err(dev, "Failed to initialize panel: %d\n", ret);
		gpiod_set_value_cansleep(ctx->reset_gpio, 1);
		regulator_bulk_disable(ARRAY_SIZE(r63315_supplies), ctx->supplies);
		return ret;
	}

	return 0;
}

static int r63315_unprepare(struct drm_panel *panel)
{
	struct r63315 *ctx = to_r63315(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = r63315_off(ctx);
	if (ret < 0)
		dev_err(dev, "Failed to un-initialize panel: %d\n", ret);

	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	regulator_bulk_disable(ARRAY_SIZE(r63315_supplies), ctx->supplies);

	return 0;
}

static const struct drm_display_mode r63315_mode = {
	.clock = (1080 + 100 + 20 + 96) * (1920 + 8 + 2 + 4) * 60 / 1000,
	.hdisplay = 1080,
	.hsync_start = 1080 + 100,
	.hsync_end = 1080 + 100 + 20,
	.htotal = 1080 + 100 + 20 + 96,
	.vdisplay = 1920,
	.vsync_start = 1920 + 8,
	.vsync_end = 1920 + 8 + 2,
	.vtotal = 1920 + 8 + 2 + 4,
	.width_mm = 68,
	.height_mm = 121,
	.type = DRM_MODE_TYPE_DRIVER,
};

static int r63315_get_modes(struct drm_panel *panel,
			    struct drm_connector *connector)
{
	return drm_connector_helper_get_modes_fixed(connector, &r63315_mode);
}

static const struct drm_panel_funcs r63315_panel_funcs = {
	.prepare = r63315_prepare,
	.unprepare = r63315_unprepare,
	.get_modes = r63315_get_modes,
};

static int r63315_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct r63315 *ctx;
	int ret;

	ctx = devm_drm_panel_alloc(dev, struct r63315, panel,
				   &r63315_panel_funcs,
				   DRM_MODE_CONNECTOR_DSI);
	if (IS_ERR(ctx))
		return PTR_ERR(ctx);

	ret = devm_regulator_bulk_get_const(dev,
					    ARRAY_SIZE(r63315_supplies),
					    r63315_supplies,
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

static void r63315_remove(struct mipi_dsi_device *dsi)
{
	struct r63315 *ctx = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "Failed to detach from DSI host: %d\n", ret);

	drm_panel_remove(&ctx->panel);
}

static const struct of_device_id r63315_of_match[] = {
	{ .compatible = "alcatel,idol3-panel-r63315" }, // FIXME
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, r63315_of_match);

static struct mipi_dsi_driver r63315_driver = {
	.probe = r63315_probe,
	.remove = r63315_remove,
	.driver = {
		.name = "panel-alcatel-idol3-r63315",
		.of_match_table = r63315_of_match,
	},
};
module_mipi_dsi_driver(r63315_driver);

MODULE_AUTHOR("linux-mdss-dsi-panel-driver-generator <fix@me>"); // FIXME
MODULE_DESCRIPTION("DRM driver for r63315 1080p videomode panel");
MODULE_LICENSE("GPL");
