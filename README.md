# STM32H7 I2C Driver

Bare-metal, register-level I2C driver for the STM32H7 (no HAL).

## Features

- Kernel clock source selection and bus clock enable for I2C1/2/3/4
- Analog and digital noise filter configuration
- Three speed modes: Standard (100 kHz), Fast (400 kHz), Fast-mode Plus (1 MHz), mapped to precomputed timing values for a 64 MHz I2C kernel clock
- Leader (master) write, interrupt driven, with 7-bit addressing
- Follower (slave) receive, interrupt driven, with 7-bit or 10-bit own address
- Transfers of any length, including over 255 bytes (see below)
- NACK detection with an error flag
- Single application callback when a transfer finishes
- An STM32 Leader Transfer Sample Application also provided

## Long transfers (RELOAD)

The hardware byte counter (`NBYTES`) is only 8 bits, so one round covers at most 255 bytes. The driver splits longer messages into chunks and keeps them on the bus as **one continuous transfer**, with no STOP between chunks.

How it works:

1. If the message is larger than 255 bytes, the driver starts with `RELOAD` enabled and `NBYTES = 255`.
2. After each 255-byte chunk the hardware pauses with SCL held low and raises the transfer-complete-reload (`TCR`) interrupt. Nothing is lost while it waits.
3. The ISR works out how many bytes are left:
   - More than 255 left: load `NBYTES = 255` again and stay in reload mode.
   - 255 or fewer left (last chunk): load the remaining count, turn `RELOAD` off and turn `AUTOEND` on.
4. Loading a non-zero `NBYTES` releases SCL and the transfer continues.
5. After the last chunk the hardware sends STOP by itself, and the callback runs.

Example, 600 bytes: 255 (reload) → 255 (reload) → 90 (auto STOP). For auto STOP only use AUTOEND.

Messages of 255 bytes or fewer skip all of this and use `AUTOEND` directly. This ensures that the 
stop bit is automatically apended post the complete data transfer

Byte feeding is separate from chunking: the ISR supplies one byte per transmit interrupt, and `NBYTES`/`RELOAD` only control the count and when the transfer ends.

## Notes

- Timing values are only valid for the I2C kernel clock and filter settings they were computed for. Recompute if either changes.
- Status: not yet verified on hardware.
