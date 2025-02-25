// SPDX-License-Identifier: ISC
/* Copyright (C) 2020 MediaTek Inc. */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/pci.h>

#include "besra.h"
#include "mac.h"
#include "../trace.h"

static const struct __base mt7902_reg_base[] = {
	[WF_AGG_BASE]	= { {0x820e2000, 0x820f2000, 0x830e2000} },
	[WF_MIB_BASE]	= { {0x820ed000, 0x820fd000, 0x830ed000} },
	[WF_TMAC_BASE]	= { {0x820e4000, 0x820f4000, 0x830e4000} },
	[WF_RMAC_BASE]	= { {0x820e5000, 0x820f5000, 0x830e5000} },
	[WF_ARB_BASE]	= { {0x820e3000, 0x820f3000, 0x830e3000} },
	[WF_LPON_BASE]	= { {0x820eb000, 0x820fb000, 0x830eb000} },
	[WF_ETBF_BASE]	= { {0x820ea000, 0x820fa000, 0x830ea000} },
	[WF_DMA_BASE]	= { {0x820e7000, 0x820f7000, 0x830e7000} },
};

static const struct __map mt7902_reg_map[] = {
	{0x54000000, 0x02000, 0x1000}, /* WFDMA_0 (PCIE0 MCU DMA0) */
	{0x55000000, 0x03000, 0x1000}, /* WFDMA_1 (PCIE0 MCU DMA1) */
	{0x56000000, 0x04000, 0x1000}, /* WFDMA reserved */
	{0x57000000, 0x05000, 0x1000}, /* WFDMA MCU wrap CR */
	{0x58000000, 0x06000, 0x1000}, /* WFDMA PCIE1 MCU DMA0 (MEM_DMA) */
	{0x59000000, 0x07000, 0x1000}, /* WFDMA PCIE1 MCU DMA1 */
	{0x820c0000, 0x08000, 0x4000}, /* WF_UMAC_TOP (PLE) */
	{0x820c8000, 0x0c000, 0x2000}, /* WF_UMAC_TOP (PSE) */
	{0x820cc000, 0x0e000, 0x2000}, /* WF_UMAC_TOP (PP) */
	{0x74030000, 0x10000, 0x1000}, /* PCIe MAC */
	{0x820e0000, 0x20000, 0x0400}, /* WF_LMAC_TOP BN0 (WF_CFG) */
	{0x820e1000, 0x20400, 0x0200}, /* WF_LMAC_TOP BN0 (WF_TRB) */
	{0x820e2000, 0x20800, 0x0400}, /* WF_LMAC_TOP BN0 (WF_AGG) */
	{0x820e3000, 0x20c00, 0x0400}, /* WF_LMAC_TOP BN0 (WF_ARB) */
	{0x820e4000, 0x21000, 0x0400}, /* WF_LMAC_TOP BN0 (WF_TMAC) */
	{0x820e5000, 0x21400, 0x0800}, /* WF_LMAC_TOP BN0 (WF_RMAC) */
	{0x820ce000, 0x21c00, 0x0200}, /* WF_LMAC_TOP (WF_SEC) */
	{0x820e7000, 0x21e00, 0x0200}, /* WF_LMAC_TOP BN0 (WF_DMA) */
	{0x820cf000, 0x22000, 0x1000}, /* WF_LMAC_TOP (WF_PF) */
	{0x820e9000, 0x23400, 0x0200}, /* WF_LMAC_TOP BN0 (WF_WTBLOFF) */
	{0x820ea000, 0x24000, 0x0200}, /* WF_LMAC_TOP BN0 (WF_ETBF) */
	{0x820eb000, 0x24200, 0x0400}, /* WF_LMAC_TOP BN0 (WF_LPON) */
	{0x820ec000, 0x24600, 0x0200}, /* WF_LMAC_TOP BN0 (WF_INT) */
	{0x820ed000, 0x24800, 0x0800}, /* WF_LMAC_TOP BN0 (WF_MIB) */
	{0x820ca000, 0x26000, 0x2000}, /* WF_LMAC_TOP BN0 (WF_MUCOP) */
	{0x820d0000, 0x30000, 0x10000}, /* WF_LMAC_TOP (WF_WTBLON) */
	{0x40000000, 0x70000, 0x10000}, /* WF_UMAC_SYSRAM */
	{0x00400000, 0x80000, 0x10000}, /* WF_MCU_SYSRAM */
	{0x00410000, 0x90000, 0x10000}, /* WF_MCU_SYSRAM (configure register) */
	{0x820f0000, 0xa0000, 0x0400}, /* WF_LMAC_TOP BN1 (WF_CFG) */
	{0x820f1000, 0xa0600, 0x0200}, /* WF_LMAC_TOP BN1 (WF_TRB) */
	{0x820f2000, 0xa0800, 0x0400}, /* WF_LMAC_TOP BN1 (WF_AGG) */
	{0x820f3000, 0xa0c00, 0x0400}, /* WF_LMAC_TOP BN1 (WF_ARB) */
	{0x820f4000, 0xa1000, 0x0400}, /* WF_LMAC_TOP BN1 (WF_TMAC) */
	{0x820f5000, 0xa1400, 0x0800}, /* WF_LMAC_TOP BN1 (WF_RMAC) */
	{0x820f7000, 0xa1e00, 0x0200}, /* WF_LMAC_TOP BN1 (WF_DMA) */
	{0x820f9000, 0xa3400, 0x0200}, /* WF_LMAC_TOP BN1 (WF_WTBLOFF) */
	{0x820fa000, 0xa4000, 0x0200}, /* WF_LMAC_TOP BN1 (WF_ETBF) */
	{0x820fb000, 0xa4200, 0x0400}, /* WF_LMAC_TOP BN1 (WF_LPON) */
	{0x820fc000, 0xa4600, 0x0200}, /* WF_LMAC_TOP BN1 (WF_INT) */
	{0x820fd000, 0xa4800, 0x0800}, /* WF_LMAC_TOP BN1 (WF_MIB) */
	{0x820cc000, 0xa5000, 0x2000}, /* WF_LMAC_TOP BN1 (WF_MUCOP) */
	{0x820c4000, 0xa8000, 0x4000}, /* WF_LMAC_TOP BN1 (WF_MUCOP) */
	{0x820b0000, 0xae000, 0x1000}, /* [APB2] WFSYS_ON */
	{0x80020000, 0xb0000, 0x10000}, /* WF_TOP_MISC_OFF */
	{0x81020000, 0xc0000, 0x10000}, /* WF_TOP_MISC_ON */
	{0x7c020000, 0xd0000, 0x10000}, /* CONN_INFRA, wfdma */
	{0x7c060000, 0xe0000, 0x10000}, /* CONN_INFRA, conn_host_csr_top */
	{0x7c000000, 0xf0000, 0x10000}, /* CONN_INFRA */
	{0x0, 0x0, 0x0}, /* imply end of search */
};

