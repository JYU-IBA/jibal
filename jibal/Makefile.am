
#This file is originally copied from GNU scientific library (gsl/Makefile.am) and does the same thing. It links all headers (that will be installed to pkgincludedir) here so they can be included as #include <jibal/jibal_foo.h>. GSL is distributed under the terms of the GNU General Public License (GPL).


header-links: remove-links
	HEADERLIST="$(top_srcdir)/*/jibal*.h"; \
	for h in $$HEADERLIST; do \
	  BASENAME=`basename $$h`; \
	  test -r $$BASENAME || $(LN_S) $$h $$BASENAME; \
	done

remove-links: 
	rm -f jibal*.h


all: all-am header-links

clean-local: remove-links
distclean-local: remove-links
