#include "config.h"
#include "tools.h"

CConfig::CConfig()
{	
	debug = 0;
	listen_port = 0;
	site_ip = "";
	connect_ip = "";
	site_port = "";
	listen_interface = "eth0";
	listen_ip = "";
	server_string  = "";
	fake_serverstring = 0;
	trytorelink = 0;
	traffic_bnc = 0;	
	use_ident = 0;
	enforce_ident = 0;
	enforce_tls = 0;
	enforce_tls_fxp = 0;	
	send_traffic_info = 0;
	relink_ip = "";
	relink_port = 0;
	relink_user = "";
	relink_pass = "";
	user_access_denied = "";
	user_login_success = "";
	add_to_passive_port = 0;	
	port_range_start = 0;
	port_range_end = 0;
	use_port_range = 0;	
	buffersize = 4096;

	pending = 0;
	admin_list = "";
	connect_timeout = 7;
	ident_timeout = 7;
	read_write_timeout = 30;

	cert_path = "";
	fxp_fromsite_list = "";
	use_fxpfromsite_list = 0;
	use_fxptosite_list = 0;
	fxp_tosite_list = "";
	uid = 0;
	debug_logfile = "";
	command_logfile = "";
	log_to_screen = 0;
	ssl_forward = 0;

	use_ssl_exclude = 0;
	sslexclude_list = "";

	entry_list = "";
	infocmd = "";
	helpcmd = "";
	admincmd = "";
	tositecmd = "";
	fromsitecmd = "";
	sslexcludecmd = "";
	site_closed = "";
	site_full = "";
	usecommands = 0;
	show_connect_failmsg = 0;
	pidfile = "";
	connectfailmsg = "";
	syslog = 0;
	use_forwarder=0;
	forwarder_sport=0;
	forwarder_dport=0;
	forwarder_ip="";
	retry_count = 5;
	no_idnt_cmd = 0;
	ssl_ascii_cache = 0;
	cmd_prefix = "";
    ssl_relink = 0;
    killcmd = "";
    entrycmd = "";
    day_limit = 0;
    week_limit = 0;
    month_limit = 0;
    opt_dh_file = "";
    translate_nosslfxp = 0;
    disable_noop = 0;
    max_numlogins = "";
	relink_notls = 0;
}

CConfig::~CConfig()
{
}

string CConfig::getkey(string name,string data)
{
	string value = "ERROR";
	unsigned int start,end;
	string tmp = data;
	name = name + "=";
	start = tmp.find(name,0);
	if (start == string::npos)
	{
		for (unsigned int i=0;i<data.length();i++) { data[i] = '0'; }
		return value;
	}
	end = tmp.find(";",start);
	if (end == string::npos)
	{
		for (unsigned int i=0;i<data.length();i++) { data[i] = '0'; }
		return value;
	}
	value = tmp.substr(start + name.length(),end-start-name.length());
	for (unsigned int i=0;i<data.length();i++) { data[i] = '0'; }
	
	return value;
}

