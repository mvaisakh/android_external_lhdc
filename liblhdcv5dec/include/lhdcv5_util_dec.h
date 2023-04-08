/*
 * lhdcv5_util_dec.h
 *
 */

#ifndef LHDCV5_UTIL_DEC_H
#define LHDCV5_UTIL_DEC_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void * HANDLE_LHDCV5_BT;

// Copy definition from external
#define BTIF_BD_ADDR_SIZE    6

// Define for LHDC stream type.
typedef enum {
  LHDC_STRM_TYPE_COMBINE,
  LHDC_STRM_TYPE_SPLIT
}LHDC_STRM_TYPE;

typedef enum {
  VERSION_5 = 550
}lhdc_ver_t;

typedef enum {
  LHDCV2_BLOCK_SIZE = 512,
  LHDCV3_BLOCK_SIZE = 256,
}lhdc_block_size_t;

typedef struct savi_bt_local_info_t{
  uint8_t bt_addr[BTIF_BD_ADDR_SIZE];
  const char *bt_name;
  uint8_t bt_len;
  uint8_t ble_addr[BTIF_BD_ADDR_SIZE];
  const char *ble_name;
  uint8_t ble_len;
}savi_bt_local_info;

typedef struct _lhdc_frame_Info
{
  uint32_t frame_len;
  uint32_t isSplit;
  uint32_t isLeft;

} lhdc_frame_Info_t;


typedef enum {
  LHDC_OUTPUT_STEREO = 0,
  LHDC_OUTPUT_LEFT_CAHNNEL,
  LHDC_OUTPUT_RIGHT_CAHNNEL,
} lhdc_channel_t;

typedef int LHDCSample;

typedef void (*print_log_fp)(char*  msg);
typedef int (*LHDC_GET_BT_INFO)(savi_bt_local_info * bt_info);



#define A2DP_LHDC_HDR_LATENCY_LOW   0x00
#define A2DP_LHDC_HDR_LATENCY_MID   0x01
#define A2DP_LHDC_HDR_LATENCY_HIGH  0x02
#define A2DP_LHDC_HDR_LATENCY_MASK  (A2DP_LHDC_HDR_LATENCY_MID | A2DP_LHDC_HDR_LATENCY_HIGH)

#define A2DP_LHDC_HDR_FRAME_NO_MASK 0xfc


int32_t lhdcv5_util_init_decoder(uint32_t *ptr, uint32_t bitPerSample, uint32_t sampleRate, uint32_t scaleTo16Bits, uint32_t is_lossless_enable, lhdc_ver_t version);

int32_t lhdcv5_util_dec_process(uint8_t * pOutBuf, uint8_t * pInput, uint32_t InLen, uint32_t *OutLen);
char *lhdcv5_util_dec_get_version();

int32_t lhdcv5_util_dec_destroy();

void lhdcv5_util_dec_register_log_cb(print_log_fp cb);

int32_t lhdcv5_util_dec_get_sample_size (uint32_t *frame_samples);
int32_t lhdcv5_util_dec_fetch_frame_info(uint8_t *frameData, uint32_t frameDataLen, lhdc_frame_Info_t *frameInfo);

int32_t lhdcv5_util_dec_channel_selsect(lhdc_channel_t channel_type);
int32_t lhdcv5_util_dec_get_mem_req(lhdc_ver_t version, uint32_t *mem_req_bytes);

//Return
#define LHDCV5_UTIL_DEC_SUCCESS 0
#define LHDCV5_UTIL_DEC_ERROR_NO_INIT -1
#define LHDCV5_UTIL_DEC_ERROR_PARAM -2
#define LHDCV5_UTIL_DEC_ERROR -3
#define LHDCV5_UTIL_DEC_ERROR_WRONG_DEC -10

#ifdef __cplusplus
}
#endif
#endif /* End of LHDCV5_UTIL_DEC_H */
