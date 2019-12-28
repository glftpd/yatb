#CPPFLAGS = -W -Wall -g -I/usr/local/ssl/include -Iinclude
CPPFLAGS = -W -Wall -O2 -I/usr/local/ssl/include -Iinclude

.cc.o   :
	 g++ -c $(CPPFLAGS) $< -o $@

all:
	@echo "To compile yatb type"
	@echo "  - 'make linux' (linux-debug,linux-static,linux-debug-static) to compile under linux"
	@echo "  - or 'make bsd' (bsd-debug,bsd-static,bsd-debug-static) to compile under bsd"
	@echo "  - or 'make cygwin' (cygwin-debug,cygwin-static,cygwin-debug-static) to compile under cygwin"
	@echo "  - or 'make solaris' (solaris-debug,solaris-static,solaris-debug-static) to compile under solaris"
	@echo "  - or 'make clean'"

include/tls_dh.h :
	openssl dhparam -noout -C 2048 >>include/tls_dh.h

linux: include/tls_dh.h src/fpwhitelist.o src/whitelist.o src/iplist.o src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o src/blowcrypt.o src/bnccheck.o src/getfp.o
	g++ src/fpwhitelist.o src/whitelist.o src/iplist.o src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -lcrypto -o bin/yatb; strip bin/yatb
	g++ src/blowcrypt.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/blowcrypt -lssl -lcrypto -lpthread; strip bin/blowcrypt
	g++ src/bnccheck.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/bnccheck -lssl -lcrypto -lpthread; strip bin/bnccheck
	g++ src/getfp.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/getfp -lssl -lcrypto -lpthread; strip bin/getfp

linux-debug: include/tls_dh.h src/fpwhitelist.o src/whitelist.o src/iplist.o src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o src/blowcrypt.o src/bnccheck.o src/getfp.o
	g++ src/fpwhitelist.o src/whitelist.o src/iplist.o src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -lcrypto -o bin/yatb-debug
	g++ src/blowcrypt.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/blowcrypt-debug -lssl -lcrypto -lpthread
	g++ src/bnccheck.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/bnccheck-debug -lssl -lcrypto -lpthread
	g++ src/getfp.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/getfp-debug -lssl -lcrypto -lpthread
	
linux-static: include/tls_dh.h src/fpwhitelist.o src/whitelist.o src/iplist.o src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o src/blowcrypt.o src/bnccheck.o src/getfp.o
	g++ -static src/fpwhitelist.o src/whitelist.o src/iplist.o src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -lcrypto -ldl -lz -o bin/yatb-static; strip bin/yatb-static
	g++ -static src/blowcrypt.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/blowcrypt-static -lssl -lcrypto -lpthread -ldl -lz; strip bin/blowcrypt-static
	g++ -static src/bnccheck.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/bnccheck-static -lssl -lcrypto -lpthread -ldl -lz; strip bin/bnccheck-static
	g++ -static src/getfp.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/getfp-static -lssl -lcrypto -lpthread -ldl -lz; strip bin/getfp-static
	
linux-debug-static: include/tls_dh.h src/fpwhitelist.o src/whitelist.o src/iplist.o src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o src/blowcrypt.o src/bnccheck.o src/getfp.o
	g++ -static src/fpwhitelist.o src/whitelist.o src/iplist.o src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -lcrypto -o bin/yatb-static-debug
	g++ -static src/blowcrypt.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/blowcrypt-static-debug -lssl -lcrypto -lpthread -ldl -lz
	g++ -static src/bnccheck.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/bnccheck-static-debug -lssl -lcrypto -lpthread -ldl -lz
	g++ -static src/getfp.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/getfp-static-debug -lssl -lcrypto -lpthread -ldl -lz
	
bsd: include/tls_dh.h src/fpwhitelist.o src/whitelist.o src/iplist.o src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o src/blowcrypt.o src/bnccheck.o src/getfp.o
	g++ src/fpwhitelist.o src/whitelist.o src/iplist.o src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -pthread -lcrypto -o bin/yatb; strip bin/yatb
	g++ src/blowcrypt.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/blowcrypt -lssl -lcrypto -pthread; strip bin/blowcrypt
	g++ src/bnccheck.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/bnccheck -lssl -lcrypto -pthread; strip bin/bnccheck
	g++ src/getfp.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/getfp -lssl -lcrypto -pthread; strip bin/getfp
	
