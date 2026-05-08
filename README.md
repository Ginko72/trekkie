## Trekkie — Pebble Watchface

![Image](http://i.imgur.com/jEKDxnH.jpg)

An LCARS-inspired Pebble watchface. Swanky.

Forked from [remixz/trekkie](https://github.com/remixz/trekkie) with a few display tweaks:
- Month number added to the date line
- No leading zero in 12h time
- am/pm indicator below the time

Modernized from the original Proof of Concept SDK to the current pebble-tool / SDK 4.x workflow.

### Requirements

- [pebble-tool](https://github.com/pebble/pebble-tool) v5+
- Pebble SDK 4.x

### Building

```
pebble build
```

### Installing

```
pebble install --emulator basalt    # Run in basalt emulator
pebble install --cloudpebble        # Deploy to phone via rePebble app
```

The built `.pbw` bundle is at `build/trekkie.pbw` and can also be sideloaded directly.
