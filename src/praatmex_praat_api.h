#pragma once
/*
 * praatmex_praat_api.h — Compatibility shim
 *
 * Praat's headers use conststring32 = char32_t* and autoSomeThing<T>.
 * This header provides the same types so our MEX code can compile
 * against Praat's internal API without pulling in private headers.
 */

#include <cstddef>
#include <cstdint>

using conststring32 = const char32_t *;
using mutablestring32 = char32_t *;

#include "melder.h"
