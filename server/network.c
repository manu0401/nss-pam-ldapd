/*
   network.c - network address entry lookup routines
   This file was part of the nss_ldap library (as ldap-network.c)
   which has been forked into the nss-ldapd library.

   Copyright (C) 1997-2005 Luke Howard
   Copyright (C) 2006 West Consulting
   Copyright (C) 2006 Arthur de Jong

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
   MA 02110-1301 USA
*/

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <sys/socket.h>
#include <errno.h>
#ifdef HAVE_LBER_H
#include <lber.h>
#endif
#ifdef HAVE_LDAP_H
#include <ldap.h>
#endif
#if defined(HAVE_THREAD_H)
#include <thread.h>
#elif defined(HAVE_PTHREAD_H)
#include <pthread.h>
#endif

#include "ldap-nss.h"
#include "util.h"
#include "common.h"
#include "log.h"

#if defined(HAVE_USERSEC_H)
#define MAXALIASES 35
#define MAXADDRSIZE 4
#endif /* HAVE_USERSEC_H */

/* write a single network entry to the stream */
static int write_netent(FILE *fp,struct netent *result)
{
  int32_t tmpint32,tmp2int32,tmp3int32;
  /* write the network name */
  WRITE_STRING(fp,result->n_name);
  /* write the alias list */
  WRITE_STRINGLIST_NULLTERM(fp,result->n_aliases);
  /* write the number of addresses */
  WRITE_INT32(fp,1);
  /* write the addresses in network byte order */
  WRITE_INT32(fp,result->n_addrtype);
  WRITE_INT32(fp,sizeof(unsigned long int));
  result->n_net=htonl(result->n_net);
  WRITE_INT32(fp,result->n_net);
  return 0;
}
static enum nss_status
_nss_ldap_parse_net (LDAPMessage * e,
                     struct ldap_state * pvt,
                     void *result, char *buffer, size_t buflen)
{

  char *tmp;
  struct netent *network = (struct netent *) result;
  enum nss_status stat;

  /* IPv6 support ? XXX */
  network->n_addrtype = AF_INET;

  stat = _nss_ldap_assign_attrval (e, ATM (LM_NETWORKS, cn), &network->n_name,
                                   &buffer, &buflen);
  if (stat != NSS_STATUS_SUCCESS)
    return stat;

  stat =
    _nss_ldap_assign_attrval (e, AT (ipNetworkNumber), &tmp, &buffer,
                              &buflen);
  if (stat != NSS_STATUS_SUCCESS)
    return stat;

  network->n_net = inet_network (tmp);

  stat =
    _nss_ldap_assign_attrvals (e, ATM (LM_NETWORKS, cn), network->n_name,
                               &network->n_aliases, &buffer, &buflen, NULL);
  if (stat != NSS_STATUS_SUCCESS)
    return stat;

  return NSS_STATUS_SUCCESS;
}

int nslcd_network_byname(FILE *fp)
{
  int32_t tmpint32;
  char *name;
  struct ldap_args a;
  int retv;
  struct netent result;
  char buffer[1024];
  int errnop;
  /* read request parameters */
  READ_STRING_ALLOC(fp,name);
  /* log call */
  log_log(LOG_DEBUG,"nslcd_network_byname(%s)",name);
  /* write the response header */
  /* FIXME: free(name) when one of these writes fails */
  WRITE_INT32(fp,NSLCD_VERSION);
  WRITE_INT32(fp,NSLCD_ACTION_NETWORK_BYNAME);
  /* do the LDAP request */
  LA_INIT(a);
  LA_STRING(a)=name;
  LA_TYPE(a)=LA_TYPE_STRING;
  retv=nss2nslcd(_nss_ldap_getbyname(&a,&result,buffer,1024,&errnop,_nss_ldap_filt_getnetbyname,LM_NETWORKS,_nss_ldap_parse_net));
  /* no more need for this string */
  free(name);
  /* write the response */
  WRITE_INT32(fp,retv);
  if (retv==NSLCD_RESULT_SUCCESS)
    write_netent(fp,&result);
  WRITE_FLUSH(fp);
  /* we're done */
  return 0;
}

