// SPDX-License-Identifier: ISC
/* Copyright (C) 2021 MediaTek Inc. */

#include "mt7902.h"
#include "mcu.h"

int mt7902e_driver_own(struct mt792x_dev *dev)
{
	u32 reg = mt7902_reg_map_l1(dev, MT_TOP_LPCR_HOST_BAND0);

	mt76_wr(dev, reg, MT_TOP_LPCR_HOST_DRV_OWN);
	if (!mt76_poll_msec(dev, reg, MT_TOP_LPCR_HOST_FW_OWN,
			    0, 500)) {
		dev_err(dev->mt76.dev, "Timeout for driver own\n");
		return -EIO;
	}

	return 0;
}

static int
mt7902_mcu_send_message(struct mt76_dev *mdev, struct sk_buff *skb,
			int cmd, int *seq)
{
	struct mt792x_dev *dev = container_of(mdev, struct mt792x_dev, mt76);
	enum mt76_mcuq_id txq = MT_MCUQ_WM;
	int ret;

	//printk(KERN_INFO "mt7902_mcu_send_message - txq: 0x%x, cmd: 0x%x, seq: 0x%x, skb: 0x%x", txq, cmd, seq, skb->len);
	ret = mt76_connac2_mcu_fill_message(mdev, skb, cmd, seq);
	if (ret)
		return ret;

	mdev->mcu.timeout = 3 * HZ;

	if (cmd == MCU_CMD(FW_SCATTER))
		txq = MT_MCUQ_FWDL;

	//printk(KERN_INFO "mt7902_mcu_send_message - txq: 0x%x, cmd: 0x%x, seq: 0x%x", txq, cmd, seq);
	return mt76_tx_queue_skb_raw(dev, mdev->q_mcu[txq], skb, 0);
}

int mt7902e_mcu_init(struct mt792x_dev *dev)
{
	static const struct mt76_mcu_ops mt7902_mcu_ops = {
		.headroom = sizeof(struct mt76_connac2_mcu_txd),
		.mcu_skb_send_msg = mt7902_mcu_send_message,
		.mcu_parse_response = mt7902_mcu_parse_response,
	};
	int err;

	dev->mt76.mcu_ops = &mt7902_mcu_ops;

	err = mt7902e_driver_own(dev);
	if (err)
		return err;

	mt76_rmw_field(dev, MT_PCIE_MAC_PM, MT_PCIE_MAC_PM_L0S_DIS, 1);

	err = mt7902_run_firmware(dev);

	mt76_queue_tx_cleanup(dev, dev->mt76.q_mcu[MT_MCUQ_FWDL], false);

	return err;
}
