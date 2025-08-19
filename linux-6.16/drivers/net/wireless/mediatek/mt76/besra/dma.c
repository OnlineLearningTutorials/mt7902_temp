// SPDX-License-Identifier: ISC
/* Copyright (C) 2020 MediaTek Inc. */

#include "besra.h"
#include "../dma.h"
#include "mac.h"

int besra_init_tx_queues(struct besra_phy *phy, int idx, int n_desc, int ring_base)
{
	int i, err;

	err = mt76_init_tx_queue(phy->mt76, 0, idx, n_desc, ring_base);
	if (err < 0)
		return err;

	for (i = 0; i <= MT_TXQ_PSD; i++)
		phy->mt76->q_tx[i] = phy->mt76->q_tx[0];

	return 0;
}

static void
besra_tx_cleanup(struct besra_dev *dev)
{
	mt76_queue_tx_cleanup(dev, dev->mt76.q_mcu[MT_MCUQ_WM], false);
	mt76_queue_tx_cleanup(dev, dev->mt76.q_mcu[MT_MCUQ_WA], false);
}

static int besra_poll_tx(struct napi_struct *napi, int budget)
{
	struct besra_dev *dev;

	dev = container_of(napi, struct besra_dev, mt76.tx_napi);

	besra_tx_cleanup(dev);

	if (napi_complete_done(napi, 0))
		besra_irq_enable(dev, MT_INT_TX_DONE_MCU);

	return 0;
}

#define Q_CONFIG(q, wfdma, int, id) do {		\
		if (wfdma)				\
			dev->wfdma_mask |= (1 << (q));	\
		dev->q_int_mask[(q)] = int;		\
		dev->q_id[(q)] = id;			\
	} while (0)

#define MCUQ_CONFIG(q, wfdma, int, id)	Q_CONFIG(q, (wfdma), (int), (id))
#define RXQ_CONFIG(q, wfdma, int, id)	Q_CONFIG(__RXQ(q), (wfdma), (int), (id))
#define TXQ_CONFIG(q, wfdma, int, id)	Q_CONFIG(__TXQ(q), (wfdma), (int), (id))

static void besra_dma_config(struct besra_dev *dev)
{
	RXQ_CONFIG(MT_RXQ_MAIN, WFDMA0, MT_INT_RX_DONE_BAND0, BESRA_RXQ_BAND0);
	RXQ_CONFIG(MT_RXQ_MCU, WFDMA0, MT_INT_RX_DONE_WM, BESRA_RXQ_MCU_WM);
	RXQ_CONFIG(MT_RXQ_MCU_WA, WFDMA0, MT_INT_RX_DONE_WA, BESRA_RXQ_MCU_WA);
	RXQ_CONFIG(MT_RXQ_EXT, WFDMA0, MT_INT_RX_DONE_BAND1, BESRA_RXQ_BAND1);
	RXQ_CONFIG(MT_RXQ_EXT_WA, WFDMA0, MT_INT_RX_DONE_WA_EXT, BESRA_RXQ_MCU_WA_EXT);
	RXQ_CONFIG(MT_RXQ_MAIN_WA, WFDMA0, MT_INT_RX_DONE_WA_MAIN, BESRA_RXQ_MCU_WA_MAIN);
	RXQ_CONFIG(MT_RXQ_TRI, WFDMA0, MT_INT_RX_DONE_BAND2, BESRA_RXQ_BAND2);
	RXQ_CONFIG(MT_RXQ_TRI_WA, WFDMA0, MT_INT_RX_DONE_WA_TRI, BESRA_RXQ_MCU_WA_TRI);

	TXQ_CONFIG(0, WFDMA0, MT_INT_TX_DONE_BAND0, BESRA_TXQ_BAND0);
	TXQ_CONFIG(1, WFDMA0, MT_INT_TX_DONE_BAND1, BESRA_TXQ_BAND1);
	TXQ_CONFIG(2, WFDMA0, MT_INT_TX_DONE_BAND2, BESRA_TXQ_BAND2);

	MCUQ_CONFIG(MT_MCUQ_WM, WFDMA0, MT_INT_TX_DONE_MCU_WM, BESRA_TXQ_MCU_WM);
	MCUQ_CONFIG(MT_MCUQ_WA, WFDMA0, MT_INT_TX_DONE_MCU_WA, BESRA_TXQ_MCU_WA);
	MCUQ_CONFIG(MT_MCUQ_FWDL, WFDMA0, MT_INT_TX_DONE_FWDL, BESRA_TXQ_FWDL);
}

