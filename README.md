# BrundleFab - A Prototype Poweder Bed Printer

## Introduction

BrundleFab is an attempt to build a 3D powder bed thermal fusion printer,
using sugar as the working medium, and coffee as the pigment.

The name comes from the 1986 movie 'The Fly':
        "Do you normally take coffee with your sugar?"

The BrundleFab is designed for experimenting with thermally fused powders,
a process where a powder is inked with an IR absorptive pigment, and heat
applied to sinter the powder together in the colored regions.

With the fuser off, it can be used with conventional powder + binder recipes,
but for that I would recommend an existing non-prototype machine, such as the
YTec Plan B.

BrundleFab uses a single layer head for all layer operations - recoat, ink
deposition, and fusing.

Here's some terrible ASCII Art explaining the layer head:

           ___
          /   T
       __/_ F _\__________oH
    ||/ <-R          S-> \ H                                      ||
    ||-------------------||-------------------||                  ||
    ||    Feed Bin       ||    Part Bin       ||    Waste Bin     ||
    ||                   ||===================||                  ||
    ||===================||         ::        ||                  ||
    ||       E :: Axis   ||       Z :: Axis   ||                  ||
    ||         ::        ||         ::        ||                  ||
    ..         ..        ..         ..        ..                  ..

    Key:

      || - Wall of the powder chambers
      -- - Top of the feed/part powder layers
      == - Top of the feed/part pistons
      F  - Thermal fuser (halogen bulb)
      T  - Thermal sensor
      R  - Recoating blade
      S  - Powder sealing blade
      H  - Ink head
      o  - Ink head rail

## Axes and Tools

* X axis is from feed to part bins.
* Y axis holds the print carriage.
* Z axis is the part bin.
* E axis is the powder feed bin.

* Build area size is 175x260x230mm

* Tool 0 is the null tool
* Tools 1..16 are the ink sprayers
* Tool 20 is the fuser (heat lamp)
* Tool 21 is the recoating blade/roller

## GCode

GCode can be loaded via serial (115200, n81) or via a FAT formatted SD card.

The following GCode commands are supported:

| GCode                 | Description                                        |
| --------------------- | -------------------------------------------------- |
| G0 Xn Yn Zn En        | Uncontrolled move                                  |
| G1 Xn Yn Zn En Fn     | Controlled move                                    |
| G10 L1 Pt Xn Yn Zn En | Set tool table entry (tool offset)                 |
| G10 L1 Pt Rn Sn       | Set tool table entry (tool standby and op. temp)   |
| G20                   | Set units to inches                                |
| G21                   | Set units to mm                                    |
| G28 Xn Yn Zn En       | Home selected axes                                 |
| G90                   | Absolute positioning                               |
| G91                   | Relative positioning                               |
| G92 Xn Yn Zn En       | Set coordinate origin                              |
| --------------------- | -------------------------------------------------- |
| M0                    | Stop                                               |
| M1                    | Sleep                                              |
| M17                   | Enable motors                                      |
| M18                   | Disable motors                                     |
| M20 dirname           | List SD files                                      |
| M23 filename          | Select SD file                                     |
| M24                   | Start SD print                                     |
| M25                   | Pause SD print                                     |
| M26 Sn                | Set position in SD file                            |
| M27                   | Report SD print position                           |
| M30 filename          | Delete file from SD                                |
| M32 filename          | Select SD and and printf                           |
| M36 filename          | Return file information                            |
| M105                  | Return tool and bed temperature                    |
| M111 Sn               | Set debug flags                                    |
| M114                  | Get current position                               |
| M115                  | Get firmware version                               |
| M116                  | Wait for tool to become ready (wait for temp)      |
| M117 message          | Display message                                    |
| M119                  | Report endstop status                              |
| M124                  | Emergency stop                                     |
| M490 message          | Send message to CNC peripheral serial bus 0        |
| M491 message          | Send message to CNC peripheral serial bus 1        |
| M492 message          | Send message to CNC peripheral serial bus 2        |
| M493 message          | Send message to CNC peripheral serial bus 3        |
| --------------------- | -------------------------------------------------- |
| T0                    | Select null tool                                   |
| T1 Pn Qn Rn Sn        | Select ink tool                                    |
|                       |   P: bitmap (bits 23..0) of nozzle                 |
|                       |   Q: bitmap (bits 47..24) of nozzle                |
|                       |   R: bitmap (bits 71..48) of nozzle                |
|                       |   S: Spray density (dots / mm)                     |
| T2 .. T16             | Additional ink heads                               |
| T20                   | Select heat lamp tool                              |

