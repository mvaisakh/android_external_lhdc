

#ifndef _LHDCBT_H_
#define _LHDCBT_H_
#ifdef __cplusplus
extern "C" {
#endif
#ifndef LHDCBT_API
#define LHDCBT_API
#endif /* LHDCBT_API  */
#include "lhdc_api.h"

typedef enum _lhdcBT_ext_func_field_t{
  LHDCBT_EXT_FUNC_AR = 0,
  LHDCBT_EXT_FUNC_LARC,
  LHDCBT_EXT_FUNC_EXTR,
  LHDCBT_EXT_FUNC_JAS,
  //LHDCBT_EXT_FUNC_META,
  LHDCBT_EXT_FUNC_MAX,
} lhdcBT_ext_func_field_t;

//
// Extend API Basic Settings Definition
//
#define LHDC_EXTEND_FUNC_CODE_A2DP_TYPE_MASK            (0x0A)
#define LHDC_EXTEND_FUNC_CODE_LIB_TYPE_MASK             (0x0C)
#define EXTEND_FUNC_CODE_MIN_BUFFER_LEN                 8

/* **********************************************
 *  API: Version Control
 * ***********************************************/
#define EXTEND_FUNC_GENERIC_VERSION_NUMBER_V1               ((unsigned int) 0x01000000)
#define EXTEND_FUNC_GENERIC_VERSION_NUMBER_V2               ((unsigned int) 0x02000000)

#define EXTEND_FUNC_MIN_BUFFER_LEN_GET_API_VERSION_V1       8
#define EXTEND_FUNC_MIN_BUFFER_LEN_GET_API_VERSION_V2       16

/* **********************************************
 *  LIB API: Config Type
 * ***********************************************/
/* * * * * * * * * * * * * * * *
 *   META                      *
 * * * * * * * * * * * * * * * */
//   META :: API Code
#define EXTEND_FUNC_CODE_SET_CONFIG_META        ((unsigned int) 0x0C000001)
#define EXTEND_FUNC_CODE_GET_CONFIG_META        ((unsigned int) 0x0C010001)
//   META :: API Version
#define EXTEND_FUNC_VER_SET_CONFIG_META_V1      ((unsigned int) 0x01000000)
#define EXTEND_FUNC_VER_SET_CONFIG_META_V2      ((unsigned int) 0x02000000)
#define EXTEND_FUNC_VER_GET_CONFIG_META_V1      ((unsigned int) 0x01000000)
#define EXTEND_FUNC_VER_GET_CONFIG_META_V2      ((unsigned int) 0x02000000)
//   META :: API Min Required Buffer Size
#define EXTEND_FUNC_VER_SET_CONFIG_META_REQ_BUFSIZE_V2      16  //TBD
#define EXTEND_FUNC_VER_GET_CONFIG_META_REQ_BUFSIZE_V2      16  //TBD

/* * * * * * * * * * * * * * * *
 *   AR                        *
 * * * * * * * * * * * * * * * */
//   AR :: API Code
#define EXTEND_FUNC_CODE_SET_CONFIG_AR          ((unsigned int) 0x0C000002)
#define EXTEND_FUNC_CODE_GET_CONFIG_AR          ((unsigned int) 0x0C010002)
//   AR :: API Version
#define EXTEND_FUNC_VER_SET_CONFIG_AR_V1        ((unsigned int) 0x01000000)
#define EXTEND_FUNC_VER_SET_CONFIG_AR_V2        ((unsigned int) 0x02000000)
#define EXTEND_FUNC_VER_SET_CONFIG_AR_V3        ((unsigned int) 0x03000000)
#define EXTEND_FUNC_VER_GET_CONFIG_AR_V1        ((unsigned int) 0x01000000)
#define EXTEND_FUNC_VER_GET_CONFIG_AR_V2        ((unsigned int) 0x02000000)
//   AR :: API Min Required Buffer Size
#define EXTEND_FUNC_VER_SET_CONFIG_AR_REQ_BUFSIZE_V2      16    //TBD
#define EXTEND_FUNC_VER_GET_CONFIG_AR_REQ_BUFSIZE_V2      16    //TBD


/* **********************************************
 *  LIB API: Data Type
 * ***********************************************/
/* * * * * * * * * * * * * * * *
 *   GYRO2D                    *
 * * * * * * * * * * * * * * * */
//   GYRO2D :: API Code
#define EXTEND_FUNC_CODE_SET_DATA_GYRO2D        ((unsigned int) 0x0D000001)
//   GYRO2D :: API Version
#define EXTEND_FUNC_VER_SET_DATA_GYRO2D_V1      ((unsigned int) 0x01000000)
#define EXTEND_FUNC_VER_SET_DATA_GYRO2D_V2      ((unsigned int) 0x02000000)
//   GYRO2D :: API Min Required Buffer Size
#define EXTEND_FUNC_VER_SET_DATA_GYRO2D_REQ_BUFSIZE_V2      16    //TBD (8 + 2gyro + 6pad)


/* **********************************************
 *  A2DP-Extended API:
 * ***********************************************/
/* * * * * * * * * * * * * * * *
 *   GetA2DPSpecifis           *
 * * * * * * * * * * * * * * * */
//   GetA2DPSpecifis :: API Code
#define EXTEND_FUNC_CODE_GET_SPECIFIC           ((unsigned int) 0x0A010001)
//   GetA2DPSpecifis :: API Version
#define EXTEND_FUNC_VER_GET_SPECIFIC_V1         ((unsigned int) 0x01000000)
#define EXTEND_FUNC_VER_GET_SPECIFIC_V2         ((unsigned int) 0x02000000)

/* ************************************************************************
 * Version format info of EXTEND_FUNC_CODE_GET_SPECIFIC
 * EXTEND_FUNC_VER_GET_SPECIFIC_V1:  Total Size: 41 bytes
   * API Version:                   (4 bytes)
   * API Code:                      (4 bytes)
   * A2DP Codec Config Code:        (1 bytes)
   * A2dp Specific1:                (8 bytes)
   * A2dp Specific2:                (8 bytes)
   * A2dp Specific3:                (8 bytes)
   * A2dp Specific4:                (8 bytes)
 * EXTEND_FUNC_VER_GET_SPECIFIC_V2:  Total Size: 64 bytes
   * API Version:                   (4 bytes)
   * API Code:                      (4 bytes)
   * A2DP Codec Config Code:        (1 bytes)
   * Reserved:                      (7 bytes)
   * A2dp Specific1:                (8 bytes)
   * A2dp Specific2:                (8 bytes)
   * A2dp Specific3:                (8 bytes)
   * A2dp Specific4:                (8 bytes)
   * Capabilities Metadata sub fields:  (7*2 bytes)
     * sub[0~1]:    JAS
     * sub[2~3]:    AR
     * sub[4~5]:    META
     * sub[6~7]:    LLAC
     * sub[8~9]:    MBR
     * sub[10~11]:  LARC
     * sub[12~13]:  LHDCV4
   * Padded:                        (2 bytes)
 * ************************************************************************/
#define LHDC_EXTEND_FUNC_CONFIG_API_VERSION_SIZE        4       /* API version */
#define LHDC_EXTEND_FUNC_CONFIG_API_CODE_SIZE           4       /* API index code */
#define LHDC_EXTEND_FUNC_CONFIG_A2DPCFG_CODE_SIZE       1       /* A2DP codec config code */
#define LHDC_EXTEND_FUNC_CONFIG_RESERVED_V2             7       /* V2 Reserved bytes */
#define LHDC_EXTEND_FUNC_CONFIG_SPECIFIC1_SIZE          8       /* Specific 1 */
#define LHDC_EXTEND_FUNC_CONFIG_SPECIFIC2_SIZE          8       /* Specific 2 */
#define LHDC_EXTEND_FUNC_CONFIG_SPECIFIC3_SIZE          8       /* Specific 3 */
#define LHDC_EXTEND_FUNC_CONFIG_SPECIFIC4_SIZE          8       /* Specific 4 */
/* Capabilities metadata fields(2 bytes for each tuple) */
#define LHDC_EXTEND_FUNC_CONFIG_CAPMETA_SIZE_V2         (7<<1)  /* V2 Capabilities */
#define LHDC_EXTEND_FUNC_CONFIG_PADDED_SIZE_V2          2       /* V2 Padded Fields */

/* Total size of buffer */
#define LHDC_EXTEND_FUNC_CONFIG_TOTAL_FIXED_SIZE_V1    (LHDC_EXTEND_FUNC_CONFIG_API_VERSION_SIZE + \
                                                          LHDC_EXTEND_FUNC_CONFIG_API_CODE_SIZE + \
                                                          LHDC_EXTEND_FUNC_CONFIG_A2DPCFG_CODE_SIZE + \
                                                          LHDC_EXTEND_FUNC_CONFIG_SPECIFIC1_SIZE + \
                                                          LHDC_EXTEND_FUNC_CONFIG_SPECIFIC2_SIZE + \
                                                          LHDC_EXTEND_FUNC_CONFIG_SPECIFIC3_SIZE + \
                                                          LHDC_EXTEND_FUNC_CONFIG_SPECIFIC4_SIZE)
