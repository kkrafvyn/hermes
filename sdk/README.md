# HERMES SDK

Developer tools for building HERMES applications.

## CLI

```bash
python sdk/cli/hermes create myapp
python sdk/cli/hermes build examples/smartfarm
python sdk/cli/hermes deploy examples/smartfarm -n 1
python sdk/cli/hermes migrate smartfarm 0 1
```

## C Macros

```c
#include "hermes_sdk.h"

HERMES_TASK(my_loop, "main", 4096, 5)
HERMES_STATE(my_state_t, g_state)
HERMES_MONITOR("myapp", "metric", value);
```

## Templates

`templates/app/` — starter application with manifest.json and app.c.
