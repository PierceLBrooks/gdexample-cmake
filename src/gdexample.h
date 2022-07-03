#ifndef GD_EXAMPLE_H
#define GD_EXAMPLE_H

#include <string>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <Godot.hpp>
#include <Node.hpp>
#include <String.hpp>
#include <Dictionary.hpp>
#include <Quat.hpp>
#include "threespace_api_export.h"

namespace godot {

class GDExample : public Node {
	GODOT_CLASS(GDExample, Node)

private:
  Quat orientation;
  bool active;
  int device;
  int name;
  tss_device_id sensor_id;
  static bool streaming;
  static int actives;
  static TSS_ComPort* port;
  static tss_device_id dongle_id;

public:
	static void _register_methods();

	GDExample();
	~GDExample();

	void _init(); // our initializer called by Godot
	void _enter_tree(); // our reinitializer called by Godot
	void _exit_tree(); // our deinitializer called by Godot

	void _process(float delta);
	void set_active(bool p_active);
	bool get_active();
	void set_device(int p_device);
	int get_device();
	void set_name(int p_name);
	int get_name();
	void set_orientation(Quat p_orientation);
	Quat get_orientation();
};

}

#endif
