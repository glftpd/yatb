#ifndef __CONFIG_H
#define __CONFIG_H

#include "global.h"



class CConfig
{
	private:
	
	string getkey(string, string);
	void getentry(string &,string,int&,string);
	void getentry(int &,string,int&,string);
	void getentry(double &,string,int&,string);

	public:
	
	CConfig();
	
	~CConfig();
		
	int readconf(string, string, int);
	
	// config vars start here

	// section [DEBUG]
	int debug;
	int log_to_screen;
	string debug_logfile;
	string command_logfile;
	int syslog;

	// section [CONNECTION]
	int listen_port;
	string site_ip;
	string connect_ip;
	string site_port;
	string listen_interface;
	string listen_ip;
	string entry_list;
	int traffic_bnc;
	string nat_pasv_ip;

	// section [LIMIT]
	double day_limit;
    double week_limit;
    double month_limit;

	// section [SSL]
	string cert_path;
	string opt_dh_file;
    int translate_nosslfxp;
	int ssl_forward;
	int use_ssl_exclude;
	string sslexclude_list;
	int enforce_tls;
	int enforce_tls_fxp;
	int crypted_cert;
	string control_cipher;
	string data_cipher;

	// section [IDENT]
	int use_ident;
	int enforce_ident;
	int no_idnt_cmd;

	// section [RELINK]
	string relink_ip;
	int relink_port;
	string relink_user;
	string relink_pass;
	int trytorelink;
	int ssl_relink;
	int relink_notls;
	int traffic_bnc_relink;


	// section [FXP]
	string fxp_fromsite_list;
	int use_fxpfromsite_list;
	string fxp_tosite_list;
	int use_fxptosite_list;
	int use_fxpiplist;
	int use_fxpiphash;
	string hash_algo;
	string iplist_file;
	int crypted_iplist;	
	string fpwhitelist_file;
	int crypted_fpwhitelist;
	int fp_new_ip_msg;
	string fp_msg_nick;

	// section [ADMIN]
	int usecommands;
	string admin_list;
	string cmd_prefix;
	string infocmd;
	string helpcmd;
	string admincmd;
	string tositecmd;
	string fromsitecmd;
	string sslexcludecmd;
	string reloadcmd;
	string entrycmd;
	string killcmd;
	string fxpipcmd;
	string fpwlcmd;

	// section [FTPDSETUP]
	string server_string;	
	int fake_serverstring;	
	int send_traffic_info;
	string user_access_denied;
	string user_login_success;
	string site_closed;
	string site_full;	
	string max_numlogins;
	int show_connect_failmsg;
	string connectfailmsg;

	// section [FORWARDER]
	int use_forwarder;
	int forwarder_sport;
	int forwarder_dport;
	string forwarder_ip;


	// section [ADVANCED]
	int add_to_passive_port;	
	int port_range_start;
	int port_range_end;	
	int use_port_range;	
	int buffersize;
	int pending;	
	int connect_timeout;
	int ident_timeout;
	int read_write_timeout;
	int uid;	
	string pidfile;
	int retry_count;	
	int ssl_ascii_cache;    
    int disable_noop;	
	int speed_write;
	int allow_noentry_connect;
	int active_bind_range_start;
	int active_bind_range_end;
	int use_active_bind;
};

extern CConfig config;

#endif

