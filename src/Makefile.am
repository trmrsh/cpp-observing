## Process this file with automake to generate Makefile.in
##
## This is the file that must be edited if you are changing anything in the source directory

## Programs

DATE = $(shell date)

install-data-hook:
	echo "This is $(PACKAGE)-$(VERSION), built on $(DATE)" > $(progdir)/VERSION

progdir = @bindir@/@PACKAGE@

prog_PROGRAMS      = airmass eclipsers ephemeris starinfo whatphases

airmass_SOURCES    = airmass.cc
eclipsers_SOURCES  = eclipsers.cc
ephemeris_SOURCES  = ephemeris.cc
starinfo_SOURCES   = starinfo.cc
whatphases_SOURCES = whatphases.cc
 
AM_CPPFLAGS = -I../include -I../. $(PCRECPP_FLAGS)

LDADD    = libobserving.la $(PCRECPP_LIBS)

## Library

lib_LTLIBRARIES = libobserving.la 

libobserving_la_SOURCES = when_visible.cc suntime.cc startime.cc