#define LHDC_EXTEND_FUNC_CONFIG_TOTAL_FIXED_SIZE_V2    (LHDC_EXTEND_FUNC_CONFIG_API_VERSION_SIZE + \
                                                          LHDC_EXTEND_FUNC_CONFIG_API_CODE_SIZE + \
                                                          LHDC_EXTEND_FUNC_CONFIG_A2DPCFG_CODE_SIZE + \
                                                          LHDC_EXTEND_FUNC_CONFIG_RESERVED_V2 + \
                                                          LHDC_EXTEND_FUNC_CONFIG_SPECIFIC1_SIZE + \
                                                          LHDC_EXTEND_FUNC_CONFIG_SPECIFIC2_SIZE + \
                                                          LHDC_EXTEND_FUNC_CONFIG_SPECIFIC3_SIZE + \
                                                          LHDC_EXTEND_FUNC_CONFIG_SPECIFIC4_SIZE + \
                                                          LHDC_EXTEND_FUNC_CONFIG_CAPMETA_SIZE_V2 + \
                                                          LHDC_EXTEND_FUNC_CONFIG_PADDED_SIZE_V2)
/* Head of each field */
#define LHDC_EXTEND_FUNC_CONFIG_API_VERSION_HEAD        (0)
#define LHDC_EXTEND_FUNC_CONFIG_API_CODE_HEAD           (LHDC_EXTEND_FUNC_CONFIG_API_VERSION_HEAD + 4)  //4
#define LHDC_EXTEND_FUNC_CONFIG_A2DPCFG_CODE_HEAD       (LHDC_EXTEND_FUNC_CONFIG_API_CODE_HEAD + 4)     //8
/* Following part in V1 */
#define LHDC_EXTEND_FUNC_A2DP_SPECIFICS1_HEAD_V1        (LHDC_EXTEND_FUNC_CONFIG_A2DPCFG_CODE_HEAD + 1) //9~16
#define LHDC_EXTEND_FUNC_A2DP_SPECIFICS2_HEAD_V1        (LHDC_EXTEND_FUNC_A2DP_SPECIFICS1_HEAD_V1 + 8)  //17~24
#define LHDC_EXTEND_FUNC_A2DP_SPECIFICS3_HEAD_V1        (LHDC_EXTEND_FUNC_A2DP_SPECIFICS2_HEAD_V1 + 8)  //25~32
#define LHDC_EXTEND_FUNC_A2DP_SPECIFICS4_HEAD_V1        (LHDC_EXTEND_FUNC_A2DP_SPECIFICS3_HEAD_V1 + 8)  //33~40
/* Following part in V2 */
#define LHDC_EXTEND_FUNC_A2DP_RESERVED_HEAD_V2          (LHDC_EXTEND_FUNC_CONFIG_A2DPCFG_CODE_HEAD + 1) //9~15
#define LHDC_EXTEND_FUNC_A2DP_SPECIFICS1_HEAD_V2        (LHDC_EXTEND_FUNC_CONFIG_A2DPCFG_CODE_HEAD + 8) //16~23
#define LHDC_EXTEND_FUNC_A2DP_SPECIFICS2_HEAD_V2        (LHDC_EXTEND_FUNC_A2DP_SPECIFICS1_HEAD_V2 + 8)  //24~31
#define LHDC_EXTEND_FUNC_A2DP_SPECIFICS3_HEAD_V2        (LHDC_EXTEND_FUNC_A2DP_SPECIFICS2_HEAD_V2 + 8)  //32~39
#define LHDC_EXTEND_FUNC_A2DP_SPECIFICS4_HEAD_V2        (LHDC_EXTEND_FUNC_A2DP_SPECIFICS3_HEAD_V2 + 8)  //40~47
#define LHDC_EXTEND_FUNC_A2DP_CAPMETA_HEAD_V2           (LHDC_EXTEND_FUNC_A2DP_SPECIFICS4_HEAD_V2 + 8)  //48~61
#define LHDC_EXTEND_FUNC_A2DP_PADDED_HEAD_V2            (LHDC_EXTEND_FUNC_A2DP_CAPMETA_HEAD_V2 + LHDC_EXTEND_FUNC_CONFIG_CAPMETA_SIZE_V2)   //62~63

