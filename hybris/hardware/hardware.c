/*
 * Copyright (C) 2012 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michael Frey <michael.frey@canonical.com>
 */

#include <dlfcn.h>
#include <stddef.h>
#include <stdint.h>

#include "hardware.h"

static void *_libhardware = NULL;

static void (*_hw_get_module)(const char *id, const struct hw_module_t **module) = NULL;

static void (*_hw_get_module_by_class)(const char *class_id, const char *inst,
                                   const struct hw_module_t **module) = NULL;

static void _init_androidhardware()
{
 _libhardware = (void *) android_dlopen("/system/lib/libhardware.so", RTLD_LAZY);
}

#define HARDWARE_DLSYM(fptr, sym) do { if (_libhardware == NULL) { _init_androidhardware(); }; if (*(fptr) == NULL) { *(fptr) = (void *) android_dlsym(_libhardware, sym); } } while (0) 

void  hw_get_module (const char *id, const struct hw_module_t **module)
{
  HARDWARE_DLSYM(&_hw_get_module, "hw_get_module");
  (*_hw_get_module)(id, module);
}

int hw_get_module_by_class(const char *class_id, const char *inst,
                           const struct hw_module_t **module)
{
  HARDWARE_DLSYM(&_hw_get_module_by_class, "hw_get_module_by_class");
  (*_hw_get_module_by_class)(class_id, inst, module);
}



