#ifndef GD_EXAMPLE_CAMERA_H
#define GD_EXAMPLE_CAMERA_H

#include <Godot.hpp>
#include <Camera.hpp>
#include <String.hpp>
#include <Quat.hpp>
#include <Vector2.hpp>
#include <Vector3.hpp>
#include <Variant.hpp>
#include <Dictionary.hpp>

namespace godot {

class GDExampleCamera : public Camera {
	GODOT_CLASS(GDExampleCamera, Camera)

private:
  Vector3 position;
  Vector2 direction;
  float speed;
  bool mode;

public:
	static void _register_methods();

	GDExampleCamera();
	~GDExampleCamera();

	void _init(); // our initializer called by Godot

	void _process(float delta);
	void _input(Variant event);
  
  void set_speed(float p_speed);
  float get_speed();
};

}

#endif
