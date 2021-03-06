/*
 * Copyright (c) 2012-2018 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

#ifndef WLAN_QCT_WLANSAP_INTERNAL_H
#define WLAN_QCT_WLANSAP_INTERNAL_H

/*
 * This file contains the internal API exposed by the wlan SAP PAL layer
 * module.
 */

#include "cds_api.h"
#include "cds_packet.h"

/* Pick up the CSR API definitions */
#include "csr_api.h"
#include "sap_api.h"
#include "sap_fsm_ext.h"
#include "sap_ch_select.h"

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------
 *  Defines
 * -------------------------------------------------------------------------*/
/* DFS Non Occupancy Period =30 minutes, in microseconds */
#define SAP_DFS_NON_OCCUPANCY_PERIOD      (30 * 60 * 1000 * 1000)

#define SAP_DEBUG
/* Used to enable or disable security on the BT-AMP link */
#define WLANSAP_SECURITY_ENABLED_STATE true

/* When MBSSID feature is enabled, SAP context is directly passed to SAP APIs */
#define CDS_GET_SAP_CB(ctx) (ptSapContext)(ctx)

#define CDS_GET_HAL_CB(ctx) cds_get_context(QDF_MODULE_ID_PE)
/* MAC Address length */
#define ANI_EAPOL_KEY_RSN_NONCE_SIZE      32

#define IS_ETSI_WEATHER_CH(_ch)   ((_ch >= 120) && (_ch <= 130))
#define IS_CH_BONDING_WITH_WEATHER_CH(_ch)   (_ch == 116)
#define IS_CHAN_JAPAN_W53(_ch)    ((_ch >= 52)  && (_ch <= 64))
#define IS_CHAN_JAPAN_INDOOR(_ch) ((_ch >= 36)  && (_ch <= 64))
#define IS_CHAN_JAPAN_OUTDOOR(_ch)((_ch >= 100) && (_ch <= 140))
#define DEFAULT_CAC_TIMEOUT (60 * 1000) /* msecs - 1 min */
#define ETSI_WEATHER_CH_CAC_TIMEOUT (10 * 60 * 1000)    /* msecs - 10 min */
#define SAP_CHAN_PREFERRED_INDOOR  1
#define SAP_CHAN_PREFERRED_OUTDOOR 2

/*----------------------------------------------------------------------------
 *  Typedefs
 * -------------------------------------------------------------------------*/
typedef struct sSapContext tSapContext;
/* tSapContext, *ptSapContext; */
/*----------------------------------------------------------------------------
 *  Type Declarations - For internal SAP context information
 * -------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 *  Opaque SAP context Type Declaration
 * -------------------------------------------------------------------------*/
/* We were only using this syntax, when this was truly opaque. */
/* (I.E., it was defined in a different file.) */

/* SAP FSM states for Access Point role */
typedef enum {
	eSAP_DISCONNECTED,
	eSAP_CH_SELECT,
	eSAP_DFS_CAC_WAIT,
	eSAP_STARTING,
	eSAP_STARTED,
	eSAP_DISCONNECTING
} eSapFsmStates_t;

/*----------------------------------------------------------------------------
 *  SAP context Data Type Declaration
 * -------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 *  Type Declarations - QOS related
 * -------------------------------------------------------------------------*/
/* SAP QOS config */
typedef struct sSapQosCfg {
	uint8_t WmmIsEnabled;
} tSapQosCfg;

typedef struct sSapAcsChannelInfo {
	uint32_t channelNum;
	uint32_t weight;
} tSapAcsChannelInfo;

#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
/*
 * In a setup having two MDM both operating in AP+AP MCC scenario
 * if both the AP decides to use same or close channel set, CTS to
 * self, mechanism is causing issues with connectivity. For this, its
 * proposed that 2nd MDM devices which comes up later should detect
 * presence of first MDM device via special Q2Q IE present in becon
 * and avoid those channels mentioned in IE.
 *
 * Following struct will keep this info in sapCtx struct, and will be used
 * to avoid such channels in Random Channel Select in case of radar ind.
 */
