// SPDX-License-Identifier: GPL-2.0
/*
 * Simple memory-mapped device MDIO MUX driver
 *
 * Author: Timur Tabi <timur@freescale.com>
 *
 * Copyright 2012 Freescale Semiconductor, Inc.
 */

#include <linux/device.h>
#include <linux/mdio-mux.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/of_mdio.h>
#include <linux/phy.h>
#include <linux/platform_device.h>

struct mdio_mux_mmioreg_state {
	void *mux_handle;
	phys_addr_t phys;
	unsigned int iosize;
	unsigned int mask;
};

/*
 * MDIO multiplexing switch function
 *
 * This function is called by the mdio-mux layer when it thinks the mdio bus
 * multiplexer needs to switch.
 *
 * 'current_child' is the current value of the mux register (masked via
 * s->mask).
 *
 * 'desired_child' is the value of the 'reg' property of the target child MDIO
 * node.
 *
 * The first time this function is called, current_child == -1.
 *
 * If current_child == desired_child, then the mux is already set to the
 * correct bus.
 */
static int mdio_mux_mmioreg_switch_fn(int current_child, int desired_child,
				      void *data)
{
	struct mdio_mux_mmioreg_state *s = data;

	if (current_child ^ desired_child) {
		void __iomem *p = ioremap(s->phys, s->iosize);
		if (!p)
			return -ENOMEM;

		switch (s->iosize) {
		case sizeof(uint8_t): {
			uint8_t x, y;

			x = ioread8(p);
			y = (x & ~s->mask) | desired_child;
			if (x != y) {
				iowrite8((x & ~s->mask) | desired_child, p);
				pr_debug("%s: %02x -> %02x\n", __func__, x, y);
			}

			break;
		}
		case sizeof(uint16_t): {
			uint16_t x, y;

			x = ioread16(p);
			y = (x & ~s->mask) | desired_child;
			if (x != y) {
				iowrite16((x & ~s->mask) | desired_child, p);
				pr_debug("%s: %04x -> %04x\n", __func__, x, y);
			}

			break;
		}
		case sizeof(uint32_t): {
			uint32_t x, y;

			x = ioread32(p);
			y = (x & ~s->mask) | desired_child;
			if (x != y) {
				iowrite32((x & ~s->mask) | desired_child, p);
				pr_debug("%s: %08x -> %08x\n", __func__, x, y);
			}

			break;
		}
		}

		iounmap(p);
	}

	return 0;
}

static int mdio_mux_mmioreg_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct mdio_mux_mmioreg_state *s;
	struct resource res;
	const __be32 *iprop;
	int len, ret;

	dev_dbg(&pdev->dev, "probing node %pOF\n", np);

	s = devm_kzalloc(&pdev->dev, sizeof(*s), GFP_KERNEL);
	if (!s)
		return -ENOMEM;

	ret = of_address_to_resource(np, 0, &res);
	if (ret)
		return dev_err_probe(&pdev->dev, ret,
				     "could not obtain memory map for node %pOF\n", np);
	s->phys = res.start;

	s->iosize = resource_size(&res);
	if (s->iosize != sizeof(uint8_t) &&
	    s->iosize != sizeof(uint16_t) &&
	    s->iosize != sizeof(uint32_t))
		return dev_err_probe(&pdev->dev, -EINVAL,
				     "only 8/16/32-bit registers are supported\n");

	iprop = of_get_property(np, "mux-mask", &len);
	if (!iprop || len != sizeof(uint32_t))
		return dev_err_probe(&pdev->dev, -ENODEV,
				     "missing or invalid mux-mask property\n");
	if (be32_to_cpup(iprop) >= BIT(s->iosize * 8))
		return dev_err_probe(&pdev->dev, -EINVAL,
				     "only 8/16/32-bit registers are supported\n");
	s->mask = be32_to_cpup(iprop);

	/*
	 * Verify that the 'reg' property of each child MDIO bus does not
	 * set any bits outside of the 'mask'.
	 */
	for_each_available_child_of_node_scoped(np, np2) {
		u64 reg;

		if (of_property_read_reg(np2, 0, &reg, NULL))
			return dev_err_probe(&pdev->dev, -ENODEV,
					     "mdio-mux child node %pOF is missing a 'reg' property\n",
					     np2);
		if ((u32)reg & ~s->mask)
			return dev_err_probe(&pdev->dev, -ENODEV,
					     "mdio-mux child node %pOF has a 'reg' value with unmasked bits\n",
					     np2);
	}

	ret = mdio_mux_init(&pdev->dev, pdev->dev.of_node,
			    mdio_mux_mmioreg_switch_fn,
			    &s->mux_handle, s, NULL);
	if (ret)
		return dev_err_probe(&pdev->dev, ret,
				     "failed to register mdio-mux bus %pOF\n", np);

	pdev->dev.platform_data = s;

	return 0;
}

static void mdio_mux_mmioreg_remove(struct platform_device *pdev)
{
	struct mdio_mux_mmioreg_state *s = dev_get_platdata(&pdev->dev);

	mdio_mux_uninit(s->mux_handle);
}

static const struct of_device_id mdio_mux_mmioreg_match[] = {
	{
		.compatible = "mdio-mux-mmioreg",
	},
	{},
};
MODULE_DEVICE_TABLE(of, mdio_mux_mmioreg_match);

static struct platform_driver mdio_mux_mmioreg_driver = {
	.driver = {
		.name		= "mdio-mux-mmioreg",
		.of_match_table = mdio_mux_mmioreg_match,
	},
	.probe		= mdio_mux_mmioreg_probe,
	.remove		= mdio_mux_mmioreg_remove,
};

module_platform_driver(mdio_mux_mmioreg_driver);

MODULE_AUTHOR("Timur Tabi <timur@freescale.com>");
MODULE_DESCRIPTION("Memory-mapped device MDIO MUX driver");
MODULE_LICENSE("GPL v2");
