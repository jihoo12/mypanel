#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include <cairo/cairo.h>
#include <wayland-egl.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
static struct wl_display *display;
static struct wl_compositor *compositor;
static struct zwlr_layer_shell_v1 *layer_shell;
static struct wl_surface *surface;
static struct zwlr_layer_surface_v1 *layer_surface;
static uint32_t width = 256, height = 256;
static struct wl_egl_window *egl_window;
static EGLDisplay egl_display;
static EGLContext egl_context;
static EGLSurface egl_surface;
static EGLConfig config;
static void check_egl_error(const char *msg) {
  EGLint error = eglGetError();
  if (error != EGL_SUCCESS) {
    fprintf(stderr, "%s: EGL error 0x%x\n", msg, error);
    exit(1);
  }
}

static void init_egl(void) {
  egl_display = eglGetDisplay((EGLNativeDisplayType)display);
  check_egl_error("eglGetDisplay");
    
  if (!eglInitialize(egl_display, NULL, NULL)) {
    check_egl_error("eglInitialize");
  }

  EGLint n;
  if (!eglChooseConfig(egl_display, (EGLint[]){
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
      }, &config, 1, &n)) {
    check_egl_error("eglChooseConfig");
  }

  egl_context = eglCreateContext(egl_display, config, EGL_NO_CONTEXT, (EGLint[]){
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    });
    check_egl_error("eglCreateContext");

    //printf("EGL initialized: display=%p, context=%p\n", (void*)egl_display, (void*)egl_context);
}
static void create_egl_surface() {
    if (egl_surface != EGL_NO_SURFACE) {
        eglDestroySurface(egl_display, egl_surface);
    }
    if (egl_window) {
        wl_egl_window_destroy(egl_window);
    }
    egl_window = wl_egl_window_create(surface, width, height);
    egl_surface = eglCreateWindowSurface(egl_display, config, egl_window, NULL);
    if (egl_surface == EGL_NO_SURFACE) {
        fprintf(stderr, "Failed to create EGL surface\n");
        exit(1);
    }
}
static void draw_frame(void);
static void layer_surface_configure(void *data,
                                    struct zwlr_layer_surface_v1 *layer_surface,
                                    uint32_t serial, uint32_t new_width, uint32_t new_height) {
    zwlr_layer_surface_v1_ack_configure(layer_surface, serial);

    width = new_width > 0 ? new_width : width;
    height = new_height > 0 ? new_height : height;

    create_egl_surface();

    if (!eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context)) {
        fprintf(stderr, "eglMakeCurrent failed\n");
        exit(1);
    }

    draw_frame();

    //printf("Layer surface configured: %dx%d\n", width, height);
}

static void layer_surface_closed(void *data,
        struct zwlr_layer_surface_v1 *layer_surface) {
  //printf("Layer surface closed\n");
    exit(0);
}

static const struct zwlr_layer_surface_v1_listener layer_surface_listener = {
    .configure = layer_surface_configure,
    .closed = layer_surface_closed,
};

static void registry_global(void *data, struct wl_registry *registry,
        uint32_t name, const char *interface, uint32_t version) {
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 4);
    } else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
        layer_shell = wl_registry_bind(registry, name, &zwlr_layer_shell_v1_interface, 1);
    }
}

static void registry_global_remove(void *data, struct wl_registry *registry,
        uint32_t name) {
    // 필요한 경우 여기에 정리 코드를 추가합니다
}

static const struct wl_registry_listener registry_listener = {
    .global = registry_global,
    .global_remove = registry_global_remove,
};

static void draw_frame(void) {
    if (egl_surface == EGL_NO_SURFACE) {
        return;  // EGL 표면이 아직 생성되지 않았으면 그리기를 건너뜁니다.
    }

    glViewport(0, 0, width, height);
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);  // 빨간색
    glClear(GL_COLOR_BUFFER_BIT);

    eglSwapBuffers(egl_display, egl_surface);

    //printf("Frame drawn\n");
}

int main(int argc, char **argv) {

    display = wl_display_connect(NULL);
    if (display == NULL) {
        fprintf(stderr, "Failed to connect to Wayland display\n");
        return 1;
    }

    struct wl_registry *registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registry_listener, NULL);
    wl_display_roundtrip(display);

    if (compositor == NULL || layer_shell == NULL) {
        fprintf(stderr, "Compositor or layer shell not available\n");
        return 1;
    }
    init_egl();
    surface = wl_compositor_create_surface(compositor);
    layer_surface = zwlr_layer_shell_v1_get_layer_surface(layer_shell,
                                                          surface, NULL, ZWLR_LAYER_SHELL_V1_LAYER_TOP, "example");

    zwlr_layer_surface_v1_set_size(layer_surface, width, height);
    zwlr_layer_surface_v1_set_exclusive_zone(layer_surface, -1);
    zwlr_layer_surface_v1_add_listener(layer_surface, &layer_surface_listener, NULL);

    wl_surface_commit(surface);

    //printf("Layer surface created and committed\n");

    while (wl_display_dispatch(display) != -1) {
        // 이벤트 처리만 하고 draw_frame은 호출하지 않습니다.
    }

    return 0;
}
