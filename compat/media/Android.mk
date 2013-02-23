LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

HYBRIS_PATH := $(LOCAL_PATH)/../../

LOCAL_SRC_FILES:= \
	media_compatibility_layer.cpp

LOCAL_MODULE:= libmedia_compat_layer
LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES := \
	$(HYBRIS_PATH)/compat/surface_flinger \
	$(HYBRIS_PATH)/compat/input \
	frameworks/base/media/libstagefright/include \
	frameworks/base/include/media/stagefright \
	frameworks/base/include/media

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libbinder \
	libhardware \
	libui \
	libgui \
	libstagefright \
	libmedia

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:=		\
	test_player.cpp \

LOCAL_MODULE:= test_player
LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES := \
	bionic \
	bionic/libstdc++/include \
	external/gtest/include \
	external/stlport/stlport \
	external/skia/include/core \
	$(HYBRIS_PATH)/compat/surface_flinger \
	$(HYBRIS_PATH)/compat/input \
	frameworks/base/include

LOCAL_SHARED_LIBRARIES := \
	libis_compat_layer \
	libsf_compat_layer \
	libmedia_compat_layer \
	libcutils \
	libutils \
	libbinder \
	libhardware \
	libui \
	libgui \
	libEGL \
	libGLESv2

include $(BUILD_EXECUTABLE)
