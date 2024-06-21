#
# spec file for package sec-counter-snapshot
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


Name:           sec-counter-snapshot
Version:        20240621.3ec0d56
Release:        0
Summary:        Demo to survive soft-reboot with snapshots
License:        Apache-2.0
Group:          System/Fhs
URL:            https://github.com/thkukuk/sec-counter
Source:         sec-counter-%{version}.tar.xz
BuildRequires:  pkgconfig
BuildRequires:  pkgconfig(libsystemd)

%description
This package contains the service files and application, to demonstrate
how an application can survive a soft-reboot. For this, the application
needs to be stored in a btrfs snapshot:
snapper create -p
systemctl start sec-counter@<snapshot id>.service

%prep
%setup -q -n sec-counter-%{version}

%build
pushd src
%make_build
popd

%install
pushd src
%make_install
popd
pushd btrfs-snapshot
mkdir -p %{buildroot}%{_unitdir}
install -m 0644 sec-counter@.service %{buildroot}%{_unitdir}/
install -m 0644 "system-sec\x2dcounter.slice" %{buildroot}%{_unitdir}/
mkdir -p %{buildroot}%{_systemdgeneratordir}
install -m 0755 sec-counter-btrfs-snapshot-generator %{buildroot}%{_systemdgeneratordir}/
popd

%files
%license LICENSE
%{_unitdir}/sec-counter@.service
%{_unitdir}/system-sec\x2dcounter.slice
%dir %{_systemdgeneratordir}
%{_systemdgeneratordir}/sec-counter-btrfs-snapshot-generator
%dir %{_libexecdir}/sec-counter/
%{_libexecdir}/sec-counter/sec-counter

%changelog
