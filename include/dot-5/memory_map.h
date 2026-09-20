#ifndef _D5_MEMORY_MAP_H_
#define _D5_MEMORY_MAP_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef _WIN32
  #include <wchar.h>
#endif

#ifndef __CUSTOM_TYPES_
#define __CUSTOM_TYPES_

typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t dword;
typedef uint64_t qword;

#endif

#define RAM_SIZE 0x8
#define ROM_SIZE 0xf8
#define ROM_ENTRY_POINT (0x100 - ROM_SIZE)

extern byte mem_load_rom(byte *rom, byte size);
extern byte mem_load_rom_from_file(const char *filename);
#ifdef _WIN32
extern byte mem_load_rom_from_file_w(const wchar_t *filename);
#endif

extern byte mem_read(byte address);
extern void mem_write(byte address, byte value);

#endif