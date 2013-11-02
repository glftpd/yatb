#include "config.h"
#include "tools.h"

CConfig::CConfig()
{
	// set some default values

	// section [DEBUG]
	debug = 0;
	debug_logfile = "";
	command_logfile = "";
	log_to_screen = 0;
	syslog = 0;

	// section [CONNECTION]
	listen_port = 0;
	site_ip = "";
	connect_ip = "";
	site_port = "";
	listen_interface = "eth0";
	listen_ip = "";
	entry_list = "";
	traffic_bnc = 0;
	nat_pasv_ip = "";

	// section [LIMIT]
	day_limit = 0;
    week_limit = 0;
    month_limit = 0;

	// section [SSL]
	cert_path = "ftpd-dsa.pem";
	opt_dh_file = "";
	crypted_cert = 0;
	enforce_tls = 0;
	enforce_tls_fxp = 0;	
	ssl_forward = 1;
	use_ssl_exclude = 0;
	sslexclude_list = "";
	translate_nosslfxp = 0;
	control_cipher = "ALL:!ADH:!EXP";
	data_cipher = "ALL:!ADH:!EXP";

	// section [IDENT]
	use_ident = 1;
	enforce_ident = 0;
	no_idnt_cmd = 0;

	// section [RELINK]
	trytorelink = 0;
	relink_ip = "";
	relink_port = 0;
	relink_user = "";
	relink_pass = "";
	ssl_relink = 0;
	relink_notls = 0;
	traffic_bnc_relink=0;

	// section [FXP]
	fxp_fromsite_list = "";
	use_fxpfromsite_list = 0;
	use_fxptosite_list = 0;
	fxp_tosite_list = "";
	use_fxpiplist = 0;
	use_fxpiphash = 0;
	hash_algo = "sha256";
	iplist_file = "";
	crypted_iplist=0;
	fpwhitelist_file = "";
	crypted_fpwhitelist = 0;
	fp_new_ip_msg = 0;
	fp_msg_nick = "";

	// section [ADMIN]
	usecommands = 0;
	admin_list = "";
	cmd_prefix = "";
	infocmd = "";
	helpcmd = "";
	admincmd = "";
	tositecmd = "";
	fromsitecmd = "";
	sslexcludecmd = "";
	reloadcmd = "";
	killcmd = "";
    entrycmd = "";
	fxpipcmd = "";
	fpwlcmd = "";

	// section [FTPD SETUP]
	server_string  = "";
	fake_serverstring = 0;	
	send_traffic_info = 0;	
	user_access_denied = "access denied.";
	user_login_success = "logged in.";
	site_closed = "The server has been shut down, try again later.";
	site_full = "The site is full, try again later.";
	max_numlogins = "Sorry, your account is restricted to";
	show_connect_failmsg = 0;
	connectfailmsg = "";


	// section [FORWARDER]
	use_forwarder=0;
	forwarder_sport=0;
	forwarder_dport=0;
	forwarder_ip="";


	// section [ADVANCED]
	add_to_passive_port = 0;	
	port_range_start = 0;
	port_range_end = 0;
	use_port_range = 0;	
	buffersize = 4096;
	pending = 10;
	connect_timeout = 7;
	ident_timeout = 7;
	read_write_timeout = 30;
	uid = 1;
	pidfile = "yatb.pid";
	retry_count = 5;
	ssl_ascii_cache = 0;
    disable_noop = 0;
	speed_write = 0;
	allow_noentry_connect = 0;
	active_bind_range_start = 0;
	active_bind_range_end = 0;
	use_active_bind = 0;
}

CConfig::~CConfig()
{
}

string CConfig::getkey(string name,string data)
{
	string value = "ERROR";
	int start,end;
	string tmp = data;
	name = name + "=";
	start = tmp.find(name,0);
	if (start == (int)string::npos)
	{
		for (int i=0;i<(int)data.length();i++) { data[i] = '0'; }
		return value;
	}
	end = tmp.find(";",start);
	if (end == (int)string::npos)
	{
		for (int i=0;i<(int)data.length();i++) { data[i] = '0'; }
		return value;
	}
	value = tmp.substr(start + name.length(),end-start-name.length());
	for (int i=0;i<(int)data.length();i++) { data[i] = '0'; }
	
	return value;
}

// read conf entry of type string
void CConfig::getentry(string &i,string s,int &ok,string daten)
{
	string val;
	if ((val=getkey(s,daten)) != "ERROR")
   	{
   		i = val.c_str();
   	}
   	else
   	{
		cout << "using default value '" << i << "' for " << s << "\n";
   		//cout << s << " missing\n";
   		ok = 0;
   	}
}

