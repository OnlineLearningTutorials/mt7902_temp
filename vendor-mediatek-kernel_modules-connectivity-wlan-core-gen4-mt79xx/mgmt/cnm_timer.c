/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 * Id: //Department/DaVinci/BRANCHES/MT6620_WIFI_DRIVER_V2_3/mgmt/cnm_timer.c#1
 */

/*! \file   "cnm_timer.c"
 *    \brief
 *
 */


/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */
#include "precomp.h"

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

/*----------------------------------------------------------------------------*/
/*!
 * \brief This routine is called to set the time to do the time out check.
 *
 * \param[in] rTimeout Time out interval from current time.
 *
 * \retval TRUE Success.
 *
 */
/*----------------------------------------------------------------------------*/
static u_int8_t cnmTimerSetTimer(IN struct ADAPTER *prAdapter,
				IN OS_SYSTIME rTimeout,
				IN enum ENUM_TIMER_WAKELOCK_TYPE_T eType)
{
	struct ROOT_TIMER *prRootTimer;
	u_int8_t fgNeedWakeLock;

	ASSERT(prAdapter);

	prRootTimer = &prAdapter->rRootTimer;

	kalSetTimer(prAdapter->prGlueInfo, rTimeout);

	if ((eType == TIMER_WAKELOCK_REQUEST)
		|| (rTimeout <= SEC_TO_SYSTIME(WAKE_LOCK_MAX_TIME)
		&& (eType == TIMER_WAKELOCK_AUTO))) {
		fgNeedWakeLock = TRUE;

		if (!prRootTimer->fgWakeLocked) {
			KAL_WAKE_LOCK(prAdapter, prRootTimer->prWakeLock);
			prRootTimer->fgWakeLocked = TRUE;
		}
	} else {
		fgNeedWakeLock = FALSE;
	}

	return fgNeedWakeLock;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief This routines is called to initialize a root timer.
 *
 * \param[in] prAdapter
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
void cnmTimerInitialize(IN struct ADAPTER *prAdapter)
{
	struct ROOT_TIMER *prRootTimer;

	KAL_SPIN_LOCK_DECLARATION();

	ASSERT(prAdapter);

	prRootTimer = &prAdapter->rRootTimer;

	/* Note: glue layer have configured timer */

	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_TIMER);
	LINK_INITIALIZE(&prRootTimer->rLinkHead);
	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_TIMER);

	KAL_WAKE_LOCK_INIT(prAdapter, prRootTimer->prWakeLock, "WLAN Timer");
	prRootTimer->fgWakeLocked = FALSE;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief This routines is called to stop a timer.
 *
 * \param[in] prTimer Pointer to a timer structure.
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
static void cnmTimerStopTimer_impl(IN struct ADAPTER *prAdapter,
	IN struct TIMER *prTimer, IN u_int8_t fgAcquireSpinlock)
{
	struct ROOT_TIMER *prRootTimer;

	KAL_SPIN_LOCK_DECLARATION();

	ASSERT(prAdapter);
	ASSERT(prTimer);

	prRootTimer = &prAdapter->rRootTimer;

	if (fgAcquireSpinlock)
		KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_TIMER);

	if (timerPendingTimer(prTimer)) {
		LINK_REMOVE_KNOWN_ENTRY(&prRootTimer->rLinkHead,
			&prTimer->rLinkEntry);

		if (LINK_IS_EMPTY(&prRootTimer->rLinkHead)) {
		       /* kalCancelTimer(prAdapter->prGlueInfo); */

		       /* Violate rule of del_timer_sync which cause DeadLock
			* If no pending timer, let the dummpy timeout happen
			* It would happen only one time
			* Prevent call del_timer_sync with SPIN_LOCK_TIMER hold
			* ===================================================
			* Function: del_timer_sync
			* Note: For !irqsafe timers, you must not hold locks
			* that are held in interrupt context while calling
			* this function. Even if the lock has nothing to do
			* with the timer in question.
			* Here's why:
			*
			* CPU0                    CPU1
			* ----                    ----
			*                         <SOFTIRQ>
			*                         call_timer_fn();
			*                         base->running_timer = mytimer;
			*  spin_lock_irq(somelock);
			*                         <IRQ>
			*                         spin_lock(somelock);
			*  del_timer_sync(mytimer);
			*   while (base->running_timer == mytimer);
			* ==================================================
			*/

			if (fgAcquireSpinlock && prRootTimer->fgWakeLocked) {
				KAL_WAKE_UNLOCK(prAdapter,
					prRootTimer->prWakeLock);
				prRootTimer->fgWakeLocked = FALSE;
			}
		}

		/* Reduce dummy timeout for power saving,
		 * especially HIF activity. If two or more timers
		 * exist and being removed timer is smallest,
		 * this dummy timeout will still happen, but it is OK.
		 */
	}

	if (fgAcquireSpinlock)
		KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_TIMER);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief This routines is called to destroy a root timer.
 *        When WIFI is off, the token shall be returned back to system.
 *
 * \param[in]
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
void cnmTimerDestroy(IN struct ADAPTER *prAdapter)
{
	struct ROOT_TIMER *prRootTimer;
	struct LINK *prTimerList;
	struct LINK_ENTRY *prLinkEntry, *prNextLinkEntry;
	struct TIMER *prTimer;

	KAL_SPIN_LOCK_DECLARATION();

	if (prAdapter == NULL) {
		DBGLOG(INIT, ERROR, "prAdapter=0x%p\n", prAdapter);

		return;
	}

	prRootTimer = &prAdapter->rRootTimer;
	prTimerList = &prRootTimer->rLinkHead;

	if (prRootTimer->fgWakeLocked) {
		KAL_WAKE_UNLOCK(prAdapter, prRootTimer->prWakeLock);
		prRootTimer->fgWakeLocked = FALSE;
	}
	KAL_WAKE_LOCK_DESTROY(prAdapter, prRootTimer->prWakeLock);

	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_TIMER);

	if (LINK_IS_INVALID(prTimerList))
		return;

	LINK_FOR_EACH_SAFE(prLinkEntry, prNextLinkEntry, prTimerList) {
		prTimer = LINK_ENTRY(prLinkEntry, struct TIMER, rLinkEntry);
		ASSERT(prTimer);

		cnmTimerStopTimer_impl(prAdapter, prTimer, FALSE);
	}

	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_TIMER);

	/* Note: glue layer will be responsible for timer destruction */
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief This routines is called to initialize a timer.
 *
 * \param[in] prTimer Pointer to a timer structure.
 * \param[in] pfnFunc Pointer to the call back function.
 * \param[in] u4Data Parameter for call back function.
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
void
cnmTimerInitTimerOption(IN struct ADAPTER *prAdapter,
			IN struct TIMER *prTimer,
			IN PFN_MGMT_TIMEOUT_FUNC pfFunc,
			IN unsigned long ulDataPtr,
			IN enum ENUM_TIMER_WAKELOCK_TYPE_T eType)
{
	if (!prAdapter || !prTimer) {
		DBGLOG(INIT, ERROR, "prAdapter=0x%p prTimer=0x%p\n",
		       prAdapter, prTimer);

		return;
	}

	if (!(eType < TIMER_WAKELOCK_NUM)) {
		DBGLOG(INIT, ERROR, "eType=0x%X\n", eType);

		return;
	}

#if DBG
	/* Note: NULL function pointer is permitted for HEM POWER */
	if (pfFunc == NULL)
		log_dbg(CNM, WARN, "Init timer with NULL callback function!\n");

	if (!prAdapter->rRootTimer.rLinkHead.prNext) {
		DBGLOG(INIT, ERROR, "rRootTimer.rLinkHead.prNext=0x%p\n",
		       prAdapter->rRootTimer.rLinkHead.prNext);

		return;
	}

	{
		struct LINK *prTimerList;
		struct LINK_ENTRY *prLinkEntry;
		struct TIMER *prPendingTimer;

		prTimerList = &(prAdapter->rRootTimer.rLinkHead);

		LINK_FOR_EACH(prLinkEntry, prTimerList) {
			prPendingTimer = LINK_ENTRY(prLinkEntry,
				struct TIMER, rLinkEntry);
			ASSERT(prPendingTimer);
			ASSERT(prPendingTimer != prTimer);
		}
	}
#endif
	if (prTimer->pfMgmtTimeOutFunc == pfFunc
		&& prTimer->rLinkEntry.prNext) {
		log_dbg(CNM, WARN, "re-init timer, func %p\n", pfFunc);
		kal_show_stack(prAdapter, NULL, NULL);
	}

	LINK_ENTRY_INITIALIZE(&prTimer->rLinkEntry);

	prTimer->pfMgmtTimeOutFunc = pfFunc;
	prTimer->ulDataPtr = ulDataPtr;
	prTimer->eType = eType;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief This routines is called to stop a timer.
 *
 * \param[in] prTimer Pointer to a timer structure.
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
void cnmTimerStopTimer(IN struct ADAPTER *prAdapter, IN struct TIMER *prTimer)
{
	ASSERT(prAdapter);
	ASSERT(prTimer);

	cnmTimerStopTimer_impl(prAdapter, prTimer, TRUE);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief This routines is called to start a timer with wake_lock.
 *
 * \param[in] prTimer Pointer to a timer structure.
 * \param[in] u4TimeoutMs Timeout to issue the timer and call back function
 *                        (unit: ms).
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
void cnmTimerStartTimer(IN struct ADAPTER *prAdapter, IN struct TIMER *prTimer,
	IN uint32_t u4TimeoutMs)
{
	struct ROOT_TIMER *prRootTimer;
	struct LINK *prTimerList;
	OS_SYSTIME rExpiredSysTime, rTimeoutSystime;

	KAL_SPIN_LOCK_DECLARATION();

	ASSERT(prAdapter);
	ASSERT(prTimer);

	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_TIMER);

	prRootTimer = &prAdapter->rRootTimer;
	prTimerList = &prRootTimer->rLinkHead;

	/* If timeout interval is larger than 1 minute, the mod value is set
	 * to the timeout value first, then per minutue.
	 */
	if (u4TimeoutMs > MSEC_PER_MIN) {
		ASSERT(u4TimeoutMs <= ((uint32_t) 0xFFFF * MSEC_PER_MIN));

		prTimer->u2Minutes = (uint16_t) (u4TimeoutMs / MSEC_PER_MIN);
		u4TimeoutMs -= (prTimer->u2Minutes * MSEC_PER_MIN);
		if (u4TimeoutMs == 0) {
			u4TimeoutMs = MSEC_PER_MIN;
			prTimer->u2Minutes--;
		}
	} else {
		prTimer->u2Minutes = 0;
	}

	/* The assertion check if MSEC_TO_SYSTIME() may be overflow. */
	ASSERT(u4TimeoutMs < (((uint32_t) 0x80000000 - MSEC_PER_SEC) / KAL_HZ));
	rTimeoutSystime = MSEC_TO_SYSTIME(u4TimeoutMs);
	if (rTimeoutSystime == 0)
		rTimeoutSystime = 1;
	rExpiredSysTime = kalGetTimeTick() + rTimeoutSystime;

	/* If no timer pending or the fast time interval is used. */
	if (LINK_IS_EMPTY(prTimerList)
		|| TIME_BEFORE(rExpiredSysTime,
			prRootTimer->rNextExpiredSysTime)) {

		prRootTimer->rNextExpiredSysTime = rExpiredSysTime;
		cnmTimerSetTimer(prAdapter, rTimeoutSystime, prTimer->eType);
	}

	/* Add this timer to checking list */
	prTimer->rExpiredSysTime = rExpiredSysTime;

	if (!timerPendingTimer(prTimer))
		LINK_INSERT_TAIL(prTimerList, &prTimer->rLinkEntry);

	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_TIMER);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief This routines is called to check the timer list.
 *
 * \param[in]
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
void cnmTimerDoTimeOutCheck(IN struct ADAPTER *prAdapter)
{
	struct ROOT_TIMER *prRootTimer;
	struct LINK *prTimerList;
	struct LINK_ENTRY *prLinkEntry;
	struct TIMER *prTimer;
	OS_SYSTIME rCurSysTime;
	PFN_MGMT_TIMEOUT_FUNC pfMgmtTimeOutFunc;
	unsigned long ulTimeoutDataPtr;
	u_int8_t fgNeedWakeLock;
	enum ENUM_TIMER_WAKELOCK_TYPE_T eType = TIMER_WAKELOCK_NONE;

	KAL_SPIN_LOCK_DECLARATION();

	ASSERT(prAdapter);

	/* acquire spin lock */
	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_TIMER);

	prRootTimer = &prAdapter->rRootTimer;
	prTimerList = &prRootTimer->rLinkHead;

	rCurSysTime = kalGetTimeTick();

	/* Set the permitted max timeout value for new one */
	prRootTimer->rNextExpiredSysTime
		= rCurSysTime + MGMT_MAX_TIMEOUT_INTERVAL;

	LINK_FOR_EACH(prLinkEntry, prTimerList) {
		if (prLinkEntry == NULL)
			break;

		prTimer = LINK_ENTRY(prLinkEntry, struct TIMER, rLinkEntry);
		ASSERT(prTimer);
		if (prLinkEntry->prNext == NULL)
			log_dbg(CNM, WARN, "timer was re-inited, func %p\n",
				prTimer->pfMgmtTimeOutFunc);

		/* Check if this entry is timeout. */
		if (!TIME_BEFORE(rCurSysTime, prTimer->rExpiredSysTime)) {
			cnmTimerStopTimer_impl(prAdapter, prTimer, FALSE);

			pfMgmtTimeOutFunc = prTimer->pfMgmtTimeOutFunc;
			ulTimeoutDataPtr = prTimer->ulDataPtr;

			if (prTimer->u2Minutes > 0) {
				prTimer->u2Minutes--;
				prTimer->rExpiredSysTime
					= rCurSysTime
						+ MSEC_TO_SYSTIME(MSEC_PER_MIN);
				LINK_INSERT_TAIL(prTimerList,
					&prTimer->rLinkEntry);
			} else if (pfMgmtTimeOutFunc) {
				KAL_RELEASE_SPIN_LOCK(prAdapter,
					SPIN_LOCK_TIMER);
			#ifdef UT_TEST_MODE
				if (testTimerTimeout(prAdapter,
						     pfMgmtTimeOutFunc,
						     ulTimeoutDataPtr))
			#endif
				(pfMgmtTimeOutFunc) (prAdapter,
					ulTimeoutDataPtr);
				KAL_ACQUIRE_SPIN_LOCK(prAdapter,
					SPIN_LOCK_TIMER);
			}

			/* Search entire list again because of nest del and add
			 * timers and current MGMT_TIMER could be volatile after
			 * stopped
			 */
			prLinkEntry = (struct LINK_ENTRY *) prTimerList;

			prRootTimer->rNextExpiredSysTime
				= rCurSysTime + MGMT_MAX_TIMEOUT_INTERVAL;
		} else if (TIME_BEFORE(prTimer->rExpiredSysTime,
			prRootTimer->rNextExpiredSysTime)) {
			prRootTimer->rNextExpiredSysTime
				= prTimer->rExpiredSysTime;

			if (prTimer->eType == TIMER_WAKELOCK_REQUEST)
				eType = TIMER_WAKELOCK_REQUEST;
			else if ((eType != TIMER_WAKELOCK_REQUEST)
				&& (prTimer->eType == TIMER_WAKELOCK_AUTO))
				eType = TIMER_WAKELOCK_AUTO;
		}
	}	/* end of for loop */

	/* Setup the prNext timeout event. It is possible the timer was already
	 * set in the above timeout callback function.
	 */
	fgNeedWakeLock = FALSE;
	if (!LINK_IS_EMPTY(prTimerList)) {
		ASSERT(TIME_AFTER(
			prRootTimer->rNextExpiredSysTime, rCurSysTime));

		fgNeedWakeLock = cnmTimerSetTimer(prAdapter,
			(OS_SYSTIME)((int32_t) prRootTimer->rNextExpiredSysTime
				- (int32_t) rCurSysTime),
			eType);
	}

	if (prRootTimer->fgWakeLocked && !fgNeedWakeLock) {
		KAL_WAKE_UNLOCK(prAdapter, prRootTimer->prWakeLock);
		prRootTimer->fgWakeLocked = FALSE;
	}

	/* release spin lock */
	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_TIMER);
}
