# m5stack-synth-emulation

GENESIS/MEGADRIVE(YM2612+SN76489) VGM player on ESP32/M5Stack

## Demo

[https://www.youtube.com/watch?v=dunNtkFS8gc](https://www.youtube.com/watch?v=dunNtkFS8gc)

## Require

* M5Stack
* [esp32-idf v3.1.3 setup](https://docs.espressif.com/projects/esp-idf/en/v3.1.3/get-started/index.html)
* GENESIS/MEGADRIVE(YM2612+SN76489) VGM format file

## Build

![](https://github.com/h1romas4/m5stack-synth-emulation/workflows/M5Stack%20CI/badge.svg)

```
git clone --recursive https://github.com/h1romas4/m5stack-synth-emulation.git
cd m5stack-synth-emulation
make
```

## Upload .vgm file to M5Stack flash

```
mv example.vgz example.vgm.gz
gzip -d example.vgm.gz
./flashrom.sh example.vgm   # not vgz
```

## Play music

```
make flash monitor
```

![](https://raw.githubusercontent.com/h1romas4/m5stack-synth-emulation/master/assets/m5stack-synth-02.jpg)

Enjoy!

## Thanks!

* [sn76489.c](https://github.com/vgmrips/vgmplay/blob/master/VGMPlay/chips/sn76489.c)
* [ym2612.cpp](https://github.com/lutris/gens/blob/master/src/gens/gens_core/sound/ym2612.cpp)
