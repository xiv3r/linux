# SPDX-License-Identifier: GPL-2.0
#
# Copyright (C) 2002 - 2007 Jeff Dike (jdike@{addtoit,linux.intel}.com)
#

ifeq ($(CONFIG_X86_32),y)
	BITS := 32
else
	BITS := 64
endif

obj-y = bugs_$(BITS).o delay.o fault.o \
	ptrace.o ptrace_$(BITS).o ptrace_user.o setjmp_$(BITS).o signal.o \
	stub_segv.o \
	sys_call_table_$(BITS).o sysrq_$(BITS).o tls_$(BITS).o \
	mem_$(BITS).o subarch.o os-Linux/

ifeq ($(CONFIG_X86_32),y)

obj-y += syscalls_32.o
obj-$(CONFIG_ELF_CORE) += elfcore.o

subarch-y = ../lib/string_32.o ../lib/atomic64_32.o ../lib/atomic64_cx8_32.o
subarch-y += ../lib/cmpxchg8b_emu.o ../lib/atomic64_386_32.o
subarch-y += ../lib/checksum_32.o
subarch-y += ../kernel/sys_ia32.o

else

obj-y += syscalls_64.o vdso/

subarch-y = ../lib/csum-partial_64.o ../lib/memcpy_64.o \
	../lib/memmove_64.o ../lib/memset_64.o

endif

subarch-$(CONFIG_MODULES) += ../kernel/module.o

USER_OBJS := bugs_$(BITS).o ptrace_user.o fault.o

$(obj)/user-offsets.s: c_flags = -Wp,-MD,$(depfile) $(USER_CFLAGS) \
	-Iarch/x86/include/generated
targets += user-offsets.s

include/generated/user_constants.h: $(obj)/user-offsets.s FORCE
	$(call filechk,offsets,__USER_CONSTANT_H__)

UNPROFILE_OBJS := stub_segv.o
CFLAGS_stub_segv.o := $(CFLAGS_NO_HARDENING)

include $(srctree)/arch/um/scripts/Makefile.rules
