/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */
#include "gl_os.h"

#include <uapi/linux/sched/types.h>
#include <linux/sched/task.h>
#include <linux/cpufreq.h>
#include <linux/pm_qos.h>

#include "precomp.h"

#define MAX_CPU_FREQ (3 * 1024 * 1024) /* in kHZ */
#define MAX_CLUSTER_NUM  3
#define CPU_ALL_CORE (0xff)
#define CPU_BIG_CORE (0xc0)
#define CPU_LITTLE_CORE (CPU_ALL_CORE - CPU_BIG_CORE)

enum ENUM_CPU_BOOST_STATUS {
	ENUM_CPU_BOOST_STATUS_INIT = 0,
	ENUM_CPU_BOOST_STATUS_START,
	ENUM_CPU_BOOST_STATUS_STOP,
	ENUM_CPU_BOOST_STATUS_NUM
};

/* mimic store_rps_map as net-sysfs.c does */
int wlan_set_rps_map(struct netdev_rx_queue *queue, unsigned long rps_value)
{
#if KERNEL_VERSION(4, 14, 0) <= CFG80211_VERSION_CODE
	struct rps_map *old_map, *map;
	cpumask_var_t mask;
	int cpu, i;
	static DEFINE_MUTEX(rps_map_mutex);

	if (!alloc_cpumask_var(&mask, GFP_KERNEL))
		return -ENOMEM;

	*cpumask_bits(mask) = rps_value;
	map = kzalloc(max_t(unsigned int,
			RPS_MAP_SIZE(cpumask_weight(mask)), L1_CACHE_BYTES),
			GFP_KERNEL);
	if (!map) {
		free_cpumask_var(mask);
		return -ENOMEM;
	}

	i = 0;
	for_each_cpu_and(cpu, mask, cpu_online_mask)
		map->cpus[i++] = cpu;

	if (i) {
		map->len = i;
	} else {
		kfree(map);
		map = NULL;
	}

	mutex_lock(&rps_map_mutex);
	old_map = rcu_dereference_protected(queue->rps_map,
				mutex_is_locked(&rps_map_mutex));
	rcu_assign_pointer(queue->rps_map, map);
	if (map)
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		static_branch_inc(&rps_needed);
#else
		static_key_slow_inc(&rps_needed);
#endif
	if (old_map)
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		static_branch_dec(&rps_needed);
#else
		static_key_slow_dec(&rps_needed);
#endif
	mutex_unlock(&rps_map_mutex);

	if (old_map)
		kfree_rcu(old_map, rcu);
	free_cpumask_var(mask);

	return 0;
#else
	return 0;
#endif
}

void kalSetRpsMap(IN struct GLUE_INFO *glue, IN unsigned long value)
{
	int32_t i = 0, j = 0;
	struct net_device *dev = NULL;

	for (i = 0; i < BSS_DEFAULT_NUM; i++) {
		dev = wlanGetNetDev(glue, i);
		if (dev) {
			for (j = 0; j < dev->real_num_rx_queues; ++j)
				wlan_set_rps_map(&dev->_rx[j], value);
		}
	}
}

uint32_t kalGetCpuBoostThreshold(void)
{
	DBGLOG(SW4, TRACE, "enter kalGetCpuBoostThreshold\n");
	/* 5, stands for 250Mbps */
	return 5;
}

void kalSetTaskUtilMinPct(IN int pid, IN unsigned int min)
{
	int ret = 0;
	unsigned int blc_1024;
	struct task_struct *p;
	struct sched_attr attr = {};

	if (pid < 0)
		return;

	/* Fill in sched_attr */
	attr.sched_policy = -1;
	attr.sched_flags =
		SCHED_FLAG_KEEP_ALL |
		SCHED_FLAG_UTIL_CLAMP |
		SCHED_FLAG_RESET_ON_FORK;

	if (min == 0) {
		attr.sched_util_min = -1;
		attr.sched_util_max = -1;
	} else {
		blc_1024 = (min << 10) / 100U;
		blc_1024 = clamp(blc_1024, 1U, 1024U);
		attr.sched_util_min = (blc_1024 << 10) / 1280;
		attr.sched_util_max = (blc_1024 << 10) / 1280;
	}

	/* get task_struct */
	rcu_read_lock();
	p = find_task_by_vpid(pid);
	if (likely(p))
		get_task_struct(p);
	rcu_read_unlock();

	/* sched_setattr */
	if (likely(p)) {
		ret = sched_setattr(p, &attr);
		put_task_struct(p);
	}
}

