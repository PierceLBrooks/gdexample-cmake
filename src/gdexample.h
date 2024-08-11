#ifndef GD_EXAMPLE_H
#define GD_EXAMPLE_H

#include <string>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <gdextension_interface.h>
#include <godot.hpp>
#include <node.hpp>
#include <string.hpp>
#include <dictionary.hpp>
#include <quaternion.hpp>
#include <class_db.hpp>
#include <os.hpp>
#include <utility_functions.hpp>
#include <global_constants.hpp>
#include "threespace_api_export.h"

namespace godot {

class GDExample : public Node {
	GDCLASS(GDExample, Node)

private:
  Quaternion orientation;
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
	void set_orientation(Quaternion p_orientation);
	Quaternion get_orientation();
};

}

#endif
