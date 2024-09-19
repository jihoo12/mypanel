#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#define WIDTH 256
#define HEIGHT 256

static struct wl_display *display = NULL;
static struct wl_compositor *compositor = NULL;
static struct zwlr_layer_shell_v1 *layer_shell = NULL;
static struct wl_surface *surface = NULL;
static struct zwlr_layer_surface_v1 *layer_surface = NULL;
static struct wl_shm *shm = NULL;
static struct wl_buffer *buffer = NULL;
static uint32_t width = WIDTH, height = HEIGHT;

// 에러 메시지를 출력하고 프로그램 종료
static void handle_error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

// 공유 메모리 파일 생성
static int create_shared_memory_file(size_t size) {
    char template[] = "/tmp/wayland-shm-XXXXXX";
    int fd = mkstemp(template);
    if (fd < 0) {
        handle_error("mkstemp");
    }
    unlink(template);
    if (ftruncate(fd, size) < 0) {
        close(fd);
        handle_error("ftruncate");
    }
    return fd;
}

// 버퍼 생성
static void create_buffer() {
    int stride = width * 4; // 4 bytes per pixel (RGBA)
    int size = stride * height;

    int fd = create_shared_memory_file(size);
    if (fd < 0) {
        fprintf(stderr, "Failed to create shared memory file\n");
        exit(EXIT_FAILURE);
    }

    void *data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        close(fd);
        handle_error("mmap");
    }

    memset(data, 0xff, size); // Fill with white color

    struct wl_shm_pool *pool = wl_shm_create_pool(shm, fd, size);
    buffer = wl_shm_pool_create_buffer(pool, 0,
                                       width, height,
                                       stride,
                                       WL_SHM_FORMAT_ARGB8888);
    wl_shm_pool_destroy(pool);
    close(fd);
}

// 레이어 표면 구성
static void layer_surface_configure(void *data,
                                    struct zwlr_layer_surface_v1 *layer_surface,
                                    uint32_t serial, uint32_t new_width, uint32_t new_height) {
    zwlr_layer_surface_v1_ack_configure(layer_surface, serial);

    width = new_width > 0 ? new_width : width;
    height = new_height > 0 ? new_height : height;

    create_buffer();

    wl_surface_attach(surface, buffer, 0, 0);
    wl_surface_damage(surface, 0, 0, width, height);
    wl_surface_commit(surface);
}

// 레이어 표면 닫힘 처리
static void layer_surface_closed(void *data,
                                 struct zwlr_layer_surface_v1 *layer_surface) {
    fprintf(stderr, "Layer surface closed\n");
    exit(EXIT_SUCCESS);
}

static const struct zwlr_layer_surface_v1_listener layer_surface_listener = {
    .configure = layer_surface_configure,
    .closed = layer_surface_closed,
};

// 글로벌 객체 등록
static void registry_global(void *data, struct wl_registry *registry,
                            uint32_t name, const char *interface, uint32_t version) {
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        compositor = wl_registry_bind(registry, name, &wl_compositor_interface, version > 4 ? 4 : version);
    } else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
        layer_shell = wl_registry_bind(registry, name, &zwlr_layer_shell_v1_interface, version > 1 ? 1 : version);
    } else if (strcmp(interface, wl_shm_interface.name) == 0) {
        shm = wl_registry_bind(registry, name, &wl_shm_interface, version > 1 ? 1 : version);
    }
}

static void registry_global_remove(void *data, struct wl_registry *registry,
                                   uint32_t name) {}

// 리소스 해제
static void cleanup() {
   if (layer_surface)
       zwlr_layer_surface_v1_destroy(layer_surface);

   if (surface)
       wl_surface_destroy(surface);

   if (compositor)
       wl_compositor_destroy(compositor);

   if (shm)
       wl_shm_destroy(shm);

   if (display)
       wl_display_disconnect(display);
}

int main(int argc, char **argv) {
   display = wl_display_connect(NULL);
   if (!display) {
       fprintf(stderr, "Failed to connect to Wayland display\n");
       return EXIT_FAILURE;
   }

   struct wl_registry *registry = wl_display_get_registry(display);
   wl_registry_add_listener(registry, &registry_listener, NULL);

   // 두 번의 라운드트립을 사용하여 모든 글로벌 객체가 준비되도록 함
   wl_display_roundtrip(display); 
   wl_display_roundtrip(display);

   if (!compositor || !layer_shell || !shm) {
       fprintf(stderr, "Compositor or layer shell or shm not available\n");
       cleanup();
       return EXIT_FAILURE;
   }

   surface = wl_compositor_create_surface(compositor);
   layer_surface = zwlr_layer_shell_v1_get_layer_surface(layer_shell,
                                                         surface,
                                                         NULL,
                                                         ZWLR_LAYER_SHELL_V1_LAYER_TOP,
                                                         "example");

   zwlr_layer_surface_v1_set_size(layer_surface, width, height);
   zwlr_layer_surface_v1_set_exclusive_zone(layer_surface, -1);
   zwlr_layer_surface_v1_add_listener(layer_surface,
                                      &layer_surface_listener,
                                      NULL);

   create_buffer();

   wl_surface_attach(surface, buffer, 0, 0);
   wl_surface_damage(surface, 0, 0, width, height);

   wl_surface_commit(surface);

   while (wl_display_dispatch(display) != -1) {
       // Main loop
   }

   cleanup();
   
   return EXIT_SUCCESS;
}