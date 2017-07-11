#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

#define TERM		"\r"
#define TERM_CODE	'\r'

int
main(int argc, char *argv[])
{
	if (argc != 3) {
		printf("please specify device file and command.\n");
		printf("    ./exe device_file command\n");
		return -1;
	}

	char *dfile = argv[1];
	unsigned char *c = argv[2];

	int USB = open(dfile, O_RDWR | O_NOCTTY);

	struct termios tty;
	struct termios tty_old;
	memset(&tty, 0, sizeof(tty));

	if (tcgetattr(USB, &tty) != 0) {
		printf("error 1\n");
	}

	// store initial setting
	tty_old = tty;

	// set baud rate
	cfsetospeed(&tty, (speed_t)B115200);
	cfsetispeed(&tty, (speed_t)B115200);

	// 8bit, none parity, stop 1bit
	tty.c_cflag |= CS8;

	// none flow control
	tty.c_cflag &= ~CRTSCTS;

	tcflush(USB, TCIFLUSH);
	if (tcsetattr(USB, TCSANOW, &tty) != 0) {
		printf("error 2\n");
	}

	unsigned char *cmd = strcat(c, TERM);
	int n = 0, spot = 0;

	do {
		n = write(USB, &cmd[spot], 1);
		spot += n;
	} while (cmd[spot - 1] != TERM_CODE && n > 0);

	char buf = '\0';
	char response[1024];
	memset(response, '\0', sizeof(response));
	n = spot = 0;

	do {
		n = read(USB, &buf, 1);
		sprintf(&response[spot], "%c", buf);
		spot += n;
	} while (buf != TERM_CODE && n > 0);

	printf("%s\n", response);
}
