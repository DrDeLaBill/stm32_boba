#ifndef VALUESSCREENVIEW_HPP
#define VALUESSCREENVIEW_HPP

#include <gui_generated/valuesscreen_screen/ValuesScreenViewBase.hpp>
#include <gui/valuesscreen_screen/ValuesScreenPresenter.hpp>

class ValuesScreenView : public ValuesScreenViewBase
{
public:
    ValuesScreenView();
    virtual ~ValuesScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
    void changeMode();

    virtual void clickLeft();
    virtual void clickRight();

    virtual void updateSensorData();
};

#endif // VALUESSCREENVIEW_HPP
