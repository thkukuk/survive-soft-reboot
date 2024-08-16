all: btrfs-soft-reboot-generator

btrfs-soft-reboot-generator: btrfs-soft-reboot-generator.c
	$(CC) -O2 -Wall -o $@ $^ -lmount -leconf

install: btrfs-soft-reboot-generator
	mkdir -p $(DESTDIR)/usr/libexec/btrfs-soft-reboot-generator/
	cp -a btrfs-soft-reboot-generator $(DESTDIR)/usr/libexec/btrfs-soft-reboot-generator/

clean:
	rm -f btrfs-soft-reboot-generator *.o *~
