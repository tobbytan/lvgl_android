# DOTOS - Android OpenGL UI Project

A lightweight UI framework for Android using LVGL and OpenGL ES.

## Features

- **LVGL v9** - Light and Versatile Graphics Library
- **OpenGL ES 2.0** - Hardware-accelerated rendering
- **EGL** - Native graphics context management
- **Custom Android Surface** - Direct surface rendering support

## Project Structure

```
dotos/
├── ui/                      # UI implementation
│   ├── lvgl_ui.c           # Main LVGL UI code
│   ├── android_surface.cpp # Android surface handling
│   └── lvgl.go            # Go bindings
├── third/lvgl/             # LVGL library
│   └── src/drivers/opengles/  # OpenGL ES driver
├── androidlib/             # Android system libraries
├── main.go                 # Entry point
└── build.sh               # Build script
```

## Requirements

- Android NDK
- Go 1.20+
- LVGL 9.x

## Build

```bash
./build.sh
```

## Usage

The application initializes the UI through `ui.InitUI()` in the main Go program. It sets up:

1. Android EGL surface
2. OpenGL ES context
3. LVGL display driver
4. Touch input handling
5. Mouse cursor (optional)

## OpenGL ES Driver

The project includes a custom OpenGL ES driver for LVGL located in `third/lvgl/src/drivers/opengles/`:

- `lv_opengles_driver.c` - Main rendering driver
- `lv_opengles_egl.c` - EGL context management
- `lv_opengles_texture.c` - Texture handling
- `glad/` - GLAD loader for OpenGL ES 2.0

### Key Features

- Vertex Array Object (VAO) support via OES extension
- Dynamic shader compilation (GLSL 100/300 es)
- Texture management
- Framebuffer support

## Configuration

Edit `third/lvgl/lv_conf.h` to configure:

```c
#define LV_MEM_SIZE        (128U * 1024U)   // Memory pool size
#define LV_COLOR_DEPTH     16                // 16 or 32 bit color
#define LV_HOR_RES_MAX    800               // Max horizontal resolution
#define LV_VER_RES_MAX    600               // Max vertical resolution
```

## License

This project includes LVGL which is MIT licensed. See [third/lvgl/README.md](third/lvgl/README.md) for details.
