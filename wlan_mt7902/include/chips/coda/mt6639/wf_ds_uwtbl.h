/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2021 MediaTek Inc.
 */

#ifndef __WF_UWTBL_REGS_H__
#define __WF_UWTBL_REGS_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* DW0 */
#define WF_UWTBL_PEER_MLD_ADDRESS_47_32__DW         0
#define WF_UWTBL_PEER_MLD_ADDRESS_47_32__ADDR       0
#define WF_UWTBL_PEER_MLD_ADDRESS_47_32__MASK       0x0000ffff /* 15- 0 */
#define WF_UWTBL_PEER_MLD_ADDRESS_47_32__SHIFT      0
#define WF_UWTBL_OWN_MLD_ID_DW                      0
#define WF_UWTBL_OWN_MLD_ID_ADDR                    0
#define WF_UWTBL_OWN_MLD_ID_MASK                    0x003f0000 /* 21-16 */
#define WF_UWTBL_OWN_MLD_ID_SHIFT                   16
/* DW1 */
#define WF_UWTBL_PEER_MLD_ADDRESS_31_0__DW          1
#define WF_UWTBL_PEER_MLD_ADDRESS_31_0__ADDR        4
#define WF_UWTBL_PEER_MLD_ADDRESS_31_0__MASK        0xffffffff /* 31- 0 */
#define WF_UWTBL_PEER_MLD_ADDRESS_31_0__SHIFT       0
/* DW2 */
#define WF_UWTBL_PN_31_0__DW                        2
#define WF_UWTBL_PN_31_0__ADDR                      8
#define WF_UWTBL_PN_31_0__MASK                      0xffffffff /* 31- 0 */
#define WF_UWTBL_PN_31_0__SHIFT                     0
/* DW3 */
#define WF_UWTBL_PN_47_32__DW                       3
#define WF_UWTBL_PN_47_32__ADDR                     12
#define WF_UWTBL_PN_47_32__MASK                     0x0000ffff /* 15- 0 */
#define WF_UWTBL_PN_47_32__SHIFT                    0
#define WF_UWTBL_COM_SN_DW                          3
#define WF_UWTBL_COM_SN_ADDR                        12
#define WF_UWTBL_COM_SN_MASK                        0x0fff0000 /* 27-16 */
#define WF_UWTBL_COM_SN_SHIFT                       16
/* DW4 */
#define WF_UWTBL_TID0_SN_DW                         4
#define WF_UWTBL_TID0_SN_ADDR                       16
#define WF_UWTBL_TID0_SN_MASK                       0x00000fff /* 11- 0 */
#define WF_UWTBL_TID0_SN_SHIFT                      0
#define WF_UWTBL_RX_BIPN_31_0__DW                   4
#define WF_UWTBL_RX_BIPN_31_0__ADDR                 16
#define WF_UWTBL_RX_BIPN_31_0__MASK                 0xffffffff /* 31- 0 */
#define WF_UWTBL_RX_BIPN_31_0__SHIFT                0
#define WF_UWTBL_TID1_SN_DW                         4
#define WF_UWTBL_TID1_SN_ADDR                       16
#define WF_UWTBL_TID1_SN_MASK                       0x00fff000 /* 23-12 */
#define WF_UWTBL_TID1_SN_SHIFT                      12
#define WF_UWTBL_TID2_SN_7_0__DW                    4
#define WF_UWTBL_TID2_SN_7_0__ADDR                  16
#define WF_UWTBL_TID2_SN_7_0__MASK                  0xff000000 /* 31-24 */
#define WF_UWTBL_TID2_SN_7_0__SHIFT                 24
/* DW5 */
#define WF_UWTBL_TID2_SN_11_8__DW                   5
#define WF_UWTBL_TID2_SN_11_8__ADDR                 20
#define WF_UWTBL_TID2_SN_11_8__MASK                 0x0000000f /* 3- 0 */
#define WF_UWTBL_TID2_SN_11_8__SHIFT                0
#define WF_UWTBL_RX_BIPN_47_32__DW                  5
#define WF_UWTBL_RX_BIPN_47_32__ADDR                20
#define WF_UWTBL_RX_BIPN_47_32__MASK                0x0000ffff /* 15- 0 */
#define WF_UWTBL_RX_BIPN_47_32__SHIFT               0
#define WF_UWTBL_TID3_SN_DW                         5
#define WF_UWTBL_TID3_SN_ADDR                       20
#define WF_UWTBL_TID3_SN_MASK                       0x0000fff0 /* 15- 4 */
#define WF_UWTBL_TID3_SN_SHIFT                      4
#define WF_UWTBL_TID4_SN_DW                         5
#define WF_UWTBL_TID4_SN_ADDR                       20
#define WF_UWTBL_TID4_SN_MASK                       0x0fff0000 /* 27-16 */
#define WF_UWTBL_TID4_SN_SHIFT                      16
#define WF_UWTBL_TID5_SN_3_0__DW                    5
#define WF_UWTBL_TID5_SN_3_0__ADDR                  20
#define WF_UWTBL_TID5_SN_3_0__MASK                  0xf0000000 /* 31-28 */
#define WF_UWTBL_TID5_SN_3_0__SHIFT                 28
/* DW6 */
#define WF_UWTBL_TID5_SN_11_4__DW                   6
#define WF_UWTBL_TID5_SN_11_4__ADDR                 24
#define WF_UWTBL_TID5_SN_11_4__MASK                 0x000000ff /* 7- 0 */
#define WF_UWTBL_TID5_SN_11_4__SHIFT                0
#define WF_UWTBL_KEY_LOC2_DW                        6
#define WF_UWTBL_KEY_LOC2_ADDR                      24
#define WF_UWTBL_KEY_LOC2_MASK                      0x00001fff /* 12- 0 */
#define WF_UWTBL_KEY_LOC2_SHIFT                     0
#define WF_UWTBL_TID6_SN_DW                         6
#define WF_UWTBL_TID6_SN_ADDR                       24
#define WF_UWTBL_TID6_SN_MASK                       0x000fff00 /* 19- 8 */
#define WF_UWTBL_TID6_SN_SHIFT                      8
#define WF_UWTBL_TID7_SN_DW                         6
#define WF_UWTBL_TID7_SN_ADDR                       24
#define WF_UWTBL_TID7_SN_MASK                       0xfff00000 /* 31-20 */
#define WF_UWTBL_TID7_SN_SHIFT                      20
/* DW7 */
#define WF_UWTBL_KEY_LOC0_DW                        7
#define WF_UWTBL_KEY_LOC0_ADDR                      28
#define WF_UWTBL_KEY_LOC0_MASK                      0x00001fff /* 12- 0 */
#define WF_UWTBL_KEY_LOC0_SHIFT                     0
#define WF_UWTBL_KEY_LOC1_DW                        7
#define WF_UWTBL_KEY_LOC1_ADDR                      28
#define WF_UWTBL_KEY_LOC1_MASK                      0x1fff0000 /* 28-16 */
#define WF_UWTBL_KEY_LOC1_SHIFT                     16
/* DW8 */
#define WF_UWTBL_AMSDU_CFG_DW                       8
#define WF_UWTBL_AMSDU_CFG_ADDR                     32
#define WF_UWTBL_AMSDU_CFG_MASK                     0x00000fff /* 11- 0 */
#define WF_UWTBL_AMSDU_CFG_SHIFT                    0
#define WF_UWTBL_WMM_Q_DW                           8
#define WF_UWTBL_WMM_Q_ADDR                         32
#define WF_UWTBL_WMM_Q_MASK                         0x06000000 /* 26-25 */
#define WF_UWTBL_WMM_Q_SHIFT                        25
#define WF_UWTBL_QOS_DW                             8
#define WF_UWTBL_QOS_ADDR                           32
#define WF_UWTBL_QOS_MASK                           0x08000000 /* 27-27 */
#define WF_UWTBL_QOS_SHIFT                          27
#define WF_UWTBL_HT_DW                              8
#define WF_UWTBL_HT_ADDR                            32
#define WF_UWTBL_HT_MASK                            0x10000000 /* 28-28 */
#define WF_UWTBL_HT_SHIFT                           28
#define WF_UWTBL_HDRT_MODE_DW                       8
#define WF_UWTBL_HDRT_MODE_ADDR                     32
#define WF_UWTBL_HDRT_MODE_MASK                     0x20000000 /* 29-29 */
#define WF_UWTBL_HDRT_MODE_SHIFT                    29
/* DW9 */
#define WF_UWTBL_RELATED_IDX0_DW                    9
#define WF_UWTBL_RELATED_IDX0_ADDR                  36
#define WF_UWTBL_RELATED_IDX0_MASK                  0x00000fff /* 11- 0 */
#define WF_UWTBL_RELATED_IDX0_SHIFT                 0
#define WF_UWTBL_RELATED_BAND0_DW                   9
#define WF_UWTBL_RELATED_BAND0_ADDR                 36
#define WF_UWTBL_RELATED_BAND0_MASK                 0x00003000 /* 13-12 */
#define WF_UWTBL_RELATED_BAND0_SHIFT                12
#define WF_UWTBL_PRIMARY_MLD_BAND_DW                9
#define WF_UWTBL_PRIMARY_MLD_BAND_ADDR              36
#define WF_UWTBL_PRIMARY_MLD_BAND_MASK              0x0000c000 /* 15-14 */
#define WF_UWTBL_PRIMARY_MLD_BAND_SHIFT             14
#define WF_UWTBL_RELATED_IDX1_DW                    9
#define WF_UWTBL_RELATED_IDX1_ADDR                  36
#define WF_UWTBL_RELATED_IDX1_MASK                  0x0fff0000 /* 27-16 */
#define WF_UWTBL_RELATED_IDX1_SHIFT                 16
#define WF_UWTBL_RELATED_BAND1_DW                   9
#define WF_UWTBL_RELATED_BAND1_ADDR                 36
#define WF_UWTBL_RELATED_BAND1_MASK                 0x30000000 /* 29-28 */
#define WF_UWTBL_RELATED_BAND1_SHIFT                28
#define WF_UWTBL_SECONDARY_MLD_BAND_DW              9
#define WF_UWTBL_SECONDARY_MLD_BAND_ADDR            36
#define WF_UWTBL_SECONDARY_MLD_BAND_MASK            0xc0000000 /* 31-30 */
#define WF_UWTBL_SECONDARY_MLD_BAND_SHIFT           30

