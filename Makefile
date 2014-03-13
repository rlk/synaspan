TIFF = -I/usr/local/include -L/usr/local/lib -ltiff

synaspan : synaspan.c Makefile
	cc -Wall -O3 -o synaspan synaspan.c $(TIFF)

