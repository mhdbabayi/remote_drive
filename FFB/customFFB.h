#pragma once

#define DIRECTINPUT_VERSION 0x0800
#include <windows.h>
#include <vector>
#include <string>
#include <map>

#include <dinput.h>
#include <math.h>
#include <InitGuid.h>







extern "C"
{
	struct DeviceInfo {
		DWORD deviceType;
		LPSTR guidInstance;
		LPSTR guidProduct;
		LPSTR instanceName;
		LPSTR productName;
	};
    struct DeviceAxisInfo {
        DWORD offset;
        DWORD type;
        DWORD flags;
        DWORD ffMaxForce;
        DWORD ffForceResolution;
        DWORD collectionNumber;
        DWORD designatorIndex;
        DWORD usagePage;
        DWORD usage;
        DWORD dimension;
        DWORD exponent;
        DWORD reportId;
        LPSTR guidType;
        LPSTR name;
    };
    struct Effects {
        typedef enum {
            ConstantForce = 0,
            RampForce = 1,
            Square = 2,
            Sine = 3,
            Triangle = 4,
            SawtoothUp = 5,
            SawtoothDown = 6,
            Spring = 7,
            Damper = 8,
            Inertia = 9,
            Friction = 10,
            CustomForce = 11
        } Type;
    };
}
void ClearDeviceInstances();
BOOL CALLBACK ffbDeviceEnumCallback(const DIDEVICEINSTANCE* pInst, void* pContext);
