# SDK port build (PIC16/18, STM8)

Port HERMES to vendor toolchains that have no PlatformIO env.

## Source files

Copy from `hardware/stubs/`:

- `generic_node.c`
- `hermes_stub_sensor.c`
- `hermes_stub_sensor.h`

## Compile defines

```c
#define HERMES_STUB_MCU_NAME "pic16"   // your registry id
#define HERMES_STUB_FAMILY   "pic"
#define HERMES_NODE_ID       1
```

## Host smoke test (GCC)

```bash
cd hardware/stubs/port
make
./hermes_stub_test
```

## Deploy after flash

```bash
python run.py
python tools/serial-bridge/bridge.py COM4 --register --node-type pic16 --name "PIC16 Node"
```

## CLI

```bash
python scripts/flash_mcu.py pic16 --plan-only
python scripts/flash_mcu.py stm8 --plan-only
```

## API

```bash
curl -H "X-Hermes-Token: hermes-dev" http://localhost:8787/api/mcus/pic16/deploy
```
