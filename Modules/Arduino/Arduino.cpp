/* Copyright © 2024 Georgy E. All rights reserved. */

#include "Arduino.h"

#include "utils.h"
#include "gtime.h"


uint32_t millis()
{
	return getMillis();
}

float constrain(float x, float a, float b)
{
	if (a <= x && x <= b) {
		return x;
	}
	if (x < a) {
		return a;
	}
	return b;
}
