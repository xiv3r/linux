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

struct s6d2aa0x {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct regulator_bulk_data *supplies;
	struct gpio_desc *reset_gpio;
	struct gpio_desc *backlight_gpio;
};

static const struct regulator_bulk_data s6d2aa0x_supplies[] = {
	{ .supply = "vcc" },
	{ .supply = "vsp" },
	{ .supply = "vsn" },
};

static inline struct s6d2aa0x *to_s6d2aa0x(struct drm_panel *panel)
{
	return container_of(panel, struct s6d2aa0x, panel);
}

static void s6d2aa0x_reset(struct s6d2aa0x *ctx)
{
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	msleep(20);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	msleep(100);
}

static int s6d2aa0x_on(struct s6d2aa0x *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	ctx->dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x11, 0x00);
	mipi_dsi_msleep(&dsi_ctx, 120);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb0, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc9,
				     0x0c, 0x19, 0x1c, 0x28, 0x32, 0x3e, 0x48,
				     0x57, 0x3b, 0x43, 0x51, 0x5f, 0x68, 0x70,
				     0x7f, 0x0c, 0x19, 0x1c, 0x28, 0x32, 0x3e,
				     0x48, 0x57, 0x3b, 0x43, 0x51, 0x5f, 0x68,
				     0x70, 0x7f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xca,
				     0x01, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00,
				     0x00, 0xff, 0x00, 0xfe, 0xb6, 0x00, 0x00,
				     0x00, 0x00, 0x00, 0xfc, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb7,
				     0x78, 0x64, 0x10, 0x64, 0xab);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb8,
				     0xb4, 0x40, 0x43, 0x49, 0x55, 0x62, 0x71,
				     0x82, 0x94, 0xa8, 0xb9, 0xcb, 0xdb, 0xe9,
				     0xf5, 0xfc, 0xff);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb9, 0xb4, 0xa0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xbb, 0x2e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xbc,
				     0x01, 0x38, 0x04, 0x04, 0x00, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_SET_ADDRESS_MODE, 0x00);
	mipi_dsi_msleep(&dsi_ctx, 20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_WRITE_CONTROL_DISPLAY,
				     0x26);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_WRITE_POWER_SAVE, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_SET_CABC_MIN_BRIGHTNESS,
				     0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x29, 0x00);
	mipi_dsi_usleep_range(&dsi_ctx, 10000, 11000);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb0, 0x00);

	return dsi_ctx.accum_err;
}

static int s6d2aa0x_off(struct s6d2aa0x *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	ctx->dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	mipi_dsi_dcs_set_display_off_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 50);
	mipi_dsi_dcs_enter_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 120);

	return dsi_ctx.accum_err;
}

