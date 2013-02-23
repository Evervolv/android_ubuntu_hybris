/*
 * Copyright (c) 2012 Carsten Munk <carsten.munk@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <dlfcn.h>
#include <stddef.h>
#include <assert.h>
int main(int argc, char **argv)
{
    void *foo = android_dlopen(argv[1], RTLD_LAZY);
    void* (*hello)(char a, char b);
    assert(foo != NULL);
    hello = android_dlsym(foo, "hello");
    assert(hello != NULL);
    (*hello)('z', 'd');
    printf("full stop\n");    
exit(0); 
}  
