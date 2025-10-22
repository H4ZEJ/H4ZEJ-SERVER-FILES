#ifndef __INC_LIBTHECORE_SIGNAL_H__
#define __INC_LIBTHECORE_SIGNAL_H__

#ifdef __cplusplus
extern "C"
{
#endif
	extern void signal_setup();
	extern void signal_timer_disable();
	extern void signal_timer_enable(int timeout_seconds);

#ifdef __cplusplus
}
#endif

#endif
//martysama0134's 8e0aa8057d3f54320e391131a48866b4
