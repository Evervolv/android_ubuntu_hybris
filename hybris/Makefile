ARCH=arm
CC=gcc
CXX=g++

COMMON_FLAGS=

ifeq ($(ARCH),arm)
	ARCHFLAGS = -DHAVE_ARM_TLS_REGISTER -DANDROID_ARM_LINKER
else
	ARCHFLAGS = -DANDROID_X86_LINKER
endif


COMMON_SOURCES=common/strlcpy.c common/hooks.c common/properties.c

ICS_SOURCES=ics/linker.c ics/dlfcn.c ics/rt.c ics/linker_environ.c ics/linker_format.c ics/init.c

all:  libhybris_ics.so libEGL.so.1 libGLESv2.so.2 libcamera.so libmediaplayer.so libhardware.so libis.so libsf.so test_camera test_media_player test_sf test_egl test_hw test_sensors test_glesv2

libhybris_ics.so: $(COMMON_SOURCES) $(ICS_SOURCES)
	$(CC) -g -shared -o $@ -ldl -pthread -fPIC -Iics -Icommon -DLINKER_DEBUG=1 -DLINKER_TEXT_BASE=0xB0000100 -DLINKER_AREA_SIZE=0x01000000 $(ARCHFLAGS) \
		$(ICS_SOURCES) $(COMMON_SOURCES)


libhardware.so.1.0: hardware/hardware.c
	$(CC) -g -shared -o $@ -fPIC -Wl,-soname,libhardware.so.1 $< libhybris_ics.so

libhardware.so.1: libhardware.so.1.0
	ln -sf libhardware.so.1.0 libhardware.so.1

libhardware.so: libhardware.so.1
	ln -sf libhardware.so.1.0 libhardware.so

libEGL.so.1.0: egl/egl.c
	$(CC) -g -shared -o $@ -fPIC -Wl,-soname,libEGL.so.1 $< libhybris_ics.so

libcamera.so.1.0: camera/camera.cpp
	$(CXX) -g -fpermissive -shared -o $@ -fPIC -I../compat/camera -Wl,-soname,libcamera.so.1 $< libhybris_ics.so

libmediaplayer.so.1.0: media/media.cpp
	$(CXX) -g -fpermissive -shared -o $@ -fPIC -I../compat/media -Wl,-soname,libmediaplayer.so.1 $< libhybris_ics.so

libis.so.1.0: is/is.cpp
	$(CXX) -g -fpermissive -shared -o $@ -fPIC -I../compat/input -Wl,-soname,libis.so.1 $< libhybris_ics.so

libsf.so.1.0: sf/sf.cpp
	$(CXX) -g -fpermissive -shared -o $@ -fPIC -I../compat/surface_flinger -Wl,-soname,libsf.so.1 $< libhybris_ics.so

libEGL.so.1: libEGL.so.1.0
	ln -sf libEGL.so.1.0 libEGL.so.1

libGLESv2.so.2.0: glesv2/gl2.c
	$(CC) -g -shared -o $@ -fPIC -Wl,-soname,libGLESv2.so.2 $< libhybris_ics.so

libGLESv2.so.2: libGLESv2.so.2.0
	ln -sf libGLESv2.so.2.0 libGLESv2.so.2

libis.so.1: libis.so.1.0
	ln -sf libis.so.1.0 libis.so.1

libis.so: libis.so.1
	ln -sf libis.so.1.0 libis.so

libsf.so.1: libsf.so.1.0
	ln -sf libsf.so.1.0 libsf.so.1

libsf.so: libsf.so.1
	ln -sf libsf.so.1.0 libsf.so

libcamera.so.1: libcamera.so.1.0
	ln -sf libcamera.so.1.0 libcamera.so.1

libcamera.so: libcamera.so.1
	ln -sf libcamera.so.1.0 libcamera.so

libmediaplayer.so.1: libmediaplayer.so.1.0
	ln -sf libmediaplayer.so.1.0 libmediaplayer.so.1

libmediaplayer.so: libmediaplayer.so.1
	ln -sf libmediaplayer.so.1.0 libmediaplayer.so

test_egl: libEGL.so.1 libGLESv2.so.2 egl/test.c libhybris_ics.so
	$(CC) -g -o $@ egl/test.c libEGL.so.1 libGLESv2.so.2 libhybris_ics.so  -I .

test_sf: libsf.so sf/test.cpp libhybris_ics.so
	$(CXX) -g -o $@ sf/test.cpp libsf.so libEGL.so.1 libGLESv2.so.2 libhybris_ics.so  -I../compat/surface_flinger

test_camera: libcamera.so libsf.so libis.so camera/test.cpp libhybris_ics.so
	$(CXX) -g -o $@ camera/test.cpp libcamera.so libsf.so libis.so libEGL.so.1 libGLESv2.so.2 libhybris_ics.so  -I../compat/camera -I../compat/input -I../compat/surface_flinger

test_media_player: libmediaplayer.so libsf.so libis.so media/test_player.cpp libhybris_ics.so
	$(CXX) -g -o $@ media/test_player.cpp libmediaplayer.so libsf.so libis.so libEGL.so.1 libGLESv2.so.2 libhybris_ics.so  -I../compat/media -I../compat/input -I../compat/surface_flinger

test_hw: libhardware.so hardware/test.c libhybris_ics.so
	$(CC) -g -o $@ hardware/test.c libhardware.so libhybris_ics.so  -I .

test_sensors: libhardware.so hardware/sensors.c libhybris_ics.so
	$(CC) -g -o $@ hardware/sensors.c libhardware.so libhybris_ics.so  -I .

test_glesv2: libEGL.so.1 libGLESv2.so.2 egl/test.c libhybris_ics.so
	$(CC) -g -o $@ glesv2/test.c -lm libEGL.so.1 libhybris_ics.so libGLESv2.so.2

clean:
	rm -rf libhybris_ics.so test_ics
	rm -rf libEGL* libGLESv2*
	rm -rf libhardware*
	rm -rf libsf*
	rm -rf libcamera*
	rm -rf libmediaplayer*
	rm -rf libis*
	rm -rf test_*
