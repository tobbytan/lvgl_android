#ifndef ANDROID_SURFACE_H
#define ANDROID_SURFACE_H

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        // EGLDisplay display;
        // EGLSurface surface;
        // EGLContext context;

        EGLNativeWindowType  window;

        int width;
        int height;
    } AndroidEGLContext;

    AndroidEGLContext *init_android_surface(void);

#ifdef __cplusplus
}
#endif
#endif