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

test:   tests/build
	$(MAKE) -C tests/build all
	ctest --test-dir tests/build

install: pico/build unix/build
	$(MAKE) -C pico/build $@
	$(MAKE) -C unix/build $@

clean:
	if [ -d pico/build ]; then $(MAKE) -C pico/build clean; fi
	if [ -d unix/build ]; then $(MAKE) -C unix/build clean; fi
	if [ -d tests/build ]; then $(MAKE) -C tests/build clean; fi

pico/build:
	mkdir $@
	cmake -B $@ pico

unix/build:
	mkdir $@
	cmake -B $@ unix

tests/build:
	mkdir $@
	cmake -B $@ tests
