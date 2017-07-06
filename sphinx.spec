#
# spec file for sphinx 
#

%define vertag		-beta
%define sphinx_user	sphinx
%define sphinx_group	sphinx

Name:           sphinx
Version:        2.2.2
Release:        1%{?dist}
Summary:        Sphinx full-text search server

Group:          Applications/Text
License:        GPLv2
URL:            http://sphinxsearch.com
Vendor:         Sphinx Technologies Inc.
Packager:       Sphinx Technologies Inc.

Source0:        http://sphinxsearch.com/snapshots/%{name}-%{version}%{vertag}.tar.gz
Source1:        %{name}.init
Source2:	http://snowball.tartarus.org/dist/libstemmer_c.tgz
Source3:	re2.tar.gz
Patch0:		config-main.patch
Patch1:		config.patch
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-build

BuildRequires:  mysql-devel
BuildRequires:  postgresql-devel
BuildRequires:	expat-devel
BuildRequires:  unixODBC-devel

Requires(post): chkconfig
Requires(preun): chkconfig
# This is for /sbin/service
Requires(preun): initscripts

%description
Sphinx is a full-text search server that adds many advanced features
on top of plain old text searching. Data can be fetched directly from
a database, or streamed in XML format. MySQL, PostgreSQL, SQL Server,
Oracle, and other databases are supported. Client programs can query
Sphinx either using native SphinxAPI library, or using regular MySQL
client programs and libraries via SQL-like SphinxQL interface.


%package -n sphinx-testing
Summary:	Internal Sphinx tests
Group:		Applications/Text
Requires:	sphinx, mysql-server, php-mysql, php-cli, php-xml

%description -n sphinx-testing
Internal Sphinx tests


# %package -n sphinx-relocate
# Summary:	Relocatable (path independent) Sphinx binaries
# Group:		Applications/Text
# Provides:	sphinx
# Prefix:		/usr
#
# %description -n sphinx-relocate
# This is a binary-only package that can be installed by a plain user,
# without root privileges. It does not include any docs, examples, or APIs.
# Also, relocatable binaries do not have any built-in paths, and require
# an explicit config path to be specified in command line.


# %package -n sphinx-max
# Summary:        Sphinx with support of mysql, posgresql, and expat sources
# Group:          Applications/Text
# BuildRequires:  mysql-devel
# BuildRequires:  postgresql-devel
# BuildRequires:  expat-devel


%prep
%setup -n %{name}-%{version}%{vertag}
%patch0 -p1
%patch1 -p1
%setup -D -T -a 2 -n %{name}-%{version}%{vertag}
%setup -D -T -a 3 -n %{name}-%{version}%{vertag}
#tar -xvf re2.tar.gz ../re2
#cp -R ../re2/* libre2/
#rm -Rf ../re2/

# Fix wrong-file-end-of-line-encoding
sed -i 's/\r//' api/ruby/spec/sphinx/sphinx_test.sql
sed -i 's/\r//' api/java/mk.cmd
sed -i 's/\r//' api/ruby/spec/fixtures/keywords.php
sed -i 's/\r//' api/ruby/lib/sphinx/response.rb


%build

%configure --sysconfdir=/etc/sphinx --with-mysql --with-re2 --with-libstemmer --with-unixodbc --with-iconv --enable-id64 --with-pgsql --with-syslog
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT INSTALL="%{__install} -p -c"

# Install sphinx initscript
install -p -D -m 0755 %{SOURCE1} $RPM_BUILD_ROOT%{_initrddir}/searchd

# Create /var/log/sphinx
mkdir -p $RPM_BUILD_ROOT%{_localstatedir}/log/sphinx

# Create /var/run/sphinx
mkdir -p $RPM_BUILD_ROOT%{_localstatedir}/run/sphinx

# Create /var/lib/sphinx
mkdir -p $RPM_BUILD_ROOT%{_localstatedir}/lib/sphinx

