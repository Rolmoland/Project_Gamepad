# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is an RT-Thread RTOS Board Support Package (BSP) for the NXP FRDM-MCXA156 development board. RT-Thread is an open-source IoT real-time operating system designed for embedded devices.

**Board**: NXP FRDM-MCXA156 (ARM Cortex-M33, MCXA156 MCU)
**RTOS**: RT-Thread v5.x
**Build System**: SCons (Python-based)
**Toolchains Supported**: GCC (arm-none-eabi), Keil MDK (armclang), IAR

## Repository Structure

```
frdm-mcxa156/                  # BSP root directory
├── applications/              # User application code
│   ├── main.c                # Application entry point
│   ├── usb_app.c/h           # USB application code
│   └── bsp_system.h          # System configuration
├── board/                     # Board-specific files
│   ├── board.c/h             # Board initialization
│   ├── Kconfig               # Board hardware configuration options
│   ├── linker_scripts/       # Linker scripts for different toolchains
│   ├── MCUX_Config/          # NXP MCUXpresso configuration
│   └── ports/                # Port-specific code (e.g., CherryUSB)
├── packages/                  # RT-Thread software packages
│   ├── nxp-mcx-cmsis-latest/ # CMSIS support for MCX series
│   └── nxp-mcx-series-latest/# NXP MCX SDK package
├── ../Libraries/              # Shared driver library (one level up)
│   └── drivers/              # Hardware abstraction layer drivers
│       ├── drv_uart.c        # UART driver
│       ├── drv_pin.c         # GPIO driver
│       ├── drv_i2c.c         # I2C driver
│       ├── drv_spi.c         # SPI driver
│       ├── drv_adc.c         # ADC driver
│       ├── drv_pwm.c         # PWM driver
│       ├── drv_hwtimer.c     # Hardware timer driver
│       ├── drv_rtc.c         # RTC driver
│       └── drv_wdt.c         # Watchdog driver
├── build/                     # Build output directory (generated)
├── rtconfig.h                 # RT-Thread configuration header (generated)
├── rtconfig.py                # Toolchain and build configuration
├── .config                    # Kconfig configuration (use menuconfig)
├── Kconfig                    # Root Kconfig file
├── SConstruct                 # SCons build script (main)
├── SConscript                 # SCons build subscript
├── project.uvprojx            # Keil MDK5 project file
└── README.md                  # BSP documentation
```

**RT-Thread Root** (5 levels up from BSP): Contains the RT-Thread kernel source, components, libcpu, and tools.

## Build System & Commands

### Prerequisites

1. **RT-Thread Env Tool**: Required for package management and configuration
   - Download from: https://www.rt-thread.org/download.html#download-rt-thread-env-tool
   - Provides `menuconfig`, `pkgs`, and `scons` commands

2. **Toolchain**: Install one of the following:
   - **GCC**: arm-none-eabi-gcc (recommended, update path in `rtconfig.py` line 20)
   - **Keil MDK**: armclang (update path in `rtconfig.py` line 23)
   - **IAR**: iccarm (update path in `rtconfig.py` line 26)

3. **Python**: Python 3.x with SCons installed

### Initial Setup

```bash
# 1. Update package dependencies (must run first!)
pkgs --update

# If packages fail to download, upgrade package manager first:
pkgs --upgrade
pkgs --update

# This will download:
# - packages/nxp-mcx-cmsis-latest/
# - packages/nxp-mcx-series-latest/
```

**IMPORTANT**: The build will fail with an error if packages are not present. Always run `pkgs --update` after cloning or when packages are missing.

### Configuration

```bash
# Open menuconfig to configure RT-Thread kernel and drivers
menuconfig

# Navigate using arrow keys, Enter to select, Space to enable/disable
# Changes are saved to .config and rtconfig.h is regenerated
```

**Key Configuration Areas**:
- `Hardware Drivers Config -> On-chip Peripheral Drivers`: Enable/disable peripherals
- `RT-Thread Components`: Enable components like file systems, network, USB
- `RT-Thread online packages`: Add third-party software packages

### Building

