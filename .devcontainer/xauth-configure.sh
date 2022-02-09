#!/bin/sh

# Specify arg of path to create the xauth at

touch $1;
xauth nlist $DISPLAY | sed -e 's/^..../ffff/' | xauth -f $1 nmerge -
