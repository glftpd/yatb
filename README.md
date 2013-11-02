Yet Another Traffic Bouncer README

requirements:
-------------

linux (or bsd)
g++
openssl + header files (>= 0.9.7)

compile:
--------

to compile just type 'make linux' or 'make bsd' for either linux or bsd version
make without parameter will show other options (debug/static versions)

install:
--------

copy blowcrypt, yatb and yatb.conf.dist to a directory you like
you will need a SSL certificate file - copy .pem file from gl installation into the same dir

encrypt conf:
-------------

you can use encrypted conf file
to do so encrypt your conf file with ./blowcrypt -e file1 file2 to encrypt file1 to file2
(for example ./blowcrypt -e yatb.conf.dist)
if you want to change your conf you can decrypt it with ./blowcrypt -d file1 file2 to decrypt file1 to file2)
(for example ./blowcrypt -d yatb.conf.dist)

start:
------

you can start yatb with ./yatb yatb.conf.dist
(or ./yatb -u yatb.conf.dist if you want to use uncrypted conf file)
if you want to use a crypted config file without being asked for the key (cron jobs) do this:
edit yatb.cc and remove // from the second line and change the key to your key
(should look like this '#define config_key "somekey"')
now compile again (do a 'make clean' first) and you're done

format of conf file:
--------------------

all entrys are of the folloing form:
key=value;
the ';' is important, also do not use " or spaces or anything else

cron job:
---------

edit bnctest.sh to fit your needs and add to crontab

options in config file:
-----------------------

- debug=0;

enable/disable debug mode

--------------------------------------------------------

- debug_logfile=log.txt;

name and path of log file

--------------------------------------------------------

- cert_path=dsa.pem;

name and path of cert file

--------------------------------------------------------

- listen_port=123;

listen port for the bnc

--------------------------------------------------------

- entry_list=;

if you want to use entry(s) add ips here
only connections from this ip(s) are accepted

--------------------------------------------------------

- listen_interface=ppp0;

interface to get ip from

--------------------------------------------------------

- listen_ip=;

you can bind to specific ip
if set listen_interface is ignored

--------------------------------------------------------

- site_ip=1.2.3.4;

ip of ftp server to connect to

--------------------------------------------------------

- site_port=123;

listen port of ftp server to connect to

--------------------------------------------------------

- connect_ip=;

ip used to connect to site (not needed)

--------------------------------------------------------

- server_string=220 FTP server ready.;

server string displayed after connect

--------------------------------------------------------

- fake_server_string=1;

if enabled server_string is used - else original ftp msg

--------------------------------------------------------

- trytorelink=1;

if enabled bnc tries to relink if connection to site fails (wrong user/pass for example)

--------------------------------------------------------

- relink_ip=1.2.3.4;

ip of site used for relinking

--------------------------------------------------------

- relink_port=21;

port of site used for relinking

--------------------------------------------------------

- relink_user=anonymous;

relink user

--------------------------------------------------------

- relink_pass=a@b.de;

relink pass

--------------------------------------------------------

- traffic_bnc=1;

if enabled control and data connections are bounced
else bnc is running in entry mode

--------------------------------------------------------

- use_ident=1;

use IDNT command

--------------------------------------------------------

- enforce_ident=0;

if enabled connection is killed if no ident response is recieved

--------------------------------------------------------

- enforce_tls=1;

if enabled connection is killed if ssl is not used

--------------------------------------------------------

- send_traffic_info=1;

if enabled upload/download info is send after logout

--------------------------------------------------------

- user_access_denied=access denied.;

ftpd message if access is denied

--------------------------------------------------------

- user_login_success=logged in.;

ftpd message if login was successfull

--------------------------------------------------------

- add_to_passive_port=0;

normally bnc uses same port number for data connections as ftpd
you can shift this if you set add_to_passive to 10 or -10 for example

--------------------------------------------------------

- port_range_start=28001;
- port_range_end=30000;

port number for relinked data connections is picked from this range

--------------------------------------------------------

- use_port_range=1;

