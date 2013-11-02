CPPFLAGS = -W -Wall -g -I/usr/local/ssl/include -Iinclude

src/.cc.o   :
	 g++ -c $(CPPFLAGS) $< 

all:
	@echo "To compile yatb type"
	@echo "  - 'make linux' (linux-debug,linux-static,linux-debug-static) to compile under linux"
	@echo "  - or 'make bsd' (bsd-debug,bsd-static,bsd-debug-static) to compile under bsd"
	@echo "  - or 'make cygwin' (cygwin-debug,cygwin-static,cygwin-debug-static) to compile under cygwin"
	@echo "  - or 'make solaris' (solaris-debug,solaris-static,solaris-debug-static) to compile under solaris"
	@echo "  - or 'make clean'"

linux: src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o
	g++ src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -lcrypto -o bin/yatb; strip bin/yatb
	g++ -W -Wall -g src/blowcrypt.cc -o bin/blowcrypt -lssl -lcrypto; strip bin/blowcrypt

linux-debug: src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o
	g++ src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -lcrypto -o bin/yatb-debug
	g++ -W -Wall -g src/blowcrypt.cc -o bin/blowcrypt-debug -lssl -lcrypto

linux-static: src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o
	g++ -static src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -lcrypto -o bin/yatb-static; strip bin/yatb-static
	g++ -static -W -Wall -g src/blowcrypt.cc -o bin/blowcrypt-static -lssl -lcrypto; strip bin/blowcrypt-static	

linux-debug-static: src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o
	g++ -static src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -lcrypto -o bin/yatb-static-debug
	g++ -static -W -Wall -g src/blowcrypt.cc -o bin/blowcrypt-static-debug -lssl -lcrypto	

bsd: src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o
	g++ src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -crypto -o bin/yatb; strip bin/yatb
	g++ -W -Wall -g src/blowcrypt.cc -o bin/blowcrypt -lssl -lcrypto; strip bin/blowcrypt

bsd-debug: src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o
	g++ src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -crypto -o bin/yatb-debug
	g++ -W -Wall -g src/blowcrypt.cc -o bin/blowcrypt-debug -lssl -lcrypto

bsd-static: src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o
	g++ -static src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -pthread -lcrypto -o bin/yatb-static; strip bin/yatb-static
	g++ -static -W -Wall -g src/blowcrypt.cc -o bin/blowcrypt-static -lssl -lcrypto; strip bin/blowcrypt-static	

bsd-debug-static: src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o
	g++ -static src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -pthread -lcrypto -o bin/yatb-static-debug
	g++ -static -W -Wall -g src/blowcrypt.cc -o bin/blowcrypt-static-debug -lssl -lcrypto	

cygwin: src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o
	g++ src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -lcrypto -o bin/yatb; strip bin/yatb.exe
	g++ -W -Wall -g src/blowcrypt.cc -o bin/blowcrypt -lssl -lcrypto; strip bin/blowcrypt.exe

cygwin-debug: src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o
	g++ src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -lcrypto -o bin/yatb-debug
	g++ -W -Wall -g src/blowcrypt.cc -o bin/blowcrypt-debug -lssl -lcrypto

cygwin-static: src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o
	g++ -static src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -lcrypto -o bin/yatb-static; strip bin/yatb-static.exe
	g++ -static -W -Wall -g src/blowcrypt.cc -o bin/blowcrypt-static -lssl -lcrypto; strip bin/blowcrypt-static.exe	

cygwin-debug-static: src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o
	g++ -static src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -lcrypto -o bin/yatb-static-debug
	g++ -static -W -Wall -g src/blowcrypt.cc -o bin/blowcrypt-static-debug -lssl -lcrypto	

solaris: src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o
	g++ src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -lcrypto -o bin/yatb; strip bin/yatb
	g++ -W -Wall -g src/blowcrypt.cc -o bin/blowcrypt -lssl -lcrypto; strip bin/blowcrypt

solaris-debug: src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o
	g++ src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -lcrypto -o bin/yatb-debug
	g++ -W -Wall -g src/blowcrypt.cc -o bin/blowcrypt-debug -lssl -lcrypto

solaris-static: src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o
	g++ -static src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -lcrypto -o bin/yatb-static; strip bin/yatb-static
	g++ -static -W -Wall -g src/blowcrypt.cc -o bin/blowcrypt-static -lssl -lcrypto; strip bin/blowcrypt-static	

solaris-debug-static: src/yatb.o src/forward.o src/counter.o src/controlthread.o src/datathread.o src/config.o src/tls.o src/stringlist.o src/tools.o src/lock.o
	g++ -static src/yatb.o src/forward.o src/counter.o src/config.o src/controlthread.o src/datathread.o src/tls.o src/stringlist.o src/tools.o src/lock.o -lssl -lpthread -lcrypto -o bin/yatb-static-debug
	g++ -static -W -Wall -g src/blowcrypt.cc -o bin/blowcrypt-static-debug -lssl -lcrypto	

       
clean:
	@(rm -f bin/yatb bin/blowcrypt bin/yatb-static bin/blowcrypt-static bin/yatb-debug bin/blowcrypt-debug bin/yatb-static-debug bin/blowcrypt-static-debug src/*.o)
	@(echo "Clean succesful")
