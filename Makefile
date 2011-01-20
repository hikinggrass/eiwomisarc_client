$(shell ./gitversionscript.sh)
linux: main.c messages.h git_rev.h
	gcc main.c /usr/lib/libargtable2.a -o eiwomisarc_client_linux
arm: main.c messages.h git_rev.h
	arm-linux-gnueabi-gcc main.c libargtable2.a -o eiwomisarc_client_armlinux
