// SPDX-License-Identifier: ISC
/* Copyright (C) 2020 MediaTek Inc.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>

#include "mt7902.h"
#include "../mt76_connac2_mac.h"
#include "../dma.h"
#include "mcu.h"

static const struct pci_device_id mt7902_pci_device_table[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_MEDIATEK, 0x7902), .driver_data = (kernel_ulong_t)MT7902_FIRMWARE_WM },
	{ },
};

static bool mt7902_disable_aspm;
module_param_named(disable_aspm, mt7902_disable_aspm, bool, 0644);
MODULE_PARM_DESC(disable_aspm, "disable PCI ASPM support");

static int mt7902e_init_reset(struct mt792x_dev *dev)
{
    printk(KERN_INFO "pci.c - mt7902e_init_reset(struct mt792x_dev *dev)");
	return mt792x_wpdma_reset(dev, true);
}

static void mt7902e_unregister_device(struct mt792x_dev *dev)
{
    printk(KERN_INFO "pci.c - mt7902e_unregister_device(struct mt792x_dev *dev)");
	int i;
	struct mt76_connac_pm *pm = &dev->pm;

	cancel_work_sync(&dev->init_work);
	mt76_unregister_device(&dev->mt76);
	mt76_for_each_q_rx(&dev->mt76, i)
		napi_disable(&dev->mt76.napi[i]);
	cancel_delayed_work_sync(&pm->ps_work);
	cancel_work_sync(&pm->wake_work);
	cancel_work_sync(&dev->reset_work);

	mt76_connac2_tx_token_put(&dev->mt76);
	__mt792x_mcu_drv_pmctrl(dev);
	mt792x_dma_cleanup(dev);
	mt792x_wfsys_reset(dev);
	skb_queue_purge(&dev->mt76.mcu.res_q);

	tasklet_disable(&dev->mt76.irq_tasklet);
}

static u32 __mt7902_reg_addr(struct mt792x_dev *dev, u32 addr)
{
    //printk(KERN_INFO "pci.c - __mt7902_reg_addr - addr: %d = 0x%08x", addr, addr);
	static const struct mt76_connac_reg_map fixed_map[] = {
	    /* chip addr, bus addr, range */
	    /* phys, maps, size */
	    { 0x820d0000, 0x30000, 0x10000 }, /* WF_LMAC_TOP (WF_WTBLON) */
		{ 0x820ed000, 0x24800, 0x00800 }, /* WF_LMAC_TOP BN0 (WF_MIB) */
		{ 0x820e4000, 0x21000, 0x00400 }, /* WF_LMAC_TOP BN0 (WF_TMAC) */
		{ 0x820e7000, 0x21e00, 0x00200 }, /* WF_LMAC_TOP BN0 (WF_DMA) */
		{ 0x820eb000, 0x24200, 0x00400 }, /* WF_LMAC_TOP BN0 (WF_LPON) */
		{ 0x820e2000, 0x20800, 0x00400 }, /* WF_LMAC_TOP BN0 (WF_AGG) */
		{ 0x820e3000, 0x20c00, 0x00400 }, /* WF_LMAC_TOP BN0 (WF_ARB) */
		{ 0x820e5000, 0x21400, 0x00800 }, /* WF_LMAC_TOP BN0 (WF_RMAC) */
		{ 0x00400000, 0x80000, 0x10000 }, /* WF_MCU_SYSRAM */
		{ 0x00410000, 0x90000, 0x10000 }, /* WF_MCU_SYSRAM (configure register) */
		{ 0x40000000, 0x70000, 0x10000 }, /* WF_UMAC_SYSRAM */
		{ 0x54000000, 0x02000, 0x01000 }, /* WFDMA PCIE0 MCU DMA0 */
		{ 0x55000000, 0x03000, 0x01000 }, /* WFDMA PCIE0 MCU DMA1 */
		{ 0x58000000, 0x06000, 0x01000 }, /* WFDMA PCIE1 MCU DMA0 (MEM_DMA) */
		{ 0x59000000, 0x07000, 0x01000 }, /* WFDMA PCIE1 MCU DMA1 */
		{ 0x7c000000, 0xf0000, 0x10000 }, /* CONN_INFRA */
		{ 0x7c020000, 0xd0000, 0x10000 }, /* CONN_INFRA, WFDMA */
		{ 0x7c060000, 0xe0000, 0x10000 }, /* CONN_INFRA, conn_host_csr_top */
		{ 0x80020000, 0xb0000, 0x10000 }, /* WF_TOP_MISC_OFF */
		{ 0x81020000, 0xc0000, 0x10000 }, /* WF_TOP_MISC_ON */
		{ 0x820c0000, 0x08000, 0x04000 }, /* WF_UMAC_TOP (PLE) */
		{ 0x820c8000, 0x0c000, 0x02000 }, /* WF_UMAC_TOP (PSE) */
		{ 0x820cc000, 0x0e000, 0x01000 }, /* WF_UMAC_TOP (PP) */
		{ 0x820cd000, 0x0f000, 0x01000 }, /* WF_MDP_TOP */
		{ 0x74030000, 0x10000, 0x10000 }, /* PCIE_MAC_IREG */
		{ 0x820ce000, 0x21c00, 0x00200 }, /* WF_LMAC_TOP (WF_SEC) */
		{ 0x820cf000, 0x22000, 0x01000 }, /* WF_LMAC_TOP (WF_PF) */
		{ 0x820e0000, 0x20000, 0x00400 }, /* WF_LMAC_TOP BN0 (WF_CFG) */
		{ 0x820e1000, 0x20400, 0x00200 }, /* WF_LMAC_TOP BN0 (WF_TRB) */
		{ 0x820e9000, 0x23400, 0x00200 }, /* WF_LMAC_TOP BN0 (WF_WTBLOFF) */
		{ 0x820ea000, 0x24000, 0x00200 }, /* WF_LMAC_TOP BN0 (WF_ETBF) */
		{ 0x820ec000, 0x24600, 0x00200 }, /* WF_LMAC_TOP BN0 (WF_INT) */
		{ 0x820f0000, 0xa0000, 0x00400 }, /* WF_LMAC_TOP BN1 (WF_CFG) */
		{ 0x820f1000, 0xa0600, 0x00200 }, /* WF_LMAC_TOP BN1 (WF_TRB) */
		{ 0x820f2000, 0xa0800, 0x00400 }, /* WF_LMAC_TOP BN1 (WF_AGG) */
		{ 0x820f3000, 0xa0c00, 0x00400 }, /* WF_LMAC_TOP BN1 (WF_ARB) */
		{ 0x820f4000, 0xa1000, 0x00400 }, /* WF_LMAC_TOP BN1 (WF_TMAC) */
		{ 0x820f5000, 0xa1400, 0x00800 }, /* WF_LMAC_TOP BN1 (WF_RMAC) */
		{ 0x820f7000, 0xa1e00, 0x00200 }, /* WF_LMAC_TOP BN1 (WF_DMA) */
		{ 0x820f9000, 0xa3400, 0x00200 }, /* WF_LMAC_TOP BN1 (WF_WTBLOFF) */
		{ 0x820fa000, 0xa4000, 0x00200 }, /* WF_LMAC_TOP BN1 (WF_ETBF) */
		{ 0x820fb000, 0xa4200, 0x00400 }, /* WF_LMAC_TOP BN1 (WF_LPON) */
		{ 0x820fc000, 0xa4600, 0x00200 }, /* WF_LMAC_TOP BN1 (WF_INT) */
		{ 0x820fd000, 0xa4800, 0x00800 }, /* WF_LMAC_TOP BN1 (WF_MIB) */
	};
	int i;

	if (addr < 0x100000)
		return addr;

	for (i = 0; i < ARRAY_SIZE(fixed_map); i++) {
		u32 ofs;
    	//printk(KERN_INFO "pci.c - __mt7902_reg_addr - addr: 0x%08x, fixed_map[%d].phys: 0x%08x", addr, i, fixed_map[i].phys);
		if (addr < fixed_map[i].phys)
			continue;

		ofs = addr - fixed_map[i].phys;
    	//printk(KERN_INFO "pci.c - __mt7902_reg_addr - ofs: 0x%08x, fixed_map[%d].size: 0x%08x", ofs, i, fixed_map[i].size);
		if (ofs > fixed_map[i].size)
			continue;

    	printk(KERN_INFO "pci.c - __mt7902_reg_addr(struct mt792x_dev *dev, 0x%08x) - fixed_map[%d].phys: 0x%08x, fixed_map[%d].maps: 0x%08x, fixed_map[%d].size: 0x%08x, ofs: 0x%08x, return:0x%08x", addr, i, fixed_map[i].phys, i, fixed_map[i].maps, i, fixed_map[i].size, ofs, ofs+fixed_map[i].maps);
		return fixed_map[i].maps + ofs;
	}

    	printk(KERN_INFO "pci.c - __mt7902_reg_addr - mt7902_reg_map_l1(dev, 0x%08x)->0x%08x", addr, addr, mt7902_reg_map_l1(dev, addr));
	if ((addr >= 0x18000000 && addr < 0x18c00000) ||
	    (addr >= 0x70000000 && addr < 0x78000000) ||
	    (addr >= 0x7c000000 && addr < 0x7c400000))
		return mt7902_reg_map_l1(dev, addr);

	dev_err(dev->mt76.dev, "Access currently unsupported address %08x\n",
		addr);

	return 0;
}

