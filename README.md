# sec-counter

Since systemd v254 there is the nice feature of "systemctl soft-reboot" and since systemd v255 systemd services can even survive a soft-reboot.

This repository contains the code for a simple demonstration of this features. It consists of a binary printing every second a counter to stderr and/or journald. Additional there are several service files and additional tools depending on where the application is stored: on the root filesystem, a btrfs subvolume, a portable service or an image.

## Portable Image

### Build

To build the portable image, just call `mkosi` in the top directory

### Usage

Attach the image and start the service:
```
sudo portablectl attach ./sec-counter_*.raw
sudo start sec-counter.service 
```

Stop the service and remove the image:
```
sudo systemctl stop sec-counter.service
sudo portablectl detach ./sec-counter_*.raw
```
