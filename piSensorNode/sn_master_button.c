
#include <wiringPi.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

void master_button_pressed();

#define LOG(...) do { printf(__VA_ARGS__); fflush(stdout); } while (0)

#define NAMED_PIPE "/tmp/SenderReceiverIO.named_pipe"
// change the Device ID in the following string
#define BUTTON_PRESSED_STRING "DEVICE_ID:12:VALUE:1"
// wiring pi numbering: http://wiringpi.com/pins/
#define PIN_NUMBER 2

int main(int argc, char **argv) {

  wiringPiSetup();
  wiringPiISR(PIN_NUMBER, INT_EDGE_RISING, master_button_pressed);
  for(;;) {
    usleep(1000);
  }
  return 0;
}

void setup() {
  if ( ! access( NAMED_PIPE, W_OK ) ) {
    LOG("Missing named pipe at %s", NAMED_PIPE);
    exit(1);
  }
}

void master_button_pressed() {
	LOG(" MASTER BUTTON PRESSED DETECTED! \n");
  int file ;
  char buffer[30];
  file = open(NAMED_PIPE, O_WRONLY );
  if (file > 0) {
    write(file, BUTTON_PRESSED_STRING, strlen(BUTTON_PRESSED_STRING));
    close(file);
  } else {
    LOG("Cannot write to file\n");
  }
}
