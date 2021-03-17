#TODO : Define SOURCES relative to %{_topdir}

%define HOME 			/homes/212694124/viswas
%define SOURCES			%{HOME}/SOURCES
%define SERVICE			/etc/systemd/system
%define SYSD			/etc/systemd


Summary:        Cgroup installer
Name:           cgroup_installer
License:        GEMS
Group:          Applications/utility
Distribution:   Mammo
Release:        1%{?dist}
Version:        1
Vendor:         viswas
Packager:       GE HealthCare
BuildArch:      x86_64


%description
Installation package for Cgroups in SLES

%prep
# we have no source, so nothing here
gcc -g %{SOURCES}/cgrules.c -std=c99 -lrt -lpthread -o %{SOURCES}/cgrules
gcc -g %{SOURCES}/live_process_tracker.c -std=c99 -lrt -lpthread -o %{SOURCES}/live_process_tracker


%install
rm -rf $RPM_BUILD_ROOT
install -d $RPM_BUILD_ROOT%{SYSD}
install -d $RPM_BUILD_ROOT%{SERVICE}
install -m 755 %{SOURCES}/cgconfig.service 	 	$RPM_BUILD_ROOT%{SERVICE}/cgconfig.service
install -m 755 %{SOURCES}/cgred.service 	        $RPM_BUILD_ROOT%{SERVICE}/cgred.service
install -m 755 %{SOURCES}/test_cgred.service 		$RPM_BUILD_ROOT%{SERVICE}/test_cgred.service
install -m 755 %{SOURCES}/slice_creator.sh              $RPM_BUILD_ROOT%{SYSD}/slice_creator.sh
install -m 755 %{SOURCES}/cgrules              		$RPM_BUILD_ROOT%{SYSD}/cgrules
install -m 755 %{SOURCES}/live_process_tracker          $RPM_BUILD_ROOT%{SYSD}/live_process_tracker



%clean
rm -rf $RPM_BUILD_ROOT

%post
#RPM is getting installed
if [ $1 == 1 ];then
	systemctl enable cgred.service
	systemctl enable cgconfig.service
	systemctl enable test_cgred.service
fi

%preun
if [ $1 == 0 ];then
        systemctl disable cgred.service
        systemctl disable cgconfig.service
        systemctl disable test_cgred.service
fi

%files
%defattr(-,root,root,-)
%{SERVICE}/cgconfig.service
%{SERVICE}/cgred.service
%{SERVICE}/test_cgred.service
%{SYSD}/slice_creator.sh
%{SYSD}/cgrules
%{SYSD}/live_process_tracker


%changelog
# let skip this for now