```bash
# Build with SCons (uses toolchain from rtconfig.py)
scons

# Clean build artifacts
scons -c

# Build with specific toolchain (override rtconfig.py)
scons --toolchain=gcc
scons --toolchain=keil
scons --toolchain=iar

# Verbose build output
scons --verbose

# Parallel build (faster, use N cores)
scons -j8
```

**Build Outputs**:
- `rtthread.elf`: Executable file (GCC)
- `rtthread.bin`: Binary for flashing
- `rtthread.map`: Memory map and symbol information
- `build/`: Object files and intermediate outputs

### Alternative: IDE-based Build

```bash
# Generate Keil MDK5 project
scons --target=mdk5

# Generate IAR project
scons --target=iar

# Then open project.uvprojx (MDK) or project.eww (IAR) in the IDE
```

## Development Workflow

### Adding New Application Code

1. Place source files in `applications/`
2. The build system automatically includes all C files in this directory
3. `main.c` contains the `main()` function - application entry point

### Enabling Hardware Peripherals

```bash
# 1. Run menuconfig
menuconfig

# 2. Navigate to: Hardware Drivers Config -> On-chip Peripheral Drivers
# 3. Enable desired peripheral (e.g., BSP_USING_I2C, BSP_USING_SPI)
# 4. Save and exit
# 5. Rebuild
scons
```

The corresponding driver will be automatically included from `../Libraries/drivers/`.

### Adding Software Packages

```bash
# 1. Open menuconfig
menuconfig

# 2. Navigate to: RT-Thread online packages
# 3. Select and enable desired package
# 4. Exit menuconfig
# 5. Update packages
pkgs --update

# 6. Rebuild
scons
```

### Working with USB (CherryUSB)

The BSP includes CherryUSB support in `board/ports/cherryusb/`. USB application code is in `applications/usb_app.c/h`.

Enable USB in menuconfig:
- `Hardware Drivers Config -> On-chip Peripheral Drivers -> Enable USB`

### Modifying Board Configuration

- **GPIO/Pin Configuration**: Edit `board/board.c` or use `drv_pin.c` API
- **Clock Configuration**: See `board/MCUX_Config/clock_config.c`
- **Linker Script**: Modify files in `board/linker_scripts/` for memory layout

## Architecture & Design Patterns

### RT-Thread Architecture

1. **Kernel Layer** (in RT-Thread root):
   - Scheduler, threads, semaphores, mutexes, message queues
   - Memory management, timers
   - Located in: `../../../../../src/` and `../../../../../libcpu/`

2. **Component Layer** (in RT-Thread root):
   - Device framework, file systems (DFS), network stacks
   - Shell (FinSH), USB stack, sensor framework
   - Located in: `../../../../../components/`

3. **BSP Layer** (this directory):
   - Board initialization (`board/board.c`)
   - Hardware abstraction layer drivers (`../Libraries/drivers/`)
   - Application code (`applications/`)

### Driver Model

RT-Thread uses a unified device driver framework:

```c
// Drivers register devices with the framework
rt_hw_uart_register(&uart_device, "uart0", RT_DEVICE_FLAG_RDWR, NULL);

// Applications use devices through the framework
rt_device_t uart = rt_device_find("uart0");
rt_device_open(uart, RT_DEVICE_OFLAG_RDWR);
rt_device_write(uart, 0, data, size);
```

**Driver Files**: Each `drv_*.c` implements the RT-Thread device interface for a peripheral.

### Hardware Abstraction

- **NXP SDK**: Located in `packages/nxp-mcx-series-latest/` - provides low-level hardware access
- **CMSIS**: Located in `packages/nxp-mcx-cmsis-latest/` - ARM Cortex-M33 core support
- **RT-Thread Drivers**: Wrap NXP SDK with RT-Thread device framework

## Important Notes

### Toolchain Configuration

**Critical**: Update `rtconfig.py` line 20 (GCC), 23 (Keil), or 26 (IAR) with your toolchain installation path before building.

```python
# Example for GCC:
EXEC_PATH = r'C:\Program Files\GNU Arm Embedded Toolchain\bin'

# Example for Keil:
EXEC_PATH = r'C:\Keil_v5'
```

