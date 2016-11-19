# Lemon Doom

Hey guys, here's my extremely simple hack of Chocolate Doom to raise the limits to the level of Crispy Doom, and nothing else. I really enjoy having raised limits (vanilla Chex Quest actually crashes during the final level in Chocolate Doom due to a visplane error, and this is not a bug), but wasn't personally comfortable with how the extra bells and whistles of Crispy Doom move it away from the pure classic DOOM experience (I even prefer playing the game at low resolution). Lemon Doom aims to be a middle-of-the-road option available for anyone interested. My programming skills are firmly at a "hobbyist" level, and a project the size of Chocolate Doom makes my head spin. There will likely not be any major updates to Lemon Doom. The one feature I'd like to perhaps implement is support for NERVE.WAD the way Crispy Doom does it.

Lemon Doom is provided with no warranty whatsoever. If you find a bug, it should almost certainly be filed with the main Chocolate Doom fork, as my changes to the code were about as minimal as it gets. I don't know how to compile this on Windows, but I can help Linux users. Compiling Lemon Doom on Debian/Ubuntu systems is as simple as running the following commands:

```
sudo apt-get install git
sudo apt-get install build-essential automake
sudo apt-get build-dep chocolate-doom
git clone https://github.com/aynrandjuggalo1/lemon-doom.git
cd lemon-doom
autoreconf -vif
./configure
make
sudo make install
```
