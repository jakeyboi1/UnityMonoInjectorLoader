#include <Windows.h>

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