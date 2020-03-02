
header-links: remove-links
#	HEADERLIST="$(top_srcdir)/jibal*.h $(top_srcdir)/*/jibal*.h"; \
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