### Build Mode

By default, builds are in **debug mode** (line 31 in `rtconfig.py`):
```python
BUILD = 'debug'  # Includes debug symbols, -O0 optimization
#BUILD = 'release'  # Release mode, -O2 optimization
```

Change to `release` for production builds.

### Package Dependencies

The build system checks for required packages before building (see `SConstruct` lines 18-33). Missing packages will cause a build failure with instructions to run `pkgs --update`.

### RT-Thread Version

This BSP is designed for RT-Thread 5.x. Check `../../../../../README_zh.md` for version information.

### Common Issues

1. **"Cannot found RT-Thread root directory"**: Set `RTT_ROOT` environment variable or ensure the directory structure is intact (BSP should be 5 levels deep from RT-Thread root)

2. **"Dependency packages missing"**: Run `pkgs --update` (or `pkgs --upgrade` then `pkgs --update`)

3. **Build fails with compiler not found**: Update `EXEC_PATH` in `rtconfig.py` with correct toolchain path

4. **Linker errors about memory regions**: Check/modify linker script in `board/linker_scripts/MCXA156_flash.ld` (GCC) or `.sct` (Keil) or `.icf` (IAR)

## Testing & Debugging

### Serial Console

- Default UART: **LPUART0** (configured in `board/Kconfig` line 29-32)
- Used by RT-Thread FinSH shell and `rt_kprintf()`
- Baud rate and settings in driver configuration

### Debugging with Keil MDK

```bash
# Generate/update MDK project
scons --target=mdk5

# Open project.uvprojx
# Set breakpoints and use MDK debugger (requires J-Link/CMSIS-DAP)
```

### Flashing Firmware

```bash
# After building, flash rtthread.bin to the board
# Using NXP tools, J-Link, or OpenOCD

# Example with J-Link:
JLinkExe -device MCXA156 -if SWD -speed 4000 -autoconnect 1
> loadfile rtthread.bin 0x00000000
> r
> g
```

## Peripheral-Specific Notes

### GPIO (drv_pin.c)
- Use RT-Thread Pin API: `rt_pin_mode()`, `rt_pin_write()`, `rt_pin_read()`
- Pin numbers defined in `drv_pin.h` or board schematic

### UART (drv_uart.c)
- Multiple LPUART instances supported
- DMA support depends on `BSP_USING_DMA` in Kconfig
- Enable specific UARTs in menuconfig (UART0 enabled by default)

### I2C (drv_i2c.c)
- Flexcomm-based I2C interfaces
- I2C0 and I2C1 available (enable in menuconfig)
- Used for onboard sensors and modules

### SPI (drv_spi.c)
- LPSPI peripheral support
- Configure bus speed and mode through device driver

### ADC (drv_adc.c)
- Multiple channels available (0, 1, 8, 13, 26)
- Enable specific channels in menuconfig

### PWM (drv_pwm.c)
- eFlex PWM module support (PWM0, PWM1, PWM2)
- Channel and frequency configuration

### Timer (drv_hwtimer.c)
- CTIMER instances (0, 1, 3, 4)
- Used for hardware timing and pulse measurement

### RTC (drv_rtc.c)
- Real-time clock support
- Battery backup domain

### Watchdog (drv_wdt.c)
- System watchdog timer
- Configure timeout period

### SDIO
- SD card interface support
- Requires DFS and ELMFAT components enabled

## References

- **RT-Thread Documentation**: https://www.rt-thread.org/document/site/
- **RT-Thread GitHub**: https://github.com/RT-Thread/rt-thread
- **FRDM-MCXA156 Quick Start Guide**: https://www.rt-thread.org/document/site/#/rt-thread-version/rt-thread-standard/tutorial/quick-start/frdm_mcxa156/quick-start
- **NXP MCXA156 Documentation**: https://www.nxp.com/products/processors-and-microcontrollers/arm-microcontrollers/mcx-arm-cortex-m-mcus
- **Env Tool User Manual**: https://www.rt-thread.org/document/site/#/development-tools/env/env