/* code definition mapping to A2DP codec specific in a2dp_codec_api.h
 * 0x01: codec_config_
 * 0x02: codec_capability_
 * 0x03: codec_local_capability_
 * 0x04: codec_selectable_capability_
 * 0x05: codec_user_config_
 * 0x06: codec_audio_config_
 */
#define LHDC_EXTEND_FUNC_A2DP_TYPE_SPECIFICS_FINAL_CFG          (0x01)
#define LHDC_EXTEND_FUNC_A2DP_TYPE_SPECIFICS_FINAL_CAP          (0x02)
#define LHDC_EXTEND_FUNC_A2DP_TYPE_SPECIFICS_LOCAL_CAP          (0x03)
#define LHDC_EXTEND_FUNC_A2DP_TYPE_SPECIFICS_SELECTABLE_CAP     (0x04)
#define LHDC_EXTEND_FUNC_A2DP_TYPE_SPECIFICS_USER_CFG           (0x05)
#define LHDC_EXTEND_FUNC_A2DP_TYPE_SPECIFICS_AUDIO_CFG          (0x06)

/************************
 * Capability Meta Format: (denotes where source capabilities bits are stored in specifics)
   * Capability Code:                   (1 byte)
   * Saving Position Info:              (1 byte)
 ************************/
/* Capabilities's code: */
#define LHDC_EXTEND_FUNC_A2DP_LHDC_JAS_CODE      (0x01)
#define LHDC_EXTEND_FUNC_A2DP_LHDC_AR_CODE       (0x02)
#define LHDC_EXTEND_FUNC_A2DP_LHDC_META_CODE     (0x03)
#define LHDC_EXTEND_FUNC_A2DP_LHDC_LLAC_CODE     (0x04)
#define LHDC_EXTEND_FUNC_A2DP_LHDC_MBR_CODE      (0x05)
#define LHDC_EXTEND_FUNC_A2DP_LHDC_LARC_CODE     (0x06)
#define LHDC_EXTEND_FUNC_A2DP_LHDC_V4_CODE       (0x07)

