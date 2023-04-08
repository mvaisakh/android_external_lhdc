
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "lhdcBT.h"
#include "lhdc_process.h"
#include "lhdc_cfg.h"
#include "cirbuf.h"
#include "llac_enc_api.h"



#define LOG_NDEBUG 0
#define LOG_TAG "lhdcBT_enc"
#include <cutils/log.h>
#define max(a,b) ((a) > (b) ? (a) : (b))

#define UP_RATE_TIME_CNT         3000  //Time about UP_RATE_TIME_CNT * 20ms
#define DOWN_RATE_TIME_CNT       4  //Time about .... ex. DOWN_RATE_TIME_CNT * 20ms
#define QUEUE_LENGTH_THRESHOLD   4

#define LHDC_BITRATE_ELEMENTS_SIZE   (sizeof(auto_bitrate_adjust_table_lhdc)/sizeof(int))
#define LLAC_BITRATE_ELEMENTS_SIZE   (sizeof(auto_bitrate_adjust_table_llac)/sizeof(int))
#define LHDC_ABR_DEFAULT_BITRATE     (400)
#define LLAC_ABR_DEFAULT_BITRATE     (400)

//to record the last bitrate index in auto_bitrate_adjust_table
static uint32_t gABR_table_index = 0;

#define AR_ALWAYS_ONx  1


static const char * rate_to_string(LHDCBT_QUALITY_T q){
    switch (q) {
        case LHDCBT_QUALITY_LOW0:
        return "LHDCBT_QUALITY_LOW0";
        case LHDCBT_QUALITY_LOW1:
        return "LHDCBT_QUALITY_LOW1";
        case LHDCBT_QUALITY_LOW2:
        return "LHDCBT_QUALITY_LOW2";
        case LHDCBT_QUALITY_LOW3:
        return "LHDCBT_QUALITY_LOW3";
        case LHDCBT_QUALITY_LOW4:
        return "LHDCBT_QUALITY_LOW4";
        case LHDCBT_QUALITY_LOW:
        return "LHDCBT_QUALITY_LOW";
        case LHDCBT_QUALITY_MID:
        return "LHDCBT_QUALITY_MID";
        case LHDCBT_QUALITY_HIGH:
        return "LHDCBT_QUALITY_HIGH";
        case LHDCBT_QUALITY_AUTO:
        return "LHDCBT_QUALITY_AUTO";
        default:
            ALOGE("%s: Incorrect quality(%d)",  __func__, q);
        return "UNKNOW_QUALITY";
    }
}


static int auto_bitrate_adjust_table_lhdc[] = {320, 350, 380, 440, 580, 600};
static int auto_bitrate_adjust_table_llac[] = {136, 160, 192, 240, 320, 400};//7, 6, 5, 4, 3, 2

static int bitrateFromIndex(lhdc_enc_type_t type, void * h, int index){

    int limit = 0;
    int result = 0;

    if (type == ENC_TYPE_LHDC) {
        lhdc_para_t * lhdc = (lhdc_para_t * )h;
        limit = lhdc_util_get_bitrate((uint32_t)lhdc->limitBitRateStatus);
        result = auto_bitrate_adjust_table_lhdc[index];
    }else if (type == ENC_TYPE_LLAC) {
        llac_para_t * llac = (llac_para_t * )h;
        limit = lhdc_util_get_bitrate((uint32_t)llac->limitBitRateStatus);
        result = auto_bitrate_adjust_table_llac[index];
    }

    return result >= limit ? limit : result;

}

static int bitrateIndexFrom(lhdc_enc_type_t type, size_t queueLength) {

    uint32_t element_size = (type == ENC_TYPE_LHDC) ? LHDC_BITRATE_ELEMENTS_SIZE : LLAC_BITRATE_ELEMENTS_SIZE;

    int newBitrateInx = 0;
    if (queueLength < QUEUE_LENGTH_THRESHOLD) {
        float queuePercenty = (1 - ((float)queueLength / QUEUE_LENGTH_THRESHOLD)) * (element_size - 1);
        newBitrateInx = (int)queuePercenty;
    }
    return newBitrateInx;
}

//lhdcBT encHandle = NULL;
static int indexOfBitrate(lhdc_enc_type_t type, void * h, int bitrate){
    uint32_t element_size = type == ENC_TYPE_LHDC ? LHDC_BITRATE_ELEMENTS_SIZE : LLAC_BITRATE_ELEMENTS_SIZE;
    for (size_t i = 0; i < element_size; i++) {
        if (bitrateFromIndex(type, h, i) >= bitrate) {
            return i;
        }
    }
    return 0;
}
/*
******************************************************************
 LHDC functions group
******************************************************************
*/

static void lhdc_encoder_set_max_bitrate(lhdc_para_t * handle, int max_rate_index) {
    if (handle == NULL || max_rate_index == LHDCBT_QUALITY_AUTO){
        ALOGE("%s: Error LHDC instance(%p), max rate(%d)",  __func__, handle, max_rate_index);
        return;
    }
    if (max_rate_index != (int)handle->limitBitRateStatus){

        handle->limitBitRateStatus = (LHDCBT_QUALITY_T)max_rate_index;

        if ((uint32_t)handle->limitBitRateStatus != (uint32_t)handle->qualityStatus ){
            if (handle->qualityStatus != LHDCBT_QUALITY_AUTO) {
                handle->qualityStatus = handle->limitBitRateStatus;
            }

            int newRate = TARGET_BITRATE_LIMIT(lhdc_util_get_bitrate(handle->limitBitRateStatus), handle->hasMinBitrateLimit ? 320 : 128);

            if (handle->lastBitrate >= newRate) {
                handle->lastBitrate = newRate;

                if (handle->version >= 2) {
                    handle->updateFramneInfo = true;
                }
                LossyEncoderSetTargetByteRate(handle->fft_blk, (handle->lastBitrate * 1000) / 8);
                ALOGD("%s: Update Max target bitrate(%s)",  __func__, rate_to_string(handle->limitBitRateStatus));
            }
        }
    }
}


static int lhdc_encoder_encode(lhdc_para_t * handle, void* p_pcm, unsigned char* p_stream){
    if (handle) {
        if (p_pcm == NULL || p_stream == NULL) {
            ALOGE("%s: Buffer error! source(%p), output(%p)",  __func__, p_pcm, p_stream);
            return 0;
        }
        int bytesSizePerBlock = 0;
        int encodedSize = -1;
        uint32_t block_size = handle->block_size;
        bytesSizePerBlock = (block_size * (handle->bits_per_sample >> 3)) << 1;
        encodedSize = LossyEncoderProcessWav(handle->fft_blk, (unsigned char *)p_pcm, block_size, 0, p_stream, bytesSizePerBlock);
        return encodedSize;
    }
    ALOGE("%s: Handle error!(%p)",  __func__, handle);
    return 0;
}




static int lhdc_encoder_set_bitrate(lhdc_para_t * handle, int bitrate_inx){
    if (handle) {

        if (bitrate_inx != (int)handle->qualityStatus) {

            if (bitrate_inx != LHDCBT_QUALITY_AUTO) {
                handle->lastBitrate = TARGET_BITRATE_LIMIT(lhdc_util_get_bitrate(bitrate_inx), handle->hasMinBitrateLimit ? 320 : 128);
            }else{
                handle->lastBitrate = LHDC_ABR_DEFAULT_BITRATE;
                lhdc_util_reset_down_bitrate(ENC_TYPE_LHDC, handle);
                lhdc_util_reset_up_bitrate(ENC_TYPE_LHDC, handle);
            }

            handle->qualityStatus = bitrate_inx;

            if ((uint32_t)handle->qualityStatus > (uint32_t)handle->limitBitRateStatus &&
                handle->qualityStatus != LHDCBT_QUALITY_AUTO) {
                handle->lastBitrate = TARGET_BITRATE_LIMIT(lhdc_util_get_bitrate(handle->limitBitRateStatus), handle->hasMinBitrateLimit ? 320 : 128);
                handle->qualityStatus = handle->limitBitRateStatus;
            }
        }
        if (handle->version >= 2) {
          handle->updateFramneInfo = true;
        }
        LossyEncoderSetTargetByteRate(handle->fft_blk, (handle->lastBitrate * 1000) / 8);
        ALOGD("%s: Update target bitrate(%s)",  __func__, rate_to_string(handle->qualityStatus));
        return 0;
    }
    ALOGE("%s: Handle error!(%p)",  __func__, handle);
    return -1;
}



