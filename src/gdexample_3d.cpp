#include "gdexample_3d.h"

using namespace godot;

void GDExample3D::_register_methods() {
	register_method("_process", &GDExample3D::_process);
	register_property<GDExample3D, float>("amplitude", &GDExample3D::amplitude, 10.0f);
	register_property<GDExample3D, float>("speed", &GDExample3D::set_speed, &GDExample3D::get_speed, 1.0f);

	register_signal<GDExample3D, String, godot_variant_type, String, godot_variant_type>(String("position_changed"), String("node"), GODOT_VARIANT_TYPE_OBJECT, String("new_pos"), GODOT_VARIANT_TYPE_VECTOR3);
}

GDExample3D::GDExample3D() {
}

GDExample3D::~GDExample3D() {
	// add your cleanup here
}

void GDExample3D::_init() {
	// initialize any variables here
	time_passed = 0.0;
	amplitude = 1.0;
	speed = 1.0;
}

void GDExample3D::_process(float delta) {
	time_passed += speed * delta;

	Vector3 new_position = Vector3(
		amplitude + (amplitude * sin(time_passed * 2.0)),
    0.0,
		amplitude + (amplitude * cos(time_passed * 1.5))
	);

	set_translation(new_position);

	time_emit += delta;
	if (time_emit > 1.0) {
		emit_signal("position_changed", this, new_position);

		time_emit = 0.0;
	}
}

void GDExample3D::set_speed(float p_speed) {
	speed = p_speed;
}

float GDExample3D::get_speed() {
	return speed;
}
