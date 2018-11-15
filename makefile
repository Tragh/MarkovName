all:
	g++ -std=gnu++11 -march=native -O2 -pipe -fomit-frame-pointer -fweb -frename-registers main.cc -o main
