ifeq (,$(wildcard ../config.mak))
$(error "../config.mak is not present, run configure !")
endif
include ../config.mak

EXTRADIST = \
	Doxyfile \

all: doxygen

doxygen:
ifeq ($(DOC),yes)
ifeq (,$(wildcard doxygen))
	PROJECT_NUMBER="$(VERSION)" doxygen Doxyfile
endif
endif

clean:
	rm -rf doxygen

install:
ifeq ($(DOC),yes)
	if [ -d doxygen/html ]; then \
		$(INSTALL) -d $(docdir)/libduck; \
		$(INSTALL) -c -m 755 doxygen/html/* $(docdir)/libduck; \
	fi
endif

uninstall:
	rm -rf $(docdir)/libduck

dist-all:
	cp $(EXTRADIST) Makefile $(DIST)

.PHONY: clean dist-all doxygen *install
