#ifndef sodium_esphome_patched_H
#define sodium_esphome_patched_H

/*
 * Marker header created by patches/06-noise-session-api.patch. Its presence
 * tells port_include/sodium/sodium_esphome.h that the tree was patched and
 * the session/aead entry points exist, so the capability macro can never be
 * advertised by an unpatched checkout (e.g. the generic CMake target built
 * straight from the pristine submodule).
 */

#endif
