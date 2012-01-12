#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>



int i2c_fd;

int hp;

int SW2_event, SW3_event, SW4_event, SW5_event;
int err;

void check_switch_events_once();
void hp_update_leds();

int main(int argc, char **argv) {


	mknod("/var/dev/i2c", S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH, makedev(10,0));

	i2c_fd = open("/var/dev/i2c", O_RDWR);

	hp = 4;
	hp_update_leds();

	while(1) {
		check_switch_events_once();

		if(SW2_event) {
			SW2_event = 0;

			if(hp > 0)
				hp--;

			hp_update_leds();
		}

		if(SW3_event) {
			SW3_event = 0;

			if(hp < 4)
				hp++;

			hp_update_leds();
		}

	}

	close(i2c_fd);

	return 0;
}

void check_switch_events_once() {

	char buf[1];
	char lastBuf[1];

	char switch_change, switch_change_up;

	if((err = read(i2c_fd, buf, 1)) < 0) {
		printf("i2c read error : %d\n", err);
	} else if(buf[0] != lastBuf[0]) {

		switch_change = (buf[0] ^ lastBuf[0]) >> 4;
		switch_change_up = switch_change & ~(buf[0] >> 4);

		if(switch_change_up & 0x1) {
			SW5_event = 1;
		}

		if(switch_change_up & 0x2) {
			SW4_event = 1;
		}

		if(switch_change_up & 0x4) {
			SW3_event = 1;
		}

		if(switch_change_up & 0x8) {
			SW2_event = 1;
		}

		lastBuf[0] = buf[0];
	}
}

void hp_update_leds() {

	char buf[1];

	/* Conversion int -> LEDS */
	buf[0] = 0x0F << hp;

	/* Inversion des hp pour décrémentation depuis le haut */
	switch(buf[0]) {
	case 0x1:
		buf[0] = 0x8;
		break;

	case 0x3:
		buf[0] = 0xC;
		break;

	case 0x7:
		buf[0] = 0xE;
		break;

	}

	if((err = write(i2c_fd, buf, 1)) < 0) {
		printf("i2c write error : %d\n", err);
	}
}
