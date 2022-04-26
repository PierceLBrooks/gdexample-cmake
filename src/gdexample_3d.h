#ifndef GD_EXAMPLE_3D_H
#define GD_EXAMPLE_3D_H

#include <Godot.hpp>
#include <Sprite3D.hpp>
#include <String.hpp>
#include <Dictionary.hpp>

namespace godot {

class GDExample3D : public Sprite3D {
	GODOT_CLASS(GDExample3D, Sprite3D)

private:
	float time_passed;
	float time_emit;
	float amplitude;
	float speed;

public:
	static void _register_methods();

	GDExample3D();
	~GDExample3D();

	void _init(); // our initializer called by Godot

	void _process(float delta);
	void set_speed(float p_speed);
	float get_speed();
};

}

#endif
