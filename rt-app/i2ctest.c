#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>

int main(int argc, char **argv) {


	mknod("/var/dev/i2c", S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH, makedev(89,0));

	int i2c_fd = open("/var/dev/i2c", O_RDWR);

	char buf[2];
	while(1) {
		read(i2c_fd, buf, 2);
		printf("Lecture %X",(int)buf);
	}

	return 0;

}
