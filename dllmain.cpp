#include "pch.h"
#include <Windows.h>
#include <cstdio>
#include <iostream>
#include "IL2CPP.hpp"
using namespace std;

void Init() {
    // Allocation a console for debugging
    AllocConsole();
    FILE* console;
    freopen_s(&console, "CONOUT$", "w", stdout);
    freopen_s(&console, "CONIN$", "r", stdin);
    cout.clear();

    // Getting the information of the dll we want to inject
    string dllName;
    string payloadNamespace;
    string payloadClass;
    string payloadMainFunction;
    cout << "Make sure the .dll you want to inject is inside of the games Managed folder or this will not work!\n";
    cout << "Input the name of the .dll file you want to inject (Include the .dll extension in the name): \n";
    cin >> dllName;
    cout << "\nInput the name of the .dll's namespace: \n";
    cin >> payloadNamespace;
    cout << "\nInput the name of the .dlls loader class: \n";
    cin >> payloadClass;
    cout << "\nInput the name of the main function in the loader class: \n";
    cin >> payloadMainFunction;
    cout << "\n";

    IL2CPPBase il2cppBase;
    il2cppBase.Init();

    // Getting the directory to our cheat .dll file
    string assemblyDir;
    assemblyDir.append(fnGetRootDir());
    assemblyDir.append("/" + dllName); // CHANGE

    // Loading our cheat assembly (.dll file) into the mono runtime allowing us to use it in our game. This funcction returns a pointer to our assembly which we can use
    MonoAssembly* cheatAssembly;
    cheatAssembly = fnAssemblyOpen(assemblyDir.c_str(), NULL); // We pass null as its not required

    // Getting our class and method from our assembly
    MonoImage* cheatImage;
    cheatImage = fnAssemblyGetImage(cheatAssembly);
    MonoClass* cheatClass;
    cheatClass = fnClassFromName(cheatImage, payloadNamespace.c_str(), payloadClass.c_str());
    MonoMethod* cheatMethod;
    cheatMethod = fnMethodFromName(cheatClass, payloadMainFunction.c_str(), 0);

    // Invoking our method (basically calling our function)
    fnRuntimeInvoke(cheatMethod, NULL, NULL, NULL);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        Init();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
