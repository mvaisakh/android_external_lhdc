/***********************************************************************
*                               LLAC encoder                           *
*                                                                      *
*                                                                      *
************************************************************************/

#ifndef LLAC_ENC_API_H
#define LLAC_ENC_API_H

/*
 * LLAC: 
 * API Usage
 *
 * STEP1.   call llac_enc_mem_alloc() once to allocate the memory
 * STEP2_0. call llac_enc_set_extra_func() before llac_enc_initial()
            to enable or disable extra function if needed (for AR, LARC and EXTH ,default is disable)
 * STEP2.   call llac_enc_initial() once to initialize encoder
 * STEP3_0. call llac_enc_set_bitrate() before llac_enc_process() 
 *          to set new bitrate if needed (for autobitrate usage)
 * STEP3.   call llac_enc_process() in a while loop to encode each frame
 * STEP4.   call llac_enc_mem_free() once to release the allocated memory 
 *        when the encode process are done 
 */



/*
 * LLAC: 
 *      LLAC_MAX_CHANNELS:   Max number of channels
 *      LLAC_MAX_SAMPLES:    Max number of samples per channel
 *      LLAC_MAX_BYTES:      Max size of encoded output data in bytess
 */
#define LLAC_MAX_CHANNELS 2
#define LLAC_MAX_SAMPLES 480
#define LLAC_MAX_BYTES 600


/**
 * LLAC:
 * llac_enc_mem_alloc - allocate all the memory needed for LLAC encoder
 * Return: the address of the allocated memory
 */
unsigned char* llac_enc_mem_alloc(void);


/**
 * LLAC:
 * llac_enc_set_extra_func - set extra function enable/disable (for AR, LARC and EXTH)
 * @func_index: can only be EXTRA_FUNC_AR, EXTRA_FUNC_LARC or EXTRA_FUNC_EXTH
 * @func_enable: can only be EXTRA_FUNC_DISABLE or EXTRA_FUNC_ENABLE
 * @llac_enc_mem_addr: the address returned by llac_enc_mem_alloc()
 */
#define EXTRA_FUNC_AR 0
#define EXTRA_FUNC_LARC 1
#define EXTRA_FUNC_EXTH 2

#define EXTRA_FUNC_DISABLE 0
#define EXTRA_FUNC_ENABLE 1

void llac_enc_set_extra_func(unsigned char func_index,
    unsigned char func_enable,
    unsigned char* llac_enc_mem_addr);


bool llac_enc_get_extra_func(unsigned char func_index,
                             unsigned char* llac_enc_mem_addr);

/**
 * LLAC:
 * llac_enc_initial - do encoder initialization
 * @sampleRate: sample rate of input file
 * @nChannels: channel number of input file
 * @bps_in: bit per sample of input file 
 * @bps_out: bit per sample of output file
 * @frame_ms: frame duration in millisecond
 * @target_bitrate: targeted bitrate
 * @nBytes: size of encoded output data in bytes
 * @nSamples: number of sample per channel
 * @real_bitrate: the actual bitrate that the encoder is gonna use
 * @llac_enc_mem_addr: the address returned by llac_enc_mem_alloc()
 */
void llac_enc_initial(unsigned int sampleRate,
                          short nChannels,
                          short bps_in,
                          short bps_out,
                          float frame_ms,
                          int target_bitrate,
                          int* nBytes,
                          unsigned int* nSamples,
                          int* real_bitrate,
                          unsigned char* llac_enc_mem_addr);


/**
 * LLAC:
 * llac_enc_set_bitrate - set new bitrate (for autobitrate usage)
 * @target_bitrate: targeted bitrate
 * @nBytes: size of encoded output data in bytes
 * @real_bitrate: the actual bitrate that the encoder is gonna use
 * @llac_enc_mem_addr: the address returned by llac_enc_mem_alloc()
 */
void llac_enc_set_bitrate(int target_bitrate,
    int* nBytes,
    int* real_bitrate,
    unsigned char* llac_enc_mem_addr);


/**
 * LLAC:
 * llac_enc_process - do encode process for one frame
 * @input: input buffer
 * @output: output buffer
 * @llac_enc_mem_addr: the address returned by llac_enc_mem_alloc()
 * Return:  size of encoded output data in bytes
 */
int llac_enc_process(int* input,
                         unsigned char* output,
                         unsigned char* llac_enc_mem_addr);


/**
 * LLAC:
 * llac_enc_mem_free - release all the memory needed for LLAC encoder
 * @llac_enc_mem_addr: the address returned by llac_enc_mem_alloc()
 */
void llac_enc_mem_free(unsigned char* llac_enc_mem_addr);

#endif /* LLAC_ENC_API_H */