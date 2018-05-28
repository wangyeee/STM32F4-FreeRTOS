CFLAGS_chalk = $(CFLAGS_snowy_family)
CFLAGS_chalk += -Ihw/platform/chalk
CFLAGS_chalk += -DHSI_VALUE=16000000 -DREBBLE_PLATFORM=chalk -DREBBLE_PLATFORM_CHALK

SRCS_chalk = $(SRCS_snowy_family)
SRCS_chalk += hw/platform/chalk/chalk.c
SRCS_chalk += hw/platform/chalk/chalk_bluetooth.c
SRCS_chalk += Resources/chalk_fpga.bin

LDFLAGS_chalk = $(LDFLAGS_snowy_family)
LIBS_chalk = $(LIBS_snowy_family)

QEMUFLAGS_chalk = -machine pebble-s4-bb -cpu cortex-m4
QEMUSPITYPE_chalk = pflash
QEMUPACKSIZE_chalk = 512000
QEMUPACKOFS_chalk = 3670016
QEMUSPINAME_chalk = chalk/3.0

HWREV_chalk = spalding

PLATFORMS += chalk
