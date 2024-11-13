// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <Windows.h>
#include <string>
#include <iostream>
#include <stdio.h>
#include <chrono>
#include "injector/injector.hpp"
#include "ini.h"

#pragma comment(lib, "XInput9_1_0.lib")
#pragma comment(lib, "XInput.lib")
#include <xinput.h>

static auto eCreateLookAtMatrix = (int(__cdecl*)(void*, void*, void*, void*))0x005BA9A0;
float* DeltaTime = (float*)0x00865144;
void* HookAddr = (void*)0x004443CF;

#define M_PI 3.14159265358979323846

struct Vec3
{
    float x;
    float y;
    float z;
};

bool IsPressed(int key)
{
    return GetAsyncKeyState(key) & 0x8000;
}

int HK_Left, HK_Right, HK_Down;
float CamAngle = 0;
float CamSpeed;

int __cdecl CreateLookAtMatrixHook(void* outMatrix, Vec3* from, Vec3* to, Vec3* up)
{
    float Target = CamAngle;

    if (IsPressed(HK_Left))
    {
        Target = -45.0f;
    }
    else if (IsPressed(HK_Right))
    {
        Target = 45.0f;
    }
    else if (IsPressed(HK_Down))
    {
        Target = -90.0f;
    }
    else
    {
        Target = 0.0f;
    }

    float angleDifference = Target - CamAngle;
    if (fabs(angleDifference) > 0.01f)
    {
        int direction = (angleDifference > 0) ? 1 : -1;

        float step = direction * CamSpeed * (*DeltaTime);
        if (fabs(step) > fabs(angleDifference))
        {
            CamAngle = Target;
        }
        else
        {
            CamAngle += step;
        }
    }

    if (CamAngle != 0.0f)
    {
        float angle = CamAngle * M_PI / 180.0f;
        Vec3 newFrom;

        newFrom.x = cos(angle) * (from->x - to->x) - sin(angle) * (from->y - to->y) + to->x;
        newFrom.y = sin(angle) * (from->x - to->x) + cos(angle) * (from->y - to->y) + to->y;
        newFrom.z = from->z;
        *from = newFrom;
    }

    return eCreateLookAtMatrix(outMatrix, from, to, up);
}

void Main()
{
	mINI::INIFile file("ModernCamera.ini");
	mINI::INIStructure ini;
	file.read(ini);

	std::string& keyLeft = ini["Main"]["LookLeft"];
	std::string& keyRight = ini["Main"]["LookRight"];
	std::string& keyBack = ini["Main"]["LookBack"];
	std::string& cSpeed = ini["Main"]["LookSpeed"];

	HK_Left = std::stoi(keyLeft);
	HK_Right = std::stoi(keyRight);
	HK_Down = std::stoi(keyBack);

	CamSpeed = std::stof(cSpeed);

	injector::MakeCALL(HookAddr, CreateLookAtMatrixHook, true);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        Main();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

