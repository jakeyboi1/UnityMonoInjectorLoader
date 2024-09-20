#include "pch.h"
#include <cstdio>
#include <Windows.h>
#include <iostream>
using namespace std;

// Added to remove need for mono headers
typedef VOID MonoObject;
typedef VOID MonoDomain;
typedef VOID MonoAssembly;
typedef VOID MonoImage;
typedef VOID MonoClass;
typedef VOID MonoMethod;
typedef VOID MonoImageOpenStatus;

// typedefs and fields for required mono functions
typedef void(__cdecl* t_mono_thread_attach)(MonoDomain*);
t_mono_thread_attach fnThreadAttach;
typedef  MonoDomain* (__cdecl* t_mono_get_root_domain)(void);
t_mono_get_root_domain fnGetRootDomain;
typedef MonoAssembly* (__cdecl* t_mono_assembly_open)(const char*, MonoImageOpenStatus*);
t_mono_assembly_open fnAssemblyOpen;
typedef MonoImage* (__cdecl* t_mono_assembly_get_image)(MonoAssembly*);
t_mono_assembly_get_image fnAssemblyGetImage;
typedef MonoClass* (__cdecl* t_mono_class_from_name)(MonoImage*, const char*, const char*);
t_mono_class_from_name fnClassFromName;
typedef MonoMethod* (__cdecl* t_mono_class_get_method_from_name)(MonoClass*, const char*, int);
t_mono_class_get_method_from_name fnMethodFromName;
typedef MonoObject* (__cdecl* t_mono_runtime_invoke)(MonoMethod*, void*, void**, MonoObject**);
t_mono_runtime_invoke fnRuntimeInvoke;
typedef const char* (__cdecl* t_mono_assembly_getrootdir)(void);
t_mono_assembly_getrootdir fnGetRootDir;
void initMonoFunctions(HMODULE mono) {
    fnThreadAttach = (t_mono_thread_attach)GetProcAddress(mono, "mono_thread_attach");
    fnGetRootDomain = (t_mono_get_root_domain)GetProcAddress(mono, "mono_get_root_domain");
    fnAssemblyOpen = (t_mono_assembly_open)GetProcAddress(mono, "mono_assembly_open");
    fnAssemblyGetImage = (t_mono_assembly_get_image)GetProcAddress(mono, "mono_assembly_get_image");
    fnClassFromName = (t_mono_class_from_name)GetProcAddress(mono, "mono_class_from_name");
    fnMethodFromName = (t_mono_class_get_method_from_name)GetProcAddress(mono, "mono_class_get_method_from_name");
    fnRuntimeInvoke = (t_mono_runtime_invoke)GetProcAddress(mono, "mono_runtime_invoke");
    fnGetRootDir = (t_mono_assembly_getrootdir)GetProcAddress(mono, "mono_assembly_getrootdir");
}

void Init() {
    // Allocating our console (Using this method of redirecting both otherwise cin fails)
    AllocConsole();
    FILE* stream;
    freopen_s(&stream, "CONOUT$", "w", stdout); // Redirect stdout
    freopen_s(&stream, "CONIN$", "r", stdin);  // Redirect stdin
    cout.clear(); // Clear the cout stream state

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
    
    // Injecting our mono DLL
    // Credit: https://www.unknowncheats.me/forum/unity/268053-mono-unity-injector.html
    #define MONO_DLL L"mono.dll"

    std::string assemblyDir;

    HMODULE mono;
    MonoDomain* domain;
    MonoAssembly* assembly;
    MonoImage* image;
    MonoClass* klass;
    MonoMethod* method;

    /* grab the mono dll module */
    mono = LoadLibraryW(MONO_DLL);
    /* initialize mono functions */
    initMonoFunctions(mono);
    /* grab the root domain */
    domain = fnGetRootDomain();
    fnThreadAttach(domain);
    /* Grab our root directory*/
    assemblyDir.append(fnGetRootDir());
    assemblyDir.append("/" + dllName);
    /* open payload assembly */
    assembly = fnAssemblyOpen(assemblyDir.c_str(), NULL);
    if (assembly == NULL) return;
    /* get image from assembly */
    image = fnAssemblyGetImage(assembly);
    if (image == NULL) return;
    /* grab the class */
    klass = fnClassFromName(image, payloadNamespace.c_str(), payloadClass.c_str());
    if (klass == NULL) return;
    /* grab the hack entrypoint */
    method = fnMethodFromName(klass, payloadMainFunction.c_str(), 0);
    if (method == NULL) return;
    /* call our entrypoint */
    fnRuntimeInvoke(method, NULL, NULL, NULL);
    cout << "Cheat injected enjoy your game!";
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
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

