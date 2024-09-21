#include "pch.h"
#include <Windows.h>
#include <cstdio>
#include <iostream>
using namespace std;

// Mono defines to avoid needing to import mono headers
// We use VOID instead of void as VOID is specifically for typedef in windows investigate using normal void
// This is essentially just making these names into Void pointers
typedef VOID MonoDomain;
typedef VOID MonoAssembly;
typedef VOID MonoImage;
typedef VOID MonoClass;
typedef VOID MonoMethod;
typedef VOID MonoImage;
typedef VOID MonoImageOpenStatus;
typedef VOID MonoObject;

// Setting up our functions for mono
// basically we are making typedefs that match the function within the official mono headers
// Here is the official def for thread attach within the mono jit.h header file
// MonoDomain* mono_jit_thread_attach (MonoDomain *domain);
// You can see all we are doing is recreating its signature (Matching its type parameters arguements etc)
// Note a key difference with thread attach specifically is we call it a void typedef. We do this as we do not need to do anything with the return value with the rest we do need the return value so we match the return value type
typedef void(__cdecl* t_mono_thread_attach)(MonoDomain*);
t_mono_thread_attach fnThreadAttach;
typedef MonoDomain* (__cdecl* t_mono_get_root_domain)(void);
t_mono_get_root_domain fnGetRootDomain;
typedef const char* (__cdecl* t_mono_assembly_getrootdir)(void);
t_mono_assembly_getrootdir fnGetRootDir;
typedef MonoAssembly* (__cdecl* t_mono_assembly_open)(const char*, MonoImageOpenStatus*);
t_mono_assembly_open fnAssemblyOpen;
typedef  MonoImage* (__cdecl* t_mono_assembly_get_image)(MonoAssembly*);
t_mono_assembly_get_image fnAssemblyGetImage;
typedef MonoClass* (__cdecl* t_mono_class_from_name)(MonoImage*, const char*, const char*);
t_mono_class_from_name fnClassFromName;
typedef MonoMethod* (__cdecl* t_mono_class_get_method_from_name)(MonoImage*, const char*, int);
t_mono_class_get_method_from_name fnMethodFromName;
typedef MonoObject* (__cdecl* t_mono_runtime_invoke)(MonoMethod*, void*, void**, MonoObject**);
t_mono_runtime_invoke fnRuntimeInvoke;

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

    // Getting our mono.dll module
    HMODULE monoModule = LoadLibraryW(L"mono.dll"); // Note we add L here to make our string coptabile with the character encoding of LoadLibraryW
    // This function is loading the mono.dll into our address space see: https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-loadlibraryw 
    // Allowing us to then retrieve mono functions from the .dll using GetProcAddress(monoModule, "whateverMonoFunction")

    // Initilizing our mono functions so we can use them
    fnThreadAttach = (t_mono_thread_attach)GetProcAddress(monoModule, "mono_thread_attach"); // This is retreiving the function mono_thread_attach from the mono.dll file, to find more information on mono functions see: https://www.mono-project.com/docs/advanced/embedding/
    // In short what this does is register a calling thread (our cheat dll in this case) within the Mono runtime allowing to run code
    fnGetRootDomain = (t_mono_get_root_domain)GetProcAddress(monoModule, "mono_get_root_domain");
    // This functions returns a pointer to the root domain of the mono runtime in our process
    fnGetRootDir = (t_mono_assembly_getrootdir)GetProcAddress(monoModule, "mono_assembly_getrootdir"); // This function will return the root directory of the assembly, aka in our case the Managed folder where the games .dll files are stored which also contains our cheat .dll file
    fnAssemblyGetImage = (t_mono_assembly_get_image)GetProcAddress(monoModule, "mono_assembly_get_image"); // This gets an image (metadata) about the assembly we loaded. Allowing us to see data within the assembly like types methods fields classes etc
    fnAssemblyOpen = (t_mono_assembly_open)GetProcAddress(monoModule, "mono_assembly_open");
    fnClassFromName = (t_mono_class_from_name)GetProcAddress(monoModule, "mono_class_from_name"); // This simply gets the class from our assembly image
    fnMethodFromName = (t_mono_class_get_method_from_name)GetProcAddress(monoModule, "mono_class_get_method_from_name"); // this simply gets a method from our assembly image
    fnRuntimeInvoke = (t_mono_runtime_invoke)GetProcAddress(monoModule, "mono_runtime_invoke");

    // Attaching our dll into mono runtime
    MonoDomain* domain;
    domain = fnGetRootDomain(); // Getting our root domain
    fnThreadAttach(domain); // Attaching this .dll to mono runtime
    
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
