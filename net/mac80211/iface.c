// SPDX-License-Identifier: GPL-2.0-only
/*
 * Interface handling
 *
 * Copyright 2002-2005, Instant802 Networks, Inc.
 * Copyright 2005-2006, Devicescape Software, Inc.
 * Copyright (c) 2006 Jiri Benc <jbenc@suse.cz>
 * Copyright 2008, Johannes Berg <johannes@sipsolutions.net>
 * Copyright 2013-2014  Intel Mobile Communications GmbH
 * Copyright (c) 2016        Intel Deutschland GmbH
 * Copyright (C) 2018-2025 Intel Corporation
 */
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/if_arp.h>
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>
#include <linux/kcov.h>
#include <net/mac80211.h>
#include <net/ieee80211_radiotap.h>
#include "ieee80211_i.h"
#include "sta_info.h"
#include "debugfs_netdev.h"
#include "mesh.h"
#include "led.h"
#include "driver-ops.h"
#include "wme.h"
#include "rate.h"

/**
 * DOC: Interface list locking
 *
 * The interface list in each struct ieee80211_local is protected
 * three-fold:
 *
 * (1) modifications may only be done under the RTNL *and* wiphy mutex
 *     *and* iflist_mtx
 * (2) modifications are done in an RCU manner so atomic readers
 *     can traverse the list in RCU-safe blocks.
 *
 * As a consequence, reads (traversals) of the list can be protected
 * by either the RTNL, the wiphy mutex, the iflist_mtx or RCU.
 */

static void ieee80211_iface_work(struct wiphy *wiphy, struct wiphy_work *work);

bool __ieee80211_recalc_txpower(struct ieee80211_link_data *link)
{
	struct ieee80211_chanctx_conf *chanctx_conf;
	int power;

	rcu_read_lock();
	chanctx_conf = rcu_dereference(link->conf->chanctx_conf);
	if (!chanctx_conf) {
		rcu_read_unlock();
		return false;
	}

	power = ieee80211_chandef_max_power(&chanctx_conf->def);
	rcu_read_unlock();

	if (link->user_power_level != IEEE80211_UNSET_POWER_LEVEL)
		power = min(power, link->user_power_level);

	if (link->ap_power_level != IEEE80211_UNSET_POWER_LEVEL)
		power = min(power, link->ap_power_level);

	if (power != link->conf->txpower) {
		link->conf->txpower = power;
		return true;
	}

	return false;
}

void ieee80211_recalc_txpower(struct ieee80211_link_data *link,
			      bool update_bss)
{
	if (__ieee80211_recalc_txpower(link) ||
	    (update_bss && ieee80211_sdata_running(link->sdata)))
		ieee80211_link_info_change_notify(link->sdata, link,
						  BSS_CHANGED_TXPOWER);
}

static u32 __ieee80211_idle_off(struct ieee80211_local *local)
{
	if (!(local->hw.conf.flags & IEEE80211_CONF_IDLE))
		return 0;

	local->hw.conf.flags &= ~IEEE80211_CONF_IDLE;
	return IEEE80211_CONF_CHANGE_IDLE;
}

static u32 __ieee80211_idle_on(struct ieee80211_local *local)
{
	if (local->hw.conf.flags & IEEE80211_CONF_IDLE)
		return 0;

	ieee80211_flush_queues(local, NULL, false);

	local->hw.conf.flags |= IEEE80211_CONF_IDLE;
	return IEEE80211_CONF_CHANGE_IDLE;
}

static u32 __ieee80211_recalc_idle(struct ieee80211_local *local,
				   bool force_active)
{
	bool working, scanning, active;
	unsigned int led_trig_start = 0, led_trig_stop = 0;

	lockdep_assert_wiphy(local->hw.wiphy);

	active = force_active ||
		 !list_empty(&local->chanctx_list) ||
		 local->monitors;

	working = !local->ops->remain_on_channel &&
		  !list_empty(&local->roc_list);

	scanning = test_bit(SCAN_SW_SCANNING, &local->scanning) ||
		   test_bit(SCAN_ONCHANNEL_SCANNING, &local->scanning);

	if (working || scanning)
		led_trig_start |= IEEE80211_TPT_LEDTRIG_FL_WORK;
	else
		led_trig_stop |= IEEE80211_TPT_LEDTRIG_FL_WORK;

	if (active)
		led_trig_start |= IEEE80211_TPT_LEDTRIG_FL_CONNECTED;
	else
		led_trig_stop |= IEEE80211_TPT_LEDTRIG_FL_CONNECTED;

	ieee80211_mod_tpt_led_trig(local, led_trig_start, led_trig_stop);

	if (working || scanning || active)
		return __ieee80211_idle_off(local);
	return __ieee80211_idle_on(local);
}

u32 ieee80211_idle_off(struct ieee80211_local *local)
{
	return __ieee80211_recalc_idle(local, true);
}

void ieee80211_recalc_idle(struct ieee80211_local *local)
{
	u32 change = __ieee80211_recalc_idle(local, false);
	if (change)
		ieee80211_hw_config(local, -1, change);
}

static int ieee80211_verify_mac(struct ieee80211_sub_if_data *sdata, u8 *addr,
				bool check_dup)
{
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_sub_if_data *iter;
	u64 new, mask, tmp;
	u8 *m;
	int ret = 0;

	lockdep_assert_wiphy(local->hw.wiphy);

	if (is_zero_ether_addr(local->hw.wiphy->addr_mask))
		return 0;

	m = addr;
	new =	((u64)m[0] << 5*8) | ((u64)m[1] << 4*8) |
		((u64)m[2] << 3*8) | ((u64)m[3] << 2*8) |
		((u64)m[4] << 1*8) | ((u64)m[5] << 0*8);

	m = local->hw.wiphy->addr_mask;
	mask =	((u64)m[0] << 5*8) | ((u64)m[1] << 4*8) |
		((u64)m[2] << 3*8) | ((u64)m[3] << 2*8) |
		((u64)m[4] << 1*8) | ((u64)m[5] << 0*8);

	if (!check_dup)
		return ret;

	list_for_each_entry(iter, &local->interfaces, list) {
		if (iter == sdata)
			continue;

		if (iter->vif.type == NL80211_IFTYPE_MONITOR &&
		    !(iter->u.mntr.flags & MONITOR_FLAG_ACTIVE))
			continue;

		m = iter->vif.addr;
		tmp =	((u64)m[0] << 5*8) | ((u64)m[1] << 4*8) |
			((u64)m[2] << 3*8) | ((u64)m[3] << 2*8) |
			((u64)m[4] << 1*8) | ((u64)m[5] << 0*8);

		if ((new & ~mask) != (tmp & ~mask)) {
			ret = -EINVAL;
			break;
		}
	}

	return ret;
}

static int ieee80211_can_powered_addr_change(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_roc_work *roc;
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_sub_if_data *scan_sdata;
	int ret = 0;

	lockdep_assert_wiphy(local->hw.wiphy);

	/* To be the most flexible here we want to only limit changing the
	 * address if the specific interface is doing offchannel work or
	 * scanning.
	 */
	if (netif_carrier_ok(sdata->dev))
		return -EBUSY;

	/* First check no ROC work is happening on this iface */
	list_for_each_entry(roc, &local->roc_list, list) {
		if (roc->sdata != sdata)
			continue;

		if (roc->started) {
			ret = -EBUSY;
			goto unlock;
		}
	}

	/* And if this iface is scanning */
	if (local->scanning) {
		scan_sdata = rcu_dereference_protected(local->scan_sdata,
						       lockdep_is_held(&local->hw.wiphy->mtx));
		if (sdata == scan_sdata)
			ret = -EBUSY;
	}

	switch (sdata->vif.type) {
	case NL80211_IFTYPE_STATION:
	case NL80211_IFTYPE_P2P_CLIENT:
		/* More interface types could be added here but changing the
		 * address while powered makes the most sense in client modes.
		 */
		break;
	default:
		ret = -EOPNOTSUPP;
	}

unlock:
	return ret;
}

static int _ieee80211_change_mac(struct ieee80211_sub_if_data *sdata,
				 void *addr)
{
	struct ieee80211_local *local = sdata->local;
	struct sockaddr *sa = addr;
	bool check_dup = true;
	bool live = false;
	int ret;

	if (ieee80211_sdata_running(sdata)) {
		ret = ieee80211_can_powered_addr_change(sdata);
		if (ret)
			return ret;

		live = true;
	}

	if (sdata->vif.type == NL80211_IFTYPE_MONITOR &&
	    !(sdata->u.mntr.flags & MONITOR_FLAG_ACTIVE))
		check_dup = false;

	ret = ieee80211_verify_mac(sdata, sa->sa_data, check_dup);
	if (ret)
		return ret;

	if (live)
		drv_remove_interface(local, sdata);
	ret = eth_mac_addr(sdata->dev, sa);

	if (ret == 0) {
		memcpy(sdata->vif.addr, sa->sa_data, ETH_ALEN);
		ether_addr_copy(sdata->vif.bss_conf.addr, sdata->vif.addr);
	}

	/* Regardless of eth_mac_addr() return we still want to add the
	 * interface back. This should not fail...
	 */
	if (live)
		WARN_ON(drv_add_interface(local, sdata));

	return ret;
}

static int ieee80211_change_mac(struct net_device *dev, void *addr)
{
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	struct ieee80211_local *local = sdata->local;

	/*
	 * This happens during unregistration if there's a bond device
	 * active (maybe other cases?) and we must get removed from it.
	 * But we really don't care anymore if it's not registered now.
	 */
	if (!dev->ieee80211_ptr->registered)
		return 0;

	guard(wiphy)(local->hw.wiphy);

	return _ieee80211_change_mac(sdata, addr);
}

static inline int identical_mac_addr_allowed(int type1, int type2)
{
	return type1 == NL80211_IFTYPE_MONITOR ||
		type2 == NL80211_IFTYPE_MONITOR ||
		type1 == NL80211_IFTYPE_P2P_DEVICE ||
		type2 == NL80211_IFTYPE_P2P_DEVICE ||
		(type1 == NL80211_IFTYPE_AP && type2 == NL80211_IFTYPE_AP_VLAN) ||
		(type1 == NL80211_IFTYPE_AP_VLAN &&
			(type2 == NL80211_IFTYPE_AP ||
			 type2 == NL80211_IFTYPE_AP_VLAN));
}

static int ieee80211_check_concurrent_iface(struct ieee80211_sub_if_data *sdata,
					    enum nl80211_iftype iftype)
{
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_sub_if_data *nsdata;

	ASSERT_RTNL();
	lockdep_assert_wiphy(local->hw.wiphy);