struct sap_avoid_channels_info {
	bool       present;
	uint8_t    channels[WNI_CFG_VALID_CHANNEL_LIST_LEN];
};
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */

typedef struct sSapContext {

	qdf_mutex_t SapGlobalLock;

	/* Include the current channel of AP */
	uint32_t channel;
	uint32_t secondary_ch;

	/* Include the SME(CSR) sessionId here */
	uint8_t sessionId;

	/* Include the key material for this physical link */
	uint8_t key_type;
	uint8_t key_length;
	uint8_t key_material[32];

	/* Include the associations MAC addresses */
	uint8_t self_mac_addr[CDS_MAC_ADDRESS_LEN];

	/* Own SSID */
	uint8_t ownSsid[MAX_SSID_LEN];
	uint32_t ownSsidLen;

	/* Flag for signaling if security is enabled */
	uint8_t ucSecEnabled;

	/* Include the SME(CSR) context here */
	tCsrRoamProfile csr_roamProfile;
	uint32_t csr_roamId;

	/* Sap session */
	bool isSapSessionOpen;

	/* SAP event Callback to hdd */
	tpWLAN_SAPEventCB pfnSapEventCallback;

	/* Include the enclosing CDS context here */
	void *p_cds_gctx;

	/*
	 * Include the state machine structure here, state var that keeps
	 * track of state machine
	 */
	eSapFsmStates_t sapsMachine;

	/* Actual storage for AP and self (STA) SSID */
	tCsrSSIDInfo SSIDList[2];

	/* Actual storage for AP bssid */
	struct qdf_mac_addr bssid;

	/* Mac filtering settings */
	eSapMacAddrACL eSapMacAddrAclMode;
	struct qdf_mac_addr acceptMacList[MAX_ACL_MAC_ADDRESS];
	uint8_t nAcceptMac;
	struct qdf_mac_addr denyMacList[MAX_ACL_MAC_ADDRESS];
	uint8_t nDenyMac;

	/* QOS config */
	tSapQosCfg SapQosCfg;

	void *pUsrContext;

	uint32_t nStaWPARSnReqIeLength;
	uint8_t pStaWpaRsnReqIE[MAX_ASSOC_IND_IE_LEN];
	tSirAPWPSIEs APWPSIEs;
	tSirRSNie APWPARSNIEs;

#ifdef FEATURE_WLAN_WAPI
	uint32_t nStaWAPIReqIeLength;
	uint8_t pStaWapiReqIE[MAX_ASSOC_IND_IE_LEN];
#endif

	uint32_t nStaAddIeLength;
	uint8_t pStaAddIE[MAX_ASSOC_IND_IE_LEN];
	uint8_t *channelList;
	uint8_t num_of_channel;
	tSapChannelListInfo SapChnlList;
	uint16_t ch_width_orig;
	struct ch_params_s ch_params;

	/* session to scan */
	bool isScanSessionOpen;
	/*
	 * This list of channels will hold 5Ghz enabled,DFS in the
	 * Current RegDomain.This list will be used to select a channel,
	 * for SAP to start including any DFS channel and also to select
	 * any random channel[5Ghz-(NON-DFS/DFS)],if SAP is operating
	 * on a DFS channel and a RADAR is detected on the channel.
	 */
	tAll5GChannelList SapAllChnlList;
	uint32_t auto_channel_select_weight;
	tSapAcsChannelInfo acsBestChannelInfo;
	bool enableOverLapCh;
	struct sap_acs_cfg *acs_cfg;

#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
	uint8_t cc_switch_mode;
#endif

#if defined(FEATURE_WLAN_STA_AP_MODE_DFS_DISABLE)
	bool dfs_ch_disable;
#endif
	bool isCacEndNotified;
	bool isCacStartNotified;
	bool is_sap_ready_for_chnl_chng;

#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
	/*
	 * In a setup having two MDM both operating in AP+AP MCC scenario
	 * if both the AP decides to use same or close channel set, CTS to
	 * self, mechanism is causing issues with connectivity. For this, its
	 * proposed that 2nd MDM devices which comes up later should detect
	 * presence of first MDM device via special Q2Q IE present in becon
	 * and avoid those channels mentioned in IE.
	 *
	 * this struct contains the list of channels on which another MDM AP
	 * in MCC mode were detected.
	 */
	struct sap_avoid_channels_info sap_detected_avoid_ch_ie;
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */
	/*
	 * sap_state, sap_status are created
	 * to inform upper layers about ACS scan status.
	 * Don't use these members for anyother purposes.
	 */
	eSapHddEvent sap_state;
	eSapStatus sap_status;
	uint32_t roc_ind_scan_id;

	qdf_event_t sap_session_opened_evt;
	bool is_pre_cac_on;
	bool pre_cac_complete;
	uint8_t chan_before_pre_cac;
	uint16_t beacon_tx_rate;
	tSirMacRateSet supp_rate_set;
	tSirMacRateSet extended_rate_set;
	enum sap_acs_dfs_mode dfs_mode;
	uint8_t sap_sta_id;
	bool is_chan_change_inprogress;
	bool enable_etsi_srd_chan_support;
	bool stop_bss_in_progress;
} *ptSapContext;

