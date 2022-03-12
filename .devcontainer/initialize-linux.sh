#!/bin/bash
HERE=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )

### Create the xauth file allowing the container to interact with a forwarded X11
XAUTH="$HERE/share/devcontainer.xauth";
touch "$XAUTH"
xauth nlist $DISPLAY | sed -e 's/^..../ffff/' | xauth -f $XAUTH nmerge -

### TODO: pulseaudio pasthrough