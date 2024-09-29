#include <fcntl.h>
#include <stdexcept>
#include <termios.h>
#include <unistd.h>

const char *port = "/dev/ttyACM0";
const speed_t baud = B115200;
int fd = -1;
struct termios config;

void openPort() {
  fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
  if (fd < 0) {
    throw std::runtime_error("failed to open port");
  }

  if (!isatty(fd)) {
    throw std::runtime_error("not a tty device");
  }

  if (tcgetattr(fd, &config) < 0) {
    throw std::runtime_error("cannot get serial device config");
  }

  cfmakeraw(&config);
  config.c_cflag |= (CLOCAL | CREAD);
  config.c_iflag &= ~(IXON | IXOFF | IXANY);

  config.c_cc[VMIN] = 0;
  config.c_cc[VTIME] = 0;

  if (cfsetispeed(&config, baud) < 0 || cfsetospeed(&config, baud) < 0) {
    throw std::runtime_error("cannot set baud rate");
  }

  if (tcsetattr(fd, TCSANOW, &config) < 0) {
    throw std::runtime_error("failed to configure port");
  }
}

int main(int argc, char **argv) {
  openPort();
  close(fd);
  return 0;
}