bsd-debug: include/tls_dh.h src/fpwhitelist.o src/whitelist.o src/iplist.o src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o src/blowcrypt.o src/bnccheck.o src/getfp.o
	g++ src/fpwhitelist.o src/whitelist.o src/iplist.o src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -pthread -lcrypto -o bin/yatb-debug
	g++ src/blowcrypt.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/blowcrypt-debug -lssl -lcrypto -pthread
	g++ src/bnccheck.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/bnccheck-debug -lssl -lcrypto -pthread
	g++ src/getfp.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/getfp-debug -lssl -lcrypto -pthread
	
bsd-static: include/tls_dh.h src/fpwhitelist.o src/whitelist.o src/iplist.o src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o src/blowcrypt.o src/bnccheck.o src/getfp.o
	g++ -static src/fpwhitelist.o src/whitelist.o src/iplist.o src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -pthread -lcrypto -ldl -lz -o bin/yatb-static; strip bin/yatb-static
	g++ -static src/blowcrypt.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/blowcrypt-static -lssl -lcrypto -pthread -ldl -lz; strip bin/blowcrypt-static
	g++ -static src/bnccheck.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/bnccheck-static -lssl -lcrypto -pthread -ldl -lz; strip bin/bnccheck-static
	g++ -static src/getfp.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/getfp-static -lssl -lcrypto -pthread -ldl -lz; strip bin/getfp-static
	
bsd-debug-static: include/tls_dh.h src/fpwhitelist.o src/iplist.o src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o src/blowcrypt.o src/bnccheck.o src/getfp.o
	g++ -static src/fpwhitelist.o src/iplist.o src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -pthread -lcrypto -ldl -lz -o bin/yatb-static-debug
	g++ -static src/blowcrypt.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/blowcrypt-static-debug -lssl -lcrypto -pthread -ldl -lz
	g++ -static src/bnccheck.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/bnccheck-static-debug -lssl -lcrypto -pthread -ldl -lz
	g++ -static src/getfp.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/getfp-static-debug -lssl -lcrypto -pthread -ldl -lz
	
cygwin: include/tls_dh.h src/fpwhitelist.o src/whitelist.o src/iplist.o src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o src/blowcrypt.o src/bnccheck.o src/getfp.o
	g++ src/fpwhitelist.o src/whitelist.o src/iplist.o src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -lcrypto -o bin/yatb; strip bin/yatb.exe
	g++ src/blowcrypt.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/blowcrypt -lssl -lcrypto -lpthread; strip bin/blowcrypt.exe
	g++ src/bnccheck.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/bnccheck -lssl -lcrypto -lpthread; strip bin/bnccheck.exe
	g++ src/getfp.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/getfp -lssl -lcrypto -lpthread; strip bin/getfp.exe
	
cygwin-debug: include/tls_dh.h src/fpwhitelist.o src/whitelist.o src/iplist.o src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o src/blowcrypt.o src/bnccheck.o src/getfp.o
	g++ src/fpwhitelist.o src/whitelist.o src/iplist.o src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -lcrypto -o bin/yatb-debug
	g++ src/blowcrypt.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/blowcrypt-debug -lssl -lcrypto -lpthread
	g++ src/bnccheck.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/bnccheck-debug -lssl -lcrypto -lpthread
	g++ src/getfp.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/getfp-debug -lssl -lcrypto -lpthread
	
cygwin-static: include/tls_dh.h src/fpwhitelist.o src/whitelist.o src/iplist.o src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o src/blowcrypt.o src/bnccheck.o src/getfp.o
	g++ -static src/fpwhitelist.o src/whitelist.o src/iplist.o src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -lcrypto -ldl -lz -o bin/yatb-static; strip bin/yatb-static.exe
	g++ -static src/blowcrypt.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/blowcrypt-static -lssl -lcrypto -lpthread -ldl -lz; strip bin/blowcrypt-static.exe
	g++ -static src/bnccheck.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/bnccheck-static -lssl -lcrypto -lpthread -ldl -lz; strip bin/bnccheck-static.exe
	g++ -static src/getfp.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/getfp-static -lssl -lcrypto -lpthread -ldl -lz; strip bin/getfp-static.exe
	
