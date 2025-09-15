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

struct hx8389c_gh9607501a {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct regulator_bulk_data *supplies;
	struct gpio_desc *reset_gpio;
};

static const struct regulator_bulk_data hx8389c_gh9607501a_supplies[] = {
	{ .supply = "vddio" },
	{ .supply = "vdd" },
};

static inline
struct hx8389c_gh9607501a *to_hx8389c_gh9607501a(struct drm_panel *panel)
{
	return container_of(panel, struct hx8389c_gh9607501a, panel);
}

static void hx8389c_gh9607501a_reset(struct hx8389c_gh9607501a *ctx)
{
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(10000, 11000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	usleep_range(2000, 3000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	msleep(80);
}

static int hx8389c_gh9607501a_on(struct hx8389c_gh9607501a *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	ctx->dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb9, 0xff, 0x83, 0x89);
	mipi_dsi_usleep_range(&dsi_ctx, 10000, 11000);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb1,
				     0x7f, 0x10, 0x10, 0xd2, 0x32, 0x80, 0x10,
				     0xf0, 0x56, 0x80, 0x20, 0x20, 0xf8, 0xaa,
				     0xaa, 0xa1, 0x00, 0x80, 0x30, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb2,
				     0x82, 0x50, 0x05, 0x07, 0xf0, 0x38, 0x11,
				     0x64, 0x5d, 0x09);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb4,
				     0x66, 0x66, 0x66, 0x70, 0x00, 0x00, 0x18,
				     0x76, 0x28, 0x76, 0xa8);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd2, 0x33);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc0, 0x30, 0x17, 0x00, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc7, 0x00, 0x80, 0x00, 0xc0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xbf, 0x05, 0x50, 0x00, 0x3e);
	mipi_dsi_usleep_range(&dsi_ctx, 10000, 11000);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb9, 0xff, 0x83, 0x89);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcc, 0x0e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd3,
				     0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00,
				     0x32, 0x10, 0x00, 0x00, 0x00, 0x03, 0xc6,
				     0x03, 0xc6, 0x00, 0x00, 0x00, 0x00, 0x35,
				     0x33, 0x04, 0x04, 0x37, 0x00, 0x00, 0x00,
				     0x05, 0x08, 0x00, 0x00, 0x0a, 0x00, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd5,
				     0x18, 0x18, 0x18, 0x18, 0x19, 0x19, 0x18,
				     0x18, 0x20, 0x21, 0x24, 0x25, 0x18, 0x18,
				     0x18, 0x18, 0x00, 0x01, 0x04, 0x05, 0x02,
				     0x03, 0x06, 0x07, 0x18, 0x18, 0x18, 0x18,
				     0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
				     0x18, 0x18, 0x18);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd6,
				     0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x19,
				     0x19, 0x25, 0x24, 0x21, 0x20, 0x18, 0x18,
				     0x18, 0x18, 0x07, 0x06, 0x03, 0x02, 0x05,
				     0x04, 0x01, 0x00, 0x18, 0x18, 0x18, 0x18,
				     0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
				     0x18, 0x18, 0x18);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb7, 0x20, 0x80, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe0,
				     0x00, 0x02, 0x06, 0x38, 0x3c, 0x3f, 0x1b,
				     0x46, 0x06, 0x09, 0x0c, 0x17, 0x10, 0x13,
				     0x16, 0x13, 0x14, 0x08, 0x13, 0x15, 0x19,
				     0x00, 0x02, 0x06, 0x37, 0x3c, 0x3f, 0x1a,
				     0x45, 0x05, 0x09, 0x0b, 0x16, 0x0f, 0x13,
				     0x15, 0x13, 0x14, 0x07, 0x12, 0x14, 0x18);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xbd, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc1,
				     0x00, 0x00, 0x08, 0x10, 0x18, 0x20, 0x28,
				     0x30, 0x38, 0x40, 0x48, 0x50, 0x58, 0x60,
				     0x68, 0x70, 0x78, 0x80, 0x88, 0x90, 0x98,
				     0xa0, 0xa8, 0xb0, 0xb8, 0xc0, 0xc8, 0xd0,
				     0xd8, 0xe0, 0xe8, 0xf0, 0xf8, 0xff, 0x00,
				     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				     0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xbd, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc1,
				     0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30,
				     0x38, 0x40, 0x48, 0x50, 0x58, 0x60, 0x68,
				     0x70, 0x78, 0x80, 0x88, 0x90, 0x98, 0xa0,
				     0xa8, 0xb0, 0xb8, 0xc0, 0xc8, 0xd0, 0xd8,
				     0xe0, 0xe8, 0xf0, 0xf8, 0xff, 0x00, 0x00,
				     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xbd, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc1,
				     0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30,
				     0x38, 0x40, 0x48, 0x50, 0x58, 0x60, 0x68,
				     0x70, 0x78, 0x80, 0x88, 0x90, 0x98, 0xa0,
				     0xa8, 0xb0, 0xb8, 0xc0, 0xc8, 0xd0, 0xd8,
				     0xe0, 0xe8, 0xf0, 0xf8, 0xff, 0x00, 0x00,
				     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc9,
				     0x1f, 0x00, 0x0f, 0x1e, 0x81, 0x1e, 0x00,
				     0x00, 0x01, 0x19, 0x00, 0x00, 0x20);
	mipi_dsi_usleep_range(&dsi_ctx, 5000, 6000);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_WRITE_POWER_SAVE, 0x00);
	mipi_dsi_usleep_range(&dsi_ctx, 5000, 6000);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_WRITE_CONTROL_DISPLAY,
				     0x24);
	mipi_dsi_usleep_range(&dsi_ctx, 5000, 6000);
	mipi_dsi_dcs_exit_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 125);
	mipi_dsi_dcs_set_display_on_multi(&dsi_ctx);
	mipi_dsi_usleep_range(&dsi_ctx, 5000, 6000);

	return dsi_ctx.accum_err;
}

