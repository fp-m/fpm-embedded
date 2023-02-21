#
# make
# make all      -- build everything
#
# make test     -- run unit tests
#
# make install  -- install RP/M binaries to /usr/local
#
# make clean    -- remove build files
#

all:    pico/build unix/build
	$(MAKE) -C pico/build $@
	$(MAKE) -C unix/build $@
	$(MAKE) -C tests $@

test:
	$(MAKE) -C tests $@

install: pico/build unix/build
	$(MAKE) -C pico/build $@
	$(MAKE) -C unix/build $@

clean:
	if [ -d pico/build ]; then $(MAKE) -C pico/build clean; fi
	if [ -d unix/build ]; then $(MAKE) -C unix/build clean; fi
	$(MAKE) -C tests $@

pico/build:
	mkdir $@
	cmake -B $@ pico

unix/build:
	mkdir $@
	cmake -B $@ unix