static u32 mt7902_rr(struct mt76_dev *mdev, u32 offset)
{
    //printk(KERN_INFO "pci.c - mt7902_rr(struct mt76_dev *mdev, 0x%08x)", offset);
	struct mt792x_dev *dev = container_of(mdev, struct mt792x_dev, mt76);
	u32 addr = __mt7902_reg_addr(dev, offset);
    //printk(KERN_INFO "pci.c - mt7902_rr - addr: 0x%08x, offset: 0x%08x", addr, offset);
	return dev->bus_ops->rr(mdev, addr);
}

static void mt7902_wr(struct mt76_dev *mdev, u32 offset, u32 val)
{
    //printk(KERN_INFO "pci.c - mt7902_wr(struct mt76_dev *mdev, 0x%08x, 0x%08x)", offset, val);
	struct mt792x_dev *dev = container_of(mdev, struct mt792x_dev, mt76);
	u32 addr = __mt7902_reg_addr(dev, offset);
    //printk(KERN_INFO "pci.c - mt7902_wr - addr: 0x%08x, offset: 0x%08x, val: 0x%08x", addr, offset, val);
	dev->bus_ops->wr(mdev, addr, val);
}

static u32 mt7902_rmw(struct mt76_dev *mdev, u32 offset, u32 mask, u32 val)
{
    //printk(KERN_INFO "pci.c - mt7902_rmw(struct mt76_dev *mdev, 0x%08x, 0x%08x, 0x%08x)", offset, mask, val);
	struct mt792x_dev *dev = container_of(mdev, struct mt792x_dev, mt76);
	u32 addr = __mt7902_reg_addr(dev, offset);
    //printk(KERN_INFO "pci.c - mt7902_rmw - addr : 0x%08x, mask: 0x%08x, val: 0x%08x", addr, mask, val);
	return dev->bus_ops->rmw(mdev, addr, mask, val);
}