	/* we hold the RTNL here so can safely walk the list */
	list_for_each_entry(nsdata, &local->interfaces, list) {
		if (nsdata != sdata && ieee80211_sdata_running(nsdata)) {
			/*
			 * Only OCB and monitor mode may coexist
			 */
			if ((sdata->vif.type == NL80211_IFTYPE_OCB &&
			     nsdata->vif.type != NL80211_IFTYPE_MONITOR) ||
			    (sdata->vif.type != NL80211_IFTYPE_MONITOR &&
			     nsdata->vif.type == NL80211_IFTYPE_OCB))
				return -EBUSY;

			/*
			 * Allow only a single IBSS interface to be up at any
			 * time. This is restricted because beacon distribution
			 * cannot work properly if both are in the same IBSS.
			 *
			 * To remove this restriction we'd have to disallow them
			 * from setting the same SSID on different IBSS interfaces
			 * belonging to the same hardware. Then, however, we're
			 * faced with having to adopt two different TSF timers...
			 */
			if (iftype == NL80211_IFTYPE_ADHOC &&
			    nsdata->vif.type == NL80211_IFTYPE_ADHOC)
				return -EBUSY;
			/*
			 * will not add another interface while any channel
			 * switch is active.
			 */
			if (nsdata->vif.bss_conf.csa_active)
				return -EBUSY;

			/*
			 * The remaining checks are only performed for interfaces
			 * with the same MAC address.
			 */
			if (!ether_addr_equal(sdata->vif.addr,
					      nsdata->vif.addr))
				continue;

			/*
			 * check whether it may have the same address
			 */
			if (!identical_mac_addr_allowed(iftype,
							nsdata->vif.type))
				return -ENOTUNIQ;

			/* No support for VLAN with MLO yet */
			if (iftype == NL80211_IFTYPE_AP_VLAN &&
			    sdata->wdev.use_4addr &&
			    nsdata->vif.type == NL80211_IFTYPE_AP &&
			    nsdata->vif.valid_links)
				return -EOPNOTSUPP;

			/*
			 * can only add VLANs to enabled APs
			 */
			if (iftype == NL80211_IFTYPE_AP_VLAN &&
			    nsdata->vif.type == NL80211_IFTYPE_AP)
				sdata->bss = &nsdata->u.ap;
		}
	}

	return ieee80211_check_combinations(sdata, NULL, 0, 0, -1);
}

static int ieee80211_check_queues(struct ieee80211_sub_if_data *sdata,
				  enum nl80211_iftype iftype)
{
	int n_queues = sdata->local->hw.queues;
	int i;

	if (iftype == NL80211_IFTYPE_NAN)
		return 0;

	if (iftype != NL80211_IFTYPE_P2P_DEVICE) {
		for (i = 0; i < IEEE80211_NUM_ACS; i++) {
			if (WARN_ON_ONCE(sdata->vif.hw_queue[i] ==
					 IEEE80211_INVAL_HW_QUEUE))
				return -EINVAL;
			if (WARN_ON_ONCE(sdata->vif.hw_queue[i] >=
					 n_queues))
				return -EINVAL;
		}
	}

	if ((iftype != NL80211_IFTYPE_AP &&
	     iftype != NL80211_IFTYPE_P2P_GO &&
	     iftype != NL80211_IFTYPE_MESH_POINT) ||
	    !ieee80211_hw_check(&sdata->local->hw, QUEUE_CONTROL)) {
		sdata->vif.cab_queue = IEEE80211_INVAL_HW_QUEUE;
		return 0;
	}

	if (WARN_ON_ONCE(sdata->vif.cab_queue == IEEE80211_INVAL_HW_QUEUE))
		return -EINVAL;

	if (WARN_ON_ONCE(sdata->vif.cab_queue >= n_queues))
		return -EINVAL;

	return 0;
}

static int ieee80211_open(struct net_device *dev)
{
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	int err;

	/* fail early if user set an invalid address */
	if (!is_valid_ether_addr(dev->dev_addr))
		return -EADDRNOTAVAIL;

	guard(wiphy)(sdata->local->hw.wiphy);

	err = ieee80211_check_concurrent_iface(sdata, sdata->vif.type);
	if (err)
		return err;

	return ieee80211_do_open(&sdata->wdev, true);
}

static void ieee80211_do_stop(struct ieee80211_sub_if_data *sdata, bool going_down)
{
	struct ieee80211_local *local = sdata->local;
	unsigned long flags;
	struct sk_buff_head freeq;
	struct sk_buff *skb, *tmp;
	u32 hw_reconf_flags = 0;
	int i, flushed;
	struct ps_data *ps;
	struct cfg80211_chan_def chandef;
	bool cancel_scan;
	struct cfg80211_nan_func *func;

	lockdep_assert_wiphy(local->hw.wiphy);

	clear_bit(SDATA_STATE_RUNNING, &sdata->state);
	synchronize_rcu(); /* flush _ieee80211_wake_txqs() */

	cancel_scan = rcu_access_pointer(local->scan_sdata) == sdata;
	if (cancel_scan)
		ieee80211_scan_cancel(local);

	ieee80211_roc_purge(local, sdata);

	switch (sdata->vif.type) {
	case NL80211_IFTYPE_STATION:
		ieee80211_mgd_stop(sdata);
		break;
	case NL80211_IFTYPE_ADHOC:
		ieee80211_ibss_stop(sdata);
		break;
	case NL80211_IFTYPE_MONITOR:
		list_del_rcu(&sdata->u.mntr.list);
		break;
	case NL80211_IFTYPE_AP_VLAN:
		ieee80211_apvlan_link_clear(sdata);
		break;
	default:
		break;
	}

	/*
	 * Remove all stations associated with this interface.
	 *
	 * This must be done before calling ops->remove_interface()
	 * because otherwise we can later invoke ops->sta_notify()
	 * whenever the STAs are removed, and that invalidates driver
	 * assumptions about always getting a vif pointer that is valid
	 * (because if we remove a STA after ops->remove_interface()
	 * the driver will have removed the vif info already!)
	 *
	 * For AP_VLANs stations may exist since there's nothing else that
	 * would have removed them, but in other modes there shouldn't
	 * be any stations.
	 */
	flushed = sta_info_flush(sdata, -1);
	WARN_ON_ONCE(sdata->vif.type != NL80211_IFTYPE_AP_VLAN && flushed > 0);

	/* don't count this interface for allmulti while it is down */
	if (sdata->flags & IEEE80211_SDATA_ALLMULTI)
		atomic_dec(&local->iff_allmultis);

	if (sdata->vif.type == NL80211_IFTYPE_AP) {
		local->fif_pspoll--;
		local->fif_probe_req--;
	} else if (sdata->vif.type == NL80211_IFTYPE_ADHOC) {
		local->fif_probe_req--;
	}

	if (sdata->dev) {
		netif_addr_lock_bh(sdata->dev);
		spin_lock_bh(&local->filter_lock);
		__hw_addr_unsync(&local->mc_list, &sdata->dev->mc,
				 sdata->dev->addr_len);
		spin_unlock_bh(&local->filter_lock);
		netif_addr_unlock_bh(sdata->dev);
	}

	timer_delete_sync(&local->dynamic_ps_timer);
	wiphy_work_cancel(local->hw.wiphy, &local->dynamic_ps_enable_work);

	WARN(ieee80211_vif_is_mld(&sdata->vif),
	     "destroying interface with valid links 0x%04x\n",
	     sdata->vif.valid_links);

	sdata->vif.bss_conf.csa_active = false;
	if (sdata->vif.type == NL80211_IFTYPE_STATION)
		sdata->deflink.u.mgd.csa.waiting_bcn = false;
	ieee80211_vif_unblock_queues_csa(sdata);

	wiphy_work_cancel(local->hw.wiphy, &sdata->deflink.csa.finalize_work);
	wiphy_work_cancel(local->hw.wiphy,
			  &sdata->deflink.color_change_finalize_work);
	wiphy_delayed_work_cancel(local->hw.wiphy,
				  &sdata->deflink.dfs_cac_timer_work);

	if (sdata->wdev.links[0].cac_started) {
		chandef = sdata->vif.bss_conf.chanreq.oper;
		WARN_ON(local->suspended);
		ieee80211_link_release_channel(&sdata->deflink);
		cfg80211_cac_event(sdata->dev, &chandef,
				   NL80211_RADAR_CAC_ABORTED,
				   GFP_KERNEL, 0);
	}

	if (sdata->vif.type == NL80211_IFTYPE_AP) {
		WARN_ON(!list_empty(&sdata->u.ap.vlans));
	} else if (sdata->vif.type == NL80211_IFTYPE_AP_VLAN) {
		/* remove all packets in parent bc_buf pointing to this dev */
		ps = &sdata->bss->ps;

		spin_lock_irqsave(&ps->bc_buf.lock, flags);
		skb_queue_walk_safe(&ps->bc_buf, skb, tmp) {
			if (skb->dev == sdata->dev) {
				__skb_unlink(skb, &ps->bc_buf);
				local->total_ps_buffered--;
				ieee80211_free_txskb(&local->hw, skb);
			}
		}
		spin_unlock_irqrestore(&ps->bc_buf.lock, flags);
	}

	if (going_down)
		local->open_count--;

	switch (sdata->vif.type) {
	case NL80211_IFTYPE_AP_VLAN:
		list_del(&sdata->u.vlan.list);
		RCU_INIT_POINTER(sdata->vif.bss_conf.chanctx_conf, NULL);
		/* see comment in the default case below */
		ieee80211_free_keys(sdata, true);
		/* no need to tell driver */
		break;
	case NL80211_IFTYPE_MONITOR:
		local->monitors--;

		if (!(sdata->u.mntr.flags & MONITOR_FLAG_ACTIVE) &&
		    !ieee80211_hw_check(&local->hw, NO_VIRTUAL_MONITOR)) {

			local->virt_monitors--;
			if (local->virt_monitors == 0) {
				local->hw.conf.flags &= ~IEEE80211_CONF_MONITOR;
				hw_reconf_flags |= IEEE80211_CONF_CHANGE_MONITOR;
			}

			ieee80211_adjust_monitor_flags(sdata, -1);
		}
		break;
	case NL80211_IFTYPE_NAN:
		/* clean all the functions */
		spin_lock_bh(&sdata->u.nan.func_lock);

		idr_for_each_entry(&sdata->u.nan.function_inst_ids, func, i) {
			idr_remove(&sdata->u.nan.function_inst_ids, i);
			cfg80211_free_nan_func(func);
		}
		idr_destroy(&sdata->u.nan.function_inst_ids);

		spin_unlock_bh(&sdata->u.nan.func_lock);
		break;
	case NL80211_IFTYPE_P2P_DEVICE:
		/* relies on synchronize_rcu() below */
		RCU_INIT_POINTER(local->p2p_sdata, NULL);
		fallthrough;
	default:
		wiphy_work_cancel(sdata->local->hw.wiphy, &sdata->work);
		/*
		 * When we get here, the interface is marked down.
		 * Free the remaining keys, if there are any
		 * (which can happen in AP mode if userspace sets
		 * keys before the interface is operating)
		 *
		 * Force the key freeing to always synchronize_net()
		 * to wait for the RX path in case it is using this
		 * interface enqueuing frames at this very time on
		 * another CPU.
		 */
		ieee80211_free_keys(sdata, true);
		skb_queue_purge(&sdata->skb_queue);
		skb_queue_purge(&sdata->status_queue);
	}

	/*
	 * Since ieee80211_free_txskb() may issue __dev_queue_xmit()
	 * which should be called with interrupts enabled, reclamation
	 * is done in two phases:
	 */
	__skb_queue_head_init(&freeq);

	/* unlink from local queues... */
	spin_lock_irqsave(&local->queue_stop_reason_lock, flags);
	for (i = 0; i < IEEE80211_MAX_QUEUES; i++) {
		skb_queue_walk_safe(&local->pending[i], skb, tmp) {
			struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
			if (info->control.vif == &sdata->vif) {
				__skb_unlink(skb, &local->pending[i]);
				__skb_queue_tail(&freeq, skb);
			}
		}
	}
	spin_unlock_irqrestore(&local->queue_stop_reason_lock, flags);

	/* ... and perform actual reclamation with interrupts enabled. */
	skb_queue_walk_safe(&freeq, skb, tmp) {
		__skb_unlink(skb, &freeq);
		ieee80211_free_txskb(&local->hw, skb);
	}

	if (sdata->vif.type == NL80211_IFTYPE_AP_VLAN)
		ieee80211_txq_remove_vlan(local, sdata);

	if (sdata->vif.txq)
		ieee80211_txq_purge(sdata->local, to_txq_info(sdata->vif.txq));

	sdata->bss = NULL;

	if (local->open_count == 0)
		ieee80211_clear_tx_pending(local);

	sdata->vif.bss_conf.beacon_int = 0;

	/*
	 * If the interface goes down while suspended, presumably because
	 * the device was unplugged and that happens before our resume,
	 * then the driver is already unconfigured and the remainder of
	 * this function isn't needed.
	 * XXX: what about WoWLAN? If the device has software state, e.g.
	 *	memory allocated, it might expect teardown commands from
	 *	mac80211 here?
	 */
	if (local->suspended) {
		WARN_ON(local->wowlan);
		WARN_ON(rcu_access_pointer(local->monitor_sdata));
		return;
	}

	switch (sdata->vif.type) {
	case NL80211_IFTYPE_AP_VLAN:
		break;
	case NL80211_IFTYPE_MONITOR:
		if (local->virt_monitors == 0)
			ieee80211_del_virtual_monitor(local);

		ieee80211_recalc_idle(local);
		ieee80211_recalc_offload(local);

		if (!(sdata->u.mntr.flags & MONITOR_FLAG_ACTIVE) &&
		    !ieee80211_hw_check(&local->hw, NO_VIRTUAL_MONITOR))
			break;

		ieee80211_link_release_channel(&sdata->deflink);
		fallthrough;
	default:
		if (!going_down)
			break;
		drv_remove_interface(local, sdata);

		/* Clear private driver data to prevent reuse */
		memset(sdata->vif.drv_priv, 0, local->hw.vif_data_size);
	}

	ieee80211_recalc_ps(local);

	if (cancel_scan)
		wiphy_delayed_work_flush(local->hw.wiphy, &local->scan_work);

	if (local->open_count == 0) {
		ieee80211_stop_device(local, false);

		/* no reconfiguring after stop! */
		return;
	}

	/* do after stop to avoid reconfiguring when we stop anyway */
	ieee80211_configure_filter(local);
	ieee80211_hw_config(local, -1, hw_reconf_flags);

	if (local->virt_monitors == local->open_count)
		ieee80211_add_virtual_monitor(local);
}

