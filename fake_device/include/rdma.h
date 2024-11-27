#ifndef FAKE_DEVICE_RDMA_H
#define FAKE_DEVICE_RDMA_H

#include "list.h"

#include <errno.h>
#include <getopt.h>
#include <libgen.h>
#include <libmnl/libmnl.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "mnl_utils.h"
#include "linux/netlink.h"

#include "rdma/rdma_netlink.h"
#include "rdma/rdma_user_cm.h"

#define pr_err( args... ) fprintf( stderr, ##args )
#define pr_out( args... ) fprintf( stdout, ##args )

struct filter_entry {
	struct list_head list;
	char*            key;
	char*            value;
	/*
   * This field means that we can try to issue .doit callback
   * on value above. This value can be converted to integer
   * with simple atoi(). Otherwise "is_doit" will be false.
   */
	uint8_t is_doit : 1;
};

struct dev_map {
	struct list_head list;
	char*            dev_name;
	uint32_t         num_ports;
	uint32_t         idx;
};

typedef struct rdma_device {
	int                argc;
	char**             argv;
	char*              filename;
	struct list_head   dev_map_list;
	uint32_t           dev_idx;
	uint32_t           port_idx;
	struct mnl_socket* nl;
	struct nlmsghdr*   nlh;
	char*              buff;
	struct list_head   filter_list;
	char*              link_name;
	char*              link_type;
	char*              dev_name;
	char*              dev_type;
} rd;

void rd_free( struct rdma_device* rd );

int  rd_send_msg( struct rdma_device* rd );
int  rd_recv_msg( struct rdma_device* rd, mnl_cb_t callback, void* data, uint32_t seq );
void rd_prepare_msg( struct rdma_device* rd, uint32_t cmd, uint32_t* seq, uint16_t flags );
int  rd_dev_init_cb( const struct nlmsghdr* nlh, void* data );
int  rd_attr_cb( const struct nlattr* attr, void* data );

#endif//! _FAKE_DEVICE_RDMA_H_
