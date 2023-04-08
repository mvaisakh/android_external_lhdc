
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "lhdcBT_dec.h"


#define LOG_NDEBUG 0
#define LOG_TAG "lhdcBT_dec"
#include <cutils/log.h>

static uint8_t serial_no = 0xff;

static int assemble_lhdc_packet(uint8_t *input, uint32_t input_len, uint8_t **pLout, uint32_t *pLlen, int upd_seq_no);

// description
//   a function to log information in LHDC decoder library
// Parameter
//   config: configuration data for LHDC v4 decoder
// return:
//   == 0: succeed
//   < 0: error
static void print_log_cb(char*  msg)
{
	if (msg == NULL)
	{
		return;
	}

    ALOGD("[WL50-ex] %s: %s", __func__, msg);
}

// description
//   init. LHDC v4 decoder 
// Parameter
//   config: configuration data for LHDC v4 decoder
// return:
//   == 0: succeed
//   < 0: error
int lhdcBT_dec_init_decoder(tLHDCV3_DEC_CONFIG *config)
{
	//ALOGD("[WL50] %s: enter", __func__);
    if (config == NULL)
	{
		return LHDCBT_DEC_FUNC_FAIL;
	}

    //ALOGD("[WL50] %s: bits_depth:%d sample_rate=%d version=%d", __func__,	config->bits_depth, config->sample_rate, config->version);

    if ((config->bits_depth != 16) && (config->bits_depth != 24))
	{
		return LHDCBT_DEC_FUNC_FAIL;
	}

    if ((config->sample_rate != 44100) && (config->sample_rate != 48000) &&
	    (config->sample_rate != 96000))
	{
		return LHDCBT_DEC_FUNC_FAIL;
	}

    if ((config->version != VERSION_3) && (config->version != VERSION_4) &&
	    (config->version != VERSION_LLAC))
	{
		return LHDCBT_DEC_FUNC_FAIL;
	}

	lhdc_register_log_cb(&print_log_cb);

    //ALOGD("[WL50] %s: start lhdcInit..", __func__);
    lhdcInit(config->bits_depth, config->sample_rate, 400000, config->version);

    lhdcChannelSelsect(LHDC_OUTPUT_STEREO);

    serial_no = 0xff;
    //ALOGD("[WL50] %s: end", __func__);
	return LHDCBT_DEC_FUNC_SUCCEED;
}



// description
//   check whether all frames of one packet are in buffer?
// Parameter
//   frameData: pointer to input buffer
//   frameBytes: length (bytes) of input buffer pointed by frameData
// return:
//   == 0: succeed
//   < 0: error
int lhdcBT_dec_check_frame_data_enough(const uint8_t *frameData, uint32_t frameBytes, uint32_t *packetBytes)
{
	uint8_t *frameDataStart = (uint8_t *)frameData;
    uint8_t *in_buf = NULL;
    uint32_t in_len = 0;
	uint32_t frame_num = 0;
	lhdc_frame_Info_t lhdc_frame_Info;
	uint32_t ptr_offset = 0;
	bool fn_ret;


    ALOGD("[WL50] %s: enter, frameBytes (%d)", __func__, (int)frameBytes);

	if ((frameData == NULL) || (packetBytes == NULL))
	{
		return LHDCBT_DEC_FUNC_FAIL;
	}
	
	*packetBytes = 0;

    frame_num = assemble_lhdc_packet(frameDataStart, frameBytes, &in_buf, &in_len, LHDCBT_DEC_NOT_UPD_SEQ_NO);
    if (frame_num == 0)
	{
        ALOGD("%s: assemble_lhdc_packet (%d)", __func__, (int)frame_num);
		return LHDCBT_DEC_FUNC_SUCCEED;
	}
	//else if (frame_num < 0)
	//{
	//	// Fail to check packet header (including invalid packet number)!
	//	return frame_num;
	//}
	
	ALOGD("[WL50] %s: in_buf (%p), frameData (%p), in_len (%d), frame_num (%d)", __func__, in_buf, frameData, (int)in_len, (int) frame_num);
	
	ptr_offset = 0;

	while ((frame_num > 0) && (ptr_offset < in_len))
	{
		fn_ret = lhdcFetchFrameInfo (in_buf + ptr_offset, &lhdc_frame_Info);
		if (fn_ret == false)
		{
			ALOGD("[WL50] %s: lhdcFetchFrameInfo(%d) fail..", __func__, (int)frame_num);
			return LHDCBT_DEC_FUNC_FAIL;
		}
		
		ALOGD("[WL50] %s: lhdcFetchFrameInfo  frame_num (%d), ptr_offset (%d), lhdc_frame_Info.frame_len (%d), in_len (%d)", __func__, (int)frame_num, (int)ptr_offset, (int)lhdc_frame_Info.frame_len, (int)in_len);
		
		if ((ptr_offset + lhdc_frame_Info.frame_len) > in_len)
		{
			ALOGD("[WL50] %s: Not Enough... frame_num(%d), ptr_offset(%d), frame_len(%d), in_len (%d)", __func__, (int)frame_num, (int)ptr_offset, (int)lhdc_frame_Info.frame_len, (int)in_len);
			return LHDCBT_DEC_FUNC_INPUT_NOT_ENOUGH;
		}

        ptr_offset += lhdc_frame_Info.frame_len;

        frame_num--;
	}


	*packetBytes = ptr_offset;

    ALOGD("[WL50] %s: end ", __func__);
    return LHDCBT_DEC_FUNC_SUCCEED;

}


