# m5stack-synth-emulation

GENESIS/MEGADRIVE(YM2612+SN76489) VGM player on ESP32/M5Stack

## Demo

![](https://raw.githubusercontent.com/h1romas4/m5stack-synth-emulation/master/assets/m5stack-synth-02.jpg)

[https://www.youtube.com/watch?v=dunNtkFS8gc](https://www.youtube.com/watch?v=dunNtkFS8gc)

## Require

* M5Stack
* [esp32-idf v3.2.3 setup](https://docs.espressif.com/projects/esp-idf/en/v3.2.3/get-started/index.html)

## Build

![](https://github.com/h1romas4/m5stack-synth-emulation/workflows/M5Stack%20CI/badge.svg)

```
git clone --recursive https://github.com/h1romas4/m5stack-synth-emulation.git
cd m5stack-synth-emulation
make
```

## Upload .vgm file to M5Stack flash

```
./flashrom.sh vgm/ym2612.vgm
```

## Play music

```
make flash monitor
```

Enjoy!

## Dependencies

|name|version|
|-|-|
|[esp-idf](https://docs.espressif.com/projects/esp-idf/en/v3.2.3/get-started/index.html)|v3.2.3|
|[esp32-arduino](https://github.com/espressif/arduino-esp32)|1.0.4|
|[m5stack](https://github.com/m5stack/M5Stack)|0.2.9|

## Thanks!

* [sn76489.c](https://github.com/vgmrips/vgmplay/blob/master/VGMPlay/chips/sn76489.c)
* [ym2612.cpp](https://github.com/lutris/gens/blob/master/src/gens/gens_core/sound/ym2612.cpp)
