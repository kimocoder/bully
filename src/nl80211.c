/*
    bully - retrieve WPA/WPA2 passphrase from a WPS-enabled AP

    nl80211 (netlink) channel control, replacing the deprecated Wireless
    Extensions (WEXT) SIOCSIWFREQ/SIOCGIWFREQ ioctls.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/sockios.h>

#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>
#include <linux/nl80211.h>

#include "nl80211.h"


int nl80211_ifindex(const char *ifname)
{
	struct ifreq ifr;
	int fd, r;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);
	r = ioctl(fd, SIOCGIFINDEX, &ifr);
	close(fd);
	return (r < 0) ? -1 : ifr.ifr_ifindex;
}


int nl80211_init(struct nl80211_state *state)
{
	state->nl_sock = nl_socket_alloc();
	if (!state->nl_sock)
		return -1;
	if (genl_connect(state->nl_sock)) {
		nl_socket_free(state->nl_sock);
		state->nl_sock = NULL;
		return -1;
	}
	state->nl80211_id = genl_ctrl_resolve(state->nl_sock, "nl80211");
	if (state->nl80211_id < 0) {
		nl_socket_free(state->nl_sock);
		state->nl_sock = NULL;
		return -1;
	}
	return 0;
}


void nl80211_cleanup(struct nl80211_state *state)
{
	if (state->nl_sock) {
		nl_socket_free(state->nl_sock);
		state->nl_sock = NULL;
	}
}


static int nl_err_cb(struct sockaddr_nl *nla, struct nlmsgerr *err, void *arg)
{
	int *ret = arg;
	(void)nla;
	*ret = err->error;
	return NL_STOP;
}

static int nl_finish_cb(struct nl_msg *msg, void *arg)
{
	int *ret = arg;
	(void)msg;
	*ret = 0;
	return NL_SKIP;
}

static int nl_ack_cb(struct nl_msg *msg, void *arg)
{
	int *ret = arg;
	(void)msg;
	*ret = 0;
	return NL_STOP;
}

/* Send 'msg', pump the socket until the kernel finishes/acks/errors.
   Takes ownership of 'msg' (always freed). Returns 0 on success. */
static int nl_send_recv(struct nl80211_state *state, struct nl_msg *msg,
			nl_recvmsg_msg_cb_t valid_cb, void *valid_arg)
{
	struct nl_cb *cb;
	int err;

	cb = nl_cb_alloc(NL_CB_DEFAULT);
	if (!cb) {
		nlmsg_free(msg);
		return -ENOMEM;
	}

	if (nl_send_auto(state->nl_sock, msg) < 0) {
		nl_cb_put(cb);
		nlmsg_free(msg);
		return -EIO;
	}

	err = 1;
	nl_cb_err(cb, NL_CB_CUSTOM, nl_err_cb, &err);
	nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, nl_finish_cb, &err);
	nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, nl_ack_cb, &err);
	if (valid_cb)
		nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, valid_cb, valid_arg);

	while (err > 0) {
		int rc = nl_recvmsgs(state->nl_sock, cb);
		if (rc < 0) {
			err = rc;
			break;
		}
	}

	nl_cb_put(cb);
	nlmsg_free(msg);
	return err;
}


int nl80211_set_freq(struct nl80211_state *state, int ifindex, int freq)
{
	struct nl_msg *msg = nlmsg_alloc();
	if (!msg)
		return -ENOMEM;

	genlmsg_put(msg, 0, 0, state->nl80211_id, 0, 0, NL80211_CMD_SET_CHANNEL, 0);
	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, ifindex);
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY_FREQ, freq);
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY_CHANNEL_TYPE, NL80211_CHAN_NO_HT);

	return nl_send_recv(state, msg, NULL, NULL);

nla_put_failure:
	nlmsg_free(msg);
	return -ENOBUFS;
}


static int nl80211_freq_cb(struct nl_msg *msg, void *arg)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	int *freq = arg;

	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	if (tb[NL80211_ATTR_WIPHY_FREQ])
		*freq = nla_get_u32(tb[NL80211_ATTR_WIPHY_FREQ]);

	return NL_SKIP;
}


int nl80211_get_freq(struct nl80211_state *state, int ifindex)
{
	int freq = 0;
	struct nl_msg *msg = nlmsg_alloc();
	if (!msg)
		return -ENOMEM;

	genlmsg_put(msg, 0, 0, state->nl80211_id, 0, 0, NL80211_CMD_GET_INTERFACE, 0);
	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, ifindex);

	if (nl_send_recv(state, msg, nl80211_freq_cb, &freq) < 0)
		return -1;
	return freq;

nla_put_failure:
	nlmsg_free(msg);
	return -ENOBUFS;
}
