/* Copyright (c) 2013, Linaro Limited
 * All rights reserved.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 */


/**
 * @file
 *
 * ODP packet IO - implementation internal
 */

#ifndef ODP_PACKET_IO_INTERNAL_H_
#define ODP_PACKET_IO_INTERNAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <odp/spinlock.h>
#include <odp/ticketlock.h>
#include <odp_packet_socket.h>
#include <odp_classification_datamodel.h>
#include <odp_align_internal.h>
#include <odp_debug_internal.h>

#include <odp/config.h>
#include <odp/hints.h>

#include <odp_packet_dpdk.h>

#define PKTIO_NAME_LEN 256

/* Forward declaration */
struct pktio_if_ops;
struct pkt_dpdk_t;

typedef struct {
	odp_queue_t loopq;		/**< loopback queue for "loop" device */
	odp_bool_t promisc;		/**< promiscuous mode state */
} pkt_loop_t;

/** Packet socket using dpdk mmaped rings for both Rx and Tx */
typedef struct {
	odp_pool_t pool;

	/********************************/
	char ifname[32];
	uint8_t min_rx_burst;
	uint8_t portid;
	uint16_t queueid;
	odp_bool_t vdev_sysc_promisc;	/**< promiscuous mode defined with
					    system call */
} pkt_dpdk_t;

struct pktio_entry {
	const struct pktio_if_ops *ops; /**< Implementation specific methods */
	/* These two locks together lock the whole pktio device */
	odp_ticketlock_t rxl;		/**< RX ticketlock */
	odp_ticketlock_t txl;		/**< TX ticketlock */
	int taken;			/**< is entry taken(1) or free(0) */
	int cls_enabled;		/**< is classifier enabled */
	odp_pktio_t handle;		/**< pktio handle */
	odp_queue_t inq_default;	/**< default input queue, if set */
	odp_queue_t outq_default;	/**< default out queue */
	union {
		pkt_loop_t pkt_loop;	/**< Using loopback for IO */
		pkt_dpdk_t pkt_dpdk;	/**< using DPDK API for IO */
	};
	enum {
		STATE_START = 0,
		STATE_STOP
	} state;
	classifier_t cls;		/**< classifier linked with this pktio*/
	char name[PKTIO_NAME_LEN];	/**< name of pktio provided to
					   pktio_open() */
	odp_pktio_t id;
	odp_pktio_param_t param;
};

typedef union {
	struct pktio_entry s;
	uint8_t pad[ODP_CACHE_LINE_SIZE_ROUNDUP(sizeof(struct pktio_entry))];
} pktio_entry_t;

typedef struct {
	odp_spinlock_t lock;
	pktio_entry_t entries[ODP_CONFIG_PKTIO_ENTRIES];
} pktio_table_t;

int is_free(pktio_entry_t *entry);

typedef struct pktio_if_ops {
	int (*init)(void);
	int (*term)(void);
	int (*open)(odp_pktio_t pktio, pktio_entry_t *pktio_entry,
		    const char *devname, odp_pool_t pool);
	int (*close)(pktio_entry_t *pktio_entry);
	int (*start)(pktio_entry_t *pktio_entry);
	int (*stop)(pktio_entry_t *pktio_entry);
	int (*recv)(pktio_entry_t *pktio_entry, odp_packet_t pkt_table[],
		    unsigned len);
	int (*send)(pktio_entry_t *pktio_entry, odp_packet_t pkt_table[],
		    unsigned len);
	int (*mtu_get)(pktio_entry_t *pktio_entry);
	int (*promisc_mode_set)(pktio_entry_t *pktio_entry,  int enable);
	int (*promisc_mode_get)(pktio_entry_t *pktio_entry);
	int (*mac_get)(pktio_entry_t *pktio_entry, void *mac_addr);
} pktio_if_ops_t;

int _odp_packet_cls_enq(pktio_entry_t *pktio_entry, const uint8_t *base,
			uint16_t buf_len, odp_packet_t *pkt_ret);

extern void *pktio_entry_ptr[];

static inline int pktio_to_id(odp_pktio_t pktio)
{
	return _odp_typeval(pktio) - 1;
}

static inline pktio_entry_t *get_pktio_entry(odp_pktio_t pktio)
{
	if (odp_unlikely(pktio == ODP_PKTIO_INVALID))
		return NULL;

	if (odp_unlikely(_odp_typeval(pktio) > ODP_CONFIG_PKTIO_ENTRIES)) {
		ODP_DBG("pktio limit %d/%d exceed\n",
			_odp_typeval(pktio), ODP_CONFIG_PKTIO_ENTRIES);
		return NULL;
	}

	return pktio_entry_ptr[pktio_to_id(pktio)];
}

static inline int pktio_cls_enabled(pktio_entry_t *entry)
{
	return entry->s.cls_enabled;
}

static inline void pktio_cls_enabled_set(pktio_entry_t *entry, int ena)
{
	entry->s.cls_enabled = ena;
}

int pktin_poll(pktio_entry_t *entry);

extern const pktio_if_ops_t loopback_pktio_ops;
extern const pktio_if_ops_t dpdk_pktio_ops;
extern const pktio_if_ops_t * const pktio_if_ops[];

#ifdef __cplusplus
}
#endif

#endif