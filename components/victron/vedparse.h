#ifndef VEDPARSE_H
#define VEDPARSE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VEDPARSE_KEY_LENGTH_MAX     64
#define VEDPARSE_VALUE_LENGTH_MAX   64
#define VEDPARSE_PROPS_MAX          32

typedef struct 
{
    char key[VEDPARSE_KEY_LENGTH_MAX];
    char value[VEDPARSE_VALUE_LENGTH_MAX];
} vedprop_t;



typedef struct 
{
    vedprop_t properties[VEDPARSE_PROPS_MAX];
    uint8_t property_count;
    uint8_t checksum;
    struct 
    {
        uint8_t checksum_valid : 1;
    };
} vedframe_t;

void VEDPARSE_init(void);
void VEDPARSE_reset(void);
bool VEDPARSE_process(uint8_t);
int32_t VEDPARSE_get_frame(vedframe_t* pFrame);
bool VEDPARSE_frame_started(void);

#ifdef __cplusplus
}
#endif

#endif // VEDPARSE_H