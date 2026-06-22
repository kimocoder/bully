prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin

W_NAME	= bully
W_ROOT	= `pwd`

CFLAGS	+= -I$(W_ROOT) -I$(W_ROOT)/utils/ -I$(W_ROOT)/tls/
CFLAGS	+= -I$(W_ROOT)/wps/ -I$(W_ROOT)/crypto/ -I$(W_ROOT)/common/
CFLAGS	+= -DUSE_INTERNAL_CRYPTO -O2

# Warnings (advisory: the build is not -Werror) and runtime hardening.
# _FORTIFY_SOURCE needs optimization (-O2 above); -U avoids redefinition
# warnings on toolchains that already enable it by default.
CWARN	= -Wall -Wextra -Wformat-security
CHARDEN	= -fstack-protector-strong -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2
CFLAGS	+= $(CWARN) $(CHARDEN)

LDFLAGS += -lpcap

# Channel control uses nl80211 (netlink) instead of the deprecated Wireless
# Extensions ioctls. Auto-enabled when libnl is present; force the legacy
# WEXT fallback with "make NL80211=0".
NL80211 ?= $(shell pkg-config --exists libnl-genl-3.0 2>/dev/null && echo 1 || echo 0)
ifeq ($(NL80211),1)
CFLAGS  += -DHAVE_NL80211 $(shell pkg-config --cflags libnl-genl-3.0)
LDFLAGS += $(shell pkg-config --libs libnl-genl-3.0)
endif

# Optional Lua scripting support for custom PIN-generation scripts (-u/--lua).
# Auto-detects a Lua dev package via pkg-config; force on with "make LUA=1"
# (errors if none is found) or off with "make LUA=0". When enabled, the
# Lua-only translation units luaclib.c and pingen.c are added to the build.
LUA_PKG ?= $(shell for p in lua5.4 lua5.3 lua5.2 lua luajit; do pkg-config --exists $$p 2>/dev/null && { echo $$p; break; }; done)
LUA     ?= $(if $(LUA_PKG),1,0)
ifeq ($(LUA),1)
  ifeq ($(LUA_PKG),)
    $(error LUA=1 requested but no Lua pkg-config package (lua5.x/lua/luajit) was found)
  endif
CFLAGS  += -DHAVE_LUA $(shell pkg-config --cflags $(LUA_PKG))
LDFLAGS += $(shell pkg-config --libs $(LUA_PKG))
LUASRC   = luaclib.c pingen.c
endif

# bully is a unity build: bully.c #includes the project .c files in INCSRC
# below, so only the translation units in OBJSRC (plus optional LUASRC) are
# passed to the compiler. INCSRC and HDRS are listed purely as build
# prerequisites, so that editing any included source or header triggers a rebuild.
OBJSRC	= $(W_NAME).c pdust.c
INCSRC	= 80211.c frame.c iface.c crc32.c timer.c utils.c nl80211.c
HDRS	= $(W_NAME).h 80211.h frame.h iface.h bswap.h pdust.h pingen.h \
	  luaclib.h nl80211.h version.h

all: $(W_NAME)

$(W_NAME): $(OBJSRC) $(LUASRC) $(INCSRC) $(HDRS)
	$(CC) $(CFLAGS) -o $(@) $(OBJSRC) $(LUASRC) $(LDFLAGS)

strip: $(W_NAME)
	strip $(W_NAME)

clean:
	-rm -f $(W_NAME) $(W_NAME).o

distclean: clean

install: all
	install -d $(DESTDIR)$(bindir)
	install -m 755 $(W_NAME) $(DESTDIR)$(bindir)

uninstall:
	-rm -f $(DESTDIR)$(bindir)/$(W_NAME)
