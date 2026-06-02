#ifndef __APP_H__
#define __APP_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void app_init(void);
void app_task(uint32_t now_ms);

#ifdef __cplusplus
}
#endif

#endif