static int lhdc_encoder_adjust_bitrate(lhdc_para_t * handle, size_t queueLen) {
    if (handle != NULL && handle->qualityStatus == LHDCBT_QUALITY_AUTO) {
        if (handle->dnBitrateCnt >= DOWN_RATE_TIME_CNT) {
            /* code */
            size_t queueLength = handle->dnBitrateSum / handle->dnBitrateCnt;

            handle->dnBitrateSum = 0;
            handle->dnBitrateCnt = 0;
            uint32_t newBitrateInx = bitrateIndexFrom(ENC_TYPE_LHDC, queueLength);

            if (TARGET_BITRATE_LIMIT(bitrateFromIndex(ENC_TYPE_LHDC, handle, newBitrateInx), handle->hasMinBitrateLimit ? 320 : 128) <= handle->lastBitrate &&
                (newBitrateInx < gABR_table_index)) {
                handle->lastBitrate = TARGET_BITRATE_LIMIT(bitrateFromIndex(ENC_TYPE_LHDC, handle, newBitrateInx), handle->hasMinBitrateLimit ? 320 : 128);
                if (handle->version >= 2) {
                  handle->updateFramneInfo = true;
                }
                LossyEncoderSetTargetByteRate(handle->fft_blk, (handle->lastBitrate * 1000) / 8);
                ALOGD("%s:[Down BiTrAtE] Update to bitrate[%u](%d), queue length(%zu)",  __func__,
                    newBitrateInx, handle->lastBitrate, queueLength);
                lhdc_util_reset_up_bitrate(ENC_TYPE_LHDC, handle);

                gABR_table_index = newBitrateInx;
            }else{
              ALOGW("%s: Down bitrate condition fails, new rate:%d, current rate:%d",  __func__,
                                    TARGET_BITRATE_LIMIT(bitrateFromIndex(ENC_TYPE_LHDC, handle, newBitrateInx), handle->hasMinBitrateLimit ? 320 : 128),
                                    handle->lastBitrate);
            }
        }

        if (handle->upBitrateCnt >= UP_RATE_TIME_CNT) {
            //clear down bitrate parameters...
            size_t queueLength = handle->upBitrateSum / handle->upBitrateCnt;
            uint32_t queuSumTmp = handle->upBitrateSum;

            handle->upBitrateSum = 0;
            handle->upBitrateCnt = 0;
            //int newBitrateInx = bitrateIndexFrom(ENC_TYPE_LHDC, queueLength);
            uint32_t newBitrateInx = indexOfBitrate(ENC_TYPE_LHDC, handle, handle->lastBitrate);
            if (newBitrateInx < (LHDC_BITRATE_ELEMENTS_SIZE - 1)) {
                newBitrateInx++;
            }

            if (TARGET_BITRATE_LIMIT(bitrateFromIndex(ENC_TYPE_LHDC, handle, newBitrateInx), handle->hasMinBitrateLimit ? 320 : 128) >= handle->lastBitrate &&
                (newBitrateInx > gABR_table_index) && queuSumTmp == 0) {
                handle->lastBitrate = TARGET_BITRATE_LIMIT(bitrateFromIndex(ENC_TYPE_LHDC, handle, newBitrateInx), handle->hasMinBitrateLimit ? 320 : 128);

                if (handle->version >= 2) {
                  handle->updateFramneInfo = true;
                }
                LossyEncoderSetTargetByteRate(handle->fft_blk, (handle->lastBitrate * 1000) / 8);
                ALOGD("%s:[Up BiTrAtE] Update bitrate[%u](%d), queue length(%zu)",  __func__,
                    newBitrateInx, handle->lastBitrate, queueLength);
                lhdc_util_reset_down_bitrate(ENC_TYPE_LHDC, handle);

                gABR_table_index = newBitrateInx;
            }else{
              ALOGW("%s: Up bitrate condition fails, new rate:%d, current rate:%d, sum of queue len:%d",  __func__,
                                    TARGET_BITRATE_LIMIT(bitrateFromIndex(ENC_TYPE_LHDC, handle, newBitrateInx), handle->hasMinBitrateLimit ? 320 : 128),
                                    handle->lastBitrate,
                                    queuSumTmp);
            }
        }


        handle->upBitrateSum += queueLen;
        handle->dnBitrateSum += queueLen;

        handle->upBitrateCnt++;
        handle->dnBitrateCnt++;

        return 0;
    }
    ALOGE("%s: Handle error!(%p)",  __func__, handle);
    return -1;
}


/*
******************************************************************
 LLAC functions group
******************************************************************
*/



//kaiden:20210311:autobirate:llac_encoder_adjust_bitrate fucntion
static int llac_encoder_adjust_bitrate(llac_para_t * handle, size_t queueLen) {

    if (handle != NULL && handle->qualityStatus == LHDCBT_QUALITY_AUTO) {
        if (handle->dnBitrateCnt >= DOWN_RATE_TIME_CNT) {
            /* code */
            size_t queueLength = handle->dnBitrateSum / handle->dnBitrateCnt;

            handle->dnBitrateSum = 0;
            handle->dnBitrateCnt = 0;
            //int newBitrateInx = bitrateIndexFrom(ENC_TYPE_LLAC, queueLength);
            if (queueLength)
            {

                uint32_t newBitrateInx = 0;
                if (bitrateFromIndex(ENC_TYPE_LLAC, handle, newBitrateInx) <= handle->lastBitrate &&
                    (newBitrateInx < gABR_table_index)) {
                    handle->lastBitrate = bitrateFromIndex(ENC_TYPE_LLAC, handle, newBitrateInx);
    
                        llac_enc_set_bitrate(handle->lastBitrate * 1000, &handle->out_nbytes, &handle->real_bitrate, handle->lh4_enc);
                        //handle->frame_per_packet = handle->host_mtu_size / handle->out_nbytes;
                        handle->updateFramneInfo = true;
    
                        ALOGD("%s:[Down BiTrAtE] Update bitrate[%u](%d), queue length(%zu)",  __func__,
                            newBitrateInx, handle->lastBitrate, queueLength);
                        lhdc_util_reset_up_bitrate(ENC_TYPE_LLAC, handle);
                        gABR_table_index = newBitrateInx;
                    }else{
                      ALOGW("%s: Down bitrate condition fails, new rate:%d, current rate:%d",  __func__,
                                            bitrateFromIndex(ENC_TYPE_LLAC, handle, newBitrateInx),
                                            handle->lastBitrate);
                }
            }

        }

        if (handle->upBitrateCnt >= UP_RATE_TIME_CNT) {
            //clear down bitrate parameters...
            size_t queueLength = handle->upBitrateSum / handle->upBitrateCnt;
            uint32_t queuSumTmp = handle->upBitrateSum;

            handle->upBitrateSum = 0;
            handle->upBitrateCnt = 0;
            // get the last index in abr table
            uint32_t newBitrateInx = gABR_table_index;

            if (newBitrateInx < (LLAC_BITRATE_ELEMENTS_SIZE - 1)) {
                newBitrateInx++;
            }

            if (bitrateFromIndex(ENC_TYPE_LLAC, handle, newBitrateInx) >= handle->lastBitrate &&
                (newBitrateInx > gABR_table_index) && queuSumTmp == 0) {
                handle->lastBitrate = bitrateFromIndex(ENC_TYPE_LLAC, handle, newBitrateInx);

                llac_enc_set_bitrate(handle->lastBitrate * 1000, &handle->out_nbytes, &handle->real_bitrate, handle->lh4_enc);
                //handle->frame_per_packet = handle->host_mtu_size / handle->out_nbytes;
                handle->updateFramneInfo = true;

                ALOGD("%s:[Up BiTrAtE] Update bitrate[%u](%d), queue length(%zu)",  __func__,
                    newBitrateInx, handle->lastBitrate, queueLength);
                lhdc_util_reset_down_bitrate(ENC_TYPE_LLAC, handle);
                gABR_table_index = newBitrateInx;
            }else{
              ALOGW("%s: Up bitrate condition fails, new rate:%d, current rate:%d, sum of queue len:%d",  __func__,
                                    bitrateFromIndex(ENC_TYPE_LLAC, handle, newBitrateInx),
                                    handle->lastBitrate,
                                    queuSumTmp);
            }
        }


        handle->upBitrateSum += queueLen;
        handle->dnBitrateSum += queueLen;

        handle->upBitrateCnt++;
        handle->dnBitrateCnt++;

        return 0;
    }
    ALOGE("%s: Handle error!(%p)",  __func__, handle);
    return -1;
}

