CC		= gcc -g
CXX             = g++ -std=c++17 -g
CFLAGS		= -O -Wall
CXXFLAGS	= $(CFLAGS)
DESTDIR		= /usr/local
PROG		= uf2fat
OBJS		= main.o

all:		$(PROG)

install:	$(PROG)
		install -s $(PROG) ${DESTDIR}/bin/$(PROG)
clean:
		rm -rf *~ *.o *.lst *.dis $(PROG) $(PROG).dSYM

$(PROG):	$(OBJS)
		$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
