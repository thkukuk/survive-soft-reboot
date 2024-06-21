# sec-counter

Since systemd v254 there is the nice feature of "systemctl soft-reboot" and since systemd v255 systemd services can even survive a soft-reboot.

This repository contains the code for a simple demonstration of this features. It consists of a binary printing every second a counter to stderr and/or journald. Additional there are several service files and additional tools depending on where the application is stored: on the root filesystem, a btrfs subvolume, a portable service or an image.

## Portable Image

A portable image is an image containing the minimal necessary tools to run the service and a systemd service file, which would be able to start the service. `portablectl` is used to *attach* or *detach* the image and make the service file available to the system, so that the admin can start/stop it. `portablectl` will take care to mount the most important directories into the image.

### Build

To build the portable image, just call `mkosi` in the top directory

### Usage

Attach the image and start the service:
```
sudo portablectl attach ./sec-counter_*.raw
sudo systemctl start sec-counter.service
```

Watch the output:
```
sudo journalctl -u sec-counter -f
```

Stop the service and remove the image:
```
sudo systemctl stop sec-counter.service
sudo portablectl detach ./sec-counter_*.raw
```

## Image Based

Image based is similar to __portable image__, except that there is no tool to attach it. This is done with the service file for the service, which must be provided separately.
The service file is also responsible to mount needed directories, like `/run/systemd/journal` to write into the journal file.

### Build

There are many tools for building such images, like `mkosi` or `kiwi`.

### Install

Copy `image/sec-counter.service` to `/etc/systemd/system` and adjust the path to the image.

### Usage

Start the service, which will also mount the image:
```
sudo systemctl start sec-counter.service
```

Watch the output:
```
sudo journalctl -u sec-counter -f
```

Stop the service:
```
sudo systemctl stop sec-counter.service
```

## Btrfs snapshot

An alternative to using images is to use btrfs snapshots created with `snapper`. This is the preferred solution on openSUSE Tumbleweed, since this creates a static version of the current root filesystem. Which means, it's as current as the root filesystem, but further updates to the root filesystem does not impact the service.

### Build

Create a RPM which contains the `sec-counter` binary, the `sec-counter@.service` service and the `system-sec\x2dcounter.slice` slice.

### Install

Install or update your RPM

### Usage

Start the service, which will also mount the image:
```
sudo snapper create -p
sudo systemctl start sec-counter@<snapshot id>.service
```

Watch the output:
```
sudo journalctl -u sec-counter@<snapshot id> -f
```

Stop the service:
```
sudo systemctl stop sec-counter@<snapshot id>.service
```
