#ifndef TEST_LVGL_H
#define TEST_LVGL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t unused;
} lv_obj_t;

typedef struct {
    uint8_t id;
} lv_img_dsc_t;

typedef struct {
    uint8_t unused;
} lv_draw_img_dsc_t;

typedef struct lv_timer_t lv_timer_t;
typedef void (*lv_timer_cb_t)(lv_timer_t *timer);

#define LV_IMG_DECLARE(name) extern const lv_img_dsc_t name

lv_timer_t *lv_timer_create(lv_timer_cb_t callback, uint32_t period, void *user_data);
void lv_timer_pause(lv_timer_t *timer);
void lv_timer_resume(lv_timer_t *timer);
void lv_timer_reset(lv_timer_t *timer);
void lv_draw_img_dsc_init(lv_draw_img_dsc_t *descriptor);
void lv_canvas_draw_img(lv_obj_t *canvas, int16_t x, int16_t y, const void *image,
                        const lv_draw_img_dsc_t *descriptor);

#endif
