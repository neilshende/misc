TOPDIR = ../..
MXFSDIR = ${TOPDIR}/include

CCOMMON = -g -Wall
CCFLAGS = ${CCOMMON} -D_REENTRANT -I${MXFSDIR}
CLDFLAGS = ${CCOMMON} -lpthread

all: mxadmin

%.o: %.c
	gcc -c $(CCFLAGS) -o $@ $<

mxadmin: fs.o requests.o device.o main.o
	gcc $(CLDFLAGS) $^ -o $@

clean:
	rm -rf *.o mxadmin


