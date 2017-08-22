CROSS		=	
PYTHON		= python
CC		= $(CROSS)gcc
AR		= $(CROSS)ar:wq
CFLAGS		= -Wall -g
ARFLAGS		= rcv

SOURCES=pylibmmal.c
HEADERS=
OBJECTS=$(SOURCES:.c=.o)
TARGETS = pylibmmal.so

.PHONY:all clean example test install
.SILENT: clean

all:$(TARGETS) example

clean:
	find . -name "*.o" | xargs rm -f 
	$(RM) *.o *.so *~ a.out .depend $(TARGETS) build -rf

test:
	$(PYTHON) -m unittest discover tests 

install:
	$(PYTHON) setup.py install

pylibmmal.so: $(SOURCES) $(HEADERS)
	$(PYTHON) setup.py build_ext --inplace

.depend:$(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -MM $^ > $@

-include .depend
