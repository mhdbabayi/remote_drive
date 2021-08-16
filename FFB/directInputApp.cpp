// directInputApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "logitechReadWrite.h"

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib , "dxguid.lib")
LPDIRECTINPUT8			g_pDI = NULL;
LPDIRECTINPUTDEVICE8	g_pDevice = NULL;

std::vector<DeviceInfo> g_vDeviceInstances;
std::vector<DeviceAxisInfo> g_vDeviceAxes;
std::map<Effects::Type, LPDIRECTINPUTEFFECT> g_mEffects;
std::map<Effects::Type, DIEFFECT>   g_mDIEFFECTS;

#define CONSOLE_WINDOW_TITLE_BUFFER_SIZE 1024

int numDevices;
int numAxis;

/*function prototypes*/
void FreeFFBDevice();
void ClearDeviceAxes();
BOOL CALLBACK _cbEnumFFBAxes(const DIDEVICEOBJECTINSTANCE* pdidoi, void* pContext);

driverMessage oldMessage; /*the last succesfully read drive input. If poll or read steat fail for any reason
                          We'll send the last message with a flag that says it has failed*/


HRESULT startDirectInput() {
    if (g_pDI != NULL)
        return S_OK;
    return DirectInput8Create(
        GetModuleHandle(NULL),
        DIRECTINPUT_VERSION,
        IID_IDirectInput8,
        (void**)&g_pDI,
        NULL);
}
DeviceInfo* EnumerateFFBDevices(int& deviceCount) {
    if (g_pDI == NULL)
        return NULL;
    ClearDeviceInstances();
    HRESULT hr = g_pDI->EnumDevices(
        DI8DEVCLASS_GAMECTRL,
        ffbDeviceEnumCallback,
        NULL,
        DIEDFL_ATTACHEDONLY | DIEDFL_FORCEFEEDBACK);
    if (g_vDeviceInstances.size() > 0)
    {
        deviceCount = (int)g_vDeviceInstances.size();
        return &g_vDeviceInstances[0];
        
    }
    else {
        deviceCount = 0;
    }
    return NULL;
}
/*
the callback function on detection of force feedback devices
*/
BOOL CALLBACK ffbDeviceEnumCallback(const DIDEVICEINSTANCE* pInst, void* pContext) 
{
    DeviceInfo di = { 0 };

    OLECHAR* guidInstance;
    StringFromCLSID(pInst->guidInstance, &guidInstance);
    OLECHAR* guidProduct;
    StringFromCLSID(pInst->guidProduct, &guidProduct);

    std::string strGuidInstance = utf16ToUTF8(guidInstance);
    std::string strGuidProduct = utf16ToUTF8(guidProduct);
    std::string strInstanceName = utf16ToUTF8(pInst->tszInstanceName);
    std::string strProductName = utf16ToUTF8(pInst->tszProductName);

    di.guidInstance = new char[strGuidInstance.length() + 1];
    di.guidProduct = new char[strGuidProduct.length() + 1];
    di.instanceName = new char[strInstanceName.length() + 1];
    di.productName = new char[strProductName.length() + 1];

    strcpy_s(di.guidInstance, strGuidInstance.length() + 1, strGuidInstance.c_str());
    strcpy_s(di.guidProduct, strGuidProduct.length() + 1, strGuidProduct.c_str());
    di.deviceType = pInst->dwDevType;
    strcpy_s(di.instanceName, strInstanceName.length() + 1, strInstanceName.c_str());
    strcpy_s(di.productName, strProductName.length() + 1, strProductName.c_str());

    g_vDeviceInstances.push_back(di);
    std::cout << "detected device named:" << di.productName<<"\n";

    return DIENUM_CONTINUE;


}
HRESULT CreateFFBDevice(LPCSTR guidInstance) {
    
    if (g_pDevice)
        FreeFFBDevice();

    GUID deviceGuid;
    // convert the guidinstance string to a guid structure
    int wcharCount = MultiByteToWideChar(CP_UTF8, 0, guidInstance, -1, NULL, 0);
    WCHAR* wstrGuidInstance = new WCHAR[wcharCount];
    MultiByteToWideChar(CP_UTF8, 0, guidInstance, -1, wstrGuidInstance, wcharCount);
    CLSIDFromString(wstrGuidInstance, &deviceGuid);
    delete[] wstrGuidInstance;

    

    LPDIRECTINPUTDEVICE8 pDevice;
    HRESULT hr = g_pDI->CreateDevice(deviceGuid, &pDevice, NULL);

    if (FAILED(hr))
    {
        std::cout << "failed at first assignment\n";
        return hr;
    }
    HWND hWnd ;

    // get a handle to the console window and use it for setting cooperative level
    TCHAR pszOldWindowTitle[CONSOLE_WINDOW_TITLE_BUFFER_SIZE];
    GetConsoleTitle(pszOldWindowTitle, CONSOLE_WINDOW_TITLE_BUFFER_SIZE);
    hWnd = FindWindow(NULL, pszOldWindowTitle);


    if (FAILED(hr = pDevice->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND)))
    {
        std::cout << "failed at setting cooperation level\n";
        return hr;
    }

    if (FAILED(hr = pDevice->SetDataFormat(&c_dfDIJoystick)))
    {
        return hr;
    }

    if (FAILED(hr = pDevice->Acquire())) {
        std::cout << "failed at acquisition";
        return hr;
    }
    g_pDevice = pDevice;
    std::cout << "ffb device created and acquired\n";
    return S_OK;

}

