#include <stdint.h>
#include <stdbool.h>

#define VEDPARSE_KEY_LENGTH_MAX     128
#define VEDPARSE_VALUE_LENGTH_MAX   128
#define VEDPARSE_PROPS_MAX          128

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
bool VEDPARSE_process(uint8_t);
int32_t VEDPARSE_get_frame(vedframe_t* pFrame);