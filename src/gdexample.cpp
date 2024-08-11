#include "gdexample.h"

using namespace godot;

TSS_ComPort* GDExample::port = nullptr;
tss_device_id GDExample::dongle_id = tss_device_id();
int GDExample::actives = 0;
bool GDExample::streaming = false;

void GDExample::_register_methods() {
  ClassDB::bind_method(D_METHOD("_process"), &GDExample::_process);
  ClassDB::bind_method(D_METHOD("_enter_tree"), &GDExample::_enter_tree);
  ClassDB::bind_method(D_METHOD("_exit_tree"), &GDExample::_exit_tree);
  ClassDB::bind_method(D_METHOD("set_active"), &GDExample::set_active);
  ClassDB::bind_method(D_METHOD("get_active"), &GDExample::get_active);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "active"), "set_active", "get_active");
  ClassDB::bind_method(D_METHOD("set_device"), &GDExample::set_device);
  ClassDB::bind_method(D_METHOD("get_device"), &GDExample::get_device);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "device"), "set_device", "get_device");
  ClassDB::bind_method(D_METHOD("set_name"), &GDExample::set_name);
  ClassDB::bind_method(D_METHOD("get_name"), &GDExample::get_name);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "name"), "set_name", "get_name");
  ClassDB::bind_method(D_METHOD("set_orientation"), &GDExample::set_orientation);
  ClassDB::bind_method(D_METHOD("get_orientation"), &GDExample::get_orientation);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "orientation"), "set_orientation", "get_orientation");
}

GDExample::GDExample() {
}

GDExample::~GDExample() {
	// add your cleanup here
}

void GDExample::_init() {
	// initialize any variables here
  UtilityFunctions::print("ready");
	device = 0;
	name = 0;
	active = false;
  orientation = Quaternion();
}

void GDExample::_enter_tree() {
	// reinitialize any variables here
  UtilityFunctions::print("enter_tree");
  active = false;
  set_active(true);
}

void GDExample::_exit_tree() {
	// deinitialize any variables here
  UtilityFunctions::print("exit_tree");
  set_active(false);
}

void GDExample::_process(float delta) {
  //UtilityFunctions::print("process");
  if ((streaming) && (active) && (port != nullptr) && (device != 0)) {
    TSS_Stream_Packet packet;
    TSS_ERROR error = tss_sensor_getLastStreamingPacket(sensor_id, &packet);
    if (error == TSS_NO_ERROR) {
      orientation = Quaternion(packet.taredOrientQuat[0], packet.taredOrientQuat[1], packet.taredOrientQuat[2], packet.taredOrientQuat[3]);
      //UtilityFunctions::print(orientation);
    }
  }
}

void GDExample::set_active(bool p_active) {
  if ((active) && (!p_active)) {
    if ((--actives <= 0) && (port != nullptr)) {
      UtilityFunctions::print("Destroying the API!");
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
      UtilityFunctions::print("Initializing the API!");
      port = new TSS_ComPort();
      port->port_name = new char[64];
      error = tss_initAPI();
      if (error) {
        UtilityFunctions::print(std::string(std::string("(")+std::string(tss_error_string[error])+") Could not initialize the API!").c_str());
        p_active = false;
        delete[] port->port_name;
        delete port;
        port = nullptr;
        --actives;
      }
      if (port != nullptr) {
        UtilityFunctions::print("Creating a Three Space Dongle from Search!");
        tss_findSensorPorts(TSS_DONGLE);
        error = tss_getNextSensorPort(port->port_name, &port->device_type, &port->connection_type);
        if (error == TSS_NO_ERROR) {
          error = tss_createDongle(port->port_name, &dongle_id);
          if (error) {
            UtilityFunctions::print(std::string(std::string("(")+std::string(tss_error_string[error])+std::string(") Failed to create TSS Dongle!")).c_str());
            tss_deinitAPI();
            p_active = false;
            delete[] port->port_name;
            delete port;
            port = nullptr;
            --actives;
          } else {
            UtilityFunctions::print("Successfully created a Three Space Dongle!");
            //tss_dongle_setWirelessRetries(dongle_id, 0, &timestamp);
          }
        } else {
          UtilityFunctions::print(std::string(std::string("(")+std::string(tss_error_string[error])+") Failed to get the port!").c_str());
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
        UtilityFunctions::print("Temporarily stopping the stream!");
        streaming = false;
        error = tss_dongle_stopStreaming(dongle_id);
        if (error) {
          UtilityFunctions::print(std::string(std::string("(")+std::string(tss_error_string[error])+std::string(") Unexpected error occurred while temporarily stopping the stream on ")+hex+std::string("!")).c_str());
          p_active = false;
          --actives;
        }
      }
      if (error == TSS_NO_ERROR) {
        UtilityFunctions::print("Creating a Three Space Wireless Sensor!");
        error = tss_dongle_setSerialNumberAtLogicalID(dongle_id, static_cast<U8>(name), *(reinterpret_cast<U32*>(&device)), &timestamp);
      }
      if (error == TSS_NO_ERROR) {
        error = tss_dongle_getWirelessSensor(dongle_id, static_cast<U8>(name), &sensor_id);
        if (error) {
          UtilityFunctions::print(std::string(std::string("(")+std::string(tss_error_string[error])+std::string(") Failed to create TSS Sensor on ")+hex+std::string("!")).c_str());
          p_active = false;
          --actives;
        } else {
          UtilityFunctions::print("Successfully created a Three Space Wireless Sensor!");
          error = tss_dongle_enableAllSensorsAndStartStreaming(dongle_id, TSS_STREAM_TARED_ORIENTATION_AS_QUATERNION, 1000, TSS_STREAM_DURATION_INFINITE);
          if (error) {
            UtilityFunctions::print(std::string(std::string("(")+std::string(tss_error_string[error])+std::string(") Streaming error on ")+hex+std::string("!")).c_str());
            p_active = false;
            --actives;
          } else {
            UtilityFunctions::print("Successfully streaming!");
            streaming = true;
          }
        }
      } else {
        UtilityFunctions::print(std::string(std::string("(")+std::string(tss_error_string[error])+std::string(") Could not assign device name at ")+hex+std::string("!")).c_str());
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

void GDExample::set_orientation(Quaternion p_orientation) {
	orientation = p_orientation;
}

Quaternion GDExample::get_orientation() {
	return orientation;
}

