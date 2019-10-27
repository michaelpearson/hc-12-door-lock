MCU  = stm8s003f3
ARCH = stm8

SERVER_TARGET  ?= bin/server.ihx
CLIENT_TARGET  ?= bin/client.ihx

LIBRARY_SRCS := $(wildcard lib/*.c)

SERVER_OBJS   = $(patsubst %, bin/%, $(LIBRARY_SRCS:.c=.rel)) bin/src/server.rel
CLIENT_OBJS   = $(patsubst %, bin/%, $(LIBRARY_SRCS:.c=.rel)) bin/src/client.rel

CC       = sdcc
LD       = sdld
AS       = sdasstm8

ASFLAGS  = -plosgff

CFLAGS   = -m$(ARCH) -p$(MCU) --std-sdcc11
CFLAGS  += -I. -Ilib
CFLAGS  += --stack-auto --noinduction --use-non-free

LDFLAGS     = -m$(ARCH) -l$(ARCH) --out-fmt-ihx
BIN_LDFLAGS = -m$(ARCH) -l$(ARCH) --out-fmt-bin

all: $(CLIENT_TARGET) $(SERVER_TARGET)

client: $(CLIENT_TARGET)
server: $(SERVER_TARGET)


$(CLIENT_TARGET): $(CLIENT_OBJS)
	$(CC) $(LDFLAGS) $(CLIENT_OBJS) -o $@
	sdobjcopy -I ihex --output-target=binary $(CLIENT_TARGET) $(CLIENT_TARGET).bin
	@echo "Image size: \c"
	@stat -f%z $(CLIENT_TARGET).bin

$(SERVER_TARGET): $(SERVER_OBJS)
	$(CC) $(LDFLAGS) $(SERVER_OBJS) -o $@
	sdobjcopy -I ihex --output-target=binary $(SERVER_TARGET) $(SERVER_TARGET).bin
	@echo "Image size: \c"
	@stat -f%z $(SERVER_TARGET).bin

bin/%.rel: %.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

flash_client: $(CLIENT_TARGET)
	stm8flash -c stlinkv2 -p $(MCU) -w $(CLIENT_TARGET)

flash_server: $(SERVER_TARGET)
	stm8flash -c stlinkv2 -p $(MCU) -w $(SERVER_TARGET)

clean:
	rm -r bin

.PHONY: clean all flash