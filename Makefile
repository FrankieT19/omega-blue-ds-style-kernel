PYTHON ?= python

.PHONY: all flashable
all:
	$(PYTHON) tools/build.py

flashable:
	$(PYTHON) tools/build.py --flashable
