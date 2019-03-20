#ifndef HIDDEN_API_BYPASS_H_
#define HIDDEN_API_BYPASS_H_

#include <android/log.h>
#include <linker/linker_namespaces.h>
#include <elf.h>

typedef Elf32_Half Elf32_Versym;
typedef Elf64_Half Elf64_Versym;
typedef Elf32_Word Elf32_Relr;
typedef Elf64_Xword Elf64_Relr;

#include <linker/linker_soinfo.h>

#include <LIEF/ELF.hpp>

#define  LOG_TAG "hidden-bypass"
#define  ALOG(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)


using get_soname_t            = const char* (*)(soinfo*);
using get_primary_namespace_t = android_namespace_t* (*)(soinfo*);

using namespace LIEF::ELF; // It's fine for this project

struct module_t {
  public:
  std::string name;
  std::unique_ptr<Binary> bin;
  uintptr_t base;

  public:
  uintptr_t get_address(const std::string& symname) {
    if (not this->bin->has_symbol(symname)) {
      return 0;
    }
    Symbol& sym = reinterpret_cast<Symbol&>(this->bin->get_symbol(symname));
    return this->base + sym.value();
  }

  operator bool() {
    return this->bin != nullptr;
  }
};


module_t get_module(const std::string& name);
void disable_namespace(void);
void disable_hidden_api(void);

constexpr uintptr_t page_mask(size_t page_size) {
  return ~(page_size - 1);
}

constexpr uintptr_t page_start(uintptr_t address, size_t page_size) {
  return address & page_mask(page_size);
}

constexpr uintptr_t page_align(uintptr_t address, size_t page_size) {
  return (address + (page_size - 1)) & page_mask(page_size);
}

constexpr uintptr_t page_offset(uintptr_t address, size_t page_size) {
  return (address & (page_size - 1));
}



#endif