void ieee80211_stop_mbssid(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_sub_if_data *tx_sdata;
	struct ieee80211_bss_conf *link_conf, *tx_bss_conf;
	struct ieee80211_link_data *tx_link, *link;
	unsigned int link_id;

	lockdep_assert_wiphy(sdata->local->hw.wiphy);

	/* Check if any of the links of current sdata is an MBSSID. */
	for_each_vif_active_link(&sdata->vif, link_conf, link_id) {
		tx_bss_conf = sdata_dereference(link_conf->tx_bss_conf, sdata);
		if (!tx_bss_conf)
			continue;

		tx_sdata = vif_to_sdata(tx_bss_conf->vif);
		RCU_INIT_POINTER(link_conf->tx_bss_conf, NULL);

		/* If we are not tx sdata reset tx sdata's tx_bss_conf to avoid recusrion
		 * while closing tx sdata at the end of outer loop below.
		 */
		if (sdata != tx_sdata) {
			tx_link = sdata_dereference(tx_sdata->link[tx_bss_conf->link_id],
						    tx_sdata);
			if (!tx_link)
				continue;

			RCU_INIT_POINTER(tx_link->conf->tx_bss_conf, NULL);
		}

		/* loop through sdatas to find if any of their links
		 * belong to same MBSSID set as the one getting deleted.
		 */
		for_each_sdata_link(tx_sdata->local, link) {
			struct ieee80211_sub_if_data *link_sdata = link->sdata;

			if (link_sdata == sdata || link_sdata == tx_sdata ||
			    rcu_access_pointer(link->conf->tx_bss_conf) != tx_bss_conf)
				continue;

			RCU_INIT_POINTER(link->conf->tx_bss_conf, NULL);

			/* Remove all links of matching MLD until dynamic link
			 * removal can be supported.
			 */
			cfg80211_stop_iface(link_sdata->wdev.wiphy, &link_sdata->wdev,
					    GFP_KERNEL);
		}

		/* If we are not tx sdata, remove links of tx sdata and proceed */
		if (sdata != tx_sdata && ieee80211_sdata_running(tx_sdata))
			cfg80211_stop_iface(tx_sdata->wdev.wiphy,
					    &tx_sdata->wdev, GFP_KERNEL);
	}
}

static int ieee80211_stop(struct net_device *dev)
{
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);

	/* close dependent VLAN interfaces before locking wiphy */
	if (sdata->vif.type == NL80211_IFTYPE_AP) {
		struct ieee80211_sub_if_data *vlan, *tmpsdata;

		list_for_each_entry_safe(vlan, tmpsdata, &sdata->u.ap.vlans,
					 u.vlan.list)
			dev_close(vlan->dev);
	}

	guard(wiphy)(sdata->local->hw.wiphy);

	wiphy_work_cancel(sdata->local->hw.wiphy, &sdata->activate_links_work);

	/* Close the dependent MBSSID interfaces with wiphy lock as we may be
	 * terminating its partner links too in case of MLD.
	 */
	if (sdata->vif.type == NL80211_IFTYPE_AP)
		ieee80211_stop_mbssid(sdata);

	ieee80211_do_stop(sdata, true);

	return 0;
}

static void ieee80211_set_multicast_list(struct net_device *dev)
{
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	struct ieee80211_local *local = sdata->local;
	int allmulti, sdata_allmulti;

	allmulti = !!(dev->flags & IFF_ALLMULTI);
	sdata_allmulti = !!(sdata->flags & IEEE80211_SDATA_ALLMULTI);

	if (allmulti != sdata_allmulti) {
		if (dev->flags & IFF_ALLMULTI)
			atomic_inc(&local->iff_allmultis);
		else
			atomic_dec(&local->iff_allmultis);
		sdata->flags ^= IEEE80211_SDATA_ALLMULTI;
	}

	spin_lock_bh(&local->filter_lock);
	__hw_addr_sync(&local->mc_list, &dev->mc, dev->addr_len);
	spin_unlock_bh(&local->filter_lock);
	wiphy_work_queue(local->hw.wiphy, &local->reconfig_filter);
}

/*
 * Called when the netdev is removed or, by the code below, before
 * the interface type changes.
 */
static void ieee80211_teardown_sdata(struct ieee80211_sub_if_data *sdata)
{
	if (WARN_ON(!list_empty(&sdata->work.entry)))
		wiphy_work_cancel(sdata->local->hw.wiphy, &sdata->work);

	/* free extra data */
	ieee80211_free_keys(sdata, false);

	ieee80211_debugfs_remove_netdev(sdata);

	ieee80211_destroy_frag_cache(&sdata->frags);

	if (ieee80211_vif_is_mesh(&sdata->vif))
		ieee80211_mesh_teardown_sdata(sdata);

	ieee80211_vif_clear_links(sdata);
	ieee80211_link_stop(&sdata->deflink);
}

static void ieee80211_uninit(struct net_device *dev)
{
	ieee80211_teardown_sdata(IEEE80211_DEV_TO_SUB_IF(dev));
}

static int ieee80211_netdev_setup_tc(struct net_device *dev,
				     enum tc_setup_type type, void *type_data)
{
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	struct ieee80211_local *local = sdata->local;

	return drv_net_setup_tc(local, sdata, dev, type, type_data);
}

static const struct net_device_ops ieee80211_dataif_ops = {
	.ndo_open		= ieee80211_open,
	.ndo_stop		= ieee80211_stop,
	.ndo_uninit		= ieee80211_uninit,
	.ndo_start_xmit		= ieee80211_subif_start_xmit,
	.ndo_set_rx_mode	= ieee80211_set_multicast_list,
	.ndo_set_mac_address 	= ieee80211_change_mac,
	.ndo_setup_tc		= ieee80211_netdev_setup_tc,
};

static u16 ieee80211_monitor_select_queue(struct net_device *dev,
					  struct sk_buff *skb,
					  struct net_device *sb_dev)
{
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
	struct ieee80211_hdr *hdr;
	int len_rthdr;

	if (local->hw.queues < IEEE80211_NUM_ACS)
		return 0;

	/* reset flags and info before parsing radiotap header */
	memset(info, 0, sizeof(*info));

	if (!ieee80211_parse_tx_radiotap(skb, dev))
		return 0; /* doesn't matter, frame will be dropped */

	len_rthdr = ieee80211_get_radiotap_len(skb->data);
	hdr = (struct ieee80211_hdr *)(skb->data + len_rthdr);
	if (skb->len < len_rthdr + 2 ||
	    skb->len < len_rthdr + ieee80211_hdrlen(hdr->frame_control))
		return 0; /* doesn't matter, frame will be dropped */

	return ieee80211_select_queue_80211(sdata, skb, hdr);
}

static const struct net_device_ops ieee80211_monitorif_ops = {
	.ndo_open		= ieee80211_open,
	.ndo_stop		= ieee80211_stop,
	.ndo_uninit		= ieee80211_uninit,
	.ndo_start_xmit		= ieee80211_monitor_start_xmit,
	.ndo_set_rx_mode	= ieee80211_set_multicast_list,
	.ndo_set_mac_address 	= ieee80211_change_mac,
	.ndo_select_queue	= ieee80211_monitor_select_queue,
};

static int ieee80211_netdev_fill_forward_path(struct net_device_path_ctx *ctx,
					      struct net_device_path *path)
{
	struct ieee80211_sub_if_data *sdata;
	struct ieee80211_local *local;
	struct sta_info *sta;
	int ret = -ENOENT;

