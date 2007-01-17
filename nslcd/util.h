/*
   util.h - LDAP utility functions
   This file was part of the nss_ldap library which has been
   forked into the nss-ldapd library.

   Copyright (C) 1997-2005 Luke Howard
   Copyright (C) 2006, 2007 West Consulting
   Copyright (C) 2006, 2007 Arthur de Jong

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

#ifndef _LDAP_NSS_LDAP_UTIL_H
#define _LDAP_NSS_LDAP_UTIL_H

/*
 * get the RDN's value: eg. if the RDN was cn=lukeh, getrdnvalue(entry)
 * would return lukeh.
 */
enum nss_status _nss_ldap_getrdnvalue(LDAPMessage *entry,
                                  const char *rdntype,
                                  char **rval, char **buf, size_t * len);

int _nss_ldap_write_rndvalue(FILE *fp,LDAPMessage *entry,const char *rdntype);

/*
 * map a distinguished name to a login name, or group entry
 */
enum nss_status _nss_ldap_dn2uid (const char *dn,
                             char **uid, char **buf, size_t * len,
                             int *pIsNestedGroup, LDAPMessage ** pRes);


/*
 * Escape '*' in a string for use as a filter
 */

int _nss_ldap_escape_string(const char *str,char *buf,size_t buflen);

/* Dictionary functions. */

struct ldap_datum
{
  void *data;
  size_t size;
};

#define NSS_LDAP_DATUM_ZERO(d)  do { \
                (d)->data = NULL; \
                (d)->size = 0; \
        } while (0)

#define NSS_LDAP_DB_NORMALIZE_CASE      0x1

struct ldap_dictionary
{
  struct ldap_datum key;
  struct ldap_datum value;
  struct ldap_dictionary *next;
};

struct ldap_dictionary *dict_new(void);
enum nss_status dict_put(struct ldap_dictionary *db,
                         unsigned flags,
                         const struct ldap_datum *key,
                         const struct ldap_datum *value);
enum nss_status dict_get(struct ldap_dictionary *db,
                         unsigned flags,
                         const struct ldap_datum *key,
                         struct ldap_datum *value);




#endif /* _LDAP_NSS_LDAP_UTIL_H */