/* Capabilities's saving position Info:
 *  1. in which specific                        (represented in leftmost 2-bits)
 *  2. at which bit position of the specific    (represented in rightmost 6-bits)
 * */
#define LHDC_EXTEND_FUNC_A2DP_SPECIFIC1_INDEX    (0x00)     //2-bit:00
#define LHDC_EXTEND_FUNC_A2DP_SPECIFIC2_INDEX    (0x40)     //2-bit:01
#define LHDC_EXTEND_FUNC_A2DP_SPECIFIC3_INDEX    (0x80)     //2-bit:10
#define LHDC_EXTEND_FUNC_A2DP_SPECIFIC4_INDEX    (0xC0)     //2-bit:11
/********************************************************************************/




#define EXTEND_FUNC_RET_OK                      ((int) 0)
#define EXTEND_FUNC_RET_INVALID_HANDLE          ((int) 0xE01)
#define EXTEND_FUNC_RET_INVALID_PARAMETER       ((int) 0xE02)
#define EXTEND_FUNC_RET_FUNC_NOT_SUPPORT        ((int) 0xE03)
#define EXTEND_FUNC_RET_VERSION_NOT_SUPPORT     ((int) 0xE04)
#define EXTEND_FUNC_RET_BUF_UNDERRUN            ((int) 0xE05)
#define EXTEND_FUNC_RET_ERROR                   ((int) 0xE06)
#define EXTEND_FUNC_RET_NOT_READY               ((int) 0xE07)
#define EXTEND_FUNC_RET_REQ_BUFSIZE_NOT_MATCH   ((int) 0xE08)


