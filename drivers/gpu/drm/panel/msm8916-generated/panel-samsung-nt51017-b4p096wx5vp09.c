// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2025 FIXME
// Generated with linux-mdss-dsi-panel-driver-generator from vendor device tree:
//   Copyright (c) 2013, The Linux Foundation. All rights reserved. (FIXME)

#include <linux/delay.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/regulator/consumer.h>

#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>
#include <drm/drm_probe_helper.h>

struct nt51017 {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct regulator *supply;
};

static inline struct nt51017 *to_nt51017(struct drm_panel *panel)
{
	return container_of(panel, struct nt51017, panel);
}

static int nt51017_on(struct nt51017 *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x83, 0x96);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x84, 0x69);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x92, 0x19);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x95, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x83, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x84, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x90, 0x77);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x94, 0xff);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x96, 0xff);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x91, 0xfd);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x90, 0x77);

	return dsi_ctx.accum_err;
}

static int nt51017_off(struct nt51017 *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	return dsi_ctx.accum_err;
}

static int nt51017_prepare(struct drm_panel *panel)
{
	struct nt51017 *ctx = to_nt51017(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = regulator_enable(ctx->supply);
	if (ret < 0) {
		dev_err(dev, "Failed to enable regulator: %d\n", ret);
		return ret;
	}

	msleep(30);

	ret = nt51017_on(ctx);
	if (ret < 0) {
		dev_err(dev, "Failed to initialize panel: %d\n", ret);
		regulator_disable(ctx->supply);
		return ret;
	}

	return 0;
}

static int nt51017_unprepare(struct drm_panel *panel)
{
	struct nt51017 *ctx = to_nt51017(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = nt51017_off(ctx);
	if (ret < 0)
		dev_err(dev, "Failed to un-initialize panel: %d\n", ret);

	regulator_disable(ctx->supply);

	return 0;
}

static const struct drm_display_mode nt51017_mode = {
	.clock = (800 + 152 + 8 + 128) * (1280 + 18 + 1 + 23) * 60 / 1000,
	.hdisplay = 800,
	.hsync_start = 800 + 152,
	.hsync_end = 800 + 152 + 8,
	.htotal = 800 + 152 + 8 + 128,
	.vdisplay = 1280,
	.vsync_start = 1280 + 18,
	.vsync_end = 1280 + 18 + 1,
	.vtotal = 1280 + 18 + 1 + 23,
	.width_mm = 129,
	.height_mm = 206,
	.type = DRM_MODE_TYPE_DRIVER,
};

static int nt51017_get_modes(struct drm_panel *panel,
			     struct drm_connector *connector)
{
	return drm_connector_helper_get_modes_fixed(connector, &nt51017_mode);
}

static const struct drm_panel_funcs nt51017_panel_funcs = {
	.prepare = nt51017_prepare,
	.unprepare = nt51017_unprepare,
	.get_modes = nt51017_get_modes,
};

static int nt51017_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct nt51017 *ctx;
	int ret;

	ctx = devm_drm_panel_alloc(dev, struct nt51017, panel,
				   &nt51017_panel_funcs,
				   DRM_MODE_CONNECTOR_DSI);
	if (IS_ERR(ctx))
		return PTR_ERR(ctx);

	ctx->supply = devm_regulator_get(dev, "lcd");
	if (IS_ERR(ctx->supply))
		return dev_err_probe(dev, PTR_ERR(ctx->supply),
				     "Failed to get lcd regulator\n");

	ctx->dsi = dsi;
	mipi_dsi_set_drvdata(dsi, ctx);

	dsi->lanes = 4;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_BURST |
			  MIPI_DSI_MODE_NO_EOT_PACKET |
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

static void nt51017_remove(struct mipi_dsi_device *dsi)
{
	struct nt51017 *ctx = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "Failed to detach from DSI host: %d\n", ret);

	drm_panel_remove(&ctx->panel);
}

static const struct of_device_id nt51017_of_match[] = {
	{ .compatible = "samsung,nt51017-b4p096wx5vp09" }, // FIXME
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, nt51017_of_match);

static struct mipi_dsi_driver nt51017_driver = {
	.probe = nt51017_probe,
	.remove = nt51017_remove,
	.driver = {
		.name = "panel-samsung-nt51017-b4p096wx5vp09",
		.of_match_table = nt51017_of_match,
	},
};
module_mipi_dsi_driver(nt51017_driver);

MODULE_AUTHOR("linux-mdss-dsi-panel-driver-generator <fix@me>"); // FIXME
MODULE_DESCRIPTION("DRM driver for NT51017 wxga video mode dsi panel");
MODULE_LICENSE("GPL");
