# i3-overview technical information
for contributors and developers

## 1. Overview ##

i3-overview is a pure C project written in [headerless C](https://github.com/milgra/headerlessc).
It uses  SDL2 library for window and handling and OpenGL context creation.
It uses a custom UI renderer called Zen UI, it is backed by OpenGL at the moment, Vulkan backend is on the roadmap.
It uses the Zen Core library for memory management, map/vector/bitmap container implementations, utf8 string and math functions.

## 2. Technology stacks ##

Graphics stack :

```
[OS][GPU] -> [SDL2] -> [ZEN_WM] -> [ZEN UI] -> ui.c -> ui controllers
```

## 3. Project structure ##

```
bin - build directory and built executable files
doc - documentation
src - code source files
 modules - external modules
 overview - main logic
tst - test resource files
```
 
## 4. Main Logic ##

```
analyzer.c - i3 tree parser and analyzer
config.c - configuration reader
fontconfig.c - system font loader
listener.c - xinput2 event listener for meta key listening
overview.c - main logic and entry point
renderer.c - i3 tree thumbnail renderer
tg_overview.c - texture generator for the thumbnail view
ui.c - ui handler
```

## 5. Program Flow ##

Entering point is in overview.c. It inits Zen WM module that inits SDL2 and openGL. Zen WM calls four functions in zenmusic.c :
- init - inits all zen music logic
- update - updates program state
- render - render UI
- destroy - cleans up memory

Internal state change happens in response to a UI event or timer event. All events are coming from Zen WM. In update they are sent to the UI and all views.
The views change their state based on the events and may call callbacks.

## 5. Development ##

Use your preferred IDE. It's advised that you hook clang-format to file save.

Please follow these guidelines :

- use clang format before commiting/after file save
- use zen_core functions and containers and memory handling
- always run all tests before push

To create a dev build, type

```
make dev
```