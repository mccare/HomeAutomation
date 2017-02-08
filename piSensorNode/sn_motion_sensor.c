
#include <wiringPi.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

void motion_detected();

#define LOG(...) do { printf(__VA_ARGS__); fflush(stdout); } while (0)

#define NAMED_PIPE "/tmp/SenderReceiverIO.named_pipe"
// change the Device ID in the following string
#define MOTION_DETECTED_STRING "DEVICE_ID:11:VALUE:1"
// wiring pi numbering: http://wiringpi.com/pins/
#define PIN_NUMBER 0

int main(int argc, char **argv) {

  wiringPiSetup();
  wiringPiISR(PIN_NUMBER, INT_EDGE_RISING, motion_detected);
  LOG( "sn_motion_service launched, ready\n");
  for(;;) {
    usleep(1000);
  }
  return 0;
}

void setup() {
  if ( ! access( NAMED_PIPE, W_OK ) ) {
    LOG( "Missing named pipe at %s\n", NAMED_PIPE);
    exit(1);
  }
}

void motion_detected() {
	LOG( " MOTION DETECTED! \n");
  int file ;
  char buffer[30];
  file = open(NAMED_PIPE, O_WRONLY );
  if (file > 0) {
    write(file, MOTION_DETECTED_STRING, strlen(MOTION_DETECTED_STRING));
    close(file);
  } else {
    LOG( "Cannot write to file\n");
  }
}
