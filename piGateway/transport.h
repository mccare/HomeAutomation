
/*
 * read device data from a named pipe
 */
struct device_reading {
  float value;
  uint8_t device_id;
};

void setup_pipe();
struct device_reading read_from_pipe(long microseconds) ;
void write_to_pipe(struct device_reading data) ;
