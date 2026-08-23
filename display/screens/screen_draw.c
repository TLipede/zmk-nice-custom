#include <lvgl.h>
#include "display/common.h"
#include "display/screens/screen_draw.h"
#include "display/screens/screen_layout.h"

#if IS_ZMK
#include <zmk/display.h>
#include <zephyr/sys/atomic.h>
#endif

static lv_color_t canvas_buf[SCREEN_VER * SCREEN_HOR];
static lv_obj_t *status_canvas = NULL; 
#if IS_ZMK
static atomic_t needs_redraw;
#else
static bool needs_redraw = false;
#endif

void screen_set_needs_redraw(void) {
#if IS_ZMK
    atomic_set(&needs_redraw, 1);
#else
    needs_redraw = true;
#endif
}

bool screen_needs_redraw(void) {
#if IS_ZMK
    return atomic_get(&needs_redraw);
#else
    return needs_redraw;
#endif
}

void screen_clear_redraw_flag(void) {
#if IS_ZMK
    atomic_clear(&needs_redraw);
#else
    needs_redraw = false;
#endif
}

lv_obj_t *screen_display(void) {
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_t *canvas = lv_canvas_create(screen);
    lv_canvas_set_buffer(canvas, canvas_buf, SCREEN_VER, SCREEN_HOR, LV_IMG_CF_TRUE_COLOR);
    lv_canvas_fill_bg(canvas, COLOR_BG, LV_OPA_COVER);
    status_canvas = canvas;

#if LEFT_DISPLAY
    screen_draw_left(canvas);
#else
    screen_draw_right(canvas);
#endif

    return screen;
}

static void screen_redraw(void) {
    if (status_canvas == NULL) return;
    
    if (screen_needs_redraw()) {
        lv_canvas_fill_bg(status_canvas, COLOR_BG, LV_OPA_COVER);

#if LEFT_DISPLAY
        screen_draw_left(status_canvas);
#else
        screen_draw_right(status_canvas);
#endif
        screen_clear_redraw_flag();
    }
}

#if IS_ZMK
static void screen_redraw_work_cb(struct k_work *work) {
    (void)work;
    screen_redraw();
}

K_WORK_DEFINE(screen_redraw_work, screen_redraw_work_cb);
#endif

void screen_update(void) {
#if IS_ZMK
    k_work_submit_to_queue(zmk_display_work_q(), &screen_redraw_work);
#else
    screen_redraw();
#endif
}

void screen_init(void) {
#if LEFT_DISPLAY
    screen_init_events_left();
#else
    screen_init_events_right();
#endif
    
#ifdef CONFIG_ZMK_DISPLAY
    k_msleep(50);
#endif
    
    lv_obj_t *screen = screen_display();
    lv_scr_load(screen);
}
