#include "Joystick.h"
#include "vjoyinterface.h"

Joystick::Joystick(int id, int logging)
{
	interfaceId = id;
	if (logging == 0) {
		log = true;
	}

	printf("Creating vJoy connection\n");

	if (!vJoyEnabled()) {
		printf("vJoy driver not enabled: Failed Getting vJoy attributes.\n");
		return;
	} else {
		printf("Driver found:\nVendor:%S, Product:%S, Ver:%S\n", TEXT(GetvJoyManufacturerString()), TEXT(GetvJoyProductString()), TEXT(GetvJoySerialNumberString()));
	};

	VjdStat status = GetVJDStatus(interfaceId);
	switch (status)
	{
	case VJD_STAT_OWN:
		printf("vJoy Device %d is already owned by this feeder\n", interfaceId);
		break;
	case VJD_STAT_FREE:
		printf("vJoy Device %d is free\n", interfaceId);
		break;
	case VJD_STAT_BUSY:
		printf("vJoy Device %d is already owned by another feeder\nCannot continue\n", interfaceId);
		return;
	case VJD_STAT_MISS:
		printf("vJoy Device %d is not installed or disabled\nCannot continue\n", interfaceId);
		return;
	default:
		printf("vJoy Device %d general error\nCannot continue\n", interfaceId);
		return;
	};

	if (!AcquireVJD(interfaceId)) {
		printf("Failed to acquire vJoy Interface number %d\n", interfaceId);
		return;
	}
	else {
		ResetVJD(interfaceId);
		connected = true;
		printf("Acquired vJoy Interface number %d\n", interfaceId);
	}
}

void Joystick::update(int l_hor, int l_ver, int r_hor, int r_ver, int btn)
{
	// Values must be between 0x0 and 0x8000
	// The values from the controller are between -1000 and 1000,
	// so we just make them be 0 to 2000, and then calibrate the controller in the simulator
	l_hor += 1000;
	l_ver += 1000;
	r_hor += 1000;
	r_ver += 1000;

	if (btn == 780) {
		button = true;
	}
	else if (btn == -780) {
		button = false;
	} // else remain as before, sometimes this value is a bit weird

	if (log)
		printf("L vert: %-5d | L hori: %-5d | R vert: %-5d | R hori: %-5d | btn: %-d\n", l_ver, l_hor, r_ver, r_hor, button);

	SetAxis(l_hor, interfaceId, HID_USAGE_X);
	SetAxis(l_ver, interfaceId, HID_USAGE_Y);
	SetAxis(r_hor, interfaceId, HID_USAGE_Z);
	SetAxis(r_ver, interfaceId, HID_USAGE_RX);
	
	SetBtn(button, interfaceId, 1);
}

Joystick::~Joystick()
{
	RelinquishVJD(interfaceId);
	printf("Disconnected from joystick\n");
}