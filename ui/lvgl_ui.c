

#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <EGL/eglext.h>

#include "android_surface.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include "lvgl.h"

#include "lvgl_ui.h"

#ifndef ABS_MT_POSITION_X
#define ABS_MT_POSITION_X 0x35
#define ABS_MT_POSITION_Y 0x36
#define BTN_TOUCH 0x14a
#endif

struct _lv_egl_config
{
    EGLint id;
    EGLint max_width;
    EGLint max_height;
    EGLint buffer_size;
    EGLint depth;
    EGLint stencil;
    EGLint samples;
    EGLint surface_type;
    EGLint renderable_type;
    EGLint r_bits;
    EGLint g_bits;
    EGLint b_bits;
    EGLint a_bits;
};

typedef struct
{
    EGLint visual_id;
} lv_egl_native_window_properties_t;
typedef void *(*lv_egl_init_display_t)(void *driver_data, int32_t width, int32_t height);
typedef void *(*lv_egl_get_display_t)(void *driver_data);
typedef void *(*lv_create_window_t)(void *driver_data, const lv_egl_native_window_properties_t *props);
typedef void (*lv_destroy_window_t)(void *driver_data, void *native_window);

typedef void (*lv_egl_set_visible_t)(void *driver_data, bool v);
typedef void (*lv_egl_flip_t)(void *driver_data, bool vsync);
typedef void (*lv_egl_native_state_deinit_t)(void **driver_data);
typedef size_t (*lv_egl_select_config_t)(void *driver_data, const lv_egl_config_t *configs,
                                         size_t config_count);

struct _lv_egl_interface
{
    lv_egl_select_config_t select_config;
    void *driver_data;
    void *native_display;
    uint16_t egl_platform;
    lv_create_window_t create_window_cb;
    lv_destroy_window_t destroy_window_cb;
    lv_egl_flip_t flip_cb;
};

struct _lv_opengles_egl
{
    EGLNativeWindowType native_window;
    EGLDisplay egl_display;
    EGLConfig egl_config;
    EGLContext egl_context;
    EGLSurface egl_surface;
    void *egl_lib_handle;
    void *opengl_lib_handle;
    lv_egl_interface_t interface;
    int32_t width;
    int32_t height;
    bool vsync;
};

// 解决 API 18 缺失 __strlen_chk 的问题
#if defined(__ANDROID__) && __ANDROID_API__ < 23
size_t __strlen_chk(const char *s, size_t s_len)
{
    return strlen(s);
}
#endif

// 解决 GCC 4.7 强制优化 sincosf 的问题
#ifndef _GNU_SOURCE
void sincosf(float x, float *sin, float *cos)
{
    *sin = sinf(x);
    *cos = cosf(x);
}
#endif

GLuint texture_id;
static AndroidEGLContext *android_egl_ctx;
static lv_opengles_egl_t *egl_ctx;
int fd = -1;

static int32_t cursor_x = 0;
static int32_t cursor_y = 0;
static int32_t last_cursor_x = -1;
static int32_t last_cursor_y = -1;
static lv_obj_t *test_btn;
static lv_obj_t *cursor;

uint8_t mouse_cursor_icon_map[] = {

#define A 0x00, 0x00, 0x00, 0xFF /* Black */
#define W 0xFF, 0xFF, 0xFF, 0xFF /* White */
#define T 0x00, 0x00, 0x00, 0x00 /* Transparent */
    // clang-format off
    /* 16x16 */
    T,A,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,A,A,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,A,W,A,T,T,T,T,T,T,T,T,T,T,T,T,
    T,A,W,W,A,T,T,T,T,T,T,T,T,T,T,T,
    T,A,W,W,W,A,T,T,T,T,T,T,T,T,T,T,
    T,A,W,W,W,W,A,T,T,T,T,T,T,T,T,T,
    T,A,W,W,W,W,W,A,T,T,T,T,T,T,T,T,
    T,A,W,W,W,W,W,W,A,T,T,T,T,T,T,T,
    T,A,W,W,W,W,W,W,W,A,T,T,T,T,T,T,
    T,A,W,W,W,W,W,W,W,W,A,T,T,T,T,T,
    T,A,W,W,W,W,W,W,W,W,W,A,T,T,T,T,
    T,A,W,W,W,W,W,W,W,A,A,A,T,T,T,T,
    T,A,W,W,W,W,W,A,T,T,T,T,T,T,T,T,
    T,A,W,W,W,A,T,T,T,T,T,T,T,T,T,T,
    T,A,W,A,T,T,T,T,T,T,T,T,T,T,T,T,
    T,A,A,T,T,T,T,T,T,T,T,T,T,T,T,T,
    // clang-format on
};

const lv_image_dsc_t mouse_cursor_icon = {
    .header = {
        .magic = LV_IMAGE_HEADER_MAGIC,
        .cf = LV_COLOR_FORMAT_ARGB8888,
        .w = 16,
        .h = 16,
        .stride = 16 * 4,
    },
    .data_size = sizeof(mouse_cursor_icon_map),
    .data = mouse_cursor_icon_map,
};