static int mt7902_dma_init(struct mt792x_dev *dev)
{
    printk(KERN_INFO "pci.c - mt7902_dma_init(struct mt792x_dev *dev)");
	int ret;

	mt76_dma_attach(&dev->mt76);

	ret = mt792x_dma_disable(dev, true);
    printk(KERN_INFO "pci.c - mt7902_dma_init - mt792x_dma_disable(dev, true)->ret: %d", ret);
	if (ret)
		return ret;

	/* init tx queue */
	ret = mt76_connac_init_tx_queues(dev->phy.mt76, MT7902_TXQ_BAND0, MT7902_TX_RING_SIZE, MT_TX_RING_BASE, NULL, 0); //mt76_connac_init_tx_queues(dev->phy.mt76, 0x0, 0x800, 0xd4300, NULL, 0)->ret: 0 
    printk(KERN_INFO "pci.c - mt7902_dma_init - mt76_connac_init_tx_queues(dev->phy.mt76, 0x%08x, 0x%08x, 0x%08x, NULL, 0)->ret: %d  (init tx queue)", MT7902_TXQ_BAND0, MT7902_TX_RING_SIZE, MT_TX_RING_BASE, ret);
	if (ret)
		return ret;

    printk(KERN_INFO "pci.c - mt7902_dma_init - mt76_wr(dev, 0x%08x, 0x4);", MT_WFDMA0_TX_RING0_EXT_CTRL);
	mt76_wr(dev, MT_WFDMA0_TX_RING0_EXT_CTRL, 0x4); //mt76_wr(dev, 0xd4600, 0x4);

	/* command to WM */
	ret = mt76_init_mcu_queue(&dev->mt76, MT_MCUQ_WM, MT7902_TXQ_MCU_WM, MT7902_TX_MCU_RING_SIZE, MT_TX_RING_BASE); //mt76_init_mcu_queue(&dev->mt76, 0x0, 0x11, 0x100, 0xd4300);->ret: 0 
    printk(KERN_INFO "pci.c - mt7902_dma_init - mt76_init_mcu_queue(&dev->mt76, 0x%08x, 0x%08x, 0x%08x, 0x%08x);->ret: %d  (command to WM)", MT_MCUQ_WM, MT7902_TXQ_MCU_WM, MT7902_TX_MCU_RING_SIZE, MT_TX_RING_BASE, ret);
	if (ret)
		return ret;

	/* firmware download */
	ret = mt76_init_mcu_queue(&dev->mt76, MT_MCUQ_FWDL, MT7902_TXQ_FWDL, MT7902_TX_FWDL_RING_SIZE, MT_TX_RING_BASE); //mt76_init_mcu_queue(&dev->mt76, 0x2, 0x10, 0x80, 0xd4300)->ret: 0 
    printk(KERN_INFO "pci.c - mt7902_dma_init - mt76_init_mcu_queue(&dev->mt76, 0x%08x, 0x%08x, 0x%08x, 0x%08x)->ret: %d  (firmware download)", MT_MCUQ_FWDL, MT7902_TXQ_FWDL, MT7902_TX_FWDL_RING_SIZE, MT_TX_RING_BASE, ret);
	if (ret)
		return ret;

	/* event from WM before firmware download */
	ret = mt76_queue_alloc(dev, &dev->mt76.q_rx[MT_RXQ_MCU], MT7902_RXQ_MCU_WM, MT7902_RX_MCU_RING_SIZE, MT_RX_BUF_SIZE, MT_RX_EVENT_RING_BASE); //mt76_queue_alloc(dev, &dev->mt76.q_rx[0x1], 0x0,  0x8, 0x800, 0xd4500)->ret: 0 
    printk(KERN_INFO "pci.c - mt7902_dma_init - mt76_queue_alloc(dev, &dev->mt76.q_rx[0x%08x], 0x%08x,  0x%08x, 0x%08x, 0x%08x)->ret: %d (event from WM before firmware download)", MT_RXQ_MCU, MT7902_RXQ_MCU_WM, MT7902_RX_MCU_RING_SIZE, MT_RX_BUF_SIZE, MT_RX_EVENT_RING_BASE, ret);
	if (ret)
		return ret;

	/* Change mcu queue after firmware download */
	ret = mt76_queue_alloc(dev, &dev->mt76.q_rx[MT_RXQ_MCU_WA], MT7902_RXQ_MCU_WM, MT7902_RX_MCU_WA_RING_SIZE, MT_RX_BUF_SIZE, MT_WFDMA0(0x540)); //mt76_queue_alloc(dev, &dev->mt76.q_rx[0x2], 0x0, 0x200, 0x800, 0xd4540)->ret: 0 
    printk(KERN_INFO "pci.c - mt7902_dma_init - mt76_queue_alloc(dev, &dev->mt76.q_rx[0x%08x], 0x%08x, 0x%08x, 0x%08x, 0x%08x)->ret: %d /* Change mcu queue after firmware download */", MT_RXQ_MCU_WA, MT7902_RXQ_MCU_WM, MT7902_RX_MCU_WA_RING_SIZE, MT_RX_BUF_SIZE, MT_WFDMA0(0x540), ret);
	if (ret)
		return ret;

	/* rx data */
	ret = mt76_queue_alloc(dev, &dev->mt76.q_rx[MT_RXQ_MAIN], MT7902_RXQ_BAND0, MT7902_RX_RING_SIZE, MT_RX_BUF_SIZE, MT_RX_DATA_RING_BASE); //mt76_queue_alloc(dev, &dev->mt76.q_rx[0x0], 0x0, 0x600, 0x800, 0xd4520)->ret: 0 
    printk(KERN_INFO "pci.c - mt7902_dma_init - mt76_queue_alloc(dev, &dev->mt76.q_rx[0x%08x], 0x%08x, 0x%08x, 0x%08x, 0x%08x)->ret: %d  /* rx data */     ", MT_RXQ_MAIN, MT7902_RXQ_BAND0, MT7902_RX_RING_SIZE, MT_RX_BUF_SIZE, MT_RX_DATA_RING_BASE, ret);
	if (ret)
		return ret;

	ret = mt76_init_queues(dev, mt792x_poll_rx);
    printk(KERN_INFO "pci.c - mt7902_dma_init - mt76_init_queues(dev, mt792x_poll_rx)->ret: %d", ret);
	if (ret < 0)
		return ret;

	netif_napi_add_tx(&dev->mt76.tx_napi_dev, &dev->mt76.tx_napi,
			  mt792x_poll_tx);
	napi_enable(&dev->mt76.tx_napi);

	return mt792x_dma_enable(dev);
}

