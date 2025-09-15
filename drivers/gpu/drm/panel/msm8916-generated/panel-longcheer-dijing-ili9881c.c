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

struct dijing_ili9881c {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct regulator *supply;
	struct gpio_desc *reset_gpio;
};

static inline
struct dijing_ili9881c *to_dijing_ili9881c(struct drm_panel *panel)
{
	return container_of(panel, struct dijing_ili9881c, panel);
}

static void dijing_ili9881c_reset(struct dijing_ili9881c *ctx)
{
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	msleep(20);
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	usleep_range(10000, 11000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	msleep(20);
}

static int dijing_ili9881c_on(struct dijing_ili9881c *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xff, 0x98, 0x81, 0x03);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x01, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x02, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x03, 0x72);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x04, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x05, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x06, 0x09);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x07, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x08, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x09, 0x01);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x0a, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x0b, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x0c, 0x01);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x0d, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x0e, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x0f, 0x14);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x10, 0x14);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x11, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x12, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x13, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x14, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x15, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x16, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x17, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x18, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x19, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x1a, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x1b, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x1c, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x1d, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x1e, 0x40);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x1f, 0x80);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x20, 0x05);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x21, 0x02);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x22, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x23, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x24, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x25, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x26, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x27, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x28, 0x33);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x29, 0x02);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x2a, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x2b, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x2c, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x2d, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x2e, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x2f, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x30, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x31, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x32, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x33, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x34, 0x04);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x35, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x36, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x37, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x38, 0x78);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x39, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x3a, 0x40);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x3b, 0x40);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x3c, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x3d, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x3e, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x3f, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x40, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x41, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x42, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x43, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x44, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x50, 0x01);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x51, 0x23);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x52, 0x45);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x53, 0x67);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x54, 0x89);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x55, 0xab);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x56, 0x01);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x57, 0x23);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x58, 0x45);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x59, 0x67);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x5a, 0x89);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x5b, 0xab);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x5c, 0xcd);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x5d, 0xef);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x5e, 0x11);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x5f, 0x01);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x60, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x61, 0x15);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x62, 0x14);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x63, 0x0e);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x64, 0x0f);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x65, 0x0c);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x66, 0x0d);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x67, 0x06);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x68, 0x02);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x69, 0x07);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x6a, 0x02);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x6b, 0x02);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x6c, 0x02);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x6d, 0x02);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x6e, 0x02);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x6f, 0x02);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x70, 0x02);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x71, 0x02);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x72, 0x02);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x73, 0x02);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x74, 0x02);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x75, 0x01);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x76, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x77, 0x14);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x78, 0x15);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x79, 0x0e);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x7a, 0x0f);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x7b, 0x0c);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x7c, 0x0d);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x7d, 0x06);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x7e, 0x02);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x7f, 0x07);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x80, 0x02);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x81, 0x02);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x82, 0x02);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x83, 0x02);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x84, 0x02);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x85, 0x02);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x86, 0x02);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x87, 0x02);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x88, 0x02);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x89, 0x02);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x8a, 0x02);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xff, 0x98, 0x81, 0x04);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x00, 0x80);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x6c, 0x15);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x6e, 0x2a);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x6f, 0x33);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x3a, 0x94);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x8d, 0x1a);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x87, 0xba);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x26, 0x76);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xb2, 0xd1);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xb5, 0x06);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xff, 0x98, 0x81, 0x01);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x22, 0x0a);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x31, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x53, 0x81);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x55, 0x8f);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x50, 0xc0);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x51, 0xc0);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x60, 0x08);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xa0, 0x08);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xa1, 0x10);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xa2, 0x25);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xa3, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xa4, 0x24);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xa5, 0x19);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xa6, 0x12);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xa7, 0x1b);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xa8, 0x77);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xa9, 0x19);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xaa, 0x25);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xab, 0x6e);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xac, 0x20);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xad, 0x17);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xae, 0x54);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xaf, 0x24);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xb0, 0x27);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xb1, 0x52);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xb2, 0x63);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xb3, 0x39);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xc0, 0x08);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xc1, 0x20);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xc2, 0x23);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xc3, 0x22);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xc4, 0x06);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xc5, 0x34);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xc6, 0x25);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xc7, 0x20);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xc8, 0x86);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xc9, 0x1f);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xca, 0x2b);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xcb, 0x74);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xcc, 0x16);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xcd, 0x1b);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xce, 0x46);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xcf, 0x21);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xd0, 0x29);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xd1, 0x54);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xd2, 0x65);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xd3, 0x39);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xff, 0x98, 0x81, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x35, 0x00);
	mipi_dsi_dcs_exit_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 120);
	mipi_dsi_dcs_set_display_on_multi(&dsi_ctx);

	return dsi_ctx.accum_err;
}

