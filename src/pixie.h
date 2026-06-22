/*
    bully - retrieve WPA/WPA2 passphrase from a WPS-enabled AP

    Shared pixiewps / pin-generation state. These globals are *declared* here
    and *defined* exactly once in pdust.c, so both the current unity build and
    any future separate compilation link cleanly under -fno-common.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/
#ifndef _PIXIE_H
#define _PIXIE_H

extern char pixie_pke[1000];		/* save pke */
extern char pixie_pkr[1000];		/* save pkr */
extern char pixie_enonce[100];		/* save e-nonce */
extern char pixie_rnonce[100];		/* save r-nonce */
extern char pixie_authkey[100];		/* save AuthKey */
extern char pixie_ehash1[100];		/* save e-hash1 */
extern char pixie_ehash2[100];		/* save e-hash2 */

extern char p_iface[20];
extern char p_bssid[256];

extern int run_pixiewps;
extern int debug_level;

/* Write src[0..n-1] as lowercase colon-separated hex ("aa:bb:cc") into dst,
   in a single pass. dst must hold at least 3*n bytes. */
void pixie_hexcolon(char *dst, const unsigned char *src, unsigned int n);

#endif /* _PIXIE_H */
