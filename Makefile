CC			= gcc
CPPFLAGS 	:= -Wall --std=c99 --pedantic -O2
GTKCFLAGS	= `pkg-config --cflags gtk+-2.0`
BINNAME		= spring

.PHONY: clean install

$(BINNAME): $(BINNAME).c
	$(CC) $(CPPFLAGS) $(GTKCFLAGS) -o $@ $<
	
clean:
	rm -vf $(BINNAME)

install:
	cp $(BINNAME) /usr/bin/$(BINNAME)
