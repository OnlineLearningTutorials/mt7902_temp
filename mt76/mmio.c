// SPDX-License-Identifier: ISC
/*
 * Copyright (C) 2016 Felix Fietkau <nbd@nbd.name>
 */

#include "mt76.h"
#include "dma.h"
#include "trace.h"

static u32 mt76_mmio_rr(struct mt76_dev *dev, u32 offset)
{
	printk(KERN_INFO "mt76_mmio.c - mt76_mmio_rr(struct mt76_dev *dev, 0x%x)", offset);
	u32 val;
	printk(KERN_INFO "mt76_mmio.c - mt76_mmio_rr - dev->mmio.regs: 0x%x", dev->mmio.regs);
	val = readl(dev->mmio.regs + offset);
	printk(KERN_INFO "mt76_mmio.c - mt76_mmio_rr - readl(0x%x)->val: 0x%x", dev->mmio.regs + offset, val);
	trace_reg_rr(dev, offset, val);
	printk(KERN_INFO "mt76_mmio.c - mt76_mmio_rr - val: 0x%x", val);
	return val;
}

static void mt76_mmio_wr(struct mt76_dev *dev, u32 offset, u32 val)
{
	printk(KERN_INFO "mt76_mmio.c - mt76_mmio_wr(struct mt76_dev *dev, 0x%x, 0x%x)", offset, val);
	trace_reg_wr(dev, offset, val);
	printk(KERN_INFO "mt76_mmio.c - mt76_mmio_wr - writel(0x%x, 0x%x) - dev->mmio.regs: 0x%x", val, dev->mmio.regs + offset, dev->mmio.regs);
	writel(val, dev->mmio.regs + offset);
}

static u32 mt76_mmio_rmw(struct mt76_dev *dev, u32 offset, u32 mask, u32 val)
{
	printk(KERN_INFO "mt76_mmio.c - mt76_mmio_rmw(struct mt76_dev *dev, 0x%x, 0x%x, 0x%x)", offset, mask, val);
	val |= mt76_mmio_rr(dev, offset) & ~mask;
	mt76_mmio_wr(dev, offset, val);
	printk(KERN_INFO "mt76_mmio.c - mt76_mmio_rmw - mt76_mmio_wr(dev, 0x%x, 0x%x) - val: 0x%x", offset, val, val);
	return val;
}

static void mt76_mmio_write_copy(struct mt76_dev *dev, u32 offset,
				 const void *data, int len)
{
	printk(KERN_INFO "mt76_mmio.c - mt76_mmio_write_copy");
	__iowrite32_copy(dev->mmio.regs + offset, data, DIV_ROUND_UP(len, 4));
}

static void mt76_mmio_read_copy(struct mt76_dev *dev, u32 offset,
				void *data, int len)
{
	printk(KERN_INFO "mt76_mmio.c - mt76_mmio_read_copy");
	__ioread32_copy(data, dev->mmio.regs + offset, DIV_ROUND_UP(len, 4));
}

static int mt76_mmio_wr_rp(struct mt76_dev *dev, u32 base,
			   const struct mt76_reg_pair *data, int len)
{
	printk(KERN_INFO "mt76_mmio.c - mt76_mmio_wr_rp ");
	while (len > 0) {
		mt76_mmio_wr(dev, data->reg, data->value);
		data++;
		len--;
	}

	return 0;
}

static int mt76_mmio_rd_rp(struct mt76_dev *dev, u32 base,
			   struct mt76_reg_pair *data, int len)
{
	printk(KERN_INFO "mt76_mmio.c - mt76_mmio_rd_rp");
	while (len > 0) {
		data->value = mt76_mmio_rr(dev, data->reg);
		data++;
		len--;
	}

	return 0;
}

void mt76_set_irq_mask(struct mt76_dev *dev, u32 addr,
		       u32 clear, u32 set)
{
	printk(KERN_INFO "mt76_mmio.c - mt76_set_irq_mask(dev, addr:0x%x, clear:0x%x, set:0x%x", addr, clear, set);
	unsigned long flags;

	spin_lock_irqsave(&dev->mmio.irq_lock, flags);
	dev->mmio.irqmask &= ~clear;
	dev->mmio.irqmask |= set;
	if (addr) {
		if (mtk_wed_device_active(&dev->mmio.wed))
			mtk_wed_device_irq_set_mask(&dev->mmio.wed,
						    dev->mmio.irqmask);
		else
			mt76_mmio_wr(dev, addr, dev->mmio.irqmask);
	}
	spin_unlock_irqrestore(&dev->mmio.irq_lock, flags);
}
EXPORT_SYMBOL_GPL(mt76_set_irq_mask);

#ifdef CONFIG_NET_MEDIATEK_SOC_WED
void mt76_mmio_wed_release_rx_buf(struct mtk_wed_device *wed)
{
	printk(KERN_INFO "mt76_mmio.c - mt76_mmio_wed_release_rx_buf");
	struct mt76_dev *dev = container_of(wed, struct mt76_dev, mmio.wed);
	int i;

	for (i = 0; i < dev->rx_token_size; i++) {
		struct mt76_txwi_cache *t;

		t = mt76_rx_token_release(dev, i);
		if (!t || !t->ptr)
			continue;

		mt76_put_page_pool_buf(t->ptr, false);
		t->ptr = NULL;

		mt76_put_rxwi(dev, t);
	}

	mt76_free_pending_rxwi(dev);
}
EXPORT_SYMBOL_GPL(mt76_mmio_wed_release_rx_buf);

