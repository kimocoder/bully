/*
    bully - retrieve WPA/WPA2 passphrase from a WPS-enabled AP

    Copyright (C) 2017  wiire         <wi7ire@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdint.h>
#include <string.h>

#include "pdust.h"
#include "pixie.h"

/* Single definition of the shared pixiewps / pin-generation state that
   pixie.h declares (extern). Kept here, in an always-compiled translation
   unit, so there is exactly one definition regardless of build model. */
char pixie_pke[1000];
char pixie_pkr[1000];
char pixie_enonce[100];
char pixie_rnonce[100];
char pixie_authkey[100];
char pixie_ehash1[100];
char pixie_ehash2[100];
char p_iface[20];
char p_bssid[256];
int run_pixiewps;
int debug_level;

/* Single-pass bin -> "aa:bb:cc" hex used by the pixiewps data collectors,
   replacing the per-byte sprintf()+strcat() (O(n^2)) builders. */
void pixie_hexcolon(char *dst, const unsigned char *src, unsigned int n)
{
	static const char hex[] = "0123456789abcdef";
	unsigned int i;
	char *p = dst;
	for (i = 0; i < n; i++) {
		if (i)
			*p++ = ':';
		*p++ = hex[src[i] >> 4];
		*p++ = hex[src[i] & 0x0f];
	}
	*p = '\0';
}

char OUI_NULL[OUI_STR_LEN] = "-";
vendor_t vendor_list[] = {
	{{0x00,0x03,0x7f}, "AtherosC", PWPS_NONE       }, /* Atheros Communications */
	{{0x00,0x10,0x18}, "Broadcom", PWPS_ECOS_SIMPLE}, /* Broadcom */
	{{0x00,0x50,0x43}, "MarvellS", PWPS_NONE       }, /* MARVELL SEMICONDUCTOR, INC */
	{{0x00,0x0c,0x43}, "RalinkTe", PWPS_RT         }, /* Ralink Technology, Corp. */
	{{0x00,0xe0,0x4c}, "RealtekS", PWPS_RTL819x    }  /* REALTEK SEMICONDUCTOR CORP. */
};
#define VENDOR_LIST_SIZE (sizeof(vendor_list) / sizeof(vendor_list[0]))

char *get_vendor(uint8_t *oui)
{
	unsigned int i;
	for (i = 0; i < VENDOR_LIST_SIZE; i++) {
		if (!memcmp(vendor_list[i].oui, oui, 3))
			return vendor_list[i].name;
	};
	return OUI_NULL;
};