# $Header: /soft/cvsroot/AliRoot/EVE/Makefile,v 1.1.1.1 2006/05/09 11:38:45 hristov Exp $

# Sorted by build order.
SUBDIRS  := Reve # Alieve
MKDEPINC := make_depend.inc

export REVESYS := ${CURDIR}

all:

${MKDEPINC}: Makefile
	@echo "Creating top-level dependencies."
	@echo "%:" > ${MKDEPINC}
	@for dir in ${SUBDIRS}; do \
	  echo -e "\t\$${MAKE} -C $$dir \$$@" >> ${MKDEPINC}; \
	done

-include ${MKDEPINC}

depend:: # This one descents into subdirs.
depend::
	@rm -rf ${MKDEPINC}
	@mkdir -p lib

distclean::
distclean::
	@rm -rf ${MKDEPINC}
	@rm -rf lib
