## Process this file with automake to generate Makefile.in

SUBDIRS = src include doc

EXTRA_DIST = pgplot_libs

export: dist
	scp $(PACKAGE)-$(VERSION).tar.gz $(WEB_SERVE):$(WEB_PATH)/software/.
	ssh $(WEB_SERVE) "cd $(WEB_PATH)/software ; \rm -f $(PACKAGE).tar.gz ; ln -s $(PACKAGE)-$(VERSION).tar.gz $(PACKAGE).tar.gz; \rm -fr $(PACKAGE); mkdir $(PACKAGE)"
	scp -r doc/html $(WEB_SERVE):$(WEB_PATH)/software/$(PACKAGE)/.
	ssh $(WEB_SERVE) "cd $(WEB_PATH)/software; sed -e 's%<\!-- observing version --><td>.*</td>%<\!-- observing version --><td>$(VERSION)</td>%' index.html > junk.html; mv junk.html index.html"
	ssh $(WEB_SERVE) "cd $(WEB_PATH)/software; sed -e 's%<\!-- observing date --><td>.*</td>%<\!-- observing date --><td>$(DATE)</td>%' index.html > junk.html; mv junk.html index.html"

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
