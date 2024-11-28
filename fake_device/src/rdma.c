#include "rdma.h"
#include <libmnl/libmnl.h>
#include <stdint.h>
#include <stdlib.h>

static int rd_init( struct rdma_device* rd, char* filename ) {
  uint32_t seq;
  int      ret;

  rd->filename = filename;
  INIT_LIST_HEAD( &rd->dev_map_list );
  INIT_LIST_HEAD( &rd->filter_list );

  rd->buff = (char*) malloc( MNL_SOCKET_BUFFER_SIZE );
  if ( !rd->buff ) return -ENOMEM;

  rd_prepare_msg( rd, RDMA_NLDEV_CMD_GET, &seq,
                  ( NLM_F_REQUEST | NLM_F_ACK | NLM_F_DUMP ) );

  ret = rd_send_msg( rd );
  if ( ret ) return ret;

  return rd_recv_msg( rd, rd_dev_init_cb, rd, seq );
}

static void rd_cleanup( struct rdma_device* rd ) {}

int main( int argc, char* argv[] ) {
  struct rdma_device rd = {};
  char*              filename;
  int                err;

  filename = basename( argv[ 0 ] );

  err = rd_init( &rd, filename );
  if ( err ) goto out;

out:
  rd_cleanup( &rd );

  return err ? EXIT_FAILURE : EXIT_SUCCESS;
}
