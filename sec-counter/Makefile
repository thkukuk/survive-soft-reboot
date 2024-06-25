all: sec-counter

sec-counter: sec-counter.c
	$(CC) -O2 -Wall -o $@ $^ -lsystemd

install: sec-counter
	mkdir -p $(DESTDIR)/usr/libexec/sec-counter/
	cp -a sec-counter $(DESTDIR)/usr/libexec/sec-counter/

clean:
	rm -f sec-counter *.o *~
