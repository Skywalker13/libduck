ifeq (,$(wildcard config.mak))
$(error "config.mak is not present, run configure !")
endif
include config.mak

PKGCONFIG_DIR = $(libdir)/pkgconfig
PKGCONFIG_FILE = libduck.pc

DDTEST = libduck-test
DDTEST_SRCS = libduck-test.c
DDTEST_OBJS = $(DDTEST_SRCS:.c=.o)
DDTEST_MAN = # $(DDTEST).1

APPS_CPPFLAGS = -Isrc $(CFG_CPPFLAGS) $(CPPFLAGS)
APPS_LDFLAGS = -Lsrc -lduck $(CFG_LDFLAGS) $(LDFLAGS)

MANS = $(DDTEST_MAN)

ifeq ($(BUILD_STATIC),yes)
ifeq ($(BUILD_SHARED),no)
  APPS_LDFLAGS += $(EXTRALIBS)
endif
endif

DISTFILE = libduck-$(VERSION).tar.bz2

EXTRADIST = \
	AUTHORS \
	ChangeLog \
	configure \
	COPYING \
	README \
	$(MANS) \

SUBDIRS = \
	DOCS \
	src \

.SUFFIXES: .c .o

all: lib apps docs

.c.o:
	$(CC) -c $(OPTFLAGS) $(CFLAGS) $(APPS_CPPFLAGS) -o $@ $<

config.mak: configure
	@echo "############################################################"
	@echo "####### Please run ./configure again - it's changed! #######"
	@echo "############################################################"

lib:
	$(MAKE) -C src

$(DDTEST): $(DDTEST_OBJS)
	$(CC) $(DDTEST_OBJS) $(APPS_LDFLAGS) -o $(DDTEST)

apps-dep:
	$(CC) -MM $(CFLAGS) $(APPS_CPPFLAGS) $(DDTEST_SRCS) 1>.depend

apps: apps-dep lib
	$(MAKE) $(DDTEST)

docs:
	$(MAKE) -C DOCS

docs-clean:
	$(MAKE) -C DOCS clean

clean:
	$(MAKE) -C src clean
	rm -f *.o
	rm -f $(DDTEST)
	rm -f .depend

distclean: clean docs-clean
	rm -f config.log
	rm -f config.mak
	rm -f $(DISTFILE)
	rm -f $(PKGCONFIG_FILE)

install: install-lib install-pkgconfig install-apps install-docs install-man

install-lib: lib
	$(MAKE) -C src install

install-pkgconfig: $(PKGCONFIG_FILE)
	$(INSTALL) -d "$(PKGCONFIG_DIR)"
	$(INSTALL) -m 644 $< "$(PKGCONFIG_DIR)"

install-apps: apps
	$(INSTALL) -d $(bindir)
	$(INSTALL) -c -m 755 $(DDTEST) $(bindir)

install-docs: docs
	$(MAKE) -C DOCS install

install-man: $(MANS)
	for m in $(MANS); do \
	  section=`echo $$m | sed -e 's/^.*\\.//'`; \
	  $(INSTALL) -d $(mandir)/man$$section; \
	  $(INSTALL) -m 644 $$m $(mandir)/man$$section; \
	done

uninstall: uninstall-lib uninstall-pkgconfig uninstall-apps uninstall-docs uninstall-man

uninstall-lib:
	$(MAKE) -C src uninstall

uninstall-pkgconfig:
	rm -f $(PKGCONFIG_DIR)/$(PKGCONFIG_FILE)

uninstall-apps:
	rm -f $(bindir)/$(DDTEST)

uninstall-docs:
	$(MAKE) -C DOCS uninstall

uninstall-man:
	for m in $(MANS); do \
	  section=`echo $$m | sed -e 's/^.*\\.//'`; \
	  rm -f $(mandir)/man$$section/$$m; \
	done

.PHONY: *clean *install* docs apps*

dist:
	-$(RM) $(DISTFILE)
	dist=$(shell pwd)/libduck-$(VERSION) && \
	for subdir in . $(SUBDIRS); do \
		mkdir -p "$$dist/$$subdir"; \
		$(MAKE) -C $$subdir dist-all DIST="$$dist/$$subdir"; \
	done && \
	tar cjf $(DISTFILE) libduck-$(VERSION)
	-$(RM) -rf libduck-$(VERSION)

dist-all:
	cp $(EXTRADIST) $(DDTEST_SRCS) Makefile $(DIST)

.PHONY: dist dist-all

#
# include dependency files if they exist
#
ifneq ($(wildcard .depend),)
include .depend
endif
