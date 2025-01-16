#ifndef SENSORLISTVIEW_HPP
#define SENSORLISTVIEW_HPP

#include <gui_generated/sensorlist_screen/SensorListViewBase.hpp>
#include <gui/sensorlist_screen/SensorListPresenter.hpp>

class SensorListView : public SensorListViewBase
{
public:
    SensorListView();
    virtual ~SensorListView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
    virtual void updateValues();
};

#endif // SENSORLISTVIEW_HPP
