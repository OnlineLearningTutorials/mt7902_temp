// SPDX-License-Identifier: ISC
/*
 * Copyright (C) 2019 Lorenzo Bianconi <lorenzo.bianconi83@gmail.com>
 */

#include "mt76.h"

struct sk_buff *
__mt76_mcu_msg_alloc(struct mt76_dev *dev, const void *data,
		     int len, int data_len, gfp_t gfp)
{
	printk(KERN_INFO "mt76_mcu.c - __mt76_mcu_msg_alloc(struct mt76_dev *dev, 0x%x, %d, %d, gfp_t gfp)", data, len, data_len);
	const struct mt76_mcu_ops *ops = dev->mcu_ops;
	struct sk_buff *skb;
	//printk(KERN_INFO "mt76_mcu.c - __mt76_mcu_msg_alloc - len = max_t(int, %d, %d)", len, data_len);
	len = max_t(int, len, data_len);
	//printk(KERN_INFO "mt76_mcu.c - __mt76_mcu_msg_alloc - len: %d", len);
	//printk(KERN_INFO "mt76_mcu.c - __mt76_mcu_msg_alloc - len = ops->headroom: %d + len: %d + ops->tailroom: %d", ops->headroom, len, ops->tailroom);
	len = ops->headroom + len + ops->tailroom;
	printk(KERN_INFO "mt76_mcu.c - __mt76_mcu_msg_alloc - len: %d", len);
	skb = alloc_skb(len, gfp);
	if (!skb)
		return NULL;

	memset(skb->head, 0, len);
	skb_reserve(skb, ops->headroom);
	printk(KERN_INFO "mt76_mcu.c - __mt76_mcu_msg_alloc - data: 0x%x, data_len: %d", data, data_len);
	if (data && data_len) {
		printk(KERN_INFO "mt76_mcu.c - __mt76_mcu_msg_alloc - skb_put_data - data: 0x%x, data_len: %d", data, data_len);
		skb_put_data(skb, data, data_len);
	}

	printk(KERN_INFO "mt76_mcu.c - __mt76_mcu_msg_alloc - data: 0x%x, skb_queue_empty: %d", data, skb_queue_empty(&dev->mcu.res_q));
	return skb;
}
EXPORT_SYMBOL_GPL(__mt76_mcu_msg_alloc);

struct sk_buff *mt76_mcu_get_response(struct mt76_dev *dev,
				      unsigned long expires)
{
	printk(KERN_INFO "mt76_mcu.c - mt76_mcu_get_response(struct mt76_dev *dev, %lu)", expires);
	unsigned long timeout;

	if (!time_is_after_jiffies(expires))
		return NULL;

	timeout = expires - jiffies;
	printk(KERN_INFO "mt76_mcu.c - mt76_mcu_get_response - timeout: %lu - jiffies: %lu, skb_queue_empty: %d, - test_bit: %d, MT76_MCU_RESET: 0x%x- phy.state: 0x%x ", timeout, jiffies,  skb_queue_empty(&dev->mcu.res_q), test_bit(MT76_MCU_RESET, &dev->phy.state),  MT76_MCU_RESET, &dev->phy.state );
	wait_event_timeout(dev->mcu.wait,
			   (!skb_queue_empty(&dev->mcu.res_q) ||
			    test_bit(MT76_MCU_RESET, &dev->phy.state)),
			   timeout);
		printk(KERN_INFO "mt76_mcu.c - mt76_mcu_get_response - timeout: %lu - jiffies: %lu, skb_queue_empty: %d, - test_bit: %d, MT76_MCU_RESET: 0x%x- phy.state: 0x%x ", expires - jiffies, jiffies,  skb_queue_empty(&dev->mcu.res_q), test_bit(MT76_MCU_RESET, &dev->phy.state),  MT76_MCU_RESET, &dev->phy.state );
	printk(KERN_INFO "mt76_mcu.c - mt76_mcu_get_response - jiffies: %lu, skb_dequeue - phy.state: 0x%x", jiffies, &dev->phy.state);
	return skb_dequeue(&dev->mcu.res_q);
}
EXPORT_SYMBOL_GPL(mt76_mcu_get_response);

void mt76_mcu_rx_event(struct mt76_dev *dev, struct sk_buff *skb)
{
	printk(KERN_INFO "mt76_mcu.c - mt76_mcu_rx_event");
	skb_queue_tail(&dev->mcu.res_q, skb);
	wake_up(&dev->mcu.wait);
}
EXPORT_SYMBOL_GPL(mt76_mcu_rx_event);

