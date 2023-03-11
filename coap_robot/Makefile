APPLICATION := coap_robot
BOARD := 4dof-arm
RIOTBASE := $(CURDIR)/../RIOT
EXTERNAL_BOARD_DIRS += $(CURDIR)/../boards/riot
COMPILE_COMMANDS_PATH ?= $(abspath $(CURDIR)/../compile_commands.json)
DEVELHELP ?= 1
QUIET ?= 1

USEMODULE += auto_init_gnrc_netif
USEMODULE += fmt
USEMODULE += gcoap
USEMODULE += gnrc_icmpv6_echo
USEMODULE += gnrc_ipv6_default
USEMODULE += gnrc_sock_udp
USEMODULE += netdev_default
USEMODULE += netutils
USEMODULE += od
USEMODULE += ps
USEMODULE += random
USEMODULE += servo
USEMODULE += shell
USEMODULE += shell_cmds_default

include $(RIOTBASE)/Makefile.include
