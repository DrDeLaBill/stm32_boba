#ifndef VALUESSCREENPRESENTER_HPP
#define VALUESSCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class ValuesScreenView;

class ValuesScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    ValuesScreenPresenter(ValuesScreenView& v);

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

    virtual ~ValuesScreenPresenter() {}

private:
    ValuesScreenPresenter();

    ValuesScreenView& view;
};

#endif // VALUESSCREENPRESENTER_HPP
