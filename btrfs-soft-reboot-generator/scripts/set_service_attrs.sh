#!/bin/sh

SERVICE="$1"

setfattr -n user.survive_final_kill_signal -v 1 "/sys/fs/cgroup/system.slice/$SERVICE/runtime"

for d in /sys/fs/cgroup/system.slice/"$SERVICE"/libpod-payload-* ; do
	setfattr -n user.survive_final_kill_signal -v 1 "$d"
done
