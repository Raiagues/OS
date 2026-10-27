# Compatibility with the original course workflow: make -C src run.
.DEFAULT_GOAL := all
.PHONY: all
all:
	$(MAKE) -C .. all
%:
	$(MAKE) -C .. $@
