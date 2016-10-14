PREFIX = /usr/local
INSTALL_PATH = $(PREFIX)/include/gstPluginWrap

HEADERS = $(wildcard *.h)

INCLUDES += $(shell pkg-config --cflags gstreamer-0.10)

CPPFLAGS = -DGST_PLUGIN_TYPE=Fake

all:

claen:

check-syntax:
	g++ -fsyntax-only $(CPPFLAGS) $(INCLUDES) $(HEADERS)

install:
	mkdir -p $(INSTALL_PATH)
	cp $(HEADERS) $(INSTALL_PATH)
	./make_pkgconfig.sh $(PREFIX)/lib/pkgconfig $(INSTALL_PATH)

uninstall:
	rm -f $(addprefix $(INSTALL_PATH)/, $(HEADERS))
	rm -f $(PREFIX)/lib/pkgconfig/gstPluginWrap.pc

.PHONY: all clean check-syntax install uninstall