enum {
	BESRA_REG_RR = 0x0,
	BESRA_REG_WR,
	BESRA_REG_RMW,
	__BESRA_REG_OPS_MAX,
};

static u32 besra_reg_wr(struct besra_dev *dev, u32 addr, u32 mask, u32 val, u8 wr)
{
	u32 ret = 0;

	switch (wr) {
	case BESRA_REG_RR:
		ret = dev->bus_ops->rr(&dev->mt76, addr);
		break;
	case BESRA_REG_WR:
		dev->bus_ops->wr(&dev->mt76, addr, val);
		break;
	case BESRA_REG_RMW:
		ret = dev->bus_ops->rmw(&dev->mt76, addr, mask, val);
		break;
	default:
		break;
	}

	return ret;
}

static u32 besra_reg_map_l1(struct besra_dev *dev, u32 addr, u32 mask, u32 val, u8 wr)
{
	u32 offset = FIELD_GET(MT_HIF_REMAP_L1_OFFSET, addr);
	u32 base = FIELD_GET(MT_HIF_REMAP_L1_BASE, addr);
	u32 backup, ret = 0;

	backup = dev->bus_ops->rr(&dev->mt76, MT_HIF_REMAP_L1);
	dev->bus_ops->rmw(&dev->mt76, MT_HIF_REMAP_L1,
			  MT_HIF_REMAP_L1_MASK,
			  FIELD_PREP(MT_HIF_REMAP_L1_MASK, base));
	/* use read to push write */
	dev->bus_ops->rr(&dev->mt76, MT_HIF_REMAP_L1);

	ret = besra_reg_wr(dev, MT_HIF_REMAP_BASE_L1 + offset, mask, val, wr);

	/* remap to ori status */
	dev->bus_ops->wr(&dev->mt76, MT_HIF_REMAP_L1, backup);

	return ret;
}

