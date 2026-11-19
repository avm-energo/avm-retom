# avm-retom

[![pipeline status](https://git.avmenergo.ru/avm-energo/avm-retom/badges/master/pipeline.svg)](https://git.avmenergo.ru/avm-energo/avm-retom/-/pipelines)
[![latest release](https://git.avmenergo.ru/avm-energo/avm-retom/-/badges/release.svg)](https://git.avmenergo.ru/avm-energo/avm-retom/-/releases)

Русская версия: [README-ru.md](README-ru.md)

**AVM RETOM** is a simple GUI client for operating **RETOM-51** devices made by NPP Dinamika (<https://dynamics.com.ru/>).

This software was made by AVM-Energo LLC with the help of Claude.

## Download

Latest release:

- **Windows** (installer): [avm-retom-setup.exe](https://git.avmenergo.ru/avm-energo/avm-retom/-/releases/permalink/latest/downloads/avm-retom-setup.exe)
- **Linux, Debian/Ubuntu**: [avm-retom.deb](https://git.avmenergo.ru/avm-energo/avm-retom/-/releases/permalink/latest/downloads/avm-retom.deb)
- **Linux, RPM** (Fedora/RED OS): [avm-retom.rpm](https://git.avmenergo.ru/avm-energo/avm-retom/-/releases/permalink/latest/downloads/avm-retom.rpm)

All releases: [https://git.avmenergo.ru/avm-energo/avm-retom/-/releases](https://git.avmenergo.ru/avm-energo/avm-retom/-/releases)

## Features

- Connection: port picker (FTDI 0403:6001 auto-highlighted), max U/I limits, connect/disconnect.
- Analog output: U / U-phase / I / I-phase per phase A (yellow) / B (green) / C (red), frequency, one rocker switch on the top-left for output ON/OFF - while ON, editing a value re-applies the signal after 400 ms - and a "3-phase from A" helper.
- Discrete: output contacts 1-4 close/open; input contacts 1-8 polled once per second.
- All driver calls run on a worker thread; the library's keepalive keeps the output alive while a signal is applied.

## Build

```sh
cmake -B build -S . -DCMAKE_PREFIX_PATH=<qt>
cmake --build build
./build/avm-retom
```

Uses an installed libavm-retom, else `../libavm-retom`, else fetches it from git. 