## Hardware

This firmware supports either a RAMPS v1.4 system (default),
or the BrundleFab prototype hardware.

To add additional hardware support, copy `pinout-ramps.h`
to a new file (ie `pinout-planf.h`), update to your pinout,
and cahnge the `SHIELD=` line in `Makefile` to `planf`

### RAMPS v1.4 Controller

#### Shield Stack

| Shield           | Arduino Pins Used |
| ---------------- | ----------------- |
| Arduino Mega2560 | N/A               |
| RAMPS v1.4       | All               |

#### Connections

| Feature       | Shield        | Connection       |
| ------------- | ------------- | ---------------- |
| X Motor       | RAMPS v1.4    | Motor X          |
| X Endstop-Min | RAMPS v1.4    | D3               |
| X Endstop-Max | RAMPS v1.4    | D2               |
| ------------- | ------------- | ---------------- |
| Pen Control   | RAMPS v1.4    | Serial2 (16, 17) |
| ------------- | ------------- | ---------------- |
| Z Motor       | RAMPS v1.4    | Motor Z          |
| Z Endstop-Max | RAMPS v1.4    | D19              |
| ------------- | ------------- | ---------------- |
| E Motor       | RAMPS v1.4    | Motor E0         |
| E Endstop-Min | RAMPS v1.4    | D14              |
| ------------- | ------------- | ---------------- |
| Fuser Enable  | RAMPS v1.4    | D8               |
| Fuser Temp    | RAMPS v1.4    | TEMP0 (A13)      |
| ------------- | ------------- | ---------------- |
| TFT SPI       | RAMPS v1.4    | SPI (50, 51, 52) |
| TFT DC        | RAMPS v1.4    | D32              |
| TFT RST       | RAMPS v1.4    | D47              |
| TFT CS        | RAMPS v1.4    | D49              |
| TFT JOY       | RAMPS v1.4    | A12              |
| ------------- | ------------- | ---------------- |
| SD  SPI       | RAMPS v1.4    | SPI (50, 51, 52) |
| SD  CS        | RAMPS v1.4    | D53              |
| ------------- | ------------- | ---------------- |

### BrundleFab (Prototype) Controller

#### Shield Stack

| Shield                  | Arduino Pins Used               |
| ----------------------- | ------------------------------- |
| Arduino Mega2560        |                                 |
| AdaFruit MotorShield v2 | SCL, SDA                        |
| Adafruit MotorShield v1 | 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 |
| AdaFruit ST7735         | 45, 47, 49, 50, 51, 52, 53, A4  |

#### Connections

| Feature       | Shield        | Connection       |
| ------------- | ------------- | ---------------- |
| X Motor       | AdaFruit v1   | Motor 2          |
| X Endstop-Min | Arduino       | D35              |
| X Endstop-Max | Arduino       | D33              |
| ------------- | ------------- | ---------------- |
| Printhead     | Arduino       | Serial3 (14, 15) |
| ------------- | ------------- | ---------------- |
| Z Motor       | AdaFruit v2   | Motor 2          |
| Z Endstop-Max | Arduino       | D39              |
| ------------- | ------------- | ---------------- |
| E Motor       | AdaFruit v2   | Motor 1          |
| E Endstop-Min | Arduino       | D37              |
| ------------- | ------------- | ---------------- |
| Fuser Enable  | Arduino       | D43              |
| Fuser Temp    | AdaFruit v1   | A0               |
| ------------- | ------------- | ---------------- |
| TFT SPI       | Arduino       | SPI (50, 51, 52) |
| TFT DC        | Arduino       | D45              |
| TFT RST       | Arduino       | D47              |
| TFT CS        | Arduino       | D49              |
| TFT JOY       | AdaFruit v1   | A4               |
| ------------- | ------------- | ---------------- |
| SD  SPI       | Arduino       | SPI (50, 51, 52) |
| SD  CS        | Arduino       | D53              |
| ------------- | ------------- | ---------------- |

### Printhead Shield Stack

See https://github.com/ezrec/BrundleFab-printhead