/* DW0 */
#define WF_UWTBL_GET_PEER_MLD_ADDRESS_47_32_(reg32) \
	READ_FIELD((reg32), WF_UWTBL_PEER_MLD_ADDRESS_47_32_)
#define WF_UWTBL_GET_OWN_MLD_ID(reg32) \
	READ_FIELD((reg32), WF_UWTBL_OWN_MLD_ID)
/* DW1 */
#define WF_UWTBL_GET_PEER_MLD_ADDRESS_31_0_(reg32) \
	READ_FIELD((reg32), WF_UWTBL_PEER_MLD_ADDRESS_31_0_)
/* DW2 */
#define WF_UWTBL_GET_PN_31_0_(reg32) \
	READ_FIELD((reg32), WF_UWTBL_PN_31_0_)
/* DW3 */
#define WF_UWTBL_GET_PN_47_32_(reg32) \
	READ_FIELD((reg32), WF_UWTBL_PN_47_32_)
#define WF_UWTBL_GET_COM_SN(reg32) \
	READ_FIELD((reg32), WF_UWTBL_COM_SN)
/* DW4 */
#define WF_UWTBL_GET_TID0_SN(reg32) \
	READ_FIELD((reg32), WF_UWTBL_TID0_SN)
#define WF_UWTBL_GET_RX_BIPN_31_0_(reg32) \
	READ_FIELD((reg32), WF_UWTBL_RX_BIPN_31_0_)
