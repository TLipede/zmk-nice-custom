#include "display/common.h"
#include "display/display_config.h"
#include "display/screens/screen_draw.h"
#include "display/util.h"
#include "display/widgets/widget_wpm.h"

LV_IMG_DECLARE(wpm);
LV_IMG_DECLARE(luna_sit_1);
LV_IMG_DECLARE(luna_sit_2);
LV_IMG_DECLARE(luna_walk_1);
LV_IMG_DECLARE(luna_walk_2);
LV_IMG_DECLARE(luna_run_1);
LV_IMG_DECLARE(luna_run_2);

enum luna_gait {
    LUNA_SIT,
    LUNA_WALK,
    LUNA_RUN,
};

static const lv_img_dsc_t *const luna_frames[][2] = {
    {&luna_sit_1, &luna_sit_2},
    {&luna_walk_1, &luna_walk_2},
    {&luna_run_1, &luna_run_2},
};

static struct {
    uint8_t value;
    uint8_t frame;
    enum luna_gait gait;
    lv_timer_t *timer;
} wpm_data = {
    .value = 0,
    .frame = 0,
    .gait = LUNA_SIT,
    .timer = NULL,
};

static enum luna_gait gait_for_wpm(uint8_t wpm) {
    if (wpm < LUNA_WALK_WPM) return LUNA_SIT;
    if (wpm < LUNA_RUN_WPM) return LUNA_WALK;
    return LUNA_RUN;
}

static void frame_timer_cb(lv_timer_t *timer) {
    (void)timer;
    wpm_data.frame ^= 1;
    screen_set_needs_redraw();
    screen_update();
}

void widget_wpm_init(uint8_t initial_wpm) {
    wpm_data.timer = lv_timer_create(frame_timer_cb, LUNA_FRAME_MS, NULL);
    lv_timer_pause(wpm_data.timer);
    widget_wpm_update(initial_wpm);
}

void widget_wpm_update(uint8_t wpm) {
    wpm_data.value = wpm;
    enum luna_gait gait = gait_for_wpm(wpm);
    if (gait == wpm_data.gait) return;

    wpm_data.gait = gait;
    wpm_data.frame = 0;
    if (gait == LUNA_SIT) {
        lv_timer_pause(wpm_data.timer);
    } else {
        lv_timer_resume(wpm_data.timer);
    }
}

void widget_wpm_draw(lv_obj_t *canvas, int16_t v) {
    lv_draw_img_dsc_t img_dsc;
    lv_draw_img_dsc_init(&img_dsc);

    pos_t pos_luna = coordinates_from_vh(v + 24, (SCREEN_HOR - 32) / 2);
    lv_canvas_draw_img(canvas, pos_luna.x, pos_luna.y,
                       luna_frames[wpm_data.gait][wpm_data.frame], &img_dsc);

    pos_t pos_wpm = coordinates_from_vh(v + 42, PADDING);
    lv_canvas_draw_img(canvas, pos_wpm.x, pos_wpm.y, &wpm, &img_dsc);

    pos_t pos_value = coordinates_from_vh(v + 42, SCREEN_HOR - PADDING - 18);
    draw_number(canvas, wpm_data.value, pos_value);
}
