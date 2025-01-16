#ifndef LOADSCREENVIEW_HPP
#define LOADSCREENVIEW_HPP

#include <gui_generated/loadscreen_screen/LoadScreenViewBase.hpp>
#include <gui/loadscreen_screen/LoadScreenPresenter.hpp>

class LoadScreenView : public LoadScreenViewBase
{
public:
    LoadScreenView();
    virtual ~LoadScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
    virtual void updateLoad();
    virtual void checkLoad();
};

#endif // LOADSCREENVIEW_HPP
