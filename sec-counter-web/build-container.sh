#!/bin/bash

set -e

UPLOAD=0

if [ "$1" = "--upload" ]; then
	UPLOAD=1
fi


VERSION=$(cat VERSION)
VER=( ${VERSION//./ } )

sudo podman pull registry.opensuse.org/opensuse/busybox:latest
sudo podman build --rm --no-cache --build-arg VERSION="${VERSION}" --build-arg BUILDTIME=$(date +%Y-%m-%dT%TZ) -t hello .
sudo podman tag localhost/hello localhost/hello:"${VERSION}"
sudo podman tag localhost/hello localhost/hello:latest
sudo podman tag localhost/hello localhost/hello:"${VER[0]}"
sudo podman tag localhost/hello localhost/hello:"${VER[0]}.${VER[1]}"
