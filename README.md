# Survive a soft-reboot as systemd service

Since systemd v254 there is the nice feature of "systemctl soft-reboot" and since systemd v255 systemd services can even survive a soft-reboot.

This repository contains the code for demonstrating various methods to make use of this feature. The main tool is `btrfs-soft-reboot-generator`, which is able to generate all needed snippets/config options for a normal systemd service to get not killed during a soft-reboot. But there are other ways like the use of protable images, too.
The examples make use a binary and service `sec-counter`, which prints every second a counter to stderr and/or journald.

## BTRFS soft-reboot generator

With `transactional-update` (used e.g. on [openSUSE MicroOS](https://microos.opensuse.org/) or SL Micro 6.x) the root filesystem is a read-only btrfs subvolume. Instead of using the root filesystem (which can change with the next soft-reboot), the subvolume is directly used as image. The ID of the subvolume is auto-detected by the `btrfs-soft-reboot-generator`, which means that always the latest and most current root subvolume is used if a service gets started.

On openSUSE Tumbleweed a different method to manage the snapshots is used, see the section [Btrfs snapshot](#btrfs-snapshot) below for more information. `btrfs-soft-reboot-generator` can be used here, too, and is the preferred generic tool, with the disadvantage, that the subvolume ID cannot be autodetected but needs to be provided by the admin the configuration file.

### Build
Create a RPM which contains the btrfs-soft-reboot-generator systemd generator.

### Install
Install or update your RPM

### Usage
Create an ini-style config snippet with the service name (without the trailing .service) as group name. The generator will generate the missing pieces at next reboot or with `systemctl daemon-reload`.
The snippets should be stored in `/etc/btrfs-soft-reboot.conf.d` as `*.conf` file. [libeconf](https://github.com/openSUSE/libeconf) is used to read the configuration, so all other directories in the search path can be used, too.

### Examples

The `btrfs-soft-reboot-generator/examples` directory contains example config files for various services from openSUSE MicroOS and Tumbleweed.

An example for `rsyncd.service`:
```
[rsyncd]
BindPaths=/var/log
# Add volumes rsyncd should have access to
# BindPaths=/data
```


## Portable Image

A portable image is an image containing the minimal necessary tools to run the service and a systemd service file, which would be able to start the service. `portablectl` is used to *attach* or *detach* the image and make the service file available to the system, so that the admin can start/stop it. `portablectl` will take care to mount the most important directories into the image.

### Build

To build the portable image, just call `mkosi` in the top directory. If you call it as normal user, make sure that `/usr/sbin` is in your $PATH.

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
The service file needs the name of the root device as `RootImage=` entry. You can provide that with a drop in file or let the generator generate this.

### Build

Create a RPM which contains the `sec-counter` binary, the `sec-counter@.service` service, the `system-sec\x2dcounter.slice` slice and the `sec-counter-btrfs-snapshot-generator`.

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
