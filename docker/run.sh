#!/bin/bash

set -eux

docker run --rm -it --privileged \
	-e DISPLAY=$DISPLAY \
	-v /tmp/.X11-unix:/tmp/.X11-unix:rw \
	-v "/home/$(whoami)/.Xauthority:/home/user/.Xauthority" \
	-w /home/user/blowmorph/ \
	--network host \
	--entrypoint bash \
	blowmorph
