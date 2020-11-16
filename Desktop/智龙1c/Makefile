KERN_DIR = /Workstation/tools/kernel/linux-3.0.101

all:
	make -C $(KERN_DIR) M=`pwd` modules
	
clean:
	make -C $(KERN_DIR) M=`pwd` modules clean

obj-m += dht1_drv.o 
gcc:
	mipsel-linux-gcc -o socket_dht1 socket.c -lpthread
	mipsel-linux-gcc -o test_io_test test_io_test.c
cp:
	cp *.ko test_io_test socket_dht1 /work/nfs_root/ -rf
