#pragma once
#include "app/common.hpp"
#include "app/ParamWidget.hpp"
#include "app.hpp"


namespace rack {


/** Implements vertical dragging behavior for ParamWidgets */
struct Knob : ParamWidget {
	/** Multiplier for mouse movement to adjust knob value */
	float speed = 1.0;

	void onButton(const event::Button &e) override;
	void onDragStart(const event::DragStart &e) override;
	void onDragEnd(const event::DragEnd &e) override;
	void onDragMove(const event::DragMove &e) override;
};


} // namespace rack
