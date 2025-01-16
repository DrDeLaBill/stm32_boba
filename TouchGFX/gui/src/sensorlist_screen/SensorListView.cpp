#include <gui/sensorlist_screen/SensorListView.hpp>

#ifndef SIMULATOR
#   include "gutils.h"
#endif


SensorListView::SensorListView()
{

}

void SensorListView::setupScreen()
{
    SensorListViewBase::setupScreen();
}

void SensorListView::tearDownScreen()
{
    SensorListViewBase::tearDownScreen();
}

void SensorListView::updateValues()
{
	static int8_t counter = 0;
	counter++;

	int width1 = angleValue.getTextWidth();
	int x1     = angleValue.getX();
	Unicode::snprintf(
		angleValueBuffer,
		ANGLEVALUE_SIZE - 1,
		"%d.%d",
		counter / 10,
#ifndef SIMULATOR
		__abs(counter % 10)
#else
		0
#endif
	);
	angleValue.resizeToCurrentText();
	int width2 = angleValue.getTextWidth();
	int x2     = x1 + width1 - width2;
	angleValue.setX(x2);
	if (backgroundAngle.isVisible()) {
		angleBack.setColor(backgroundAngle.getColor());
	} else {
		angleBack.setColor(background.getColor());
	}
	angleBack.invalidate();
	angleValue.invalidate();


	width1 = distanceValue.getTextWidth();
	x1     = distanceValue.getX();
	Unicode::snprintf(
		distanceValueBuffer,
		DISTANCEVALUE_SIZE - 1,
		"%d.%d",
		counter / 10,
#ifndef SIMULATOR
		__abs(counter % 10)
#else
		0
#endif
	);
	distanceValue.resizeToCurrentText();
	width2 = distanceValue.getTextWidth();
	x2     = x1 + width1 - width2;
	distanceValue.setX(x2);
	if (backgroundDistance.isVisible()) {
		distanceBack.setColor(backgroundDistance.getColor());
	} else {
		distanceBack.setColor(background.getColor());
	}
	distanceBack.invalidate();
	distanceValue.invalidate();
}
