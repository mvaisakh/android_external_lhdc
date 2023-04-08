#ifndef _LHDCV5BT_DEC_H_
#define _LHDCV5BT_DEC_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "lhdcv5_util_dec.h"

#define LHDCV5BT_SAMPLE_RATE_44K    (44100)
#define LHDCV5BT_SAMPLE_RATE_48K    (48000)
#define LHDCV5BT_SAMPLE_RATE_96K    (96000)
#define LHDCV5BT_SAMPLE_RATE_192K   (192000)

#define LHDCV5BT_BIT_DEPTH_16    (16)
#define LHDCV5BT_BIT_DEPTH_24    (24)
#define LHDCV5BT_BIT_DEPTH_32    (32)

#define LHDCV5BT_BIT_RATE_64K    (64000)
#define LHDCV5BT_BIT_RATE_128K   (128000)
#define LHDCV5BT_BIT_RATE_192K   (192000)
#define LHDCV5BT_BIT_RATE_256K   (256000)
#define LHDCV5BT_BIT_RATE_320K   (320000)
#define LHDCV5BT_BIT_RATE_400K   (400000)
#define LHDCV5BT_BIT_RATE_600K   (600000)
#define LHDCV5BT_BIT_RATE_900K   (900000)
#define LHDCV5BT_BIT_RATE_1000K  (1000000)

#define LHDCV5BT_FRAME_DUR_5MS   (50)
#define LHDCV5BT_FRAME_DUR_10MS  (100)

typedef struct  
{
  lhdc_ver_t version;
  uint32_t sample_rate;
  uint32_t bits_depth;
  uint32_t bit_rate;
  uint32_t lossless_enable;
} tLHDCV5_DEC_CONFIG;


// lib APIs
int32_t lhdcv5BT_dec_init_decoder(HANDLE_LHDCV5_BT *handle, tLHDCV5_DEC_CONFIG *config);
int32_t lhdcv5BT_dec_check_frame_data_enough(const uint8_t *frameData, uint32_t frameBytes, uint32_t *packetBytes);
int32_t lhdcv5BT_dec_decode(const uint8_t *frameData, uint32_t frameBytes, uint8_t* pcmData, uint32_t* pcmBytes, uint32_t bits_depth);
int32_t lhdcv5BT_dec_deinit_decoder(HANDLE_LHDCV5_BT handle);

#define LHDCBT_DEC_NOT_UPD_SEQ_NO			0
#define LHDCBT_DEC_UPD_SEQ_NO				1

typedef enum __LHDCV5BT_DEC_API_RET__
{
  LHDCV5BT_DEC_API_SUCCEED            =  0,
  LHDCV5BT_DEC_API_FAIL               = -1,
  LHDCV5BT_DEC_API_INVALID_INPUT      = -2,
  LHDCV5BT_DEC_API_INVALID_OUTPUT     = -3,
  LHDCV5BT_DEC_API_INVALID_SEQ_NO     = -4,
  LHDCV5BT_DEC_API_INIT_DECODER_FAIL  = -5,
  LHDCV5BT_DEC_API_CHANNEL_SETUP_FAIL = -6,
  LHDCV5BT_DEC_API_FRAME_INFO_FAIL    = -7,
  LHDCV5BT_DEC_API_INPUT_NOT_ENOUGH   = -8,
  LHDCV5BT_DEC_API_OUTPUT_NOT_ENOUGH  = -9,
  LHDCV5BT_DEC_API_DECODE_FAIL        = -10,
  LHDCV5BT_DEC_API_ALLOC_MEM_FAIL  = -11,

} LHDCV5BT_DEC_API_RET_T;


#ifdef __cplusplus
}
#endif
#endif /* _LHDCBT_DEC_H_ */