	sdata = IEEE80211_DEV_TO_SUB_IF(ctx->dev);
	local = sdata->local;

	if (!local->ops->net_fill_forward_path)
		return -EOPNOTSUPP;

	rcu_read_lock();
	switch (sdata->vif.type) {
	case NL80211_IFTYPE_AP_VLAN:
		sta = rcu_dereference(sdata->u.vlan.sta);
		if (sta)
			break;
		if (sdata->wdev.use_4addr)
			goto out;
		if (is_multicast_ether_addr(ctx->daddr))
			goto out;
		sta = sta_info_get_bss(sdata, ctx->daddr);
		break;
	case NL80211_IFTYPE_AP:
		if (is_multicast_ether_addr(ctx->daddr))
			goto out;
		sta = sta_info_get(sdata, ctx->daddr);
		break;
	case NL80211_IFTYPE_STATION:
		if (sdata->wdev.wiphy->flags & WIPHY_FLAG_SUPPORTS_TDLS) {
			sta = sta_info_get(sdata, ctx->daddr);
			if (sta && test_sta_flag(sta, WLAN_STA_TDLS_PEER)) {
				if (!test_sta_flag(sta, WLAN_STA_TDLS_PEER_AUTH))
					goto out;

				break;
			}
		}

		sta = sta_info_get(sdata, sdata->deflink.u.mgd.bssid);
		break;
	default:
		goto out;
	}

	if (!sta)
		goto out;

	ret = drv_net_fill_forward_path(local, sdata, &sta->sta, ctx, path);
out:
	rcu_read_unlock();

	return ret;
}

static const struct net_device_ops ieee80211_dataif_8023_ops = {
	.ndo_open		= ieee80211_open,
	.ndo_stop		= ieee80211_stop,
	.ndo_uninit		= ieee80211_uninit,
	.ndo_start_xmit		= ieee80211_subif_start_xmit_8023,
	.ndo_set_rx_mode	= ieee80211_set_multicast_list,
	.ndo_set_mac_address	= ieee80211_change_mac,
	.ndo_fill_forward_path	= ieee80211_netdev_fill_forward_path,
	.ndo_setup_tc		= ieee80211_netdev_setup_tc,
};

static bool ieee80211_iftype_supports_hdr_offload(enum nl80211_iftype iftype)
{
	switch (iftype) {
	/* P2P GO and client are mapped to AP/STATION types */
	case NL80211_IFTYPE_AP:
	case NL80211_IFTYPE_STATION:
		return true;
	default:
		return false;
	}
}

static bool ieee80211_set_sdata_offload_flags(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_local *local = sdata->local;
	u32 flags;

	flags = sdata->vif.offload_flags;

	if (ieee80211_hw_check(&local->hw, SUPPORTS_TX_ENCAP_OFFLOAD) &&
	    ieee80211_iftype_supports_hdr_offload(sdata->vif.type)) {
		flags |= IEEE80211_OFFLOAD_ENCAP_ENABLED;

		if (!ieee80211_hw_check(&local->hw, SUPPORTS_TX_FRAG) &&
		    local->hw.wiphy->frag_threshold != (u32)-1)
			flags &= ~IEEE80211_OFFLOAD_ENCAP_ENABLED;

		if (local->virt_monitors)
			flags &= ~IEEE80211_OFFLOAD_ENCAP_ENABLED;
	} else {
		flags &= ~IEEE80211_OFFLOAD_ENCAP_ENABLED;
	}

	if (ieee80211_hw_check(&local->hw, SUPPORTS_RX_DECAP_OFFLOAD) &&
	    ieee80211_iftype_supports_hdr_offload(sdata->vif.type)) {
		flags |= IEEE80211_OFFLOAD_DECAP_ENABLED;

		if (local->virt_monitors &&
		    !ieee80211_hw_check(&local->hw, SUPPORTS_CONC_MON_RX_DECAP))
			flags &= ~IEEE80211_OFFLOAD_DECAP_ENABLED;
	} else {
		flags &= ~IEEE80211_OFFLOAD_DECAP_ENABLED;
	}

	if (sdata->vif.offload_flags == flags)
		return false;

	sdata->vif.offload_flags = flags;
	ieee80211_check_fast_rx_iface(sdata);
	return true;
}

static void ieee80211_set_vif_encap_ops(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_sub_if_data *bss = sdata;
	bool enabled;

	if (sdata->vif.type == NL80211_IFTYPE_AP_VLAN) {
		if (!sdata->bss)
			return;

		bss = container_of(sdata->bss, struct ieee80211_sub_if_data, u.ap);
	}

	if (!ieee80211_hw_check(&local->hw, SUPPORTS_TX_ENCAP_OFFLOAD) ||
	    !ieee80211_iftype_supports_hdr_offload(bss->vif.type))
		return;

	enabled = bss->vif.offload_flags & IEEE80211_OFFLOAD_ENCAP_ENABLED;
	if (sdata->wdev.use_4addr &&
	    !(bss->vif.offload_flags & IEEE80211_OFFLOAD_ENCAP_4ADDR))
		enabled = false;

	sdata->dev->netdev_ops = enabled ? &ieee80211_dataif_8023_ops :
					   &ieee80211_dataif_ops;
}

static void ieee80211_recalc_sdata_offload(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_sub_if_data *vsdata;

	if (ieee80211_set_sdata_offload_flags(sdata)) {
		drv_update_vif_offload(local, sdata);
		ieee80211_set_vif_encap_ops(sdata);
	}

	list_for_each_entry(vsdata, &local->interfaces, list) {
		if (vsdata->vif.type != NL80211_IFTYPE_AP_VLAN ||
		    vsdata->bss != &sdata->u.ap)
			continue;

		ieee80211_set_vif_encap_ops(vsdata);
	}
}

void ieee80211_recalc_offload(struct ieee80211_local *local)
{
	struct ieee80211_sub_if_data *sdata;

	if (!ieee80211_hw_check(&local->hw, SUPPORTS_TX_ENCAP_OFFLOAD))
		return;

	lockdep_assert_wiphy(local->hw.wiphy);

	list_for_each_entry(sdata, &local->interfaces, list) {
		if (!ieee80211_sdata_running(sdata))
			continue;

		ieee80211_recalc_sdata_offload(sdata);
	}
}

void ieee80211_adjust_monitor_flags(struct ieee80211_sub_if_data *sdata,
				    const int offset)
{
	struct ieee80211_local *local = sdata->local;
	u32 flags = sdata->u.mntr.flags;

#define ADJUST(_f, _s)	do {					\
	if (flags & MONITOR_FLAG_##_f)				\
		local->fif_##_s += offset;			\
	} while (0)

	ADJUST(FCSFAIL, fcsfail);
	ADJUST(PLCPFAIL, plcpfail);
	ADJUST(CONTROL, control);
	ADJUST(CONTROL, pspoll);
	ADJUST(OTHER_BSS, other_bss);
	if (!(flags & MONITOR_FLAG_SKIP_TX))
		local->tx_mntrs += offset;

#undef ADJUST
}

static void ieee80211_set_default_queues(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_local *local = sdata->local;
	int i;

	for (i = 0; i < IEEE80211_NUM_ACS; i++) {
		if (ieee80211_hw_check(&local->hw, QUEUE_CONTROL))
			sdata->vif.hw_queue[i] = IEEE80211_INVAL_HW_QUEUE;
		else if (local->hw.queues >= IEEE80211_NUM_ACS)
			sdata->vif.hw_queue[i] = i;
		else
			sdata->vif.hw_queue[i] = 0;
	}
	sdata->vif.cab_queue = IEEE80211_INVAL_HW_QUEUE;
}

static void ieee80211_sdata_init(struct ieee80211_local *local,
				 struct ieee80211_sub_if_data *sdata)
{
	sdata->local = local;

	INIT_LIST_HEAD(&sdata->key_list);

	/*
	 * Initialize the default link, so we can use link_id 0 for non-MLD,
	 * and that continues to work for non-MLD-aware drivers that use just
	 * vif.bss_conf instead of vif.link_conf.
	 *
	 * Note that we never change this, so if link ID 0 isn't used in an
	 * MLD connection, we get a separate allocation for it.
	 */
	ieee80211_link_init(sdata, -1, &sdata->deflink, &sdata->vif.bss_conf);
}

int ieee80211_add_virtual_monitor(struct ieee80211_local *local)
{
	struct ieee80211_sub_if_data *sdata;
	int ret;

	ASSERT_RTNL();
	lockdep_assert_wiphy(local->hw.wiphy);

	if (local->monitor_sdata ||
	    ieee80211_hw_check(&local->hw, NO_VIRTUAL_MONITOR))
		return 0;

	sdata = kzalloc(sizeof(*sdata) + local->hw.vif_data_size, GFP_KERNEL);
	if (!sdata)
		return -ENOMEM;

	/* set up data */
	sdata->vif.type = NL80211_IFTYPE_MONITOR;
	snprintf(sdata->name, IFNAMSIZ, "%s-monitor",
		 wiphy_name(local->hw.wiphy));
	sdata->wdev.iftype = NL80211_IFTYPE_MONITOR;
	sdata->wdev.wiphy = local->hw.wiphy;

	ieee80211_sdata_init(local, sdata);

	ieee80211_set_default_queues(sdata);

	if (ieee80211_hw_check(&local->hw, WANT_MONITOR_VIF)) {
		ret = drv_add_interface(local, sdata);
		if (WARN_ON(ret)) {
			/* ok .. stupid driver, it asked for this! */
			kfree(sdata);
			return ret;
		}
	}

	set_bit(SDATA_STATE_RUNNING, &sdata->state);

	ret = ieee80211_check_queues(sdata, NL80211_IFTYPE_MONITOR);
	if (ret) {
		kfree(sdata);
		return ret;
	}

	mutex_lock(&local->iflist_mtx);
	rcu_assign_pointer(local->monitor_sdata, sdata);
	mutex_unlock(&local->iflist_mtx);

	ret = ieee80211_link_use_channel(&sdata->deflink, &local->monitor_chanreq,
					 IEEE80211_CHANCTX_EXCLUSIVE);
	if (ret) {
		mutex_lock(&local->iflist_mtx);
		RCU_INIT_POINTER(local->monitor_sdata, NULL);
		mutex_unlock(&local->iflist_mtx);
		synchronize_net();
		drv_remove_interface(local, sdata);
		kfree(sdata);
		return ret;
	}

	skb_queue_head_init(&sdata->skb_queue);
	skb_queue_head_init(&sdata->status_queue);
	wiphy_work_init(&sdata->work, ieee80211_iface_work);

	return 0;
}

