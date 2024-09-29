#include <cassert>
#include <fcntl.h>
#include <iostream>
#include <stdexcept>
#include <termios.h>
#include <unistd.h>

const char *port = "/dev/ttyACM0";
const speed_t baud = B115200;
int fd = -1;
struct termios config;

unsigned char readChar() {
  unsigned char c;
  int bytesRead = read(fd, &c, 1);
  if (bytesRead > 0) {
    return c;
  }
  return 0;
}

void writeChar(unsigned char charToWrite) {
  int bytesWritten = write(fd, &charToWrite, 1);
  if (bytesWritten < 0) {
    std::cerr << "Error writing\n";
  }
  while (readChar() != 'R') {} // block until arduino receives char
}

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

  if (cfsetispeed(&config, baud) < 0 || cfsetospeed(&config, baud) < 0) {
    throw std::runtime_error("cannot set baud rate");
  }

  // https://stackoverflow.com/questions/27667299/
  cfmakeraw(&config);
  config.c_iflag &= ~(IXON | IXOFF | IXANY);
  config.c_cflag |= (CLOCAL | CREAD);

  // http://unixwiz.net/techtips/termios-vmin-vtime.html
  config.c_cc[VMIN] = 0;
  config.c_cc[VTIME] = 20;

  if (tcsetattr(fd, TCSANOW, &config) < 0) {
    throw std::runtime_error("failed to configure port");
  }
}

int main(int argc, char **argv) {
  openPort();

  std::cout << "waiting for connection: ";
  while (readChar() != 'X') {
    std::cout << ".";
    usleep(10000);
  }

  std::cout << "\n\nconnection established\n\n";

  for (char c = 'A'; c <= 'Z'; c++) {
    writeChar(c);
    std::cout << "sent: " << c << std::endl;
  }

  close(fd);
}
