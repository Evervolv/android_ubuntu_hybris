/*
 * Copyright (C) 2012 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michael Frey <michael.frey@canonical.com>
 */

#include <dlfcn.h>
#include <stddef.h>

extern void *android_dlopen(const char *filename, int flag);

static void *_libc = NULL;

static void _init_androidc()
{
    _libc = (void *) android_dlopen("/system/lib/libc.so", RTLD_LAZY);
}

void init_hybris()
{
    if (_libc == NULL)
        _init_androidc();
}