void ieee80211_del_virtual_monitor(struct ieee80211_local *local)
{
	struct ieee80211_sub_if_data *sdata;

	if (ieee80211_hw_check(&local->hw, NO_VIRTUAL_MONITOR))
		return;

	ASSERT_RTNL();
	lockdep_assert_wiphy(local->hw.wiphy);

	mutex_lock(&local->iflist_mtx);

	sdata = rcu_dereference_protected(local->monitor_sdata,
					  lockdep_is_held(&local->iflist_mtx));
	if (!sdata) {
		mutex_unlock(&local->iflist_mtx);
		return;
	}

	clear_bit(SDATA_STATE_RUNNING, &sdata->state);
	ieee80211_link_release_channel(&sdata->deflink);

	if (ieee80211_hw_check(&local->hw, WANT_MONITOR_VIF))
		drv_remove_interface(local, sdata);

	RCU_INIT_POINTER(local->monitor_sdata, NULL);
	mutex_unlock(&local->iflist_mtx);

	synchronize_net();

	kfree(sdata);
}

/*
 * NOTE: Be very careful when changing this function, it must NOT return
 * an error on interface type changes that have been pre-checked, so most
 * checks should be in ieee80211_check_concurrent_iface.
 */
int ieee80211_do_open(struct wireless_dev *wdev, bool coming_up)
{
	struct ieee80211_sub_if_data *sdata = IEEE80211_WDEV_TO_SUB_IF(wdev);
	struct net_device *dev = wdev->netdev;
	struct ieee80211_local *local = sdata->local;
	u64 changed = 0;
	int res;
	u32 hw_reconf_flags = 0;

	lockdep_assert_wiphy(local->hw.wiphy);

	switch (sdata->vif.type) {
	case NL80211_IFTYPE_AP_VLAN: {
		struct ieee80211_sub_if_data *master;

		if (!sdata->bss)
			return -ENOLINK;

		list_add(&sdata->u.vlan.list, &sdata->bss->vlans);

		master = container_of(sdata->bss,
				      struct ieee80211_sub_if_data, u.ap);
		sdata->control_port_protocol =
			master->control_port_protocol;
		sdata->control_port_no_encrypt =
			master->control_port_no_encrypt;
		sdata->control_port_over_nl80211 =
			master->control_port_over_nl80211;
		sdata->control_port_no_preauth =
			master->control_port_no_preauth;
		sdata->vif.cab_queue = master->vif.cab_queue;
		memcpy(sdata->vif.hw_queue, master->vif.hw_queue,
		       sizeof(sdata->vif.hw_queue));
		sdata->vif.bss_conf.chanreq = master->vif.bss_conf.chanreq;

		sdata->crypto_tx_tailroom_needed_cnt +=
			master->crypto_tx_tailroom_needed_cnt;

		ieee80211_apvlan_link_setup(sdata);

		break;
		}
	case NL80211_IFTYPE_AP:
		sdata->bss = &sdata->u.ap;
		break;
	case NL80211_IFTYPE_MESH_POINT:
	case NL80211_IFTYPE_STATION:
	case NL80211_IFTYPE_MONITOR:
	case NL80211_IFTYPE_ADHOC:
	case NL80211_IFTYPE_P2P_DEVICE:
	case NL80211_IFTYPE_OCB:
	case NL80211_IFTYPE_NAN:
		/* no special treatment */
		break;
	case NL80211_IFTYPE_UNSPECIFIED:
	case NUM_NL80211_IFTYPES:
	case NL80211_IFTYPE_P2P_CLIENT:
	case NL80211_IFTYPE_P2P_GO:
	case NL80211_IFTYPE_WDS:
		/* cannot happen */
		WARN_ON(1);
		break;
	}

	if (local->open_count == 0) {
		/* here we can consider everything in good order (again) */
		local->reconfig_failure = false;

		res = drv_start(local);
		if (res)
			goto err_del_bss;
		ieee80211_led_radio(local, true);
		ieee80211_mod_tpt_led_trig(local,
					   IEEE80211_TPT_LEDTRIG_FL_RADIO, 0);
	}

	/*
	 * Copy the hopefully now-present MAC address to
	 * this interface, if it has the special null one.
	 */
	if (dev && is_zero_ether_addr(dev->dev_addr)) {
		eth_hw_addr_set(dev, local->hw.wiphy->perm_addr);
		memcpy(dev->perm_addr, dev->dev_addr, ETH_ALEN);

		if (!is_valid_ether_addr(dev->dev_addr)) {
			res = -EADDRNOTAVAIL;
			goto err_stop;
		}
	}

	sdata->vif.addr_valid = sdata->vif.type != NL80211_IFTYPE_MONITOR ||
				(sdata->u.mntr.flags & MONITOR_FLAG_ACTIVE);
	switch (sdata->vif.type) {
	case NL80211_IFTYPE_AP_VLAN:
		/* no need to tell driver, but set carrier and chanctx */
		if (sdata->bss->active) {
			struct ieee80211_link_data *link;

			for_each_link_data(sdata, link) {
				ieee80211_link_vlan_copy_chanctx(link);
			}

			netif_carrier_on(dev);
			ieee80211_set_vif_encap_ops(sdata);
		} else {
			netif_carrier_off(dev);
		}
		break;
	case NL80211_IFTYPE_MONITOR:
		if ((sdata->u.mntr.flags & MONITOR_FLAG_ACTIVE) ||
		    ieee80211_hw_check(&local->hw, NO_VIRTUAL_MONITOR)) {
			res = drv_add_interface(local, sdata);
			if (res)
				goto err_stop;
		} else {
			if (local->virt_monitors == 0 && local->open_count == 0) {
				res = ieee80211_add_virtual_monitor(local);
				if (res)
					goto err_stop;
			}
			local->virt_monitors++;

			/* must be before the call to ieee80211_configure_filter */
			if (local->virt_monitors == 1) {
				local->hw.conf.flags |= IEEE80211_CONF_MONITOR;
				hw_reconf_flags |= IEEE80211_CONF_CHANGE_MONITOR;
			}
		}

		local->monitors++;

		ieee80211_adjust_monitor_flags(sdata, 1);
		ieee80211_configure_filter(local);
		ieee80211_recalc_offload(local);
		ieee80211_recalc_idle(local);

		netif_carrier_on(dev);
		break;
	default:
		if (coming_up) {
			ieee80211_del_virtual_monitor(local);
			ieee80211_set_sdata_offload_flags(sdata);

			res = drv_add_interface(local, sdata);
			if (res)
				goto err_stop;

			ieee80211_set_vif_encap_ops(sdata);
			res = ieee80211_check_queues(sdata,
				ieee80211_vif_type_p2p(&sdata->vif));
			if (res)
				goto err_del_interface;
		}

		if (sdata->vif.type == NL80211_IFTYPE_AP) {
			local->fif_pspoll++;
			local->fif_probe_req++;

			ieee80211_configure_filter(local);
		} else if (sdata->vif.type == NL80211_IFTYPE_ADHOC) {
			local->fif_probe_req++;
		}

		if (sdata->vif.probe_req_reg)
			drv_config_iface_filter(local, sdata,
						FIF_PROBE_REQ,
						FIF_PROBE_REQ);

		if (sdata->vif.type != NL80211_IFTYPE_P2P_DEVICE &&
		    sdata->vif.type != NL80211_IFTYPE_NAN)
			changed |= ieee80211_reset_erp_info(sdata);
		ieee80211_link_info_change_notify(sdata, &sdata->deflink,
						  changed);

		switch (sdata->vif.type) {
		case NL80211_IFTYPE_STATION:
		case NL80211_IFTYPE_ADHOC:
		case NL80211_IFTYPE_AP:
		case NL80211_IFTYPE_MESH_POINT:
		case NL80211_IFTYPE_OCB:
			netif_carrier_off(dev);
			break;
		case NL80211_IFTYPE_P2P_DEVICE:
		case NL80211_IFTYPE_NAN:
			break;
		default:
			/* not reached */
			WARN_ON(1);
		}

		/*
		 * Set default queue parameters so drivers don't
		 * need to initialise the hardware if the hardware
		 * doesn't start up with sane defaults.
		 * Enable QoS for anything but station interfaces.
		 */
		ieee80211_set_wmm_default(&sdata->deflink, true,
			sdata->vif.type != NL80211_IFTYPE_STATION);
	}

	switch (sdata->vif.type) {
	case NL80211_IFTYPE_P2P_DEVICE:
		rcu_assign_pointer(local->p2p_sdata, sdata);
		break;
	case NL80211_IFTYPE_MONITOR:
		list_add_tail_rcu(&sdata->u.mntr.list, &local->mon_list);
		break;
	default:
		break;
	}

	/*
	 * set_multicast_list will be invoked by the networking core
	 * which will check whether any increments here were done in
	 * error and sync them down to the hardware as filter flags.
	 */
	if (sdata->flags & IEEE80211_SDATA_ALLMULTI)
		atomic_inc(&local->iff_allmultis);

	if (coming_up)
		local->open_count++;

	if (local->open_count == 1)
		ieee80211_hw_conf_init(local);
	else if (hw_reconf_flags)
		ieee80211_hw_config(local, -1, hw_reconf_flags);

	ieee80211_recalc_ps(local);

	set_bit(SDATA_STATE_RUNNING, &sdata->state);

	return 0;
 err_del_interface:
	drv_remove_interface(local, sdata);
 err_stop:
	if (!local->open_count)
		drv_stop(local, false);
 err_del_bss:
	sdata->bss = NULL;
	if (sdata->vif.type == NL80211_IFTYPE_AP_VLAN)
		list_del(&sdata->u.vlan.list);
	/* might already be clear but that doesn't matter */
	clear_bit(SDATA_STATE_RUNNING, &sdata->state);
	return res;
}

static void ieee80211_if_setup(struct net_device *dev)
{
	ether_setup(dev);
	dev->priv_flags &= ~IFF_TX_SKB_SHARING;
	dev->priv_flags |= IFF_NO_QUEUE;
	dev->netdev_ops = &ieee80211_dataif_ops;
	dev->needs_free_netdev = true;
}

static void ieee80211_iface_process_skb(struct ieee80211_local *local,
					struct ieee80211_sub_if_data *sdata,
					struct sk_buff *skb)
{
	struct ieee80211_mgmt *mgmt = (void *)skb->data;

	lockdep_assert_wiphy(local->hw.wiphy);

