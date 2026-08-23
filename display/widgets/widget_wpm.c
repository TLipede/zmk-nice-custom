#include <stdbool.h>

#include "display/common.h"
#include "display/display_config.h"
#include "display/screens/screen_draw.h"
#include "display/util.h"
#include "display/widgets/widget_wpm.h"

#if IS_ZMK
#include <zephyr/sys/atomic.h>
#endif

LV_IMG_DECLARE(wpm);
LV_IMG_DECLARE(luna_sit_1);
LV_IMG_DECLARE(luna_sit_2);
LV_IMG_DECLARE(luna_walk_1);
LV_IMG_DECLARE(luna_walk_2);
LV_IMG_DECLARE(luna_run_1);
LV_IMG_DECLARE(luna_run_2);
LV_IMG_DECLARE(luna_sneak_1);
LV_IMG_DECLARE(luna_sneak_2);

enum luna_gait {
    LUNA_SIT,
    LUNA_WALK,
    LUNA_RUN,
    LUNA_SNEAK,
};

static const lv_img_dsc_t *const luna_frames[][2] = {
    {&luna_sit_1, &luna_sit_2},
    {&luna_walk_1, &luna_walk_2},
    {&luna_run_1, &luna_run_2},
    {&luna_sneak_1, &luna_sneak_2},
};

static struct {
    uint8_t value;
    uint8_t frame;
    enum luna_gait gait;
    bool sneaking;
    bool jumping;
    uint32_t jump_until;
    lv_timer_t *timer;
} wpm_data = {
    .value = 0,
    .frame = 0,
    .gait = LUNA_SIT,
    .sneaking = false,
    .jumping = false,
    .jump_until = 0,
    .timer = NULL,
};

#if IS_ZMK
static atomic_t jump_requested;
#else
static bool jump_requested;
#endif

static enum luna_gait gait_for_wpm(uint8_t wpm) {
    if (wpm < LUNA_WALK_WPM) return LUNA_SIT;
    if (wpm < LUNA_RUN_WPM) return LUNA_WALK;
    return LUNA_RUN;
}

static enum luna_gait current_gait(void) {
    return wpm_data.sneaking ? LUNA_SNEAK : gait_for_wpm(wpm_data.value);
}

static void update_gait(void) {
    enum luna_gait gait = current_gait();
    if (gait == wpm_data.gait) return;

    wpm_data.gait = gait;
    wpm_data.frame = 0;
    if (wpm_data.timer == NULL) return;

    if (gait == LUNA_SIT && !wpm_data.jumping) {
        lv_timer_pause(wpm_data.timer);
    } else {
        lv_timer_resume(wpm_data.timer);
    }
}

static void frame_timer_cb(lv_timer_t *timer) {
    (void)timer;
    if (wpm_data.jumping && (int32_t)(lv_tick_get() - wpm_data.jump_until) >= 0) {
        wpm_data.jumping = false;
        if (wpm_data.gait == LUNA_SIT) lv_timer_pause(wpm_data.timer);
    } else if (wpm_data.gait != LUNA_SIT) {
        wpm_data.frame ^= 1;
    }
    screen_set_needs_redraw();
    screen_update();
}

void widget_wpm_init(uint8_t initial_wpm) {
    wpm_data.timer = lv_timer_create(frame_timer_cb, LUNA_FRAME_MS, NULL);
    lv_timer_pause(wpm_data.timer);
    wpm_data.value = initial_wpm;
    wpm_data.gait = current_gait();
    if (wpm_data.gait != LUNA_SIT) lv_timer_resume(wpm_data.timer);
}

void widget_wpm_update(uint8_t wpm) {
    wpm_data.value = wpm;
    update_gait();
}

void widget_wpm_update_layer(uint8_t layer) {
    wpm_data.sneaking = layer == LUNA_SNEAK_LAYER;
    update_gait();
}

void widget_wpm_trigger_jump(void) {
#if IS_ZMK
    atomic_set(&jump_requested, 1);
#else
    jump_requested = true;
#endif
}

void widget_wpm_draw(lv_obj_t *canvas, int16_t v) {
    lv_draw_img_dsc_t img_dsc;
    lv_draw_img_dsc_init(&img_dsc);

#if IS_ZMK
    bool start_jump = atomic_cas(&jump_requested, 1, 0);
#else
    bool start_jump = jump_requested;
    jump_requested = false;
#endif
    if (start_jump) {
        wpm_data.jumping = true;
        wpm_data.jump_until = lv_tick_get() + LUNA_JUMP_MS;
        lv_timer_resume(wpm_data.timer);
    }

    int16_t luna_v = v + 24 - (wpm_data.jumping ? LUNA_JUMP_PIXELS : 0);
    pos_t pos_luna = coordinates_from_vh(luna_v, (SCREEN_HOR - 32) / 2);
    lv_canvas_draw_img(canvas, pos_luna.x, pos_luna.y,
                       luna_frames[wpm_data.gait][wpm_data.frame], &img_dsc);

    pos_t pos_wpm = coordinates_from_vh(v + 42, PADDING);
    lv_canvas_draw_img(canvas, pos_wpm.x, pos_wpm.y, &wpm, &img_dsc);

    pos_t pos_value = coordinates_from_vh(v + 42, SCREEN_HOR - PADDING - 18);
    draw_number(canvas, wpm_data.value, pos_value);
}
