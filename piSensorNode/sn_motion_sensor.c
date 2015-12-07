
#include <wiringPi.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

void motion_detected();


#define NAMED_PIPE "/tmp/SenderReceiverIO.named_pipe"
// change the Device ID in the following string
#define MOTION_DETECTED_STRING "DEVICE_ID:11:VALUE:1"

int main(int argc, char **argv) {
  
  wiringPiSetup();
  wiringPiISR(1, INT_EDGE_RISING, motion_detected);
  for(;;) {
    usleep(1000);
  }
  return 0;
}

void setup() {
  if ( ! access( NAMED_PIPE, W_OK ) ) {
    fprintf(stderr, "Missing named pipe at %s", NAMED_PIPE);
    exit(1);
  }
}

void motion_detected() { 
	fprintf(stderr,  " MOTION DETECTED! \n");
  int file ;
  char buffer[30];
  file = open(NAMED_PIPE, O_WRONLY );
  if (file > 0) {
    write(file, MOTION_DETECTED_STRING, strlen(MOTION_DETECTED_STRING));
    close(file);
  } else {
    fprintf(stderr, "Cannot write to file\n");
  }
}
