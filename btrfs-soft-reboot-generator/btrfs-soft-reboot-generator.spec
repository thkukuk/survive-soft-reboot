#
# spec file for package btrfs-soft-reboot-generator
#
# Copyright (c) 2024 SUSE LLC
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via https://bugs.opensuse.org/
#


Name:           btrfs-soft-reboot-generator
Version:        20240820.73bab8a
Release:        0
Summary:        systemd generator to let services survive a soft-reboot
License:        Apache-2.0
URL:            https://github.com/thkukuk/survive-soft-reboot/
Source:         %{name}-%{version}.tar.xz
Source1:        LICENSE
BuildRequires:  meson
BuildRequires:  pkgconfig
BuildRequires:  pkgconfig(libeconf)
BuildRequires:  pkgconfig(mount)

%description
This package contains a systemd generator, which will, based on ini style config
files, create system unit snippets during boot. This snippets contain the systemd 
unit config, so that this service will not be killed by a soft-reboot.

%prep
%setup -q

%build
cp -a %SOURCE1 .
%meson
%meson_build

%install
%meson_install

%files
%license LICENSE
%dir %{_systemdgeneratordir}
%{_systemdgeneratordir}/btrfs-soft-reboot-generator

%changelog