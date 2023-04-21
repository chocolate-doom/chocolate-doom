Crispy Doom is just the right middle ground between the original DOS Doom games and Doom source ports that are centered in modified/updated graphics and features.

For Mac, there unfortunately isn't an installer containing an app that we can just drag and drop into our /Applications folder, double-click, and play.  To play Crispy Doom, you have to compile the software yourself from the source code.  But this isn't a bad thing; Crispy Doom is an "open-source" program, meaning that the code which makes it function is free and available to the public.  This allows people a lot more knowledgeable and talented than myself to maintain and update it.  It also allows for greater customization and personalization.

My programming knowledge is very limited.  Prior to installing Crispy Doom, I had never done this before.  But after combing through the internet for some guides and explanations, I was able to figure it out.  Then I decided to make a plain English, step by step guide so that other people could easily install and enjoy this program; even the not so tech-savvy.

This may be your first experience using the terminal (found in /Applications/Utilities) on your Mac, which I know is intimidating.  However, you'll quickly realize what a useful tool it can be.

## Step 1: GAMES

First things first, you need to have the games in order to use Crispy Doom.  They are available for purchase through both Steam and GOG.  If you don't want to pay for them, there are free shareware versions available to play. Links to these can be found on the Crispy Doom homepage:

[https://github.com/fabiangreffrath/crispy-doom](https://github.com/fabiangreffrath/crispy-doom)

Each game comes as a file with the extension ".wad".  So hold onto those for now.


## Step 2: SETUP

### 2A: System Settings
While installing Crispy Doom, we're going to need to be able to see all of the files on your computer.  By default, many of the files that work behind the scenes are hidden from you.  To make these visible, there is an easy shortcut.  In finder, hold down the command and shift keys and type a period [CMD + SHIFT + .].  To change the default setting to always show hidden files, enter this command into your terminal:

`defaults write com.apple.finder AppleShowAllFiles -bool TRUE; killall Finder;`

To change the setting back, enter:

`defaults write com.apple.finder AppleShowAllFiles -bool FALSE; killall Finder;`

### 2B: XCode
Next, you need to install "XCode", which is a program from Apple used in software development.  It can be downloaded for free directly from the app store, but in my experience that takes too long.  Not to mention that Apple's download/installation progress bar doesn't give too much information (I can only stare at that circle and wait for so long before I think it froze).  So it can also be found on the Apple site at this link:

[https://developer.apple.com/download/release/](https://developer.apple.com/download/release/)

Sign in with your AppleID, download the app and install it.

### 2C: Homebrew
#### Part 1
The next program we'll need to install is Homebrew.  Homebrew is a package-manager which can be used to install/uninstall various software onto your computer.  Install Homebrew using the instructions at their website here:

[https://brew.sh/](https://brew.sh/)

#### Part 2
This next step I can't remember if Homebrew automatically does for you or not, but there's an easy way to check.  When you tell your computer to run an executable file, it looks for the program's files in several folders.  This is called your system path.  It checks the system path in a set order, and runs the first file it comes across that matches.  So even if you have two copies of "Program A", whichever one is first in the system path is the one that will be run.  Essentially, we need to tell the computer to look in the folders that Homebrew uses first when it's looking to run a file.

In your terminal, type this command:
`echo $PATH`

If your output says:
`/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin`

Then you're good to go; Homebrew automatically told the computer to look in it's folder first and you can skip Part 3.

If the output says:
`/usr/bin:/bin:/usr/sbin:/sbin`

Then we have to manually tell the computer what to do, so keep reading.

#### Part 3
Originally, computers were entirely text-based.  You interacted with the operating system through typing commands into a Terminal; this is known as a "command line interface (CLI)".  Now, operating systems usually interact with you through pictures and mouse clicks; these are called "Graphical User Interfaces (GUIs)".  Just like your can tell your computer to open an app by clicking on it (a GUI), you can tell it to open the same app by entering a specific command into the Terminal (a CLI).  Your terminal application is really just a CLI for interacting with your computer.

The program that interprets the written commands in a CLI is called the "shell".  We are going to edit your "shell profile", which gives instructions to the shell about how to interact with your computer.  There are several popular ones in use.  When Mac updated to Catalina, they started using "zsh" as the default shell; prior to that, Mac had always used the "bash" shell.  So based on what operating system you're using, follow the corresponding instructions.

##### Bash:
We are going to be editing the .bash_profile.  Shells refer to folders as "directories", so from here on out I will be using that term when working in terminal as well.  Open Finder, and go to your home directory.  Look for a file called ".bash_profile".  If it's there, double click it and open it.  If not, which is most likely the case, we're going to create it.

To create the file, open up your Terminal application.  It should automatically be in your home directory, but just in case we're going to be sure. Type "cd" into the terminal and hit enter.  "cd" stands for change directory, or go to this folder.  Entering "cd" on it's own will bring your terminal back to the home directory by default.  Now type "touch .bash_profile".  This tells your computer "create an empty filer and name it .bash_profile."  Now go back to your Finder window and open the file.

Type this line into the file, save it and close it:

`export PATH=/usr/local/bin:$PATH`

This line tells the computer "I want the new system path to be this: take Homebrew's folder, and put it at the front of the default system path".

Go back into Terminal, and in the home directory run the command:

`source .bash_profile`

This means "reload the instructions you use on what to do", since we just changed them. Minimize or exit out of your terminal, and go to the next part.

##### Zsh:
We are going to be editing the .zshrc.  Shells refer to folders as "directories", so from here on out I will be using that term when working in terminal as well.  Open Finder, and go to your home directory.  Look for a file called ".zshrc".  If it's there, double click it and open it.  If not, which is most likely the case, we're going to create it.

To create the file, open up your Terminal application.  It should automatically be in your home directory, but just in case we're going to be sure. Type "cd" into the terminal and hit enter.  "cd" means change directory, or go to this folder.  Entering "cd" on it's own will bring your terminal back to the home directory by default.  Now type "touch .zshrc".  This tells your computer "create an empty file in the home directory, and name it .zshrc."  Now go back to your Finder window and open the file.

Type this line into the file, save it and close it:

`export PATH=/usr/local/bin:$PATH`

This line tells the computer "I want the new system path to be this: take Homebrew's folder, and put it at the front of the default system path".

Go back into Terminal, and in the home directory run the command:

`source .zshrc`

This means "reload the instructions you use on what to do", since we just changed them. Minimize or exit out of your terminal, and go to the next part.

#### Part 4
Crispy Doom needs certain software installed on your system in order to work correctly.  So we need to install that software ourself, and Homebrew is the tool we are going to use.  Enter this command into the terminal:

`brew install libpng; brew install libsamplerate; brew install sdl2; brew install sdl2_mixer; brew install sdl2_net; brew install pkg-config`

One by one, this command installs the software you'll need for running Crispy Doom.

For MP3 & FLAC sound support, `brew install sdl2_mixer` (homebrew's current way of installing the sdl2_mixer package) will not work. Instead, install sdl2_mixer with `brew install --with-flac --with-mpg123 https://raw.githubusercontent.com/Homebrew/homebrew-core/a9114e/Formula/sdl2_mixer.rb` (or any other alternative brew formula).

### 2D: Crispy Doom
Last but not least, we need to get the source code for Crispy Doom.  Go to the Crispy Doom homepage and click the green "clone or download" button.  Click "download ZIP" and download the file.  Then in your downloads folder double click it to "unzip" the contents.  Now you'll have a folder titled "crispy-doom-master", and we're ready to begin compiling.


## Step 3: BUILD

We have reached the point where we will begin compiling the software.  First, we're going to create some folders to work in.  Open up your terminal, and type this command, except replace $USER with your username:

`mkdir -p /Users/$USER/CrispyDoom/Compile /Users/$USER/CrispyDoom/WADS`

This command tells the computer to make a directory "CrispyDoom" in your User directory, and it should contain two empty directories, "Compile" and "WADS".

Now we're going to move the terminal to that folder, so type:

`cd /Users/$USER/CrispyDoom/Compile`

Open Finder and go to the newly created CrispyDoom folder.  Drag and drop the "crispy-doom-master" folder into "Compile" (you can delete the zip version).  Also, take your .WAD files from earlier and drop them into the WADS folder.

Back to the terminal.  This process is thankfully extremely easy because the majority of it is automated, we only need to run a few commands. Enter the Compile/crispy-doom-master directory:

`cd crispy-doom-master`

Then type:

`./autogen.sh`

This generates the next file we'll need.  Once it is finished, enter these commands one by one, waiting for each to finish before running the next.

```
./configure
make
make install
```

And you have officially installed your first self-compiled program!

Hang on to the crispy-doom-master folder, because if you ever want to uninstall it, all you have to do is enter that directory in terminal and type: `make uninstall`.

Running Crispy Doom will need to be done from the command line, but consider creating some shell scripts to automate the process for you, that way it's just an icon you can double click.

To run the Crispy Doom settings file, enter this into terminal:

`/usr/local/bin/crispy-doom-setup`

These settings will be saved in the /Users/$USER/Library/Application Support/crispy-doom directory.

To play the games, enter:

`/usr/local/bin/crispy-doom` (followed by the path to your WAD file).

For example,

`/usr/local/bin/crispy-doom /Users/$USER/CrispyDoom/WADS/doom.wad`

**Enjoy!**