static u32 besra_reg_map_l2(struct besra_dev *dev, u32 addr, u32 mask, u32 val, u8 wr)
{
	u32 offset = FIELD_GET(MT_HIF_REMAP_L2_OFFSET, addr);
	u32 base = FIELD_GET(MT_HIF_REMAP_L2_BASE, addr);
	u32 backup, ret = 0;

	backup = dev->bus_ops->rr(&dev->mt76, MT_HIF_REMAP_L2);
	dev->bus_ops->rmw(&dev->mt76, MT_HIF_REMAP_L2,
			  MT_HIF_REMAP_L2_MASK,
			  FIELD_PREP(MT_HIF_REMAP_L2_MASK, base));

	/* use read to push write */
	dev->bus_ops->rr(&dev->mt76, MT_HIF_REMAP_L2);

	ret = besra_reg_wr(dev, MT_HIF_REMAP_BASE_L2 + offset, mask, val, wr);

	/* remap to ori status */
	dev->bus_ops->wr(&dev->mt76, MT_HIF_REMAP_L2, backup);

	return ret;
}

static u32 __besra_reg_addr(struct besra_dev *dev, u32 addr, u32 mask, u32 val, u8 wr)
{
	int i;

	if (addr < 0x100000)
		return besra_reg_wr(dev, addr, mask, val, wr);

	for (i = 0; i < dev->reg.map_size; i++) {
		u32 ofs;

		if (addr < dev->reg.map[i].phys)
			continue;

		ofs = addr - dev->reg.map[i].phys;
		if (ofs > dev->reg.map[i].size)
			continue;

		addr = dev->reg.map[i].mapped + ofs;

		return besra_reg_wr(dev, addr, mask, val, wr);
	}

	if ((addr >= MT_INFRA_BASE && addr < MT_WFSYS0_PHY_START) ||
	    (addr >= MT_WFSYS0_PHY_START && addr < MT_WFSYS1_PHY_START) ||
	    (addr >= MT_WFSYS1_PHY_START && addr <= MT_WFSYS1_PHY_END))
		return besra_reg_map_l1(dev, addr, mask, val, wr);

	if (dev_is_pci(dev->mt76.dev) &&
	    ((addr >= MT_CBTOP1_PHY_START && addr <= MT_CBTOP1_PHY_END) ||
	     (addr >= MT_CBTOP2_PHY_START && addr <= MT_CBTOP2_PHY_END)))
		return besra_reg_map_l1(dev, addr, mask, val, wr);

	/* CONN_INFRA: covert to phyiscal addr and use layer 1 remap */
	if (addr >= MT_INFRA_MCU_START && addr <= MT_INFRA_MCU_END) {
		addr = addr - MT_INFRA_MCU_START + MT_INFRA_BASE;
		return besra_reg_map_l1(dev, addr, mask, val, wr);
	}

	return besra_reg_map_l2(dev, addr, mask, val, wr);
}

static u32 besra_rr(struct mt76_dev *mdev, u32 offset)
{
	struct besra_dev *dev = container_of(mdev, struct besra_dev, mt76);

	return __besra_reg_addr(dev, offset, 0, 0, BESRA_REG_RR);
}

static void besra_wr(struct mt76_dev *mdev, u32 offset, u32 val)
{
	struct besra_dev *dev = container_of(mdev, struct besra_dev, mt76);

	__besra_reg_addr(dev, offset, 0, val, BESRA_REG_WR);
}

static u32 besra_rmw(struct mt76_dev *mdev, u32 offset, u32 mask, u32 val)
{
	struct besra_dev *dev = container_of(mdev, struct besra_dev, mt76);

	return __besra_reg_addr(dev, offset, mask, val, BESRA_REG_RMW);
}

