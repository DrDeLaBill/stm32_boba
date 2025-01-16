#ifndef SENSORLISTPRESENTER_HPP
#define SENSORLISTPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class SensorListView;

class SensorListPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    SensorListPresenter(SensorListView& v);

    /**
     * The activate function is called automatically when this screen is "switched in"
     * (ie. made active). Initialization logic can be placed here.
     */
    virtual void activate();

    /**
     * The deactivate function is called automatically when this screen is "switched out"
     * (ie. made inactive). Teardown functionality can be placed here.
     */
    virtual void deactivate();

    virtual ~SensorListPresenter() {}

private:
    SensorListPresenter();

    SensorListView& view;
};

#endif // SENSORLISTPRESENTER_HPP
