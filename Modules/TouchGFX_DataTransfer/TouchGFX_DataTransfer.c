/* Functions called by the TouchGFX HAL to invoke the actual data transfer to ILI9341.
 * Pero, 2021
 */
#include "TouchGFX_DataTransfer.h"

#include "glog.h"
#include "st7796.h"

extern void DisplayDriver_TransferCompleteCallback();

static uint8_t isTransmittingData = 0;

uint32_t touchgfxDisplayDriverTransmitActive(void)
{
	return isTransmittingData;
}

void touchgfxDisplayDriverTransmitBlock(uint8_t* pixels, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	isTransmittingData = 1;
	ST7796_SetWindow(x, y, x+w-1, y+h-1);
	ST7796_DrawBitmap(w, h, pixels);
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if (hspi->Instance == DISPLAY_SPI.Instance) {
		ST7796_EndOfDrawBitmap();
		isTransmittingData = 0;
		DisplayDriver_TransferCompleteCallback();
	}
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
	if (hspi->Instance == DISPLAY_SPI.Instance) {
		printTagLog("TGFX", "HAL_SPI_ErrorCallback");
	}
}

void HAL_SPI_AbortCpltCallback(SPI_HandleTypeDef *hspi)
{
	if (hspi->Instance == DISPLAY_SPI.Instance) {
		printTagLog("TGFX", "HAL_SPI_AbortCpltCallback");
	}
}
