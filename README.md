# Sprinkled Doom

Welcome to the official Sprinkled Doom!

[![Sprinkled Doom](https://github.com/atsb/chocolate-doom-plus-plus/actions/workflows/main.yml/badge.svg)](https://github.com/atsb/chocolate-doom-plus-plus/actions/workflows/main.yml)

This project is a fork of Chocolate-Doom that aims to implement the ability to raise or decrease the static limits according to Doom+ or Doom v1.2 respectively. 

These are the static limits of Doom plus: http://prboom-plus.sourceforge.net/doom-plus.features.html

Link to the original hack: https://www.doomworld.com/idgames/utils/exe_edit/doomhack

Doom v1.2 had lower static limits than later versions, because they were raised to accomodate Doom 2 which has bigger levels. Doom v1.2 compatibility is not the main objective though, it's only to make it easy to change the limits. Some changes are documented here, but may not be accurate: https://tcrf.net/Doom_(PC,_1993)/Revisional_Differences

## How to build it by yourself

This project uses cmake and SDL2 libraries.  Ensure that (based on your platform of choice) you have
all the required dev libraries setup in your build system.  I am assuming that whoever wants to build
the software, is a developer themselves.  Users, please use the binaries provided.

Feel free to try to build it on your own for any learning excercises you may want to do, but
I cannot list every way to build this on all platforms.
