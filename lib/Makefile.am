
SUBDIRS = include src


if HAVE_DOXYGEN

all-local: doc


doc: doxyfile
	$(DOXYGEN) $<


clean-local:
	rm -rf $(top_builddir)/lib/doc

endif

