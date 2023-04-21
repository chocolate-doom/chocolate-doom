- **What's the matter with daily builds and releases?**

  Daily builds are compiled snapshots of the Crispy Doom code base. They are provided as a courtesy of @fragglet and can be found here: http://latest.chocolate-doom.org/.

  Crispy Doom's daily builds are built in the same environment as Chocolate Doom's daily builds and releases. From version 5.5 on, Crispy Doom releases are nothing more than the daily builds from the same day the release was tagged in the GIT source code repository. This means that from version 5.5 on, both daily and released builds of both Chocolate and Crispy Doom are built in the same environment, using the same compiler and support libraries.

- **I am having sound issues on Windows, e.g. distorted or high-pitched SFX or music.**

  Mostly these issues aren't directly related to the port or its source code. Instead, these issues are most likely between SDL2(\_Mixer), your Windows version and/or your sound drivers. Please try the following steps:

  - In recent releases of the SDL library, the default SDL audio backend on Windows has been changed to "WASAPI", which reportedly causes some problems for some Windows users. However, it is still possible to explicitly change the audio backend back to "directsound" by [setting the "SDL_AUDIODRIVER" environment variable](https://wiki.libsdl.org/FAQUsingSDL#Win32-1). You can view or modify the environment variables by selecting **System** from the **Control Panel**, selecting **Advanced system settings**, and clicking **Environment Variables**.
  - Please try to locate the ["Advanced" tab in the "Speaker Properties" window](https://superuser.com/questions/698522/how-should-i-decide-on-a-default-audio-format) for your sound hardware and change the default format to something like "CD quality".
  - Please try to configure your speaker setup as plain stereo, even if you actually use a more advanced setup like e.g. 5.1 Surround.
  - It may help to disable SFX resampling by setting the `use_libsamplerate` key to `0` in `crispy-doom.cfg`.
  - It may help to update your sound card driver to a version that functions properly with the new Windows Audio Session API ("WASAPI") which SDL (>= 2.0.6) uses as its default backend.
  - Replacing the SDL2 DLLs with [older versions](https://github.com/fabiangreffrath/crispy-doom/wiki/Sound-pitch-fix-&-Music-Pack) may help.

- **The music volume slider also changes the SFX volume.**

  For some odd reasons it is impossible to set SFX and music volume separately for the same process in some versions of Windows. To mitigate this, make sure `crispy-midiproc.exe` is in the same directory (and of the same version) as `crispy-doom.exe` (in some releases, it was hidden in the `unsup/` subdirectory). However, if you experience erroneous music progression with certain music mods, try to move or rename `crispy-midiproc.exe` out of the way.

- **I have problems downloading, extracting or executing the binary Windows releases.**

  Do you happen to have Avast installed? It erroneously claims our releases to be infected and blocks them from any reasonable action. If you want to be 100% sure, you may want to check our releases with an online malware detection tool like e.g. [VirusTotal](https://www.virustotal.com/#/home/upload).

  Furthermore, please make sure to start Crispy Doom from a directory to which you have write access (e.g. your Desktop), else it will be impossible for the executable to create its configuration files.
