
# cleanrules.make - rules for "make clean" targets

# $$$ build-specific cleanrules?
.PHONY: clean
clean::
	@list='$(SUBDIRS)'; for d in $$list; do \
	(echo "Cleaning/depending in $$d..." && cd $$d && $(MAKE) -s localclean && $(MAKE) -s depend); \
	done ;
	@echo "Cleaning/depending in local project..."
	@$(MAKE) -s localclean
	@$(MAKE) -s depend

.PHONY: cleanonly
cleanonly:
	@list='$(SUBDIRS)'; for d in $$list; do \
	(echo "ONLY Cleaning in $$d..." && cd $$d && $(MAKE) -s localclean); \
	done ;
	@echo "ONLY Cleaning in local project..."
	@$(MAKE) -s localclean
	
.PHONY: localclean
localclean:
	@rm -rf $(OBJDIR) $(THISTARGET) .depend.$(TARGET)$(FEATURESTRING)
