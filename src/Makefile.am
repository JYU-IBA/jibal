lib_LTLIBRARIES = libjibal.la
libjibal_la_SOURCES = phys.c masses.c units.c gsto.c
libjibal_la_LDFLAGS = -version-info 0:0:0
pkginclude_HEADERS = jibal_phys.h jibal_masses.h jibal_units.h jibal_gsto.h
noinst_HEADERS = defaults.h

AM_CFLAGS = -I$(top_srcdir)

bin_PROGRAMS = get_stop
get_stop_SOURCES = get_stop.c
get_stop_LDADD = libjibal.la
