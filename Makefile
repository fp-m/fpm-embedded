#
# make
# make all      -- build everything
#
# make test     -- run unit tests
#
# make install  -- install FP/M binaries to /usr/local
#
# make clean    -- remove build files
#

all:    unix/build pico/build
	$(MAKE) -C unix/build $@
	$(MAKE) -C pico/build $@

test:   tests/build
	$(MAKE) -C tests/build all
	ctest --test-dir tests/build

install: unix/build pico/build
	$(MAKE) -C unix/build $@
	$(MAKE) -C pico/build $@

clean:
	rm -rf unix/build pico/build tests/build

pico/build:
	mkdir $@
	cmake -B $@ pico

unix/build:
	mkdir $@
	cmake -B $@ unix

tests/build:
	mkdir $@
	cmake -B $@ tests #-DCMAKE_BUILD_TYPE=Debug
