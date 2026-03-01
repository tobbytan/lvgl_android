#ifndef LV_CONF_H
#define LV_CONF_H

#define LV_COLOR_DEPTH     16       // 或 32，根据你的 EGL config
#define LV_HOR_RES_MAX     800
#define LV_VER_RES_MAX     600
#define LV_USE_LOG         1
#define LV_LOG_PRINTF      1
// #define LV_LOG_LEVEL       LV_LOG_LEVEL_INFO

#define LV_MEM_CUSTOM      0
#define LV_MEM_SIZE        (64U*1024U * 1024U)   // 根据设备内存调

#define LV_USE_DRAW_SW     1
#define LV_USE_DRAW_NANOVG 0               // GLES2 加速可选，后期再开

// 其他根据需要开启，比如字体、图片解码等
#define LV_FONT_MONTSERRAT_12 1
#define LV_USE_BIDI        0
#define LV_USE_ARABIC_PERSIAN_CHARS 0

// #define LV_USE_OPENGLES_DEBUG 1
#define LV_USE_OPENGLES    1
#define LV_USE_EGL         1

// #define LV_USE_DRAW_OPENGLES 1
#define LV_USE_MATRIX 1
#define LV_USE_FLOAT 1
#define LV_USE_DRAW_OPENGLES    1   // 核心：开启 GLES draw unit
// #define LV_USE_LINUX_DRM        1   // 如果是 Linux 嵌入式（如 RK3288/3399/车机）
// #define LV_LINUX_DRM_USE_EGL    1   // 用 EGL 创建上下文（推荐）

#endif