/*
    bully - retrieve WPA/WPA2 passphrase from a WPS-enabled AP

    nl80211 (netlink) channel control, replacing the deprecated Wireless
    Extensions (WEXT) SIOCSIWFREQ/SIOCGIWFREQ ioctls.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/
#ifndef _NL80211_H
#define _NL80211_H

struct nl_sock;

struct nl80211_state {
	struct nl_sock	*nl_sock;
	int		nl80211_id;
};

/* Resolve the interface index for a name (standard netdevice ioctl). */
int  nl80211_ifindex(const char *ifname);

/* Allocate the netlink socket and resolve the nl80211 family. */
int  nl80211_init(struct nl80211_state *state);
void nl80211_cleanup(struct nl80211_state *state);

/* Set the operating channel by center frequency (MHz, 20 MHz / no-HT). */
int  nl80211_set_freq(struct nl80211_state *state, int ifindex, int freq);

/* Return the current center frequency in MHz, or <= 0 on failure. */
int  nl80211_get_freq(struct nl80211_state *state, int ifindex);

#endif /* _NL80211_H */
