#ifndef GD_EXAMPLE_2D_H
#define GD_EXAMPLE_2D_H

#include <Godot.hpp>
#include <Sprite.hpp>
#include <String.hpp>
#include <Dictionary.hpp>

namespace godot {

class GDExample2D : public Sprite {
	GODOT_CLASS(GDExample2D, Sprite)

private:
	float time_passed;
	float time_emit;
	float amplitude;
	float speed;

public:
	static void _register_methods();

	GDExample2D();
	~GDExample2D();

	void _init(); // our initializer called by Godot

	void _process(float delta);
	void set_speed(float p_speed);
	float get_speed();
};

}

#endif
