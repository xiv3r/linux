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

struct samsung {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct regulator_bulk_data *supplies;
	struct gpio_desc *reset_gpio;
};

static const struct regulator_bulk_data samsung_supplies[] = {
	{ .supply = "vddio" },
	{ .supply = "vdd" },
};

static inline struct samsung *to_samsung(struct drm_panel *panel)
{
	return container_of(panel, struct samsung, panel);
}

static void samsung_reset(struct samsung *ctx)
{
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(5000, 6000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	usleep_range(1000, 2000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(10000, 11000);
}

static int samsung_on(struct samsung *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	ctx->dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf0, 0x5a, 0x5a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfc, 0x5a, 0x5a);
	mipi_dsi_dcs_exit_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 120);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb8, 0x38, 0x0b, 0x30);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xca,
				     0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x80,
				     0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
				     0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
				     0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
				     0x80, 0x80, 0x00, 0x00, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb2,
				     0x40, 0x08, 0x20, 0x00, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb6, 0x28, 0x0b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_WRITE_POWER_SAVE, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf7, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfc, 0xa5, 0xa5);
	mipi_dsi_dcs_set_display_on_multi(&dsi_ctx);

	return dsi_ctx.accum_err;
}

static int samsung_off(struct samsung *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	ctx->dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	mipi_dsi_dcs_set_display_off_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 35);
	mipi_dsi_dcs_enter_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 120);

	return dsi_ctx.accum_err;
}

static int samsung_prepare(struct drm_panel *panel)
{
	struct samsung *ctx = to_samsung(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = regulator_bulk_enable(ARRAY_SIZE(samsung_supplies), ctx->supplies);
	if (ret < 0) {
		dev_err(dev, "Failed to enable regulators: %d\n", ret);
		return ret;
	}

	samsung_reset(ctx);

	ret = samsung_on(ctx);
	if (ret < 0) {
		dev_err(dev, "Failed to initialize panel: %d\n", ret);
		gpiod_set_value_cansleep(ctx->reset_gpio, 1);
		regulator_bulk_disable(ARRAY_SIZE(samsung_supplies), ctx->supplies);
		return ret;
	}

	return 0;
}

static int samsung_unprepare(struct drm_panel *panel)
{
	struct samsung *ctx = to_samsung(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = samsung_off(ctx);
	if (ret < 0)
		dev_err(dev, "Failed to un-initialize panel: %d\n", ret);

	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	regulator_bulk_disable(ARRAY_SIZE(samsung_supplies), ctx->supplies);

	return 0;
}

static const struct drm_display_mode samsung_mode = {
	.clock = (480 + 90 + 2 + 50) * (800 + 13 + 1 + 2) * 60 / 1000,
	.hdisplay = 480,
	.hsync_start = 480 + 90,
	.hsync_end = 480 + 90 + 2,
	.htotal = 480 + 90 + 2 + 50,
	.vdisplay = 800,
	.vsync_start = 800 + 13,
	.vsync_end = 800 + 13 + 1,
	.vtotal = 800 + 13 + 1 + 2,
	.width_mm = 56,
	.height_mm = 94,
	.type = DRM_MODE_TYPE_DRIVER,
};

static int samsung_get_modes(struct drm_panel *panel,
			     struct drm_connector *connector)
{
	return drm_connector_helper_get_modes_fixed(connector, &samsung_mode);
}

static const struct drm_panel_funcs samsung_panel_funcs = {
	.prepare = samsung_prepare,
	.unprepare = samsung_unprepare,
	.get_modes = samsung_get_modes,
};

static int samsung_bl_update_status(struct backlight_device *bl)
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

// TODO: Check if /sys/class/backlight/.../actual_brightness actually returns
// correct values. If not, remove this function.
static int samsung_bl_get_brightness(struct backlight_device *bl)
{
	struct mipi_dsi_device *dsi = bl_get_data(bl);
	u16 brightness;
	int ret;

	dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	ret = mipi_dsi_dcs_get_display_brightness(dsi, &brightness);
	if (ret < 0)
		return ret;

	dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	return brightness & 0xff;
}

static const struct backlight_ops samsung_bl_ops = {
	.update_status = samsung_bl_update_status,
	.get_brightness = samsung_bl_get_brightness,
};

static struct backlight_device *
samsung_create_backlight(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	const struct backlight_properties props = {
		.type = BACKLIGHT_RAW,
		.brightness = 255,
		.max_brightness = 255,
	};

	return devm_backlight_device_register(dev, dev_name(dev), dev, dsi,
					      &samsung_bl_ops, &props);
}

static int samsung_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct samsung *ctx;
	int ret;

	ctx = devm_drm_panel_alloc(dev, struct samsung, panel,
				   &samsung_panel_funcs,
				   DRM_MODE_CONNECTOR_DSI);
	if (IS_ERR(ctx))
		return PTR_ERR(ctx);

	ret = devm_regulator_bulk_get_const(dev,
					    ARRAY_SIZE(samsung_supplies),
					    samsung_supplies,
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
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_BURST |
			  MIPI_DSI_MODE_NO_EOT_PACKET;

	ctx->panel.prepare_prev_first = true;

	ctx->panel.backlight = samsung_create_backlight(dsi);
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

static void samsung_remove(struct mipi_dsi_device *dsi)
{
	struct samsung *ctx = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "Failed to detach from DSI host: %d\n", ret);

	drm_panel_remove(&ctx->panel);
}

static const struct of_device_id samsung_of_match[] = {
	{ .compatible = "samsung,s6288a0" }, // FIXME
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, samsung_of_match);

static struct mipi_dsi_driver samsung_driver = {
	.probe = samsung_probe,
	.remove = samsung_remove,
	.driver = {
		.name = "panel-samsung-s6288a0",
		.of_match_table = samsung_of_match,
	},
};
module_mipi_dsi_driver(samsung_driver);

MODULE_AUTHOR("linux-mdss-dsi-panel-driver-generator <fix@me>"); // FIXME
MODULE_DESCRIPTION("DRM driver for Samsung WVGA video mode dsi panel");
MODULE_LICENSE("GPL");
