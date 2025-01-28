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

all:    unix/build pico/build-rp2040 pico/build-rp2350-arm
	$(MAKE) -C unix/build $@
	$(MAKE) -C pico/build-rp2040 $@
	$(MAKE) -C pico/build-rp2350-arm $@

test:   tests/build
	$(MAKE) -C tests/build all
	ctest --test-dir tests/build

install: unix/build pico/build-rp2040 pico/build-rp2350-arm
	$(MAKE) -C unix/build $@
	$(MAKE) -C pico/build-rp2040 $@
	$(MAKE) -C pico/build-rp2350-arm $@

clean:
	rm -rf unix/build pico/build-rp2040 pico/build-rp2350-arm tests/build

pico/build-rp2040:
	mkdir $@
	cmake -B $@ pico -DRP2040=1

pico/build-rp2350-arm:
	mkdir $@
	cmake -B $@ pico -DRP2350_ARM=1

unix/build:
	mkdir $@
	cmake -B $@ unix

tests/build:
	mkdir $@
	cmake -B $@ tests #-DCMAKE_BUILD_TYPE=Debug