#define WF_UWTBL_GET_TID1_SN(reg32) \
	READ_FIELD((reg32), WF_UWTBL_TID1_SN)
#define WF_UWTBL_GET_TID2_SN_7_0_(reg32) \
	READ_FIELD((reg32), WF_UWTBL_TID2_SN_7_0_)
/* DW5 */
#define WF_UWTBL_GET_TID2_SN_11_8_(reg32) \
	READ_FIELD((reg32), WF_UWTBL_TID2_SN_11_8_)
#define WF_UWTBL_GET_RX_BIPN_47_32_(reg32) \
	READ_FIELD((reg32), WF_UWTBL_RX_BIPN_47_32_)
#define WF_UWTBL_GET_TID3_SN(reg32) \
	READ_FIELD((reg32), WF_UWTBL_TID3_SN)
#define WF_UWTBL_GET_TID4_SN(reg32) \
	READ_FIELD((reg32), WF_UWTBL_TID4_SN)
#define WF_UWTBL_GET_TID5_SN_3_0_(reg32) \
	READ_FIELD((reg32), WF_UWTBL_TID5_SN_3_0_)
/* DW6 */
#define WF_UWTBL_GET_TID5_SN_11_4_(reg32) \
	READ_FIELD((reg32), WF_UWTBL_TID5_SN_11_4_)
