# avm-retom

[![pipeline status](https://git.avmenergo.ru/avm-energo/avm-retom/badges/master/pipeline.svg)](https://git.avmenergo.ru/avm-energo/avm-retom/-/pipelines)
[![latest release](https://git.avmenergo.ru/avm-energo/avm-retom/-/badges/release.svg)](https://git.avmenergo.ru/avm-energo/avm-retom/-/releases)

English: [README.md](README.md)

**AVM RETOM** — простой графический (GUI) клиент для работы с устройствами **РЕТОМ-51** производства НПП «Динамика» (<https://dynamics.com.ru/>).

Программа разработана ООО «АВМ-Энерго» с помощью Claude.

## Скачать

Последний релиз:

- **Windows** (установщик): [avm-retom-setup.exe](https://git.avmenergo.ru/avm-energo/avm-retom/-/releases/permalink/latest/downloads/avm-retom-setup.exe)
- **Linux, Debian/Ubuntu**: [avm-retom.deb](https://git.avmenergo.ru/avm-energo/avm-retom/-/releases/permalink/latest/downloads/avm-retom.deb)
- **Linux, RPM** (Fedora/RED OS): [avm-retom.rpm](https://git.avmenergo.ru/avm-energo/avm-retom/-/releases/permalink/latest/downloads/avm-retom.rpm)

Все релизы: [https://git.avmenergo.ru/avm-energo/avm-retom/-/releases](https://git.avmenergo.ru/avm-energo/avm-retom/-/releases)

## Возможности

- Подключение: выбор порта (FTDI 0403:6001 подсвечивается автоматически), ограничения максимальных U/I, подключение/отключение.
- Аналоговый выход: U / фаза U / I / фаза I по каждой фазе A (жёлтая) / B (зелёная) / C (красная), частота, один переключатель слева вверху для включения/выключения выхода — при включённом выходе изменение значения повторно применяет сигнал через 400 мс — и кнопка «3-phase from A» для симметричной трёхфазной системы.
- Дискретные сигналы: выходные контакты 1–4 замыкаются/размыкаются; входные контакты 1–8 опрашиваются раз в секунду.
- Все вызовы драйвера выполняются в рабочем потоке; keepalive библиотеки поддерживает выход, пока сигнал подан.

## Сборка

```sh
cmake -B build -S . -DCMAKE_PREFIX_PATH=<qt>
cmake --build build
./build/avm-retom
```

Используется установленная библиотека `libavm-retom`, иначе `../libavm-retom`, иначе она загружается из git.