static void __besra_dma_prefetch(struct besra_dev *dev, u32 ofs)
{
#define PREFETCH(_base, _depth)	((_base) << 16 | (_depth))

	/* prefetch SRAM wrapping boundary for tx/rx ring. */
	mt76_wr(dev, MT_MCUQ_EXT_CTRL(MT_MCUQ_FWDL) + ofs, PREFETCH(0x0, 0x4));
	mt76_wr(dev, MT_MCUQ_EXT_CTRL(MT_MCUQ_WM) + ofs, PREFETCH(0x40, 0x4));
	mt76_wr(dev, MT_TXQ_EXT_CTRL(0) + ofs, PREFETCH(0x80, 0x4));
	mt76_wr(dev, MT_TXQ_EXT_CTRL(1) + ofs, PREFETCH(0xc0, 0x4));
	mt76_wr(dev, MT_MCUQ_EXT_CTRL(MT_MCUQ_WA) + ofs, PREFETCH(0x100, 0x4));
	mt76_wr(dev, MT_TXQ_EXT_CTRL(2) + ofs, PREFETCH(0x140, 0x4));

	mt76_wr(dev, MT_RXQ_EXT_CTRL(MT_RXQ_MCU) + ofs, PREFETCH(0x180, 0x4));
	mt76_wr(dev, MT_RXQ_EXT_CTRL(MT_RXQ_MCU_WA) + ofs, PREFETCH(0x1c0, 0x4));
	//mt76_wr(dev, MT_RXQ_EXT_CTRL(MT_RXQ_MAIN_WA) + ofs, PREFETCH(0x1c0, 0x4));
	mt76_wr(dev, MT_RXQ_EXT_CTRL(MT_RXQ_EXT_WA) + ofs, PREFETCH(0x200, 0x4));
	mt76_wr(dev, MT_RXQ_EXT_CTRL(MT_RXQ_MAIN) + ofs, PREFETCH(0x240, 0x4));
	mt76_wr(dev, MT_RXQ_EXT_CTRL(MT_RXQ_EXT) + ofs, PREFETCH(0x280, 0x4));
	mt76_wr(dev, MT_RXQ_EXT_CTRL(MT_RXQ_TRI) + ofs, PREFETCH(0x2c0, 0x4));
}

void besra_dma_prefetch(struct besra_dev *dev)
{
	__besra_dma_prefetch(dev, 0);
	if (dev->hif2)
		__besra_dma_prefetch(dev, MT_WFDMA0_PCIE1(0) - MT_WFDMA0(0));
}

static void besra_dma_disable(struct besra_dev *dev, bool rst)
{
	u32 hif1_ofs = 0;

	if (dev->hif2)
		hif1_ofs = MT_WFDMA0_PCIE1(0) - MT_WFDMA0(0);

	/* reset */
	if (rst) {
		mt76_clear(dev, MT_WFDMA0_RST,
			   MT_WFDMA0_RST_DMASHDL_ALL_RST |
			   MT_WFDMA0_RST_LOGIC_RST);

		mt76_set(dev, MT_WFDMA0_RST,
			 MT_WFDMA0_RST_DMASHDL_ALL_RST |
			 MT_WFDMA0_RST_LOGIC_RST);

		if (dev->hif2) {
			mt76_clear(dev, MT_WFDMA0_RST + hif1_ofs,
				   MT_WFDMA0_RST_DMASHDL_ALL_RST |
				   MT_WFDMA0_RST_LOGIC_RST);

			mt76_set(dev, MT_WFDMA0_RST + hif1_ofs,
				 MT_WFDMA0_RST_DMASHDL_ALL_RST |
				 MT_WFDMA0_RST_LOGIC_RST);
		}
	}

	/* disable */
	mt76_clear(dev, MT_WFDMA0_GLO_CFG,
		   MT_WFDMA0_GLO_CFG_TX_DMA_EN |
		   MT_WFDMA0_GLO_CFG_RX_DMA_EN |
		   MT_WFDMA0_GLO_CFG_OMIT_TX_INFO |
		   MT_WFDMA0_GLO_CFG_OMIT_RX_INFO |
		   MT_WFDMA0_GLO_CFG_OMIT_RX_INFO_PFET2);

	if (dev->hif2) {
		mt76_clear(dev, MT_WFDMA0_GLO_CFG + hif1_ofs,
			   MT_WFDMA0_GLO_CFG_TX_DMA_EN |
			   MT_WFDMA0_GLO_CFG_RX_DMA_EN |
			   MT_WFDMA0_GLO_CFG_OMIT_TX_INFO |
			   MT_WFDMA0_GLO_CFG_OMIT_RX_INFO |
			   MT_WFDMA0_GLO_CFG_OMIT_RX_INFO_PFET2);
	}
}

