# Chocolate-Doom-Installer

A unofficial installer for installing Chocolate Doom. Official project can be found here: [https://github.com/chocolate-doom/chocolate-doom](https://github.com/chocolate-doom/chocolate-doom)

## Features

- Admin rights are not required
- No registry edits made
- Completely uninstalls everything
- Adds start menu shortcuts

## Compiling from Source

1. Make sure you are on a windows computer running Windows 2000 or newer.
2. Download Chocolate Doom from here: [https://github.com/chocolate-doom/chocolate-doom/releases](https://github.com/chocolate-doom/chocolate-doom/releases) and select &quot;chocolate-doom-3.0.0-win32.zip&quot; then extract it.
3. Download this repository and extract it.
4. Your file tree must look like this for it to work:
* C:
* ---- Chocolate Doom Installer
    + doom.ico
    + setup.ico
    + INSTALL.BAT
    + UNINSTALL.BAT
    + Chocolate Doom Installer.sed
    + ---- chocolate-doom-3.0.0-win32
        + (Chocolate Doom Files)

5. In the windows search bar search for &quot;iexpress&quot; or &quot;iexpress.exe&quot; and right click and select &quot;Run as Administrator&quot;.
6. Select &quot;Open existing Self Extraction Directive file:&quot; and browse to &quot;Chocolate Doom Installer.sed&quot; located at &quot;C:\Chocolate Doom Installer\Chocolate Doom Installer.sed&quot;. If you get an error when you click next about not finding a file go back to step 4 and double check your file structure.
7. To compile just click &quot;Create Package&quot; click next then next again. The exe will be located at &quot;C:\Chocolate Doom Installer\Chocolate Doom Installer.exe&quot;.
8. (Optional) If you want to change the installer click &quot;Modify Self Extraction Directive file&quot; then click next to change it.
9. (Optional) If you want to change the installer icon use Resource Hacker found here: [http://www.angusj.com/resourcehacker/](http://www.angusj.com/resourcehacker/)

## Known Bugs

- If you use the UNINSTALL.BAT found under &quot;Documents\My Games\Chocolate Doom\UNINSTALL.BAT&quot; the folder Chocolate Doom won&#39;t be deleted. Using the uninstall shortcut found under the start menu works fine however.