// read conf entry of type int
void CConfig::getentry(int &i,string s,int &ok,string daten)
{
	string val;
	if ((val=getkey(s,daten)) != "ERROR")
   	{
   		i = atoi(val.c_str());
   	}
   	else
   	{
		cout << "using default value '" << i << "' for " << s << "\n";
   		//cout << s << " missing\n";
   		ok = 0;
   	}
}

// read conf entry of type double
void CConfig::getentry(double &i,string s,int &ok, string daten)
{
	string val;
	if ((val=getkey(s,daten)) != "ERROR")
   	{
   		i = atof(val.c_str());
   	}
   	else
   	{
		cout << "using default value '" << i << "' for " << s << "\n";
   		//cout << s << " missing\n";
   		ok = 0;
   	}
}

int CConfig::readconf(string filename,string key,int crypted)
{
	int s;
	if (!filesize(filename,s))
	{
		cout << "Could not find config file!\n";
		return 0;
	}
	else
	{	
		unsigned char *bufferin,*bufferout;
		
		bufferout = new unsigned char [s+1];
		
		memset(bufferout,'\0',s+1);
		readfile(filename,&bufferin,s);
		
		string daten; // store uncrypted conf file
		
		if (crypted)
		{
			decrypt(key,bufferin,bufferout,s);
			daten = (char*)bufferout;
			memset(bufferin,'\0',s+1);
			memset(bufferout,'\0',s+1);
			delete [] bufferin;
	 		delete [] bufferout;			
		}
		else
		{			
			daten = (char *)bufferin;
	    	memset(bufferin,'\0',s+1);
			memset(bufferout,'\0',s+1);
	    	delete [] bufferin;
	 		delete [] bufferout;
		}    
		
 		int ok = 1;	// store if all vars could be read
	 	
		// section [DEBUG
		getentry(debug,"debug",ok,daten);
		getentry(log_to_screen,"log_to_screen",ok,daten);
		getentry(debug_logfile,"debug_logfile",ok,daten);
		getentry(command_logfile,"command_logfile",ok,daten);
		getentry(syslog,"syslog",ok,daten);

		// section [CONNECTION]
		getentry(listen_port,"listen_port",ok,daten);
		getentry(site_ip,"site_ip",ok,daten);
		getentry(site_port,"site_port",ok,daten);
		getentry(entry_list,"entry_list",ok,daten);
		getentry(connect_ip,"connect_ip",ok,daten);
		getentry(listen_interface,"listen_interface",ok,daten);
		getentry(listen_ip,"listen_ip",ok,daten);
		getentry(traffic_bnc,"traffic_bnc",ok,daten);
		getentry(nat_pasv_ip,"nat_pasv_ip",ok,daten);

		// section [LIMIT]
		getentry(day_limit,"day_limit",ok,daten);
		getentry(week_limit,"week_limit",ok,daten);
		getentry(month_limit,"month_limit",ok,daten);

		// section [SSL]
		getentry(cert_path,"cert_path",ok,daten);
		getentry(opt_dh_file,"opt_dh_file",ok,daten);
		getentry(crypted_cert,"crypted_cert",ok,daten);
		getentry(enforce_tls,"enforce_tls",ok,daten);
		getentry(enforce_tls_fxp,"enforce_tls_fxp",ok,daten);
		getentry(ssl_forward,"ssl_forward",ok,daten);
		getentry(use_ssl_exclude,"use_ssl_exclude",ok,daten);
		getentry(sslexclude_list,"sslexclude_list",ok,daten);
		getentry(translate_nosslfxp,"translate_nosslfxp",ok,daten);
		getentry(control_cipher,"control_cipher",ok,daten);
		getentry(data_cipher,"data_cipher",ok,daten);
		
		// section [IDENT]
		getentry(use_ident,"use_ident",ok,daten);
		getentry(enforce_ident,"enforce_ident",ok,daten);
		getentry(no_idnt_cmd,"no_idnt_cmd",ok,daten);

		// section [RELINK]
		getentry(trytorelink,"trytorelink",ok,daten);
		getentry(relink_ip,"relink_ip",ok,daten);
		getentry(relink_port,"relink_port",ok,daten);
		getentry(relink_user,"relink_user",ok,daten);
		getentry(relink_pass,"relink_pass",ok,daten);
		getentry(ssl_relink,"ssl_relink",ok,daten);
		getentry(relink_notls,"relink_notls",ok,daten);
		getentry(traffic_bnc_relink,"traffic_bnc_relink",ok,daten);


		// section [FXP]
		getentry(use_fxpfromsite_list,"use_fxpfromsite_list",ok,daten);
		getentry(fxp_fromsite_list,"fxp_fromsite_list",ok,daten);
		getentry(use_fxptosite_list,"use_fxptosite_list",ok,daten);
		getentry(fxp_tosite_list,"fxp_tosite_list",ok,daten);
		getentry(use_fxpiplist,"use_fxpiplist",ok,daten);
		getentry(use_fxpiphash,"use_fxpiphash",ok,daten);
		getentry(hash_algo,"hash_algo",ok,daten);
		getentry(iplist_file,"iplist_file",ok,daten);
		getentry(crypted_iplist,"crypted_iplist",ok,daten);		
		getentry(fpwhitelist_file,"fpwhitelist_file",ok,daten);
		getentry(crypted_fpwhitelist,"crypted_fpwhitelist",ok,daten);
		getentry(fp_new_ip_msg,"fp_new_ip_msg",ok,daten);
		getentry(fp_msg_nick,"fp_msg_nick",ok,daten);

		// section [ADMIN]
		getentry(usecommands,"usecommands",ok,daten);
		getentry(admin_list,"admin_list",ok,daten);
		getentry(cmd_prefix,"cmd_prefix",ok,daten);
		getentry(infocmd,"infocmd",ok,daten);
		getentry(helpcmd,"helpcmd",ok,daten);
		getentry(admincmd,"admincmd",ok,daten);
		getentry(tositecmd,"tositecmd",ok,daten);
		getentry(fromsitecmd,"fromsitecmd",ok,daten);
		getentry(sslexcludecmd,"sslexcludecmd",ok,daten);
		getentry(reloadcmd,"reloadcmd",ok,daten);
		getentry(entrycmd,"entrycmd",ok,daten);
		getentry(killcmd,"killcmd",ok,daten);
		getentry(fxpipcmd,"fxpipcmd",ok,daten);
		getentry(fpwlcmd,"fpwlcmd",ok,daten);		

		// section [FTPD SETUP]
		getentry(server_string,"server_string",ok,daten);
		getentry(fake_serverstring,"fake_serverstring",ok,daten);
		getentry(send_traffic_info,"send_traffic_info",ok,daten);
		getentry(user_access_denied,"user_access_denied",ok,daten);
		getentry(user_login_success,"user_login_success",ok,daten);
		getentry(site_closed,"site_closed",ok,daten);
		getentry(site_full,"site_full",ok,daten);
		getentry(max_numlogins,"max_numlogins",ok,daten);
		getentry(show_connect_failmsg,"show_connect_failmsg",ok,daten);
		getentry(connectfailmsg,"connectfailmsg",ok,daten);

		// section [FORWARDER]
		getentry(use_forwarder,"use_forwarder",ok,daten);
		getentry(forwarder_sport,"forwarder_sport",ok,daten);
		getentry(forwarder_dport,"forwarder_dport",ok,daten);
		getentry(forwarder_ip,"forwarder_ip",ok,daten);

		// section [ADVANCED]
		getentry(add_to_passive_port,"add_to_passive_port",ok,daten);
		getentry(port_range_start,"port_range_start",ok,daten);
		getentry(port_range_end,"port_range_end",ok,daten);
		getentry(use_port_range,"use_port_range",ok,daten);
		getentry(buffersize,"buffersize",ok,daten);
		getentry(pending,"pending",ok,daten);
		getentry(connect_timeout,"connect_timeout",ok,daten);
		getentry(ident_timeout,"ident_timeout",ok,daten);
		getentry(read_write_timeout,"read_write_timeout",ok,daten);
		getentry(uid,"uid",ok,daten);
		getentry(pidfile,"pidfile",ok,daten);
		getentry(retry_count,"retry_count",ok,daten);
		getentry(ssl_ascii_cache,"ssl_ascii_cache",ok,daten);
		getentry(disable_noop,"disable_noop",ok,daten);
		getentry(speed_write,"speed_write",ok,daten);
		getentry(allow_noentry_connect,"allow_noentry_connect",ok,daten);
		getentry(active_bind_range_start,"active_bind_range_start",ok,daten);
		getentry(active_bind_range_end,"active_bind_range_end",ok,daten);
		getentry(use_active_bind,"use_active_bind",ok,daten);

   		for(int i=0;i < (int)daten.length();i++)
   		{
   			daten[i] = '0';
   		}
		return 1;
 		//if (ok == 1) return 1;
 		//else return 0;
	}

}