#define WF_UWTBL_GET_KEY_LOC2(reg32) \
	READ_FIELD((reg32), WF_UWTBL_KEY_LOC2)
#define WF_UWTBL_GET_TID6_SN(reg32) \
	READ_FIELD((reg32), WF_UWTBL_TID6_SN)
#define WF_UWTBL_GET_TID7_SN(reg32) \
	READ_FIELD((reg32), WF_UWTBL_TID7_SN)
/* DW7 */
#define WF_UWTBL_GET_KEY_LOC0(reg32) \
	READ_FIELD((reg32), WF_UWTBL_KEY_LOC0)
#define WF_UWTBL_GET_KEY_LOC1(reg32) \
	READ_FIELD((reg32), WF_UWTBL_KEY_LOC1)
/* DW8 */
#define WF_UWTBL_GET_AMSDU_CFG(reg32) \
	READ_FIELD((reg32), WF_UWTBL_AMSDU_CFG)
#define WF_UWTBL_GET_WMM_Q(reg32) \
	READ_FIELD((reg32), WF_UWTBL_WMM_Q)
#define WF_UWTBL_GET_QOS(reg32) \
	READ_FIELD((reg32), WF_UWTBL_QOS)
#define WF_UWTBL_GET_HT(reg32) \
	READ_FIELD((reg32), WF_UWTBL_HT)
#define WF_UWTBL_GET_HDRT_MODE(reg32) \
	READ_FIELD((reg32), WF_UWTBL_HDRT_MODE)
/* DW9 */
#define WF_UWTBL_GET_RELATED_IDX0(reg32) \
	READ_FIELD((reg32), WF_UWTBL_RELATED_IDX0)
#define WF_UWTBL_GET_RELATED_BAND0(reg32) \
	READ_FIELD((reg32), WF_UWTBL_RELATED_BAND0)
#define WF_UWTBL_GET_PRIMARY_MLD_BAND(reg32) \
	READ_FIELD((reg32), WF_UWTBL_PRIMARY_MLD_BAND)
#define WF_UWTBL_GET_RELATED_IDX1(reg32) \
	READ_FIELD((reg32), WF_UWTBL_RELATED_IDX1)
#define WF_UWTBL_GET_RELATED_BAND1(reg32) \
	READ_FIELD((reg32), WF_UWTBL_RELATED_BAND1)