static int dijing_ili9881c_off(struct dijing_ili9881c *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	mipi_dsi_dcs_set_display_off_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 50);
	mipi_dsi_dcs_enter_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 120);

	return dsi_ctx.accum_err;
}

static int dijing_ili9881c_prepare(struct drm_panel *panel)
{
	struct dijing_ili9881c *ctx = to_dijing_ili9881c(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = regulator_enable(ctx->supply);
	if (ret < 0) {
		dev_err(dev, "Failed to enable regulator: %d\n", ret);
		return ret;
	}

	dijing_ili9881c_reset(ctx);

	ret = dijing_ili9881c_on(ctx);
	if (ret < 0) {
		dev_err(dev, "Failed to initialize panel: %d\n", ret);
		gpiod_set_value_cansleep(ctx->reset_gpio, 1);
		regulator_disable(ctx->supply);
		return ret;
	}

	return 0;
}

static int dijing_ili9881c_unprepare(struct drm_panel *panel)
{
	struct dijing_ili9881c *ctx = to_dijing_ili9881c(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = dijing_ili9881c_off(ctx);
	if (ret < 0)
		dev_err(dev, "Failed to un-initialize panel: %d\n", ret);

	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	regulator_disable(ctx->supply);

	return 0;
}

static const struct drm_display_mode dijing_ili9881c_mode = {
	.clock = (720 + 150 + 10 + 150) * (1280 + 20 + 2 + 19) * 60 / 1000,
	.hdisplay = 720,
	.hsync_start = 720 + 150,
	.hsync_end = 720 + 150 + 10,
	.htotal = 720 + 150 + 10 + 150,
	.vdisplay = 1280,
	.vsync_start = 1280 + 20,
	.vsync_end = 1280 + 20 + 2,
	.vtotal = 1280 + 20 + 2 + 19,
	.width_mm = 62,
	.height_mm = 110,
	.type = DRM_MODE_TYPE_DRIVER,
};

static int dijing_ili9881c_get_modes(struct drm_panel *panel,
				     struct drm_connector *connector)
{
	return drm_connector_helper_get_modes_fixed(connector, &dijing_ili9881c_mode);
}

static const struct drm_panel_funcs dijing_ili9881c_panel_funcs = {
	.prepare = dijing_ili9881c_prepare,
	.unprepare = dijing_ili9881c_unprepare,
	.get_modes = dijing_ili9881c_get_modes,
};

static int dijing_ili9881c_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct dijing_ili9881c *ctx;
	int ret;

	ctx = devm_drm_panel_alloc(dev, struct dijing_ili9881c, panel,
				   &dijing_ili9881c_panel_funcs,
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

	dsi->lanes = 4;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_SYNC_PULSE |
			  MIPI_DSI_MODE_NO_EOT_PACKET |
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

static void dijing_ili9881c_remove(struct mipi_dsi_device *dsi)
{
	struct dijing_ili9881c *ctx = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "Failed to detach from DSI host: %d\n", ret);

	drm_panel_remove(&ctx->panel);
}

static const struct of_device_id dijing_ili9881c_of_match[] = {
	{ .compatible = "longcheer,dijing-ili9881c" }, // FIXME
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, dijing_ili9881c_of_match);

static struct mipi_dsi_driver dijing_ili9881c_driver = {
	.probe = dijing_ili9881c_probe,
	.remove = dijing_ili9881c_remove,
	.driver = {
		.name = "panel-longcheer-dijing-ili9881c",
		.of_match_table = dijing_ili9881c_of_match,
	},
};
module_mipi_dsi_driver(dijing_ili9881c_driver);

MODULE_AUTHOR("linux-mdss-dsi-panel-driver-generator <fix@me>"); // FIXME
MODULE_DESCRIPTION("DRM driver for dijing ILI9881C 720p video mode dsi panel");
MODULE_LICENSE("GPL");