static LIST_HEAD(wlan_policy_list);
struct wlan_policy {
	struct freq_qos_request	qos_req;
	struct list_head	list;
};

void kalSetCpuFreq(IN int32_t freq)
{
	int cpu, ret;
	struct cpufreq_policy *policy;
	struct wlan_policy *wReq;

	if (list_empty(&wlan_policy_list)) {
		for_each_possible_cpu(cpu) {
			policy = cpufreq_cpu_get(cpu);
			if (!policy)
				continue;

			wReq = kzalloc(sizeof(struct wlan_policy), GFP_KERNEL);
			if (!wReq)
				break;

			ret = freq_qos_add_request(&policy->constraints,
				&wReq->qos_req, FREQ_QOS_MIN, 0);
			if (ret < 0) {
				pr_info("%s: freq_qos_add_request fail cpu%d\n",
					__func__, cpu);
				kfree(wReq);
				break;
			}

			list_add_tail(&wReq->list, &wlan_policy_list);
			cpufreq_cpu_put(policy);
		}
	}

	list_for_each_entry(wReq, &wlan_policy_list, list) {
		freq_qos_update_request(&wReq->qos_req, freq);
	}
}

void kalSetDramBoost(IN struct ADAPTER *prAdapter, IN u_int8_t onoff)
{
	/* TODO */
}

int32_t kalBoostCpu(IN struct ADAPTER *prAdapter,
		    IN uint32_t u4TarPerfLevel,
		    IN uint32_t u4BoostCpuTh)
{
	struct GLUE_INFO *prGlueInfo = NULL;
	int32_t i4Freq = -1;
	static u_int8_t fgRequested = ENUM_CPU_BOOST_STATUS_INIT;

	prGlueInfo = (struct GLUE_INFO *)wiphy_priv(wlanGetWiphy());
	i4Freq = (u4TarPerfLevel >= u4BoostCpuTh) ? MAX_CPU_FREQ : -1;

	if (fgRequested == ENUM_CPU_BOOST_STATUS_INIT) {
		/* initially enable rps working at small cores */
		kalSetRpsMap(prGlueInfo, CPU_LITTLE_CORE);
		fgRequested = ENUM_CPU_BOOST_STATUS_STOP;
	}

	if (u4TarPerfLevel >= u4BoostCpuTh) {
		if (fgRequested == ENUM_CPU_BOOST_STATUS_STOP) {
			pr_info("kalBoostCpu start (%d>=%d)\n",
				u4TarPerfLevel, u4BoostCpuTh);
			fgRequested = ENUM_CPU_BOOST_STATUS_START;

			kalSetTaskUtilMinPct(prGlueInfo->u4TxThreadPid, 100);
			kalSetTaskUtilMinPct(prGlueInfo->u4RxThreadPid, 100);
			kalSetTaskUtilMinPct(prGlueInfo->u4HifThreadPid, 100);
			kalSetRpsMap(prGlueInfo, CPU_BIG_CORE);
			kalSetCpuFreq(i4Freq);
			kalSetDramBoost(prAdapter, TRUE);
		}
	} else {
		if (fgRequested == ENUM_CPU_BOOST_STATUS_START) {
			pr_info("kalBoostCpu stop (%d<%d)\n",
				u4TarPerfLevel, u4BoostCpuTh);
			fgRequested = ENUM_CPU_BOOST_STATUS_STOP;

			kalSetTaskUtilMinPct(prGlueInfo->u4TxThreadPid, 0);
			kalSetTaskUtilMinPct(prGlueInfo->u4RxThreadPid, 0);
			kalSetTaskUtilMinPct(prGlueInfo->u4HifThreadPid, 0);
			kalSetRpsMap(prGlueInfo, CPU_LITTLE_CORE);
			kalSetCpuFreq(i4Freq);
			kalSetDramBoost(prAdapter, FALSE);
		}
	}

	return 0;
}

