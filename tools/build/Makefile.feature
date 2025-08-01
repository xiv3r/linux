# SPDX-License-Identifier: GPL-2.0-only
feature_dir := $(srctree)/tools/build/feature

ifneq ($(OUTPUT),)
  OUTPUT_FEATURES = $(OUTPUT)feature/
  $(shell mkdir -p $(OUTPUT_FEATURES))
endif

feature_check = $(eval $(feature_check_code))
define feature_check_code
  feature-$(1) := $(shell $(MAKE) OUTPUT=$(OUTPUT_FEATURES) CC="$(CC)" CXX="$(CXX)" CFLAGS="$(EXTRA_CFLAGS) $(FEATURE_CHECK_CFLAGS-$(1))" CXXFLAGS="$(EXTRA_CXXFLAGS) $(FEATURE_CHECK_CXXFLAGS-$(1))" LDFLAGS="$(LDFLAGS) $(FEATURE_CHECK_LDFLAGS-$(1))" -C $(feature_dir) $(OUTPUT_FEATURES)test-$1.bin >/dev/null 2>/dev/null && echo 1 || echo 0)
endef

feature_set = $(eval $(feature_set_code))
define feature_set_code
  feature-$(1) := 1
endef

#
# Build the feature check binaries in parallel, ignore errors, ignore return value and suppress output:
#

#
# Note that this is not a complete list of all feature tests, just
# those that are typically built on a fully configured system.
#
# [ Feature tests not mentioned here have to be built explicitly in
#   the rule that uses them - an example for that is the 'bionic'
#   feature check. ]
#
# These + the ones in FEATURE_TESTS_EXTRA are included in
# tools/build/feature/test-all.c and we try to build it all together
# then setting all those features to '1' meaning they are all enabled.
#
# There are things like fortify-source that will be set to 1 because test-all
# is built with the flags needed to test if its enabled, resulting in
#
#   $ rm -rf /tmp/b ; mkdir /tmp/b ; make -C tools/perf O=/tmp/b feature-dump
#   $ grep fortify-source /tmp/b/FEATURE-DUMP
#   feature-fortify-source=1
#   $
#
#   All the others should have lines in tools/build/feature/test-all.c like:
#
#    #define main main_test_disassembler_init_styled
#    # include "test-disassembler-init-styled.c"
#    #undef main
#
#    #define main main_test_libzstd
#    # include "test-libzstd.c"
#    #undef main
#
#    int main(int argc, char *argv[])
#    {
#      main_test_disassembler_four_args();
#      main_test_libzstd();
#      return 0;
#    }
#
#    If the sample above works, then we end up with these lines in the FEATURE-DUMP
#    file:
#
#    feature-disassembler-four-args=1
#    feature-libzstd=1
#
FEATURE_TESTS_BASIC :=                  \
        backtrace                       \
        libdw                           \
        eventfd                         \
        fortify-source                  \
        get_current_dir_name            \
        gettid				\
        glibc                           \
        libbfd                          \
        libbfd-buildid			\
        libelf                          \
        libelf-getphdrnum               \
        libelf-gelf_getnote             \
        libelf-getshdrstrndx            \
        libelf-zstd                     \
        libnuma                         \
        numa_num_possible_cpus          \
        libperl                         \
        libpython                       \
        libslang                        \
        libtraceevent                   \
        libtracefs                      \
        libcpupower                     \
        pthread-attr-setaffinity-np     \
        pthread-barrier     		\
        reallocarray                    \
        stackprotector-all              \
        timerfd                         \
        zlib                            \
        lzma                            \
        get_cpuid                       \
        bpf                             \
        scandirat			\
        sched_getcpu			\
        sdt				\
        setns				\
        libaio				\
        libzstd				\
        disassembler-four-args		\
        disassembler-init-styled	\
        file-handle

