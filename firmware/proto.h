#ifndef PROTO_H
#define PROTO_H

typedef enum {
  PULSE_CMD_GET = 1,
  PULSE_CMD_START
} PulseCommand;

typedef struct __attribute__ ((packed)) {
  uint16_t base;
  uint16_t upper;
  uint16_t lower;
} Message;


#endif // PROTO_H
