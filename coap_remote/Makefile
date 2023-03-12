APPLICATION := coap_remote
BOARD := nucleo-f446re-remote
RIOTBASE := $(CURDIR)/../RIOT
EXTERNAL_BOARD_DIRS += $(CURDIR)/../boards/riot
COMPILE_COMMANDS_PATH ?= $(abspath $(CURDIR)/compile_commands.json)
DEVELHELP ?= 1
QUIET ?= 1

USEMODULE += auto_init_gnrc_netif
USEMODULE += gnrc_icmpv6_echo
USEMODULE += gnrc_ipv6_default
USEMODULE += gnrc_sock_udp
USEMODULE += nanocoap_sock
USEMODULE += netdev_default
USEMODULE += od
USEMODULE += ps
USEMODULE += random
USEMODULE += shell
USEMODULE += shell_cmds_default
USEMODULE += tiny_strerror
USEMODULE += ztimer_msec

FEATURES_REQUIRED += periph_adc

include $(RIOTBASE)/Makefile.include
