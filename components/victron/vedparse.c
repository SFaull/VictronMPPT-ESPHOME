#include "vedparse.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

typedef enum
{
    IDLE = 0,
    RECORD_BEGIN,
    RECORD_NAME,
    RECORD_VALUE,
    CHECKSUM,
    RECORD_HEX,
} VEDPARSE_state_e;

typedef struct 
{
    uint8_t prop_char_index;
    vedframe_t frame;
    vedprop_t prop;
    VEDPARSE_state_e state;
    uint8_t checksum;
} VEDPARSE_data_t;

const char* checksumTagName = "CHECKSUM";
static VEDPARSE_data_t _internal;

void doAddProperty(vedframe_t*, vedprop_t*);
bool doIsHexChar(char c);

/**
 * @brief Add a property to the frame
 * 
 * @param pFrame Pointer to frame
 * @param pProp Pointer to the property
 */
void doAddProperty(vedframe_t* pFrame, vedprop_t* pProp)
{
    //printf("Key: %s, Value: %s\n", pProp->key, pProp->value);

    uint8_t index = pFrame->property_count;

    vedprop_t* pDest = &pFrame->properties[index];
    vedprop_t* pSrc = pProp;

    memcpy(pDest, pSrc, sizeof(vedprop_t));

    pFrame->property_count++;
}

/**
 * @brief Returns true if the character is a hexadecimal symbol
 * 
 * @param c 
 * @return true 
 * @return false 
 */
bool doIsHexChar(char c) {
    return ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'));
}

/**
 * @brief Initialise the module by erasing the internal data structure
 * 
 */
void VEDPARSE_init(void)
{
    memset(&_internal, 0, sizeof(VEDPARSE_data_t)); // clear any history
}

/**
 * @brief Process an incomming byte of data. Returns true once a frame is ready
 * 
 * @param inbyte 
 * @return true 
 * @return false 
 */
bool VEDPARSE_process(uint8_t inbyte)
{
    bool frame_ready = false;
    //printf("State: %d\n", _internal.state);

    // check for hex records, we will ignore these
    if ( (inbyte == ':') && (_internal.state != CHECKSUM) ) 
    {
        _internal.state = RECORD_HEX;
    }

    // compute the checksum on the fly
    if (_internal.state != RECORD_HEX) 
    {
        _internal.checksum += inbyte;
    }
    
    // convert the char to upper case if we can
    inbyte = toupper(inbyte);

    // run the sate machine
    switch(_internal.state) 
    {
        case IDLE:  /* wait for \n of the start of an record */

            switch(inbyte) 
            {
                case '\n':
                    _internal.state = RECORD_BEGIN;
                break;
                case '\r': /* Skip */
                default:
                    // do nothing
                break;
            }
        break;

        case RECORD_BEGIN:
            _internal.prop.key[_internal.prop_char_index++] = inbyte; // first we will be writing to the key
            _internal.state = RECORD_NAME; 
        break;

        case RECORD_NAME:
        switch(inbyte) // The record name is being received, terminated by a \t
        {
            case '\t':
                // the Checksum record indicates a EOR
                if ( _internal.prop_char_index < VEDPARSE_KEY_LENGTH_MAX ) 
                {
                    _internal.prop.key[_internal.prop_char_index++] = 0; /* Zero terminate */
                    if (strcmp(_internal.prop.key, checksumTagName) == 0)  // see if this is the checksum property
                    {
                        _internal.state = CHECKSUM;
                        break;
                    }
                }
                _internal.prop_char_index = 0; // reset the indexer
                _internal.state = RECORD_VALUE;
            break;
            default:
                // add byte to name, but do no overflow
                if ( _internal.prop_char_index < VEDPARSE_KEY_LENGTH_MAX  )
                    _internal.prop.key[_internal.prop_char_index++] = inbyte; 
            break;
        }
        break;

        case RECORD_VALUE:
            // The record value is being received. The \r indicates a new record.
            switch(inbyte) 
            {
                case '\n':
                    // forward record, only if it could be stored completely
                    if ( _internal.prop_char_index < VEDPARSE_VALUE_LENGTH_MAX ) 
                    {
                        _internal.prop.value[_internal.prop_char_index++] = 0; /* Zero terminate */
                        doAddProperty(&_internal.frame, &_internal.prop); // add the completed property to the frame struct
                    }
                    _internal.prop_char_index = 0; // reset the indexer
                    _internal.state = RECORD_BEGIN;
                break;
            case '\r': /* Skip */
            break;
            default:
                // add byte to value, but do no overflow
                if ( _internal.prop_char_index < VEDPARSE_VALUE_LENGTH_MAX)
                    _internal.prop.value[_internal.prop_char_index++] = inbyte; 
            break;
            }
        break;

        case CHECKSUM:
        {
            //printf("CSUM is %02X\n", _internal.checksum);
            bool valid = (_internal.checksum == 0);
            _internal.frame.checksum_valid = valid;
            _internal.frame.checksum = _internal.checksum;
            #if 0
            if (!valid)
                printf("[CHECKSUM] Invalid frame\n");
            #endif
            _internal.checksum = 0;
            _internal.state = IDLE;
            frame_ready = true;
            // frameEndEvent(valid); // TODO: add callback
            break;
        }

        case RECORD_HEX:
        if (!doIsHexChar(inbyte)) // TODO: check me
        {
            _internal.checksum = 0;
            _internal.state = IDLE;
        }
        break;
    }

    return frame_ready;
}

/**
 * @brief Copy the current frame to the data structure via the supplied pointer
 * 
 * @param pFrame Pointer to frame
 * @return int32_t 0 if no error
 */
int32_t VEDPARSE_get_frame(vedframe_t* pFrame)
{
    // copy the frame
    memcpy(pFrame, &_internal.frame, sizeof(vedframe_t));

    // rest the module by erasing the internal data structure (including the frame)
    VEDPARSE_reset();
    return 0;
}

/**
 * @brief Reset the parser. Should be called after each frame
 * 
 */
void VEDPARSE_reset(void)
{
    VEDPARSE_init();
}