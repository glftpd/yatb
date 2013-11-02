#ifndef __CONFIG_H
#define __CONFIG_H

#include "global.h"



class CConfig
{
	private:
	
	string getkey(string, string);
	
	public:
	
	CConfig();
	
	~CConfig();
	
	
	int readconf(string, string);
	
	int debug;
	int listen_port;
	string site_ip;
	string connect_ip;
	string site_port;
	string listen_interface;
	string listen_ip;
	string server_string;
	
	int fake_serverstring;
	int trytorelink;
	int traffic_bnc;
	
	int use_ident;
	int enforce_ident;
	int enforce_tls;
	int enforce_tls_fxp;
	
	int send_traffic_info;
	string relink_ip;
	int relink_port;
	string relink_user;
	string relink_pass;
	string user_access_denied;
	string user_login_success;
	string max_numlogins;
	int add_to_passive_port;
	int relink_notls;
	int port_range_start;
	int port_range_end;
	int use_port_range;
	
	int buffersize;

	int pending;
	string admin_list;
	int connect_timeout;
	int ident_timeout;
	int read_write_timeout;

	string cert_path;
	string fxp_fromsite_list;
	int use_fxpfromsite_list;
	string fxp_tosite_list;
	int use_fxptosite_list;
	int uid;
	string debug_logfile;
	string command_logfile;
	int log_to_screen;

	int ssl_forward;
	int use_ssl_exclude;
	string sslexclude_list;

	string entry_list;
	string infocmd;
	string helpcmd;
	string admincmd;
	string tositecmd;
	string fromsitecmd;
	string sslexcludecmd;
	string site_closed;
	string site_full;
	string reloadcmd;
	string entrycmd;
	string killcmd;
	int usecommands;
	int show_connect_failmsg;
	string pidfile;
	string connectfailmsg;
	int syslog;
	int use_forwarder;
	int forwarder_sport;
	int forwarder_dport;
	string forwarder_ip;
	int retry_count;
	int no_idnt_cmd;
	int ssl_ascii_cache;
	string cmd_prefix;
	int crypted_cert;
    int ssl_relink;
    double day_limit;
    double week_limit;
    double month_limit;
    string opt_dh_file;
    int translate_nosslfxp;
    int disable_noop;
	int traffic_bnc_relink;
};

extern CConfig config;
extern int use_blowconf;

#endif