/*
******************************************************************
 LHDC library public functions group
******************************************************************
*/

void lhdcBT_free_handle(HANDLE_LHDC_BT handle) {
    lhdc_cb_t * lhdcBT = (lhdc_cb_t *)handle;
    if (!lhdcBT)
    {
        ALOGE("%s: Handle is NULL!!!", __func__);
        return;
    }
    switch(lhdcBT->enc_type){
        case ENC_TYPE_LHDC:
        lhdc_encoder_free(lhdcBT->enc.lhdc);
        break;
        case ENC_TYPE_LLAC:
        llac_encoder_free(lhdcBT->enc.llac);
        break;
        default:
        break;
    }

    ar_process_free(lhdcBT->ar_filter);

    free(lhdcBT);
}


HANDLE_LHDC_BT lhdcBT_get_handle(int version){
    ALOGD("%s: Version number %d", __func__, version);

    if(version <= 0)
    {
      return NULL;
    }

    lhdc_cb_t * lhdcBT = (lhdc_cb_t *)malloc(sizeof(lhdc_cb_t));
    memset(lhdcBT, 0 , sizeof(lhdc_cb_t));

#ifdef AR_ALWAYS_ON
    lhdcBT->ar_filter = ar_process_new();
#else
    if(version >= 3)
    {
        lhdcBT->ar_filter = ar_process_new();
    }
#endif

    if (version <= 3)
    {
        lhdcBT->enc.lhdc = lhdc_encoder_new(version);
        lhdcBT->enc_type = ENC_TYPE_LHDC;
    }else if (version == 4){
        lhdcBT->enc.llac = llac_encoder_new();
        lhdcBT->enc_type = ENC_TYPE_LLAC;
    }else{
        lhdcBT->enc_type = ENC_TYPE_UNKNOWN;
        free(lhdcBT);
        lhdcBT = NULL;
    }

    return lhdcBT;
}
int lhdcBT_init_encoder(HANDLE_LHDC_BT handle,int sampling_freq, int bitPerSample, int bitrate_inx,
    int dualChannel, int need_padding, int mtu, int interval) {

    int result = 0;
    unsigned int samples_per_frame = 0;
    lhdc_cb_t * lhdcBT = (lhdc_cb_t *)handle;
    if (!lhdcBT)
    {
        ALOGE("%s: Handle is NULL!!!", __func__);
        return -1;
    }

    if (sampling_freq != 44100 && sampling_freq != 48000 && sampling_freq != 96000)
    {
      ALOGE("%s: Invalid Sample Rate (%d)!!!", __func__, sampling_freq);
      return -1;
    }

    if (bitPerSample != LHDCBT_SMPL_FMT_S16 && bitPerSample != LHDCBT_SMPL_FMT_S24)
    {
      ALOGE("%s: Invalid Bit Per Sample (%d)!!!", __func__, bitPerSample);
      return -1;
    }

    if (bitrate_inx < LHDCBT_QUALITY_LOW0 || bitrate_inx >= LHDCBT_QUALITY_MAX)
    {
      ALOGE("%s: invalid bit rate index (%d)!!!", __func__, bitrate_inx);
      return -1;
    }

    if (dualChannel != 0 && dualChannel != 1)
    {
      ALOGE("%s: invalid Channel mode (%d)!!!", __func__, dualChannel);
      return -1;
    }

    if (need_padding != 0)
    {
      ALOGE("%s: invalid need padding (%d)!!!", __func__, need_padding);
      return -1;
    }

    if (mtu <= 0 || mtu >= 4096 )
    {
      ALOGE("%s: invalid mtu (%d)!!!", __func__, mtu);
      return -1;
    }

    if (interval <= 0 || interval > 20) //default: 10ms or 20ms
    {
      ALOGE("%s: invalid interval (%d)!!!", __func__, interval);
      return -1;
    }

    enc_t * enc = &lhdcBT->enc;

    //reset ABR table index record
    gABR_table_index = 0;

    switch(lhdcBT->enc_type){
        case ENC_TYPE_LHDC:
           result = lhdc_encoder_init(enc->lhdc, sampling_freq, bitPerSample, bitrate_inx, dualChannel, need_padding, mtu, interval);
           samples_per_frame = lhdc_encoder_get_frame_len(enc->lhdc);

           //depend on bitrate:400 position in auto_bitrate_adjust_table_lhdc
           gABR_table_index = 3;
           break;
        case ENC_TYPE_LLAC:
            result = llac_encoder_init(enc->llac, sampling_freq, bitPerSample, bitrate_inx, mtu, interval);
            samples_per_frame = llac_encoder_get_frame_len(enc->llac);

            //depend on bitrate:400 position in auto_bitrate_adjust_table_lhdc
            gABR_table_index = 5;
           break;
        default:
        break;
    }

    if (result >= 0 && samples_per_frame > 0 && lhdcBT->ar_filter != NULL){
        // number of channels is fixed to "2"
        result = ar_process_init(lhdcBT->ar_filter, sampling_freq, bitPerSample, 2, samples_per_frame);
    }

    return result;
}

void lhdcBT_set_max_bitrate(HANDLE_LHDC_BT handle, int max_rate_index) {

    lhdc_cb_t * lhdcBT = (lhdc_cb_t *)handle;
    if (!lhdcBT)
    {
        ALOGE("%s: Handle is NULL!!!", __func__);
        return;
    }
    if (max_rate_index < LHDCBT_QUALITY_LOW0 || max_rate_index >= LHDCBT_QUALITY_MAX)
    {
      ALOGE("%s: invalid bit rate index (%d)!!!", __func__, max_rate_index);
      return;
    }
    enc_t * enc = &lhdcBT->enc;

    switch(lhdcBT->enc_type){
        case ENC_TYPE_LHDC:
            return lhdc_encoder_set_max_bitrate(enc->lhdc, max_rate_index);
        case ENC_TYPE_LLAC: {

            //return llac_encoder_init(enc->llac, sampling_freq, bitPerSample, bitrate_inx, mtu, interval);
            ALOGD("%s: LLAC not supported", __func__);
        }
        default:
        break;
    }
}


int lhdcBT_encode(HANDLE_LHDC_BT handle, void* p_pcm, unsigned char* p_stream){

    lhdc_cb_t * lhdcBT = (lhdc_cb_t *)handle;
    if (!lhdcBT)
    {
        ALOGE("%s: Handle is NULL!!!", __func__);
        return -1;
    }
    if (!p_pcm)
    {
        ALOGE("%s: p_pcm is NULL!!!", __func__);
        return -1;
    }
    if (!p_stream)
    {
        ALOGE("%s: p_stream is NULL!!!", __func__);
        return -1;
    }
    enc_t * enc = &lhdcBT->enc;

    switch(lhdcBT->enc_type){
        case ENC_TYPE_LHDC:
            return lhdc_encoder_encode(enc->lhdc, p_pcm, p_stream);

        case ENC_TYPE_LLAC: {

            //return llac_encoder_init(enc->llac, sampling_freq, bitPerSample, bitrate_inx, mtu, interval);
            ALOGD("%s: LLAC not supported", __func__);
            break;
        }
        default:
        break;
    }
    return -1;
}