# FEATURE_TESTS_BASIC + FEATURE_TESTS_EXTRA is the complete list
# of all feature tests
FEATURE_TESTS_EXTRA :=                  \
         bionic                         \
         compile-32                     \
         compile-x32                    \
         cplus-demangle                 \
         cxa-demangle                   \
         gtk2                           \
         gtk2-infobar                   \
         hello                          \
         libbabeltrace                  \
         libcapstone                    \
         libbfd-liberty                 \
         libbfd-liberty-z               \
         libopencsd                     \
         cxx                            \
         llvm                           \
         clang                          \
         libbpf                         \
         libbpf-strings                 \
         libpfm4                        \
         libdebuginfod			\
         clang-bpf-co-re		\
         bpftool-skeletons


FEATURE_TESTS ?= $(FEATURE_TESTS_BASIC)

ifeq ($(FEATURE_TESTS),all)
  FEATURE_TESTS := $(FEATURE_TESTS_BASIC) $(FEATURE_TESTS_EXTRA)
endif

FEATURE_DISPLAY ?=              \
         libdw                  \
         glibc                  \
         libelf                 \
         libnuma                \
         numa_num_possible_cpus \
         libperl                \
         libpython              \
         libcapstone            \
         llvm-perf              \
         zlib                   \
         lzma                   \
         get_cpuid              \
         bpf			\
         libaio			\
         libzstd

#
# Declare group members of a feature to display the logical OR of the detection
# result instead of each member result.
#
FEATURE_GROUP_MEMBERS-libbfd = libbfd-liberty libbfd-liberty-z

#
# Declare list of feature dependency packages that provide pkg-config files.
#
FEATURE_PKG_CONFIG ?=           \
         libtraceevent          \
         libtracefs

feature_pkg_config = $(eval $(feature_pkg_config_code))
define feature_pkg_config_code
  FEATURE_CHECK_CFLAGS-$(1) := $(shell $(PKG_CONFIG) --cflags $(1) 2>/dev/null)
  FEATURE_CHECK_LDFLAGS-$(1) := $(shell $(PKG_CONFIG) --libs $(1) 2>/dev/null)
endef

# Set FEATURE_CHECK_(C|LD)FLAGS-$(package) for packages using pkg-config.
ifneq ($(PKG_CONFIG),)
  $(foreach package,$(FEATURE_PKG_CONFIG),$(call feature_pkg_config,$(package)))
endif

# Set FEATURE_CHECK_(C|LD)FLAGS-all for all FEATURE_TESTS features.
# If in the future we need per-feature checks/flags for features not
# mentioned in this list we need to refactor this ;-).
set_test_all_flags = $(eval $(set_test_all_flags_code))
define set_test_all_flags_code
  FEATURE_CHECK_CFLAGS-all  += $(FEATURE_CHECK_CFLAGS-$(1))
  FEATURE_CHECK_LDFLAGS-all += $(FEATURE_CHECK_LDFLAGS-$(1))
endef

$(foreach feat,$(FEATURE_TESTS),$(call set_test_all_flags,$(feat)))

#
# Special fast-path for the 'all features are available' case:
#
$(call feature_check,all,$(MSG))

#
# Just in case the build freshly failed, make sure we print the
# feature matrix:
#
ifeq ($(feature-all), 1)
  #
  # test-all.c passed - just set all the core feature flags to 1:
  #
  $(foreach feat,$(FEATURE_TESTS),$(call feature_set,$(feat)))
  #
  # test-all.c does not comprise these tests, so we need to
  # for this case to get features proper values
  #
  $(call feature_check,compile-32)
  $(call feature_check,compile-x32)
  $(call feature_check,bionic)
  $(call feature_check,libbabeltrace)
else
  $(foreach feat,$(FEATURE_TESTS),$(call feature_check,$(feat)))
endif

#
# Print the result of the feature test:
#
feature_print_status = $(eval $(feature_print_status_code))

feature_group = $(eval $(feature_gen_group)) $(GROUP)