static int besra_mmio_init(struct mt76_dev *mdev,
			    void __iomem *mem_base,
			    u32 device_id)
{
	struct mt76_bus_ops *bus_ops;
	struct besra_dev *dev;

	dev = container_of(mdev, struct besra_dev, mt76);
	mt76_mmio_init(&dev->mt76, mem_base);

	switch (device_id) {
	case 0x7902:
		dev->reg.base = mt7902_reg_base;
		dev->reg.map = mt7902_reg_map;
		dev->reg.map_size = ARRAY_SIZE(mt7902_reg_map);
		break;
	default:
		return -EINVAL;
	}

	dev->bus_ops = dev->mt76.bus;
	bus_ops = devm_kmemdup(dev->mt76.dev, dev->bus_ops, sizeof(*bus_ops),
			       GFP_KERNEL);
	if (!bus_ops)
		return -ENOMEM;

	bus_ops->rr = besra_rr;
	bus_ops->wr = besra_wr;
	bus_ops->rmw = besra_rmw;
	dev->mt76.bus = bus_ops;

	mdev->rev = (device_id << 16) |
		    (mt76_rr(dev, MT_HW_REV) & 0xff);
	dev_dbg(mdev->dev, "ASIC revision: %04x\n", mdev->rev);

	return 0;
}

void besra_dual_hif_set_irq_mask(struct besra_dev *dev,
				  bool write_reg,
				  u32 clear, u32 set)
{
	struct mt76_dev *mdev = &dev->mt76;
	unsigned long flags;

	spin_lock_irqsave(&mdev->mmio.irq_lock, flags);

	mdev->mmio.irqmask &= ~clear;
	mdev->mmio.irqmask |= set;

	if (write_reg) {
		mt76_wr(dev, MT_INT_MASK_CSR, mdev->mmio.irqmask);
		mt76_wr(dev, MT_INT1_MASK_CSR, mdev->mmio.irqmask);
	}

	spin_unlock_irqrestore(&mdev->mmio.irq_lock, flags);
}

static void besra_rx_poll_complete(struct mt76_dev *mdev,
				    enum mt76_rxq_id q)
{
	struct besra_dev *dev = container_of(mdev, struct besra_dev, mt76);

	besra_irq_enable(dev, MT_INT_RX(q));
}

/* TODO: support 2/4/6/8 MSI-X vectors */
static void besra_irq_tasklet(struct tasklet_struct *t)
{
	struct besra_dev *dev = from_tasklet(dev, t, irq_tasklet);
	u32 intr, intr1, mask;

	mt76_wr(dev, MT_INT_MASK_CSR, 0);
	if (dev->hif2)
		mt76_wr(dev, MT_INT1_MASK_CSR, 0);

	intr = mt76_rr(dev, MT_INT_SOURCE_CSR);
	intr &= dev->mt76.mmio.irqmask;
	mt76_wr(dev, MT_INT_SOURCE_CSR, intr);

	if (dev->hif2) {
		intr1 = mt76_rr(dev, MT_INT1_SOURCE_CSR);
		intr1 &= dev->mt76.mmio.irqmask;
		mt76_wr(dev, MT_INT1_SOURCE_CSR, intr1);

		intr |= intr1;
	}

	trace_dev_irq(&dev->mt76, intr, dev->mt76.mmio.irqmask);

	mask = intr & MT_INT_RX_DONE_ALL;
	if (intr & MT_INT_TX_DONE_MCU)
		mask |= MT_INT_TX_DONE_MCU;

	besra_irq_disable(dev, mask);

	if (intr & MT_INT_TX_DONE_MCU)
		napi_schedule(&dev->mt76.tx_napi);

	if (intr & MT_INT_RX(MT_RXQ_MAIN) && MT_RXQ_VALID(MT_RXQ_MAIN))
		napi_schedule(&dev->mt76.napi[MT_RXQ_MAIN]);

	if (intr & MT_INT_RX(MT_RXQ_EXT) && MT_RXQ_VALID(MT_RXQ_EXT))
		napi_schedule(&dev->mt76.napi[MT_RXQ_EXT]);

	if (intr & MT_INT_RX(MT_RXQ_MCU) && MT_RXQ_VALID(MT_RXQ_MCU))
		napi_schedule(&dev->mt76.napi[MT_RXQ_MCU]);

	if (intr & MT_INT_RX(MT_RXQ_MCU_WA) && MT_RXQ_VALID(MT_RXQ_MCU_WA))
		napi_schedule(&dev->mt76.napi[MT_RXQ_MCU_WA]);

	if (intr & MT_INT_RX(MT_RXQ_MAIN_WA) && MT_RXQ_VALID(MT_RXQ_MAIN_WA))
		napi_schedule(&dev->mt76.napi[MT_RXQ_MAIN_WA]);

	if (intr & MT_INT_RX(MT_RXQ_EXT_WA) && MT_RXQ_VALID(MT_RXQ_EXT_WA))
		napi_schedule(&dev->mt76.napi[MT_RXQ_EXT_WA]);

	if (intr & MT_INT_RX(MT_RXQ_TRI) && MT_RXQ_VALID(MT_RXQ_TRI))
		napi_schedule(&dev->mt76.napi[MT_RXQ_TRI]);

	if (intr & MT_INT_RX(MT_RXQ_TRI_WA) && MT_RXQ_VALID(MT_RXQ_TRI_WA))
		napi_schedule(&dev->mt76.napi[MT_RXQ_TRI_WA]);

	if (intr & MT_INT_MCU_CMD) {
		u32 val = mt76_rr(dev, MT_MCU_CMD);

		mt76_wr(dev, MT_MCU_CMD, val);
		if (val & MT_MCU_CMD_ERROR_MASK) {
			dev->reset_state = val;
			ieee80211_queue_work(mt76_hw(dev), &dev->reset_work);
			wake_up(&dev->reset_wait);
		}
	}
}