int lhdcBT_encodeV3(HANDLE_LHDC_BT handle, void* p_pcm, unsigned char* out_put, uint32_t * written, uint32_t * out_frames){
    lhdc_cb_t * lhdcBT = (lhdc_cb_t *)handle;
    if (!lhdcBT)
    {
        ALOGE("%s: Handle is NULL!!!", __func__);
        return -1;
    }
    if (p_pcm == NULL)
    {
        ALOGE("%s: input pcm buffer ptr is NULL!!!", __func__);
        return -1;
    }
    if (out_put == NULL)
    {
        ALOGE("%s: output pcm buffer ptr is NULL!!!", __func__);
        return -1;
    }
    if (written == NULL)
    {
        ALOGE("%s: written address is NULL!!!", __func__);
        return -1;
    }
    if (out_frames == NULL)
    {
        ALOGE("%s: out_frames address is NULL!!!", __func__);
        return -1;
    }

    return lhdc_util_encv4_process( handle, p_pcm, out_put, written, out_frames);
}



int lhdcBT_get_block_Size(HANDLE_LHDC_BT handle){

    lhdc_cb_t * lhdcBT = (lhdc_cb_t *)handle;
    if (!lhdcBT)
    {
        ALOGE("%s: Handle is NULL!!!", __func__);
        return -1;
    }
    enc_t * enc = &lhdcBT->enc;

    switch(lhdcBT->enc_type){
        case ENC_TYPE_LHDC:
            return lhdc_encoder_get_frame_len(enc->lhdc);

        case ENC_TYPE_LLAC: {
            return llac_encoder_get_frame_len(enc->llac);   //llac_encoder_encode(enc->llac, p_pcm, out_put, written, out_fraems);
        }
        default:
        break;
    }
    return 0;
}


int lhdcBT_get_bitrate(HANDLE_LHDC_BT handle) {

    lhdc_cb_t * lhdcBT = (lhdc_cb_t *)handle;
    if (!lhdcBT)
    {
        ALOGE("%s: Handle is NULL!!!", __func__);
        return -1;
    }
    enc_t * enc = &lhdcBT->enc;

    switch(lhdcBT->enc_type){
        case ENC_TYPE_LHDC:
            return lhdc_encoder_get_target_bitrate(enc->lhdc);

        case ENC_TYPE_LLAC: {
            return llac_encoder_get_target_bitrate(enc->llac);  //llac_encoder_encode(enc->llac, p_pcm, out_put, written, out_fraems);
        }
        default:
        break;
    }
    return -1;
}



int lhdcBT_set_bitrate(HANDLE_LHDC_BT handle, int bitrate_inx){

    lhdc_cb_t * lhdcBT = (lhdc_cb_t *)handle;
    if (!lhdcBT)
    {
        ALOGE("%s: Handle is NULL!!!", __func__);
        return -1;
    }

    if(bitrate_inx < LHDCBT_QUALITY_LOW0 || bitrate_inx >= LHDCBT_QUALITY_MAX)
    {
      ALOGE("%s: invalid bit rate index (%d)!!!", __func__, bitrate_inx);
      return -1;
    }

    enc_t * enc = &lhdcBT->enc;

    gABR_table_index = 0;

    switch(lhdcBT->enc_type){
        case ENC_TYPE_LHDC: {
          lhdc_para_t *lhdc = enc->lhdc;
          if(lhdc == NULL) {
            ALOGD("%s: LHDC [Reset BiTrAtE] null ptr!",  __func__);
            return -1;
          }

          if(bitrate_inx == LHDCBT_QUALITY_RESET_AUTO) {
            if(lhdc->qualityStatus != LHDCBT_QUALITY_AUTO) {
              ALOGD("%s: LHDC [Reset BiTrAtE] only work in ABR! (%d)",  __func__, lhdc->qualityStatus);
              return -1;
            }
            // change bitrate only, do not update qualityStatus
            lhdc->lastBitrate = LHDC_ABR_DEFAULT_BITRATE;
            lhdc_util_reset_up_bitrate(ENC_TYPE_LHDC, lhdc);
            lhdc_util_reset_down_bitrate(ENC_TYPE_LHDC, lhdc);
            if (lhdc->version >= 2) {
              lhdc->updateFramneInfo = true;
            }
            LossyEncoderSetTargetByteRate(lhdc->fft_blk, (lhdc->lastBitrate * 1000) / 8);
            ALOGD("%s: LHDC [Reset BiTrAtE] Reset bitrate to (%d)",  __func__, lhdc->lastBitrate);
            gABR_table_index = 3; //depend on bitrate:400 position in auto_bitrate_adjust_table_lhdc
            return 0;
          } else {
            // normal case, will update qualityStatus
            if (bitrate_inx == LHDCBT_QUALITY_AUTO) {
              //depend on bitrate:400 position in auto_bitrate_adjust_table_lhdc
              gABR_table_index = 3;
            }
            ALOGD("%s: LHDC set bitrate_inx %d", __func__, bitrate_inx);
            return lhdc_encoder_set_bitrate(lhdc, bitrate_inx);
          }
        }

        case ENC_TYPE_LLAC: {
          llac_para_t * llac = enc->llac;
          if(llac == NULL) {
            ALOGD("%s: LLAC [Reset BiTrAtE] null ptr!",  __func__);
            return -1;
          }

          if(bitrate_inx == LHDCBT_QUALITY_RESET_AUTO) {
            if(llac->qualityStatus != LHDCBT_QUALITY_AUTO) {
              ALOGD("%s: LLAC [Reset BiTrAtE] only work in ABR! (%d)",  __func__, llac->qualityStatus);
              return -1;
            }
            // change bitrate only, do not update qualityStatus
            llac->lastBitrate = LLAC_ABR_DEFAULT_BITRATE;
            ALOGD("%s: LLAC [Reset BiTrAtE] Reset bitrate to (%d)", __func__, llac->lastBitrate);
            llac_enc_set_bitrate(llac->lastBitrate * 1000, &llac->out_nbytes, &llac->real_bitrate, llac->lh4_enc);
            llac->updateFramneInfo = true;
            lhdc_util_reset_up_bitrate(ENC_TYPE_LLAC, llac);
            lhdc_util_reset_down_bitrate(ENC_TYPE_LLAC, llac);
            gABR_table_index = 5; //depend on bitrate:400 position in auto_bitrate_adjust_table_llac
            return 0;
          } else {
            //return llac_encoder_init(enc->llac, sampling_freq, bitPerSample, bitrate_inx, mtu, interval);
            ALOGD("%s: LLAC not supported", __func__);
            return -1;
          }
        }
        default:
        break;
    }
    return -1;
}



int lhdcBT_adjust_bitrate(HANDLE_LHDC_BT handle, size_t queueLen) {

    lhdc_cb_t * lhdcBT = (lhdc_cb_t *)handle;
    if (!lhdcBT)
    {
        ALOGE("%s: Handle is NULL!!!", __func__);
        return -1;
    }

    if (queueLen < 0)
    {
        ALOGE("%s: Invalid queue Len (%zu)!!!", __func__, queueLen);
        return -1;
    }
    enc_t * enc = &lhdcBT->enc;

    switch(lhdcBT->enc_type){
        case ENC_TYPE_LHDC:
            return lhdc_encoder_adjust_bitrate(enc->lhdc, queueLen);

        case ENC_TYPE_LLAC: {
            return llac_encoder_adjust_bitrate(enc->llac, queueLen);
        }
        default:
        break;
    }
    return -1;
}