static int hx8389c_gh9607501a_off(struct hx8389c_gh9607501a *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	ctx->dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x28, 0x00);
	mipi_dsi_msleep(&dsi_ctx, 60);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x10, 0x00);
	mipi_dsi_msleep(&dsi_ctx, 60);

	return dsi_ctx.accum_err;
}

static int hx8389c_gh9607501a_prepare(struct drm_panel *panel)
{
	struct hx8389c_gh9607501a *ctx = to_hx8389c_gh9607501a(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = regulator_bulk_enable(ARRAY_SIZE(hx8389c_gh9607501a_supplies), ctx->supplies);
	if (ret < 0) {
		dev_err(dev, "Failed to enable regulators: %d\n", ret);
		return ret;
	}

	hx8389c_gh9607501a_reset(ctx);

	ret = hx8389c_gh9607501a_on(ctx);
	if (ret < 0) {
		dev_err(dev, "Failed to initialize panel: %d\n", ret);
		gpiod_set_value_cansleep(ctx->reset_gpio, 1);
		regulator_bulk_disable(ARRAY_SIZE(hx8389c_gh9607501a_supplies), ctx->supplies);
		return ret;
	}

	return 0;
}

static int hx8389c_gh9607501a_unprepare(struct drm_panel *panel)
{
	struct hx8389c_gh9607501a *ctx = to_hx8389c_gh9607501a(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = hx8389c_gh9607501a_off(ctx);
	if (ret < 0)
		dev_err(dev, "Failed to un-initialize panel: %d\n", ret);

	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	regulator_bulk_disable(ARRAY_SIZE(hx8389c_gh9607501a_supplies), ctx->supplies);

	return 0;
}

static const struct drm_display_mode hx8389c_gh9607501a_mode = {
	.clock = (540 + 10 + 14 + 10) * (960 + 9 + 2 + 5) * 60 / 1000,
	.hdisplay = 540,
	.hsync_start = 540 + 10,
	.hsync_end = 540 + 10 + 14,
	.htotal = 540 + 10 + 14 + 10,
	.vdisplay = 960,
	.vsync_start = 960 + 9,
	.vsync_end = 960 + 9 + 2,
	.vtotal = 960 + 9 + 2 + 5,
	.width_mm = 62,
	.height_mm = 110,
	.type = DRM_MODE_TYPE_DRIVER,
};

static int hx8389c_gh9607501a_get_modes(struct drm_panel *panel,
					struct drm_connector *connector)
{
	return drm_connector_helper_get_modes_fixed(connector, &hx8389c_gh9607501a_mode);
}

static const struct drm_panel_funcs hx8389c_gh9607501a_panel_funcs = {
	.prepare = hx8389c_gh9607501a_prepare,
	.unprepare = hx8389c_gh9607501a_unprepare,
	.get_modes = hx8389c_gh9607501a_get_modes,
};

static int hx8389c_gh9607501a_bl_update_status(struct backlight_device *bl)
{
	struct mipi_dsi_device *dsi = bl_get_data(bl);
	u16 brightness = backlight_get_brightness(bl);
	int ret;

	dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	ret = mipi_dsi_dcs_set_display_brightness(dsi, brightness);
	if (ret < 0)
		return ret;

	dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	return 0;
}

static const struct backlight_ops hx8389c_gh9607501a_bl_ops = {
	.update_status = hx8389c_gh9607501a_bl_update_status,
};

static struct backlight_device *
hx8389c_gh9607501a_create_backlight(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	const struct backlight_properties props = {
		.type = BACKLIGHT_RAW,
		.brightness = 255,
		.max_brightness = 255,
	};

	return devm_backlight_device_register(dev, dev_name(dev), dev, dsi,
					      &hx8389c_gh9607501a_bl_ops, &props);
}

static int hx8389c_gh9607501a_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct hx8389c_gh9607501a *ctx;
	int ret;

	ctx = devm_drm_panel_alloc(dev, struct hx8389c_gh9607501a, panel,
				   &hx8389c_gh9607501a_panel_funcs,
				   DRM_MODE_CONNECTOR_DSI);
	if (IS_ERR(ctx))
		return PTR_ERR(ctx);

	ret = devm_regulator_bulk_get_const(dev,
					    ARRAY_SIZE(hx8389c_gh9607501a_supplies),
					    hx8389c_gh9607501a_supplies,
					    &ctx->supplies);
	if (ret < 0)
		return ret;

	ctx->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->reset_gpio))
		return dev_err_probe(dev, PTR_ERR(ctx->reset_gpio),
				     "Failed to get reset-gpios\n");

	ctx->dsi = dsi;
	mipi_dsi_set_drvdata(dsi, ctx);

	dsi->lanes = 2;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_SYNC_PULSE |
			  MIPI_DSI_MODE_NO_EOT_PACKET |
			  MIPI_DSI_CLOCK_NON_CONTINUOUS;

	ctx->panel.prepare_prev_first = true;

	ret = drm_panel_of_backlight(&ctx->panel);
	if (ret)
		return dev_err_probe(dev, ret, "Failed to get backlight\n");

	/* Fallback to DCS backlight if no backlight is defined in DT */
	if (!ctx->panel.backlight) {
		ctx->panel.backlight = hx8389c_gh9607501a_create_backlight(dsi);
		if (IS_ERR(ctx->panel.backlight))
			return dev_err_probe(dev, PTR_ERR(ctx->panel.backlight),
					     "Failed to create backlight\n");
	}

	drm_panel_add(&ctx->panel);

	ret = mipi_dsi_attach(dsi);
	if (ret < 0) {
		drm_panel_remove(&ctx->panel);
		return dev_err_probe(dev, ret, "Failed to attach to DSI host\n");
	}

	return 0;
}

static void hx8389c_gh9607501a_remove(struct mipi_dsi_device *dsi)
{
	struct hx8389c_gh9607501a *ctx = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "Failed to detach from DSI host: %d\n", ret);

	drm_panel_remove(&ctx->panel);
}

static const struct of_device_id hx8389c_gh9607501a_of_match[] = {
	{ .compatible = "samsung,hx8389c-gh9607501a" }, // FIXME
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, hx8389c_gh9607501a_of_match);

static struct mipi_dsi_driver hx8389c_gh9607501a_driver = {
	.probe = hx8389c_gh9607501a_probe,
	.remove = hx8389c_gh9607501a_remove,
	.driver = {
		.name = "panel-samsung-hx8389c-gh9607501a",
		.of_match_table = hx8389c_gh9607501a_of_match,
	},
};
module_mipi_dsi_driver(hx8389c_gh9607501a_driver);

MODULE_AUTHOR("linux-mdss-dsi-panel-driver-generator <fix@me>"); // FIXME
MODULE_DESCRIPTION("DRM driver for HX8389C qhd video mode dsi panel");
MODULE_LICENSE("GPL");
