#include <gui/loadscreen_screen/LoadScreenView.hpp>

#ifndef SIMULATOR
#   include "gsystem.h"
#   include "Timer.h"
#endif

#ifndef SIMULATOR
utl::Timer timer(5000);
#endif
LoadScreenView::LoadScreenView()
{
#ifndef SIMULATOR
	timer.start();
#endif
}

void LoadScreenView::setupScreen()
{
    LoadScreenViewBase::setupScreen();
}

void LoadScreenView::tearDownScreen()
{
    LoadScreenViewBase::tearDownScreen();
}

void LoadScreenView::updateLoad()
{
	static float zAngle = 0;

	if (!textureLoad.getAlpha()) {
		textureLoad.setVisible(true);
		textName.setVisible(false);
		textName.invalidate();
	}
	textureLoad.setAngles(0, 0, zAngle);
	textureLoad.invalidate();

	zAngle += 0.05f;
}

void LoadScreenView::checkLoad()
{
#ifndef SIMULATOR
	if (timer.wait()) {
		return;
	}
	if (is_system_ready()) {
		changeToMainScreen();
	}
#else
	changeToMainScreen();
#endif
}
