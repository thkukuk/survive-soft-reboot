#
# spec file for package sec-counter
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


Name:           sec-counter
Version:        20240821.99746da
Release:        0
Summary:        Service printing seconds since start
License:        Apache-2.0
URL:            https://github.com/thkukuk/survive-soft-reboot/
Source:         %{name}-%{version}.tar.xz
Source1:        LICENSE
BuildRequires:  meson
BuildRequires:  pkgconfig
BuildRequires:  pkgconfig(libsystemd)

%description
This package provides a service, which will print every second a message
since how many seconds it is already running to stdout and/or journald.

%prep
%setup -q

%build
cp -a %{SOURCE1} .
%meson
%meson_build

%install
%meson_install

%files
%license LICENSE
%{_libexecdir}/sec-counter
%{_unitdir}/sec-counter.service

%changelog