int lhdcBT_set_ext_func_state(HANDLE_LHDC_BT handle, lhdcBT_ext_func_field_t field, bool enabled,
    void * priv /*nullable*/, int priv_data_len){
    lhdc_cb_t * lhdcBT = (lhdc_cb_t *)handle;
    if (!lhdcBT)
    {
        ALOGE("%s: Handle is NULL!!!", __func__);
        return -1;
    }
    if (field < LHDCBT_EXT_FUNC_AR || field >= LHDCBT_EXT_FUNC_MAX)
    {
        ALOGE("%s: invalid field (%d) !!!", __func__, field);
        return -1;
    }
    enc_t * enc = &lhdcBT->enc;

//void LhdcExtFuncArEnable(FFT_BLOCK *fb, int enable_ar);
//void LhdcExtFuncJasEnable(FFT_BLOCK *fb, int enable_Jas);
//void LhdcExtFuncMetaEnable(FFT_BLOCK *fb, int enable_meta, unsigned char *pmeta_data, int meta_data_len, int conti_frame_cnt);
    switch(lhdcBT->enc_type){
        case ENC_TYPE_LHDC:
        {
            switch((lhdc_ext_func_t)field){
                case EXT_FUNC_AR:
                LhdcExtFuncArEnable(enc->lhdc->fft_blk, enabled ? 1 : 0);
                ALOGD("%s: lhdc AR func = %d", __func__, enabled);
                return 0;
                break;

                case EXT_FUNC_JAS:
                LhdcExtFuncJasEnable(enc->lhdc->fft_blk, enabled ? 1 : 0);
                ALOGD("%s: lhdc JAS func = %d", __func__, enabled);
                return 0;
                break;

                case EXT_FUNC_META:
                if (!priv || !priv_data_len) {
                    return -1;
                }
                LhdcExtFuncMetaEnable(enc->lhdc->fft_blk, enabled ? 1 : 0, priv, priv_data_len, 8);
                ALOGD("%s: lhdc Meta func = %d", __func__, enabled);
                return 0;
                break;

                default:
                break;
            }
        }
        break;

        case ENC_TYPE_LLAC:
        {
            int8_t f_inx =  -1;
            uint8_t f_enbaled = enabled ? 1 : 0;
            switch((lhdc_ext_func_t)field){
                case EXT_FUNC_AR:
                f_inx = EXTRA_FUNC_AR;
                break;

                case EXT_FUNC_LARC:
                f_inx = EXTRA_FUNC_LARC;
                break;

                default:
                break;
            }
            if (f_inx >= 0)
            {
                ALOGD("%s: f_inx:%d, f_enbaled:%d, enc->llac->lh4_enc(%p)", __func__, f_inx, f_enbaled, enc->llac->lh4_enc);
                llac_enc_set_extra_func(f_inx,
                                        f_enbaled,
                                        enc->llac->lh4_enc);
                return 0;
            }
        }
        break;

        default:
        break;
    }

    return -1;
}

int lhdcBT_get_ext_func_state(HANDLE_LHDC_BT handle, lhdcBT_ext_func_field_t field, bool * enabled){
    lhdc_cb_t * lhdcBT = (lhdc_cb_t *)handle;
    if (!lhdcBT)
    {
        ALOGE("%s: Handle is NULL!!!", __func__);
        return -1;
    }
    if (!enabled)
    {
        ALOGE("%s: enabled is NULL!!!", __func__);
        return -1;
    }
    enc_t * enc = &lhdcBT->enc;
    *enabled = lhdc_util_get_ext_func_state(lhdcBT->enc_type, enc, (lhdc_ext_func_t)field);


    return 0;
}

int lhdcBT_set_hasMinBitrateLimit(HANDLE_LHDC_BT handle, bool enabled ){
    lhdc_cb_t * lhdcBT = (lhdc_cb_t *)handle;
    if (!lhdcBT)
    {
        ALOGE("%s: Handle is NULL!!!", __func__);
        return -1;
    }
    enc_t * enc = &lhdcBT->enc;


    switch(lhdcBT->enc_type){
        case ENC_TYPE_LHDC:
            ALOGD("%s:set value (%d)",  __func__, enabled);
            enc->lhdc->hasMinBitrateLimit = enabled;
            break;

        case ENC_TYPE_LLAC: {
            ALOGD("%s: ENC_TYPE_LLAC unsupported!!!", __func__);
            break;
        }
        default:
        break;
    }

    return 0;
}

/*
******************************************************************
 Extend API functions group
******************************************************************
*/

//
// META
//
static int lhdcBT_set_cfg_meta_v1(HANDLE_LHDC_BT handle, const char* userConfig, const int configLen) 
{
    lhdc_cb_t * lhdcBT = (lhdc_cb_t *)handle;
    PST_LHDC_SET_META pset_meta = (PST_LHDC_SET_META)userConfig;

    unsigned char  *pmeta_metadata = NULL;

    if (handle == NULL) {
        ALOGE("(LHDC-exAPI) %s: Handle error!(%p)",  __func__, handle);
        return EXTEND_FUNC_RET_INVALID_HANDLE;
    }

    if (userConfig == NULL)
    {
        ALOGE("(LHDC-exAPI) %s: User Config error!(%p)",  __func__, userConfig);
        return EXTEND_FUNC_RET_INVALID_PARAMETER;		
    }
	
    if (configLen < (int) sizeof (ST_LHDC_SET_META))
    {
        // Buffer is to small
    	ALOGE("(LHDC-exAPI) %s: Buffer too small(req:%d)",  __func__, (int)sizeof(ST_LHDC_SET_META));
        return EXTEND_FUNC_RET_BUF_UNDERRUN;
    }




    if (pset_meta->meta_ver != META_ST_VER_V2)
    {
    	ALOGE("(LHDC-exAPI) %s: ver not match",  __func__);
        return EXTEND_FUNC_RET_INVALID_PARAMETER;
    }
	
    if (pset_meta->meta_mem_size != (int) sizeof (ST_LHDC_SET_META))
    {
    	ALOGE("(LHDC-exAPI) %s: mata size not match", __func__);
        return EXTEND_FUNC_RET_INVALID_PARAMETER;
    }

    if (configLen < ((int) pset_meta->meta_metadata_length) + ((int) sizeof (ST_LHDC_SET_META)))
    {
    	ALOGE("(LHDC-exAPI) %s: cfg size too small", __func__);
        return EXTEND_FUNC_RET_INVALID_PARAMETER;
    }

    enc_t * enc = &lhdcBT->enc;

    switch(lhdcBT->enc_type)
    {
    case ENC_TYPE_LHDC:
        ALOGD("(LHDC-exAPI) %s: ENC_TYPE_LHDC", __func__);
        pmeta_metadata = (unsigned char*) (pset_meta + 1);

        LhdcExtFuncMetaEnable(enc->lhdc->fft_blk, 
                              pset_meta->meta_enable, 
                              pmeta_metadata, 
                              pset_meta->meta_metadata_length, 
                              pset_meta->meta_set);
        break;

    case ENC_TYPE_LLAC:
    	ALOGD("(LHDC-exAPI) %s: ENC_TYPE_LLAC", __func__);
        return EXTEND_FUNC_RET_FUNC_NOT_SUPPORT;
        break;

    default:
        return EXTEND_FUNC_RET_FUNC_NOT_SUPPORT;
        break;
    }


    return EXTEND_FUNC_RET_OK;
}