cygwin-debug-static: include/tls_dh.h src/fpwhitelist.o src/whitelist.o src/iplist.o src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o src/blowcrypt.o src/bnccheck.o src/getfp.o
	g++ -static src/fpwhitelist.o src/whitelist.o src/iplist.o src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -lcrypto -ldl -lz -o bin/yatb-static-debug
	g++ -static src/blowcrypt.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/blowcrypt-static-debug -lssl -lcrypto -lpthread -ldl -lz
	g++ -static src/bnccheck.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/bnccheck-static-debug -lssl -lcrypto -lpthread -ldl -lz
	g++ -static src/getfp.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/getfp-static-debug -lssl -lcrypto -lpthread -ldl -lz
	
solaris: include/tls_dh.h src/fpwhitelist.o src/whitelist.o src/iplist.o src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o src/blowcrypt.o src/bnccheck.o src/getfp.o
	g++ src/fpwhitelist.o src/whitelist.o src/iplist.o src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -lcrypto -o bin/yatb; strip bin/yatb
	g++ src/blowcrypt.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/blowcrypt -lssl -lcrypto -lpthread; strip bin/blowcrypt
	g++ src/bnccheck.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/bnccheck -lssl -lcrypto -lpthread; strip bin/bnccheck
	g++ src/getfp.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/getfp -lssl -lcrypto -lpthread; strip bin/getfp
	
solaris-debug: include/tls_dh.h src/fpwhitelist.o src/whitelist.o src/iplist.o src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o src/blowcrypt.o src/bnccheck.o src/getfp.o
	g++ src/fpwhitelist.o src/whitelist.o src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -lcrypto -o bin/yatb-debug
	g++ src/blowcrypt.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/blowcrypt-debug -lssl -lcrypto -lpthread
	g++ src/bnccheck.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/bnccheck-debug -lssl -lcrypto -lpthread
	g++ src/getfp.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/getfp-debug -lssl -lcrypto -lpthread
	
solaris-static: include/tls_dh.h src/fpwhitelist.o src/whitelist.o src/iplist.o src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o src/blowcrypt.o src/bnccheck.o src/getfp.o
	g++ -static src/fpwhitelist.o src/whitelist.o src/iplist.o src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -lcrypto -ldl -lz -o bin/yatb-static; strip bin/yatb-static
	g++ -static src/blowcrypt.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/blowcrypt-static -lssl -lcrypto -lpthread -ldl -lz; strip bin/blowcrypt-static
	g++ -static src/bnccheck.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/bnccheck-static -lssl -lcrypto -lpthread -ldl -lz; strip bin/bnccheck-static
	g++ -static src/getfp.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/getfp-static -lssl -lcrypto -lpthread -ldl -lz; strip bin/getfp-static
	
solaris-debug-static: include/tls_dh.h src/fpwhitelist.o src/whitelist.o src/iplist.o src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o src/blowcrypt.o src/bnccheck.o src/getfp.o
	g++ -static src/fpwhitelist.o src/whitelist.o src/iplist.o src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -lcrypto -ldl -lz -o bin/yatb-static-debug
	g++ -static src/blowcrypt.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/blowcrypt-static-debug -lssl -lcrypto -lpthread -ldl -lz
	g++ -static src/bnccheck.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/bnccheck-static-debug -lssl -lcrypto -lpthread -ldl -lz
	g++ -static src/getfp.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/getfp-static-debug -lssl -lcrypto -lpthread -ldl -lz
       
clean:
	@(rm -f bin/yatb bin/blowcrypt bin/yatb-static bin/blowcrypt-static bin/yatb-debug bin/blowcrypt-debug bin/yatb-static-debug bin/blowcrypt-static-debug bin/bnccheck bin/bnccheck-static bin/bnccheck-debug bin/bnccheck-static-debug bin/*.exe src/*.o bin/getfp* include/tls_dh.h)
	@(echo "Clean succesful")
