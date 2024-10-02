#pragma once
#include "IL2CPPTypes.hpp"

class IL2CPPBase {
public:
    void Init() {
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
    }
};