static int besra_dma_enable(struct besra_dev *dev)
{
	u32 hif1_ofs = 0;
	u32 irq_mask;

	if (dev->hif2)
		hif1_ofs = MT_WFDMA0_PCIE1(0) - MT_WFDMA0(0);

	/* reset dma idx */
	mt76_wr(dev, MT_WFDMA0_RST_DTX_PTR, ~0);
	if (dev->hif2)
		mt76_wr(dev, MT_WFDMA0_RST_DTX_PTR + hif1_ofs, ~0);

	/* configure delay interrupt off */
	mt76_wr(dev, MT_WFDMA0_PRI_DLY_INT_CFG0, 0);
	mt76_wr(dev, MT_WFDMA0_PRI_DLY_INT_CFG1, 0);
	mt76_wr(dev, MT_WFDMA0_PRI_DLY_INT_CFG2, 0);

	if (dev->hif2) {
		mt76_wr(dev, MT_WFDMA0_PRI_DLY_INT_CFG0 + hif1_ofs, 0);
		mt76_wr(dev, MT_WFDMA0_PRI_DLY_INT_CFG1 + hif1_ofs, 0);
		mt76_wr(dev, MT_WFDMA0_PRI_DLY_INT_CFG2 + hif1_ofs, 0);
	}

	/* configure perfetch settings */
	besra_dma_prefetch(dev);

	/* hif wait WFDMA idle */
	mt76_set(dev, MT_WFDMA0_BUSY_ENA,
		 MT_WFDMA0_BUSY_ENA_TX_FIFO0 |
		 MT_WFDMA0_BUSY_ENA_TX_FIFO1 |
		 MT_WFDMA0_BUSY_ENA_RX_FIFO);

	if (dev->hif2)
		mt76_set(dev, MT_WFDMA0_BUSY_ENA + hif1_ofs,
			 MT_WFDMA0_PCIE1_BUSY_ENA_TX_FIFO0 |
			 MT_WFDMA0_PCIE1_BUSY_ENA_TX_FIFO1 |
			 MT_WFDMA0_PCIE1_BUSY_ENA_RX_FIFO);

	mt76_poll(dev, MT_WFDMA_EXT_CSR_HIF_MISC,
		  MT_WFDMA_EXT_CSR_HIF_MISC_BUSY, 0, 1000);

	/* set WFDMA Tx/Rx */
	mt76_set(dev, MT_WFDMA0_GLO_CFG,
		 MT_WFDMA0_GLO_CFG_TX_DMA_EN |
		 MT_WFDMA0_GLO_CFG_RX_DMA_EN |
		 MT_WFDMA0_GLO_CFG_OMIT_TX_INFO |
		 MT_WFDMA0_GLO_CFG_OMIT_RX_INFO_PFET2);

	if (dev->hif2) {
		mt76_set(dev, MT_WFDMA0_GLO_CFG + hif1_ofs,
			 MT_WFDMA0_GLO_CFG_TX_DMA_EN |
			 MT_WFDMA0_GLO_CFG_RX_DMA_EN |
			 MT_WFDMA0_GLO_CFG_OMIT_TX_INFO |
			 MT_WFDMA0_GLO_CFG_OMIT_RX_INFO_PFET2);

		mt76_set(dev, MT_WFDMA_HOST_CONFIG,
			 MT_WFDMA_HOST_CONFIG_PDMA_BAND);
	}

	/* enable interrupts for TX/RX rings */
	irq_mask = MT_INT_RX_DONE_MCU |
		   MT_INT_TX_DONE_MCU |
		   MT_INT_MCU_CMD;

	if (!dev->phy.band_idx)
		irq_mask |= MT_INT_BAND0_RX_DONE;

	if (dev->dbdc_support)
		irq_mask |= MT_INT_BAND1_RX_DONE;

	if (dev->tbtc_support)
		irq_mask |= MT_INT_BAND2_RX_DONE;

	besra_irq_enable(dev, irq_mask);

	return 0;
}