void ClearDeviceInstances()
{
    for (int i = 0; i < g_vDeviceInstances.size(); i++)
    {
        delete[] g_vDeviceInstances[i].guidInstance;
        delete[] g_vDeviceInstances[i].guidProduct;
        delete[] g_vDeviceInstances[i].instanceName;
        delete[] g_vDeviceInstances[i].productName;
    }
    g_vDeviceInstances.clear();
}
// function to free all devices and effects
void FreeFFBDevice()
{
    for (auto const& effect : g_mEffects) {
        if (effect.second != NULL) {
            effect.second->Stop();
            effect.second->Release();
        }
    }
    g_mEffects.clear();
    if (g_pDevice) {
        g_pDevice->Unacquire();
        g_pDevice->Release();
        g_pDevice = NULL;
    }
}

DeviceAxisInfo* EnumerateFFBAxes(int& axisCount) {
    if (g_pDevice == NULL)
        return NULL;
    DWORD _axisCount = 0;
    ClearDeviceAxes();
    g_pDevice->EnumObjects(_cbEnumFFBAxes, (void*)&_axisCount, DIDFT_AXIS);

    if (g_vDeviceAxes.size() > 0) {
        axisCount = (int)g_vDeviceAxes.size();
        std::cout << "found " << axisCount << " axes on the device names: "<<g_vDeviceAxes[0].name <<"\n";
        return &g_vDeviceAxes[0];

    }
    else {
        axisCount = 0;
        std::cout << "found no axes on the device\n";
    }
    return NULL;
    
}

BOOL CALLBACK _cbEnumFFBAxes(const DIDEVICEOBJECTINSTANCE* pdidoi, void* pContext)
{
    if ((pdidoi->dwFlags & DIDOI_FFACTUATOR) != 0)
    {
        DWORD* axisCount = (DWORD*)pContext;
        (*axisCount)++;

        DeviceAxisInfo dai = { 0 };
        int daiSize = sizeof(DeviceAxisInfo);
        OLECHAR* guidType;
        StringFromCLSID(pdidoi->guidType, &guidType);

        std::string strGuidType = utf16ToUTF8(guidType);
        std::string strName = utf16ToUTF8(pdidoi->tszName);

        dai.guidType = new char[strGuidType.length() + 1];
        dai.name = new char[strName.length() + 1];

        strcpy_s(dai.guidType, strGuidType.length() + 1, strGuidType.c_str());
        strcpy_s(dai.name, strName.length() + 1, strName.c_str());

        dai.offset = pdidoi->dwOfs;
        dai.type = pdidoi->dwType;
        dai.flags = pdidoi->dwFlags;
        dai.ffMaxForce = pdidoi->dwFFMaxForce;
        dai.ffForceResolution = pdidoi->dwFFForceResolution;
        dai.collectionNumber = pdidoi->wCollectionNumber;
        dai.designatorIndex = pdidoi->wDesignatorIndex;
        dai.usagePage = pdidoi->wUsagePage;
        dai.usage = pdidoi->wUsage;
        dai.dimension = pdidoi->dwDimension;
        dai.exponent = pdidoi->wExponent;
        dai.reportId = pdidoi->wReportId;

        g_vDeviceAxes.push_back(dai);
    }

    return DIENUM_CONTINUE;
}
void ClearDeviceAxes()
{
    for (int i = 0; i < g_vDeviceAxes.size(); i++)
    {
        delete[] g_vDeviceAxes[i].guidType;
        delete[] g_vDeviceAxes[i].name;
    }
    g_vDeviceAxes.clear();
}
HRESULT readSteeringPosition() {
    HRESULT hr;
    DIJOYSTATE steeringWheelState;
    if (FAILED(hr = g_pDevice->Poll()))
    {
        std::cout << "failed at polling\n";
        return hr;
    }
    if (FAILED(hr = g_pDevice->GetDeviceState(sizeof(steeringWheelState), &steeringWheelState))) {
        std::cout << "failed at getting device state\n";
        return hr;
    }
    std::cout << "steering position is at :" << steeringWheelState.lX << "\n";
    return S_OK;

}
void DIFullStartUp() {
    startDirectInput();
    EnumerateFFBDevices(numDevices);
    CreateFFBDevice(g_vDeviceInstances[0].guidInstance);
    EnumerateFFBAxes(numAxis);
}
driverMessage getDriverInputs(LARGE_INTEGER& frequency) {
    driverMessage newMessage;
    DIJOYSTATE joyState;
    LARGE_INTEGER HRMsgTime;
    if (FAILED( g_pDevice->Poll()))
    {
        std::cout << "failed at polling\n";
        oldMessage.latestRead = FALSE;
        return oldMessage;
    }
    if (FAILED(g_pDevice->GetDeviceState(sizeof(joyState), &joyState))) {
        std::cout << "failed at getting device state\n";
        oldMessage.latestRead = FALSE;
        return oldMessage;
        
    }
    std::cout << "steering position is at :" << joyState.lX << "\n";
    getHRTimeMS(&HRMsgTime, &frequency, newMessage.time);
    newMessage.time = newMessage.time;
    // put your signal conditioning code here 
    newMessage.stAngle = __int8(joyState.lX);
    newMessage.throttle = __int8(joyState.lRz);
    ULONG timeSecs, timeRemainder;
    timeSecs = (ULONG)(newMessage.time / (1000));
    timeRemainder = (ULONG)((newMessage.time - (ULONGLONG)timeSecs * 1000) * 1000000);
    memcpy(newMessage.message, &(timeSecs), sizeof(timeSecs));
    memcpy(newMessage.message + 4, &timeRemainder, sizeof(timeRemainder));
    memcpy(newMessage.message + 8, &newMessage.stAngle, sizeof(newMessage.stAngle));
    memcpy(newMessage.message + 9, &newMessage.message, sizeof(newMessage.message));
    newMessage.latestRead = TRUE;
    oldMessage = newMessage;
    return newMessage;
}



// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