#define WF_UWTBL_GET_SECONDARY_MLD_BAND(reg32) \
	READ_FIELD((reg32), WF_UWTBL_SECONDARY_MLD_BAND)

/* DW0 */
#define WF_UWTBL_SET_PEER_MLD_ADDRESS_47_32_(reg32, val32) \
	WRITE_FIELD((reg32), WF_UWTBL_PEER_MLD_ADDRESS_47_32_, val32)
#define WF_UWTBL_SET_OWN_MLD_ID(reg32, val32) \
	WRITE_FIELD((reg32), WF_UWTBL_OWN_MLD_ID, val32)
/* DW1 */
#define WF_UWTBL_SET_PEER_MLD_ADDRESS_31_0_(reg32, val32) \
	WRITE_FIELD((reg32), WF_UWTBL_PEER_MLD_ADDRESS_31_0_, val32)
/* DW2 */
#define WF_UWTBL_SET_PN_31_0_(reg32, val32) \
	WRITE_FIELD((reg32), WF_UWTBL_PN_31_0_, val32)
/* DW3 */
#define WF_UWTBL_SET_PN_47_32_(reg32, val32) \
	WRITE_FIELD((reg32), WF_UWTBL_PN_47_32_, val32)
#define WF_UWTBL_SET_COM_SN(reg32, val32) \
	WRITE_FIELD((reg32), WF_UWTBL_COM_SN, val32)
/* DW4 */
#define WF_UWTBL_SET_TID0_SN(reg32, val32) \
	WRITE_FIELD((reg32), WF_UWTBL_TID0_SN, val32)
#define WF_UWTBL_SET_RX_BIPN_31_0_(reg32, val32) \
	WRITE_FIELD((reg32), WF_UWTBL_RX_BIPN_31_0_, val32)
#define WF_UWTBL_SET_TID1_SN(reg32, val32) \
	WRITE_FIELD((reg32), WF_UWTBL_TID1_SN, val32)
#define WF_UWTBL_SET_TID2_SN_7_0_(reg32, val32) \
	WRITE_FIELD((reg32), WF_UWTBL_TID2_SN_7_0_, val32)
/* DW5 */
#define WF_UWTBL_SET_TID2_SN_11_8_(reg32, val32) \
	WRITE_FIELD((reg32), WF_UWTBL_TID2_SN_11_8_, val32)
#define WF_UWTBL_SET_RX_BIPN_47_32_(reg32, val32) \
	WRITE_FIELD((reg32), WF_UWTBL_RX_BIPN_47_32_, val32)
#define WF_UWTBL_SET_TID3_SN(reg32, val32) \
	WRITE_FIELD((reg32), WF_UWTBL_TID3_SN, val32)
#define WF_UWTBL_SET_TID4_SN(reg32, val32) \
	WRITE_FIELD((reg32), WF_UWTBL_TID4_SN, val32)
#define WF_UWTBL_SET_TID5_SN_3_0_(reg32, val32) \
	WRITE_FIELD((reg32), WF_UWTBL_TID5_SN_3_0_, val32)
/* DW6 */
#define WF_UWTBL_SET_TID5_SN_11_4_(reg32, val32) \
	WRITE_FIELD((reg32), WF_UWTBL_TID5_SN_11_4_, val32)
#define WF_UWTBL_SET_KEY_LOC2(reg32, val32) \
	WRITE_FIELD((reg32), WF_UWTBL_KEY_LOC2, val32)
#define WF_UWTBL_SET_TID6_SN(reg32, val32) \
	WRITE_FIELD((reg32), WF_UWTBL_TID6_SN, val32)
#define WF_UWTBL_SET_TID7_SN(reg32, val32) \
	WRITE_FIELD((reg32), WF_UWTBL_TID7_SN, val32)
/* DW7 */
#define WF_UWTBL_SET_KEY_LOC0(reg32, val32) \
	WRITE_FIELD((reg32), WF_UWTBL_KEY_LOC0, val32)
