#ifndef _SERIAL_STATUS_H_
#define _SERIAL_STATUS_H_

typedef enum {
  BUS_OK = 0,               /* Bus is ok */
  BUS_LOCKED = 1,           /* Bus is locked and being used by some device */
  BUS_ADDRESS_NACK = 2,     /* Could not find the device for given address */
  BUS_DATA_NACK = 3,        /* could not send/receive data */
  BUS_TIMEOUT = 4,          /* device timer timedout */
  BUS_INVALID = 5,          /* Bus invalid argument */
  BUS_NOT_OWNED = 6,        /* Bus is owned by another device */
  BUS_UNKNOWN_ERROR = 7
} serial_bus_status_t;

#define SERIAL_BUS_MAX_ERROR 7

#endif /* _SERIAL_STATUS_H_ */
