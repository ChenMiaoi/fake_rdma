#ifndef FAKE_RDMA_LIBMNL_UTILS_H
#define FAKE_RDMA_LIBMNL_UTILS_H

#include "libmnl/libmnl.h"
#include "uapi/linux/genetlink.h"
#include "uapi/linux/netlink.h"
#include "whole_utils.h"

#include <errno.h>
#include <stdint.h>
#include <string.h>

struct mnlu_gen_socket {
	struct mnl_socket* nl;
	char*              buf;
	uint32_t           family;
	uint32_t           maxattr;
	unsigned int       seq;
	uint8_t            version;
};

int  mnlu_gen_socket_open( struct mnlu_gen_socket* nlg,
													 const char*             family_name,
													 uint8_t                 version );
void mnlu_gen_socket_close( struct mnlu_gen_socket* nlg );

struct mnl_socket* mnlu_socket_open( int bus );

int mnlu_socket_recv_run( struct mnl_socket* nl,
													unsigned int       seq,
													void*              buf,
													size_t             buf_size,
													mnl_cb_t           cb,
													void*              data );

#endif//! FAKE_RDMA_LIBMNL_UTILS_H
