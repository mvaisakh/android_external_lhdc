

#ifndef _LHDCBT_EXT_FUNC_H_
#define _LHDCBT_EXT_FUNC_H_
#ifdef __cplusplus
extern "C" {
#endif


#define EXTEND_FUNC_RET_OK                      ((int) 0)
#define EXTEND_FUNC_RET_INVALID_HANDLE          ((int) 0xE01)
#define EXTEND_FUNC_RET_INVALID_PARAMETER       ((int) 0xE02)
#define EXTEND_FUNC_RET_FUNC_NOT_SUPPORT        ((int) 0xE03)
#define EXTEND_FUNC_RET_VERSION_NOT_SUPPORT     ((int) 0xE04)
#define EXTEND_FUNC_RET_BUF_UNDERRUN            ((int) 0xE05)
#define EXTEND_FUNC_RET_ERROR                   ((int) 0xE06)
#define EXTEND_FUNC_RET_NOT_READY               ((int) 0xE07)
#define EXTEND_FUNC_RET_REQ_BUFSIZE_NOT_MATCH   ((int) 0xE08)


/* **********************************************
 *  API: Version Control
 * ***********************************************/
#define EXTEND_FUNC_GENERIC_VERSION_NUMBER_V1               ((unsigned int) 0x01000000)
#define EXTEND_FUNC_GENERIC_VERSION_NUMBER_V2               ((unsigned int) 0x02000000)
#define EXTEND_FUNC_MIN_BUFFER_LEN                          (8)

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


/* * * * * * * * * * * * * * * *
 *   GetA2DPSpecifis           *
 * * * * * * * * * * * * * * * */
//   GetA2DPSpecifis :: API Code
#define EXTEND_FUNC_CODE_GET_SPECIFIC           ((unsigned int) 0x0A010001)
//   GetA2DPSpecifis :: API Version
#define EXTEND_FUNC_VER_GET_SPECIFIC_V2         ((unsigned int) 0x02000000)

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


#ifdef __cplusplus
}
#endif
#endif /* _LHDCBT_EXT_FUNC_H_ */
