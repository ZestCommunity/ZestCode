#include "ifi/v5_api.h"
#include "kapi.h"
#include "vdml/vdml.h"

double battery_get_voltage() {
	if (!internal_port_mutex_take(V5_PORT_BATTERY)) {
		errno = EACCES;
		return PROS_ERR_F;
	}
	double rtn = vexBatteryVoltageGet();
	internal_port_mutex_give(V5_PORT_BATTERY);
	return rtn;
}

double battery_get_current() {
	if (!internal_port_mutex_take(V5_PORT_BATTERY)) {
		errno = EACCES;
		return PROS_ERR_F;
	}
	double rtn = vexBatteryCurrentGet();
	internal_port_mutex_give(V5_PORT_BATTERY);
	return rtn;
}

double battery_get_temperature() {
	if (!internal_port_mutex_take(V5_PORT_BATTERY)) {
		errno = EACCES;
		return PROS_ERR_F;
	}
	double rtn = vexBatteryTemperatureGet();
	internal_port_mutex_give(V5_PORT_BATTERY);
	return rtn;
}

double battery_get_capacity() {
	if (!internal_port_mutex_take(V5_PORT_BATTERY)) {
		errno = EACCES;
		return PROS_ERR_F;
	}
	double rtn = vexBatteryCapacityGet();
	internal_port_mutex_give(V5_PORT_BATTERY);
	return rtn;
}