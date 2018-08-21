# m5stack-synth-emulation

GENESIS/MEGADRIVE(YM2612/SN76496) VGM player on ESP32/M5Stack

## Require

* M5Stack
* [esp32-idf setup](https://esp-idf.readthedocs.io/en/latest/get-started/index.html#setup-toolchain)
* GENESIS/MEGADRIVE(YM2612/SN76496) VGM format file

## Build

```
git clone --recursive https://github.com/h1romas4/m5stack-synth-emulation.git
cd m5stack-synth-emulation.git
make
```

## Upload .vgm file to M5Stack flash

```
mv example.vgz example.vgm.gz
gzip -d example.vgm.gz
flashrom.sh example.vgm   # not vgz
```

Please change if the vgm header position is different. [main.cpp](https://github.com/h1romas4/m5stack-synth-emulation/blob/master/main/main.cpp#L17)

```
int vgmpos = 0x40;
```

## Play music

```
make flash monitor
```

![](https://raw.githubusercontent.com/h1romas4/m5stack-synth-emulation/master/assets/m5stack-synth.jpg)

Enjoy!

## Thanks!

* [sn76496.c](https://github.com/notaz/picodrive/blob/master/pico/sound/sn76496.c)
* [ym2612.cpp](https://github.com/lutris/gens/blob/master/src/gens/gens_core/sound/ym2612.cpp)