#define WF_UWTBL_SET_KEY_LOC1(reg32, val32) \
	WRITE_FIELD((reg32), WF_UWTBL_KEY_LOC1, val32)
/* DW8 */
#define WF_UWTBL_SET_AMSDU_CFG(reg32, val32) \
	WRITE_FIELD((reg32), WF_UWTBL_AMSDU_CFG, val32)
#define WF_UWTBL_SET_WMM_Q(reg32, val32) \
	WRITE_FIELD((reg32), WF_UWTBL_WMM_Q, val32)
#define WF_UWTBL_SET_QOS(reg32, val32) \
	WRITE_FIELD((reg32), WF_UWTBL_QOS, val32)
#define WF_UWTBL_SET_HT(reg32, val32) \
	WRITE_FIELD((reg32), WF_UWTBL_HT, val32)
#define WF_UWTBL_SET_HDRT_MODE(reg32, val32) \
	WRITE_FIELD((reg32), WF_UWTBL_HDRT_MODE, val32)
/* DW9 */
#define WF_UWTBL_SET_RELATED_IDX0(reg32, val32) \
	WRITE_FIELD((reg32), WF_UWTBL_RELATED_IDX0, val32)
#define WF_UWTBL_SET_RELATED_BAND0(reg32, val32) \
	WRITE_FIELD((reg32), WF_UWTBL_RELATED_BAND0, val32)
#define WF_UWTBL_SET_PRIMARY_MLD_BAND(reg32, val32) \
	WRITE_FIELD((reg32), WF_UWTBL_PRIMARY_MLD_BAND, val32)
#define WF_UWTBL_SET_RELATED_IDX1(reg32, val32) \
	WRITE_FIELD((reg32), WF_UWTBL_RELATED_IDX1, val32)
#define WF_UWTBL_SET_RELATED_BAND1(reg32, val32) \
	WRITE_FIELD((reg32), WF_UWTBL_RELATED_BAND1, val32)
#define WF_UWTBL_SET_SECONDARY_MLD_BAND(reg32, val32) \
	WRITE_FIELD((reg32), WF_UWTBL_SECONDARY_MLD_BAND, val32)

/* DW0 */
#define WF_UWTBL_CLR_PEER_MLD_ADDRESS_47_32_(reg32) \
	CLEAR_FIELD((reg32), WF_UWTBL_PEER_MLD_ADDRESS_47_32_)
#define WF_UWTBL_CLR_OWN_MLD_ID(reg32) \
	CLEAR_FIELD((reg32), WF_UWTBL_OWN_MLD_ID)
/* DW1 */
#define WF_UWTBL_CLR_PEER_MLD_ADDRESS_31_0_(reg32) \
	CLEAR_FIELD((reg32), WF_UWTBL_PEER_MLD_ADDRESS_31_0_)
/* DW2 */
#define WF_UWTBL_CLR_PN_31_0_(reg32) \
	CLEAR_FIELD((reg32), WF_UWTBL_PN_31_0_)
/* DW3 */
#define WF_UWTBL_CLR_PN_47_32_(reg32) \
	CLEAR_FIELD((reg32), WF_UWTBL_PN_47_32_)
#define WF_UWTBL_CLR_COM_SN(reg32) \
	CLEAR_FIELD((reg32), WF_UWTBL_COM_SN)
/* DW4 */
#define WF_UWTBL_CLR_TID0_SN(reg32) \
	CLEAR_FIELD((reg32), WF_UWTBL_TID0_SN)
#define WF_UWTBL_CLR_RX_BIPN_31_0_(reg32) \
	CLEAR_FIELD((reg32), WF_UWTBL_RX_BIPN_31_0_)
#define WF_UWTBL_CLR_TID1_SN(reg32) \
	CLEAR_FIELD((reg32), WF_UWTBL_TID1_SN)