	if (ieee80211_is_action(mgmt->frame_control) &&
	    mgmt->u.action.category == WLAN_CATEGORY_BACK) {
		struct sta_info *sta;
		int len = skb->len;

		sta = sta_info_get_bss(sdata, mgmt->sa);
		if (sta) {
			switch (mgmt->u.action.u.addba_req.action_code) {
			case WLAN_ACTION_ADDBA_REQ:
				ieee80211_process_addba_request(local, sta,
								mgmt, len);
				break;
			case WLAN_ACTION_ADDBA_RESP:
				ieee80211_process_addba_resp(local, sta,
							     mgmt, len);
				break;
			case WLAN_ACTION_DELBA:
				ieee80211_process_delba(sdata, sta,
							mgmt, len);
				break;
			default:
				WARN_ON(1);
				break;
			}
		}
	} else if (ieee80211_is_action(mgmt->frame_control) &&
		   mgmt->u.action.category == WLAN_CATEGORY_HT) {
		switch (mgmt->u.action.u.ht_smps.action) {
		case WLAN_HT_ACTION_NOTIFY_CHANWIDTH: {
			u8 chanwidth = mgmt->u.action.u.ht_notify_cw.chanwidth;
			struct ieee80211_rx_status *status;
			struct link_sta_info *link_sta;
			struct sta_info *sta;

			sta = sta_info_get_bss(sdata, mgmt->sa);
			if (!sta)
				break;

			status = IEEE80211_SKB_RXCB(skb);
			if (!status->link_valid)
				link_sta = &sta->deflink;
			else
				link_sta = rcu_dereference_protected(sta->link[status->link_id],
							lockdep_is_held(&local->hw.wiphy->mtx));
			if (link_sta)
				ieee80211_ht_handle_chanwidth_notif(local, sdata, sta,
								    link_sta, chanwidth,
								    status->band);
			break;
		}
		default:
			WARN_ON(1);
			break;
		}
	} else if (ieee80211_is_action(mgmt->frame_control) &&
		   mgmt->u.action.category == WLAN_CATEGORY_VHT) {
		switch (mgmt->u.action.u.vht_group_notif.action_code) {
		case WLAN_VHT_ACTION_OPMODE_NOTIF: {
			struct ieee80211_rx_status *status;
			enum nl80211_band band;
			struct sta_info *sta;
			u8 opmode;

			status = IEEE80211_SKB_RXCB(skb);
			band = status->band;
			opmode = mgmt->u.action.u.vht_opmode_notif.operating_mode;

			sta = sta_info_get_bss(sdata, mgmt->sa);

			if (sta)
				ieee80211_vht_handle_opmode(sdata,
							    &sta->deflink,
							    opmode, band);

			break;
		}
		case WLAN_VHT_ACTION_GROUPID_MGMT:
			ieee80211_process_mu_groups(sdata, &sdata->deflink,
						    mgmt);
			break;
		default:
			WARN_ON(1);
			break;
		}
	} else if (ieee80211_is_action(mgmt->frame_control) &&
		   mgmt->u.action.category == WLAN_CATEGORY_S1G) {
		switch (mgmt->u.action.u.s1g.action_code) {
		case WLAN_S1G_TWT_TEARDOWN:
		case WLAN_S1G_TWT_SETUP:
			ieee80211_s1g_rx_twt_action(sdata, skb);
			break;
		default:
			break;
		}
	} else if (ieee80211_is_action(mgmt->frame_control) &&
		   mgmt->u.action.category == WLAN_CATEGORY_PROTECTED_EHT) {
		if (sdata->vif.type == NL80211_IFTYPE_STATION) {
			switch (mgmt->u.action.u.ttlm_req.action_code) {
			case WLAN_PROTECTED_EHT_ACTION_TTLM_REQ:
				ieee80211_process_neg_ttlm_req(sdata, mgmt,
							       skb->len);
				break;
			case WLAN_PROTECTED_EHT_ACTION_TTLM_RES:
				ieee80211_process_neg_ttlm_res(sdata, mgmt,
							       skb->len);
				break;
			case WLAN_PROTECTED_EHT_ACTION_TTLM_TEARDOWN:
				ieee80211_process_ttlm_teardown(sdata);
				break;
			case WLAN_PROTECTED_EHT_ACTION_LINK_RECONFIG_RESP:
				ieee80211_process_ml_reconf_resp(sdata, mgmt,
								 skb->len);
				break;
			case WLAN_PROTECTED_EHT_ACTION_EPCS_ENABLE_RESP:
				ieee80211_process_epcs_ena_resp(sdata, mgmt,
								skb->len);
				break;
			case WLAN_PROTECTED_EHT_ACTION_EPCS_ENABLE_TEARDOWN:
				ieee80211_process_epcs_teardown(sdata, mgmt,
								skb->len);
				break;
			default:
				break;
			}
		}
	} else if (ieee80211_is_ext(mgmt->frame_control)) {
		if (sdata->vif.type == NL80211_IFTYPE_STATION)
			ieee80211_sta_rx_queued_ext(sdata, skb);
		else
			WARN_ON(1);
	} else if (ieee80211_is_data_qos(mgmt->frame_control)) {
		struct ieee80211_hdr *hdr = (void *)mgmt;
		struct sta_info *sta;

		/*
		 * So the frame isn't mgmt, but frame_control
		 * is at the right place anyway, of course, so
		 * the if statement is correct.
		 *
		 * Warn if we have other data frame types here,
		 * they must not get here.
		 */
		WARN_ON(hdr->frame_control &
				cpu_to_le16(IEEE80211_STYPE_NULLFUNC));
		WARN_ON(!(hdr->seq_ctrl &
				cpu_to_le16(IEEE80211_SCTL_FRAG)));
		/*
		 * This was a fragment of a frame, received while
		 * a block-ack session was active. That cannot be
		 * right, so terminate the session.
		 */
		sta = sta_info_get_bss(sdata, mgmt->sa);
		if (sta) {
			u16 tid = ieee80211_get_tid(hdr);

			__ieee80211_stop_rx_ba_session(
				sta, tid, WLAN_BACK_RECIPIENT,
				WLAN_REASON_QSTA_REQUIRE_SETUP,
				true);
		}
	} else switch (sdata->vif.type) {
	case NL80211_IFTYPE_STATION:
		ieee80211_sta_rx_queued_mgmt(sdata, skb);
		break;
	case NL80211_IFTYPE_ADHOC:
		ieee80211_ibss_rx_queued_mgmt(sdata, skb);
		break;
	case NL80211_IFTYPE_MESH_POINT:
		if (!ieee80211_vif_is_mesh(&sdata->vif))
			break;
		ieee80211_mesh_rx_queued_mgmt(sdata, skb);
		break;
	default:
		WARN(1, "frame for unexpected interface type");
		break;
	}
}

static void ieee80211_iface_process_status(struct ieee80211_sub_if_data *sdata,
					   struct sk_buff *skb)
{
	struct ieee80211_mgmt *mgmt = (void *)skb->data;

	if (ieee80211_is_action(mgmt->frame_control) &&
	    mgmt->u.action.category == WLAN_CATEGORY_S1G) {
		switch (mgmt->u.action.u.s1g.action_code) {
		case WLAN_S1G_TWT_TEARDOWN:
		case WLAN_S1G_TWT_SETUP:
			ieee80211_s1g_status_twt_action(sdata, skb);
			break;
		default:
			break;
		}
	}
}

static void ieee80211_iface_work(struct wiphy *wiphy, struct wiphy_work *work)
{
	struct ieee80211_sub_if_data *sdata =
		container_of(work, struct ieee80211_sub_if_data, work);
	struct ieee80211_local *local = sdata->local;
	struct sk_buff *skb;

	if (!ieee80211_sdata_running(sdata))
		return;

	if (test_bit(SCAN_SW_SCANNING, &local->scanning))
		return;

	if (!ieee80211_can_run_worker(local))
		return;

	/* first process frames */
	while ((skb = skb_dequeue(&sdata->skb_queue))) {
		kcov_remote_start_common(skb_get_kcov_handle(skb));

		if (skb->protocol == cpu_to_be16(ETH_P_TDLS))
			ieee80211_process_tdls_channel_switch(sdata, skb);
		else
			ieee80211_iface_process_skb(local, sdata, skb);

		kfree_skb(skb);
		kcov_remote_stop();
	}

	/* process status queue */
	while ((skb = skb_dequeue(&sdata->status_queue))) {
		kcov_remote_start_common(skb_get_kcov_handle(skb));

		ieee80211_iface_process_status(sdata, skb);
		kfree_skb(skb);

		kcov_remote_stop();
	}

	/* then other type-dependent work */
	switch (sdata->vif.type) {
	case NL80211_IFTYPE_STATION:
		ieee80211_sta_work(sdata);
		break;
	case NL80211_IFTYPE_ADHOC:
		ieee80211_ibss_work(sdata);
		break;
	case NL80211_IFTYPE_MESH_POINT:
		if (!ieee80211_vif_is_mesh(&sdata->vif))
			break;
		ieee80211_mesh_work(sdata);
		break;
	case NL80211_IFTYPE_OCB:
		ieee80211_ocb_work(sdata);
		break;
	default:
		break;
	}
}

static void ieee80211_activate_links_work(struct wiphy *wiphy,
					  struct wiphy_work *work)
{
	struct ieee80211_sub_if_data *sdata =
		container_of(work, struct ieee80211_sub_if_data,
			     activate_links_work);
	struct ieee80211_local *local = wiphy_priv(wiphy);

	if (local->in_reconfig)
		return;

	ieee80211_set_active_links(&sdata->vif, sdata->desired_active_links);
	sdata->desired_active_links = 0;
}

/*
 * Helper function to initialise an interface to a specific type.
 */
static void ieee80211_setup_sdata(struct ieee80211_sub_if_data *sdata,
				  enum nl80211_iftype type)
{
	static const u8 bssid_wildcard[ETH_ALEN] = {0xff, 0xff, 0xff,
						    0xff, 0xff, 0xff};

	/* clear type-dependent unions */
	memset(&sdata->u, 0, sizeof(sdata->u));
	memset(&sdata->deflink.u, 0, sizeof(sdata->deflink.u));

	/* and set some type-dependent values */
	sdata->vif.type = type;
	sdata->vif.p2p = false;
	sdata->wdev.iftype = type;

	sdata->control_port_protocol = cpu_to_be16(ETH_P_PAE);
	sdata->control_port_no_encrypt = false;
	sdata->control_port_over_nl80211 = false;
	sdata->control_port_no_preauth = false;
	sdata->vif.cfg.idle = true;
	sdata->vif.bss_conf.txpower = INT_MIN; /* unset */

	sdata->noack_map = 0;

	/* only monitor/p2p-device differ */
	if (sdata->dev) {
		sdata->dev->netdev_ops = &ieee80211_dataif_ops;
		sdata->dev->type = ARPHRD_ETHER;
	}

	skb_queue_head_init(&sdata->skb_queue);
	skb_queue_head_init(&sdata->status_queue);
	wiphy_work_init(&sdata->work, ieee80211_iface_work);
	wiphy_work_init(&sdata->activate_links_work,
			ieee80211_activate_links_work);

