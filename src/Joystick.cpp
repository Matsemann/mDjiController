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

void Joystick::update(int l_hor, int l_ver, int r_hor, int r_ver, int lever_left, int lever_right, int camera)
{
	// Values must be between 0x0 and 0x8000
	// The values from the controller are between -1000 and 1000, so we just scale them between 0 and 16000 (maximum range in vJoyMonitor)	
	l_hor += 1000;
	l_ver += 1000;
	r_hor += 1000;
	r_ver += 1000;
	camera -= 1000;

	l_hor *= 16;
	l_ver *= 16;
	r_hor *= 16;
	r_ver *= 16;
	camera *= -16; // negative multiplication to make the value feel natural (left = negative | right = positive)

	// Read values from left lever
	// -780 = 'OFF' position | 0 = 'CL' position | 780 = 'HL' position
	if (lever_left == -780) {
		button_1 = true;
		button_2 = false;
		button_3 = false;
	}
	else if (lever_left == 780) {
		button_1 = false;
		button_2 = false;
		button_3 = true;
	} // else remain as before, sometimes this value is a bit weird
	else if (lever_left == 0) {
		button_1 = false;
		button_2 = true;
		button_3 = false;
	}

	// Read values from right lever
	// -780 = 'GPS' position | 0 = 'ATTI' position | 780 = 'ATTI' position
	if (lever_right == -780) {
		button_4 = true;
		button_5 = false;
		button_6 = false;
	}
	else if (lever_right == 780) {
		button_4 = false;
		button_5 = false;
		button_6 = true;
	} // else remain as before, sometimes this value is a bit weird
	else {
		button_4 = false;
		button_5 = true;
		button_6 = false;
	}

	if (log)
		printf("L vert: %-5d | L hori: %-5d | R vert: %-5d | R hori: %-5d | btn1: %-d | btn2: %-d | btn3: %-d | btn4: %-d | btn5: %-d | btn6: %-d | camera: %-d\n", l_ver, l_hor, r_ver, r_hor, button_1, button_2, button_3, button_4, button_5, button_6, camera);

	// Send stick values to vJoy
	SetAxis(l_hor, interfaceId, HID_USAGE_X);
	SetAxis(l_ver, interfaceId, HID_USAGE_Y);
	SetAxis(r_hor, interfaceId, HID_USAGE_Z);
	SetAxis(r_ver, interfaceId, HID_USAGE_RX);
	SetAxis(camera, interfaceId, HID_USAGE_RY);
	
	// Send button values to vJoy
	SetBtn(button_1, interfaceId, 1);
	SetBtn(button_2, interfaceId, 2);
	SetBtn(button_3, interfaceId, 3);
	SetBtn(button_4, interfaceId, 4);
	SetBtn(button_5, interfaceId, 5);
	SetBtn(button_6, interfaceId, 6);
}

Joystick::~Joystick()
{
	RelinquishVJD(interfaceId);
	printf("Disconnected from joystick\n");
}