static int mt7902_pci_probe(struct pci_dev *pdev,
			    const struct pci_device_id *id)
{
    printk(KERN_INFO "pci.c - mt7902_pci_probe starting the mt7902 driver from the pci probe function. This is the first function to display in dmesg  ============================================================================================================================================================================================");
	static const struct mt76_driver_ops drv_ops = {
		/* txwi_size = txd size + txp size */
		.txwi_size = MT_TXD_SIZE + sizeof(struct mt76_connac_hw_txp),
		.drv_flags = MT_DRV_TXWI_NO_FREE | MT_DRV_HW_MGMT_TXQ |
			     MT_DRV_AMSDU_OFFLOAD,
		.survey_flags = SURVEY_INFO_TIME_TX |
				SURVEY_INFO_TIME_RX |
				SURVEY_INFO_TIME_BSS_RX,
		.token_size = MT7902_TOKEN_SIZE,
		.tx_prepare_skb = mt7902e_tx_prepare_skb,
		.tx_complete_skb = mt76_connac_tx_complete_skb,
		.rx_check = mt7902_rx_check,
		.rx_skb = mt7902_queue_rx_skb,
		.rx_poll_complete = mt792x_rx_poll_complete,
		.sta_add = mt7902_mac_sta_add,
		.sta_assoc = mt7902_mac_sta_assoc,
		.sta_remove = mt7902_mac_sta_remove,
		.update_survey = mt792x_update_channel,
	};
	static const struct mt792x_hif_ops mt7902_pcie_ops = {
		.init_reset = mt7902e_init_reset,
		.reset = mt7902e_mac_reset,
		.mcu_init = mt7902e_mcu_init,
		.drv_own = mt792xe_mcu_drv_pmctrl,
		.fw_own = mt792xe_mcu_fw_pmctrl,
	};
	static const struct mt792x_irq_map irq_map = {
		.host_irq_enable = MT_WFDMA0_HOST_INT_ENA,
		.tx = {
			.all_complete_mask = MT_INT_TX_DONE_ALL,
			.mcu_complete_mask = MT_INT_TX_DONE_MCU,
		},
		.rx = {
			.data_complete_mask = MT_INT_RX_DONE_DATA,
			.wm_complete_mask = MT_INT_RX_DONE_WM,
			.wm2_complete_mask = MT_INT_RX_DONE_WM2,
		},
	};
	struct ieee80211_ops *ops;
	struct mt76_bus_ops *bus_ops;
	struct mt792x_dev *dev;
	struct mt76_dev *mdev;
	u8 features;
	int ret;
	u16 cmd;
	u32 pci_header;

	printk(KERN_INFO "pci.c - mt7902_pci_probe pcim_enable_device");
	ret = pcim_enable_device(pdev);         //mt7902e 0000:03:00.0: enabling device (0000 -> 0002)
    printk(KERN_INFO "pci.c - mt7902_pci_probe pcim_enable_device->ret : %d", ret);
	if (ret)
		return ret;


	//printk(KERN_INFO "pci.c - mt7902_pci_probe - pcim_iomap(bar0): 0x%08x, pcim_iomap(bar1): 0x%08x, pcim_iomap(bar2): 0x%08x", pcim_iomap(pdev, 0, 32000), pcim_iomap(pdev, 1, 32000), pcim_iomap(pdev, 2, 32000));
	ret = pcim_iomap_regions_request_all(pdev, BIT(0), pci_name(pdev));  //pcim_iomap_regions(pdev, 1, 0000:03:00.0)->ret : 0
    printk(KERN_INFO "pci.c - mt7902_pci_probe pcim_iomap_regions(pdev, %d, %s)->ret : %d", BIT(0), pci_name(pdev), ret);
	if (ret)
		return ret;
    printk(KERN_INFO "pci.c - mt7902_pci_probe - pci_read_config_word(pdev, %d, 0x%08x);", PCI_COMMAND, &cmd);
	pci_read_config_word(pdev, PCI_COMMAND, &cmd);   //pci_read_config_word(pdev, 4, 0x1158b61e);
	if (!(cmd & PCI_COMMAND_MEMORY)) {
		cmd |= PCI_COMMAND_MEMORY;
		printk(KERN_INFO "pci.c - mt7902_pci_probe - pci_write_config_word(pdev, %d, 0x%08x);", PCI_COMMAND, &cmd);
		pci_write_config_word(pdev, PCI_COMMAND, cmd);
	}
	
	pci_read_config_dword(pdev, 0, &pci_header); printk(KERN_DEBUG "pci.c - mt7902_pci_probe - pci_header - 0x%08x", pci_header);
	pci_read_config_dword(pdev, 4, &pci_header); printk(KERN_DEBUG "pci.c - mt7902_pci_probe - pci_header - 0x%08x", pci_header);
	pci_read_config_dword(pdev, 8, &pci_header); printk(KERN_DEBUG "pci.c - mt7902_pci_probe - pci_header - 0x%08x", pci_header);
	pci_read_config_dword(pdev, 12, &pci_header); printk(KERN_DEBUG "pci.c - mt7902_pci_probe - pci_header - 0x%08x", pci_header);
	pci_read_config_dword(pdev, 16, &pci_header); printk(KERN_DEBUG "pci.c - mt7902_pci_probe - pci_header - 0x%08x", pci_header);
	pci_read_config_dword(pdev, 20, &pci_header); printk(KERN_DEBUG "pci.c - mt7902_pci_probe - pci_header - 0x%08x", pci_header);
	pci_read_config_dword(pdev, 24, &pci_header); printk(KERN_DEBUG "pci.c - mt7902_pci_probe - pci_header - 0x%08x", pci_header);
	pci_read_config_dword(pdev, 28, &pci_header); printk(KERN_DEBUG "pci.c - mt7902_pci_probe - pci_header - 0x%08x", pci_header);
	pci_read_config_dword(pdev, 32, &pci_header); printk(KERN_DEBUG "pci.c - mt7902_pci_probe - pci_header - 0x%08x", pci_header);
	pci_read_config_dword(pdev, 36, &pci_header); printk(KERN_DEBUG "pci.c - mt7902_pci_probe - pci_header - 0x%08x", pci_header);
	pci_read_config_dword(pdev, 40, &pci_header); printk(KERN_DEBUG "pci.c - mt7902_pci_probe - pci_header - 0x%08x", pci_header);
	pci_read_config_dword(pdev, 44, &pci_header); printk(KERN_DEBUG "pci.c - mt7902_pci_probe - pci_header - 0x%08x", pci_header);
	pci_read_config_dword(pdev, 48, &pci_header); printk(KERN_DEBUG "pci.c - mt7902_pci_probe - pci_header - 0x%08x", pci_header);
	pci_read_config_dword(pdev, 52, &pci_header); printk(KERN_DEBUG "pci.c - mt7902_pci_probe - pci_header - 0x%08x", pci_header);
	pci_read_config_dword(pdev, 56, &pci_header); printk(KERN_DEBUG "pci.c - mt7902_pci_probe - pci_header - 0x%08x", pci_header);
	pci_read_config_dword(pdev, 60, &pci_header); printk(KERN_DEBUG "pci.c - mt7902_pci_probe - pci_header - 0x%08x", pci_header);
	
	
	printk(KERN_INFO "pci.c - mt7902_pci_probe - pci_set_master(pdev);  cmd: 0x%04x", cmd);
	pci_set_master(pdev);

	ret = pci_alloc_irq_vectors(pdev, 1, 1, PCI_IRQ_ALL_TYPES);
    printk(KERN_INFO "pci.c - mt7902_pci_probe pci_alloc_irq_vectors->ret : %d", ret);
	if (ret < 0)
		return ret;

	ret = dma_set_mask(&pdev->dev, DMA_BIT_MASK(32));      //dma_set_mask(pdev, 0xffffffff)->ret: 0
    printk(KERN_INFO "pci.c - mt7902_pci_probe dma_set_mask(pdev, 0x%08x)->ret: %d", DMA_BIT_MASK(32), ret);
	if (ret)
		goto err_free_pci_vec;

    printk(KERN_INFO "pci.c - mt7902_pci_probe mt7902_disable_aspm: %d", mt7902_disable_aspm);
	if (mt7902_disable_aspm)
		mt76_pci_disable_aspm(pdev);

	ops = mt792x_get_mac80211_ops(&pdev->dev, &mt7902_ops,
				      (void *)id->driver_data, &features);       //mt792x_core.c - mt792x_get_mac80211_ops
	printk(KERN_INFO "pci.c - mt7902_pci_probe mt792x_get_mac80211_ops: %d", !ops);
	if (!ops) {
		ret = -ENOMEM;
		goto err_free_pci_vec;
	}

	mdev = mt76_alloc_device(&pdev->dev, sizeof(*dev), ops, &drv_ops);
	printk(KERN_INFO "pci.c - mt7902_pci_probe mt76_alloc_device: %d", !mdev);
	if (!mdev) {
		ret = -ENOMEM;
		goto err_free_pci_vec;
	}
	printk(KERN_INFO "pci.c - mt7902_pci_probe pci_set_drvdata");
	pci_set_drvdata(pdev, mdev);

	printk(KERN_INFO "pci.c - mt7902_pci_probe mt76_mmio_init");
	dev = container_of(mdev, struct mt792x_dev, mt76);
	dev->fw_features = features;
	dev->hif_ops = &mt7902_pcie_ops;
	dev->irq_map = &irq_map;
	printk(KERN_INFO "pci.c - mt7902_pci_probe mt76_mmio_init(&dev->mt76, pcim_iomap_table(pdev)[0]: 0x%08x, pcim_iomap_table(pdev)[1]: 0x%08x, pcim_iomap(bar0): 0x%08x, pcim_iomap(bar1): 0x%08x);",  pcim_iomap_table(pdev)[0], pcim_iomap_table(pdev)[1], pcim_iomap(pdev, 0, 32000), pcim_iomap(pdev, 1, 32000));
	mt76_mmio_init(&dev->mt76, pcim_iomap_table(pdev)[0]);
	tasklet_init(&mdev->irq_tasklet, mt792x_irq_tasklet, (unsigned long)dev);
	printk(KERN_INFO "pci.c - mt7902_pci_probe devm_kmemdup");
	dev->phy.dev = dev;
	dev->phy.mt76 = &dev->mt76.phy;
	dev->mt76.phy.priv = &dev->phy;
	dev->bus_ops = dev->mt76.bus;

	bus_ops = devm_kmemdup(dev->mt76.dev, dev->bus_ops, sizeof(*bus_ops),
			       GFP_KERNEL);
	printk(KERN_INFO "pci.c - mt7902_pci_probe devm_kmemdup: %d", !bus_ops);
	if (!bus_ops) {
		ret = -ENOMEM;
		goto err_free_dev;
	}

	bus_ops->rr = mt7902_rr;
	bus_ops->wr = mt7902_wr;
	bus_ops->rmw = mt7902_rmw;
	dev->mt76.bus = bus_ops;

	ret = mt792xe_mcu_fw_pmctrl(dev);
	printk(KERN_INFO "pci.c - mt7902_pci_probe mt792xe_mcu_fw_pmctrl->ret: %d", ret);
	if (ret)
		goto err_free_dev;

	ret = __mt792xe_mcu_drv_pmctrl(dev);
	printk(KERN_INFO "pci.c - mt7902_pci_probe __mt792xe_mcu_drv_pmctrl->ret: %d", ret);
	if (ret)
		goto err_free_dev;


	printk(KERN_INFO "pci.c - mt7902_pci_probe - rev - 0x%08x  - 0x%08x", mt7902_l1_rr(dev, MT_HW_CHIPID), MT_HW_CHIPID);  // mt7902_pci_probe - rev - 0x7902  - 0x70010200
	printk(KERN_INFO "pci.c - mt7902_pci_probe - rev - 0x%08x  - 0x%08x", (mt7902_l1_rr(dev, MT_HW_REV) & 0xff), MT_HW_REV); //mt7902_pci_probe - rev - 0x0  - 0x70010204
	mdev->rev = (mt7902_l1_rr(dev, MT_HW_CHIPID) << 16) |
		    (mt7902_l1_rr(dev, MT_HW_REV) & 0xff);
	dev_info(mdev->dev, "ASIC revision: %04x\n", mdev->rev);  //mt7902e 0000:03:00.0: ASIC revision: 79020000

	ret = mt792x_wfsys_reset(dev);  //mt792x_dma.c - mt792x_wfsys_reset
	printk(KERN_INFO "pci.c - mt7902_pci_probe mt792x_wfsys_reset->ret: %d", ret);
	if (ret)
		goto err_free_dev;
		
	printk(KERN_INFO "pci.c - mt7902_pci_probe 	mt76_wr(dev, 0x%08x, 0)", irq_map.host_irq_enable);  //mt76_wr(dev, 0xd4204, 0)
	mt76_wr(dev, irq_map.host_irq_enable, 0);
	printk(KERN_INFO "pci.c - mt7902_pci_probe mt76_wr(dev, 0x%08x, 0xff)", MT_PCIE_MAC_INT_ENABLE);  //mt76_wr(dev, 0x10188, 0xff)
	mt76_wr(dev, MT_PCIE_MAC_INT_ENABLE, 0xff);

	ret = devm_request_irq(mdev->dev, pdev->irq, mt792x_irq_handler,
			       IRQF_SHARED, KBUILD_MODNAME, dev);
	printk(KERN_INFO "pci.c - mt7902_pci_probe devm_request_irq->ret: %d", ret);
	if (ret)
		goto err_free_dev;

	ret = mt7902_dma_init(dev);  //mt76_dma.c - mt76_dma_attach
	printk(KERN_INFO "pci.c - mt7902_pci_probe mt7902_dma_init->ret: %d", ret);
	if (ret)
		goto err_free_irq;

	ret = mt7902_register_device(dev);  // init.c - mt7902_register_device(struct mt792x_dev *dev)
	printk(KERN_INFO "pci.c - mt7902_pci_probe mt7902_register_device->ret: %d", ret);
	if (ret)
		goto err_free_irq;

	return 0;

err_free_irq:
	devm_free_irq(&pdev->dev, pdev->irq, dev);
err_free_dev:
	mt76_free_device(&dev->mt76);
err_free_pci_vec:
	pci_free_irq_vectors(pdev);

	return ret;
}

