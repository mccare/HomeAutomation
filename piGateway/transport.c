
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
#define NAMED_PIPE_OUTPUT_FILE "/tmp/SenderReceiverIO_device_%d.named_pipe"
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

  if ( sscanf(buffer, "DEVICE_ID:%i:VALUE:%f", &(reading.device_id), &(reading.value)) != 2 ) {
    fprintf(stderr, "Cannot scan line with device and value %s\n", buffer );
    reading.device_id = 0;
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

void write_to_pipe(struct device_reading data) {
  char filename[BUFFER_SIZE];
  char output[BUFFER_SIZE];
  int chars_written = 0;
  int write_pipe_fs;

  snprintf(filename, BUFFER_SIZE, NAMED_PIPE_OUTPUT_FILE, data.device_id);

  if( access( filename, F_OK ) == -1 ) {
    fprintf(stderr, "Will not write: no such file %s", filename);
    return;
  }

  write_pipe_fs = open( filename, O_WRONLY | O_NONBLOCK );
  if (write_pipe_fs < 0) {
    fprintf(stderr, "Error opening pipe for writing... %s", filename);
    return;
  }
  snprintf(output, BUFFER_SIZE, "%f", data.value);

  chars_written = write(write_pipe_fs, output, strlen(output));
  if (chars_written <= 0) {
    fprintf(stderr, "Error writing to pipe ");
  }
  close(write_pipe_fs);
}
