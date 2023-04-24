- **What's the TRUECOLOR renderer?**

  Crispy Doom features an experimental Truecolor rendering mode, which needs to be explicitly enabled at compile time by running either `./configure --enable-truecolor` or `cmake -DCRISPY_TRUECOLOR`, respectively.

  Why is this still not enabled by default?

  - The three non-Doom games are in no way prepared for it.
  - This rendering mode only uses the first palette, so any mod with a custom PLAYPAL (or COLORMAP) will look differently than intended by its author.
  - It cannot use the foreground/background lookup table for translucency anymore, so all values for all three RGB channels would have to be calculated for each translucent pixel during each rendered frame, which consumes quite a lot of computing time.

- **How are the daily builds and releases created?**

  Daily builds are compiled snapshots of the Crispy Doom code base.
  They are provided courtesy of @fragglet and can be found here: http://latest.chocolate-doom.org/.
  Crispy Doom's daily builds are built in the same environment as Chocolate Doom's daily builds and releases.

  From version 6.0 on, Crispy Doom releases are automatically built in a MSYS2 environment using the latest GCC/MinGW compiler and library versions available at the time the release tag was pushed.

- **I am having sound issues on Windows, e.g. distorted or high-pitched SFX or music.**

  Mostly these issues aren't directly related to the port or its source code. Instead, these issues are most likely between SDL2(\_Mixer), your Windows version and/or your sound drivers. Please try the following steps:

  - In recent releases of the SDL library, the default SDL audio backend on Windows has been changed to "WASAPI", which reportedly causes some problems for some Windows users. However, it is still possible to explicitly change the audio backend back to "directsound" by [setting the "SDL_AUDIODRIVER" environment variable](https://wiki.libsdl.org/FAQUsingSDL#Win32-1). You can view or modify the environment variables by selecting **System** from the **Control Panel**, selecting **Advanced system settings**, and clicking **Environment Variables**.
  - Please try to locate the ["Advanced" tab in the "Speaker Properties" window](https://superuser.com/questions/698522/how-should-i-decide-on-a-default-audio-format) for your sound hardware and change the default format to something like "CD quality".
  - Please try to configure your speaker setup as plain stereo, even if you actually use a more advanced setup like e.g. 5.1 Surround.
  - It may help to disable SFX resampling by setting the `use_libsamplerate` key to `0` in `crispy-doom.cfg`.
  - It may help to update your sound card driver to a version that functions properly with the new Windows Audio Session API ("WASAPI") which SDL (>= 2.0.6) uses as its default backend.

- **I have problems downloading, extracting or executing the binary Windows releases.**

  Do you happen to have Avast installed? It erroneously claims our releases to be infected and blocks them from any reasonable action. If you want to be 100% sure, you may want to check our releases with an online malware detection tool like e.g. [VirusTotal](https://www.virustotal.com/#/home/upload).

  Furthermore, please make sure to start Crispy Doom from a directory to which you have write access (e.g. your Desktop), else it will be impossible for the executable to create its configuration files.