static void mt7902_pci_remove(struct pci_dev *pdev)
{
    printk(KERN_INFO "pci.c - mt7902_pci_remove(struct pci_dev *pdev)");
	struct mt76_dev *mdev = pci_get_drvdata(pdev);
	struct mt792x_dev *dev = container_of(mdev, struct mt792x_dev, mt76);

	mt7902e_unregister_device(dev);
	set_bit(MT76_REMOVED, &mdev->phy.state);
	devm_free_irq(&pdev->dev, pdev->irq, dev);
	mt76_free_device(&dev->mt76);
	pci_free_irq_vectors(pdev);
}

static int mt7902_pci_suspend(struct device *device)
{
    printk(KERN_INFO "pci.c - mt7902_pci_suspend(struct device *device)");
	struct pci_dev *pdev = to_pci_dev(device);
	struct mt76_dev *mdev = pci_get_drvdata(pdev);
	struct mt792x_dev *dev = container_of(mdev, struct mt792x_dev, mt76);
	struct mt76_connac_pm *pm = &dev->pm;
	int i, err;

	pm->suspended = true;
	flush_work(&dev->reset_work);
	cancel_delayed_work_sync(&pm->ps_work);
	cancel_work_sync(&pm->wake_work);

	err = mt792x_mcu_drv_pmctrl(dev);
	if (err < 0)
		goto restore_suspend;

	err = mt76_connac_mcu_set_hif_suspend(mdev, true);
	if (err)
		goto restore_suspend;

	/* always enable deep sleep during suspend to reduce
	 * power consumption
	 */
	mt76_connac_mcu_set_deep_sleep(&dev->mt76, true);

	napi_disable(&mdev->tx_napi);
	mt76_worker_disable(&mdev->tx_worker);

	mt76_for_each_q_rx(mdev, i) {
		napi_disable(&mdev->napi[i]);
	}

	/* wait until dma is idle  */
	mt76_poll(dev, MT_WFDMA0_GLO_CFG,
		  MT_WFDMA0_GLO_CFG_TX_DMA_BUSY |
		  MT_WFDMA0_GLO_CFG_RX_DMA_BUSY, 0, 1000);

	/* put dma disabled */
	mt76_clear(dev, MT_WFDMA0_GLO_CFG,
		   MT_WFDMA0_GLO_CFG_TX_DMA_EN | MT_WFDMA0_GLO_CFG_RX_DMA_EN);

	/* disable interrupt */
	mt76_wr(dev, dev->irq_map->host_irq_enable, 0);
	mt76_wr(dev, MT_PCIE_MAC_INT_ENABLE, 0x0);
	synchronize_irq(pdev->irq);
	tasklet_kill(&mdev->irq_tasklet);

	err = mt792x_mcu_fw_pmctrl(dev);
	if (err)
		goto restore_napi;

	return 0;

restore_napi:
	mt76_for_each_q_rx(mdev, i) {
		napi_enable(&mdev->napi[i]);
	}
	napi_enable(&mdev->tx_napi);

	if (!pm->ds_enable)
		mt76_connac_mcu_set_deep_sleep(&dev->mt76, false);

	mt76_connac_mcu_set_hif_suspend(mdev, false);

restore_suspend:
	pm->suspended = false;

	if (err < 0)
		mt792x_reset(&dev->mt76);

	return err;
}