static int lhdcBT_get_cfg_meta_v1(HANDLE_LHDC_BT handle, char* userConfig, const int configLen) 
{
    lhdc_cb_t * lhdcBT = (lhdc_cb_t *)handle;
    PST_LHDC_GET_META pget_meta = (PST_LHDC_GET_META)userConfig;
    bool jas_enabled = false;

    if (handle == NULL) {
        ALOGE("(LHDC-exAPI) %s: Handle error!(%p)",  __func__, handle);
        return EXTEND_FUNC_RET_INVALID_HANDLE;
    }

    if (userConfig == NULL)
    {
        ALOGE("(LHDC-exAPI) %s: User Config error!(%p)",  __func__, userConfig);
        return EXTEND_FUNC_RET_INVALID_PARAMETER;		
    }

    if (configLen < (int) sizeof (ST_LHDC_GET_META))
    {
         // Buffer is to small
    	ALOGE("(LHDC-exAPI) %s: Buffer too small(req:%d)",  __func__, (int)sizeof(ST_LHDC_GET_META));
        return EXTEND_FUNC_RET_BUF_UNDERRUN;
    }


    enc_t * enc = &lhdcBT->enc;

    switch(lhdcBT->enc_type)
    {
    case ENC_TYPE_LHDC:
        {
        	ALOGD("(LHDC-exAPI) %s: ENC_TYPE_LHDC",  __func__);
			pget_meta->meta_ver = META_ST_VER_V2;
			pget_meta->meta_mem_size = (int) sizeof (ST_LHDC_GET_META);
			pget_meta->meta_st = (LhdcGetExtFuncState(enc->lhdc->fft_blk, EXT_FUNC_META) << 1) | 0x01;  // Get current frame include metadata or not

			//2021/06/10: Append JAS status her ,let UI can get JAS status from LHDC,
			jas_enabled = lhdc_util_get_ext_func_state(lhdcBT->enc_type, enc, EXT_FUNC_JAS);
			if(jas_enabled)
			{
				pget_meta->jas_status = 1;
			}
			else
			{
				pget_meta->jas_status = 0;
			}
        }
        break;

    case ENC_TYPE_LLAC:
    	ALOGD("(LHDC-exAPI) %s: ENC_TYPE_LLAC",  __func__);
        return EXTEND_FUNC_RET_FUNC_NOT_SUPPORT;
        break;

    default:
        return EXTEND_FUNC_RET_FUNC_NOT_SUPPORT;
        break;
    }


    return EXTEND_FUNC_RET_OK;
}


//
// AR
//
static int Ar_set_ext_func_state(HANDLE_LHDC_BT handle, bool enabled){
    lhdc_cb_t * lhdcBT = (lhdc_cb_t *)handle;
    if (!lhdcBT)
    {
        ALOGD("(LHDC-exAPI) %s: Handle is NULL!!!", __func__);
        return -1;
    }
    enc_t * enc = &lhdcBT->enc;

    switch(lhdcBT->enc_type){
        case ENC_TYPE_LHDC:
        {
            LhdcExtFuncArEnable(enc->lhdc->fft_blk, enabled ? 1 : 0);
            ALOGD("(LHDC-exAPI) %s: lhdc AR func = %d", __func__, enabled);
        }
        break;
        case ENC_TYPE_LLAC:
        {
            int8_t f_inx = EXTRA_FUNC_AR;
            uint8_t f_enbaled = enabled ? 1 : 0;
            ALOGD("(LHDC-exAPI) %s: f_enbaled:%d, enc->llac->lh4_enc(%p)", __func__, f_enbaled, enc->llac->lh4_enc);
            llac_enc_set_extra_func(f_inx, f_enbaled, enc->llac->lh4_enc);
        }
        break;
        default:
        break;
    }
    return 0;
}

static int lhdcBT_set_data_gyro_2d_v1(HANDLE_LHDC_BT handle, const char *userData, const int dataLen) {

    PST_LHDC_AR_GYRO pargyro = (PST_LHDC_AR_GYRO) userData;

    lhdc_cb_t * lhdcBT = (lhdc_cb_t *)handle;

    enc_t * enc = &lhdcBT->enc;

    bool ar_enabled = false;

    int res;

    if (handle == NULL) {
        ALOGE("(LHDC-exAPI) %s: Handle error!(%p)",  __func__, handle);
        return EXTEND_FUNC_RET_INVALID_HANDLE;
    }

    if (userData == NULL)
    {
        ALOGE("(LHDC-exAPI) %s: User Data error!(%p)",  __func__, userData);
        return EXTEND_FUNC_RET_INVALID_PARAMETER;		
    }

    if (dataLen < (int) sizeof (ST_LHDC_AR_GYRO))
    {
         // Buffer is to small
    	ALOGE("(LHDC-exAPI) %s: Buffer too small",  __func__);
        return EXTEND_FUNC_RET_BUF_UNDERRUN;
    }

	ALOGD("(LHDC-exAPI) %s: gyro[%d %d %d]",  __func__,
			pargyro->world_coordinate_x, pargyro->world_coordinate_y, pargyro->world_coordinate_z);

    ar_enabled = lhdc_util_get_ext_func_state(lhdcBT->enc_type, enc, EXT_FUNC_AR);

#ifdef AR_ALWAYS_ON
    ar_enabled = true;
#endif

    if (ar_enabled)
    {
        res = ar_set_gyro_pos(lhdcBT->ar_filter, pargyro->world_coordinate_x, pargyro->world_coordinate_y, pargyro->world_coordinate_z);
    }
    else
    {
        ALOGD("(LHDC-exAPI) %s: AR not enabled!",  __func__);
        return EXTEND_FUNC_RET_FUNC_NOT_SUPPORT;
    }

    if(res != 0)
    {
    	ALOGD("(LHDC-exAPI) %s: set_gyro_pos error(%d)",  __func__, res);
        return EXTEND_FUNC_RET_ERROR;
    }

    return EXTEND_FUNC_RET_OK;
}


static int lhdcBT_set_cfg_ar_v3(HANDLE_LHDC_BT handle, const char *userConfig, const int configLen) {

    lhdc_cb_t * lhdcBT = (lhdc_cb_t *)handle;

    enc_t * enc = &lhdcBT->enc;

    PST_LHDC_AR pset_ar_cfg = (PST_LHDC_AR)userConfig;

    bool ar_enabled = false;

    int res;

    if (handle == NULL) {
        ALOGE("(LHDC-exAPI) %s: Handle error!(%p)",  __func__, handle);
        return EXTEND_FUNC_RET_INVALID_HANDLE;
    }

    if (userConfig == NULL)
    {
        ALOGE("(LHDC-exAPI) %s: User Config error!(%p)",  __func__, userConfig);
        return EXTEND_FUNC_RET_INVALID_PARAMETER;		
    }

    if (configLen < (int) sizeof (ST_LHDC_AR))
    {
         // Buffer is to small
    	ALOGE("(LHDC-exAPI) %s: Buffer too small",  __func__);
        return EXTEND_FUNC_RET_BUF_UNDERRUN;
    }
    ALOGD("(LHDC-exAPI) %s: config AR %d",  __func__, pset_ar_cfg->app_ar_enabled);

    ar_enabled = pset_ar_cfg->app_ar_enabled ? 1 : 0;

    Ar_set_ext_func_state(handle, ar_enabled);
    //LhdcExtFuncArEnable(enc->lhdc->fft_blk, pset_ar_cfg->app_ar_enabled ? 1 : 0);

    ar_enabled = lhdc_util_get_ext_func_state(lhdcBT->enc_type, enc, EXT_FUNC_AR);

    ALOGD("(LHDC-exAPI) %s: AR enabled %d",  __func__, ar_enabled);

#ifdef AR_ALWAYS_ON
    ar_enabled = true;
#endif

    if (ar_enabled)
    {
        res = ar_set_cfg(lhdcBT->ar_filter, &pset_ar_cfg->Ch1_Pos, &pset_ar_cfg->Ch1_L_PreGain, pset_ar_cfg->app_ar_enabled);
    }
    else
    {
        ALOGD("(LHDC-exAPI) %s: AR not enabled!",  __func__);
        return EXTEND_FUNC_RET_FUNC_NOT_SUPPORT;
    }

    if(res != 0)
    {
        return EXTEND_FUNC_RET_ERROR;
    }

    return EXTEND_FUNC_RET_OK;
}

	
static int lhdcBT_get_cfg_ar_v1(HANDLE_LHDC_BT handle, char *userConfig, const int configLen) {

    //PST_LHDC_AR pset_ar_cfg = (PST_LHDC_AR)userConfig;

    lhdc_cb_t * lhdcBT = (lhdc_cb_t *)handle;

    enc_t * enc = &lhdcBT->enc;

    bool ar_enabled = false;


    int res = 0;

    if (handle == NULL) {
        ALOGE("(LHDC-exAPI) %s: Handle error!(%p)",  __func__, handle);
        return EXTEND_FUNC_RET_INVALID_HANDLE;
    }

    if (userConfig == NULL)
    {
        ALOGE("(LHDC-exAPI) %s: User Config error!(%p)",  __func__, userConfig);
        return EXTEND_FUNC_RET_INVALID_PARAMETER;		
    }

    if (configLen <= (int) 0)
    {
         // Buffer is to small
    	ALOGE("(LHDC-exAPI) %s: Buffer too small",  __func__);
        return EXTEND_FUNC_RET_BUF_UNDERRUN;
    }

    ar_enabled = lhdc_util_get_ext_func_state(lhdcBT->enc_type, enc, EXT_FUNC_AR);

#ifdef AR_ALWAYS_ON
    ar_enabled = true;
#endif

    if (ar_enabled)
    {
    	ALOGD("(LHDC-exAPI) %s: AR enabled",  __func__);
    	//2021/11/09: ar_get_cfg not supported yet
        //res = ar_get_cfg(lhdcBT->ar_filter, &pset_ar_cfg->Ch1_Pos, &pset_ar_cfg->Ch1_L_PreGain);
    	return EXTEND_FUNC_RET_ERROR;
    }
    else
    {
        ALOGD("(LHDC-exAPI) %s: AR not enabled!",  __func__);
        return EXTEND_FUNC_RET_FUNC_NOT_SUPPORT;
    }

    if(res != 0)
    {
        return EXTEND_FUNC_RET_ERROR;
    }

    ALOGD("(LHDC-exAPI) %s: End OK",  __func__);
    return EXTEND_FUNC_RET_OK;	
}



