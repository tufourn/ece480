#include <cassert>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

const char *port = "/dev/ttyACM0";
constexpr speed_t baud = B115200;
constexpr int nozzleCount = 12;
constexpr uint8_t threshold = 128;

/*
 * read a byte from serial
 */
unsigned char readChar(int fd) {
  unsigned char c;
  int bytesRead = read(fd, &c, 1);
  if (bytesRead > 0) {
    return c;
  }
  return 0;
}

/*
 * write a byte to serial, block until arduino receives it
 */
void writeChar(int fd, unsigned char charToWrite) {
  int bytesWritten = write(fd, &charToWrite, 1);
  if (bytesWritten < 0) {
    std::cerr << "Error writing\n";
  }
  while (readChar(fd) != 'R') {
  } // block until arduino sends a 'R' back indicating byte received
}

/*
 * opens serial port, returns negative if fail
 */
int openPort(const char *port) {
  struct termios config;

  int fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
  if (fd < 0) {
    std::cerr << "failed to open port\n";
    return -1;
  }

  if (!isatty(fd)) {
    std::cerr << "not a tty device\n";
    return -1;
  }

  if (tcgetattr(fd, &config) < 0) {
    std::cerr << "cannot get serial device config\n";
    return -1;
  }

  if (cfsetispeed(&config, baud) < 0 || cfsetospeed(&config, baud) < 0) {
    std::cerr << "cannot set baud rate\n";
    return -1;
  }

  // https://stackoverflow.com/questions/27667299/
  cfmakeraw(&config);
  config.c_iflag &= ~(IXON | IXOFF | IXANY);
  config.c_cflag |= (CLOCAL | CREAD);

  // http://unixwiz.net/techtips/termios-vmin-vtime.html
  config.c_cc[VMIN] = 0;
  config.c_cc[VTIME] = 20;

  if (tcsetattr(fd, TCSANOW, &config) < 0) {
    std::cerr << "failed to configure port\n";
    return -1;
  }

  return fd;
}

/*
 * parse an image into a 2D vector of 0 and 1 based on threshold value
 * returns empty vector if fail
 */
std::vector<std::vector<uint8_t>> parseImage(const char *imagePath) {
  // default stb_image's first pixel is top left, we want to start bottom left
  stbi_set_flip_vertically_on_load(true);

  // open image, read gray channel
  int width, height, channel;
  unsigned char *image =
      stbi_load(imagePath, &width, &height, &channel, STBI_grey);
  if (image == nullptr) {
    std::cerr << "failed to open: " << imagePath << std::endl;
    return {};
  }

  // pad height so it's divisible by nozzleCount
  int remainder = height % nozzleCount;
  int paddedHeight = height + (remainder != 0 ? nozzleCount - remainder : 0);

  // generate dot matrix based on threshold, higher is 0, lower is 1
  std::vector<std::vector<uint8_t>> dotMatrix(paddedHeight,
                                              std::vector<uint8_t>(width));
  for (size_t row = 0; row < height; row++) {
    for (size_t col = 0; col < width; col++) {
      if (image[row * width + col] > threshold) {
        dotMatrix[row][col] = 0;
      } else {
        dotMatrix[row][col] = 1;
      }
    }
  }

  // write the dot matrix to a text file for testing
  std::ofstream outFile("dot_matrix.txt");
  if (!outFile.is_open()) {
    std::cerr << "failed to open output file." << std::endl;
    stbi_image_free(image);
    return {};
  }
  for (size_t row = 0; row < dotMatrix.size(); row++) {
    for (size_t col = 0; col < dotMatrix[row].size(); col++) {
      outFile << static_cast<int>(dotMatrix[row][col]);
    }
    outFile << "\n";
  }
  outFile.close();

  // free stb image buffer
  stbi_image_free(image);

  return dotMatrix;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "usage: ./main <image_file>\n";
    return -1;
  }

  const char *imagePath = argv[1];

  std::vector<std::vector<uint8_t>> dotMatrix = parseImage(imagePath);
  if (dotMatrix.empty()) {
    std::cerr << "failed to parse image\n";
    return -1;
  }

  int fd = openPort(port);
  if (fd < 0) {
    std::cerr << "failed to set up serial communication\n";
    return -1;
  }

  // arduino sends out 'X' when it finishes resetting
  std::cout << "waiting for connection: ";
  while (readChar(fd) != 'X') {
    std::cout << ".";
    usleep(10000);
  }

  std::cout << "\n\nconnection established\n\n";

  for (char c = 'A'; c <= 'Z'; c++) {
    writeChar(fd, c);
    std::cout << "sent: " << c << std::endl;
  }

  close(fd);
}
