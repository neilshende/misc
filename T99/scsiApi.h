#pragma once
#include <stdint.h>

int acquireReservation( char * dev, uint64_t key );
int changeReservation( char * dev, uint64_t key );
int checkReservation( char *dev, uint64_t &key );
int releaseReservation( char * dev, uint64_t key );
int resetReservation( char *dev, uint64_t key );

