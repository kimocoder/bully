/*
    bully - retrieve WPA/WPA2 passphrase from a WPS-enabled AP

    Copyright (C) 2012  Brian Purcell <purcell.briand@gmail.com>

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

#include "iface.h"
#ifdef HAVE_NL80211
#include "nl80211.h"
#endif


int set_chan(struct global *G, int chan)
{
	if (!G->index[chan]) {
		vprint("[X] AP channel '%d' not found in the current channel list\n", chan);
		exit(5);
	};
	return set_chanx(G, G->index[chan]);
};


#ifdef HAVE_NL80211

int set_chanx(struct global *G, int chanx)
{
	int freq, channel = 0;
	static struct nl80211_state nlstate;
	static int nl_ready = 0;

	if (!nl_ready) {
		if (nl80211_init(&nlstate) != 0) {
			vprint("[X] Unable to initialize nl80211 (netlink) for '%s', exiting\n", G->ifname);
			exit(8);
		};
		nl_ready = 1;
	};

	int ifindex = nl80211_ifindex(G->ifname);
	if (ifindex < 0) {
		vprint("[X] Unable to resolve interface index for '%s', exiting\n", G->ifname);
		exit(8);
	};

	if (chanx) {
		channel = G->chans[chanx];
		freq = G->freqs[chanx];
		vprint("[+] Switching interface '%s' to channel '%d'\n", G->ifname, channel);
		if (nl80211_set_freq(&nlstate, ifindex, freq) != 0) {
			usleep(10000);
			if (nl80211_set_freq(&nlstate, ifindex, freq) != 0) {
				vprint("[!] nl80211 set channel on '%s' failed\n", G->ifname);
				vprint("[X] Unable to set channel on '%s', exiting\n", G->ifname);
				exit(8);
			};
		};
	} else {
		freq = nl80211_get_freq(&nlstate, ifindex);
		if (freq <= 0) {
			vprint("[!] nl80211 get channel on '%s' failed\n", G->ifname);
		} else {
			for (chanx=1; chanx<=G->chans[0]; chanx++)
				if (freq == G->freqs[chanx]) {
					channel = G->chans[chanx];
					goto set_exit;
				};
			vprint("[X] Unknown frequency '%d' MHz reported by interface '%s'\n", freq, G->ifname);
		};
		chanx = channel = 0;
	};
set_exit:
	if (channel)
		snprintf(G->schan, 8, "%d", channel);
	else
		memcpy(G->schan, "unknown", 8);
	return chanx;
};

#else /* !HAVE_NL80211 : legacy Wireless Extensions fallback */

int set_chanx(struct global *G, int chanx)
{
        int sock = 0, freq, result, channel = 0;
        struct iwreq wrq;
	memset(&wrq, 0, sizeof(struct iwreq));
	strncpy(wrq.ifr_name, G->ifname, IFNAMSIZ - 1);
        if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                vprint("[!] Socket open for ioctl() on '%s' failed with '%d'\n", G->ifname, sock);
		return 0;
	};
	if (chanx) {
		channel = G->chans[chanx];
		wrq.u.freq.m = (double)channel;
		wrq.u.freq.e = (double)0;
		wrq.u.freq.flags = IW_FREQ_FIXED;
                vprint("[+] Switching interface '%s' to channel '%d'\n", G->ifname, channel);
		if (ioctl(sock, SIOCSIWFREQ, &wrq) < 0) {
			usleep(10000);
			if ((result = ioctl(sock, SIOCSIWFREQ, &wrq)) < 0) {
				vprint("[!] ioctl(SIOCSIWFREQ) on '%s' failed with '%d'\n", G->ifname, result);
				vprint("[X] Unable to set channel on '%s', exiting\n", G->ifname);
				exit(8);
			};
		};
	} else {
		if ((result = ioctl(sock, SIOCGIWFREQ, &wrq)) < 0) {
			vprint("[!] ioctl(SIOCGIWFREQ) on '%s' failed with '%d'\n", G->ifname, result);
		} else {
			/* WEXT reports either a channel number (small value) or a
			   frequency of m * 10^e Hz; normalize both to MHz */
			double f = (double)wrq.u.freq.m;
			int e = wrq.u.freq.e;
			while (e-- > 0) f *= 10;

			if (f <= 1000) {		/* value is a channel number */
				channel = (int)f;
				for (chanx=1; chanx<=G->chans[0]; chanx++)
					if (channel == G->chans[chanx])
						goto set_exit;
				vprint("[X] Unknown channel '%d' reported by interface '%s'\n", channel, G->ifname);
			} else {			/* value is a frequency in Hz */
				freq = (int)(f / 1000000.0 + 0.5);
				for (chanx=1; chanx<=G->chans[0]; chanx++)
					if (freq == G->freqs[chanx]) {
						channel = G->chans[chanx];
						goto set_exit;
					};
				vprint("[X] Unknown frequency '%d' MHz reported by interface '%s'\n", freq, G->ifname);
			};
		};
		chanx = channel = 0;
	};
set_exit:
	close(sock);
	if (channel)
		snprintf(G->schan, 8, "%d", channel);
	else
		memcpy(G->schan, "unknown", 8);
        return chanx;
};

#endif /* HAVE_NL80211 */


int next_chan(struct global *G)
{
	int next = G->chanx + 1;
	if (G->chans[0] < next)
		next = 1;
	return set_chanx(G, next);
};


int get_hwmac(char *ifname, uint8 *mac)
{
        int sock = 0, result;
        struct ifreq irq;
	memset(&irq, 0, sizeof(irq));
	strncpy(irq.ifr_name, ifname, IFNAMSIZ - 1);
        if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return sock;
	if ((result = ioctl(sock, SIOCGIFHWADDR, &irq)) < 0) {
		close(sock);
		return result;
	};
	memcpy(mac, irq.ifr_hwaddr.sa_data, 6);
	close(sock);
        return 0;
};