#define WF_UWTBL_CLR_TID2_SN_7_0_(reg32) \
	CLEAR_FIELD((reg32), WF_UWTBL_TID2_SN_7_0_)
/* DW5 */
#define WF_UWTBL_CLR_TID2_SN_11_8_(reg32) \
	CLEAR_FIELD((reg32), WF_UWTBL_TID2_SN_11_8_)
#define WF_UWTBL_CLR_RX_BIPN_47_32_(reg32) \
	CLEAR_FIELD((reg32), WF_UWTBL_RX_BIPN_47_32_)
#define WF_UWTBL_CLR_TID3_SN(reg32) \
	CLEAR_FIELD((reg32), WF_UWTBL_TID3_SN)
#define WF_UWTBL_CLR_TID4_SN(reg32) \
	CLEAR_FIELD((reg32), WF_UWTBL_TID4_SN)
#define WF_UWTBL_CLR_TID5_SN_3_0_(reg32) \
	CLEAR_FIELD((reg32), WF_UWTBL_TID5_SN_3_0_)
/* DW6 */
#define WF_UWTBL_CLR_TID5_SN_11_4_(reg32) \
	CLEAR_FIELD((reg32), WF_UWTBL_TID5_SN_11_4_)
#define WF_UWTBL_CLR_KEY_LOC2(reg32) \
	CLEAR_FIELD((reg32), WF_UWTBL_KEY_LOC2)
#define WF_UWTBL_CLR_TID6_SN(reg32) \
	CLEAR_FIELD((reg32), WF_UWTBL_TID6_SN)
#define WF_UWTBL_CLR_TID7_SN(reg32) \
	CLEAR_FIELD((reg32), WF_UWTBL_TID7_SN)
/* DW7 */
#define WF_UWTBL_CLR_KEY_LOC0(reg32) \
	CLEAR_FIELD((reg32), WF_UWTBL_KEY_LOC0)
#define WF_UWTBL_CLR_KEY_LOC1(reg32) \
	CLEAR_FIELD((reg32), WF_UWTBL_KEY_LOC1)
/* DW8 */
#define WF_UWTBL_CLR_AMSDU_CFG(reg32) \
	CLEAR_FIELD((reg32), WF_UWTBL_AMSDU_CFG)
#define WF_UWTBL_CLR_WMM_Q(reg32) \
	CLEAR_FIELD((reg32), WF_UWTBL_WMM_Q)
#define WF_UWTBL_CLR_QOS(reg32) \
	CLEAR_FIELD((reg32), WF_UWTBL_QOS)
#define WF_UWTBL_CLR_HT(reg32) \
	CLEAR_FIELD((reg32), WF_UWTBL_HT)
#define WF_UWTBL_CLR_HDRT_MODE(reg32) \
	CLEAR_FIELD((reg32), WF_UWTBL_HDRT_MODE)
/* DW9 */
#define WF_UWTBL_CLR_RELATED_IDX0(reg32) \
	CLEAR_FIELD((reg32), WF_UWTBL_RELATED_IDX0)
#define WF_UWTBL_CLR_RELATED_BAND0(reg32) \
	CLEAR_FIELD((reg32), WF_UWTBL_RELATED_BAND0)
#define WF_UWTBL_CLR_PRIMARY_MLD_BAND(reg32) \
	CLEAR_FIELD((reg32), WF_UWTBL_PRIMARY_MLD_BAND)
#define WF_UWTBL_CLR_RELATED_IDX1(reg32) \
	CLEAR_FIELD((reg32), WF_UWTBL_RELATED_IDX1)
#define WF_UWTBL_CLR_RELATED_BAND1(reg32) \
	CLEAR_FIELD((reg32), WF_UWTBL_RELATED_BAND1)
#define WF_UWTBL_CLR_SECONDARY_MLD_BAND(reg32) \
	CLEAR_FIELD((reg32), WF_UWTBL_SECONDARY_MLD_BAND)

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __WF_UWTBL_REGS_H__ */

