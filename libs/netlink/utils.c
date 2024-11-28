#include "libmnl/libmnl.h"
#include "libnetlink.h"

#include <string.h>

static const enum mnl_attr_data_type extack_policy[ NLMSGERR_ATTR_MAX + 1 ] = {
  [NLMSGERR_ATTR_MSG]  = MNL_TYPE_NUL_STRING,
  [NLMSGERR_ATTR_OFFS] = MNL_TYPE_U32,
};

static int err_attr_cb( const struct nlattr* attr, void* data ) {
  const struct nlattr** tb = data;
  uint16_t              type;

  if ( mnl_attr_type_valid( attr, NLMSGERR_ATTR_MAX ) < 0 ) {
    fprintf( stderr, "Invalid extack attribute\n" );
    return MNL_CB_ERROR;
  }

  type = mnl_attr_get_type( attr );
  if ( mnl_attr_validate( attr, extack_policy[ type ] ) < 0 ) {
    fprintf( stderr, "extack attribute %d failed validation\n",
             type );
    return MNL_CB_ERROR;
  }

  tb[ type ] = attr;
  return MNL_CB_OK;
}

static void print_ext_ack_msg( bool is_err, const char* msg ) {
  fprintf( stderr, "%s: %s", is_err ? "Error" : "Warning", msg );
  if ( msg[ strlen( msg ) - 1 ] != '.' )
    fprintf( stderr, "." );
  fprintf( stderr, "\n" );
}

int nl_dump_ext_ack( const struct nlmsghdr* nlh, nl_ext_ack_fn_t errfn ) {
  struct nlattr*         tb[ NLMSGERR_ATTR_MAX + 1 ] = {};
  const struct nlmsgerr* err                         = mnl_nlmsg_get_payload( nlh );
  const struct nlmsghdr* err_nlh                     = NULL;
  unsigned int           hlen                        = sizeof( *err );
  const char*            msg                         = NULL;
  uint32_t               off                         = 0;

  /* no TLVs, nothing to do here */
  if ( !( nlh->nlmsg_flags & NLM_F_ACK_TLVS ) )
    return 0;

  /* if NLM_F_CAPPED is set then the inner err msg was capped */
  if ( !( nlh->nlmsg_flags & NLM_F_CAPPED ) )
    hlen += mnl_nlmsg_get_payload_len( &err->msg );

  if ( mnl_attr_parse( nlh, hlen, err_attr_cb, tb ) != MNL_CB_OK )
    return 0;

  if ( tb[ NLMSGERR_ATTR_MSG ] )
    msg = mnl_attr_get_str( tb[ NLMSGERR_ATTR_MSG ] );

  if ( tb[ NLMSGERR_ATTR_OFFS ] ) {
    off = mnl_attr_get_u32( tb[ NLMSGERR_ATTR_OFFS ] );

    if ( off > nlh->nlmsg_len ) {
      fprintf( stderr,
               "Invalid offset for NLMSGERR_ATTR_OFFS\n" );
      off = 0;
    } else if ( !( nlh->nlmsg_flags & NLM_F_CAPPED ) )
      err_nlh = &err->msg;
  }

  if ( tb[ NLMSGERR_ATTR_MISS_TYPE ] )
    fprintf( stderr, "Missing required attribute type %u\n",
             mnl_attr_get_u32( tb[ NLMSGERR_ATTR_MISS_TYPE ] ) );

  if ( errfn )
    return errfn( msg, off, err_nlh );

  if ( msg && *msg != '\0' ) {
    bool is_err = !!err->error;

    print_ext_ack_msg( is_err, msg );
    return is_err ? 1 : 0;
  }

  return 0;
}

int nl_dump_ext_ack_done( const struct nlmsghdr* nlh, unsigned int offset, int error ) {
  struct nlattr* tb[ NLMSGERR_ATTR_MAX + 1 ] = {};
  const char*    msg                         = NULL;

  if ( mnl_attr_parse( nlh, offset, err_attr_cb, tb ) != MNL_CB_OK )
    return 0;

  if ( tb[ NLMSGERR_ATTR_MSG ] )
    msg = mnl_attr_get_str( tb[ NLMSGERR_ATTR_MSG ] );

  if ( msg && *msg != '\0' ) {
    bool is_err = !!error;

    print_ext_ack_msg( is_err, msg );
    return is_err ? 1 : 0;
  }

  return 0;
}
