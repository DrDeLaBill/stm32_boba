/* Copyright © 2024 Georgy E. All rights reserved. */

#include "Arduino.h"

#include "utils.h"
#include "gtime.h"


uint32_t millis()
{
	return getMillis();
}

int32_t constrain(int32_t x, int32_t a, int32_t b)
{
	if (a <= x && x <= b) {
		return x;
	}
	if (x < a) {
		return a;
	}
	return b;
}