if enabled port number is taken from specified range else same as ftpd uses

--------------------------------------------------------

- buffersize=4096;

size of packets in bytes

--------------------------------------------------------

- pending=10;

number of pending connections

--------------------------------------------------------

- connect_timeout=10;

connection timeout in seconds

--------------------------------------------------------

- ident_timeout=2;

ident timeout in seconds

--------------------------------------------------------

- read_write_timeout=10;

read/write timeout in seconds

--------------------------------------------------------

- enforce_tls_fxp=0;

enforce ssl site to site transfers

--------------------------------------------------------

- admin_list=hawk;

list of added admins - can be changed at runtime

--------------------------------------------------------

- use_fxpfromsite_list=0;

if enabled only users in fxp_fromsite_list are allowed to fxp from site

--------------------------------------------------------

- fxp_fromsite_list=;

users allowed to fxp from site

--------------------------------------------------------

- use_fxptosite_list=0;

if enabled only users in fxp_fromsite_list are allowed to fxp to site

--------------------------------------------------------

- fxp_tosite_list=;

users allowed to fxp to site

--------------------------------------------------------

- uid=1;

if started as root bnc changes uid to this after binding to port

--------------------------------------------------------

- log_to_screen=0;

if enabled debug messages are printed to console and not to logfile

--------------------------------------------------------

- use_ssl_exclude=0;

if enabled users in sslexclude_list are not forced to use

--------------------------------------------------------

- sslexclude_list=;

list of users not forced to use ssl

--------------------------------------------------------

- infocmd=bncinfo;

info command

--------------------------------------------------------

- helpcmd=bnchelp;

help command

--------------------------------------------------------

- admincmd=admin;

prefix for adminlist commands

--------------------------------------------------------

- tositecmd=tosite;

prefix for fxp to site list commands

--------------------------------------------------------

- fromsitecmd=fromsite;

prefix for fxp from site list commands

--------------------------------------------------------

- sslexcludecmd=nossl;

prefix for ssl exclude list commands

--------------------------------------------------------

- site_closed=The server has been shut down, try again later.;

site closed message

--------------------------------------------------------

- site_full=The site is full, try again later.;

site full message

--------------------------------------------------------

- reloadcmd=rehash;

reload command

--------------------------------------------------------

- usecommands=1;

enable/disable all admin commands

--------------------------------------------------------

- showconnectfailmsg=1;

send message to client if connect to site fails

--------------------------------------------------------

- pidfile=yatb.pid;

file to store pid of yatb

--------------------------------------------------------

- connectfailmsg=427 Connection failed!;

if connection to site failes this message is send

--------------------------------------------------------

- syslog=1;

if enabled start errors and logins/logouts and failed logins are
logged to daemon.log (normally in /var/log/)

--------------------------------------------------------

- ssl_forward=0;

if enabled encrypted packets are passed to client
else they are decrypted and encrypted again

--------------------------------------------------------
special bnc commands:
---------------------

(this are the default commands - can be changed in conf)

this commands can be used if user is in admin list:
 'adminshow' - show added admins
 'adminadd user1[,user2]' - add new admin(s)
 'admindel user1[,user2]' - delete admin(s)

 'tositeshow' - show added users allowed to fxp to site
 'tositeadd user1[,user2]' - add new user(s)
 'tositedel user1[,user2]' - delete user(s)

 'fromsiteshow' - show added users allowed to fxp from site
 'fromsiteadd user1[,user2]' - add new user(s)
 'fromsitedel user1[,user2]' - delete user(s)

 'nosslshow' - show added users excluded from using ssl
 'nossladd user1[,user2]' - add new user(s)
 'nossldel user1[,user2]' - delete user(s)

 'bncinfo' - show some infos

 'bnchelp' - show this help

 'reload' - reload config (for now only some values)

notes:
------

bnc chroots to start dir
so log/cert file should not be placed outside start dir
set cert/logfile to 777 cause bnc changes uid
join #yatb on efnet for comments/requests etc

known bugs:
-----------

thread problems with some libc versions

