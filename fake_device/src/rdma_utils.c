#include "rdma.h"

#include <libmnl/libmnl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static const enum mnl_attr_data_type nldev_policy[ RDMA_NLDEV_ATTR_MAX ] = {
  [RDMA_NLDEV_ATTR_DEV_INDEX]  = MNL_TYPE_U32,
  [RDMA_NLDEV_ATTR_DEV_NAME]   = MNL_TYPE_NUL_STRING,
  [RDMA_NLDEV_ATTR_PORT_INDEX] = MNL_TYPE_U32,
};

static void filters_cleanup( struct rdma_device* rd ) {
  struct filter_entry *fe, *tmp;

  list_for_each_entry_safe( fe, tmp, &rd->filter_list, list ) {
    list_del( &fe->list );
    free( fe->key );
    free( fe->value );
    free( fe );
  }
}

static struct dev_map* dev_map_alloc( const char* dev_name ) {
  struct dev_map* dev_map;

  dev_map = calloc( 1, sizeof( *dev_map ) );
  if ( !dev_map )
    return NULL;
  dev_map->dev_name = strdup( dev_name );
  if ( !dev_map->dev_name ) {
    free( dev_map );
    return NULL;
  }

  return dev_map;
}

static void dev_map_cleanup( struct rdma_device* rd ) {
  struct dev_map *dev_map, *tmp;

  list_for_each_entry_safe( dev_map, tmp, &rd->dev_map_list, list ) {
    list_del( &dev_map->list );
    free( dev_map->dev_name );
    free( dev_map );
  }
}

int rd_attr_cb( const struct nlattr* attr, void* data ) {
  const struct nlattr** tb = data;
  int                   type;

  if ( mnl_attr_type_valid( attr, RDMA_NLDEV_ATTR_MAX - 1 ) < 0 )
    /* We received unknown attribute */
    return MNL_CB_OK;

  type = mnl_attr_get_type( attr );

  if ( mnl_attr_validate( attr, nldev_policy[ type ] ) < 0 )
    return MNL_CB_ERROR;

  tb[ type ] = attr;
  return MNL_CB_OK;
}

int rd_dev_init_cb( const struct nlmsghdr* nlh, void* data ) {
  struct nlattr*      tb[ RDMA_NLDEV_ATTR_MAX ] = {};
  struct dev_map*     dev_map;
  struct rdma_device* rd = (struct rdma_device*) data;
  const char*         dev_name;

  mnl_attr_parse( nlh, 0, rd_attr_cb, tb );
  if ( !tb[ RDMA_NLDEV_ATTR_DEV_NAME ] || !tb[ RDMA_NLDEV_ATTR_DEV_INDEX ] )
    return MNL_CB_ERROR;

  if ( !tb[ RDMA_NLDEV_ATTR_PORT_INDEX ] ) {
    pr_err( "This tool doesn't support switches yet\n" );
    return MNL_CB_ERROR;
  }

  dev_name = mnl_attr_get_str( tb[ RDMA_NLDEV_ATTR_DEV_NAME ] );

  dev_map = dev_map_alloc( dev_name );
  if ( !dev_map )
    /* The main function will cleanup the allocations */
    return MNL_CB_ERROR;
  list_add_tail( &dev_map->list, &rd->dev_map_list );

  dev_map->num_ports = mnl_attr_get_u32( tb[ RDMA_NLDEV_ATTR_PORT_INDEX ] );
  dev_map->idx       = mnl_attr_get_u32( tb[ RDMA_NLDEV_ATTR_DEV_INDEX ] );

  return MNL_CB_OK;
}

int rd_send_msg( struct rdma_device* rd ) {
  int ret;

  rd->nl = mnlu_socket_open( NETLINK_RDMA );
  if ( !rd->nl ) {
    pr_err( "Failed to open NETLINK_RDMA socket\n" );
    return -ENODEV;
  }

  ret = mnl_socket_sendto( rd->nl, rd->nlh, rd->nlh->nlmsg_len );
  if ( ret < 0 ) {
    pr_err( "Failed to send to socket with err %d\n", ret );
    goto err;
  }
  return 0;

err:
  return ret;
}

int rd_recv_msg( struct rdma_device* rd, mnl_cb_t callback, void* data, unsigned int seq ) {
  char buf[ MNL_SOCKET_BUFFER_SIZE ];
  int  ret;

  ret = mnlu_socket_recv_run( rd->nl, seq, buf, MNL_SOCKET_BUFFER_SIZE, callback, data );
  if ( ret < 0 ) perror( "error" );

  return ret;
}

void rd_prepare_msg( struct rdma_device* rd, uint32_t cmd, uint32_t* seq, uint16_t flags ) {
  *seq = time( NULL );

  rd->nlh              = mnl_nlmsg_put_header( rd->buff );
  rd->nlh->nlmsg_type  = RDMA_NL_GET_TYPE( RDMA_NL_NLDEV, cmd );
  rd->nlh->nlmsg_seq   = *seq;
  rd->nlh->nlmsg_flags = flags;
}


void rd_free( struct rdma_device* rd ) {
  if ( !rd ) return;

  free( rd->buff );
  dev_map_cleanup( rd );
  filters_cleanup( rd );
}
