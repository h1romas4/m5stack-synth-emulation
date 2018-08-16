# m5stack-synth-emulation

PSG(SN76496) VGM player on ESP32/M5STACK

## Require

* M5STACK
* [esp32-idf setup](https://esp-idf.readthedocs.io/en/latest/get-started/index.html#setup-toolchain)
* VGM format file for sn76496(PSG)

## Compile

```
git clone --recursive https://github.com/h1romas4/m5stack-synth-emulation.git
cd m5stack-synth-emulation.git
make
```

## Upload .vgm file to M5STACK flash

```
mv example.vgz example.vgm.gz
gzip -d example.vgm.gz
flashrom.sh example.vgm   # not vgz
```

## Play music

```
make flash monitor
```

![](https://raw.githubusercontent.com/h1romas4/m5stack-synth-emulation/master/assets/m5stack-synth.jpg)

Enjoy!

## Thanks!

* [sn76496.c](https://github.com/notaz/picodrive/blob/master/pico/sound/sn76496.c)
