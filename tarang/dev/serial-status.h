#ifndef _SERIAL_STATUS_H_
#define _SERIAL_STATUS_H_

typedef enum {
  BUS_OK = 0,               /* Bus is ok */
  BUS_LOCKED = 1,           /* Bus is locked and being used by some device */
  BUS_ADDRESS_NACK = 2,     /* Could not find the device for given address */
  BUS_DATA_NACK = 3,        /* could not send/receive data */
  BUS_TIMEOUT = 4,          /* device timer timedout */
  BUS_INVALID = 5,          /* Bus hardware is in invalid state */
  BUS_BUSY = 6,             /* Bus cannot be accessed or it is locked */
  BUS_UNKNOWN_ERROR = 7
} serial_bus_status_t;
#endif /* _SERIAL_STATUS_H_ */
