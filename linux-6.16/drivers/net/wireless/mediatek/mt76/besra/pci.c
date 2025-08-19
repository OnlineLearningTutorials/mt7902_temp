// SPDX-License-Identifier: ISC
/* Copyright (C) 2020 MediaTek Inc.
 *
 * Author: Ryder Lee <ryder.lee@mediatek.com>
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>

#include "besra.h"
#include "mac.h"
#include "../trace.h"

static LIST_HEAD(hif_list);
static DEFINE_SPINLOCK(hif_lock);
static u32 hif_idx;

static const struct pci_device_id besra_pci_device_table[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_MEDIATEK, 0x7902) }, //bellwether
	{ },
};

static const struct pci_device_id besra_hif_device_table[] = {
	{ },
};

static struct besra_hif *besra_pci_get_hif2(u32 idx)
{
	struct besra_hif *hif;
	u32 val;

	spin_lock_bh(&hif_lock);

	list_for_each_entry(hif, &hif_list, list) {
		val = readl(hif->regs + MT_PCIE_RECOG_ID);
		val &= MT_PCIE_RECOG_ID_MASK;
		if (val != idx)
			continue;

		get_device(hif->dev);
		goto out;
	}
	hif = NULL;

out:
	spin_unlock_bh(&hif_lock);

	return hif;
}

static void besra_put_hif2(struct besra_hif *hif)
{
	if (!hif)
		return;

	put_device(hif->dev);
}

static struct besra_hif *besra_pci_init_hif2(struct pci_dev *pdev)
{
	hif_idx++;
	if (!pci_get_device(PCI_VENDOR_ID_MEDIATEK, 0x7916, NULL) &&
	    !pci_get_device(PCI_VENDOR_ID_MEDIATEK, 0x790a, NULL))
		return NULL;

	writel(hif_idx | MT_PCIE_RECOG_ID_SEM,
	       pcim_iomap_table(pdev)[0] + MT_PCIE_RECOG_ID);

	return besra_pci_get_hif2(hif_idx);
}

static int besra_pci_hif2_probe(struct pci_dev *pdev)
{
	struct besra_hif *hif;

	hif = devm_kzalloc(&pdev->dev, sizeof(*hif), GFP_KERNEL);
	if (!hif)
		return -ENOMEM;

	hif->dev = &pdev->dev;
	hif->regs = pcim_iomap_table(pdev)[0];
	hif->irq = pdev->irq;
	spin_lock_bh(&hif_lock);
	list_add(&hif->list, &hif_list);
	spin_unlock_bh(&hif_lock);
	pci_set_drvdata(pdev, hif);

	return 0;
}

static int besra_pci_probe(struct pci_dev *pdev,
			    const struct pci_device_id *id)
{
	struct besra_dev *dev;
	struct mt76_dev *mdev;
	struct besra_hif *hif2;
	int irq;
	int ret;

	ret = pcim_enable_device(pdev);
	if (ret)
		return ret;

	ret = pcim_iomap_regions(pdev, BIT(0), pci_name(pdev));
	if (ret)
		return ret;

	pci_set_master(pdev);

	ret = dma_set_mask(&pdev->dev, DMA_BIT_MASK(32));
	if (ret)
		return ret;

	mt76_pci_disable_aspm(pdev);

	if (id->device == 0x790a)
		return besra_pci_hif2_probe(pdev);

	dev = besra_mmio_probe(&pdev->dev, pcim_iomap_table(pdev)[0],
				id->device);
	if (IS_ERR(dev))
		return PTR_ERR(dev);

	mdev = &dev->mt76;
	besra_wfsys_reset(dev);
	hif2 = besra_pci_init_hif2(pdev);

	ret = pci_alloc_irq_vectors(pdev, 1, 1, PCI_IRQ_ALL_TYPES);
	if (ret < 0)
		goto free_device;

	irq = pdev->irq;
	ret = devm_request_irq(mdev->dev, irq, besra_irq_handler,
			       IRQF_SHARED, KBUILD_MODNAME, dev);
	if (ret)
		goto free_irq_vector;

	mt76_wr(dev, MT_INT_MASK_CSR, 0);

	/* master switch of PCIe tnterrupt enable */
	mt76_wr(dev, MT_PCIE_MAC_INT_ENABLE, 0xff);

	if (hif2) {
		dev->hif2 = hif2;

		mt76_wr(dev, MT_INT1_MASK_CSR, 0);
		/* master switch of PCIe tnterrupt enable */
		mt76_wr(dev, MT_PCIE1_MAC_INT_ENABLE, 0xff);

		ret = devm_request_irq(mdev->dev, dev->hif2->irq,
				       besra_irq_handler, IRQF_SHARED,
				       KBUILD_MODNAME "-hif", dev);
		if (ret)
			goto free_hif2;
	}

	ret = besra_register_device(dev);
	if (ret)
		goto free_hif2_irq;

	return 0;

free_hif2_irq:
	if (dev->hif2)
		devm_free_irq(mdev->dev, dev->hif2->irq, dev);
free_hif2:
	if (dev->hif2)
		put_device(dev->hif2->dev);
	devm_free_irq(mdev->dev, irq, dev);
free_irq_vector:
	pci_free_irq_vectors(pdev);
free_device:
	mt76_free_device(&dev->mt76);

	return ret;
}

static void besra_hif_remove(struct pci_dev *pdev)
{
	struct besra_hif *hif = pci_get_drvdata(pdev);

	list_del(&hif->list);
}

static void besra_pci_remove(struct pci_dev *pdev)
{
	struct mt76_dev *mdev;
	struct besra_dev *dev;

	mdev = pci_get_drvdata(pdev);
	dev = container_of(mdev, struct besra_dev, mt76);
	besra_put_hif2(dev->hif2);
	besra_unregister_device(dev);
}

struct pci_driver besra_hif_driver = {
	.name		= KBUILD_MODNAME "_hif",
	.id_table	= besra_hif_device_table,
	.probe		= besra_pci_probe,
	.remove		= besra_hif_remove,
};

struct pci_driver besra_pci_driver = {
	.name		= KBUILD_MODNAME,
	.id_table	= besra_pci_device_table,
	.probe		= besra_pci_probe,
	.remove		= besra_pci_remove,
};

MODULE_DEVICE_TABLE(pci, besra_pci_device_table);
MODULE_DEVICE_TABLE(pci, besra_hif_device_table);
MODULE_FIRMWARE(MT7902_FIRMWARE_WA);
MODULE_FIRMWARE(MT7902_FIRMWARE_WM);
MODULE_FIRMWARE(MT7902_ROM_PATCH);
MODULE_FIRMWARE(MT7902_FIRMWARE_ROM);
MODULE_FIRMWARE(MT7902_FIRMWARE_ROM_SRAM);