/*
	uint8_t llac_test_ptn[0x100] = {
		0x90, 0x01, 0x04, 0xF7, 0x7C, 0x65, 0xEA, 0x83, 
		0xBB, 0x66, 0x4B, 0xA0, 0xF6, 0x51, 0xB8, 0x48, 
		0x0C, 0xE7, 0x9D, 0xB9, 0x3B, 0x28, 0x0E, 0x82, 
		0x73, 0xC6, 0xD7, 0xF9, 0xFC, 0x60, 0x34, 0xA9, 
		0xEA, 0x46, 0x03, 0xD4, 0xA9, 0x63, 0x41, 0x30, 
		0x37, 0x54, 0xBF, 0x47, 0x61, 0xAD, 0x7A, 0xB7, 
		0xF0, 0x6E, 0x49, 0x00, 0xC7, 0xF9, 0xC6, 0x23, 
		0x99, 0x01, 0xA1, 0x08, 0x79, 0xF4, 0x53, 0xC2, 
		0xDF, 0xD8, 0x6F, 0x81, 0xB9, 0x8D, 0x65, 0x71, 
		0xA2, 0x77, 0xF7, 0x0C, 0x65, 0x2D, 0x78, 0x80, 
		0xA8, 0xBA, 0xC0, 0xA0, 0xD4, 0x98, 0xB4, 0xF5, 
		0xB3, 0x6C, 0x61, 0x14, 0xD4, 0x93, 0xA5, 0x2D, 
		0x07, 0x4D, 0x49, 0x1B, 0x78, 0x9F, 0x4E, 0xC7, 
		0x28, 0x15, 0x35, 0x2E, 0x00, 0x44, 0xD5, 0x62, 
		0xA3, 0x95, 0x86, 0xE4, 0xDC, 0x9E, 0xD7, 0xAC, 
		0xC0, 0x90, 0x6D, 0x62, 0xED, 0xBA, 0x4A, 0x89, 
		0xAE, 0x7F, 0x50, 0xC6, 0xAD, 0x57, 0xBA, 0x47, 
		0xBA, 0x45, 0xEC, 0x0F, 0x99, 0xA6, 0xA0, 0x3F, 
		0xBD, 0x14, 0xAA, 0xBD, 0x74, 0x5B, 0x49, 0x7E, 
		0xF9, 0xD1, 0xB7, 0x6A, 0x4F, 0xF3, 0x31, 0xD9, 
		0x3D, 0x79, 0xBF, 0x79, 0x1C, 0x99, 0x74, 0xD7, 
		0x1B, 0x3B, 0x04, 0xF9, 0x61, 0xB4, 0xC3, 0x7B, 
		0x86, 0x6B, 0x5A, 0x6F, 0xAD, 0x95, 0xB2, 0xD7, 
		0x6D, 0xCC, 0xC8, 0x33, 0x0D, 0x87, 0xC6, 0x06, 
		0xFC, 0xD1, 0x17, 0x8C, 0xD0, 0x33, 0x37, 0x55, 
		0xA1, 0x07, 0x6F, 0x70, 0xD1, 0x97, 0x2A, 0x97, 
		0x44, 0xE8, 0x8A, 0xBA, 0xC1, 0x28, 0xA9, 0x23, 
		0x54, 0xD8, 0x89, 0x63, 0x56, 0x8B, 0x32, 0xA6, 
		0x38, 0xA1, 0xFA, 0x83, 0x87, 0x2A, 0x2B, 0x6F, 
		0xD7, 0x8E, 0x18, 0x79, 0xEC, 0x53, 0xAE, 0x10, 
		0xC8, 0xF6, 0x49, 0xD7, 0xA2, 0xC5, 0x00, 0xED, 
		0x62, 0xFD, 0xBA, 0x48, 0x00, 0x00, 0x00, 0x00};
	uint8_t llac_ptn[0x100];
*/