void init_touch_device()
{

    fd = open("/dev/input/event2", O_RDONLY | O_NONBLOCK);
    if (fd < 0)
    {
        perror("无法打开输入设备，请检查 Root 权限");
    }
    if (ioctl(fd, EVIOCGRAB, 1) < 0)
    {
        perror("EVIOCGRAB 失败");
        close(fd);
        fd = -1;
        return;
    }
}

static egl_swap()
{
    // eglSwapBuffers(android_egl_ctx->display, android_egl_ctx->surface);
}

static uint32_t my_tick_cb(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

static void my_flush_cb2(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    uint32_t w = lv_area_get_width(area);
    uint32_t h = lv_area_get_height(area);

    glBindTexture(GL_TEXTURE_2D, texture_id);

    // 只更新局部区域到纹理
    glTexSubImage2D(GL_TEXTURE_2D, 0, area->x1, area->y1, w, h, GL_RGB,
                    GL_UNSIGNED_SHORT_5_6_5, px_map);

    // 画整个纹理到屏幕
    GLint crop[4] = {0, android_egl_ctx->height, android_egl_ctx->width, -android_egl_ctx->height};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, crop);
    glDrawTexiOES(0, 0, 0, android_egl_ctx->width, android_egl_ctx->height);

    egl_swap();

    lv_display_flush_ready(disp);
}

static void my_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{

    if (lv_display_flush_is_last(disp))
    {
        const int32_t disp_width = lv_display_get_horizontal_resolution(disp);
        const int32_t disp_height = lv_display_get_vertical_resolution(disp);
        /* The texture occupies the full screen even if `area` is not the full screen which happens with RENDER_MODE_DIRECT */
        lv_area_t full_area;
        lv_area_set(&full_area, 0, 0, disp_width, disp_height);

        /* Get the texture id containing LVGL generated UI */
        unsigned int texture_id = lv_opengles_texture_get_texture_id(disp);
        /* This function will render to the current context */
        lv_opengles_render_texture(texture_id, &full_area, LV_OPA_COVER, disp_width, disp_height, &full_area, false, true);
        lv_opengles_egl_update(egl_ctx);
    }
    lv_display_flush_ready(disp);
}

static void my_touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    struct input_event ev;
    static int32_t last_x = 0;
    static int32_t last_y = 0;
    static bool pressed = false;

    while (read(fd, &ev, sizeof(struct input_event)) > 0)
    {
        // printf("type: %d, code: %d, value: %d\n", ev.type, ev.code, ev.value);
        if (ev.type == EV_ABS)
        {
            if (ev.code == ABS_MT_POSITION_X || ev.code == ABS_X)
            {
                last_x = ev.value;
            }
            else if (ev.code == ABS_MT_POSITION_Y || ev.code == ABS_Y)
            {
                last_y = ev.value;
            }
        }
        else if (ev.type == EV_KEY && (ev.code == BTN_TOUCH || ev.code == BTN_LEFT))
        {
            pressed = (ev.value > 0);
        }
        else if (ev.type == EV_REL)
        {
            if (ev.code == REL_X)
            {
                cursor_x += ev.value;
            }
            else if (ev.code == REL_Y)
            {
                cursor_y += ev.value;
            }

            if (cursor_x < 0)
            {
                cursor_x = 0;
            }
            else if (cursor_x > android_egl_ctx->width - 1)
            {
                cursor_x = android_egl_ctx->width - 1;
            }
            if (cursor_y < 0)
            {
                cursor_y = 0;
            }
            else if (cursor_y > android_egl_ctx->height - 1)
            {
                cursor_y = android_egl_ctx->height - 1;
            }
            //
            last_x = cursor_x;
            last_y = cursor_y;
        }
    }
    //  printf("Touch: %s, %d, %d\n", pressed ? "Pressed" : "Released", last_x, last_y);

    // 将坐标和状态反馈给 LVGL
    data->point.x = last_x;
    data->point.y = last_y;
    data->state = pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

static void btn_event_cb(lv_event_t *e)
{
    //
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target_obj(e);
    if (code == LV_EVENT_CLICKED)
    {
        lv_opengles_egl_clear(egl_ctx);
        lv_opengles_egl_context_destroy(egl_ctx);

        raise(SIGINT);
    }
}

int init_my_ui()
{
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Hello world");
    lv_obj_center(label);

    test_btn = lv_button_create(lv_screen_active());
    lv_obj_set_size(test_btn, 120, 50);
    lv_obj_set_pos(test_btn, 50, 50);
    lv_obj_add_event_cb(test_btn, btn_event_cb, LV_EVENT_ALL, NULL);

    lv_obj_t *btn_label = lv_label_create(test_btn);

    lv_label_set_text(btn_label, "Click Me");
    lv_obj_center(btn_label);
}

static void* create_window_cb(void *driver_data, const lv_egl_native_window_properties_t *props)
{
    printf("create_window_cb\n");
    LV_UNUSED(props);

    return android_egl_ctx->window;
}

static void destroy_window_cb(void *driver_data, void *native_window)
{
    LV_UNUSED(driver_data);
    LV_UNUSED(native_window);
}

