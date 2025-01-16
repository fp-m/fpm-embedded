CC		= gcc -g
CXX             = g++ -std=c++17 -g
CFLAGS		= -O -Wall -I../../include
CXXFLAGS	= $(CFLAGS)
DESTDIR		= /usr/local
PROG		= uf2fat
OBJS		= main.o dump.o format.o fatfs.o unicode.o diskio.o
VPATH           = ../../fatfs

all:		$(PROG)

install:	$(PROG)
		install -s $(PROG) ${DESTDIR}/bin/$(PROG)
clean:
		rm -rf *~ *.o *.lst *.dis $(PROG) $(PROG).dSYM

$(PROG):	$(OBJS)
		$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
