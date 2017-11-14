CROSS		=	
PYTHON		= python
CC		= $(CROSS)gcc
AR		= $(CROSS)ar:wq
CFLAGS		= -Wall -g
ARFLAGS		= rcv
CODE_STYLE	= astyle --align-pointer=name --align-reference=name --suffix=none --break-blocks --pad-oper --pad-header --break-blocks --keep-one-line-blocks --indent-switches --indent=tab=4

SOURCES=$(wildcard src/*.c)
HEADERS=$(wildcard src/*.h)
OBJECTS=$(SOURCES:.c=.o)
TARGETS = pylibmmal.so

.PHONY:all clean example test style install python2_test python3_test
.SILENT: clean

all:$(TARGETS) example

clean:
	find . -name "*.o" | xargs rm -f 
	$(RM) *.o *.so *~ a.out .depend $(TARGETS) build dist *.egg-info -rf

test:
	make python2_test
	make python3_test

python2_test python3_test:clean $(TARGETS)
	$(PYTHON) -m unittest discover tests 


style:
	@find -regex '.*/.*\.\(c\|cpp\|h\)$$' | xargs $(CODE_STYLE)

install:
	$(PYTHON) setup.py install

pylibmmal.so: $(SOURCES) $(HEADERS)
	$(PYTHON) setup.py build_ext --inplace

.depend:$(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -MM $^ > $@

-include .depend

python2_test:PYTHON=python
python3_test:PYTHON=python3.5
