#ifndef SENDER_TRANSMITTER_H
#define SENDER_TRANSMITTER_H

/*
 * Transmitter: uploads captured photos to the receiver over HTTP.
 */

/** Upload task: takes photo packets from the queue and POSTs them to the receiver */
void transmitTask(void* pvParameters);

#endif // SENDER_TRANSMITTER_H
