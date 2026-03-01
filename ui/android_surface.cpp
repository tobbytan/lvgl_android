
#include <ui/DisplayInfo.h>
#include <gui/ISurfaceComposer.h>
#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>

#include <GLES/gl.h>
#include <GLES/glext.h>
#include <EGL/eglext.h>
#include "android_surface.h"

using namespace android;

static sp<SurfaceComposerClient> gSession;
static sp<SurfaceControl> gControl;
static sp<Surface> gSurface;

AndroidEGLContext *init_android_surface()
{

    AndroidEGLContext *ctx =
        (AndroidEGLContext *)malloc(sizeof(AndroidEGLContext));
    gSession = new SurfaceComposerClient();
    sp<IBinder> dtoken(SurfaceComposerClient::getBuiltInDisplay(
        ISurfaceComposer::eDisplayIdMain));
    DisplayInfo dinfo;
    status_t status = SurfaceComposerClient::getDisplayInfo(dtoken, &dinfo);
    if (status)
        return NULL;

    int width = dinfo.w;
    int height = dinfo.h;
    printf("Display size: %d x %d\n", width, height);

    gControl = gSession->createSurface(String8("BootAnimation"),
                                       width, height, PIXEL_FORMAT_RGBA_8888);
    SurfaceComposerClient::openGlobalTransaction();
    gControl->setLayer(0x40000000);
    SurfaceComposerClient::closeGlobalTransaction();

    gSurface = gControl->getSurface();
    #if 0
    // initialize opengl and egl
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
    EGLSurface surface  = eglCreateWindowSurface(display, config, gSurface.get(), NULL);
    // EGLSurface surface = ctx->surface;
    EGLContext context = eglCreateContext(display, config, NULL, NULL);
    // EGLContext context = ctx->context;
    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);
    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE)
        return NULL;

    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_SCISSOR_TEST);
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    eglSwapBuffers(display, surface);
    sleep(2);
    glClearColor(1, 1, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    eglSwapBuffers(display, surface);
    ctx->window = gSurface.get();
    printf("window init %x~~~~~~~~~~\n", ctx->window);
    #endif
    ctx->window = gSurface.get();
    printf("window init %x~~~~~~~~~~\n", ctx->window);
    ctx->width = width;
    ctx->height = height;

    return ctx;
}

