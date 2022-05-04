#include "gdexample_2d.h"

using namespace godot;

void GDExample2D::_register_methods() {
	register_method("_process", &GDExample2D::_process);
	register_property<GDExample2D, float>("amplitude", &GDExample2D::amplitude, 10.0f);
	register_property<GDExample2D, float>("speed", &GDExample2D::set_speed, &GDExample2D::get_speed, 1.0f);

	register_signal<GDExample2D, String, godot_variant_type, String, godot_variant_type>(String("position_changed"), String("node"), GODOT_VARIANT_TYPE_OBJECT, String("new_pos"), GODOT_VARIANT_TYPE_VECTOR2);
}

GDExample2D::GDExample2D() {
}

GDExample2D::~GDExample2D() {
	// add your cleanup here
}

void GDExample2D::_init() {
	// initialize any variables here
	time_passed = 0.0;
	amplitude = 10.0;
	speed = 1.0;
}

void GDExample2D::_process(float delta) {
	time_passed += speed * delta;

	Vector2 new_position = Vector2(
		amplitude + (amplitude * sin(time_passed * 2.0)),
		amplitude + (amplitude * cos(time_passed * 1.5))
	);

	set_position(new_position);

	time_emit += delta;
	if (time_emit > 1.0) {
		emit_signal("position_changed", this, new_position);

		time_emit = 0.0;
	}
}

void GDExample2D::set_speed(float p_speed) {
	speed = p_speed;
}

float GDExample2D::get_speed() {
	return speed;
}