irqreturn_t besra_irq_handler(int irq, void *dev_instance)
{
	struct besra_dev *dev = dev_instance;

	mt76_wr(dev, MT_INT_MASK_CSR, 0);
	if (dev->hif2)
		mt76_wr(dev, MT_INT1_MASK_CSR, 0);

	if (!test_bit(MT76_STATE_INITIALIZED, &dev->mphy.state))
		return IRQ_NONE;

	tasklet_schedule(&dev->irq_tasklet);

	return IRQ_HANDLED;
}

struct besra_dev *besra_mmio_probe(struct device *pdev,
				     void __iomem *mem_base, u32 device_id)
{
	static const struct mt76_driver_ops drv_ops = {
		/* txwi_size = txd size + txp size */
		.txwi_size = MT_TXD_SIZE + sizeof(struct besra_txp),
		.drv_flags = MT_DRV_TXWI_NO_FREE | MT_DRV_HW_MGMT_TXQ,
		.survey_flags = SURVEY_INFO_TIME_TX |
				SURVEY_INFO_TIME_RX |
				SURVEY_INFO_TIME_BSS_RX,
		.token_size = BESRA_TOKEN_SIZE,
		.tx_prepare_skb = besra_tx_prepare_skb,
		.tx_complete_skb = besra_tx_complete_skb,
		.rx_skb = besra_queue_rx_skb,
		.rx_check = besra_rx_check,
		.rx_poll_complete = besra_rx_poll_complete,
		.sta_ps = besra_sta_ps,
		.sta_add = besra_mac_sta_add,
		.sta_remove = besra_mac_sta_remove,
		.update_survey = besra_update_channel,
	};
	struct ieee80211_ops *ops;
	struct besra_dev *dev;
	struct mt76_dev *mdev;
	int ret;

	ops = devm_kmemdup(pdev, &besra_ops, sizeof(besra_ops), GFP_KERNEL);
	if (!ops)
		return ERR_PTR(-ENOMEM);

	mdev = mt76_alloc_device(pdev, sizeof(*dev), ops, &drv_ops);
	if (!mdev)
		return ERR_PTR(-ENOMEM);

	dev = container_of(mdev, struct besra_dev, mt76);

	ret = besra_mmio_init(mdev, mem_base, device_id);
	if (ret)
		goto error;

	tasklet_setup(&dev->irq_tasklet, besra_irq_tasklet);

	mt76_wr(dev, MT_INT_MASK_CSR, 0);

	return dev;

error:
	mt76_free_device(&dev->mt76);

	return ERR_PTR(ret);
}

static int __init besra_init(void)
{
	int ret;

	ret = pci_register_driver(&besra_hif_driver);
	if (ret)
		return ret;

	ret = pci_register_driver(&besra_pci_driver);
	if (ret)
		pci_unregister_driver(&besra_hif_driver);

	return ret;
}

static void __exit besra_exit(void)
{
	pci_unregister_driver(&besra_pci_driver);
	pci_unregister_driver(&besra_hif_driver);
}

module_init(besra_init);
module_exit(besra_exit);
MODULE_LICENSE("Dual BSD/GPL");
