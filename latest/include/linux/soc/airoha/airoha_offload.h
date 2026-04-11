#ifndef __AIROHA_OFFLOAD_H
#define __AIROHA_OFFLOAD_H

#include <linux/types.h>
#include <linux/gfp_types.h>

struct airoha_ppe_dev;
struct airoha_npu;

struct airoha_npu_tx_dma_desc {
	__le32 buf0;
	__le32 ctrl;
	__le32 info1;
	__le32 info2;
};

struct airoha_npu_rx_dma_desc {
	__le32 buf0;
	__le32 ctrl;
	__le32 info1;
	__le32 info2;
};

enum airoha_npu_wlan_set_cmd {
	AIROHA_NPU_WLAN_SET_CMD_NONE = 0,
};

enum airoha_npu_wlan_get_cmd {
	AIROHA_NPU_WLAN_GET_CMD_NONE = 0,
};

static inline int airoha_npu_wlan_send_msg(struct airoha_npu *npu,
	int ifindex, enum airoha_npu_wlan_set_cmd cmd,
	void *val, int len, gfp_t gfp)
{
	return -EOPNOTSUPP;
}

static inline int airoha_npu_wlan_get_msg(struct airoha_npu *npu,
	int ifindex, enum airoha_npu_wlan_get_cmd cmd,
	void *val, int len, gfp_t gfp)
{
	return -EOPNOTSUPP;
}

static inline void airoha_npu_wlan_enable_irq(struct airoha_npu *npu, int q) {}

#endif
