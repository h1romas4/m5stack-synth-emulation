# m5stack-synth-emulation

GENESIS/MEGADRIVE(YM2612+SN76489) VGM player on ESP32/M5Stack

## Demo

![](https://raw.githubusercontent.com/h1romas4/m5stack-synth-emulation/master/assets/m5stack-synth-02.jpg)

[https://www.youtube.com/watch?v=dunNtkFS8gc](https://www.youtube.com/watch?v=dunNtkFS8gc)

## Require

* M5Stack
* [esp32-idf toolchain setup](https://docs.espressif.com/projects/esp-idf/en/stable/get-started/index.html#setup-toolchain)

```
$ xtensa-esp32-elf-gcc -v
gcc version 5.2.0 (crosstool-NG crosstool-ng-1.22.0-80-g6c4433a)
```

## Build

![](https://github.com/h1romas4/m5stack-synth-emulation/workflows/M5Stack%20CI/badge.svg)

### Compile

```
git clone --recursive https://github.com/h1romas4/m5stack-synth-emulation.git
cd m5stack-synth-emulation
# This repository includes eps-idf v3.2.3
export IDF_PATH=$(pwd)/esp-idf
make
```

### Upload sample VGM file to M5Stack flash

```
./flashrom.sh vgm/ym2612.vgm
```

### Play music

```
make flash monitor
```

### Create VGM file

* [mml2vgm](https://github.com/kuma4649/mml2vgm) by [kumatan](https://github.com/kuma4649) san
* [mucomMD2vgm](https://github.com/kuma4649/mucomMD2vgm) by [kumatan](https://github.com/kuma4649) san

## Binary release

Extract [release.tar.gz](https://github.com/h1romas4/m5stack-synth-emulation/releases) and Assign the binary to the following address:

|address|module|
|-|-|
|`0x1000`|`build/bootloader/bootloader.bin`|
|`0x8000`|`build/partitions.bin`|
|`0xe000`|`build/ota_data_initial.bin`|
|`0x10000`|`build/m5stack-synth-emulation.bin`|
|`0x211000`|`vgm/ym2612.vgm` or `vgm/sn76489.vgm`|

```
python ${IDF_PATH}/components/esptool_py/esptool/esptool.py \
    --chip esp32 \
    --port <SET IT TO YOUR COM PORT> \
    --baud 921600 \
    --before default_reset \
    --after hard_reset write_flash -z \
    --flash_mode dio --flash_freq 80m \
    --flash_size detect \
    0x1000 ./build/bootloader/bootloader.bin \
    0x8000 ./build/partitions.bin \
    0xe000 ./build/ota_data_initial.bin \
    0x10000 ./build/m5stack-synth-emulation.bin \
    0x211000 ./vgm/ym2612.vgm
```

## Dependencies

|name|version|
|-|-|
|[esp-idf](https://docs.espressif.com/projects/esp-idf/en/v3.2.3/get-started/index.html)|v3.2.3|
|[esp32-arduino](https://github.com/espressif/arduino-esp32)|1.0.4|
|[m5stack](https://github.com/m5stack/M5Stack)|0.2.9|

## License

[GNU General Public License v2.0](https://github.com/h1romas4/m5stack-synth-emulation/blob/master/LICENSE.txt)

## Thanks!

* [sn76489.c](https://github.com/vgmrips/vgmplay/blob/master/VGMPlay/chips/sn76489.c)
* [ym2612.cpp](https://github.com/lutris/gens/blob/master/src/gens/gens_core/sound/ym2612.cpp)