/*
******************************************************************
 Extend API library public functions group
******************************************************************
*/

// 1. API -- Set User Config (Extend)
int lhdcBT_set_user_exconfig(HANDLE_LHDC_BT handle, char* userConfig, int clen) {

    unsigned char *pucConfig = (unsigned char *) userConfig;
    unsigned int exFuncVer;
    unsigned int exFuncCode;
    int result = EXTEND_FUNC_RET_OK;

    //ALOGD("(LHDC-exAPI) %s: enter\n",  __func__);

    if (handle == NULL) {
        ALOGE("(LHDC-exAPI) %s: Handle error!(%p)",  __func__, handle);
        return EXTEND_FUNC_RET_INVALID_HANDLE;
    }

    if (userConfig == NULL)
    {
        ALOGE("(LHDC-exAPI) %s: User Config error!(%p)",  __func__, userConfig);
        return EXTEND_FUNC_RET_INVALID_PARAMETER;		
    }

    if (clen < (int)EXTEND_FUNC_CODE_MIN_BUFFER_LEN)
    {
         // Buffer is to small
    	ALOGE("(LHDC-exAPI) %s: Buffer too small",  __func__);
        return EXTEND_FUNC_RET_BUF_UNDERRUN;
    }

    exFuncVer = (((unsigned int) pucConfig[3]) & ((unsigned int)0xff)) |
               ((((unsigned int) pucConfig[2]) & ((unsigned int)0xff)) << 8)  |
               ((((unsigned int) pucConfig[1]) & ((unsigned int)0xff)) << 16) |
               ((((unsigned int) pucConfig[0]) & ((unsigned int)0xff)) << 24);
    exFuncCode = (((unsigned int) pucConfig[7]) & ((unsigned int)0xff)) |
                ((((unsigned int) pucConfig[6]) & ((unsigned int)0xff)) << 8)  |
                ((((unsigned int) pucConfig[5]) & ((unsigned int)0xff)) << 16) |
                ((((unsigned int) pucConfig[4]) & ((unsigned int)0xff)) << 24);

    switch (exFuncCode) {
    case EXTEND_FUNC_CODE_SET_CONFIG_META:
	
        switch (exFuncVer) {
        case EXTEND_FUNC_VER_SET_CONFIG_META_V1:
        	ALOGD("(LHDC-exAPI) %s: SET_CONFIG_META\n",  __func__);
            result = lhdcBT_set_cfg_meta_v1 (handle, userConfig, clen);
            break;

        default:
            ALOGE("(LHDC-exAPI) %s: Invalid Ex. Function Version!(0x%X)",  __func__, exFuncVer);
            return EXTEND_FUNC_RET_VERSION_NOT_SUPPORT;
        }
        break;
		
    case EXTEND_FUNC_CODE_SET_CONFIG_AR:
        switch (exFuncVer) {
        case EXTEND_FUNC_VER_SET_CONFIG_AR_V3:
        	ALOGD("(LHDC-exAPI) %s: SET_CONFIG_AR\n",  __func__);
            result = lhdcBT_set_cfg_ar_v3 (handle, userConfig, clen);
            break;

        default:
            ALOGE("(LHDC-exAPI) %s: Invalid Ex. Function Version!(0x%X)",  __func__, exFuncVer);
            return EXTEND_FUNC_RET_VERSION_NOT_SUPPORT;
        }
        break;

    default:
        ALOGE("(LHDC-exAPI) %s: Invalid Ex. Function Code!(0x%X)",  __func__, exFuncCode);
        return EXTEND_FUNC_RET_FUNC_NOT_SUPPORT;
    } // switch (exFuncCode)

    return result;
}


// 2. API -- Get User Config (Extend)
int lhdcBT_get_user_exconfig(HANDLE_LHDC_BT handle, char* userConfig, int clen) {


    unsigned char *pucConfig = (unsigned char *) userConfig;
    unsigned int exFuncVer;
    unsigned int exFuncCode;
    int result = EXTEND_FUNC_RET_OK;

    //ALOGD("%s:(LHDC-exAPI) enter\n",  __func__);

    if (handle == NULL) {
        ALOGE("(LHDC-exAPI) %s: Handle error!(%p)",  __func__, handle);
        return EXTEND_FUNC_RET_INVALID_HANDLE;
    }

    if (userConfig == NULL)
    {
        ALOGE("(LHDC-exAPI) %s: User Config error!(%p)",  __func__, userConfig);
        return EXTEND_FUNC_RET_INVALID_PARAMETER;		
    }

    if (clen < (int)EXTEND_FUNC_CODE_MIN_BUFFER_LEN)
    {
         // Buffer is to small
    	ALOGE("(LHDC-exAPI) %s: Buffer too small",  __func__);
        return EXTEND_FUNC_RET_BUF_UNDERRUN;
    }

    exFuncVer = (((unsigned int) pucConfig[3]) & ((unsigned int)0xff)) |
               ((((unsigned int) pucConfig[2]) & ((unsigned int)0xff)) << 8)  |
               ((((unsigned int) pucConfig[1]) & ((unsigned int)0xff)) << 16) |
               ((((unsigned int) pucConfig[0]) & ((unsigned int)0xff)) << 24);
    exFuncCode = (((unsigned int) pucConfig[7]) & ((unsigned int)0xff)) |
                ((((unsigned int) pucConfig[6]) & ((unsigned int)0xff)) << 8)  |
                ((((unsigned int) pucConfig[5]) & ((unsigned int)0xff)) << 16) |
                ((((unsigned int) pucConfig[4]) & ((unsigned int)0xff)) << 24);

    switch (exFuncCode) {
    case EXTEND_FUNC_CODE_GET_CONFIG_META:
	
        switch (exFuncVer) {
        case EXTEND_FUNC_VER_GET_CONFIG_META_V1:
        	ALOGD("(LHDC-exAPI) %s: GET_CONFIG_META\n",  __func__);
            result = lhdcBT_get_cfg_meta_v1 (handle, userConfig, clen);
            break;

        default:
            ALOGE("(LHDC-exAPI) %s: Invalid Ex. Function Version!(0x%X)",  __func__, exFuncVer);
            return EXTEND_FUNC_RET_VERSION_NOT_SUPPORT;
        }
        break;

    case EXTEND_FUNC_CODE_GET_CONFIG_AR:
	
        switch (exFuncVer) {
        case EXTEND_FUNC_VER_GET_CONFIG_AR_V1:
        	ALOGD("(LHDC-exAPI) %s: GET_CONFIG_AR\n",  __func__);
            result = lhdcBT_get_cfg_ar_v1 (handle, userConfig, clen);
            break;

        default:
            ALOGE("(LHDC-exAPI) %s: Invalid Ex. Function Version!(0x%X)",  __func__, exFuncVer);
            return EXTEND_FUNC_RET_VERSION_NOT_SUPPORT;
        }
        break;

    default:
        ALOGE("(LHDC-exAPI) %s: Invalid Ex. Function Code!(0x%X)",  __func__, exFuncCode);
        return EXTEND_FUNC_RET_FUNC_NOT_SUPPORT;
    } // switch (exFuncCode)

    return result;

}