static int mt7902_pci_resume(struct device *device)
{
    printk(KERN_INFO "pci.c - mt7902_pci_resume");
	struct pci_dev *pdev = to_pci_dev(device);
	struct mt76_dev *mdev = pci_get_drvdata(pdev);
	struct mt792x_dev *dev = container_of(mdev, struct mt792x_dev, mt76);
	struct mt76_connac_pm *pm = &dev->pm;
	int i, err;

	err = mt792x_mcu_drv_pmctrl(dev);
	if (err < 0)
		goto failed;

	mt792x_wpdma_reinit_cond(dev);

	/* enable interrupt */
	mt76_wr(dev, MT_PCIE_MAC_INT_ENABLE, 0xff);
	mt76_connac_irq_enable(&dev->mt76,
			       dev->irq_map->tx.all_complete_mask |
			       MT_INT_RX_DONE_ALL | MT_INT_MCU_CMD);
	mt76_set(dev, MT_MCU2HOST_SW_INT_ENA, MT_MCU_CMD_WAKE_RX_PCIE);

	/* put dma enabled */
	mt76_set(dev, MT_WFDMA0_GLO_CFG,
		 MT_WFDMA0_GLO_CFG_TX_DMA_EN | MT_WFDMA0_GLO_CFG_RX_DMA_EN);

	mt76_worker_enable(&mdev->tx_worker);

	local_bh_disable();
	mt76_for_each_q_rx(mdev, i) {
		napi_enable(&mdev->napi[i]);
		napi_schedule(&mdev->napi[i]);
	}
	napi_enable(&mdev->tx_napi);
	napi_schedule(&mdev->tx_napi);
	local_bh_enable();

	/* restore previous ds setting */
	if (!pm->ds_enable)
		mt76_connac_mcu_set_deep_sleep(&dev->mt76, false);

	err = mt76_connac_mcu_set_hif_suspend(mdev, false);

	mt7902_regd_update(dev);

failed:
	pm->suspended = false;

	if (err < 0)
		mt792x_reset(&dev->mt76);

	return err;
}

