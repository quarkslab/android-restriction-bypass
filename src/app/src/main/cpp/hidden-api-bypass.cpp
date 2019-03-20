
#include "hidden-api-bypass.hpp"

#include <elf.h>
#include <jni.h>
#include <string>
#include <dlfcn.h>
#include <link.h>
#include <sys/mman.h>
#include <unordered_map>

#include <runtime/runtime.h>
#include <runtime/hidden_api.h>
#include <runtime/java_vm_ext.h>

// See: http://androidxref.com/9.0.0_r3/xref/bionic/linker/linker_soinfo.h#273
get_soname_t get_soname = nullptr;

// See: http://androidxref.com/9.0.0_r3/xref/bionic/linker/linker_soinfo.h#285
get_primary_namespace_t get_primary_namespace = nullptr;

static std::unordered_map<uintptr_t, soinfo*>* g_soinfo_handles_map = nullptr;
static module_t linker;
static art::Runtime* runtime = nullptr;

module_t get_module(const std::string& name) {
  module_t module;
  module.name = name;
  dl_iterate_phdr([] (dl_phdr_info* info, size_t size, void* data) {
      module_t* m = reinterpret_cast<module_t*>(data);
      if (info->dlpi_name == nullptr) {
        return 0;
      }
      if (std::string(info->dlpi_name).find(m->name) != std::string::npos) {
        m->base = info->dlpi_addr;
        m->bin  = Parser::parse(info->dlpi_name);
        return 1;
      }
      return 0;
  }, reinterpret_cast<void*>(&module));
  return std::move(module);
}




void disable_namespace(void) {
  for (auto&& [hdl, info] : *g_soinfo_handles_map) {
    const char* name        = get_soname(info);
    android_namespace_t* ns = get_primary_namespace(info);
    void* start = reinterpret_cast<void*>(page_start(reinterpret_cast<uintptr_t>(ns), 4096));
    size_t size = page_align(reinterpret_cast<uintptr_t>(ns) + sizeof(android_namespace_t), 4096);
    mprotect(start, size, PROT_READ | PROT_WRITE);
    ns->set_ld_library_paths({"/system/lib64", "/sytem/lib"});
    ns->set_isolated(false);
  }
}

__attribute__((constructor))
void __ctor(void) {
  ALOG("PID: %d", getpid());
  linker     = get_module("linker64");
  if (not linker) {
    ALOG("Unable to find linker!");
  } else {
    g_soinfo_handles_map  = reinterpret_cast<decltype(g_soinfo_handles_map)>(linker.get_address("__dl_g_soinfo_handles_map"));
    get_soname            = reinterpret_cast<decltype(get_soname)>(linker.get_address("__dl__ZNK6soinfo10get_sonameEv"));
    get_primary_namespace = reinterpret_cast<decltype(get_primary_namespace)>(linker.get_address("__dl__ZN6soinfo21get_primary_namespaceEv"));
  }
}

void disable_hidden_api(void) {
  ALOG("Version: %s",           runtime->GetVersion());
  ALOG("EnforcementPolicy: %x", runtime->GetHiddenApiEnforcementPolicy());
  // Disable
  runtime->SetHiddenApiEnforcementPolicy(art::hiddenapi::EnforcementPolicy::kNoChecks);
}

extern "C" JNIEXPORT void JNICALL
Java_re_android_hiddenapi_MainActivity_disableProtectedNamespace(JNIEnv* env, jobject /* this */) {
  return disable_namespace();
}

extern "C" JNIEXPORT void JNICALL
Java_re_android_hiddenapi_MainActivity_disableHiddenApi(JNIEnv* env, jobject /* this */) {
  return disable_hidden_api();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_re_android_hiddenapi_MainActivity_isProtectedNamespaceEnabled(JNIEnv* env, jobject /* this */) {
  void* art_handle = dlopen("/system/lib64/libart.so", RTLD_NOW);
  return art_handle == nullptr;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_re_android_hiddenapi_MainActivity_isHiddenApiEnabled(JNIEnv* env, jobject /* this */) {
  jclass Debug = env->FindClass("android/os/Debug");

  if (Debug == nullptr) {
    ALOG("Can find android/os/Debug");
    return true;
  }

  // One the method that is black listed
  jmethodID mid = env->GetStaticMethodID(Debug, "getVmFeatureList", "()[Ljava/lang/String;");

  if (mid == nullptr) {
    ALOG("Can't load method!");
    return true;
  }
  return false;
}


extern "C" jint JNI_OnLoad(JavaVM *vm, void *reserved) {
  runtime = reinterpret_cast<art::JavaVMExt*>(vm)->GetRuntime();
  return JNI_VERSION_1_4;
}