u32 mt76_mmio_wed_init_rx_buf(struct mtk_wed_device *wed, int size)
{
	printk(KERN_INFO "mt76_mmio.c - mt76_mmio_wed_init_rx_buf(struct mtk_wed_device *wed, int size:%d)", size);
	struct mt76_dev *dev = container_of(wed, struct mt76_dev, mmio.wed);
	struct mtk_wed_bm_desc *desc = wed->rx_buf_ring.desc;
	struct mt76_queue *q = &dev->q_rx[MT_RXQ_MAIN];
	int i, len = SKB_WITH_OVERHEAD(q->buf_size);
	struct mt76_txwi_cache *t = NULL;

	for (i = 0; i < size; i++) {
		enum dma_data_direction dir;
		dma_addr_t addr;
		u32 offset;
		int token;
		void *buf;

		t = mt76_get_rxwi(dev);
		if (!t)
			goto unmap;

		buf = mt76_get_page_pool_buf(q, &offset, q->buf_size);
		if (!buf)
			goto unmap;

		addr = page_pool_get_dma_addr(virt_to_head_page(buf)) + offset;
		dir = page_pool_get_dma_dir(q->page_pool);
		printk(KERN_INFO "mt76_mmio.c - mt76_mmio_wed_init_rx_buf - dma_sync_single_for_device(dev->dma_dev, addr, len, dir);");
		dma_sync_single_for_device(dev->dma_dev, addr, len, dir);

		desc->buf0 = cpu_to_le32(addr);
		token = mt76_rx_token_consume(dev, buf, t, addr);
		if (token < 0) {
			mt76_put_page_pool_buf(buf, false);
			goto unmap;
		}

		token = FIELD_PREP(MT_DMA_CTL_TOKEN, token);
#ifdef CONFIG_ARCH_DMA_ADDR_T_64BIT
		token |= FIELD_PREP(MT_DMA_CTL_SDP0_H, addr >> 32);
#endif
		desc->token |= cpu_to_le32(token);
		desc++;
	}

	return 0;

unmap:
	if (t)
		mt76_put_rxwi(dev, t);
	mt76_mmio_wed_release_rx_buf(wed);

	return -ENOMEM;
}
EXPORT_SYMBOL_GPL(mt76_mmio_wed_init_rx_buf);

int mt76_mmio_wed_offload_enable(struct mtk_wed_device *wed)
{
	printk(KERN_INFO "mt76_mmio.c - mt76_mmio_wed_offload_enable");
	struct mt76_dev *dev = container_of(wed, struct mt76_dev, mmio.wed);

	spin_lock_bh(&dev->token_lock);
	dev->token_size = wed->wlan.token_start;
	spin_unlock_bh(&dev->token_lock);

	return !wait_event_timeout(dev->tx_wait, !dev->wed_token_count, HZ);
}
EXPORT_SYMBOL_GPL(mt76_mmio_wed_offload_enable);

void mt76_mmio_wed_offload_disable(struct mtk_wed_device *wed)
{
	printk(KERN_INFO "mt76_mmio.c - mt76_mmio_wed_offload_disable");
	struct mt76_dev *dev = container_of(wed, struct mt76_dev, mmio.wed);

	spin_lock_bh(&dev->token_lock);
	dev->token_size = dev->drv->token_size;
	spin_unlock_bh(&dev->token_lock);
}
EXPORT_SYMBOL_GPL(mt76_mmio_wed_offload_disable);

void mt76_mmio_wed_reset_complete(struct mtk_wed_device *wed)
{
	printk(KERN_INFO "mt76_mmio.c - mt76_mmio_wed_reset_complete");
	struct mt76_dev *dev = container_of(wed, struct mt76_dev, mmio.wed);

	complete(&dev->mmio.wed_reset_complete);
}
EXPORT_SYMBOL_GPL(mt76_mmio_wed_reset_complete);
#endif /*CONFIG_NET_MEDIATEK_SOC_WED */

void mt76_mmio_init(struct mt76_dev *dev, void __iomem *regs)
{
	printk(KERN_INFO "mt76_mmio.c - mt76_mmio_init");
	static const struct mt76_bus_ops mt76_mmio_ops = {
		.rr = mt76_mmio_rr,
		.rmw = mt76_mmio_rmw,
		.wr = mt76_mmio_wr,
		.write_copy = mt76_mmio_write_copy,
		.read_copy = mt76_mmio_read_copy,
		.wr_rp = mt76_mmio_wr_rp,
		.rd_rp = mt76_mmio_rd_rp,
		.type = MT76_BUS_MMIO,
	};

	dev->bus = &mt76_mmio_ops;
	dev->mmio.regs = regs;

	spin_lock_init(&dev->mmio.irq_lock);
}
EXPORT_SYMBOL_GPL(mt76_mmio_init);