static void flip_cb(void *driver_data, bool vsync)
{

    LV_UNUSED(driver_data);
    LV_UNUSED(vsync);

    eglSwapBuffers(egl_ctx->egl_display, egl_ctx->egl_surface);
}
static size_t select_config_cb(void *driver_data, const lv_egl_config_t *configs, size_t config_count)
{
    size_t i;
    // printf("config count: %ld\n", config_count);
    // for (i = 0; i < config_count; ++i)
    // {
    //     printf("Got config %zu %#x %dx%d %d %d %d %d buffer size %d depth %d  samples %d stencil %d surface type %d renderable type %d\n",
    //            i, configs[i].id,
    //            configs[i].max_width, configs[i].max_height, configs[i].r_bits, configs[i].g_bits, configs[i].b_bits, configs[i].a_bits,
    //            configs[i].buffer_size, configs[i].depth, configs[i].samples, configs[i].stencil,
    //            configs[i].surface_type & EGL_WINDOW_BIT, configs[i].renderable_type & EGL_OPENGL_ES2_BIT);
    // }

    for (i = 0; i < config_count; ++i)
    {
        lv_color_format_t config_cf = lv_opengles_egl_color_format_from_egl_config(&configs[i]);
        const bool resolution_matches = configs[i].max_width >= android_egl_ctx->width &&
                                        configs[i].max_height >= android_egl_ctx->height;
        const bool is_nanovg_compatible = (configs[i].renderable_type & EGL_OPENGL_ES2_BIT) != 0 &&
                                          configs[i].stencil == 8 && configs[i].samples == 4;
        const bool is_window = (configs[i].surface_type & EGL_WINDOW_BIT) != 0;
        const bool is_compatible_with_draw_unit = is_nanovg_compatible || !LV_USE_DRAW_NANOVG;
        if (is_window && resolution_matches && config_cf == LV_COLOR_FORMAT_ARGB8888 && is_compatible_with_draw_unit)
        {
            LV_LOG_INFO("Choosing config %zu", i);
            return i;
        }
    }
    return config_count;
}
static lv_egl_interface_t lv_get_egl_interface(lv_display_t *display)
{
    return (lv_egl_interface_t){
        .driver_data = display,
        .create_window_cb = create_window_cb,
        .destroy_window_cb = destroy_window_cb,
        .egl_platform = 0,
        .native_display = EGL_DEFAULT_DISPLAY,
        .flip_cb = flip_cb,
        .select_config = select_config_cb,
    };
}

int start_lvgl_ui()
{
    init_touch_device();
    android_egl_ctx = init_android_surface();
    printf("init_android_surface ret = %x \n", android_egl_ctx);
    int width = android_egl_ctx->width;
    int height = android_egl_ctx->height;

    cursor_x = width / 2;
    cursor_y = height / 2;
    // glEnable(GL_TEXTURE_2D);

    // glGenTextures(1, &texture_id);
    // glBindTexture(GL_TEXTURE_2D, texture_id);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

#if 0    
    const EGLint attribs[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_DEPTH_SIZE, 0,
        EGL_NONE};
    EGLint w, h, dummy;
    EGLint numConfigs;
    EGLConfig config;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    // EGLDisplay display = ctx->display;

    eglInitialize(display, 0, 0);
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);


    EGLNativeWindowType native_window = android_egl_ctx->window;
    printf("why ?????~~~~~~~~\n");
    EGLSurface surface  = eglCreateWindowSurface(display, config, native_window, NULL);
    printf("create coose conifg ok~~~~~~~~\n");
#endif
    EGLint vertex_array_id = 1;
    glGenVertexArrays(1, &vertex_array_id);

    lv_init();

    lv_egl_interface_t ifc = lv_get_egl_interface(NULL);
    egl_ctx = lv_opengles_egl_context_create(&ifc);

    lv_display_t *display = lv_opengles_texture_create(width, height);

    lv_tick_set_cb(my_tick_cb);

    // lv_display_t *display = lv_display_create(width, height);

    printf("display create");
    // uint32_t buf_size = width * height * 4;
    // void *buf = malloc(buf_size);

    // if (buf)
    // {
    //     lv_display_set_buffers(display, buf, NULL, buf_size, LV_DISPLAY_RENDER_MODE_PARTIAL);
    // }

    lv_display_set_flush_cb(display, my_flush_cb);

    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, my_touch_read_cb);

    lv_obj_t *cursor = lv_image_create(lv_screen_active());
    // lv_obj_set_size(cursor, 16, 16);
    // lv_obj_set_style_bg_color(cursor, lv_color_hex(0xFF0000), 0);
    // lv_obj_set_style_bg_opa(cursor, LV_OPA_COVER, 0);
    // lv_obj_set_style_border_width(cursor, 0, 0);
    // lv_obj_clear_flag(cursor, LV_OBJ_FLAG_CLICKABLE);

    LV_IMAGE_DECLARE(mouse_cursor_icon); /* Declare the image source. */

    lv_image_set_src(cursor, &mouse_cursor_icon);

    lv_indev_set_cursor(indev, cursor);

    init_my_ui();

    while (1)
    {

        lv_timer_handler();
        usleep(5000);
    }
}