// description
//   decode all frames in one packet
// Parameter
//   frameData: pointer to input buffer
//   frameBytes: length (bytes) of input buffer pointed by frameData
//   pcmData: pointer to output buffer
//   pcmBytes: length (bytes) of pcm samples in output buffer
// return:
//   == 0: succeed
//   < 0: error
int lhdcBT_dec_decode(const uint8_t *frameData, uint32_t frameBytes, uint8_t* pcmData, uint32_t* pcmBytes, uint32_t bits_depth)
{
	uint8_t *frameDataStart = (uint8_t *)frameData;
    uint32_t dec_sum = 0;
	uint32_t lhdc_out_len = 0;
    uint8_t *in_buf = NULL;
    uint32_t in_len = 0;
	uint32_t frame_num = 0;
	lhdc_frame_Info_t lhdc_frame_Info;
	uint32_t ptr_offset = 0;
	bool fn_ret;
    uint32_t frame_samples;
	uint32_t frame_bytes;
	uint32_t pcmSpaceBytes;

    //ALOGD("[WL50] %s: enter, assemble_lhdc_packet(frameBytes %d)", __func__, (int)frameBytes);

    if ((frameData == NULL) || 
	    (pcmData == NULL) ||
		(pcmBytes == NULL))
	{
		return LHDCBT_DEC_FUNC_FAIL;
	}

    pcmSpaceBytes = *pcmBytes;
	*pcmBytes = 0;

/*
    if(frameBytes >= 16)
    {
		for(int i=0; i<16; i++)
		{
			ALOGD("[WL50] %s: dumpFrame[%d]= 0x%02X", __func__, i, (int)*(frameDataStart+i));
		}
    }
    else
    {
		for(int i=0; i<(int)frameBytes; i++)
		{
			ALOGD("[WL50] %s: dumpFrame[%d]= 0x%02X", __func__, i, (int)*(frameDataStart+i));
		}
    }
*/

    frame_num = assemble_lhdc_packet(frameDataStart, frameBytes, &in_buf, &in_len, LHDCBT_DEC_UPD_SEQ_NO);
    if (frame_num == 0)
	{
		return LHDCBT_DEC_FUNC_SUCCEED;
	}
	//ALOGD("[WL50] %s: frameData=0x%p, in_buf=0x%p, in_len=%d", __func__, frameData, in_buf, in_len);
    //ALOGD("[WL50] %s: get frame_num=%d", __func__, (int)frame_num);

    frame_samples = lhdcGetSampleSize ();
	if (bits_depth == 16)
	{
		frame_bytes = frame_samples * 2 * 2;
	}
	else
	{
		frame_bytes = frame_samples * 4 * 2;
	}
    ALOGD("[WL50] %s: frame_samples=%d", __func__, (int)frame_samples);

    ptr_offset = 0;
    dec_sum = 0;

	while ((frame_num > 0) && (ptr_offset < in_len))
	{
		fn_ret = lhdcFetchFrameInfo (in_buf + ptr_offset, &lhdc_frame_Info);
		if (fn_ret == false)
		{
			//ALOGD("[WL50] %s: lhdcFetchFrameInfo(%d) fail..", __func__, (int)frame_num);
			return LHDCBT_DEC_FUNC_FAIL;
		}

		if ((ptr_offset + lhdc_frame_Info.frame_len) > in_len)
		{
			return LHDCBT_DEC_FUNC_INPUT_NOT_ENOUGH;
		}

		if ((dec_sum + frame_bytes) > pcmSpaceBytes)
		{
			return LHDCBT_DEC_FUNC_OUTPUT_NOT_ENOUGH;
		}

 		//ALOGD("[WL50] %s: get ptr_offset=%d, dec_sum=%d", __func__, ptr_offset, dec_sum);
        lhdc_out_len = lhdcDecodeProcess(((uint8_t *)pcmData) + dec_sum, in_buf + ptr_offset, lhdc_frame_Info.frame_len);
        //ALOGD("[WL50] %s: lhdcDecodeProcess(frm=%d, frame_len=%d out_len=%d)..", __func__, (int)frame_num, (int)lhdc_frame_Info.frame_len, (int)lhdc_out_len);

        //if (lhdc_out_len % frame_samples)
        //{
        //    TRACE_A2DP_DECODER_I("[CP][LHDC]error!!! dec_sum: %d decode_temp: %d", dec_sum, lhdc_decode_temp);
        //    return LHDCBT_DEC_FUNC_FAIL;
        //}

        ptr_offset += lhdc_frame_Info.frame_len;
        dec_sum += lhdc_out_len;

        frame_num--;
	}

    *pcmBytes = (uint32_t) dec_sum;

    //ALOGD("[WL50] %s: end (dec_sum=%d pcmBytes=%d)", __func__, (int)dec_sum, (int)*pcmBytes);
    return LHDCBT_DEC_FUNC_SUCCEED;
}


