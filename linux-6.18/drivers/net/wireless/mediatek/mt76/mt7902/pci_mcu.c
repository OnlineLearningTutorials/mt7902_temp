// SPDX-License-Identifier: ISC
/* Copyright (C) 2021 MediaTek Inc. */

#include "mt7902.h"
#include "mcu.h"

#define DEBUG


int mt7902e_driver_own(struct mt792x_dev *dev)
{
	printk(KERN_DEBUG "pci_mcu.c - mt7902e_driver_own(dev)");
	u32 reg = mt7902_reg_map_l1(dev, MT_TOP_LPCR_HOST_BAND0);
	u32 val;

	// 1. Log the target register address (helps verify the L1 mapping)
	dev_info(dev->mt76.dev, "Requesting driver own at register 0x%x\n", reg);

	// 2. Log current state before writing
	val = mt76_rr(dev, reg);
	dev_info(dev->mt76.dev, "LPCR state before write: 0x%08x\n", val);


	mt76_wr(dev, reg, MT_TOP_LPCR_HOST_DRV_OWN);
	if (!mt76_poll_msec(dev, reg, MT_TOP_LPCR_HOST_FW_OWN,
			    0, 500)) {
		val = mt76_rr(dev, reg);
		dev_err(dev->mt76.dev, "Timeout for driver own! Register 0x%x = 0x%08x (Expected FW_OWN bit to clear)\n", 
			reg, val);
		
		/* Check if the bus is even alive - if 0xFFFFFFFF, the PCIe link is down */
		if (val == 0xffffffff)
			dev_err(dev->mt76.dev, "PCIe bus appears to be down (Read 0xFFFFFFFF)\n");
			
		dev_err(dev->mt76.dev, "Timeout for driver own\n");
		return -EIO;
	}

	dev_info(dev->mt76.dev, "Driver ownership acquired successfully\n");
	return 0;
}

static int
mt7902_mcu_send_message(struct mt76_dev *mdev, struct sk_buff *skb,
			int cmd, int *seq)
{
	//printk(KERN_DEBUG "pci_mcu.c - mt7902_mcu_send_message(mdev, skb, cmd: %d, seq: %d)", cmd, seq);
	struct mt792x_dev *dev = container_of(mdev, struct mt792x_dev, mt76);
	enum mt76_mcuq_id txq = MT_MCUQ_WM;
	int ret;

	// 1. Log the Command and Sequence (Use dev_dbg to avoid flooding)
	dev_info(mdev->dev, "MCU send msg: cmd=0x%x seq=%d len=%d\n", 
		cmd, (seq ? *seq : -1), skb->len);


	ret = mt76_connac2_mcu_fill_message(mdev, skb, cmd, seq);
	if (ret) {
		dev_err(mdev->dev, "Failed to fill MCU message (cmd 0x%x): %d\n", cmd, ret);
		return ret;
	}

	mdev->mcu.timeout = 3 * HZ;

	if (cmd == MCU_CMD(FW_SCATTER)) {
		txq = MT_MCUQ_FWDL;
		dev_info(mdev->dev, "Using FWDL queue for scatter command\n");
	}

	// 2. Track the actual transmission status
	ret = mt76_tx_queue_skb_raw(dev, mdev->q_mcu[txq], skb, 0);
	if (ret) {
		dev_err(mdev->dev, "MCU tx failed (cmd 0x%x, q %d): %d\n", cmd, txq, ret);
		
		/* Check if queue is full */
		if (ret == -EBUSY)
			dev_err(mdev->dev, "MCU Queue %d is full/stuck!\n", txq);
	}

	return ret;

	//return mt76_tx_queue_skb_raw(dev, mdev->q_mcu[txq], skb, 0);
}

int mt7902e_mcu_init(struct mt792x_dev *dev)
{
	printk(KERN_DEBUG "pci_mcu.c - mt7902e_mcu_init(dev)");
	static const struct mt76_mcu_ops mt7902_mcu_ops = {
		.headroom = sizeof(struct mt76_connac2_mcu_txd),
		.mcu_skb_send_msg = mt7902_mcu_send_message,
		.mcu_parse_response = mt7902_mcu_parse_response,
	};
	int err;

	dev->mt76.mcu_ops = &mt7902_mcu_ops;

	dev_info(dev->mt76.dev, "Requesting Driver Ownership (LP Control)\n");
	err = mt7902e_driver_own(dev);
	if (err) {
		dev_err(dev->mt76.dev, "Failed to get driver ownership: %d\n", err);
		return err;
	}

	dev_info(dev->mt76.dev, "Disabling PCIe L0s power state\n");
	mt76_rmw_field(dev, MT_PCIE_MAC_PM, MT_PCIE_MAC_PM_L0S_DIS, 1);

	dev_info(dev->mt76.dev, "Loading and starting MT7902 firmware...\n");
	err = mt7902_run_firmware(dev);
	if (err) {
		dev_err(dev->mt76.dev, "Firmware run failed: %d (Check /lib/firmware/)\n", err);
		return err;
	}
	dev_info(dev->mt76.dev, "Firmware is running successfully\n");

	dev_info(dev->mt76.dev, "Cleaning up FWDL (Firmware Download) queue\n");
	mt76_queue_tx_cleanup(dev, dev->mt76.q_mcu[MT_MCUQ_FWDL], false);

	return err;
}
