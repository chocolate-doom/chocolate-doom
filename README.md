
# RNG OF DOOM

A extention of chocolate doom where you can edit the rng with Dehacked files.

```
Patch File for DeHackEd v3.0
Doom version = 19
Patch format = 6


# RNG table uses misc section
Misc 0
RNG Table = [1,2,3,4,5] # inside a list
```


## Installation

installation for linux (debian)

```bash
# install chocolate doom
sudo apt install chocolate-doom -y
```

```bash
# install dependancies
sudo apt install automake autoconf libtool git pkg-config -y
```

```bash
# replace all of chocolate dooms files
git clone https://github.com/someStranger8/RNG-OF-DOOM
cd RNG-OF-DOOM
make
make install
chocolate-doom -iwad doom2.wad -file nuts.wad -deh crazy_rng.deh
```
