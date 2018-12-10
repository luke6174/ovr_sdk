LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)					# clean everything up to prepare for a module

include ../../../../cflags.mk

LOCAL_MODULE    := cinema				# generate libcinema.so
LOCAL_SRC_FILES	:= 	../../../Src/CinemaApp.cpp \
					../../../Src/Native.cpp \
					../../../Src/View.cpp \
					../../../Src/SceneManager.cpp \
					../../../Src/ViewManager.cpp \
					../../../Src/ShaderManager.cpp \
					../../../Src/ModelManager.cpp \
					../../../Src/MovieManager.cpp \
					../../../Src/MoviePlayerView.cpp \
					../../../Src/MovieSelectionView.cpp \
					../../../Src/TheaterSelectionView.cpp \
					../../../Src/TheaterSelectionComponent.cpp \
					../../../Src/CarouselBrowserComponent.cpp \
					../../../Src/MovieCategoryComponent.cpp \
					../../../Src/MoviePosterComponent.cpp \
					../../../Src/MovieSelectionComponent.cpp \
					../../../Src/ResumeMovieView.cpp \
					../../../Src/ResumeMovieComponent.cpp \
					../../../Src/CarouselSwipeHintComponent.cpp \
					../../../Src/CinemaStrings.cpp

LOCAL_STATIC_LIBRARIES += vrsound vrmodel vrlocale vrgui vrappframework libovrkernel
LOCAL_SHARED_LIBRARIES += vrapi

include $(BUILD_SHARED_LIBRARY)			# start building based on everything since CLEAR_VARS

$(call import-module,LibOVRKernel/Projects/Android/jni)
$(call import-module,VrApi/Projects/AndroidPrebuilt/jni)
$(call import-module,VrAppFramework/Projects/Android/jni)
$(call import-module,VrAppSupport/VrGUI/Projects/Android/jni)
$(call import-module,VrAppSupport/VrLocale/Projects/Android/jni)
$(call import-module,VrAppSupport/VrModel/Projects/Android/jni)
$(call import-module,VrAppSupport/VrSound/Projects/Android/jni)
