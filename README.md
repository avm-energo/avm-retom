# avm-retom

Qt Widgets GUI for operating the RETOM-51 test bench through `libavm-retom`.

- Connection: port picker (FTDI 0403:6001 auto-highlighted), max U/I limits, connect/disconnect.
- Analog output: U / U-phase / I / I-phase per phase A/B/C, frequency, Apply, Output OFF, "3-phase from A" helper.
- Discrete: output contact close/open; input contacts bits 0-7 polled once per second.
- All driver calls run on a worker thread; the library's keepalive keeps the output alive while a signal is applied.

Build: `cmake -B build -S . -DCMAKE_PREFIX_PATH=<qt>` (uses an installed libavm-retom, else `../libavm-retom`,
else fetches from git). Run: `./build/avm-retom`.

Note: libavm-retom ignores the output-contact number (only contact 0 is confirmed on hardware).