//
// META
//
#define META_ST_VER_V1                           ((unsigned int) 0x00010000)
#define META_ST_VER_V2                           ((unsigned int) 0x00020000)
#pragma pack (push)
#pragma pack (1)
typedef struct __ST_LHDC_SET_META {

    unsigned char     header[8];
    unsigned int      meta_ver;
    unsigned char     meta_mem_size;
    unsigned char     meta_enable;
    unsigned char     meta_set;
    unsigned char     meta_metadata_length;
//  unsigned char     meta_metadata[64];

} ST_LHDC_SET_META, *PST_LHDC_SET_META;
#pragma pack(pop)

#pragma pack (push)
#pragma pack (1)
typedef struct __ST_LHDC_GET_META {

    unsigned char     header[8];
    unsigned int      meta_ver;
    unsigned char     meta_mem_size;
    unsigned char     meta_st;     // [0] 1: reserved bit, [1] 1: in meta mode/0: not in meta mode
    unsigned char     jas_status;

} ST_LHDC_GET_META, *PST_LHDC_GET_META;
#pragma pack(pop)

//
// AR
//

#pragma pack (push)
#pragma pack (1)
typedef struct __ST_LHDC_AR
{
    unsigned char     header[8];
    unsigned int      ver;
    unsigned int      size;
    unsigned int      app_ar_enabled;
    int Ch1_Pos;
    int Ch2_Pos;
    int Ch3_Pos;
    int Ch4_Pos;
    int Ch5_Pos;
    int Ch6_Pos;
    float Ch1_L_PreGain;
    float Ch1_R_PreGain;
    float Ch2_L_PreGain;
    float Ch2_R_PreGain;
    float Ch3_L_PreGain;
    float Ch3_R_PreGain;
    float Ch4_L_PreGain;
    float Ch4_R_PreGain;
    float Ch5_L_PreGain;
    float Ch5_R_PreGain;
    float Ch6_L_PreGain;
    float Ch6_R_PreGain;
    float Ch1_PostGain;
    float Ch2_PostGain;
    float Ch3_PostGain;
    float Ch4_PostGain;
    float Ch5_PostGain;
    float Ch6_PostGain;
    float Dry_Val;
    float Wet_Val;
    float Dis_1;
    float Dis_2;
    float Dis_3;
    float Dis_4;
    float Dis_5;
    float Rev_1;
    float Rev_2;
    float Rev_3;
    float Rev_4;
    float Rev_5;
    float Rev_gain;
    float ThreeD_gain;
} ST_LHDC_AR, *PST_LHDC_AR;
#pragma pack(pop)