// description
//   de-initialize (free) all resources allocated by LHDC v4 decoder
// Parameter
//   none
// return:
//   == 0: succceed
int lhdcBT_dec_deinit_decoder(void)
{
	ALOGD("[WL50] %s: enter", __func__);
    lhdcDestroy();

    return LHDCBT_DEC_FUNC_SUCCEED;
}

// description
//   check number of frames in one packet and return pointer to first byte of 1st frame in current packet
// Parameter
//   input: pointer to input buffer
//   input_len: length (bytes) of input buffer pointed by input
//   pLout: pointer to pointer to output buffer
//   pLlen: length (bytes) of encoded stream in output buffer
// return:
//   > 0: number of frames in current packet
//   == 0: No frames in current packet
//   < 0: error
static int assemble_lhdc_packet(uint8_t *input, uint32_t input_len, uint8_t **pLout, uint32_t *pLlen, int upd_seq_no)
{
    uint8_t hdr = 0, seqno = 0xff;
    int ret = LHDCBT_DEC_FUNC_FAIL;
    uint32_t status = 0;
	uint32_t lhdc_total_frame_nb = 0;


    if ((input == NULL) || 
		(pLout == NULL) || 
		(pLlen == NULL)) 
	{

		return LHDCBT_DEC_FUNC_FAIL;
	}
	
	if (input_len < 2)
	{
		return LHDCBT_DEC_FUNC_FAIL;
	}

    hdr = (*input);
    input++;
    seqno = (*input);
    input++;
    input_len -= 2;

    //Check latency and update value when changed.
    status = hdr & A2DP_LHDC_HDR_LATENCY_MASK;

    //Get number of frame in packet.
    status = (hdr & A2DP_LHDC_HDR_FRAME_NO_MASK) >> 2;

    ALOGD("[WL50] %s: enter", __func__);

    if (status <= 0)
    {
        ALOGD("%s: No any frame in packet.", __func__);
        return 0;
    }


    lhdc_total_frame_nb = status;

    if (seqno != serial_no)
    {
        ALOGD("%s: Packet lost! now(%d), expect(%d)", __func__, seqno, serial_no);
        //serial_no = seqno;
		//return LHDCBT_DEC_FUNC_INVALID_SEQ_NO;
    }
	
	if (upd_seq_no == LHDCBT_DEC_UPD_SEQ_NO)
	{
        serial_no = seqno + 1;
	}

    // log average bit rate
    //sav_lhdc_log_bytes_len(input_len);

    *pLlen = input_len;
    *pLout = input;

    ret = (int) lhdc_total_frame_nb;

    ALOGD("[WL50] %s: end frame number (%d)", __func__, ret);
    return ret;
}

