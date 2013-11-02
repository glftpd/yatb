CPPFLAGS = -W -Wall -g -I/usr/local/ssl/include

.cc.o   :
	 g++ -c $(CPPFLAGS) $< 

all:
	@echo "To compile yatb type"
	@echo "  - 'make linux' (linux-debug,linux-static,linux-debug-static) to compile under linux"
	@echo "  - or 'make bsd' (bsd-debug,bsd-static,bsd-debug-static) to compile under bsd"
	@echo "  - or 'make cygwin' (cygwin-debug,cygwin-static,cygwin-debug-static) to compile under cygwin"
	@echo "  - or 'make solaris' (solaris-debug,solaris-static,solaris-debug-static) to compile under solaris"
	@echo "  - or 'make clean'"

linux: yatb.o forward.o counter.o controlthread.o datathread.o config.o tls.o stringlist.o tools.o lock.o
	g++ yatb.o forward.o counter.o config.o controlthread.o datathread.o tls.o stringlist.o tools.o lock.o -lssl -lpthread -lcrypto -o yatb; strip yatb
	g++ -W -Wall -g blowcrypt.cc -o blowcrypt -lssl -lcrypto; strip blowcrypt
	
linux-debug: yatb.o forward.o counter.o controlthread.o datathread.o config.o tls.o stringlist.o tools.o lock.o
	g++ yatb.o forward.o counter.o config.o controlthread.o datathread.o tls.o stringlist.o tools.o lock.o -lssl -lpthread -lcrypto -o yatb-debug
	g++ -W -Wall -g blowcrypt.cc -o blowcrypt-debug -lssl -lcrypto

linux-static: yatb.o forward.o counter.o controlthread.o datathread.o config.o tls.o stringlist.o tools.o lock.o
	g++ -static yatb.o forward.o counter.o config.o controlthread.o datathread.o tls.o stringlist.o tools.o lock.o -lssl -lpthread -lcrypto -o yatb-static; strip yatb-static
	g++ -static -W -Wall -g blowcrypt.cc -o blowcrypt-static -lssl -lcrypto; strip blowcrypt-static

linux-debug-static: yatb.o forward.o counter.o controlthread.o datathread.o config.o tls.o stringlist.o tools.o lock.o
	g++ -static yatb.o forward.o counter.o config.o controlthread.o datathread.o tls.o stringlist.o tools.o lock.o -lssl -lpthread -lcrypto -o yatb-static-debug
	g++ -static -W -Wall -g blowcrypt.cc -o blowcrypt-static-debug -lssl -lcrypto

bsd: yatb.o forward.o counter.o controlthread.o datathread.o config.o tls.o stringlist.o tools.o lock.o
	g++ yatb.o forward.o counter.o config.o controlthread.o datathread.o tls.o stringlist.o tools.o lock.o -lssl -pthread -lcrypto -o yatb; strip yatb
	g++ -W -Wall -g blowcrypt.cc -o blowcrypt -lssl -lcrypto; strip blowcrypt

bsd-debug: yatb.o forward.o counter.o controlthread.o datathread.o config.o tls.o stringlist.o tools.o lock.o
	g++ -g yatb.o forward.o counter.o config.o controlthread.o datathread.o tls.o stringlist.o tools.o lock.o -lssl -pthread -lcrypto -o yatb-debug
	g++ -W -Wall -g blowcrypt.cc -o blowcrypt-debug -lssl -lcrypto

bsd-static: yatb.o forward.o counter.o controlthread.o datathread.o config.o tls.o stringlist.o tools.o lock.o
	g++ -static yatb.o forward.o counter.o config.o controlthread.o datathread.o tls.o stringlist.o tools.o lock.o -lssl -pthread -lcrypto -o yatb-static; strip yatb-static
	g++ -static -W -Wall -g blowcrypt.cc -o blowcrypt-static -lssl -lcrypto; strip blowcrypt-static

bsd-debug-static: yatb.o forward.o counter.o controlthread.o datathread.o config.o tls.o stringlist.o tools.o lock.o
	g++ -static yatb.o forward.o counter.o config.o controlthread.o datathread.o tls.o stringlist.o tools.o lock.o -lssl -pthread -lcrypto -o yatb-static-debug
	g++ -static -W -Wall -g blowcrypt.cc -o blowcrypt-static-debug -lssl -lcrypto