# Create sphinx.conf
cp $RPM_BUILD_ROOT%{_sysconfdir}/sphinx/sphinx-min.conf.dist \
    $RPM_BUILD_ROOT%{_sysconfdir}/sphinx/sphinx.conf

# Copy the test suite
mkdir -p $RPM_BUILD_ROOT%{_datadir}/sphinx/src
mkdir -p $RPM_BUILD_ROOT%{_datadir}/sphinx/test
cp -R test/* $RPM_BUILD_ROOT%{_datadir}/sphinx/test/
cp src/tests $RPM_BUILD_ROOT%{_datadir}/sphinx/src/

# Copy the api
mkdir -p $RPM_BUILD_ROOT%{_datadir}/sphinx/api
cp -R api/* $RPM_BUILD_ROOT%{_datadir}/sphinx/api/

# Updates sphinx.conf paths
sed -i 'sX/var/log/searchd.pidX/var/run/sphinx/searchd.pidXg' \
$RPM_BUILD_ROOT%{_sysconfdir}/sphinx/sphinx.conf

sed -i 'sX/var/log/X/var/log/sphinx/Xg' \
$RPM_BUILD_ROOT%{_sysconfdir}/sphinx/sphinx.conf

sed -i 'sX/var/data/X/var/lib/sphinx/Xg' \
$RPM_BUILD_ROOT%{_sysconfdir}/sphinx/sphinx.conf

# Create /etc/logrotate.d/sphinx
mkdir -p $RPM_BUILD_ROOT%{_sysconfdir}/logrotate.d
cat > $RPM_BUILD_ROOT%{_sysconfdir}/logrotate.d/sphinx << EOF
/var/log/sphinx/*.log {
       weekly
       rotate 10
       copytruncate
       delaycompress
       compress
       notifempty
       missingok
}
EOF

# Install the testing suite
# TODO?


%clean
rm -rf $RPM_BUILD_ROOT


%post
# register service
if [ "$1" = 1 ];
then
    /sbin/chkconfig --add searchd
fi

# create user/group, and update permissions
groupadd -r %{sphinx_group} 2>/dev/null || true
useradd -M -r -d /var/lib/sphinx -s /bin/bash -c "Sphinx server" -g %{sphinx_group} %{sphinx_user} 2>/dev/null || true
usermod -g %{sphinx_group} %{sphinx_user} 2>/dev/null || true
chown -R %{sphinx_user}:%{sphinx_group} /var/lib/sphinx /var/log/sphinx /var/run/sphinx

# print some further pointers
echo
echo "Sphinx installed!"
echo "Now create a full-text index, start the search daemon, and you're all set."
echo
echo "To manage indexes:"
echo "    editor %{_sysconfdir}/sphinx/sphinx.conf"
echo
echo "To rebuild all disk indexes:"
echo "    sudo -u sphinx indexer --all --rotate"
echo
echo "To start/stop search daemon:"
echo "    service searchd start/stop"
echo
echo "To query search daemon using MySQL client:"
echo "    mysql -h 0 -P 9306"
echo "    mysql> SELECT * FROM test1 WHERE MATCH('test');"
echo
echo "See the manual at /usr/share/doc/sphinx-%{version} for details."
echo
echo "For commercial support please contact Sphinx Technologies Inc at"
echo "http://sphinxsearch.com/contacts.html"
echo


%post -n sphinx-testing
if [ "$1" = 1 ];
then
    ln -s `which searchd` /usr/share/sphinx/src/searchd
    ln -s `which indexer` /usr/share/sphinx/src/indexer
    ln -s `which spelldump` /usr/share/sphinx/src/spelldump
fi


%preun
if [ "$1" = 0 ] ; then
    /sbin/service searchd stop >/dev/null 2>&1
    /sbin/chkconfig --del searchd
fi


%files
%defattr(-,root,root,-)
/usr/share/man/man1/indexer.1.gz
/usr/share/man/man1/indextool.1.gz
/usr/share/man/man1/searchd.1.gz
/usr/share/man/man1/spelldump.1.gz
/usr/local/include/re2/filtered_re2.h
/usr/local/include/re2/re2.h
/usr/local/include/re2/set.h
/usr/local/include/re2/stringpiece.h
/usr/local/include/re2/variadic_function.h
/usr/local/lib/libre2.a
/usr/local/lib/libre2.so
/usr/local/lib/libre2.so.0
/usr/local/lib/libre2.so.0.0.0
%doc COPYING doc/sphinx.html doc/sphinx.txt sphinx-min.conf.dist sphinx.conf.dist example.sql
%{_datadir}/sphinx/api/*
%dir %{_sysconfdir}/sphinx
%config(noreplace) %{_sysconfdir}/sphinx/sphinx.conf
%exclude %{_sysconfdir}/sphinx/*.conf.dist
%exclude %{_sysconfdir}/sphinx/example.sql
%{_initrddir}/searchd
%config(noreplace) %{_sysconfdir}/logrotate.d/sphinx
%{_bindir}/*
%dir %{_localstatedir}/log/sphinx
%dir %{_localstatedir}/run/sphinx
%dir %{_localstatedir}/lib/sphinx

%files -n sphinx-testing
%{_datadir}/sphinx/test/*
%{_datadir}/sphinx/src/*
# TODO?


# %files -n sphinx-relocate
# %defattr(-,root,root,-)
# %{_bindir}/*
# %{_datadir}/sphinx/api/*


%changelog
* Tue Feb 11 2014 Anton Tsitlionok <deogar@sphinxsearch.com> 2.2.2-beta-1
- New version 2.2.2-beta

* Mon Nov 13 2013 Anton Tsitlionok <deogar@sphinxsearch.com> 2.2.1-beta-1
- New version 2.2.1-beta

* Thu Oct 10 2013 Anton Tsitlionok <deogar@sphinxsearch.com> 2.1.2-release-1
- New stable version 2.1.2-release
- PostgreSQL support is now default

* Wed Feb 20 2013 Anton Tsitlionok <deogar@sphinxsearch.com> 2.1.1-beta-1
- 2.1.1-beta

* Mon Oct 22 2012 Anton Tsitlionok <deogar@sphinxsearch.com> 2.0.6-release-1
- New stable version 2.0.6-release
- fixed binlog_path to work with /var/lib/sphinx/ in default minimal config

* Sat Jul 28 2012 Anton Tsitlionok <deogar@sphinxsearch.com> 2.0.5-release-1
- New stable version 2.0.5-release

* Tue Jul 21 2010 Andrew Aksyonoff <shodan@shodan.ru> 1.10-beta-1
- create a user/group, and display tips on install
- install HTML docs too

* Tue Jul 20 2010 Andrew Aksyonoff <shodan@shodan.ru> 1.10-beta-1
- updated for 1.10-beta

* Sun Jun 6 2010 Alexey N. Vinogradov <a.vinogradov@sphinxsearch.com> 0.9.9-3
- Added relocatable binary-only subpackage
- API moved from doc to /usr/share/sphinx/api

* Wed Jun 2 2010 Alexey N. Vinogradov <a.vinogradov@sphinxsearch.com> 0.9.9-2
- Added the testing subpackage

* Thu May 27 2010 Alexey N. Vinogradov <a.vinogradov@sphinxsearch.com> 0.9.9-1
- Added the record to /etc/services, according IANA

* Mon May 24 2010 Alexey N. Vinogradov <a.vinogradov@sphinxsearch.com> 0.9.9-1
- Added logrotate.d entry.

* Wed May 12 2010 Alexey N. Vinogradov <a.vinogradov@sphinxsearch.com> 0.9.9-1
- Added logrotate - from existing rhel5 rpm

* Thu May 6 2010 Alexey N. Vinogradov <a.vinogradov@sphinxsearch.com> 0.9.9-1
- Initial rpm release.
