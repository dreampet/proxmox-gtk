all: proxmox-gtk

CC = gcc
CXX = g++
LIBS=`pkg-config --libs gtk+-3.0 libcurl`
CFLAGS=`pkg-config --cflags gtk+-3.0 libcurl` -g
CXXFLAGS=`pkg-config --cflags gtk+-3.0 libcurl` \
	-std=c++11 \
	-g

OBJS = pveapi.o \
	proxmox-gtk.o  \
	curlite/curlite.o  \
	Jzon/Jzon.o

proxmox-gtk: $(OBJS)
	$(CXX) -o $@ $^ $(LIBS)

clean:
	rm -f proxmox-gtk $(OBJS)
