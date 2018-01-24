//
// CNDOOM timers
//

#ifndef __CN_TIMER_H__
#define __CN_TIMER_H__

// [cndoom] new configuration variables for timer (see m_config.c)
extern int cn_timer_offset_x;
extern int cn_timer_offset_y;
extern int cn_timer_color_index;
extern int cn_timer_shadow_index;
extern int cn_timer_bg_colormap;
extern int cn_timer_enabled;

extern void CN_DrawTimer (void);
extern void CN_UpdateTimerLocation(int anchor);
extern void CN_ResetTimer (void);
extern void CN_DrawIntermissionTime (int x, int y, int time);
extern void CN_DrawTotalTime (void);

#endif

