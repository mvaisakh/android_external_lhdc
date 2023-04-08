#ifndef _PROCESSV3_BLOCK_H_
#define _PROCESSV3_BLOCK_H_


#ifndef __FFT_BLOCK__
#define __FFT_BLOCK__
typedef void *FFT_BLOCK;
#endif //End of __FFT_BLOCK__


#ifndef __LHDC_STATUS__
#define __LHDC_STATUS__
typedef enum _lhdc_error {
    LHDC_ST_OK,
    LHDC_ST_ERR,
    LHDC_ST_MAX
} LHDC_STATE;
#endif //End of __LHDC_STATUS__


//int LossyEncoderLoadQualitySetting(FFT_BLOCK *fb, char *file_name);

/**
 * Allocate new LHDC control block.
 */
FFT_BLOCK LossyEncoderNewV3(void);

/**
 * Destroy LHDC control block.
 */
int LossyEncoderDeleteV3(FFT_BLOCK fb);

/**
 * Initial LHDC encoder.
 * Input :
 *  fb: LHDC control block.
 *  sample_rate : The sampling rate setting value as you wanted. (Only support 44.1Khz/48Khz/96KHz)
 *  channels    : Fixed to 2 channel.
 *  block_size  : Fixed to 256.
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
void LossyEncoderInitV3(FFT_BLOCK fb, int sample_rate, int bits_per_sample, int channels, int block_size, int sink_buf_len, int target_byte_rate, int fast_mode, int one_frame_per_channel, int need_min_byte_rate, int process_size, int target_mtu_byte, int no_btr_limit_frame_cnt);


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
int LossyEncoderProcessWavV3(FFT_BLOCK fb, unsigned char *wav, int ns, int final, unsigned char *out, int out_len);

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
int LossyEncoderProcessPCMV3(FFT_BLOCK fb, int *pcm0, int *pcm1, int ns, int final, unsigned char *out, int out_len);

/**
 * To change target byte rate at runtime.
 * (Only support 400kbps, 500kbps, 560kbps, 900kbps)
 */
void LossyEncoderSetTargetByteRateV3(FFT_BLOCK fb, int target_byte_rate);
#endif // _PROCESSV3_BLOCK_H_