static void mt7902_pci_shutdown(struct pci_dev *pdev)
{
    printk(KERN_INFO "pci.c - mt7902_pci_shutdown");
	mt7902_pci_remove(pdev);
}

static DEFINE_SIMPLE_DEV_PM_OPS(mt7902_pm_ops, mt7902_pci_suspend, mt7902_pci_resume);

static struct pci_driver mt7902_pci_driver = {
	.name		= KBUILD_MODNAME,
	.id_table	= mt7902_pci_device_table,
	.probe		= mt7902_pci_probe,
	.remove		= mt7902_pci_remove,
	.shutdown	= mt7902_pci_shutdown,
	.driver.pm	= pm_sleep_ptr(&mt7902_pm_ops),
};

module_pci_driver(mt7902_pci_driver);

MODULE_DEVICE_TABLE(pci, mt7902_pci_device_table);

MODULE_FIRMWARE(MT7902_FIRMWARE_WM);
MODULE_FIRMWARE(MT7902_ROM_PATCH);
MODULE_AUTHOR("Sean Wang <sean.wang@mediatek.com>");
MODULE_AUTHOR("Lorenzo Bianconi <lorenzo@kernel.org>");
MODULE_DESCRIPTION("MediaTek MT7902E (PCIe) wireless driver");
MODULE_LICENSE("Dual BSD/GPL");
