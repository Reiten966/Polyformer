#if defined(bcm2835)
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "bcm2835_stream.h"

uint32_t millis()
{
	struct timeval now;
	gettimeofday(&now, NULL);
	return (uint32_t) ( now.tv_usec / 1000 );
}

Stream::Stream(const char* port)
{
	Stream::port = port;
}

void Stream::begin(unsigned long baud, int flags)
{
	struct termios options;

	fd = open(port, flags);
	if (fd == -1) {
		printf("[ERROR] UART open(%s)\n", port);
		return;
	}
	fcntl(fd, F_SETFL, O_RDWR);

	// Use 8 data bit, no parity and 1 stop bit
	tcgetattr(fd, &options);
	options.c_cflag = CS8 | CLOCAL | CREAD | CBAUDEX;
	options.c_cflag &= ~CBAUD;	// use the extended baud
	options.c_cflag &= ~PARENB;	// no parity
	options.c_cflag &= ~CSTOPB;	// 1 stop bit
	options.c_iflag = IGNPAR;
	options.c_oflag = baud;
	options.c_lflag = baud;
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &options);
}

void Stream::end()
{
	::close(fd);
}

int Stream::available()
{
	int result;
	if (ioctl(fd, FIONREAD, &result) == -1)
		return -1;
	return result;
}

uint8_t Stream::write(const uint8_t data)
{
    return (uint8_t)::write(fd, &data, 1);
}

uint8_t Stream::read()
{
	uint8_t data = -1;
	if (::read(fd, &data, 1) == -1)
		return -1;
	return data;
}

Stream Serial("/dev/serial0");
Stream Serial1("/dev/serial1");

#endif
