# SPDX-License-Identifier: GPL-2.0-only
obj-$(CONFIG_MLX5_INFINIBAND) += mlx5_ib.o

mlx5_ib-y := ah.o \
	     cmd.o \
	     cong.o \
	     counters.o \
	     cq.o \
	     data_direct.o \
	     dm.o \
	     dmah.o \
	     doorbell.o \
	     fs.o \
	     gsi.o \
	     ib_virt.o \
	     mad.o \
	     main.o \
	     mem.o \
	     mr.o \
	     qp.o \
	     qpc.o \
	     restrack.o \
	     srq.o \
	     srq_cmd.o \
	     umr.o \
	     wr.o

mlx5_ib-$(CONFIG_INFINIBAND_ON_DEMAND_PAGING) += odp.o
mlx5_ib-$(CONFIG_MLX5_ESWITCH) += ib_rep.o
mlx5_ib-$(CONFIG_INFINIBAND_USER_ACCESS) += devx.o \
					    qos.o \
					    std_types.o
mlx5_ib-$(CONFIG_MLX5_MACSEC) += macsec.o