	switch (type) {
	case NL80211_IFTYPE_P2P_GO:
		type = NL80211_IFTYPE_AP;
		sdata->vif.type = type;
		sdata->vif.p2p = true;
		fallthrough;
	case NL80211_IFTYPE_AP:
		skb_queue_head_init(&sdata->u.ap.ps.bc_buf);
		INIT_LIST_HEAD(&sdata->u.ap.vlans);
		sdata->vif.bss_conf.bssid = sdata->vif.addr;
		break;
	case NL80211_IFTYPE_P2P_CLIENT:
		type = NL80211_IFTYPE_STATION;
		sdata->vif.type = type;
		sdata->vif.p2p = true;
		fallthrough;
	case NL80211_IFTYPE_STATION:
		sdata->vif.bss_conf.bssid = sdata->deflink.u.mgd.bssid;
		ieee80211_sta_setup_sdata(sdata);
		break;
	case NL80211_IFTYPE_OCB:
		sdata->vif.bss_conf.bssid = bssid_wildcard;
		ieee80211_ocb_setup_sdata(sdata);
		break;
	case NL80211_IFTYPE_ADHOC:
		sdata->vif.bss_conf.bssid = sdata->u.ibss.bssid;
		ieee80211_ibss_setup_sdata(sdata);
		break;
	case NL80211_IFTYPE_MESH_POINT:
		if (ieee80211_vif_is_mesh(&sdata->vif))
			ieee80211_mesh_init_sdata(sdata);
		break;
	case NL80211_IFTYPE_MONITOR:
		sdata->dev->type = ARPHRD_IEEE80211_RADIOTAP;
		sdata->dev->netdev_ops = &ieee80211_monitorif_ops;
		sdata->u.mntr.flags = MONITOR_FLAG_CONTROL |
				      MONITOR_FLAG_OTHER_BSS;
		break;
	case NL80211_IFTYPE_NAN:
		idr_init(&sdata->u.nan.function_inst_ids);
		spin_lock_init(&sdata->u.nan.func_lock);
		sdata->vif.bss_conf.bssid = sdata->vif.addr;
		break;
	case NL80211_IFTYPE_AP_VLAN:
	case NL80211_IFTYPE_P2P_DEVICE:
		sdata->vif.bss_conf.bssid = sdata->vif.addr;
		break;
	case NL80211_IFTYPE_UNSPECIFIED:
	case NL80211_IFTYPE_WDS:
	case NUM_NL80211_IFTYPES:
		WARN_ON(1);
		break;
	}

	/* need to do this after the switch so vif.type is correct */
	ieee80211_link_setup(&sdata->deflink);

	ieee80211_debugfs_recreate_netdev(sdata, false);
}

static int ieee80211_runtime_change_iftype(struct ieee80211_sub_if_data *sdata,
					   enum nl80211_iftype type)
{
	struct ieee80211_local *local = sdata->local;
	int ret, err;
	enum nl80211_iftype internal_type = type;
	bool p2p = false;

	ASSERT_RTNL();

	if (!local->ops->change_interface)
		return -EBUSY;

	/* for now, don't support changing while links exist */
	if (ieee80211_vif_is_mld(&sdata->vif))
		return -EBUSY;

	switch (sdata->vif.type) {
	case NL80211_IFTYPE_AP:
		if (!list_empty(&sdata->u.ap.vlans))
			return -EBUSY;
		break;
	case NL80211_IFTYPE_STATION:
	case NL80211_IFTYPE_ADHOC:
	case NL80211_IFTYPE_OCB:
		/*
		 * Could maybe also all others here?
		 * Just not sure how that interacts
		 * with the RX/config path e.g. for
		 * mesh.
		 */
		break;
	default:
		return -EBUSY;
	}

	switch (type) {
	case NL80211_IFTYPE_AP:
	case NL80211_IFTYPE_STATION:
	case NL80211_IFTYPE_ADHOC:
	case NL80211_IFTYPE_OCB:
		/*
		 * Could probably support everything
		 * but here.
		 */
		break;
	case NL80211_IFTYPE_P2P_CLIENT:
		p2p = true;
		internal_type = NL80211_IFTYPE_STATION;
		break;
	case NL80211_IFTYPE_P2P_GO:
		p2p = true;
		internal_type = NL80211_IFTYPE_AP;
		break;
	default:
		return -EBUSY;
	}

	ret = ieee80211_check_concurrent_iface(sdata, internal_type);
	if (ret)
		return ret;

	ieee80211_stop_vif_queues(local, sdata,
				  IEEE80211_QUEUE_STOP_REASON_IFTYPE_CHANGE);
	/* do_stop will synchronize_rcu() first thing */
	ieee80211_do_stop(sdata, false);

	ieee80211_teardown_sdata(sdata);

	ieee80211_set_sdata_offload_flags(sdata);
	ret = drv_change_interface(local, sdata, internal_type, p2p);
	if (ret)
		type = ieee80211_vif_type_p2p(&sdata->vif);

	/*
	 * Ignore return value here, there's not much we can do since
	 * the driver changed the interface type internally already.
	 * The warnings will hopefully make driver authors fix it :-)
	 */
	ieee80211_check_queues(sdata, type);

	ieee80211_setup_sdata(sdata, type);
	ieee80211_set_vif_encap_ops(sdata);

	err = ieee80211_do_open(&sdata->wdev, false);
	WARN(err, "type change: do_open returned %d", err);

	ieee80211_wake_vif_queues(local, sdata,
				  IEEE80211_QUEUE_STOP_REASON_IFTYPE_CHANGE);
	return ret;
}

int ieee80211_if_change_type(struct ieee80211_sub_if_data *sdata,
			     enum nl80211_iftype type)
{
	int ret;

	ASSERT_RTNL();

	if (type == ieee80211_vif_type_p2p(&sdata->vif))
		return 0;

	if (ieee80211_sdata_running(sdata)) {
		ret = ieee80211_runtime_change_iftype(sdata, type);
		if (ret)
			return ret;
	} else {
		/* Purge and reset type-dependent state. */
		ieee80211_teardown_sdata(sdata);
		ieee80211_setup_sdata(sdata, type);
	}

	/* reset some values that shouldn't be kept across type changes */
	if (type == NL80211_IFTYPE_STATION)
		sdata->u.mgd.use_4addr = false;

	return 0;
}

static void ieee80211_assign_perm_addr(struct ieee80211_local *local,
				       u8 *perm_addr, enum nl80211_iftype type)
{
	struct ieee80211_sub_if_data *sdata;
	u64 mask, start, addr, val, inc;
	u8 *m;
	u8 tmp_addr[ETH_ALEN];
	int i;

	lockdep_assert_wiphy(local->hw.wiphy);

	/* default ... something at least */
	memcpy(perm_addr, local->hw.wiphy->perm_addr, ETH_ALEN);

	if (is_zero_ether_addr(local->hw.wiphy->addr_mask) &&
	    local->hw.wiphy->n_addresses <= 1)
		return;

	switch (type) {
	case NL80211_IFTYPE_MONITOR:
		/* doesn't matter */
		break;
	case NL80211_IFTYPE_AP_VLAN:
		/* match up with an AP interface */
		list_for_each_entry(sdata, &local->interfaces, list) {
			if (sdata->vif.type != NL80211_IFTYPE_AP)
				continue;
			memcpy(perm_addr, sdata->vif.addr, ETH_ALEN);
			break;
		}
		/* keep default if no AP interface present */
		break;
	case NL80211_IFTYPE_P2P_CLIENT:
	case NL80211_IFTYPE_P2P_GO:
		if (ieee80211_hw_check(&local->hw, P2P_DEV_ADDR_FOR_INTF)) {
			list_for_each_entry(sdata, &local->interfaces, list) {
				if (sdata->vif.type != NL80211_IFTYPE_P2P_DEVICE)
					continue;
				if (!ieee80211_sdata_running(sdata))
					continue;
				memcpy(perm_addr, sdata->vif.addr, ETH_ALEN);
				return;
			}
		}
		fallthrough;
	default:
		/* assign a new address if possible -- try n_addresses first */
		for (i = 0; i < local->hw.wiphy->n_addresses; i++) {
			bool used = false;

			list_for_each_entry(sdata, &local->interfaces, list) {
				if (ether_addr_equal(local->hw.wiphy->addresses[i].addr,
						     sdata->vif.addr)) {
					used = true;
					break;
				}
			}

			if (!used) {
				memcpy(perm_addr,
				       local->hw.wiphy->addresses[i].addr,
				       ETH_ALEN);
				break;
			}
		}

		/* try mask if available */
		if (is_zero_ether_addr(local->hw.wiphy->addr_mask))
			break;

		m = local->hw.wiphy->addr_mask;
		mask =	((u64)m[0] << 5*8) | ((u64)m[1] << 4*8) |
			((u64)m[2] << 3*8) | ((u64)m[3] << 2*8) |
			((u64)m[4] << 1*8) | ((u64)m[5] << 0*8);

		if (__ffs64(mask) + hweight64(mask) != fls64(mask)) {
			/* not a contiguous mask ... not handled now! */
			pr_info("not contiguous\n");
			break;
		}

		/*
		 * Pick address of existing interface in case user changed
		 * MAC address manually, default to perm_addr.
		 */
		m = local->hw.wiphy->perm_addr;
		list_for_each_entry(sdata, &local->interfaces, list) {
			if (sdata->vif.type == NL80211_IFTYPE_MONITOR)
				continue;
			m = sdata->vif.addr;
			break;
		}
		start = ((u64)m[0] << 5*8) | ((u64)m[1] << 4*8) |
			((u64)m[2] << 3*8) | ((u64)m[3] << 2*8) |
			((u64)m[4] << 1*8) | ((u64)m[5] << 0*8);

		inc = 1ULL<<__ffs64(mask);
		val = (start & mask);
		addr = (start & ~mask) | (val & mask);
		do {
			bool used = false;

			tmp_addr[5] = addr >> 0*8;
			tmp_addr[4] = addr >> 1*8;
			tmp_addr[3] = addr >> 2*8;
			tmp_addr[2] = addr >> 3*8;
			tmp_addr[1] = addr >> 4*8;
			tmp_addr[0] = addr >> 5*8;

			val += inc;

			list_for_each_entry(sdata, &local->interfaces, list) {
				if (ether_addr_equal(tmp_addr, sdata->vif.addr)) {
					used = true;
					break;
				}
			}

			if (!used) {
				memcpy(perm_addr, tmp_addr, ETH_ALEN);
				break;
			}
			addr = (start & ~mask) | (val & mask);
		} while (addr != start);

		break;
	}
}

int ieee80211_if_add(struct ieee80211_local *local, const char *name,
		     unsigned char name_assign_type,
		     struct wireless_dev **new_wdev, enum nl80211_iftype type,
		     struct vif_params *params)
{
	struct net_device *ndev = NULL;
	struct ieee80211_sub_if_data *sdata = NULL;
	struct txq_info *txqi;
	int ret, i;

	ASSERT_RTNL();
	lockdep_assert_wiphy(local->hw.wiphy);