define feature_gen_group
  GROUP := $(1)
  ifneq ($(feature_verbose),1)
    GROUP += $(FEATURE_GROUP_MEMBERS-$(1))
  endif
endef

define feature_print_status_code
  ifneq (,$(filter 1,$(foreach feat,$(call feature_group,$(feat)),$(feature-$(feat)))))
    MSG = $(shell printf '...%40s: [ \033[32mon\033[m  ]' $(1))
  else
    MSG = $(shell printf '...%40s: [ \033[31mOFF\033[m ]' $(1))
  endif
endef

feature_print_text = $(eval $(feature_print_text_code))
define feature_print_text_code
    MSG = $(shell printf '...%40s: %s' $(1) $(2))
endef

#
# generates feature value assignment for name, like:
#   $(call feature_assign,libdw) == feature-libdw=1
#
feature_assign = feature-$(1)=$(feature-$(1))

FEATURE_DUMP_FILENAME = $(OUTPUT)FEATURE-DUMP$(FEATURE_USER)
FEATURE_DUMP := $(shell touch $(FEATURE_DUMP_FILENAME); cat $(FEATURE_DUMP_FILENAME))

feature_dump_check = $(eval $(feature_dump_check_code))
define feature_dump_check_code
  ifeq ($(findstring $(1),$(FEATURE_DUMP)),)
    $(2) := 1
  endif
endef

#
# First check if any test from FEATURE_DISPLAY
# and set feature_display := 1 if it does
$(foreach feat,$(FEATURE_DISPLAY),$(call feature_dump_check,$(call feature_assign,$(feat)),feature_display))

#
# Now also check if any other test changed,
# so we force FEATURE-DUMP generation
$(foreach feat,$(FEATURE_TESTS),$(call feature_dump_check,$(call feature_assign,$(feat)),feature_dump_changed))

# The $(feature_display) controls the default detection message
# output. It's set if:
# - detected features differes from stored features from
#   last build (in $(FEATURE_DUMP_FILENAME) file)
# - one of the $(FEATURE_DISPLAY) is not detected
# - VF is enabled

ifeq ($(feature_dump_changed),1)
  $(shell rm -f $(FEATURE_DUMP_FILENAME))
  $(foreach feat,$(FEATURE_TESTS),$(shell echo "$(call feature_assign,$(feat))" >> $(FEATURE_DUMP_FILENAME)))
endif

feature_display_check = $(eval $(feature_check_display_code))
define feature_check_display_code
  ifneq ($(feature-$(1)), 1)
    feature_display := 1
  endif
endef

$(foreach feat,$(FEATURE_DISPLAY),$(call feature_display_check,$(feat)))

ifeq ($(VF),1)
  feature_display := 1
  feature_verbose := 1
endif

ifneq ($(feature_verbose),1)
  #
  # Determine the features to omit from the displayed message, as only the
  # logical OR of the detection result will be shown.
  #
  FEATURE_OMIT := $(foreach feat,$(FEATURE_DISPLAY),$(FEATURE_GROUP_MEMBERS-$(feat)))
endif

feature_display_entries = $(eval $(feature_display_entries_code))
define feature_display_entries_code
  ifeq ($(feature_display),1)
    $$(info )
    $$(info Auto-detecting system features:)
    $(foreach feat,$(filter-out $(FEATURE_OMIT),$(FEATURE_DISPLAY)),$(call feature_print_status,$(feat),) $$(info $(MSG)))
  endif

  ifeq ($(feature_verbose),1)
    $(eval TMP := $(filter-out $(FEATURE_DISPLAY),$(FEATURE_TESTS)))
    $(foreach feat,$(TMP),$(call feature_print_status,$(feat),) $$(info $(MSG)))
  endif
endef

ifeq ($(FEATURE_DISPLAY_DEFERRED),)
  $(call feature_display_entries)
  $(info )
endif
