/* Copyright © 2023 Georgy E. All rights reserved. */

#pragma once


#include <stdint.h>

#include "Timer.h"
#include "StorageAT.h"


#define STORAGE_DRIVER_BEDUG      (false)
#define STORAGE_DRIVER_USE_BUFFER (true)


struct StorageDriver: public IStorageDriver
{
private:
	static constexpr char TAG[] = "DRVR";

	static bool hasError;
	static utl::Timer timer;

#if STORAGE_DRIVER_USE_BUFFER
    static bool     hasBuffer;
    static uint8_t  bufferPage[STORAGE_PAGE_SIZE];
    static uint32_t lastAddress;
#endif

public:
    StorageStatus read(uint32_t address, uint8_t *data, uint32_t len) override;
    StorageStatus write(uint32_t address, uint8_t *data, uint32_t len) override;
};