	if (type == NL80211_IFTYPE_P2P_DEVICE || type == NL80211_IFTYPE_NAN) {
		struct wireless_dev *wdev;

		sdata = kzalloc(sizeof(*sdata) + local->hw.vif_data_size,
				GFP_KERNEL);
		if (!sdata)
			return -ENOMEM;
		wdev = &sdata->wdev;

		sdata->dev = NULL;
		strscpy(sdata->name, name, IFNAMSIZ);
		ieee80211_assign_perm_addr(local, wdev->address, type);
		memcpy(sdata->vif.addr, wdev->address, ETH_ALEN);
		ether_addr_copy(sdata->vif.bss_conf.addr, sdata->vif.addr);
	} else {
		int size = ALIGN(sizeof(*sdata) + local->hw.vif_data_size,
				 sizeof(void *));
		int txq_size = 0;

		if (type != NL80211_IFTYPE_AP_VLAN &&
		    (type != NL80211_IFTYPE_MONITOR ||
		     (params->flags & MONITOR_FLAG_ACTIVE)))
			txq_size += sizeof(struct txq_info) +
				    local->hw.txq_data_size;

		ndev = alloc_netdev_mqs(size + txq_size,
					name, name_assign_type,
					ieee80211_if_setup, 1, 1);
		if (!ndev)
			return -ENOMEM;

		dev_net_set(ndev, wiphy_net(local->hw.wiphy));

		ndev->pcpu_stat_type = NETDEV_PCPU_STAT_TSTATS;

		ndev->needed_headroom = local->tx_headroom +
					4*6 /* four MAC addresses */
					+ 2 + 2 + 2 + 2 /* ctl, dur, seq, qos */
					+ 6 /* mesh */
					+ 8 /* rfc1042/bridge tunnel */
					- ETH_HLEN /* ethernet hard_header_len */
					+ IEEE80211_ENCRYPT_HEADROOM;
		ndev->needed_tailroom = IEEE80211_ENCRYPT_TAILROOM;

		ret = dev_alloc_name(ndev, ndev->name);
		if (ret < 0) {
			free_netdev(ndev);
			return ret;
		}

		ieee80211_assign_perm_addr(local, ndev->perm_addr, type);
		if (is_valid_ether_addr(params->macaddr))
			eth_hw_addr_set(ndev, params->macaddr);
		else
			eth_hw_addr_set(ndev, ndev->perm_addr);
		SET_NETDEV_DEV(ndev, wiphy_dev(local->hw.wiphy));

		/* don't use IEEE80211_DEV_TO_SUB_IF -- it checks too much */
		sdata = netdev_priv(ndev);
		ndev->ieee80211_ptr = &sdata->wdev;
		memcpy(sdata->vif.addr, ndev->dev_addr, ETH_ALEN);
		ether_addr_copy(sdata->vif.bss_conf.addr, sdata->vif.addr);
		memcpy(sdata->name, ndev->name, IFNAMSIZ);

		if (txq_size) {
			txqi = netdev_priv(ndev) + size;
			ieee80211_txq_init(sdata, NULL, txqi, 0);
		}

		sdata->dev = ndev;
	}

	/* initialise type-independent data */
	sdata->wdev.wiphy = local->hw.wiphy;

	ieee80211_sdata_init(local, sdata);

	ieee80211_init_frag_cache(&sdata->frags);

	wiphy_delayed_work_init(&sdata->dec_tailroom_needed_wk,
				ieee80211_delayed_tailroom_dec);

	for (i = 0; i < NUM_NL80211_BANDS; i++) {
		struct ieee80211_supported_band *sband;
		sband = local->hw.wiphy->bands[i];
		sdata->rc_rateidx_mask[i] =
			sband ? (1 << sband->n_bitrates) - 1 : 0;
		if (sband) {
			__le16 cap;
			u16 *vht_rate_mask;

			memcpy(sdata->rc_rateidx_mcs_mask[i],
			       sband->ht_cap.mcs.rx_mask,
			       sizeof(sdata->rc_rateidx_mcs_mask[i]));

			cap = sband->vht_cap.vht_mcs.rx_mcs_map;
			vht_rate_mask = sdata->rc_rateidx_vht_mcs_mask[i];
			ieee80211_get_vht_mask_from_cap(cap, vht_rate_mask);
		} else {
			memset(sdata->rc_rateidx_mcs_mask[i], 0,
			       sizeof(sdata->rc_rateidx_mcs_mask[i]));
			memset(sdata->rc_rateidx_vht_mcs_mask[i], 0,
			       sizeof(sdata->rc_rateidx_vht_mcs_mask[i]));
		}
	}

	ieee80211_set_default_queues(sdata);

	/* setup type-dependent data */
	ieee80211_setup_sdata(sdata, type);

	if (ndev) {
		ndev->ieee80211_ptr->use_4addr = params->use_4addr;
		if (type == NL80211_IFTYPE_STATION)
			sdata->u.mgd.use_4addr = params->use_4addr;

		ndev->features |= local->hw.netdev_features;
		ndev->priv_flags |= IFF_LIVE_ADDR_CHANGE;
		ndev->hw_features |= ndev->features &
					MAC80211_SUPPORTED_FEATURES_TX;
		sdata->vif.netdev_features = local->hw.netdev_features;

		netdev_set_default_ethtool_ops(ndev, &ieee80211_ethtool_ops);

		/* MTU range is normally 256 - 2304, where the upper limit is
		 * the maximum MSDU size. Monitor interfaces send and receive
		 * MPDU and A-MSDU frames which may be much larger so we do
		 * not impose an upper limit in that case.
		 */
		ndev->min_mtu = 256;
		if (type == NL80211_IFTYPE_MONITOR)
			ndev->max_mtu = 0;
		else
			ndev->max_mtu = local->hw.max_mtu;

		ret = cfg80211_register_netdevice(ndev);
		if (ret) {
			free_netdev(ndev);
			return ret;
		}
	}

	mutex_lock(&local->iflist_mtx);
	list_add_tail_rcu(&sdata->list, &local->interfaces);
	mutex_unlock(&local->iflist_mtx);

	if (new_wdev)
		*new_wdev = &sdata->wdev;

	return 0;
}

void ieee80211_if_remove(struct ieee80211_sub_if_data *sdata)
{
	ASSERT_RTNL();
	lockdep_assert_wiphy(sdata->local->hw.wiphy);

	mutex_lock(&sdata->local->iflist_mtx);
	list_del_rcu(&sdata->list);
	mutex_unlock(&sdata->local->iflist_mtx);

	if (sdata->vif.txq)
		ieee80211_txq_purge(sdata->local, to_txq_info(sdata->vif.txq));

	synchronize_rcu();

	cfg80211_unregister_wdev(&sdata->wdev);

	if (!sdata->dev) {
		ieee80211_teardown_sdata(sdata);
		kfree(sdata);
	}
}

void ieee80211_sdata_stop(struct ieee80211_sub_if_data *sdata)
{
	if (WARN_ON_ONCE(!test_bit(SDATA_STATE_RUNNING, &sdata->state)))
		return;
	ieee80211_do_stop(sdata, true);
}

void ieee80211_remove_interfaces(struct ieee80211_local *local)
{
	struct ieee80211_sub_if_data *sdata, *tmp;
	LIST_HEAD(unreg_list);

	ASSERT_RTNL();

	/* Before destroying the interfaces, make sure they're all stopped so
	 * that the hardware is stopped. Otherwise, the driver might still be
	 * iterating the interfaces during the shutdown, e.g. from a worker
	 * or from RX processing or similar, and if it does so (using atomic
	 * iteration) while we're manipulating the list, the iteration will
	 * crash.
	 *
	 * After this, the hardware should be stopped and the driver should
	 * have stopped all of its activities, so that we can do RCU-unaware
	 * manipulations of the interface list below.
	 */
	cfg80211_shutdown_all_interfaces(local->hw.wiphy);

	guard(wiphy)(local->hw.wiphy);

	WARN(local->open_count, "%s: open count remains %d\n",
	     wiphy_name(local->hw.wiphy), local->open_count);

	mutex_lock(&local->iflist_mtx);
	list_splice_init(&local->interfaces, &unreg_list);
	mutex_unlock(&local->iflist_mtx);

	list_for_each_entry_safe(sdata, tmp, &unreg_list, list) {
		bool netdev = sdata->dev;

		/*
		 * Remove IP addresses explicitly, since the notifier will
		 * skip the callbacks if wdev->registered is false, since
		 * we can't acquire the wiphy_lock() again there if already
		 * inside this locked section.
		 */
		sdata->vif.cfg.arp_addr_cnt = 0;
		if (sdata->vif.type == NL80211_IFTYPE_STATION &&
		    sdata->u.mgd.associated)
			ieee80211_vif_cfg_change_notify(sdata,
							BSS_CHANGED_ARP_FILTER);

		list_del(&sdata->list);
		cfg80211_unregister_wdev(&sdata->wdev);

		if (!netdev)
			kfree(sdata);
	}
}

static int netdev_notify(struct notifier_block *nb,
			 unsigned long state, void *ptr)
{
	struct net_device *dev = netdev_notifier_info_to_dev(ptr);
	struct ieee80211_sub_if_data *sdata;

	if (state != NETDEV_CHANGENAME)
		return NOTIFY_DONE;

	if (!dev->ieee80211_ptr || !dev->ieee80211_ptr->wiphy)
		return NOTIFY_DONE;

	if (dev->ieee80211_ptr->wiphy->privid != mac80211_wiphy_privid)
		return NOTIFY_DONE;

	sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	memcpy(sdata->name, dev->name, IFNAMSIZ);
	ieee80211_debugfs_rename_netdev(sdata);

	return NOTIFY_OK;
}

static struct notifier_block mac80211_netdev_notifier = {
	.notifier_call = netdev_notify,
};

int ieee80211_iface_init(void)
{
	return register_netdevice_notifier(&mac80211_netdev_notifier);
}

void ieee80211_iface_exit(void)
{
	unregister_netdevice_notifier(&mac80211_netdev_notifier);
}

void ieee80211_vif_inc_num_mcast(struct ieee80211_sub_if_data *sdata)
{
	if (sdata->vif.type == NL80211_IFTYPE_AP)
		atomic_inc(&sdata->u.ap.num_mcast_sta);
	else if (sdata->vif.type == NL80211_IFTYPE_AP_VLAN)
		atomic_inc(&sdata->u.vlan.num_mcast_sta);
}

void ieee80211_vif_dec_num_mcast(struct ieee80211_sub_if_data *sdata)
{
	if (sdata->vif.type == NL80211_IFTYPE_AP)
		atomic_dec(&sdata->u.ap.num_mcast_sta);
	else if (sdata->vif.type == NL80211_IFTYPE_AP_VLAN)
		atomic_dec(&sdata->u.vlan.num_mcast_sta);
}

void ieee80211_vif_block_queues_csa(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_local *local = sdata->local;

	if (ieee80211_hw_check(&local->hw, HANDLES_QUIET_CSA))
		return;

	ieee80211_stop_vif_queues_norefcount(local, sdata,
					     IEEE80211_QUEUE_STOP_REASON_CSA);
}

void ieee80211_vif_unblock_queues_csa(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_local *local = sdata->local;

	ieee80211_wake_vif_queues_norefcount(local, sdata,
					     IEEE80211_QUEUE_STOP_REASON_CSA);
}
