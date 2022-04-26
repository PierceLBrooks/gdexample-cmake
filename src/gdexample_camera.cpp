#include "gdexample_camera.h"
#include <Input.hpp>
#include <Math.hpp>
#include <Defs.hpp>
#include <Viewport.hpp>
#include <InputEventMouseButton.hpp>
#include <InputEventMouseMotion.hpp>
#include <cmath>

using namespace godot;

void GDExampleCamera::_register_methods() {
	register_method("_process", &GDExampleCamera::_process);
	register_method("_input", &GDExampleCamera::_input);
	register_property<GDExampleCamera, float>("speed", &GDExampleCamera::set_speed, &GDExampleCamera::get_speed, 1.0f);
}

GDExampleCamera::GDExampleCamera() {
}

GDExampleCamera::~GDExampleCamera() {
	// add your cleanup here
}

void GDExampleCamera::_init() {
	// initialize any variables here
  direction = Vector2(0.0, 0.0);
  speed = 0.01;
  mode = false;
}

void GDExampleCamera::_process(float delta) {
  /*if (!mode) {
    return;
  }
  Input* input = Input::get_singleton();
  Viewport* viewport = get_viewport();
  input->warp_mouse_position(viewport->get_size() * 0.5);*/
}

void GDExampleCamera::_input(Variant event) {
  Ref<InputEventMouseButton> button = event;
  if ((button.is_valid()) && (button->is_doubleclick())) {
    Input* input = Input::get_singleton();
    mode = !mode;
    if (mode) {
      input->set_mouse_mode(Input::MouseMode::MOUSE_MODE_CAPTURED);
    } else {
      input->set_mouse_mode(Input::MouseMode::MOUSE_MODE_VISIBLE);
    }
    return;
  }
  Ref<InputEventMouseMotion> motion = event;
  if (motion.is_valid()) {
    if (mode) {
      Vector3 normal = Vector3(std::cos(direction[0] * (Math_PI / 180.0)), 0.0, std::sin(direction[0] * (Math_PI / 180.0)));
      Vector2 relative = motion->get_relative() * speed;
      float vertical = -relative[1];
      rotate_y(-direction[0] * (Math_PI / 180.0));
      direction[0] -= relative[0];
      if ((direction[1] + vertical > -50.0) && (direction[1] + vertical < 50.0)) {
        direction[1] += vertical;
        rotate_x(vertical * (Math_PI / 180.0));
      }
      rotate_y(direction[0] * (Math_PI / 180.0));
    }
    return;
  }
}

void GDExampleCamera::set_speed(float p_speed) {
	speed = p_speed;
}

float GDExampleCamera::get_speed() {
	return speed;
}
