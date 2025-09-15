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

#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>
#include <drm/drm_probe_helper.h>

struct tianma_nt35596_5p5xa {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct regulator_bulk_data *supplies;
	struct gpio_desc *reset_gpio;
};

static const struct regulator_bulk_data tianma_nt35596_5p5xa_supplies[] = {
	{ .supply = "vsp" },
	{ .supply = "vsn" },
};

static inline
struct tianma_nt35596_5p5xa *to_tianma_nt35596_5p5xa(struct drm_panel *panel)
{
	return container_of(panel, struct tianma_nt35596_5p5xa, panel);
}

static void tianma_nt35596_5p5xa_reset(struct tianma_nt35596_5p5xa *ctx)
{
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(5000, 6000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	usleep_range(5000, 6000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(15000, 16000);
}

static int tianma_nt35596_5p5xa_on(struct tianma_nt35596_5p5xa *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	ctx->dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xff, 0xee);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x18, 0x40);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x18, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xff, 0x01);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x5c, 0x82);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x5d, 0x82);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x5e, 0x02);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x5f, 0x02);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x60, 0x0a);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xff, 0x03);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x00, 0x50);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x01, 0x50);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x02, 0x50);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x03, 0x50);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x04, 0x50);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x05, 0x50);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x06, 0x50);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x07, 0x50);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x08, 0x50);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x09, 0x46);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x0a, 0x3c);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x0b, 0x32);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x0c, 0x28);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x0d, 0x20);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x0e, 0x18);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x0f, 0x10);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x10, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x11, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x12, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x13, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x32, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x33, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x34, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x35, 0x02);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x36, 0x06);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x37, 0x0d);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x38, 0x0d);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x39, 0x10);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x3a, 0x10);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x3b, 0x0e);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x3f, 0x10);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x40, 0x12);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x41, 0x14);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x42, 0x14);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x43, 0x12);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x44, 0x10);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x45, 0x0e);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x46, 0x0c);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x47, 0x0a);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x48, 0x08);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x49, 0x06);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x4a, 0x04);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x4b, 0x02);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x4c, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x4e, 0x05);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x1a, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x53, 0x77);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x54, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x55, 0x77);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x5b, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x63, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xff, 0x04);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x13, 0xf3);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x14, 0xf2);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x15, 0xf1);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x16, 0xf0);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x21, 0xff);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x22, 0xf4);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x23, 0xe9);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x24, 0xde);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x25, 0xd3);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x26, 0xc8);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x27, 0xbd);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x28, 0xb2);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x29, 0xa7);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x2a, 0x9c);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xff, 0x04);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x07, 0x20);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x08, 0x06);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xff, 0x05);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x23, 0x5c);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x35, 0x5f);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x7a, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x7e, 0x04);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x7f, 0x1a);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x81, 0x03);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x82, 0x02);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x84, 0x03);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x92, 0x63);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xc5, 0x31);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xff, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x35, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x53, 0x24);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x55, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5e, 0x28);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x51, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd3, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd4, 0x08);
	mipi_dsi_dcs_exit_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 120);
	mipi_dsi_dcs_set_display_on_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 40);

	return dsi_ctx.accum_err;
}

static int tianma_nt35596_5p5xa_off(struct tianma_nt35596_5p5xa *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	ctx->dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xff, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_set_display_off_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 40);
	mipi_dsi_dcs_enter_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 120);

	return dsi_ctx.accum_err;
}

