#ifndef SETTINGSSCREENVIEW_HPP
#define SETTINGSSCREENVIEW_HPP

#include <gui_generated/settingsscreen_screen/SettingsScreenViewBase.hpp>
#include <gui/settingsscreen_screen/SettingsScreenPresenter.hpp>

class SettingsScreenView : public SettingsScreenViewBase
{
public:
    SettingsScreenView();
    virtual ~SettingsScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
    unsigned settingsPage;

    unsigned sensitivity_idx;
    unsigned regulation_mm;

    virtual void begin();
    virtual void update();
    virtual void goBackAndSave();
};

#endif // SETTINGSSCREENVIEW_HPP
