/* This file is part of GNU Dico.
   Copyright (C) 1998-2000, 2008, 2010, 2012 Sergey Poznyakoff

   GNU Dico is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GNU Dico is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Dico.  If not, see <http://www.gnu.org/licenses/>. */

#include <dicod.h>
#if defined(HAVE_CRYPT_H)
# include <crypt.h>
#endif
#include "md5.h"
#include "sha1.h"

static int
my_strncasecmp(const char *p, const char *q, size_t len)
{
    for ( ;len; len--, p++, q++) {
	if (*p != toupper (*q))
	    return 1;
    }
    return 0;
}

typedef int (*pwcheck_fp)(const char *, const char *);

static int
chk_crypt(const char *db_pass, const char *pass)
{
  return strcmp(db_pass, crypt(pass, db_pass));
}

static int
chk_md5(const char *db_pass, const char *pass)
{
    unsigned char md5digest[16];
    struct md5_ctx md5context;
    size_t size = 0;
    unsigned char *buf = NULL;
    int rc;
    
    md5_init_ctx(&md5context);
    md5_process_bytes(pass, strlen(pass), &md5context);
    md5_finish_ctx(&md5context, md5digest);

    if (dico_base64_decode((unsigned char*)db_pass, strlen(db_pass),
			   &buf, &size))
	return -1;

    if (size != 16) {
	free(buf);
	return -1;
    }
    rc = memcmp(md5digest, buf, sizeof md5digest);
    free(buf);
    return rc;
}

static int
chk_smd5(const char *db_pass, const char *pass)
{
    int rc;
    unsigned char md5digest[16];
    struct md5_ctx md5context;
    size_t size = 0;
    unsigned char *buf = NULL;

    if (dico_base64_decode((unsigned char*)db_pass, strlen(db_pass),
			   &buf, &size))
	return -1;
    if (size <= 16) {
	dico_log(L_ERR, 0, "malformed SMD5 password: %s", db_pass);
	free(buf);
	return -1;
    }
  
    md5_init_ctx(&md5context);
    md5_process_bytes(pass, strlen(pass), &md5context);
    md5_process_bytes(buf + 16, size - 16, &md5context);
    md5_finish_ctx(&md5context, md5digest);

    rc = memcmp(md5digest, buf, sizeof md5digest);
    free(buf);
    return rc;
}

static int
chk_sha(const char *db_pass, const char *pass)
{
    int rc;
    unsigned char sha1digest[20];
    struct sha1_ctx sha1context;
    size_t size = 0;
    unsigned char *buf = NULL;

    sha1_init_ctx(&sha1context);
    sha1_process_bytes(pass, strlen(pass), &sha1context);
    sha1_finish_ctx(&sha1context, sha1digest);

    if (dico_base64_decode((unsigned char *)db_pass, strlen(db_pass),
			   &buf, &size))
	return -1;
    if (size != 20) {
	free(buf);
	return -1;
    }
  
    rc = memcmp(sha1digest, buf, sizeof sha1digest);
    free(buf);
    return rc;
}

static int
chk_ssha(const char *db_pass, const char *pass)
{
    int rc;
    unsigned char sha1digest[20];
    struct sha1_ctx sha1context;
    size_t size = 0;
    unsigned char *buf = NULL;

    if (dico_base64_decode((unsigned char *)db_pass, strlen(db_pass),
			   &buf, &size))
	return -1;
    if (size <= 16) {
	dico_log(L_ERR, 0, "malformed SSHA1 password: %s", db_pass);
	free(buf);
	return -1;
    }
  
    sha1_init_ctx(&sha1context);
    sha1_process_bytes(pass, strlen(pass), &sha1context);
    sha1_process_bytes(buf + 20, size - 20, &sha1context);
    sha1_finish_ctx(&sha1context, sha1digest);

    rc = memcmp(sha1digest, buf, sizeof sha1digest);
    free(buf);
    return rc;
}

static struct passwd_algo {
    char *algo;
    size_t len;
    pwcheck_fp pwcheck;
} pwtab[] = {
#define DP(s, f) { #s, sizeof (#s) - 1, f }
    DP (CRYPT, chk_crypt),
    DP (MD5, chk_md5),
    DP (SMD5, chk_smd5),
    DP (SHA, chk_sha),
    DP (SSHA, chk_ssha),
    { NULL }
#undef DP
};

static pwcheck_fp
find_pwcheck(const char *algo, int len)
{
    struct passwd_algo *p;
    for (p = pwtab; p->algo; p++)
	if (len == p->len && my_strncasecmp(p->algo, algo, len) == 0)
	    return p->pwcheck;
    return NULL;
}

int
dicod_check_password(const char *db_pass, const char *pass)
{
    if (db_pass[0] == '{') {
	int len;
	const char *algo;
	pwcheck_fp pwcheck;

	algo = db_pass + 1;
	for (len = 0; algo[len] != '}'; len++)
	    if (algo[len] == 0) {
		/* Possibly malformed password */
		return -1;
	    }
	db_pass = algo + len + 1;
	pwcheck = find_pwcheck(algo, len);
	if (pwcheck)
	    return pwcheck(db_pass, pass);
	else {
	    dico_log(L_ERR, 0, "unsupported password algorithm scheme: %.*s",
		     len, algo);
	    return -1;
	}
    } else if (strcmp(crypt(pass, db_pass), db_pass) == 0)
	return 0;
    return strcmp(pass, db_pass);
}

