#include "gdexample.h"

using namespace godot;

TSS_ComPort* GDExample::port = nullptr;
tss_device_id GDExample::dongle_id = tss_device_id();
int GDExample::actives = 0;
bool GDExample::streaming = false;

void GDExample::_register_methods() {
	register_method("_process", &GDExample::_process);
	register_method("_enter_tree", &GDExample::_enter_tree);
	register_method("_exit_tree", &GDExample::_exit_tree);
	register_property<GDExample, bool>("active", &GDExample::set_active, &GDExample::get_active, true);
	register_property<GDExample, int>("device", &GDExample::set_device, &GDExample::get_device, 0);
	register_property<GDExample, int>("name", &GDExample::set_name, &GDExample::get_name, 0);
	register_property<GDExample, Quat>("orientation", &GDExample::set_orientation, &GDExample::get_orientation, Quat());
}

GDExample::GDExample() {
}

GDExample::~GDExample() {
	// add your cleanup here
}

void GDExample::_init() {
	// initialize any variables here
  Godot::print("ready");
	device = 0;
	name = 0;
	active = false;
  orientation = Quat();
}

void GDExample::_enter_tree() {
	// reinitialize any variables here
  Godot::print("enter_tree");
  active = false;
  set_active(true);
}

void GDExample::_exit_tree() {
	// deinitialize any variables here
  Godot::print("exit_tree");
  set_active(false);
}

void GDExample::_process(float delta) {
  //Godot::print("process");
  if ((streaming) && (active) && (port != nullptr) && (device != 0)) {
    TSS_Stream_Packet packet;
    TSS_ERROR error = tss_sensor_getLastStreamingPacket(sensor_id, &packet);
    if (error == TSS_NO_ERROR) {
      orientation = Quat(packet.taredOrientQuat[0], packet.taredOrientQuat[1], packet.taredOrientQuat[2], packet.taredOrientQuat[3]);
      //Godot::print(orientation);
    }
  }
}

void GDExample::set_active(bool p_active) {
  if ((active) && (!p_active)) {
    if ((--actives <= 0) && (port != nullptr)) {
      Godot::print("Destroying the API!");
      streaming = false;
      tss_dongle_stopStreaming(dongle_id);
      tss_removeDongle(dongle_id);
      tss_deinitAPI();
      delete[] port->port_name;
      delete port;
      port = nullptr;
    }
  }
  if ((!active) && (p_active)) {
    U32 timestamp = 0;
    TSS_ERROR error = TSS_NO_ERROR;
    ++actives;
    if (port == nullptr) {
      Godot::print("Initializing the API!");
      port = new TSS_ComPort();
      port->port_name = new char[64];
      error = tss_initAPI();
      if (error) {
        Godot::print(std::string(std::string("(")+std::string(tss_error_string[error])+") Could not initialize the API!").c_str());
        p_active = false;
        delete[] port->port_name;
        delete port;
        port = nullptr;
        --actives;
      }
      if (port != nullptr) {
        Godot::print("Creating a Three Space Dongle from Search!");
        tss_findSensorPorts(TSS_DONGLE);
        error = tss_getNextSensorPort(port->port_name, &port->device_type, &port->connection_type);
        if (error == TSS_NO_ERROR) {
          error = tss_createDongle(port->port_name, &dongle_id);
          if (error) {
            Godot::print(std::string(std::string("(")+std::string(tss_error_string[error])+std::string(") Failed to create TSS Dongle!")).c_str());
            tss_deinitAPI();
            p_active = false;
            delete[] port->port_name;
            delete port;
            port = nullptr;
            --actives;
          } else {
            Godot::print("Successfully created a Three Space Dongle!");
            //tss_dongle_setWirelessRetries(dongle_id, 0, &timestamp);
          }
        } else {
          Godot::print(std::string(std::string("(")+std::string(tss_error_string[error])+") Failed to get the port!").c_str());
          tss_deinitAPI();
          p_active = false;
          delete[] port->port_name;
          delete port;
          port = nullptr;
          --actives;
        }
      }
    }
    if ((error == TSS_NO_ERROR) && (port != nullptr) && (device != 0)) {
      std::stringstream stream;
      stream << std::hex << device;
      std::string hex = stream.str();
      if (streaming) {
        Godot::print("Temporarily stopping the stream!");
        streaming = false;
        error = tss_dongle_stopStreaming(dongle_id);
        if (error) {
          Godot::print(std::string(std::string("(")+std::string(tss_error_string[error])+std::string(") Unexpected error occurred while temporarily stopping the stream on ")+hex+std::string("!")).c_str());
          p_active = false;
          --actives;
        }
      }
      if (error == TSS_NO_ERROR) {
        Godot::print("Creating a Three Space Wireless Sensor!");
        error = tss_dongle_setSerialNumberAtLogicalID(dongle_id, static_cast<U8>(name), *(reinterpret_cast<U32*>(&device)), &timestamp);
      }
      if (error == TSS_NO_ERROR) {
        error = tss_dongle_getWirelessSensor(dongle_id, static_cast<U8>(name), &sensor_id);
        if (error) {
          Godot::print(std::string(std::string("(")+std::string(tss_error_string[error])+std::string(") Failed to create TSS Sensor on ")+hex+std::string("!")).c_str());
          p_active = false;
          --actives;
        } else {
          Godot::print("Successfully created a Three Space Wireless Sensor!");
          error = tss_dongle_enableAllSensorsAndStartStreaming(dongle_id, TSS_STREAM_TARED_ORIENTATION_AS_QUATERNION, 1000, TSS_STREAM_DURATION_INFINITE);
          if (error) {
            Godot::print(std::string(std::string("(")+std::string(tss_error_string[error])+std::string(") Streaming error on ")+hex+std::string("!")).c_str());
            p_active = false;
            --actives;
          } else {
            Godot::print("Successfully streaming!");
            streaming = true;
          }
        }
      } else {
        Godot::print(std::string(std::string("(")+std::string(tss_error_string[error])+std::string(") Could not assign device name at ")+hex+std::string("!")).c_str());
        p_active = false;
        --actives;
      }
    }
  }
	active = p_active;
}

bool GDExample::get_active() {
	return active;
}

void GDExample::set_device(int p_device) {
  device = p_device;
}

int GDExample::get_device() {
	return device;
}

void GDExample::set_name(int p_name) {
  name = p_name;
}

int GDExample::get_name() {
	return name;
}

void GDExample::set_orientation(Quat p_orientation) {
	orientation = p_orientation;
}

Quat GDExample::get_orientation() {
	return orientation;
}

