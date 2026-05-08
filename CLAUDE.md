# Trekkie — CLAUDE.md

## Project Overview
An LCARS-inspired Pebble watchface written in C for Pebble SDK 4.x.
Forked from remixz/trekkie with minor display mods: month number in date line,
no leading zero in 12h time, am/pm indicator below the time.
Modernized from the original PoC SDK structure to SDK 4.3 / pebble-tool workflow.

## Architecture
- Single C source file: `src/trek_watch.c`
- Resources: LCARS TTF fonts + PNG background/icon in `resources/`
- Project metadata and resource manifest: `appinfo.json`
- Build system: `pebble build` via pebble-tool (SDK 4.3)

## Project Structure
```
trekkie/
  appinfo.json          # Project metadata, resource manifest, UUID
  src/
    trek_watch.c        # All watchface logic
  resources/
    fonts/
      LCARS.ttf         # 60pt LCARS time font
      LCARS_BOLD.TTF    # 17pt bold LCARS date/ampm font
    images/
      background.png    # LCARS background (144x168)
      icon.png          # App menu icon
  archive/              # Pre-modernization artifacts (PoC SDK era)
    Makefile
    resources/
      resource_map.json
```

## Key Entry Points
- `main()` — calls init → app_event_loop → deinit
- `init()` — creates window, registers tick handler
- `window_load()` — allocates all layers, sets fonts, initial display
- `window_unload()` — destroys all layers and bitmaps
- `tick_handler()` — called every MINUTE_UNIT, drives display update
- `update_display()` — core rendering logic; date, time, and am/pm layers
- `deinit()` — unsubscribes tick service, destroys window

## Build Commands
```
pebble build                        # Compile for aplite + basalt
pebble install --emulator basalt    # Run in emulator
pebble install --cloudpebble        # Deploy to phone via rePebble app
```

## SDK 4.x Conventions (vs original PoC SDK)
| Old (PoC)                          | New (SDK 4.x)                              |
|------------------------------------|--------------------------------------------|
| #include "pebble_os.h" etc.        | #include <pebble.h>                        |
| PBL_APP_INFO(...) macro            | appinfo.json                               |
| Stack-allocated Window window      | Heap  Window *s_window = window_create()   |
| Stack-allocated TextLayer          | Heap  TextLayer *s_x = text_layer_create() |
| BmpContainer                       | GBitmap + BitmapLayer                      |
| handle_init(AppContextRef)         | init(void) + window_load(Window*)          |
| pbl_main(void *params)             | int main(void)                             |
| app_event_loop(params, &handlers)  | app_event_loop()                           |
| PblTm / get_time()                 | struct tm * / time(NULL) + localtime()     |
| string_format_time()               | strftime()                                 |
| resource_init_current_app()        | Automatic                                  |

## Conventions
- All module-level variables prefixed s_ (static module state)
- C style: flat procedural, no malloc beyond SDK create/destroy calls
- Static char buffers for display strings
- Battery-conscious rendering: date layer only redraws on change (strncmp guard)
- Pebble aplite display dimensions: 144 x 168 px

## Resources (Resource IDs)
- RESOURCE_ID_IMAGE_BACKGROUND    — LCARS background PNG
- RESOURCE_ID_IMAGE_MENU_ICON     — app menu icon
- RESOURCE_ID_FONT_LCARS_60       — 60pt LCARS font (time display), digits + colon only
- RESOURCE_ID_FONT_LCARS_BOLD_17  — 17pt bold LCARS font (date and am/pm)

## Target Platforms
- aplite  — original Pebble / Pebble Steel (1-bit display, 144x168)
- basalt  — Pebble Time (color display) — layout unchanged, no color-specific tuning yet
