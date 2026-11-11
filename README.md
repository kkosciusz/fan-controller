# Bathroom fan controller (ATtiny85)

Fan relay on PB1 (active high), light switch on PB0 (active low).
Fan turns on after the light has been on for 3 min and stays on for 2 min
after the light is turned off.

## Layout

    src/app.c, app.h    application logic
    src/hal.h           hardware functions the logic needs
    src/pt.h            minimal protothreads
    avr/                ATtiny85 HAL and main()
    test/               fake HAL and tests
    Makefile

## Setup

    sudo apt install gcc-avr avr-libc binutils-avr make g++ pkg-config catch2

## Use

    make size
    make test
    make flash
