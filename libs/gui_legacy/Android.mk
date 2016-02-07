LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	BitTube.cpp \
	BufferItemConsumer.cpp \
	BufferQueue.cpp \
	ConsumerBase.cpp \
	CpuConsumer.cpp \
	DisplayEventReceiver.cpp \
	DummyConsumer.cpp \
	GLConsumer.cpp \
	GraphicBufferAlloc.cpp \
	GuiConfig.cpp \
	IDisplayEventConnection.cpp \
	IGraphicBufferAlloc.cpp \
	IGraphicBufferProducer.cpp \
	ISensorEventConnection.cpp \
	ISensorServer.cpp \
	ISurface.cpp \
	ISurfaceComposer.cpp \
	ISurfaceComposerClient.cpp \
	LayerState.cpp \
	Sensor.cpp \
	SensorEventQueue.cpp \
	SensorManager.cpp \
	Surface.cpp \
	SurfaceControl.cpp \
	SurfaceComposerClient.cpp \

LOCAL_SHARED_LIBRARIES := \
	libbinder \
	libcutils \
	libEGL \
	libGLESv2 \
	libsync \
	libui \
	libutils \


LOCAL_MODULE:= libgui_legacy

ifeq ($(TARGET_BOARD_PLATFORM), omap4)
	LOCAL_CFLAGS += -DUSE_FENCE_SYNC
endif
ifeq ($(TARGET_BOARD_PLATFORM), s5pc110)
	LOCAL_CFLAGS += -DUSE_FENCE_SYNC
endif
ifeq ($(TARGET_BOARD_PLATFORM), exynos5)
	LOCAL_CFLAGS += -DUSE_NATIVE_FENCE_SYNC
	LOCAL_CFLAGS += -DUSE_WAIT_SYNC
endif
ifneq ($(filter generic%,$(TARGET_DEVICE)),)
    # Emulator build
    LOCAL_CFLAGS += -DUSE_FENCE_SYNC
endif

ifeq ($(call is-vendor-board-platform,QCOM),true)
ifneq ($(TARGET_QCOM_DISPLAY_VARIANT),legacy)
	LOCAL_CFLAGS += -DUSE_NATIVE_FENCE_SYNC
endif
endif

include $(BUILD_SHARED_LIBRARY)

ifeq (,$(ONE_SHOT_MAKEFILE))
include $(call first-makefiles-under,$(LOCAL_PATH))
endif
