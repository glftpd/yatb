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
	int site_port;
	string listen_interface;
	string listen_ip;
	string server_string;
	
	int fake_server_string;
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
	int add_to_passive_port;
	
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
	int usecommands;
	int showconnectfailmsg;
	string pidfile;
	string connectfailmsg;
	int syslog;
	int use_forwarder;
	int forwarder_sport;
	int forwarder_dport;
	string forwarder_ip;
};

extern CConfig config;
extern int use_blowconf;

#endif

