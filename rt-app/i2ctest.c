#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>

int main(int argc, char **argv) {


	mknod("/var/dev/i2c", S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH, makedev(10,0));

	int i2c_fd = open("/var/dev/i2c", O_RDWR);

	char lastBuf[1];
	char buf[1];

	int err;

	while(1) {
		if((err = read(i2c_fd, buf, 1)) < 0) {
			printf("i2c read error : %d\n", err);
		}

		if(buf[0] != lastBuf[0]) {
			lastBuf[0] = buf[0];
			printf("Lecture : %0X\n", buf[0]);
		}
	}

	close(i2c_fd);

	return 0;
}
