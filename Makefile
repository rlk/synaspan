TIFF = -I/usr/local/include -L/usr/local/lib -ltiff -lm

synaspan : synaspan.c Makefile
	cc -Wall -O3 -o synaspan synaspan.c $(TIFF)