/*----------------------------------------------------------------------------
 *  External declarations for global context
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 *  SAP state machine event definition
 * -------------------------------------------------------------------------*/
/* The event structure */
typedef struct sWLAN_SAPEvent {
	/* A VOID pointer type for all possible inputs */
	void *params;
	/* State machine input event message */
	uint32_t event;
	/* introduced to handle csr_roam_completeCallback roamStatus */
	uint32_t u1;
	/* introduced to handle csr_roam_completeCallback roamResult */
	uint32_t u2;
} tWLAN_SAPEvent, *ptWLAN_SAPEvent;

/*----------------------------------------------------------------------------
 * Function Declarations and Documentation
 * -------------------------------------------------------------------------*/
QDF_STATUS wlansap_context_get(ptSapContext ctx);
void wlansap_context_put(ptSapContext ctx);

QDF_STATUS
wlansap_scan_callback
	(tHalHandle halHandle,
	void *pContext,
	uint8_t sessionId, uint32_t scanID, eCsrScanStatus scanStatus);

QDF_STATUS
wlansap_pre_start_bss_acs_scan_callback(
	tHalHandle hal_handle,
	void *pcontext,
	uint8_t sessionid,
	uint32_t scanid,
	eCsrScanStatus scan_status
);

QDF_STATUS
wlansap_roam_callback
	(void *pContext,
	tCsrRoamInfo *pCsrRoamInfo,
	uint32_t roamId,
	eRoamCmdStatus roamStatus, eCsrRoamResult roamResult);

QDF_STATUS wlansap_clean_cb(ptSapContext pSapCtx, uint32_t freeFlag);
QDF_STATUS SapFsm(ptSapContext sapContext, ptWLAN_SAPEvent sapEvent,
			 uint8_t *status);

void
wlansap_pmc_full_pwr_req_cb(void *callbackContext, QDF_STATUS status);

uint8_t sap_select_channel(tHalHandle halHandle, ptSapContext pSapCtx,
			   tScanResultHandle pScanResult);

QDF_STATUS
sap_signal_hdd_event(ptSapContext sapContext,
		  tCsrRoamInfo *pCsrRoamInfo,
		  eSapHddEvent sapHddevent, void *);

QDF_STATUS sap_fsm(ptSapContext sapContext, ptWLAN_SAPEvent sapEvent);

eSapStatus
sapconvert_to_csr_profile(tsap_Config_t *pconfig_params,
		       eCsrRoamBssType bssType,
		       tCsrRoamProfile *profile);

void sap_free_roam_profile(tCsrRoamProfile *profile);

QDF_STATUS
sap_is_peer_mac_allowed(ptSapContext sapContext, uint8_t *peerMac);

void
sap_sort_mac_list(struct qdf_mac_addr *macList, uint8_t size);

void
sap_add_mac_to_acl(struct qdf_mac_addr *macList, uint8_t *size,
	       uint8_t *peerMac);

void
sap_remove_mac_from_acl(struct qdf_mac_addr *macList, uint8_t *size,
		    uint8_t index);