static int tianma_nt35596_5p5xa_prepare(struct drm_panel *panel)
{
	struct tianma_nt35596_5p5xa *ctx = to_tianma_nt35596_5p5xa(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = regulator_bulk_enable(ARRAY_SIZE(tianma_nt35596_5p5xa_supplies), ctx->supplies);
	if (ret < 0) {
		dev_err(dev, "Failed to enable regulators: %d\n", ret);
		return ret;
	}

	tianma_nt35596_5p5xa_reset(ctx);

	ret = tianma_nt35596_5p5xa_on(ctx);
	if (ret < 0) {
		dev_err(dev, "Failed to initialize panel: %d\n", ret);
		gpiod_set_value_cansleep(ctx->reset_gpio, 1);
		regulator_bulk_disable(ARRAY_SIZE(tianma_nt35596_5p5xa_supplies), ctx->supplies);
		return ret;
	}

	return 0;
}

static int tianma_nt35596_5p5xa_unprepare(struct drm_panel *panel)
{
	struct tianma_nt35596_5p5xa *ctx = to_tianma_nt35596_5p5xa(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = tianma_nt35596_5p5xa_off(ctx);
	if (ret < 0)
		dev_err(dev, "Failed to un-initialize panel: %d\n", ret);

	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	regulator_bulk_disable(ARRAY_SIZE(tianma_nt35596_5p5xa_supplies), ctx->supplies);

	return 0;
}

static const struct drm_display_mode tianma_nt35596_5p5xa_mode = {
	.clock = (1080 + 184 + 8 + 20) * (1920 + 8 + 2 + 2) * 60 / 1000,
	.hdisplay = 1080,
	.hsync_start = 1080 + 184,
	.hsync_end = 1080 + 184 + 8,
	.htotal = 1080 + 184 + 8 + 20,
	.vdisplay = 1920,
	.vsync_start = 1920 + 8,
	.vsync_end = 1920 + 8 + 2,
	.vtotal = 1920 + 8 + 2 + 2,
	.width_mm = 68,
	.height_mm = 121,
	.type = DRM_MODE_TYPE_DRIVER,
};

static int tianma_nt35596_5p5xa_get_modes(struct drm_panel *panel,
					  struct drm_connector *connector)
{
	return drm_connector_helper_get_modes_fixed(connector, &tianma_nt35596_5p5xa_mode);
}

static const struct drm_panel_funcs tianma_nt35596_5p5xa_panel_funcs = {
	.prepare = tianma_nt35596_5p5xa_prepare,
	.unprepare = tianma_nt35596_5p5xa_unprepare,
	.get_modes = tianma_nt35596_5p5xa_get_modes,
};

static int tianma_nt35596_5p5xa_bl_update_status(struct backlight_device *bl)
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
static int tianma_nt35596_5p5xa_bl_get_brightness(struct backlight_device *bl)
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

static const struct backlight_ops tianma_nt35596_5p5xa_bl_ops = {
	.update_status = tianma_nt35596_5p5xa_bl_update_status,
	.get_brightness = tianma_nt35596_5p5xa_bl_get_brightness,
};

static struct backlight_device *
tianma_nt35596_5p5xa_create_backlight(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	const struct backlight_properties props = {
		.type = BACKLIGHT_RAW,
		.brightness = 255,
		.max_brightness = 255,
	};

	return devm_backlight_device_register(dev, dev_name(dev), dev, dsi,
					      &tianma_nt35596_5p5xa_bl_ops, &props);
}

static int tianma_nt35596_5p5xa_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct tianma_nt35596_5p5xa *ctx;
	int ret;

	ctx = devm_drm_panel_alloc(dev, struct tianma_nt35596_5p5xa, panel,
				   &tianma_nt35596_5p5xa_panel_funcs,
				   DRM_MODE_CONNECTOR_DSI);
	if (IS_ERR(ctx))
		return PTR_ERR(ctx);

	ret = devm_regulator_bulk_get_const(dev,
					    ARRAY_SIZE(tianma_nt35596_5p5xa_supplies),
					    tianma_nt35596_5p5xa_supplies,
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
			  MIPI_DSI_CLOCK_NON_CONTINUOUS;

	ctx->panel.prepare_prev_first = true;

	ctx->panel.backlight = tianma_nt35596_5p5xa_create_backlight(dsi);
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

static void tianma_nt35596_5p5xa_remove(struct mipi_dsi_device *dsi)
{
	struct tianma_nt35596_5p5xa *ctx = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "Failed to detach from DSI host: %d\n", ret);

	drm_panel_remove(&ctx->panel);
}

static const struct of_device_id tianma_nt35596_5p5xa_of_match[] = {
	{ .compatible = "huawei,kiwi-tianma-nt35596" }, // FIXME
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, tianma_nt35596_5p5xa_of_match);

static struct mipi_dsi_driver tianma_nt35596_5p5xa_driver = {
	.probe = tianma_nt35596_5p5xa_probe,
	.remove = tianma_nt35596_5p5xa_remove,
	.driver = {
		.name = "panel-huawei-kiwi-tianma-nt35596",
		.of_match_table = tianma_nt35596_5p5xa_of_match,
	},
};
module_mipi_dsi_driver(tianma_nt35596_5p5xa_driver);

MODULE_AUTHOR("linux-mdss-dsi-panel-driver-generator <fix@me>"); // FIXME
MODULE_DESCRIPTION("DRM driver for TIANMA_NT35596_5P5_1080PXA_VIDEO");
MODULE_LICENSE("GPL");