int CConfig::readconf(string filename,string key)
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
		
		string daten;
		
		if (use_blowconf == 1)
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
		
 	int ok = 1;	
 		
   	string val;

		if ((val=getkey("listen_port",daten)) != "ERROR")
   	{
   		listen_port = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "wrong key or listen_port missing\n";
   		
   		return 0;
   	}

   	if ((val=getkey("debug",daten)) != "ERROR")
   	{
   		debug = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "debug missing\n";
   		
   		ok = 0;
   	}
   	
		
		if ((val=getkey("use_fxpfromsite_list",daten)) != "ERROR")
    {
   		use_fxpfromsite_list = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "use_fxpfromsite_list missing\n";
   		
   		ok = 0;
   	}
		
   	if ((val=getkey("site_port",daten)) != "ERROR")
   	{
   		site_port = val.c_str();
   	}
   	else
   	{
   		cout << "site_port missing\n";
   		
   		ok = 0;
   	}

   	if ((val=getkey("site_ip",daten)) != "ERROR")
   	{
   		site_ip = val;
   	}
   	else
   	{
   		cout << "site_ip missing\n";
   		
   		ok = 0;
   	}
		
		if ((val=getkey("entry_list",daten)) != "ERROR")
   	{
   		entry_list = val;
   	}
   	else
   	{
   		cout << "entry_list missing\n";
   		
   		ok = 0;
   	}
		
		if ((val=getkey("connect_ip",daten)) != "ERROR")
   	{
   		connect_ip = val;
   	}
   	else
   	{
   		cout << "connect_ip missing\n";
   		
   		ok = 0;
   	}
		
   	if ((val=getkey("cert_path",daten)) != "ERROR")
   	{
   		cert_path = val;
   	}
   	else
   	{
   		cout << "cert_path missing\n";
   		
   		ok = 0;
   	}
		
		if ((val=getkey("fxp_fromsite_list",daten)) != "ERROR")
    {
   		fxp_fromsite_list = val;
   	}
   	else
   	{
   		cout << "fxp_fromsite_list missing\n";
   		
   		ok = 0;
   	}
		
 		if ((val=getkey("listen_interface",daten)) != "ERROR")
    {
   		listen_interface = val;
   	}
   	else
   	{
   		cout << "listen_interface missing\n";
   		
   		ok = 0;
   	}
		
		if ((val=getkey("listen_ip",daten)) != "ERROR")
    {
   		listen_ip = val;
   	}
   	else
   	{
   		cout << "listen_ip missing\n";
   		
   		ok = 0;
   	}
		
		
 		if ((val=getkey("server_string",daten)) != "ERROR")
    {
   		server_string = val;
   	}
   	else
   	{
   		cout << "server_string missing\n";
   		
   		ok = 0;
   	}

 		if ((val=getkey("fake_serverstring",daten)) != "ERROR")
    {
   		fake_serverstring = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "fake_serverstring missing\n";
   		
   		ok = 0;
   	}

 		if ((val=getkey("trytorelink",daten)) != "ERROR")
    {
   		trytorelink = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "trytorelink missing\n";
   		
   		ok = 0;
   	}

 		if ((val=getkey("traffic_bnc",daten)) != "ERROR")
    {
   		traffic_bnc = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "traffic_bnc missing\n";
   		
   		ok = 0;
   	}

 		if ((val=getkey("enforce_tls_fxp",daten)) != "ERROR")
    {
   		enforce_tls_fxp = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "enforce_tls_fxp missing\n";
   		
   		ok = 0;
   	}


 		if ((val=getkey("use_ident",daten)) != "ERROR")
    {
   		use_ident = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "use_ident missing\n";
   		
   		ok = 0;
   	}

		if ((val=getkey("enforce_ident",daten)) != "ERROR")
   	{
   		enforce_ident = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "enforce_ident missing\n";
   		
   		ok = 0;
   	}

 		if ((val=getkey("enforce_tls",daten)) != "ERROR")
    {
	 		enforce_tls = atoi(val.c_str());
	 	}
	 	else
	 	{
	 		cout << "enforce_tls missing\n";
	 		
	 		ok = 0;
	 	}

 		if ((val=getkey("send_traffic_info",daten)) != "ERROR")
    {
   		send_traffic_info = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "send_traffic_info missing\n";
   		
   		ok = 0;
   	}

 		if ((val=getkey("relink_ip",daten)) != "ERROR")
   	{
   		relink_ip = val;
   	}
   	else
   	{
   		cout << "relink_ip missing\n";
   		
   		ok = 0;
   	}

 		if ((val=getkey("relink_port",daten)) != "ERROR")
   	{
   		relink_port = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "relink_port missing\n";
   		
   		ok = 0;
   	}

 		if ((val=getkey("relink_user",daten)) != "ERROR")
   	{
   		relink_user = val;
   	}
   	else
   	{
   		cout << "relink_user missing\n";
   		
   		ok = 0;
   	}

 		if ((val=getkey("relink_pass",daten)) != "ERROR")
   	{
   		relink_pass = val;
   	}
   	else
   	{
   		cout << "relink_pass missing\n";
   		
   		ok = 0;
   	}

 		if ((val=getkey("user_access_denied",daten)) != "ERROR")
   	{
   		user_access_denied = val;
   	}
   	else
   	{
   		cout << "user_access_denied\n";
   		
   		ok = 0;
   	}

 		if ((val=getkey("user_login_success",daten)) != "ERROR")
   	{
   		user_login_success = val;
   	}
   	else
   	{
   		cout << "user_login_success\n";
   		
   		ok = 0;
   	}

 		if ((val=getkey("add_to_passive_port",daten)) != "ERROR")
   	{
   		add_to_passive_port = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "add_to_passive_port missing\n";
   		
   		ok = 0;
   	}

 		if ((val=getkey("add_to_passive_port",daten)) != "ERROR")
   	{
   		add_to_passive_port = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "add_to_passive_port missing\n";
   		
   		ok = 0;
   	}

 		if ((val=getkey("port_range_start",daten)) != "ERROR")
   	{
   		port_range_start = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "port_range_start missing\n";
   		
   		ok = 0;
   	}

 		if ((val=getkey("port_range_end",daten)) != "ERROR")
   	{
   		port_range_end = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "port_range_end missing\n";
   		
   		ok = 0;
   	}

 		if ((val=getkey("use_port_range",daten)) != "ERROR")
   	{
   		use_port_range = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "use_port_range missing\n";
   		
   		ok = 0;
   	}
	

 		if ((val=getkey("buffersize",daten)) != "ERROR")
   	{
   		buffersize = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "buffersize missing\n";
   		
   		ok = 0;
   	}

 		if ((val=getkey("buffersize",daten)) != "ERROR")
   	{
   		buffersize = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "buffersize missing\n";
   		
   		ok = 0;
   	}

 		if ((val=getkey("pending",daten)) != "ERROR")
   	{
   		pending = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "pending missing\n";
   		
   		ok = 0;
   	}

 		if ((val=getkey("connect_timeout",daten)) != "ERROR")
   	{
   		connect_timeout = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "connect_timeout missing\n";
   		
   		ok = 0;
   	}

   	if ((val=getkey("ident_timeout",daten)) != "ERROR")
   	{
   		ident_timeout = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "ident_timeout missing\n";
   		
   		ok = 0;
   	}

   	if ((val=getkey("read_write_timeout",daten)) != "ERROR")
   	{
   		read_write_timeout = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "read_write_timeout missing\n";
   		
   		ok = 0;
   	}

 
   	if ((val=getkey("admin_list",daten)) != "ERROR")
   	{
   		admin_list = val;
   	}
   	else
   	{
   		cout << "admin_list missing\n";
   		
   		ok = 0;
   	}
		
		if ((val=getkey("use_fxptosite_list",daten)) != "ERROR")
    {
   		use_fxptosite_list = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "use_fxptosite_list missing\n";
   		
   		ok = 0;
   	}
		
		if ((val=getkey("fxp_tosite_list",daten)) != "ERROR")
    {
   		fxp_tosite_list = val;
   	}
   	else
   	{
   		cout << "fxp_tosite_list missing\n";
   		
   		ok = 0;
   	}
		
		
		if ((val=getkey("uid",daten)) != "ERROR")
   	{
   		uid = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "uid missing\n";
   		
   		ok = 0;
   	}
   	
   	
   	if ((val=getkey("debug_logfile",daten)) != "ERROR")
   	{
   		debug_logfile = val;
   	}
   	else
   	{
   		cout << "debug_logfile missing\n";
   		
   		ok = 0;
   	}
   	
   	if ((val=getkey("command_logfile",daten)) != "ERROR")
   	{
   		command_logfile = val;
   	}
   	else
   	{
   		cout << "command_logfile missing\n";
   		
   		ok = 0;
   	}
		
		if ((val=getkey("log_to_screen",daten)) != "ERROR")
   	{
   		log_to_screen = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "log_to_screen missing\n";
   		
   		ok = 0;
   	}
		 		 		
   	
   	if ((val=getkey("use_ssl_exclude",daten)) != "ERROR")
   	{
   		use_ssl_exclude = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "use_ssl_exclude missing\n";
   		
   		ok = 0;
   	}
 		
 		if ((val=getkey("sslexclude_list",daten)) != "ERROR")
   	{
   		sslexclude_list = val;
   	}
   	else
   	{
   		cout << "sslexclude_list missing\n";
   		
   		ok = 0;
   	}
 		

 		if ((val=getkey("infocmd",daten)) != "ERROR")
   	{
   		infocmd = val;
   	}
   	else
   	{
   		cout << "infocmd missing\n";
   		
   		ok = 0;
   	}
 		
 		if ((val=getkey("helpcmd",daten)) != "ERROR")
   	{
   		helpcmd = val;
   	}
   	else
   	{
   		cout << "helpcmd missing\n";
   		
   		ok = 0;
   	}
 		
 		if ((val=getkey("admincmd",daten)) != "ERROR")
   	{
   		admincmd = val;
   	}
   	else
   	{
   		cout << "admincmd missing\n";
   		
   		ok = 0;
   	}
 		
 	if ((val=getkey("tositecmd",daten)) != "ERROR")
   	{
   		tositecmd = val;
   	}
   	else
   	{
   		cout << "tositecmd missing\n";
   		
   		ok = 0;
   	}
   	
   	if ((val=getkey("killcmd",daten)) != "ERROR")
   	{
   		killcmd = val;
   	}
   	else
   	{
   		cout << "killcmd missing\n";
   		
   		ok = 0;
   	}
 		
 	if ((val=getkey("fromsitecmd",daten)) != "ERROR")
   	{
   		fromsitecmd = val;
   	}
   	else
   	{
   		cout << "fromsitecmd missing\n";
   		
   		ok = 0;
   	}
 		
 		if ((val=getkey("sslexcludecmd",daten)) != "ERROR")
   	{
   		sslexcludecmd = val;
   	}
   	else
   	{
   		cout << "sslexcludecmd missing\n";
   		
   		ok = 0;
   	}
 		
 		if ((val=getkey("site_closed",daten)) != "ERROR")
   	{
   		site_closed = val;
   	}
   	else
   	{
   		cout << "site_closed missing\n";
   		
   		ok = 0;
   	}
 		
 		if ((val=getkey("site_full",daten)) != "ERROR")
   	{
   		site_full = val;
   	}
   	else
   	{
   		cout << "site_full missing\n";
   		
   		ok = 0;
   	}
 		
 		if ((val=getkey("reloadcmd",daten)) != "ERROR")
   	{
   		reloadcmd = val;
   	}
   	else
   	{
   		cout << "reloadcmd missing\n";
   		
   		ok = 0;
   	}
 		
 		if ((val=getkey("usecommands",daten)) != "ERROR")
   	{
   		usecommands = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "usecommands missing\n";
   		
   		ok = 0;
   	}
   	
   	if ((val=getkey("show_connect_failmsg",daten)) != "ERROR")
   	{
   		show_connect_failmsg = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "show_connect_failmsg missing\n";
   		
   		ok = 0;
   	}
   	
   	if ((val=getkey("pidfile",daten)) != "ERROR")
   	{
   		pidfile = val;
   	}
   	else
   	{
   		cout << "pidfile missing\n";
   		
   		ok = 0;
   	}
 		
 		if ((val=getkey("connectfailmsg",daten)) != "ERROR")
   	{
   		connectfailmsg = val;
   	}
   	else
   	{
   		cout << "connectfailmsg missing\n";
   		
   		ok = 0;
   	}
 		
 		if ((val=getkey("syslog",daten)) != "ERROR")
   	{
   		syslog = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "syslog missing\n";
   		
   		ok = 0;
   	}
   	
   	if ((val=getkey("ssl_forward",daten)) != "ERROR")
   	{
   		ssl_forward = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "ssl_forward missing\n";
   		
   		ok = 0;
   	}
   	
   	if ((val=getkey("use_forwarder",daten)) != "ERROR")
   	{
   		use_forwarder = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "use_forwarder missing\n";
   		
   		ok = 0;
   	}
   	
   	if ((val=getkey("forwarder_sport",daten)) != "ERROR")
   	{
   		forwarder_sport = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "forwarder_sport missing\n";
   		
   		ok = 0;
   	}
   	
   	if ((val=getkey("forwarder_dport",daten)) != "ERROR")
   	{
   		forwarder_dport = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "forwarder_dport missing\n";
   		
   		ok = 0;
   	}
   	
   	if ((val=getkey("forwarder_ip",daten)) != "ERROR")
   	{
   		forwarder_ip = val;
   	}
   	else
   	{
   		cout << "forwarder_ip missing\n";
   		
   		ok = 0;
   	}
   	
   	if ((val=getkey("retry_count",daten)) != "ERROR")
   	{
   		retry_count = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "retry_count missing\n";
   		
   		ok = 0;
   	}
   	
   	if ((val=getkey("no_idnt_cmd",daten)) != "ERROR")
   	{
   		no_idnt_cmd = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "no_idnt_cmd missing\n";
   		
   		ok = 0;
   	}
   	
   	if ((val=getkey("ssl_ascii_cache",daten)) != "ERROR")
   	{
   		ssl_ascii_cache = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "ssl_ascii_cache missing\n";
   		
   		ok = 0;
   	}
   	
   	if ((val=getkey("cmd_prefix",daten)) != "ERROR")
   	{
   		cmd_prefix = val;
   	}
   	else
   	{
   		cout << "cmd_prefix missing\n";
   		
   		ok = 0;
   	}
   	
   	if ((val=getkey("crypted_cert",daten)) != "ERROR")
   	{
   		crypted_cert = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "crypted_cert missing\n";
   		
   		ok = 0;
   	}

    if ((val=getkey("ssl_relink",daten)) != "ERROR")
   	{
   		ssl_relink = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "ssl_relink missing\n";
   		
   		ok = 0;
   	}
   	
   	if ((val=getkey("day_limit",daten)) != "ERROR")
   	{
   		day_limit = atof(val.c_str());
   	}
   	else
   	{
   		cout << "day_limit missing\n";
   		
   		ok = 0;
   	}
   	
   	if ((val=getkey("week_limit",daten)) != "ERROR")
   	{
   		week_limit = atof(val.c_str());
   	}
   	else
   	{
   		cout << "week_limit missing\n";
   		
   		ok = 0;
   	}
   	
   	if ((val=getkey("month_limit",daten)) != "ERROR")
   	{
   		month_limit = atof(val.c_str());
   	}
   	else
   	{
   		cout << "month_limit missing\n";
   		
   		ok = 0;
   	}
   	
   	if ((val=getkey("opt_dh_file",daten)) != "ERROR")
   	{
   		opt_dh_file = val;
   	}
   	else
   	{
   		cout << "opt_dh_file missing\n";
   		
   		ok = 0;
   	}
   	
   	if ((val=getkey("entrycmd",daten)) != "ERROR")
   	{
   		entrycmd = val;
   	}
   	else
   	{
   		cout << "entrycmd missing\n";
   		
   		ok = 0;
   	}
   	
   	if ((val=getkey("translate_nosslfxp",daten)) != "ERROR")
   	{
   		translate_nosslfxp = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "translate_nosslfxp missing\n";
   		
   		ok = 0;
   	}
   	
   	if ((val=getkey("disable_noop",daten)) != "ERROR")
   	{
   		disable_noop = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "disable_noop missing\n";
   		
   		ok = 0;
   	}
   	
   	if ((val=getkey("max_numlogins",daten)) != "ERROR")
   	{
   		max_numlogins = val;
   	}
   	else
   	{
   		cout << "max_numlogins missing\n";
   		
   		ok = 0;
   	}
   	
	if ((val=getkey("relink_notls",daten)) != "ERROR")
   	{
   		relink_notls = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "relink_notls missing\n";
   		
   		ok = 0;
   	}
	
	if ((val=getkey("traffic_bnc_relink",daten)) != "ERROR")
   	{
   		traffic_bnc_relink = atoi(val.c_str());
   	}
   	else
   	{
   		cout << "traffic_bnc_relink missing\n";
   		
   		ok = 0;
   	}

   	for(unsigned int i=0;i < daten.length();i++)
   	{
   		daten[i] = '0';
   	}
		
 		if (ok == 1) return 1;
 		else return 0;
	}

}