int besra_dma_init(struct besra_dev *dev)
{
	u32 hif1_ofs = 0;
	int ret;

	besra_dma_config(dev);

	mt76_dma_attach(&dev->mt76);

	if (dev->hif2)
		hif1_ofs = MT_WFDMA0_PCIE1(0) - MT_WFDMA0(0);

	besra_dma_disable(dev, true);

	/* init tx queue */
	ret = besra_init_tx_queues(&dev->phy,
				    MT_TXQ_ID(dev->phy.band_idx),
				    BESRA_TX_RING_SIZE,
				    MT_TXQ_RING_BASE(0));
	if (ret)
		return ret;

	/* command to WM */
	ret = mt76_init_mcu_queue(&dev->mt76, MT_MCUQ_WM,
				  MT_MCUQ_ID(MT_MCUQ_WM),
				  BESRA_TX_MCU_RING_SIZE,
				  MT_MCUQ_RING_BASE(MT_MCUQ_WM));
	if (ret)
		return ret;

	/* command to WA */
	ret = mt76_init_mcu_queue(&dev->mt76, MT_MCUQ_WA,
				  MT_MCUQ_ID(MT_MCUQ_WA),
				  BESRA_TX_MCU_RING_SIZE,
				  MT_MCUQ_RING_BASE(MT_MCUQ_WA));
	if (ret)
		return ret;

	/* firmware download */
	ret = mt76_init_mcu_queue(&dev->mt76, MT_MCUQ_FWDL,
				  MT_MCUQ_ID(MT_MCUQ_FWDL),
				  BESRA_TX_FWDL_RING_SIZE,
				  MT_MCUQ_RING_BASE(MT_MCUQ_FWDL));
	if (ret)
		return ret;

	/* event from WM */
	ret = mt76_queue_alloc(dev, &dev->mt76.q_rx[MT_RXQ_MCU],
			       MT_RXQ_ID(MT_RXQ_MCU),
			       BESRA_RX_MCU_RING_SIZE,
			       MT_RX_BUF_SIZE,
			       MT_RXQ_RING_BASE(MT_RXQ_MCU));
	if (ret)
		return ret;

	/* event from WA */
	ret = mt76_queue_alloc(dev, &dev->mt76.q_rx[MT_RXQ_MCU_WA],
			       MT_RXQ_ID(MT_RXQ_MCU_WA),
			       BESRA_RX_MCU_RING_SIZE,
			       MT_RX_BUF_SIZE,
			       MT_RXQ_RING_BASE(MT_RXQ_MCU_WA));
	if (ret)
		return ret;

	/* rx data queue for band0 */
	if (!dev->phy.band_idx) {
		ret = mt76_queue_alloc(dev, &dev->mt76.q_rx[MT_RXQ_MAIN],
				       MT_RXQ_ID(MT_RXQ_MAIN),
				       BESRA_RX_RING_SIZE,
				       MT_RX_BUF_SIZE,
				       MT_RXQ_RING_BASE(MT_RXQ_MAIN));
		if (ret)
			return ret;
	}

	/* tx free notify event from WA for band0 */
	ret = mt76_queue_alloc(dev, &dev->mt76.q_rx[MT_RXQ_MAIN_WA],
			       MT_RXQ_ID(MT_RXQ_MAIN_WA),
			       BESRA_RX_MCU_RING_SIZE,
			       MT_RX_BUF_SIZE,
			       MT_RXQ_RING_BASE(MT_RXQ_MAIN_WA));
	if (ret)
		return ret;

	if (dev->dbdc_support || (dev->phy.band_idx == MT_BAND1)) {
		/* rx data queue for band1 */
		ret = mt76_queue_alloc(dev, &dev->mt76.q_rx[MT_RXQ_EXT],
				       MT_RXQ_ID(MT_RXQ_EXT),
				       BESRA_RX_RING_SIZE,
				       MT_RX_BUF_SIZE,
				       MT_RXQ_RING_BASE(MT_RXQ_EXT) + hif1_ofs);
		if (ret)
			return ret;

		/* tx free notify event from WA for band1 */
		ret = mt76_queue_alloc(dev, &dev->mt76.q_rx[MT_RXQ_EXT_WA],
				       MT_RXQ_ID(MT_RXQ_EXT_WA),
				       BESRA_RX_MCU_RING_SIZE,
				       MT_RX_BUF_SIZE,
				       MT_RXQ_RING_BASE(MT_RXQ_EXT_WA) + hif1_ofs);
		if (ret)
			return ret;
	}

	if (dev->tbtc_support || (dev->phy.band_idx == MT_BAND2)) {
		/* rx data queue for band2 */
		ret = mt76_queue_alloc(dev, &dev->mt76.q_rx[MT_RXQ_TRI],
				       MT_RXQ_ID(MT_RXQ_TRI),
				       BESRA_RX_RING_SIZE,
				       MT_RX_BUF_SIZE,
				       MT_RXQ_RING_BASE(MT_RXQ_TRI) + hif1_ofs);
		if (ret)
			return ret;
	}

	ret = mt76_init_queues(dev, mt76_dma_rx_poll);
	if (ret < 0)
		return ret;

	netif_tx_napi_add(&dev->mt76.tx_napi_dev, &dev->mt76.tx_napi,
			  besra_poll_tx, NAPI_POLL_WEIGHT);
	napi_enable(&dev->mt76.tx_napi);

	besra_dma_enable(dev);

	return 0;
}

void besra_dma_cleanup(struct besra_dev *dev)
{
	besra_dma_disable(dev, true);

	mt76_dma_cleanup(&dev->mt76);
}
