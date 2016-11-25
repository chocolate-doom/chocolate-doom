# Lemon Doom

Hey guys, here's my extremely simple hack of Chocolate Doom to raise the limits in all four games to the levels of Doom-Plus, and nothing else (I previously erroneously stated that it raised the limits to the levels of Crispy Doom, but I was referring to version 1.0 of that fork. Crispy Doom has long ago become completely limit-removing). I really enjoy having raised limits (vanilla Chex Quest actually crashes during the final level in Chocolate Doom due to a visplane error, and this is not a bug), but wasn't personally comfortable with how the extra bells and whistles of Crispy Doom move it away from the pure classic DOOM experience (I even prefer playing the game at low resolution). Lemon Doom aims to be a middle-of-the-road option available for anyone interested. My programming skills are firmly at a "hobbyist" level, and a project the size of Chocolate Doom makes my head spin. There will likely not be any major updates to Lemon Doom. The one feature I'd like to perhaps implement is support for NERVE.WAD the way Crispy Doom does it.

Lemon Doom is provided with no warranty whatsoever. If you find a bug, it should almost certainly be filed with the main Chocolate Doom fork, as my changes to the code were about as minimal as it gets. I don't know how to compile this on Windows, but I can help Linux users. Compiling and installing Lemon Doom on Debian/Ubuntu systems is as simple as running the following commands:

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

After a couple days basking in the glow of my incredible hack, I've begun to feel more ambitious. Lemon Doom will now aim to build upon what I perceive as the rock-solid base of Chocolate Doom and create a fork that is Boom-compatible. Yes! Possibly the most CONSERVATIVE source port in the history of gaming combines with a classic source port that was conservative for its time, released under a very LIBERAL license to create the ultimate bipartisan DOOM experience! It'll blow your gourd! Lemon Doom (which could very well end up being called "Chocolate Boom", but I like "Lemon Doom" for now) will attempt a merger between the Chocolate Doom code and the classic code for Boom v2.02. My vision is to have it do for the classic Boom executable what Chocolate Doom does for vanilla -- as accurate a reproduction as possible, though I am also more "liberal-minded" than the original Chocolate Doom, and am open to certain enhancements within reason. Lemon Doom / Chocolate Boom is a conservative source port at heart. Lemon Doom will likely not incorporate any enhancements from later Boom derivatives such as PrBoom+ or even MBF. As I work to achieve this goal, Lemon Doom will likely be in an extremely experimental and unuseable state. Compile the "chocolate-boom" branch at your own peril!