#pragma pack (push)
#pragma pack (1)
typedef struct __ST_LHDC_AR_GYRO
{
    unsigned char     header[8];
    int world_coordinate_x;
    int world_coordinate_y;
    int world_coordinate_z;

} ST_LHDC_AR_GYRO, *PST_LHDC_AR_GYRO;
#pragma pack(pop)

#ifdef NEW_API_SET
//for NEW API used!!!!
typedef struct {
  uint32_t sample_rate;
  LHDCBT_SMPL_FMT_T bits_per_sample;
  LHDCBT_QUALITY_T audio_quality;
  bool channel_split_enabled;
  uint32_t packet_mtu;
  uint32_t encode_interval;

  bool output_size_cal;
} lhdc_init_param_t;

int lhdcBT_init(HANDLE_LHDC_BT handle, lhdc_init_param_t * param);
#else
HANDLE_LHDC_BT lhdcBT_get_handle(int version);


void lhdcBT_free_handle(HANDLE_LHDC_BT handle);

//static const char* LHDC_ENCODE_NAME = "lhdcBT_encode";
//Encoder for V2
int lhdcBT_encode(HANDLE_LHDC_BT hLhdcParam, void* p_pcm, unsigned char* p_stream);
//Encoder for V3
int lhdcBT_encodeV3(HANDLE_LHDC_BT hLhdcParam, void* p_pcm, unsigned char* out_put, uint32_t * written, uint32_t * out_fraems);

int lhdcBT_get_bitrate(HANDLE_LHDC_BT hLhdcParam);

int lhdcBT_set_bitrate(HANDLE_LHDC_BT handle, int bitrate_inx);

//int lhdcBT_get_sampling_freq(HANDLE_LHDC_BT handle);

//int lhdcBT_init_handle_encode(HANDLE_LHDC_BT handle,int sampling_freq, int bitPerSample, int bitrate_inx, int dualChannel, int need_padding, int mtu, int interval);

//int lhdcBT_get_error_code(HANDLE_LHDC_BT handle);

int lhdcBT_adjust_bitrate(HANDLE_LHDC_BT handle, size_t queueLength) ;

//void lhdcBT_setLimitBitRate(HANDLE_LHDC_BT handle, int max_rate_index);

//uint8_t lhdcBT_getSupportedVersion(HANDLE_LHDC_BT handle);

int     lhdcBT_get_block_Size(HANDLE_LHDC_BT handle);

int lhdcBT_set_ext_func_state(HANDLE_LHDC_BT handle, lhdcBT_ext_func_field_t field, bool enabled, void * priv, int priv_data_len);

int lhdcBT_get_ext_func_state(HANDLE_LHDC_BT handle, lhdcBT_ext_func_field_t field, bool * enabled);

//
// Extra API
//
// 1. API -- Set User Config (Extend)
int lhdcBT_set_user_exconfig(HANDLE_LHDC_BT handle, char* userConfig, int clen);
// 2. API -- Get User Config (Extend)
int lhdcBT_get_user_exconfig(HANDLE_LHDC_BT handle, char* userConfig, int clen);
// 3. API -- Set User Data (Extend)
void lhdcBT_set_user_exdata(HANDLE_LHDC_BT handle, char* userConfig, int clen);
// 4. API -- Get Version 
int lhdcBT_get_user_exApiver(HANDLE_LHDC_BT handle, char *version, int clen);

#endif
#ifdef __cplusplus
}
#endif
#endif /* _LHDCBT_H_ */
