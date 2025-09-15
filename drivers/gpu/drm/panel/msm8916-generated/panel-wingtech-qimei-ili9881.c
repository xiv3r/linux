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

struct ili9881_qimei {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct regulator_bulk_data *supplies;
	struct gpio_desc *reset_gpio;
};

static const struct regulator_bulk_data ili9881_qimei_supplies[] = {
	{ .supply = "vsp" },
	{ .supply = "vsn" },
};

static inline struct ili9881_qimei *to_ili9881_qimei(struct drm_panel *panel)
{
	return container_of(panel, struct ili9881_qimei, panel);
}

static void ili9881_qimei_reset(struct ili9881_qimei *ctx)
{
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(5000, 6000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	usleep_range(5000, 6000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	msleep(30);
}

static int ili9881_qimei_on(struct ili9881_qimei *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	ctx->dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x98, 0x81, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x03, 0x21);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x04, 0x06);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x05, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x06, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x07, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x08, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x09, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0a, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0b, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0c, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0d, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0e, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0f, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x10, 0x40);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x11, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x12, 0x0b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x13, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x14, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x15, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x16, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x17, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x18, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x19, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1a, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1b, 0x54);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1c, 0xbb);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1d, 0x3c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1e, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1f, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x20, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x21, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x22, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x23, 0xc0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x24, 0x50);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x25, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x26, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x27, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x30, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x31, 0x23);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x32, 0x45);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x33, 0x67);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x34, 0x89);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x35, 0xab);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x36, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x37, 0x23);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x38, 0x45);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x39, 0x67);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3a, 0x89);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3b, 0xab);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3c, 0xcd);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3d, 0xef);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x50, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x51, 0x14);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x52, 0x15);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x53, 0x0c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x54, 0x0d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x55, 0x0e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x56, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x57, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x58, 0x11);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x59, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5a, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5b, 0x0a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5c, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5d, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5e, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5f, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x60, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x61, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x62, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x63, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x64, 0x06);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x65, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x66, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x67, 0x14);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x68, 0x15);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x69, 0x11);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6a, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6b, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6c, 0x0e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6d, 0x0d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6e, 0x0c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6f, 0x06);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x70, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x71, 0x0a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x72, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x73, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x74, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x75, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x76, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x77, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x78, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x79, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x7a, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x7b, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x7c, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x98, 0x81, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1f, 0x75);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xab, 0x74);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5d, 0x4c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x8a, 0xc0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe4, 0xe0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x76, 0xd4);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x78, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x74, 0x3b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x8e, 0x15);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x40, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x7b, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x72, 0x25);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x87, 0x3a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6c, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x49, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2d, 0x80);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x32, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3c, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x98, 0x81, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x41, 0x32);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x42, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x35, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x22, 0x0a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x31, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x53, 0x81);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x55, 0x88);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x50, 0xa3);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x51, 0xa3);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x60, 0x30);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa0, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa1, 0x20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa2, 0x2c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa3, 0x18);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa4, 0x1b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa5, 0x2e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa6, 0x20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa7, 0x21);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa8, 0x88);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa9, 0x1f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xaa, 0x2a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xab, 0x6d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xac, 0x1b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xad, 0x1a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xae, 0x4f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xaf, 0x24);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb0, 0x27);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb1, 0x43);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb2, 0x56);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb3, 0x3f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc0, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc1, 0x22);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc2, 0x32);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc3, 0x12);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc4, 0x14);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc5, 0x27);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc6, 0x1b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc7, 0x1d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc8, 0x8d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc9, 0x1a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xca, 0x26);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcb, 0x7a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcc, 0x19);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcd, 0x18);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xce, 0x4c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcf, 0x20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd0, 0x28);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd1, 0x51);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd2, 0x5c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd3, 0x3f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x98, 0x81, 0x00);
	mipi_dsi_dcs_exit_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 120);
	mipi_dsi_dcs_set_display_on_multi(&dsi_ctx);
	mipi_dsi_usleep_range(&dsi_ctx, 10000, 11000);

	return dsi_ctx.accum_err;
}

static int ili9881_qimei_off(struct ili9881_qimei *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	ctx->dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	mipi_dsi_dcs_set_display_off_multi(&dsi_ctx);
	mipi_dsi_usleep_range(&dsi_ctx, 10000, 11000);
	mipi_dsi_dcs_enter_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 140);

	return dsi_ctx.accum_err;
}

