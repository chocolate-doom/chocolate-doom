# Lemon Doom

Lemon Doom is an extremely simple hack of Chocolate Doom to raise the limits in all four games to the levels of Doom-Plus, and nothing else (I previously erroneously stated that it raised the limits to the levels of Crispy Doom, but I was referring to version 1.0 of that fork. Crispy Doom has long ago become completely limit-removing). I really enjoy having raised limits (vanilla Chex Quest actually crashes during the final level in Chocolate Doom due to a visplane error, and this is not a bug), but wasn't personally comfortable with how the extra bells and whistles of Crispy Doom move it away from the pure classic DOOM experience (I even prefer playing the game at low resolution). Lemon Doom aims to be a middle-of-the-road option available for anyone interested. It just aims to be a well-maintained fork of Chocolate Doom that keeps in step with the main branch.

Lemon Doom is provided with no warranty whatsoever. If you find a bug, it should almost certainly be filed with the main Chocolate Doom fork, as my changes to the code were about as minimal as it gets. I don't know how to compile this on Windows, but I can help Linux users. Compiling and installing Lemon Doom on Ubuntu is as simple as running the following commands:

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

At one time, I was interested in making Lemon Doom Boom-compatible (probably by painstakingly merging the sources for Chocolate Doom & Boom v2.02), but have decided not to pursue this. My primary technological interest is in Unix System Administration, and large programming projects are beyond my scope. However, I do plan on making minor cosmetic tweaks, such as writing an appropriate man page for Unix-based systems. Anyone, like me, whose sense of order is upset by the fact that Crispy Doom's man page is an unedited clone of Chocolate Doom's will not have to lose any sleep over Lemon Doom's documentation. My goal is to make a professionally-presented hack of Chocolate Doom that fills a small gap.

There are actually a few enhancements that may lie ahead on the Lemon Doom roadmap. Eventually, I would be interested in removing the limits entirely (I'd probably do this by studying the Boom v2.02 source code), and, I'm interested in creating a proper MASTER LEVELS FOR DOOM II launcher using the same GUI toolkit as the Setup program which will also aim for support for Maximum Doom or any user-created database of WADs. If such enhancements happen, they probably lie in the distant future, as I am currently very busy studying for school & acquiring the necessary skillset to become a System Admin.
