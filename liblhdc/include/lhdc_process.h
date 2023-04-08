#ifndef _PROCESS_BLOCK_H_
#define _PROCESS_BLOCK_H_
#include <stdbool.h>


struct FFT_block_s;
typedef struct FFT_block_s FFT_BLOCK;

typedef enum {
	EXT_FUNC_AR,
	EXT_FUNC_LARC,
	EXT_FUNC_EXT_FLG,
	EXT_FUNC_JAS,
	EXT_FUNC_META
} lhdc_ext_func_t;


typedef enum _lhdc_error {
    LHDC_ST_OK,
    LHDC_ST_ERR,
    LHDC_ST_MAX
} LHDC_STATE;


int LossyEncoderLoadQualitySetting(FFT_BLOCK *fb, char *file_name);

/**
 * Allocate new LHDC control block.
 */
//Adjust by John
//FFT_BLOCK *LossyEncoderNew(void);
FFT_BLOCK *LossyEncoderNew(int32_t version);

/**
 * Destroy LHDC control block.
 */
int LossyEncoderDelete(FFT_BLOCK *fb);

/**
 * Initial LHDC encoder.
 * Input :
 *  fb: LHDC control block.
 *  sample_rate : The sampling rate setting value as you wanted. (Only support 44.1Khz/48Khz/96KHz)
 *  channels    : Fixed to 2 channel.
 *  block_size  : Fixed to 512.
 *  sink_buf_len: Fixed to 10 * 1024.
 *  target_byte_rate : The target byte rate.(eg. 400k bit per sec = 50k byte per sec, Only support 400kbps, 500kbps, 560kbps, 900kbps)
 *  fast_mode   : Always fixed to 0.
 *  split       : Fixed to 0.
 *  need_padding: Fixed to 0.
 *  process_size: Fixed to 256.
 *
 * Return value :
 *  LHDC_ST_ERR : Parameters of init have error.
 *  LHDC_ST_OK : LHDC encoder initial OK.
 */
//void LossyEncoderInit(FFT_BLOCK *fb, int sample_rate, int bits_per_sample, int channels, int block_size, int sink_buf_len, int target_byte_rate, int fast_mode, int one_frame_per_channel, int need_min_byte_rate, int process_size, int target_mtu_byte, int no_btr_limit_frame_cnt);
void LossyEncoderInit(FFT_BLOCK *fb, int sample_rate, int bits_per_sample, int channels, int block_size, int sink_buf_len, int target_byte_rate, int fast_mode, int one_frame_per_channel, int need_min_byte_rate, int process_size, int target_mtu_byte, int no_btr_limit_frame_cnt, int min_bits_reserved, int lhdc_ver);


/**
 * LHDC encode function
 *  fb: LHDC control block.
 *  wav : The PCM data. please input non-planer and compact PCM data.
 *        (eg. The input stream format is 96KHz/24bits stereo and the LHDC request 512 samples for each frame.
 *         So the PCM data length should be 512 * 2 * (24/8) = 3072 bytes. The data order should be L/R, L/R, L/R....).
 *  ns  : The number of samples, not PCM data byte length. The LHDC encoder only supports 512.
 *  final   : Fixed to 0.
 *  out : Output buffer pointer.
 *  out_len : The output buffer size to protect overwrite.
 *
 *  Return value :
 *      The return value should be the encoded size, otherwise an error occurs (less than or equal to 0)
 *
 */
int LossyEncoderProcessWav(FFT_BLOCK *fb, unsigned char *wav, int ns, int final, unsigned char *out, int out_len);

/**
 * LHDC encode function
 *  fb: LHDC control block.
 *  pcm0 : The left channel PCM data.
 *  pcm1 : The right channel PCM data.
 *  ns  : The number of samples, not PCM data byte length. The LHDC encoder only supports 512.
 *  final   : Fixed to 0.
 *  out : Output buffer pointer.
 *  out_len : The output buffer size to protect overwrite.
 *
 *  Return value :
 *      The return value should be the encoded size, otherwise an error occurs (less than or equal to 0)
 *
 */
int LossyEncoderProcessPCM(FFT_BLOCK *fb, int *pcm0, int *pcm1, int ns, int final, unsigned char *out, int out_len);

/**
 * To change target byte rate at runtime.
 * (Only support 400kbps, 500kbps, 560kbps, 900kbps)
 */
void LossyEncoderSetTargetByteRate(FFT_BLOCK *fb, int target_byte_rate);
void LossyEncoderResetAlignmentBuf(FFT_BLOCK *fb);
void LossyEncoderUpdateFrameSize(FFT_BLOCK *fb, int target_mtu_byte, int target_byte_rate);//For auto bit rate

void LhdcExtFuncArEnable(FFT_BLOCK *fb, int enable_ar);
void LhdcExtFuncJasEnable(FFT_BLOCK *fb, int enable_Jas);
void LhdcExtFuncMetaEnable(FFT_BLOCK *fb, int enable_meta, unsigned char *pmeta_data, int meta_data_len, int conti_frame_cnt);
bool LhdcGetExtFuncState(FFT_BLOCK *fb, lhdc_ext_func_t ext_func);
#endif // _PROCESS_BLOCK_H_