static int ili9881_qimei_prepare(struct drm_panel *panel)
{
	struct ili9881_qimei *ctx = to_ili9881_qimei(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = regulator_bulk_enable(ARRAY_SIZE(ili9881_qimei_supplies), ctx->supplies);
	if (ret < 0) {
		dev_err(dev, "Failed to enable regulators: %d\n", ret);
		return ret;
	}

	ili9881_qimei_reset(ctx);

	ret = ili9881_qimei_on(ctx);
	if (ret < 0) {
		dev_err(dev, "Failed to initialize panel: %d\n", ret);
		gpiod_set_value_cansleep(ctx->reset_gpio, 1);
		regulator_bulk_disable(ARRAY_SIZE(ili9881_qimei_supplies), ctx->supplies);
		return ret;
	}

	return 0;
}

static int ili9881_qimei_unprepare(struct drm_panel *panel)
{
	struct ili9881_qimei *ctx = to_ili9881_qimei(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = ili9881_qimei_off(ctx);
	if (ret < 0)
		dev_err(dev, "Failed to un-initialize panel: %d\n", ret);

	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	regulator_bulk_disable(ARRAY_SIZE(ili9881_qimei_supplies), ctx->supplies);

	return 0;
}

static const struct drm_display_mode ili9881_qimei_mode = {
	.clock = (720 + 168 + 24 + 240) * (1280 + 12 + 4 + 20) * 60 / 1000,
	.hdisplay = 720,
	.hsync_start = 720 + 168,
	.hsync_end = 720 + 168 + 24,
	.htotal = 720 + 168 + 24 + 240,
	.vdisplay = 1280,
	.vsync_start = 1280 + 12,
	.vsync_end = 1280 + 12 + 4,
	.vtotal = 1280 + 12 + 4 + 20,
	.width_mm = 59,
	.height_mm = 104,
	.type = DRM_MODE_TYPE_DRIVER,
};

static int ili9881_qimei_get_modes(struct drm_panel *panel,
				   struct drm_connector *connector)
{
	return drm_connector_helper_get_modes_fixed(connector, &ili9881_qimei_mode);
}

static const struct drm_panel_funcs ili9881_qimei_panel_funcs = {
	.prepare = ili9881_qimei_prepare,
	.unprepare = ili9881_qimei_unprepare,
	.get_modes = ili9881_qimei_get_modes,
};

static int ili9881_qimei_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct ili9881_qimei *ctx;
	int ret;

	ctx = devm_drm_panel_alloc(dev, struct ili9881_qimei, panel,
				   &ili9881_qimei_panel_funcs,
				   DRM_MODE_CONNECTOR_DSI);
	if (IS_ERR(ctx))
		return PTR_ERR(ctx);

	ret = devm_regulator_bulk_get_const(dev,
					    ARRAY_SIZE(ili9881_qimei_supplies),
					    ili9881_qimei_supplies,
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
			  MIPI_DSI_MODE_VIDEO_HSE | MIPI_DSI_MODE_NO_EOT_PACKET |
			  MIPI_DSI_CLOCK_NON_CONTINUOUS |
			  MIPI_DSI_MODE_VIDEO_NO_HFP |
			  MIPI_DSI_MODE_VIDEO_NO_HBP |
			  MIPI_DSI_MODE_VIDEO_NO_HSA;

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

static void ili9881_qimei_remove(struct mipi_dsi_device *dsi)
{
	struct ili9881_qimei *ctx = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "Failed to detach from DSI host: %d\n", ret);

	drm_panel_remove(&ctx->panel);
}

static const struct of_device_id ili9881_qimei_of_match[] = {
	{ .compatible = "wingtech,qimei-ili9881" }, // FIXME
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, ili9881_qimei_of_match);

static struct mipi_dsi_driver ili9881_qimei_driver = {
	.probe = ili9881_qimei_probe,
	.remove = ili9881_qimei_remove,
	.driver = {
		.name = "panel-wingtech-qimei-ili9881",
		.of_match_table = ili9881_qimei_of_match,
	},
};
module_mipi_dsi_driver(ili9881_qimei_driver);

MODULE_AUTHOR("linux-mdss-dsi-panel-driver-generator <fix@me>"); // FIXME
MODULE_DESCRIPTION("DRM driver for ili9881_qimei_HD720p_video_Yassy");
MODULE_LICENSE("GPL");
