CPPFLAGS = -W -Wall -g -I/usr/local/ssl/include -Iinclude

.cc.o   :
	 g++ -c $(CPPFLAGS) $< -o $@

all:
	@echo "To compile yatb type"
	@echo "  - 'make linux' (linux-debug,linux-static,linux-debug-static) to compile under linux"
	@echo "  - or 'make bsd' (bsd-debug,bsd-static,bsd-debug-static) to compile under bsd"
	@echo "  - or 'make cygwin' (cygwin-debug,cygwin-static,cygwin-debug-static) to compile under cygwin"
	@echo "  - or 'make solaris' (solaris-debug,solaris-static,solaris-debug-static) to compile under solaris"
	@echo "  - or 'make clean'"

linux: src/iplist.o src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o src/blowcrypt.o src/bnccheck.o
	g++ src/iplist.o src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -lcrypto -o bin/yatb; strip bin/yatb
	g++ src/blowcrypt.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/blowcrypt -lssl -lcrypto; strip bin/blowcrypt
	g++ src/bnccheck.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/bnccheck -lssl -lcrypto; strip bin/bnccheck

linux-debug: src/iplist.o src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o src/blowcrypt.o src/bnccheck.o
	g++ src/iplist.o src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -lcrypto -o bin/yatb-debug
	g++ src/blowcrypt.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/blowcrypt-debug -lssl -lcrypto
	g++ src/bnccheck.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/bnccheck-debug -lssl -lcrypto

linux-static: src/iplist.o src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o src/blowcrypt.o src/bnccheck.o
	g++ -static src/iplist.o src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -lcrypto -o bin/yatb-static; strip bin/yatb-static
	g++ -static src/blowcrypt.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/blowcrypt-static -lssl -lcrypto -lpthread; strip bin/blowcrypt-static
	g++ -static src/bnccheck.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/bnccheck-static -lssl -lcrypto -lpthread; strip bin/bnccheck-static

linux-debug-static: src/iplist.o src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o src/blowcrypt.o src/bnccheck.o
	g++ -static src/iplist.o src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -lcrypto -o bin/yatb-static-debug
	g++ -static src/blowcrypt.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/blowcrypt-static-debug -lssl -lcrypto -lpthread
	g++ -static src/bnccheck.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/bnccheck-static-debug -lssl -lcrypto -lpthread

bsd: src/iplist.o src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o src/blowcrypt.o src/bnccheck.o
	g++ src/iplist.o src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -pthread -lcrypto -o bin/yatb; strip bin/yatb
	g++ src/blowcrypt.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/blowcrypt -lssl -lcrypto; strip bin/blowcrypt
	g++ src/bnccheck.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/bnccheck -lssl -lcrypto; strip bin/bnccheck
	
bsd-debug: src/iplist.o src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o src/blowcrypt.o src/bnccheck.o
	g++ src/iplist.o src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -pthread -lcrypto -o bin/yatb-debug
	g++ src/blowcrypt.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/blowcrypt-debug -lssl -lcrypto
	g++ src/bnccheck.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/bnccheck-debug -lssl -lcrypto

bsd-static: src/iplist.o src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o src/blowcrypt.o src/bnccheck.o
	g++ -static src/iplist.o src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -pthread -lcrypto -o bin/yatb-static; strip bin/yatb-static
	g++ -static src/blowcrypt.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/blowcrypt-static -lssl -lcrypto -pthread; strip bin/blowcrypt-static
	g++ -static src/bnccheck.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/bnccheck-static -lssl -lcrypto -pthread; strip bin/bnccheck-static

bsd-debug-static: src/iplist.o src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o src/blowcrypt.o src/bnccheck.o
	g++ -static src/iplist.o src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -pthread -lcrypto -o bin/yatb-static-debug
	g++ -static src/blowcrypt.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/blowcrypt-static-debug -lssl -lcrypto -pthread
	g++ -static src/bnccheck.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/bnccheck-static-debug -lssl -lcrypto -pthread

cygwin: src/iplist.o src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o src/blowcrypt.o src/bnccheck.o
	g++ src/iplist.o src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -lcrypto -o bin/yatb; strip bin/yatb.exe
	g++ src/blowcrypt.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/blowcrypt -lssl -lcrypto; strip bin/blowcrypt.exe
	g++ src/bnccheck.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/bnccheck -lssl -lcrypto; strip bin/bnccheck.exe

cygwin-debug: src/iplist.o src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o src/blowcrypt.o src/bnccheck.o
	g++ src/iplist.o src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -lcrypto -o bin/yatb-debug
	g++ src/blowcrypt.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/blowcrypt-debug -lssl -lcrypto
	g++ src/bnccheck.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/bnccheck-debug -lssl -lcrypto

cygwin-static: src/iplist.o src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o src/blowcrypt.o src/bnccheck.o
	g++ -static src/iplist.o src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -lcrypto -o bin/yatb-static; strip bin/yatb-static.exe
	g++ -static src/blowcrypt.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/blowcrypt-static -lssl -lcrypto -lpthread; strip bin/blowcrypt-static.exe
	g++ -static src/bnccheck.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/bnccheck-static -lssl -lcrypto -lpthread; strip bin/bnccheck-static.exe

cygwin-debug-static: src/iplist.o src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o src/blowcrypt.o src/bnccheck.o
	g++ -static src/iplist.o src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -lcrypto -o bin/yatb-static-debug
	g++ -static src/blowcrypt.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/blowcrypt-static-debug -lssl -lcrypto -lpthread
	g++ -static src/bnccheck.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/bnccheck-static-debug -lssl -lcrypto -lpthread

solaris: src/iplist.o src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o src/blowcrypt.o src/bnccheck.o
	g++ src/iplist.o src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -lcrypto -o bin/yatb; strip bin/yatb
	g++ src/blowcrypt.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/blowcrypt -lssl -lcrypto; strip bin/blowcrypt
	g++ src/bnccheck.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/bnccheck -lssl -lcrypto; strip bin/bnccheck

solaris-debug: src/iplist.o src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o src/blowcrypt.o src/bnccheck.o
	g++ src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -lcrypto -o bin/yatb-debug
	g++ src/blowcrypt.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/blowcrypt-debug -lssl -lcrypto
	g++ src/bnccheck.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/bnccheck-debug -lssl -lcrypto

solaris-static: src/iplist.o src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o src/blowcrypt.o src/bnccheck.o
	g++ -static src/iplist.o src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -lcrypto -o bin/yatb-static; strip bin/yatb-static
	g++ -static src/blowcrypt.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/blowcrypt-static -lssl -lcrypto -lpthread; strip bin/blowcrypt-static
	g++ -static src/bnccheck.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/bnccheck-static -lssl -lcrypto -lpthread; strip bin/bnccheck-static

solaris-debug-static: src/iplist.o src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o src/blowcrypt.o src/bnccheck.o
	g++ -static src/iplist.o src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -lcrypto -o bin/yatb-static-debug
	g++ -static src/blowcrypt.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/blowcrypt-static-debug -lssl -lcrypto -lpthread
	g++ -static src/bnccheck.o src/config.o src/lock.o src/counter.o src/tools.o -o bin/bnccheck-static-debug -lssl -lcrypto -lpthread

       
clean:
	@(rm -f bin/yatb bin/blowcrypt bin/yatb-static bin/blowcrypt-static bin/yatb-debug bin/blowcrypt-debug bin/yatb-static-debug bin/blowcrypt-static-debug bin/bnccheck bin/bnccheck-static bin/bnccheck-debug bin/bnccheck-static-debug bin/*.exe src/*.o)
	@(echo "Clean succesful")
