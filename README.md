Yet Another Traffic Bouncer README

What do you need?
_________________

*NIX
openssl + openssl header files (>= 0.9.8)
g++

Compiling
_________

first do a 'make clean'
then do 'make system'
system can be: linux,bsd,solaris,cygwin
you can also do 'make system-static' 'make system-debug' or 'make system-debug-static' to make static/debug versions

Setup
_____

Now copy a cert file (ftpd-dsa.pem) to bin directory
Also copy yatb.conf.dist to bin dir
If you have a rsa cert, you will need a dh file
(can be generated with 'openssl dhparam -out dh1024.pem -2 1024')

Encrypting config file
______________________

You can encrypt your config file and the cert file if you want
to do so use the included blowcrypt
syntax is: blowcrypt -e file to encrypt file and blowcrypt -d file to decrypt
(you should encrypt the files on another shell and only upload the encrypted files)

File permissions
________________

You can start yatb as root (it will change uid in this case)
if you do so, make sure that bin dir and conf/cert belongs to this user (default is daemon)

Start
_____

To start yatb type ./yatb conffile (when using encrypted conf)
or ./yatb -u conffile (when using uncrypted conf)

format of conf file
___________________

all entrys are of the folloing form:
key=value;
the ';' is important, also do not use " or spaces or anything else

quick start
___________

most values can use the default from conf
you only have to change a few sections:
[ Connection ] and [ SSL ]

-------------------------------------------------------------------------------------------------------

config file values
__________________

[ Debug ]
debug=0; // enable debug mode

log_to_screen=1; // log to screen instead of file

debug_logfile=log.txt; // name of logfile

command_logfile=; // special file to log only site commands and replys

syslog=0; // enable logging to syslog

[ Connection ]
listen_port=123; // listen port from yatb

site_ip=1.2.3.4; // ip from site - can be a list of ips like site_ip=ip1,ip2,ip3;
                 // in this case, every new connection gets next ip in list
                 
site_port=123;  // port from site - can be a list - see above

entry_list=;  // a entry(s) are use add ips here

connect_ip=;  // use a sepcial ip to connect to site

listen_interface=eth0; // gets ip for passive mode connections from this interface
                       // can be something like ppp0 for dialup connections

listen_ip=; // bind to special ip - overrides listen_interface

traffic_bnc=1; // run as traffic bnc or entry

[ Limit ]
day_limit=0; // daily limit

week_limit=0; // weekly limit

month_limit=0; // monthly limit

[ SSL ]
cert_path=ftpd-dsa.pem; // name of cert file

opt_dh_file=; // when using rsa cert specify a dh file here

crypted_cert=0; // use a crypted cert

enforce_tls=1; // enforce ssl usage

enforce_tls_fxp=0; // enforce ssl fxp

ssl_forward=1; // only forward ssl packets
               // else all packets are de and encrypted
               
use_ssl_exclude=0; // use the ssl exclude list to allow specials users to login without ssl

sslexclude_list=; // list of users which must not use ssl

translate_nosslfxp=0; // no function yet

[ Ident ]
use_ident=1; // make an ident request

enforce_ident=0; // enforce ident reply

no_idnt_cmd=0; // don't sent IDNT command to site

[ Relink ]
trytorelink=0; // enable relink feature

relink_ip=1.2.3.4; // ip of relink site

relink_port=21; // port of relink site

relink_user=anonymous; // username for relinking

relink_pass=a@b.de; // password for relinking

ssl_relink=0; // login with ssl to relink site

relink_notls=0; // relink, if no ssl is used

traffic_bnc_relink=0; // run as traffic bnc if relinked

[ Fxp ]
use_fxpfromsite_list=0; // use list with users that are allowed to fxp from site

fxp_fromsite_list=; // list with users that are allowed to fxp from site

use_fxptosite_list=0; // use list with users that are allowed to fxp to site

fxp_tosite_list=; // list with users that are allowed to fxp to site

use_fxpiplist=0; // use list with ips of allowed sites - no fxp transfers to other sites possible
                 // ips are entered via admin commands
                 
use_fxpiphash=0; // only store a hash of ip in memory

hash_algo=sha256; // algo used for hashing

iplist_file=; // file to store ips for fxp ip list
              // format is ip,comment1,comment2
              // when using hash, hashed ip must be stored in file!

crypted_iplist=0; // use crypted ip list file

[ Admin ]
usecommands=0; // enable the admin commands

admin_list=hawk; // list of admins

cmd_prefix=e; // prefix for all commands

infocmd=info; // info command

helpcmd=help; // help command

admincmd=admin; // admin command

tositecmd=tosite; // fxp to site command

fromsitecmd=fromsite; // fxp from site command

sslexcludecmd=nossl; // ssl exclude command

reloadcmd=reload; // reload command (reload the conf)

entrycmd=entry; // entry command

killcmd=kill; // kill command (kill conf,cert and yatb)

fxpipcmd=fxpip; // fxp ip command

list commands have show,add and delete
list commands are: admin,tosite,fromsite,sslexclude,entry and fxpip
fxpip has a save command
example: add a new addmin -> eadminadd newadmin
         del a entry -> eentyrdel 1.2.3.4
you don't have to put 'site ' before commands but you can set prefix to 'site e' for example         

[ Ftpd setup ]
server_string=220 FTP server ready.; // fake server string
                                     // must start with '220 ' !!!!!!
                                     
fake_serverstring=1; // use fake server string

send_traffic_info=0; // sends a traffic statistic after logout


// edit this replys to match your ftpd

user_access_denied=access denied.;
user_login_success=logged in.;
site_closed=The server has been shut down, try again later.;
site_full=The site is full, try again later.;
max_numlogins=Sorry, your account is restricted to;

show_connect_failmsg=1; // show a msg if bnc can't conenct to site - else just disconnect

connectfailmsg=427 Connection failed!;

// simple port forwarder
[ Forwarder ]
use_forwarder=0;
forwarder_sport=81;
forwarder_dport=80;
forwarder_ip=www.heise.de;

[ Advanced ]
add_to_passive_port=0; // shift the passive port by value - else same port like the ftpd uses is taken

port_range_start=28001; // port range for relinking and ONLY FOR RELINKING!!
port_range_end=30000;

use_port_range=1; // use the port range for relinking

buffersize=4096; // packet size

pending=50; // maximum of pending connections

connect_timeout=7; // connect timeout in seconds

ident_timeout=7; // ident timeout in seconds

read_write_timeout=30; // read/write timeout in seconds

uid=1; // if started as root switch to this uid

pidfile=yatb.pid; // file to store pid

retry_count=10; // read/write retrys

ssl_ascii_cache=0; // cache ascii connections in ssl mode - speeds up dirlisting for gl 1.32

disable_noop=0; // disable noop command

speed_write=0; // if enabled a tuned read/write loop is used in datathread (it's faster but not 100% tested)

allow_noentry_connect=0; // if enabled not only entry bnc may connect to traffic bnc