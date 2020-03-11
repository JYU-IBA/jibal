lib_LTLIBRARIES = libjibal.la
libjibal_la_SOURCES = phys.c masses.c units.c gsto.c material.c layer.c kin.c cross_section.c
libjibal_la_LDFLAGS = -version-info 0:0:0
pkginclude_HEADERS = jibal_phys.h jibal_masses.h jibal_units.h jibal_gsto.h jibal_material.h jibal_layer.h jibal_kin.h jibal_cross_section.h
noinst_HEADERS = defaults.h

AM_CFLAGS = -I$(top_srcdir) -DJIBAL_DATADIR=\"$(pkgdatadir)/\"

bin_PROGRAMS = get_stop get_kin
get_stop_SOURCES = get_stop.c
get_stop_LDADD = libjibal.la

get_kin_SOURCES = get_kin.c
get_kin_LDADD = libjibal.la
