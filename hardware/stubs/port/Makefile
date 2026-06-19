# Host-side smoke test for HERMES stub port layer (validates stub C compiles).
# Vendor SDK (MPLAB X, STVD) — copy sources into your project instead.

ROOT := ../..
CC ?= gcc
CFLAGS := -Wall -Wextra -std=c99 -I$(ROOT) \
	-DHERMES_STUB_MCU_NAME=\"port_test\" \
	-DHERMES_STUB_FAMILY=\"generic\" \
	-DHERMES_NODE_ID=1

SRCS := $(ROOT)/generic_node.c $(ROOT)/hermes_stub_sensor.c

.PHONY: all run clean

all: hermes_stub_test

hermes_stub_test: $(SRCS)
	$(CC) $(CFLAGS) -o $@ $(SRCS)

run: hermes_stub_test
	./hermes_stub_test

clean:
	rm -f hermes_stub_test