// 3. API -- Set User Data (Extend)
void lhdcBT_set_user_exdata(HANDLE_LHDC_BT handle, char* userData, int clen) {

    unsigned char *pucData = (unsigned char *) userData;
    unsigned int exFuncVer;
    unsigned int exFuncCode;
    int result = EXTEND_FUNC_RET_OK;

    //ALOGD("(LHDC-exAPI) %s: enter\n",  __func__);

    if (handle == NULL) {
        ALOGE("(LHDC-exAPI) %s: Handle error!(%p)",  __func__, handle);
        return; // EXTEND_FUNC_RET_INVALID_HANDLE;
    }

    if (userData == NULL)
    {
        ALOGE("(LHDC-exAPI) %s: User Data error!(%p)",  __func__, userData);
        return; // EXTEND_FUNC_RET_INVALID_PARAMETER;		
    }

    if (clen < (int)EXTEND_FUNC_CODE_MIN_BUFFER_LEN)
    {
         // Buffer is to small
    	ALOGE("(LHDC-exAPI) %s: Buffer too small",  __func__);
        return;
    }

    exFuncVer = (((unsigned int) pucData[3]) & ((unsigned int)0xff)) |
               ((((unsigned int) pucData[2]) & ((unsigned int)0xff)) << 8)  |
               ((((unsigned int) pucData[1]) & ((unsigned int)0xff)) << 16) |
               ((((unsigned int) pucData[0]) & ((unsigned int)0xff)) << 24);
    exFuncCode = (((unsigned int) pucData[7]) & ((unsigned int)0xff)) |
                ((((unsigned int) pucData[6]) & ((unsigned int)0xff)) << 8)  |
                ((((unsigned int) pucData[5]) & ((unsigned int)0xff)) << 16) |
                ((((unsigned int) pucData[4]) & ((unsigned int)0xff)) << 24);

    switch (exFuncCode) {
    case EXTEND_FUNC_CODE_SET_DATA_GYRO2D:
	
        switch (exFuncVer) {
        case EXTEND_FUNC_VER_SET_DATA_GYRO2D_V1:
        	ALOGD("(LHDC-exAPI) %s: SET_DATA_GYRO\n",  __func__);
            result = lhdcBT_set_data_gyro_2d_v1 (handle, userData, clen);
            break;

        default:
            ALOGE("(LHDC-exAPI) %s: Invalid Ex. Function Version!(0x%X)",  __func__, exFuncVer);
            return; // EXTEND_FUNC_RET_FUNC_NOT_SUPPORT;
        }
        break;
		
    default:
        ALOGE("(LHDC-exAPI) %s: Invalid Ex. Function Code!(0x%X)",  __func__, exFuncCode);
        return; // EXTEND_FUNC_RET_FUNC_NOT_SUPPORT;
    } // switch (exFuncCode)

    //return result;
}


// 4. API -- Get Version
int lhdcBT_get_user_exApiver(HANDLE_LHDC_BT handle, char *version, int clen) {

    unsigned char *pucApiVer = (unsigned char *) version;
    unsigned int exFuncVer = 0;
    unsigned int exFuncCode = 0;
    unsigned int exFuncMinBufSize = 0;
    int result = EXTEND_FUNC_RET_OK;

    //ALOGD("(LHDC-exAPI) %s: enter\n",  __func__);

    if (version == NULL)
    {
        ALOGE("(LHDC-exAPI) %s: API verion buffer error!(%p)",  __func__, version);
        return EXTEND_FUNC_RET_INVALID_PARAMETER;
    }

    if (handle == NULL) {
        ALOGE("(LHDC-exAPI) %s: Handle error!(%p)",  __func__, handle);
        return EXTEND_FUNC_RET_INVALID_HANDLE;
    }

    if (clen < (int)EXTEND_FUNC_MIN_BUFFER_LEN_GET_API_VERSION_V1)
    {
         // Buffer is to small
    	ALOGE("(LHDC-exAPI) %s: Buffer too small",  __func__);
        return EXTEND_FUNC_RET_BUF_UNDERRUN;
    }

    /*
    ALOGD("%s: clen=%d, APICode[4:7] = [0x%02X, 0x%02X, 0x%02X, 0x%02X]",  __func__,
        clen, pucApiVer[4], pucApiVer[5], pucApiVer[6], pucApiVer[7]);
        */

    exFuncCode = (((unsigned int) pucApiVer[7]) & ((unsigned int)0xff)) |
                ((((unsigned int) pucApiVer[6]) & ((unsigned int)0xff)) << 8)  |
                ((((unsigned int) pucApiVer[5]) & ((unsigned int)0xff)) << 16) |
                ((((unsigned int) pucApiVer[4]) & ((unsigned int)0xff)) << 24);

    switch (exFuncCode) {
    // Config API
    case EXTEND_FUNC_CODE_SET_CONFIG_META:
        exFuncVer = EXTEND_FUNC_VER_SET_CONFIG_META_V1;
        break;

    case EXTEND_FUNC_CODE_SET_CONFIG_AR:
        exFuncVer = EXTEND_FUNC_VER_SET_CONFIG_AR_V3;
        break;

    case EXTEND_FUNC_CODE_GET_CONFIG_META:
        exFuncVer = EXTEND_FUNC_VER_GET_CONFIG_META_V1;
        break;

    case EXTEND_FUNC_CODE_GET_CONFIG_AR:
        exFuncVer = EXTEND_FUNC_VER_GET_CONFIG_AR_V1;
        break;

    // Data API
    case EXTEND_FUNC_CODE_SET_DATA_GYRO2D:
        exFuncVer = EXTEND_FUNC_VER_SET_DATA_GYRO2D_V1;
        break;

    // A2DP codec Specific API
    case EXTEND_FUNC_CODE_GET_SPECIFIC:
        exFuncVer = EXTEND_FUNC_VER_GET_SPECIFIC_V2;
        exFuncMinBufSize = LHDC_EXTEND_FUNC_CONFIG_TOTAL_FIXED_SIZE_V2;
        break;

    default:
        ALOGE("(LHDC-exAPI) %s: Invalid Ex. Function Code!(0x%X)",  __func__, exFuncCode);
        return EXTEND_FUNC_RET_FUNC_NOT_SUPPORT;
    } // switch (exFuncCode)

    /* fill in current version of target API */
    pucApiVer[3] = (unsigned char) (exFuncVer & ((unsigned int)0xff));
    pucApiVer[2] = (unsigned char) ((exFuncVer >> 8) & ((unsigned int)0xff));
    pucApiVer[1] = (unsigned char) ((exFuncVer >> 16) & ((unsigned int)0xff));
    pucApiVer[0] = (unsigned char) ((exFuncVer >> 24) & ((unsigned int)0xff));

    /**** Version Control Field: *****/
    if(exFuncVer >= EXTEND_FUNC_GENERIC_VERSION_NUMBER_V2 &&
        clen >= EXTEND_FUNC_MIN_BUFFER_LEN_GET_API_VERSION_V2)
    {
      /* After API V2.0.0.0, append "minimum required buffer size" into API response */
      pucApiVer[11] = (unsigned char) (exFuncMinBufSize & ((unsigned int)0xff));
      pucApiVer[10] = (unsigned char) ((exFuncMinBufSize >> 8) & ((unsigned int)0xff));
      pucApiVer[9] = (unsigned char) ((exFuncMinBufSize >> 16) & ((unsigned int)0xff));
      pucApiVer[8] = (unsigned char) ((exFuncMinBufSize >> 24) & ((unsigned int)0xff));
    }

    ALOGD("(LHDC-exAPI) %s: return Ver=0x[%02X %02X %02X %02X]",  __func__,
        pucApiVer[4], pucApiVer[5], pucApiVer[6], pucApiVer[7]);

    return result;
}