int nslcd_network_byaddr(FILE *fp)
{
  int32_t tmpint32;
  int af;
  int len;
  char addr[64],name[1024];
  struct ldap_args a;
  int retv=456;
  struct netent result;
  char buffer[1024];
  int errnop;
  /* read address family */
  READ_INT32(fp,af);
  if (af!=AF_INET)
  {
    log_log(LOG_WARNING,"incorrect address family specified: %d",af);
    return -1;
  }
  /* read address length */
  READ_INT32(fp,len);
  if ((len>64)||(len<=0))
  {
    log_log(LOG_WARNING,"address length incorrect: %d",len);
    return -1;
  }
  /* read address */
  READ(fp,addr,len);
  /* translate the address to a string */
  if (inet_ntop(af,addr,name,1024)==NULL)
  {
    log_log(LOG_WARNING,"unable to convert address to string");
    return -1;
  }
  /* log call */
  log_log(LOG_DEBUG,"nslcd_network_byaddr(%s)",name);
  /* write the response header */
  WRITE_INT32(fp,NSLCD_VERSION);
  WRITE_INT32(fp,NSLCD_ACTION_NETWORK_BYADDR);
  /* prepare the LDAP request */
  LA_INIT(a);
  LA_STRING(a)=name;
  LA_TYPE(a)=LA_TYPE_STRING;
  /* do requests until we find a result */
  /* TODO: probably do more sofisticated queries */
  while (retv==456)
  {
    /* do the request */
    retv=nss2nslcd(_nss_ldap_getbyname(&a,&result,buffer,1024,&errnop,_nss_ldap_filt_getnetbyaddr,LM_NETWORKS,_nss_ldap_parse_net));
    /* if no entry was found, retry with .0 stripped from the end */
    if ((retv==NSLCD_RESULT_NOTFOUND) &&
        (strlen(name)>2) &&
        (strncmp(name+strlen(name)-2,".0",2)==0))
    {
      /* strip .0 and try again */
      name[strlen(name)-2]='\0';
      retv=456;
    }
  }
  /* write the response */
  WRITE_INT32(fp,retv);
  if (retv==NSLCD_RESULT_SUCCESS)
    write_netent(fp,&result);
  WRITE_FLUSH(fp);
  /* we're done */
  return 0;
}

int nslcd_network_all(FILE *fp)
{
  int32_t tmpint32;
  static struct ent_context *net_context;
  /* these are here for now until we rewrite the LDAP code */
  struct netent result;
  char buffer[1024];
  int errnop;
  int retv;
  /* log call */
  log_log(LOG_DEBUG,"nslcd_network_all()");
  /* write the response header */
  WRITE_INT32(fp,NSLCD_VERSION);
  WRITE_INT32(fp,NSLCD_ACTION_NETWORK_ALL);
  /* initialize context */
  if (_nss_ldap_ent_context_init(&net_context)==NULL)
    return -1;
  /* loop over all results */
  while ((retv=nss2nslcd(_nss_ldap_getent(&net_context,&result,buffer,1024,&errnop,_nss_ldap_filt_getnetent,LM_NETWORKS,_nss_ldap_parse_net)))==NSLCD_RESULT_SUCCESS)
  {
    /* write the result */
    WRITE_INT32(fp,retv);
    if (retv==NSLCD_RESULT_SUCCESS)
      write_netent(fp,&result);
  }
  /* write the final result code */
  WRITE_INT32(fp,retv);
  WRITE_FLUSH(fp);
  /* FIXME: if a previous call returns what happens to the context? */
  _nss_ldap_enter();
  _nss_ldap_ent_context_release(net_context);
  _nss_ldap_leave();
  /* we're done */
  return 0;
}