void
sap_print_acl(struct qdf_mac_addr *macList, uint8_t size);

bool
sap_search_mac_list(struct qdf_mac_addr *macList, uint8_t num_mac,
		 uint8_t *peerMac, uint8_t *index);

QDF_STATUS sap_acquire_global_lock(ptSapContext pSapCtx);

QDF_STATUS sap_release_global_lock(ptSapContext pSapCtx);

#ifdef FEATURE_WLAN_CH_AVOID
void sap_update_unsafe_channel_list(ptSapContext pSapCtx);
#endif /* FEATURE_WLAN_CH_AVOID */

uint8_t
sap_indicate_radar(ptSapContext sapContext,
		 tSirSmeDfsEventInd *dfs_event);

QDF_STATUS sap_init_dfs_channel_nol_list(ptSapContext sapContext);

bool sap_dfs_is_channel_in_nol_list(ptSapContext sapContext,
				    uint8_t channelNumber,
				    ePhyChanBondState chanBondState);
void sap_dfs_cac_timer_callback(void *data);

void sap_cac_reset_notify(tHalHandle hHal);

bool
sap_channel_matrix_check(ptSapContext sapContext,
			 ePhyChanBondState cbMode,
			 uint8_t target_channel);

bool is_concurrent_sap_ready_for_channel_change(tHalHandle hHal,
			ptSapContext sapContext);
bool sap_is_conc_sap_doing_scc_dfs(tHalHandle hal,
			ptSapContext given_sapctx);
uint8_t sap_get_total_number_sap_intf(tHalHandle hHal);

bool sap_dfs_is_w53_invalid(tHalHandle hHal, uint8_t channelID);

bool sap_dfs_is_channel_in_preferred_location(tHalHandle hHal,
					      uint8_t channelID);

QDF_STATUS sap_goto_channel_sel(
	ptSapContext sapContext,
	ptWLAN_SAPEvent sapEvent,
	bool sap_do_acs_pre_start_bss,
	bool check_for_connection_update);

void sap_config_acs_result(tHalHandle hal, ptSapContext sap_ctx,
							uint32_t sec_ch);
/**
 * sap_check_in_avoid_ch_list() - checks if given channel present is channel
 * avoidance list
 * avoid_channels_info struct
 * @sap_ctx:        sap context.
 * @channel:        channel to be checked in sap_ctx's avoid ch list
 *
 * sap_ctx contains sap_avoid_ch_info strcut containing the list of channels on
 * which MDM device's AP with MCC was detected. This function checks if given
 * channel is present in that list.
 *
 * Return: true, if channel was present, false othersie.
 */
bool
sap_check_in_avoid_ch_list(ptSapContext sap_ctx, uint8_t channel);
QDF_STATUS sap_open_session(tHalHandle hHal, ptSapContext sapContext,
				uint32_t *session_id);
QDF_STATUS sap_close_session(tHalHandle hHal,
			     ptSapContext sapContext,
			     csr_roamSessionCloseCallback callback, bool valid);

/**
 * sap_select_default_oper_chan() - Select AP mode default operating channel
 * @acs_cfg: pointer to ACS config info
 *
 * Select AP mode default operating channel based on ACS hw mode and channel
 * range configuration when ACS scan fails due to some reasons, such as scan
 * timeout, etc.
 *
 * Return: Selected operating channel number
 */
uint8_t sap_select_default_oper_chan(struct sap_acs_cfg *acs_cfg);

/**
 * sap_channel_in_acs_channel_list() - check if channel in acs channel list
 * @channel_num: channel to check
 * @sap_ctx: struct ptSapContext
 * @spect_info_params: strcut tSapChSelSpectInfo
 *
 * This function checks if specified channel is in the configured ACS channel
 * list.
 *
 * Return: channel number if in acs channel list or SAP_CHANNEL_NOT_SELECTED
 */
uint8_t sap_channel_in_acs_channel_list(uint8_t channel_num,
					ptSapContext sap_ctx,
					tSapChSelSpectInfo *spect_info_params);

#ifdef __cplusplus
}
#endif
#endif
