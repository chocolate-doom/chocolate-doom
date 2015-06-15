
Name: cndoom
Summary: Conservative source port
Version: 2.0.3.2
Release: 1
Source: http://doom.com.hr/cndoom/cndoom-2.0.3.2.tar.gz
URL: http://www.doom.com.hr/
Group: Amusements/Games
BuildRoot: /var/tmp/cndoom-buildroot
License: GNU General Public License, version 2
Packager: Zvonimir Buzanic <admin@doom.com.hr>
Prefix: %{_prefix}
Autoreq: 0
Requires: libSDL-1.2.so.0, libSDL_mixer-1.2.so.0, libSDL_net-1.2.so.0

%prep
rm -rf $RPM_BUILD_ROOT

%setup -q

%build
./configure \
 	--prefix=/usr \
	--exec-prefix=/usr \
	--bindir=/usr/bin \
	--sbindir=/usr/sbin \
	--sysconfdir=/etc \
	--datadir=/usr/share \
	--includedir=/usr/include \
	--libdir=/usr/lib \
	--libexecdir=/usr/lib \
	--localstatedir=/var/lib \
	--sharedstatedir=/usr/com \
	--mandir=/usr/share/man \
	--infodir=/usr/share/info
make

%install
%makeinstall

%clean
rm -rf $RPM_BUILD_ROOT

%description
%(sed -n "/==/ q; p " < README)

See http://www.doom.com.hr/ for more information.

%package -n cnheretic
Summary: Conservative source port (Heretic binaries)
Group: Amusements/Games
Requires: libSDL-1.2.so.0, libSDL_mixer-1.2.so.0, libSDL_net-1.2.so.0

%files
%{_mandir}/man5/cndoom.cfg.5*
%{_mandir}/man5/default.cfg.5*
%{_mandir}/man6/cndoom.6*
%{_mandir}/man6/cnsetup.6*
%{_mandir}/man6/cnserver.6*
/usr/share/doc/cndoom/*
/usr/games/cndoom
/usr/games/cnserver
/usr/games/cndoom-setup
/usr/share/icons/*
/usr/share/applications/*

%description -n cnheretic
%(sed -n "/==/ q; p " < README)

These are the Heretic binaries.

See http://www.doom.com.hr/ for more information.

%files -n cnheretic
%{_mandir}/man5/cnheretic.cfg.5*
%{_mandir}/man5/heretic.cfg.5*
%{_mandir}/man6/cnheretic.6*
/usr/share/doc/cnheretic/*
/usr/games/cnheretic
/usr/games/cnheretic-setup

%package -n cnhexen
Summary: Conservative source port (Hexen binaries)
Group: Amusements/Games
Requires: libSDL-1.2.so.0, libSDL_mixer-1.2.so.0, libSDL_net-1.2.so.0

%description -n cnhexen
%(sed -n "/==/ q; p " < README)

These are the Hexen binaries.

See http://www.doom.com.hr/ for more information.

%files -n cnhexen
%{_mandir}/man5/cnhexen.cfg.5*
%{_mandir}/man5/hexen.cfg.5*
%{_mandir}/man6/cnhexen.6*
/usr/share/doc/cnhexen/*
/usr/games/cnhexen
/usr/games/cnhexen-setup

%package -n cnstrife
Summary: Conservative source port (Strife binaries)
Group: Amusements/Games
Requires: libSDL-1.2.so.0, libSDL_mixer-1.2.so.0, libSDL_net-1.2.so.0

%description -n cnstrife
%(sed -n "/==/ q; p " < README)

These are the Strife binaries.

See http://www.doom.com.hr/ for more information.

%files -n cnstrife
%{_mandir}/man5/cnstrife.cfg.5*
%{_mandir}/man5/strife.cfg.5*
%{_mandir}/man6/cnstrife.6*
/usr/share/doc/cnstrife/*
/usr/games/cnstrife
/usr/games/cnstrife-setup

