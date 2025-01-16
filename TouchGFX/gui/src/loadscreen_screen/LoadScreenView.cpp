#include <gui/loadscreen_screen/LoadScreenView.hpp>

#ifndef SIMULATOR
#   include "gsystem.h"
#   include "Timer.h"
#endif

#ifndef SIMULATOR
utl::Timer timer(2000);
#endif
#ifndef SIMULATOR
LoadScreenView::LoadScreenView(): timer(2000)
#else
LoadScreenView::LoadScreenView()
#endif
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

	if (!textureLoad.isVisible()) {
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
#ifdef SIMULATOR
	changeToMainScreen();
#else
	if (timer.wait()) {
		return;
	}

	if (is_system_ready()) {
		changeToMainScreen();
	}
#endif
}
