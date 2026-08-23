#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "display/display_config.h"
#include "display/util.h"
#include "display/widgets/widget_wpm.h"

struct lv_timer_t {
    lv_timer_cb_t callback;
    uint32_t period;
    uint32_t remaining;
    bool paused;
};

static struct lv_timer_t timers[2];
static uint8_t timer_count;

static struct {
    int16_t x;
    int16_t y;
    const void *image;
} first_draw;
static bool captured_draw;

const lv_img_dsc_t wpm = {.id = 1};
const lv_img_dsc_t luna_sit_1 = {.id = 2};
const lv_img_dsc_t luna_sit_2 = {.id = 3};
const lv_img_dsc_t luna_walk_1 = {.id = 4};
const lv_img_dsc_t luna_walk_2 = {.id = 5};
const lv_img_dsc_t luna_run_1 = {.id = 6};
const lv_img_dsc_t luna_run_2 = {.id = 7};
const lv_img_dsc_t luna_sneak_1 = {.id = 8};
const lv_img_dsc_t luna_sneak_2 = {.id = 9};

lv_timer_t *lv_timer_create(lv_timer_cb_t callback, uint32_t period, void *user_data) {
    (void)user_data;
    assert(timer_count < 2);
    lv_timer_t *timer = &timers[timer_count++];
    *timer = (lv_timer_t){
        .callback = callback,
        .period = period,
        .remaining = period,
        .paused = false,
    };
    return timer;
}

void lv_timer_pause(lv_timer_t *timer) { timer->paused = true; }
void lv_timer_resume(lv_timer_t *timer) { timer->paused = false; }
void lv_timer_reset(lv_timer_t *timer) { timer->remaining = timer->period; }

static void advance_timers(uint32_t elapsed) {
    for (uint8_t i = 0; i < timer_count; i++) {
        lv_timer_t *timer = &timers[i];
        uint32_t remaining_elapsed = elapsed;

        while (!timer->paused && remaining_elapsed >= timer->remaining) {
            remaining_elapsed -= timer->remaining;
            timer->remaining = timer->period;
            timer->callback(timer);
        }
        if (!timer->paused) timer->remaining -= remaining_elapsed;
    }
}

void lv_draw_img_dsc_init(lv_draw_img_dsc_t *descriptor) { (void)descriptor; }

void lv_canvas_draw_img(lv_obj_t *canvas, int16_t x, int16_t y, const void *image,
                        const lv_draw_img_dsc_t *descriptor) {
    (void)canvas;
    (void)descriptor;
    if (!captured_draw) {
        first_draw = (typeof(first_draw)){.x = x, .y = y, .image = image};
        captured_draw = true;
    }
}

pos_t coordinates_from_vh(int16_t v, int16_t h) {
    return (pos_t){.x = SCREEN_VER - v - 1, .y = h, .v = v, .h = h};
}

void draw_number(lv_obj_t *canvas, uint8_t number, pos_t pos) {
    (void)canvas;
    (void)number;
    (void)pos;
}

void screen_set_needs_redraw(void) {}
void screen_update(void) {}

static void draw_luna(const lv_img_dsc_t *expected_image, int16_t expected_x) {
    captured_draw = false;
    widget_wpm_draw(NULL, 0);
    assert(captured_draw);
    assert(first_draw.image == expected_image);
    assert(first_draw.x == expected_x);
}

int main(void) {
    const int16_t ground_x = SCREEN_VER - 24 - 1;

    widget_wpm_init(LUNA_WALK_WPM - 1);
    draw_luna(&luna_sit_1, ground_x);

    widget_wpm_update(LUNA_WALK_WPM);
    draw_luna(&luna_walk_1, ground_x);
    widget_wpm_update(LUNA_RUN_WPM - 1);
    draw_luna(&luna_walk_1, ground_x);
    widget_wpm_update(LUNA_RUN_WPM);
    draw_luna(&luna_run_1, ground_x);

    widget_wpm_update_layer(LUNA_SNEAK_LAYER);
    draw_luna(&luna_sneak_1, ground_x);
    widget_wpm_update_layer(0);
    draw_luna(&luna_run_1, ground_x);

    widget_wpm_update(0);
    widget_wpm_trigger_jump();
    draw_luna(&luna_sit_1, ground_x + LUNA_JUMP_PIXELS);
    advance_timers(LUNA_JUMP_MS - 1);
    draw_luna(&luna_sit_1, ground_x + LUNA_JUMP_PIXELS);
    advance_timers(1);
    draw_luna(&luna_sit_1, ground_x);

    puts("Luna state tests passed");
    return 0;
}
