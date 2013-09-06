## Process this file with automake to generate Makefile.in

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src include

export: dist
	scp $(PACKAGE)-$(VERSION).tar.gz $(WEB_SERVE):$(WEB_PATH)/software/.
	ssh $(WEB_SERVE) "cd $(WEB_PATH)/software ; ln -sf $(PACKAGE)-$(VERSION).tar.gz $(PACKAGE).tar.gz"

## File of aliases

ALIASES = Observing

.PHONY : $(ALIASES)

DATE    = $(shell date)

progdir = @bindir@/@PACKAGE@

prog_DATA = $(ALIASES)

$(ALIASES):
	@echo '#' > $(ALIASES)
	@echo '# This file defines aliases for the $(PACKAGE) v$(VERSION) software.' >> $(ALIASES)
	@echo '# Source it to define them.' >> $(ALIASES)
	@echo '#' >> $(ALIASES)
	@echo 'alias airmass    $(progdir)/airmass'    >> $(ALIASES)
	@echo 'alias eclipsers  $(progdir)/eclipsers'  >> $(ALIASES)
	@echo 'alias ephemeris  $(progdir)/ephemeris'  >> $(ALIASES)
	@echo 'alias starinfo   $(progdir)/starinfo'   >> $(ALIASES)
	@echo 'alias whatphases $(progdir)/whatphases' >> $(ALIASES)
	@echo ' ' >> $(ALIASES)
	@echo 'echo " "' >> $(ALIASES)
	@echo 'echo "Welcome to the $(PACKAGE) software, version $(VERSION), built $(DATE)."' >> $(ALIASES)
	@echo 'echo "This provides programs which may be useful while observing."' >> $(ALIASES)
	@echo 'echo " "' >> $(ALIASES)
	@echo 'echo "Commands available are: "' >> $(ALIASES)
	@echo 'echo " "' >> $(ALIASES)
	@echo 'echo "airmass, eclipsers, ephemeris, starinfo and whatphases"' >> $(ALIASES)
	@echo 'echo " "' >> $(ALIASES)
	@echo 'echo "See ${prefix}/html/$(PACKAGE)/index.html for help."' >> $(ALIASES)
	@echo 'echo " "' >> $(ALIASES)

clean-local:
	-rm -f $(ALIASES)