static int s6d2aa0x_prepare(struct drm_panel *panel)
{
	struct s6d2aa0x *ctx = to_s6d2aa0x(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = regulator_bulk_enable(ARRAY_SIZE(s6d2aa0x_supplies), ctx->supplies);
	if (ret < 0) {
		dev_err(dev, "Failed to enable regulators: %d\n", ret);
		return ret;
	}

	s6d2aa0x_reset(ctx);

	ret = s6d2aa0x_on(ctx);
	if (ret < 0) {
		dev_err(dev, "Failed to initialize panel: %d\n", ret);
		gpiod_set_value_cansleep(ctx->reset_gpio, 1);
		regulator_bulk_disable(ARRAY_SIZE(s6d2aa0x_supplies), ctx->supplies);
		return ret;
	}

	return 0;
}

static int s6d2aa0x_unprepare(struct drm_panel *panel)
{
	struct s6d2aa0x *ctx = to_s6d2aa0x(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = s6d2aa0x_off(ctx);
	if (ret < 0)
		dev_err(dev, "Failed to un-initialize panel: %d\n", ret);

	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	regulator_bulk_disable(ARRAY_SIZE(s6d2aa0x_supplies), ctx->supplies);

	return 0;
}

static const struct drm_display_mode s6d2aa0x_mode = {
	.clock = (720 + 134 + 4 + 117) * (1280 + 8 + 2 + 6) * 60 / 1000,
	.hdisplay = 720,
	.hsync_start = 720 + 134,
	.hsync_end = 720 + 134 + 4,
	.htotal = 720 + 134 + 4 + 117,
	.vdisplay = 1280,
	.vsync_start = 1280 + 8,
	.vsync_end = 1280 + 8 + 2,
	.vtotal = 1280 + 8 + 2 + 6,
	.width_mm = 69,
	.height_mm = 124,
	.type = DRM_MODE_TYPE_DRIVER,
};

static int s6d2aa0x_get_modes(struct drm_panel *panel,
			      struct drm_connector *connector)
{
	return drm_connector_helper_get_modes_fixed(connector, &s6d2aa0x_mode);
}

static const struct drm_panel_funcs s6d2aa0x_panel_funcs = {
	.prepare = s6d2aa0x_prepare,
	.unprepare = s6d2aa0x_unprepare,
	.get_modes = s6d2aa0x_get_modes,
};

static int s6d2aa0x_bl_update_status(struct backlight_device *bl)
{
	struct mipi_dsi_device *dsi = bl_get_data(bl);
	struct s6d2aa0x *ctx = mipi_dsi_get_drvdata(dsi);
	u16 brightness = backlight_get_brightness(bl);
	int ret;

	gpiod_set_value_cansleep(ctx->backlight_gpio, !!brightness);

	dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	ret = mipi_dsi_dcs_set_display_brightness(dsi, brightness);
	if (ret < 0)
		return ret;

	dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	return 0;
}

static const struct backlight_ops s6d2aa0x_bl_ops = {
	.update_status = s6d2aa0x_bl_update_status,
};

static struct backlight_device *
s6d2aa0x_create_backlight(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	const struct backlight_properties props = {
		.type = BACKLIGHT_RAW,
		.brightness = 255,
		.max_brightness = 255,
	};

	return devm_backlight_device_register(dev, dev_name(dev), dev, dsi,
					      &s6d2aa0x_bl_ops, &props);
}

static int s6d2aa0x_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct s6d2aa0x *ctx;
	int ret;

	ctx = devm_drm_panel_alloc(dev, struct s6d2aa0x, panel,
				   &s6d2aa0x_panel_funcs,
				   DRM_MODE_CONNECTOR_DSI);
	if (IS_ERR(ctx))
		return PTR_ERR(ctx);

	ret = devm_regulator_bulk_get_const(dev,
					    ARRAY_SIZE(s6d2aa0x_supplies),
					    s6d2aa0x_supplies,
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
			  MIPI_DSI_MODE_NO_EOT_PACKET | MIPI_DSI_CLOCK_NON_CONTINUOUS;

	ctx->panel.prepare_prev_first = true;

	ctx->panel.backlight = s6d2aa0x_create_backlight(dsi);
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

static void s6d2aa0x_remove(struct mipi_dsi_device *dsi)
{
	struct s6d2aa0x *ctx = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "Failed to detach from DSI host: %d\n", ret);

	drm_panel_remove(&ctx->panel);
}

static const struct of_device_id s6d2aa0x_of_match[] = {
	{ .compatible = "samsung,s6d2aa0x62-lpm053a250a" }, // FIXME
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, s6d2aa0x_of_match);

static struct mipi_dsi_driver s6d2aa0x_driver = {
	.probe = s6d2aa0x_probe,
	.remove = s6d2aa0x_remove,
	.driver = {
		.name = "panel-samsung-s6d2aa0x62-lpm053a250a",
		.of_match_table = s6d2aa0x_of_match,
	},
};
module_mipi_dsi_driver(s6d2aa0x_driver);

MODULE_AUTHOR("linux-mdss-dsi-panel-driver-generator <fix@me>"); // FIXME
MODULE_DESCRIPTION("DRM driver for s6d2aa0x hd video mode dsi panel");
MODULE_LICENSE("GPL");