int mt76_mcu_send_and_get_msg(struct mt76_dev *dev, int cmd, const void *data,
			      int len, bool wait_resp, struct sk_buff **ret_skb)
{
	printk(KERN_INFO "mt76_mcu.c - mt76_mcu_send_and_get_msg(struct mt76_dev *dev, cmd:%d, data:0x%x, len:%d, wait_resp:%d, struct sk_buff **ret_skb) ", cmd, data, len, wait_resp);
	struct sk_buff *skb;

	if (dev->mcu_ops->mcu_send_msg)
		return dev->mcu_ops->mcu_send_msg(dev, cmd, data, len, wait_resp);

	skb = mt76_mcu_msg_alloc(dev, data, len);
	if (!skb)
		return -ENOMEM;

	printk(KERN_INFO "mt76_mcu.c - mt76_mcu_send_and_get_msg - mt76_mcu_skb_send_and_get_msg(dev, skb, %d, %d, ret_skb)", cmd, wait_resp);
	return mt76_mcu_skb_send_and_get_msg(dev, skb, cmd, wait_resp, ret_skb);
}
EXPORT_SYMBOL_GPL(mt76_mcu_send_and_get_msg);

int mt76_mcu_skb_send_and_get_msg(struct mt76_dev *dev, struct sk_buff *skb,
				  int cmd, bool wait_resp,
				  struct sk_buff **ret_skb)
{
	printk(KERN_INFO "mt76_mcu.c - mt76_mcu_skb_send_and_get_msg(struct mt76_dev *dev, struct sk_buff *skb, %d, %d, struct sk_buff **ret_skb)", cmd, wait_resp);
	unsigned long expires;
	int ret, seq;

	if (ret_skb)
		*ret_skb = NULL;

	printk(KERN_INFO "mt76_mcu.c - mt76_mcu_skb_send_and_get_msg - mutex_lock - skb_queue_empty:%d", skb_queue_empty(&dev->mcu.res_q));
	mutex_lock(&dev->mcu.mutex);

	ret = dev->mcu_ops->mcu_skb_send_msg(dev, skb, cmd, &seq);
	printk(KERN_INFO "mt76_mcu.c - mt76_mcu_skb_send_and_get_msg - mcu_skb_send_msg->ret:%d, skb_queue_empty:%d, seq:0x%x", ret, skb_queue_empty(&dev->mcu.res_q), &seq);
	if (ret < 0)
		goto out;

	if (!wait_resp) {
		ret = 0;
		goto out;
	}

	expires = jiffies + dev->mcu.timeout;
	printk(KERN_INFO "mt76_mcu.c - mt76_mcu_skb_send_and_get_msg - expires:%lu", expires);
	do {
		skb = mt76_mcu_get_response(dev, expires);
		printk(KERN_INFO "mt76_mcu.c - mt76_mcu_skb_send_and_get_msg - skb:%d", skb);
		ret = dev->mcu_ops->mcu_parse_response(dev, cmd, skb, seq);
		printk(KERN_INFO "mt76_mcu.c - mt76_mcu_skb_send_and_get_msg - dev->mcu_ops->mcu_parse_response(dev, %d, skb, %d)->ret: %d", cmd, seq, ret);
		if (!ret && ret_skb)
			*ret_skb = skb;
		else
			dev_kfree_skb(skb);
	} while (ret == -EAGAIN);

out:
	printk(KERN_INFO "mt76_mcu.c - mt76_mcu_skb_send_and_get_msg - mutex_unlock");
	mutex_unlock(&dev->mcu.mutex);

	return ret;
}
EXPORT_SYMBOL_GPL(mt76_mcu_skb_send_and_get_msg);

int __mt76_mcu_send_firmware(struct mt76_dev *dev, int cmd, const void *data,
			     int len, int max_len)
{
	printk(KERN_INFO "mt76_mcu.c - __mt76_mcu_send_firmware");
	int err, cur_len;

	while (len > 0) {
		cur_len = min_t(int, max_len, len);

		err = mt76_mcu_send_msg(dev, cmd, data, cur_len, false);
		if (err)
			return err;

		data += cur_len;
		len -= cur_len;

		if (dev->queue_ops->tx_cleanup)
			dev->queue_ops->tx_cleanup(dev,
						   dev->q_mcu[MT_MCUQ_FWDL],
						   false);
	}

	return 0;
}
EXPORT_SYMBOL_GPL(__mt76_mcu_send_firmware);
