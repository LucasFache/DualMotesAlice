CONTIKI_PROJECT =  observed-sender-node observed-receiver-node root-node
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
all:$(CONTIKI_PROJECT)

CONTIKI = ../../../..

ifndef TARGET
TARGET=zoul
BOARD=firefly
endif

ifneq ($(TARGET),zoul)
$(error Software is platform specific, only for Zolertia Zoul.)
endif

MODULE += os/services/alice
MAKE_ROUTING = MAKE_ROUTING_RPL_CLASSIC

include $(CONTIKI)/Makefile.include
