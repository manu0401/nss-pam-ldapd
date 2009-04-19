/*
   nslcd.h - file describing client/server protocol

   Copyright (C) 2006 West Consulting
   Copyright (C) 2006, 2007, 2009 Arthur de Jong

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301 USA
*/

#ifndef _NSLCD_H
#define _NSLCD_H 1

/*
   The protocol used between the nslcd client and server is a simple binary
   protocol. It is request/response based where the client initiates a
   connection, does a single request and closes the connection again. Any
   mangled or not understood messages will be silently ignored by the server.

   A request looks like:
     INT32  NSLCD_VERSION
     INT32  NSLCD_ACTION_*
     [request parameters if any]
   A response looks like:
     INT32  NSLCD_VERSION
     INT32  NSLCD_ACTION_* (the original request type)
     [result(s)]
     INT32  NSLCD_RESULT_END
   A single result entry looks like:
     INT32  NSLCD_RESULT_BEGIN
     [result value(s)]
   If a response would return multiple values (e.g. for NSLCD_ACTION_*_ALL
   functions) each return value will be preceded by a NSLCD_RESULT_BEGIN
   value. After the last returned result the server sends
   NSLCD_RESULT_END. If some error occurs (e.g. LDAP server unavailable,
   error in the request, etc) the server terminates the connection to signal
   an error condition (breaking the protocol).

   These are the available basic data types:
     INT32  - 32-bit integer value
     TYPE   - a typed field that is transferred using sizeof()
     STRING - a string length (32bit) followed by the string value (not
              null-terminted) the string itself is assumed to be UTF-8
     STRINGLIST - a 32-bit number noting the number of strings followed by
                  the strings one at a time

   Furthermore the ADDRESS compound data type is defined as:
     INT32  type of address: e.g. AF_INET or AF_INET6
     INT32  lenght of address
     RAW    the address itself in network byte order
   With the ADDRESSLIST using the same construct as with STRINGLIST.

   The protocol uses host-byte order for all types (except in the raw
   address above).
*/

/* The current version of the protocol. Note that version 1
   is experimental and this version will be used until a
   1.0 release of nss-ldapd is made. */
#define NSLCD_VERSION 1

/* Email alias (/etc/aliases) NSS requests. The result values for a
   single entry are:
     STRING      alias name
     STRINGLIST  alias rcpts */
#define NSLCD_ACTION_ALIAS_BYNAME       4001
#define NSLCD_ACTION_ALIAS_ALL          4002

/* Ethernet address/name mapping NSS requests. The result values for a
   single entry are:
     STRING            ether name
     TYPE(uint8_t[6])  ether address */
#define NSLCD_ACTION_ETHER_BYNAME       3001
#define NSLCD_ACTION_ETHER_BYETHER      3002
#define NSLCD_ACTION_ETHER_ALL          3005

/* Group and group membership related NSS requests. The result values
   for a single entry are:
     STRING       group name
     STRING       group password
     TYPE(gid_t)  group id
     STRINGLIST   members (usernames) of the group
     (not that the BYMEMER call returns an emtpy members list) */
#define NSLCD_ACTION_GROUP_BYNAME       5001
#define NSLCD_ACTION_GROUP_BYGID        5002
#define NSLCD_ACTION_GROUP_BYMEMBER     5003
#define NSLCD_ACTION_GROUP_ALL          5004

/* Hostname (/etc/hosts) lookup NSS requests. The result values
   for an entry are:
     STRING       host name
     STRINGLIST   host aliases
     ADDRESSLIST  host addresses */
#define NSLCD_ACTION_HOST_BYNAME        6001
#define NSLCD_ACTION_HOST_BYADDR        6002
#define NSLCD_ACTION_HOST_ALL           6005

/* Netgroup NSS request return a number of results. Result values
   can be either a reference to another netgroup:
     INT32   NETGROUP_TYPE_NETGROUP
     STRING  other netgroup name
   or a netgroup triple:
     INT32   NETGROUP_TYPE_TRIPLE
     STRING  host
     STRING  user
     STRING  domain */
#define NSLCD_ACTION_NETGROUP_BYNAME   12001
#define NETGROUP_TYPE_NETGROUP 123
#define NETGROUP_TYPE_TRIPLE   456

/* Network name (/etc/networks) NSS requests. Result values for a single
   entry are:
     STRING       network name
     STRINGLIST   network aliases
     ADDRESSLIST  network addresses */
#define NSLCD_ACTION_NETWORK_BYNAME     8001
#define NSLCD_ACTION_NETWORK_BYADDR     8002
#define NSLCD_ACTION_NETWORK_ALL        8005

/* User account (/etc/passwd) NSS requests. Result values are:
     STRING       user name
     STRING       user password
     TYPE(uid_t)  user id
     TYPE(gid_t)  group id
     STRING       gecos information
     STRING       home directory
     STRING       login shell */
#define NSLCD_ACTION_PASSWD_BYNAME      1001
#define NSLCD_ACTION_PASSWD_BYUID       1002
#define NSLCD_ACTION_PASSWD_ALL         1004

/* Protocol information requests. Result values are:
     STRING      protocol name
     STRINGLIST  protocol aliases
     INT32       protocol number */
#define NSLCD_ACTION_PROTOCOL_BYNAME    9001
#define NSLCD_ACTION_PROTOCOL_BYNUMBER  9002
#define NSLCD_ACTION_PROTOCOL_ALL       9003

/* RPC information requests. Result values are:
     STRING      rpc name
     STRINGLIST  rpc aliases
     INT32       rpc number */
#define NSLCD_ACTION_RPC_BYNAME        10001
#define NSLCD_ACTION_RPC_BYNUMBER      10002
#define NSLCD_ACTION_RPC_ALL           10003

/* Service (/etc/services) information requests. Result values are:
     STRING      service name
     STRINGLIST  service aliases
     INT32       service (port) number
     STRING      service protocol */
#define NSLCD_ACTION_SERVICE_BYNAME    11001
#define NSLCD_ACTION_SERVICE_BYNUMBER  11002
#define NSLCD_ACTION_SERVICE_ALL       11005

/* Extended user account (/etc/shadow) information requests. Result
   values for a single entry are:
     STRING  user name
     STRING  user password
     INT32   last password change
     INT32   mindays
     INT32   maxdays
     INT32   warn
     INT32   inact
     INT32   expire
     INT32   flag */
#define NSLCD_ACTION_SHADOW_BYNAME      2001
#define NSLCD_ACTION_SHADOW_ALL         2005

/* PAM-related requests. The requests and responses need to be defined. */
#define NSLCD_ACTION_PAM_AUTHC         20001
#define NSLCD_ACTION_PAM_AUTHZ         20002
#define NSLCD_ACTION_PAM_SESS_O        20003
#define NSLCD_ACTION_PAM_SESS_C        20004
#define NSLCD_ACTION_PAM_PWMOD         20005

/* Request result codes. */
#define NSLCD_RESULT_BEGIN                 0
#define NSLCD_RESULT_END                   3

#endif /* not _NSLCD_H */
