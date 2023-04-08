#ifndef __LHDC_API_H__
#define __LHDC_API_H__


#ifdef __cplusplus
extern "C" {
#endif
#include "lhdc_process.h"
#include "cirbuf.h"

#define LHDC_ENC_LIB_COPYRIGHT  "LHDC Encode Library @Copyright SAVITECH 2021"
#define LHDC_VERSION            LHDCVER

typedef enum {
    LHDCBT_SMPL_FMT_S16 = 16,
    LHDCBT_SMPL_FMT_S24 = 24,
} LHDCBT_SMPL_FMT_T;

typedef enum name {
    LHDCBT_QUALITY_LOW0 = 0,
    LHDCBT_QUALITY_LOW1,
    LHDCBT_QUALITY_LOW2,
    LHDCBT_QUALITY_LOW3,
    LHDCBT_QUALITY_LOW4,
    LHDCBT_QUALITY_LOW,
    LHDCBT_QUALITY_MID,
    LHDCBT_QUALITY_HIGH,
    LHDCBT_QUALITY_AUTO,
    LHDCBT_QUALITY_RESET_AUTO,
    LHDCBT_QUALITY_MAX
} LHDCBT_QUALITY_T;


#define LHDCV1_BT_ENC_BLOCK_SIZE  512

#define LHDCV2_BT_ENC_BLOCK_SIZE  512

#define LHDCV3_BT_ENC_BLOCK_SIZE  256


#define MAX_CHANNEL		    		2
#define LHDC_AR_PROCESS_CH			6
#define LHDC_SEPARATE_OUTPUR_CH     4
#define LHDC_AR_VERSION             "LHDC AR v1.1a.14 2021-7-7  @Copyright SAVITECH 2021"

typedef enum _lhdc_enc_type_t {
    ENC_TYPE_UNKNOWN = 0,
    ENC_TYPE_LHDC,
    ENC_TYPE_LLAC,
    ENC_TYPE_MAX,
    ENC_TYPE_ERR = ENC_TYPE_MAX
} lhdc_enc_type_t;

typedef enum _lhdc_filter_type_t{
    LHDC_FILTER_PASSTHROUGH = 0,
    LHDC_FILTER_AR,
    LHDC_FILTER_ERR = 0xffU
} lhdc_filter_type_t;

typedef enum _lhdc_mtu_size_t
{
  LHDC_MTU_MIN   = 300,
  LHDC_MTU_2MBPS = 660,
  LHDC_MTU_3MBPS = 1023,
  LHDC_MTU_MAX   = 4096,
} LHDC_MTU_SIZE_T;


typedef struct _llac_para_t {
    
    uint8_t* lh4_enc;
    int32_t  sample_rate;
    int32_t  bits_per_sample;

    LHDCBT_QUALITY_T qualityStatus;      //0xff == Auto bitrate
    LHDCBT_QUALITY_T limitBitRateStatus;
    int32_t  lastBitrate;     //Record lastbitrate for auto bitrate adjust.
    int32_t  real_bitrate;

    uint16_t avg_cnt;
    uint32_t avgValue;

    uint16_t changeBRCnt;
    uint32_t chageBRAvg;

    uint16_t upBitrateCnt;
    uint32_t upBitrateSum;

    uint16_t dnBitrateCnt;
    uint32_t dnBitrateSum;


    bool updateFramneInfo;
    uint32_t samples_per_frame; //equal to block size
    savi_circ_buf input_cbuf;
    int32_t  out_nbytes;    //output size per frame;
    uint32_t host_mtu_size;
    uint32_t frame_per_packet;
    uint32_t encode_interval;

    //kaiden:20210311:autobirate:malloc maximum nbyte size
    bool new_packet_flag;

} llac_para_t;

typedef struct _lhdc_para_t {

    FFT_BLOCK * fft_blk;
    int32_t  sample_rate;
    int32_t  bits_per_sample;

    LHDCBT_QUALITY_T qualityStatus;      //0xff == Auto bitrate
    LHDCBT_QUALITY_T limitBitRateStatus;
    int32_t lastBitrate;     //Record lastbitrate for auto bitrate adjust.
    int32_t version;

    uint16_t avg_cnt;
    uint32_t avgValue;

    uint16_t changeBRCnt;
    uint32_t chageBRAvg;

    uint16_t upBitrateCnt;
    uint32_t upBitrateSum;

    uint16_t dnBitrateCnt;
    uint32_t dnBitrateSum;

    bool updateFramneInfo;
    uint32_t block_size;
    uint32_t target_mtu_size;
    uint32_t host_mtu_size;
    uint32_t frame_per_packet;
    uint32_t encode_interval;
    savi_circ_buf input_cbuf;
    uint32_t queue_frame_cnt;

    bool hasMinBitrateLimit;
} lhdc_para_t;

//L_20210408 .start
typedef struct _lhdc_ar_para_t {
	int sample_rate;
	int bits_per_sample;
	int channels;
	unsigned int samples_per_frame;
	unsigned int    app_ar_enabled;
        unsigned long   ulNumBytes;
        unsigned char*  pucARParams;
        unsigned int	uiARParamsBytes;//Ask AR how many mem size
        unsigned char*  pucLhdcArMem;
        unsigned int	uiLhdcArMemBytes;//Ask AR how many mem size
} lhdc_ar_para_t;
//L_20210408 .end

typedef struct _lhdc_filter_t{
    uint8_t * priv; //save alloc mem point
    lhdc_filter_type_t type; //don't del..
} lhdc_filter_t;


typedef union _enc_t {
        lhdc_para_t *lhdc;
        llac_para_t *llac;
} enc_t;
typedef struct _lhdc_control_block_t {

    lhdc_enc_type_t enc_type;       //LLAC or LHDC
    int err;

    enc_t enc;
    lhdc_ar_para_t * ar_filter; //AR Param

} lhdc_cb_t;


inline static int32_t TARGET_BITRATE_LIMIT(int32_t X, int32_t Y){
     //((X < Y) ? (Y) : (X))

     if (X < Y)
        return Y;
    else
        return X;

}

typedef void * HANDLE_LHDC_BT;

extern llac_para_t * llac_encoder_new();
extern void llac_encoder_free(llac_para_t * llac);
extern int32_t llac_encoder_get_frame_len(llac_para_t  *  handle);
extern int32_t llac_encoder_get_target_bitrate(llac_para_t  *  handle);
extern int32_t llac_encoder_init(llac_para_t  *  handle, uint32_t sample_rate, uint32_t bits_per_sample, uint32_t bitrate_inx, uint32_t mtu_size, uint32_t encode_interval);
extern int32_t llac_encoder_set_target_mtu(llac_para_t * handle, uint32_t mtu);

extern lhdc_para_t * lhdc_encoder_new(int version);
extern void lhdc_encoder_free(lhdc_para_t * handle);
extern int32_t lhdc_encoder_get_frame_len(lhdc_para_t * handle);
extern int32_t lhdc_encoder_get_target_bitrate(lhdc_para_t * handle);
extern bool lhdc_get_ext_func_state(lhdc_enc_type_t type, void *h, lhdc_ext_func_t ext_type);
extern int32_t lhdc_encoder_init(lhdc_para_t * handle,int32_t sampling_freq, uint32_t bitPerSample, uint32_t bitrate_inx, uint32_t dualChannel, uint32_t need_padding, uint32_t mtu, uint32_t interval);
extern int32_t lhdc_encoder_set_target_mtu(lhdc_para_t * handle, uint32_t mtu);


extern void ar_process_free(lhdc_ar_para_t * handle);
extern lhdc_ar_para_t * ar_process_new(void);
extern int ar_process_init(lhdc_ar_para_t * handle, int sample_rate, int bits_per_sample, int channels, unsigned int samples_per_frame);
extern int ar_set_gyro_pos(lhdc_ar_para_t * handle, int world_coordinate_x, int world_coordinate_y, int world_coordinate_z);
extern int ar_set_cfg(lhdc_ar_para_t * handle,int *Pos, float *Gain, unsigned int app_ar_enabled);
extern int ar_get_cfg(lhdc_ar_para_t * handle,int *Pos, float *Gain);

//Common function


extern void lhdc_util_reset_up_bitrate(lhdc_enc_type_t type, void * h);
extern void lhdc_util_reset_down_bitrate(lhdc_enc_type_t type, void * h);
extern bool lhdc_util_get_ext_func_state(lhdc_enc_type_t type, void *h, lhdc_ext_func_t ext_type);
extern int32_t lhdc_util_encv4_process(HANDLE_LHDC_BT handle, void* p_pcm, unsigned char* out_put, uint32_t * written, uint32_t * out_fraems);
extern uint32_t lhdc_util_get_bitrate(uint32_t index);
extern uint8_t * lhdc_util_get_buffer(void);

#ifdef __cplusplus
}
#endif
#endif //End of __LHDC_API_H__