cygwin: yatb.o forward.o counter.o controlthread.o datathread.o config.o tls.o stringlist.o tools.o lock.o
	g++ yatb.o forward.o counter.o config.o controlthread.o datathread.o tls.o stringlist.o tools.o lock.o -lssl -lpthread -lcrypto -o yatb; strip yatb.exe
	g++ -W -Wall -g blowcrypt.cc -o blowcrypt -lssl -lcrypto; strip blowcrypt.exe
	
cygwin-debug: yatb.o forward.o counter.o controlthread.o datathread.o config.o tls.o stringlist.o tools.o lock.o
	g++ -g yatb.o forward.o counter.o config.o controlthread.o datathread.o tls.o stringlist.o tools.o lock.o -lssl -lpthread -lcrypto -o yatb-debug
	g++ -W -Wall -g blowcrypt.cc -o blowcrypt-debug -lssl -lcrypto

cygwin-static: yatb.o forward.o counter.o controlthread.o datathread.o config.o tls.o stringlist.o tools.o lock.o
	g++ -static yatb.o forward.o counter.o config.o controlthread.o datathread.o tls.o stringlist.o tools.o lock.o -lssl -lpthread -lcrypto -o yatb-static; strip yatb-static.exe
	g++ -static -W -Wall -g blowcrypt.cc -o blowcrypt-static -lssl -lcrypto; strip blowcrypt-static.exe

cygwin-debug-static: yatb.o forward.o counter.o controlthread.o datathread.o config.o tls.o stringlist.o tools.o lock.o
	g++ -static yatb.o forward.o counter.o config.o controlthread.o datathread.o tls.o stringlist.o tools.o lock.o -lssl -lpthread -lcrypto -o yatb-static-debug
	g++ -static -W -Wall -g blowcrypt.cc -o blowcrypt-static-debug -lssl -lcrypto	

solaris: yatb.o forward.o counter.o controlthread.o datathread.o config.o tls.o stringlist.o tools.o lock.o
	g++ yatb.o forward.o counter.o config.o controlthread.o datathread.o tls.o stringlist.o tools.o lock.o -lssl -lpthread -lcrypto -lnsl -lsocket -lresolv -o yatb; strip yatb
	g++ -W -Wall -g -I/usr/local/ssl/include blowcrypt.cc -o blowcrypt -lssl -lcrypto; strip blowcrypt

solaris-debug: yatb.o forward.o counter.o controlthread.o datathread.o config.o tls.o stringlist.o tools.o lock.o
	g++ yatb.o forward.o counter.o config.o controlthread.o datathread.o tls.o stringlist.o tools.o lock.o -lssl -lpthread -lcrypto -lnsl -lsocket -lresolv -o yatb-debug
	g++ -W -Wall -g -I/usr/local/ssl/include blowcrypt.cc -o blowcrypt-debug -lssl -lcrypto

solaris-static: yatb.o forward.o counter.o controlthread.o datathread.o config.o tls.o stringlist.o tools.o lock.o
	g++ -static yatb.o forward.o counter.o config.o controlthread.o datathread.o tls.o stringlist.o tools.o lock.o -lssl -lpthread -lcrypto -lnsl -lsocket -lresolv -o yatb-static; strip yatb-static
	g++ -static -W -Wall -g -I/usr/local/ssl/include blowcrypt.cc -o blowcrypt-static -lssl -lcrypto; strip blowcrypt-static

solaris-debug-static: yatb.o forward.o counter.o controlthread.o datathread.o config.o tls.o stringlist.o tools.o lock.o
	g++ -static yatb.o forward.o counter.o config.o controlthread.o datathread.o tls.o stringlist.o tools.o lock.o -lssl -lpthread -lcrypto -lnsl -lsocket -o yatb-static-debug
	g++ -static -W -Wall -g -I/usr/local/ssl/include blowcrypt.cc -o blowcrypt-static-debug -lssl -lcrypto
			
clean:
	@(rm -f yatb blowcrypt yatb-static blowcrypt-static yatb-debug blowcrypt-debug yatb-static-debug blowcrypt-static-debug *.o)
	@(echo "Clean succesful")
