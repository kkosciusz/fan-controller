# Bathroom fan controller: ATtiny85 firmware + host tests.

MCU   ?= attiny85
# Digispark-style boards run at 16.5 MHz
F_CPU ?= 16500000UL

# ---------------------------------------------------------------- firmware
AVR_CC     = avr-gcc
AVR_CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -std=gnu11 \
             -Wall -Wextra -Wno-implicit-fallthrough \
             -ffunction-sections -fdata-sections -MMD -MP -Isrc
AVR_LDFLAGS = -mmcu=$(MCU) -Wl,--gc-sections
AVR_SRC    = src/app.c avr/hal_avr.c avr/main.c
AVR_OBJ    = $(addprefix build/avr/,$(AVR_SRC:.c=.o))

all: test size hex

build/avr/%.o: %.c
	@mkdir -p $(dir $@)
	$(AVR_CC) $(AVR_CFLAGS) -c -o $@ $<

build/fw.elf: $(AVR_OBJ)
	$(AVR_CC) $(AVR_LDFLAGS) -o $@ $^

build/fw.hex: build/fw.elf
	avr-objcopy -O ihex -R .eeprom $< $@

hex: build/fw.hex
flash: build/fw.hex
	micronucleus --run $<

size: build/fw.elf
	avr-size --format=avr --mcu=$(MCU) $<

# ------------------------------------------------------------- host tests
# Set SAN= (empty) to build without sanitizers, e.g. under WSL1.
SAN ?= -fsanitize=address,undefined
HOST_COMMON = -O0 -g $(SAN) -Wall -Wextra -MMD -MP -Isrc -Itest
HOST_C_OBJ  = build/host/src/app.o build/host/test/hal_fake.o
CATCH_CFLAGS = $(shell pkg-config --cflags catch2-with-main 2>/dev/null)
CATCH_LIBS   = $(shell pkg-config --libs catch2-with-main 2>/dev/null || echo -lCatch2Main -lCatch2)

build/host/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -std=gnu11 -Wno-implicit-fallthrough $(HOST_COMMON) -c -o $@ $<

build/host/test/test_fan.o: test/test_fan.cpp
	@mkdir -p $(dir $@)
	$(CXX) -std=c++17 $(HOST_COMMON) $(CATCH_CFLAGS) -c -o $@ $<

build/tests: build/host/test/test_fan.o $(HOST_C_OBJ)
	$(CXX) $(SAN) -o $@ $^ $(CATCH_LIBS)

test: build/tests
	./build/tests

clean:
	$(RM) -rf build

.PHONY: all flash test clean size hex

-include $(AVR_OBJ:.o=.d) $(HOST_C_OBJ:.o=.d) build/host/test/test_fan.d
