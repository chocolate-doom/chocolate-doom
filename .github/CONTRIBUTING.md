Thanks for contributing to Chocolate Doom! Whatever your contribution,
whether it's code or just a bug report, it's greatly appreciated.

The project is governed by the
[Contributor Covenant](http://contributor-covenant.org/version/1/4/)
version 1.4. By contributing to the project you agree to abide by its
terms. To report violations, please send an email to fraggle@gmail.com.

### Reporting bugs

Before reporting a bug, it's worth checking if this really is a bug.
Chocolate Doom's mission is to reproduce the Vanilla (DOS) versions of
the Doom engine games, bugs and all. Check out the
[NOT-BUGS](../NOT-BUGS.md) file for a list of common issues which aren't
really bugs at all. You might also try searching [the GitHub issues
list](https://github.com/chocolate-doom/chocolate-doom/issues) to see
if your bug has already been reported.

If you're confident that you've found a real bug (or even if you're
not sure!) please go ahead and [file an issue on
GitHub](https://github.com/chocolate-doom/chocolate-doom/issues/new).
You'll need a GitHub account, but it's pretty easy to sign up.

Please try to give as much information as possible:

* What version of Chocolate Doom are you using? Check the title bar of
  the window for the version number.

* Chocolate Doom runs on many different operating systems (not just
  Windows!). Please say which operating system and what version of it
  you're using.

* Please say which game you're playing (Doom 1, Doom 2, Heretic,
  Hexen, Strife, etc.) and if you're using any fan-made WADs or mods,
  please say which mods (and where they can be downloaded!). It helps
  to give the full command line you're using to start the game.

* Please mention if you have any special configuration you think may be
  relevant, too.

### Feature requests

Chocolate Doom is always open to new feature requests; however, please
be aware that the project is designed around a deliberately limited
[philosophy](../PHILOSOPHY.md), and many features common in other source
ports will not be accepted. Here are a few common requests which are
often rejected:

* "High resolution" rendering (greater than 320x200 display).

* An option to disable Vanilla limits, such as the visplane rendering
  limit.

* Ability to play "No Rest For The Living", the expansion pack which
  comes with the XBLA / BFG Edition of Doom.

If you're not sure whether your feature is in line with the project
philosophy, don't worry - just ask anyway!
To make a feature request, [file an issue on
GitHub](https://github.com/chocolate-doom/chocolate-doom/issues/new).

### Bug fixes / code submission

Thank you for contributing code to Chocolate Doom! Please check the
following guidelines before opening a pull request:

* All code must be licensed under [the GNU General Public License,
  version 2](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html).
  Please don't reuse code that isn't GPL, or that is GPLv3 licensed.
  Be aware that by submitting your code to the project, you're agreeing
  to license it under the GPL.

* Please follow the coding style guidelines described in the
  [HACKING](../HACKING.md) file.

* Please don't make unnecessary changes which just change formatting
  without any actual change to program logic. While being consistent
  is nice, such changes destroy the ability to use the `git blame`
  command to see when code was last changed.

* The guidelines given above in the "feature requests" section also
  apply here. New features which aren't in line with the project
  philosophy are likely to be rejected. If you're not sure, open a
  feature request first and ask before you start implementing your
  feature.

* Follow the guidelines for [how to write a Git commit
  message](http://chris.beams.io/posts/git-commit/). In short: the
  first line should be a short summary; keep to an 80 column limit;
  use the imperative mood ("fix bug X", rather than "fixed bug X" or
  "fixing bug X"). If your change fixes a particular subsystem,
  prefix the summary with that subsystem: eg. "doom: Fix bug X" or
  "textscreen: Change size of X".

* If you're making a change related to a bug, reference the GitHub
  issue number in the commit message, eg. "This is a partial fix
  for #646". This will link your commit into the issue comments. If
  your change is a fix for the bug, put the word "fixes" before the
  issue number to automatically close the issue once your change
  is merged.

