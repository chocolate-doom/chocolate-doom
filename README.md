
# RNG OF DOOM

Chcoclate doom but I played with the RNG. See ```EFFECTS.MD``` for full list of effects.


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
chocolate-doom -iwad doom2.wad -file nuts.wad
```
