
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <fcntl.h>           /* Definition of AT_* constants */
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include "transport.h"

/*
 * This part will create a named pipe in the /tmp/ directory 
 * call setup_pipe() to create the named pipe
 * call read_from_pipe(microseconds) to read data from pipe, the pipe is configured to be non blocking
 * return value of device_id of 0 means an error or data is not ready
 * 
 * with a script or program in your prefered programming language you can send just write to the named pipe:
 * DEVICE_ID:<int as device_id>:VALUE:<float>
 * 
 * The SenderReceiver should then create a message and send it out via the RFM69 bus
*/

#define NAMED_PIPE "/tmp/SenderReceiverIO.named_pipe"
#define BUFFER_SIZE 100
 
static int named_pipe_fs = 0;
void setup_pipe() 
{
  umask(0);
  if( access( NAMED_PIPE, F_OK ) == -1 ) {
    mkfifo(NAMED_PIPE, 0666);
  }
  named_pipe_fs = open( NAMED_PIPE, O_RDWR | O_NONBLOCK );
  if (named_pipe_fs < 0) {
    fprintf(stderr, "Error opening pipe...");
    exit(1);
  }
  return ;
}


// will parse DEVICE:%i:VALUE:%f into a struct
static struct device_reading process_buffer(char* buffer) {
  struct device_reading reading;
  char *input[4];
  int loop;

  reading.device_id = 0;
  
  // read the string into an array 
  input[0] = strtok(buffer, ":");
  for (loop = 1; loop < 4; loop ++) {
    input[loop] = strtok(NULL, ":");
  }
  if (strcmp(input[0], "DEVICE_ID") != 0) {
    fprintf(stderr, "Invalid first token in buffer %s\n", buffer );
    return reading;
  }
  if (strcmp(input[2], "VALUE") != 0) {
    fprintf(stderr, "Invalid third token in buffer %s\n", buffer );
    return reading;
  }
  if (sscanf(input[3], "%f", &(reading.value)) != 1) {
    fprintf(stderr, "Invalid fourth token in buffer %s\n", buffer );
    return reading;
  } 
  if (sscanf(input[1], "%i", &(reading.device_id)) != 1) {
    fprintf(stderr, "Invalid second token in buffer %s\n", buffer );
    return reading;
  } 
  
  return reading;
}


/*
 * Non blocking reading from the pipe, will return device_id 0 if no data is ready or an error occured
*/
struct device_reading read_from_pipe(long microseconds) 
{
  struct timeval timeout;
  struct device_reading reading;
  fd_set rfds;
  int retval;
  char buffer[BUFFER_SIZE];
  
  timeout.tv_sec = 0;
  timeout.tv_usec = microseconds; 
  
  reading.device_id = 0; 
    
  FD_ZERO(&rfds);
  FD_SET(named_pipe_fs, &rfds);
  
  retval = select(named_pipe_fs + 1, &rfds, NULL, NULL, &timeout);
  
  if (retval == -1) {
    fprintf(stderr, "Error on select of named pipe");
  } else if (retval) {
    retval = read(named_pipe_fs, &buffer, BUFFER_SIZE - 1);
    if (retval == 0 ) {
      fprintf(stderr, "Nothing to read, this should not happen...");
    } else {
      buffer[retval] = 0;
      reading = process_buffer(buffer);
      fprintf(stderr, "Found device %i and value %f\n", reading.device_id, reading.value);
      return reading;
    }
  } 
  return reading;
}
