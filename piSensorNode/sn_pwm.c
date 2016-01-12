#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// device number 10
#define NAMED_PIPE "/tmp/SenderReceiverIO_device_10.named_pipe"
#define BUFFER_SIZE 100
#define PWM_PIN 1

static int named_pipe_fs;

void setup_pipe() 
{
  umask(0);
  if( access( NAMED_PIPE, F_OK ) == -1 ) {
    mkfifo(NAMED_PIPE, 0666);
  }
  named_pipe_fs = open( NAMED_PIPE, O_RDWR );
  if (named_pipe_fs < 0) {
    fprintf(stderr, "Error opening pipe...");
    exit(1);
  }
  return ;
}

void setup_dimmer() 
{ 
  wiringPiSetup();
  pinMode(PWM_PIN, PWM_OUTPUT);
  pwmSetClock(40);
  pwmSetMode(PWM_MODE_MS);
} 

void set_dimmer(int percentage) {
  // fprintf(stderr, "Percentage : %d\n", percentage);
  if (percentage > 95) {
   pwmWrite(PWM_PIN,0);
  } else if ( percentage < 10 ) {
   pwmWrite(PWM_PIN,1024);
  } else {
    pwmWrite(PWM_PIN, (1024 - percentage * 10) ) ;
  } 
}

int main(int argc, char*  argv[]) {
  char buffer[BUFFER_SIZE];
  int retval;
  setup_pipe();
  setup_dimmer();
  fprintf(stderr, "setup complete\n");  
  while (true) {
     retval = read(named_pipe_fs, &buffer, BUFFER_SIZE -1);
     if (retval <= 0) {
	      fprintf(stderr, "Error reading\n");
        exit(1);
     } else {
       set_dimmer(atoi(buffer));
       // fprintf(stderr, buffer);
       // fprintf(stderr, "\n\n");
     } 
  }
  
}

