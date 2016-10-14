HEADERS = $(wildcard *.h)

INCLUDES += $(shell pkg-config --cflags gstreamer-0.10)

CPPFLAGS = -DGST_PLUGIN_TYPE=Fake

all:

claen:

check-syntax:
	g++ -fsyntax-only $(CPPFLAGS) $(INCLUDES) $(HEADERS)

.PHONY: all clean check-syntax
