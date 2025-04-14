#pragma once

#include "DeviceContext.h"

class Device {
private:
	virtual void Function0();
	virtual void Function1();
	virtual void Function2();
	virtual void Function3();
	virtual void Function4();
public:
	virtual DeviceContext* beginFrame();
};