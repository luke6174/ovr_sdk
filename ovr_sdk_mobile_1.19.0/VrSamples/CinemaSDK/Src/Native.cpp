/************************************************************************************

Filename    :   Native.cpp
Content     :
Created     :	6/20/2014
Authors     :   Jim Dosé

Copyright   :   Copyright (c) Facebook Technologies, LLC and its affiliates. All rights reserved.

This source code is licensed under the BSD-style license found in the
LICENSE file in the Cinema/ directory. An additional grant 
of patent rights can be found in the PATENTS file in the same directory.

*************************************************************************************/

#include "CinemaApp.h"
#include "Native.h"

#if defined( OVR_OS_ANDROID )
extern "C" {

long Java_com_oculus_cinemasdk_MainActivity_nativeSetAppInterface( JNIEnv *jni, jclass clazz, jobject activity,
		jstring fromPackageName, jstring commandString, jstring uriString )
{
	OVR_LOG( "nativeSetAppInterface" );
	return (new OculusCinema::CinemaApp())->SetActivity( jni, clazz, activity, fromPackageName, commandString, uriString );
}

void Java_com_oculus_cinemasdk_MainActivity_nativeSetVideoSize( JNIEnv *jni, jclass clazz, jlong interfacePtr, int width, int height, int rotation, int duration )
{
	OVR_LOG( "nativeSetVideoSizes: width=%i height=%i rotation=%i duration=%i", width, height, rotation, duration );

	OculusCinema::CinemaApp * cinema = static_cast< OculusCinema::CinemaApp * >( ( (App *)interfacePtr )->GetAppInterface() );
	cinema->GetMessageQueue().PostPrintf( "video %i %i %i %i", width, height, rotation, duration );
}

jobject Java_com_oculus_cinemasdk_MainActivity_nativePrepareNewVideo( JNIEnv *jni, jclass clazz, jlong interfacePtr )
{
	OculusCinema::CinemaApp * cinema = static_cast< OculusCinema::CinemaApp * >( ( (App *)interfacePtr )->GetAppInterface() );

	// set up a message queue to get the return message
	// TODO: make a class that encapsulates this work
	ovrMessageQueue result( 1 );
	cinema->GetMessageQueue().PostPrintf( "newVideo %p", &result );

	result.SleepUntilMessage();
	const char * msg = result.GetNextMessage();
	jobject	texobj;
	sscanf( msg, "surfaceTexture %p", &texobj );
	free( (void *)msg );

	return texobj;
}

}	// extern "C"
#endif

//==============================================================

namespace OculusCinema
{

// Java method ids
static jmethodID 	getExternalCacheDirectoryMethodId = NULL;
static jmethodID	createVideoThumbnailMethodId = NULL;
static jmethodID	checkForMovieResumeId = NULL;
static jmethodID 	isPlayingMethodId = NULL;
static jmethodID 	isPlaybackFinishedMethodId = NULL;
static jmethodID 	hadPlaybackErrorMethodId = NULL;
static jmethodID	getPositionMethodId = NULL;
static jmethodID	getDurationMethodId = NULL;
static jmethodID	setPositionMethodId = NULL;
static jmethodID 	seekDeltaMethodId = NULL;
static jmethodID 	startMovieMethodId = NULL;
static jmethodID 	pauseMovieMethodId = NULL;
static jmethodID 	resumeMovieMethodId = NULL;
static jmethodID 	stopMovieMethodId = NULL;
static jmethodID 	togglePlayingMethodId = NULL;

#if defined( OVR_OS_ANDROID )
// Error checks and exits on failure
static jmethodID GetMethodID( App * app, jclass cls, const char * name, const char * signature )
{
	jmethodID mid = app->GetJava()->Env->GetMethodID( cls, name, signature );
	if ( !mid )
	{
    	OVR_FAIL( "Couldn't find %s methodID", name );
    }

	return mid;
}
#endif

void Native::OneTimeInit( App *app, jclass mainActivityClass )
{
	OVR_LOG( "Native::OneTimeInit" );

	const double start = SystemClock::GetTimeInSeconds();

#if defined( OVR_OS_ANDROID )
	getExternalCacheDirectoryMethodId 	= GetMethodID( app, mainActivityClass, "getExternalCacheDirectory", "()Ljava/lang/String;" );
	createVideoThumbnailMethodId 		= GetMethodID( app, mainActivityClass, "createVideoThumbnail", "(Ljava/lang/String;Ljava/lang/String;II)Z" );
	checkForMovieResumeId 				= GetMethodID( app, mainActivityClass, "checkForMovieResume", "(Ljava/lang/String;)Z" );
	isPlayingMethodId 					= GetMethodID( app, mainActivityClass, "isPlaying", "()Z" );
	isPlaybackFinishedMethodId			= GetMethodID( app, mainActivityClass, "isPlaybackFinished", "()Z" );
	hadPlaybackErrorMethodId			= GetMethodID( app, mainActivityClass, "hadPlaybackError", "()Z" );
	getPositionMethodId 				= GetMethodID( app, mainActivityClass, "getPosition", "()I" );
	getDurationMethodId 				= GetMethodID( app, mainActivityClass, "getDuration", "()I" );
	setPositionMethodId 				= GetMethodID( app, mainActivityClass, "setPosition", "(I)V" );
	seekDeltaMethodId					= GetMethodID( app, mainActivityClass, "seekDelta", "(I)V" );
	startMovieMethodId 					= GetMethodID( app, mainActivityClass, "startMovie", "(Ljava/lang/String;ZZZ)V" );
	pauseMovieMethodId 					= GetMethodID( app, mainActivityClass, "pauseMovie", "()V" );
	resumeMovieMethodId 				= GetMethodID( app, mainActivityClass, "resumeMovie", "()V" );
	stopMovieMethodId 					= GetMethodID( app, mainActivityClass, "stopMovie", "()V" );
	togglePlayingMethodId 				= GetMethodID( app, mainActivityClass, "togglePlaying", "()Z" );
#endif
	OVR_LOG( "Native::OneTimeInit: %3.1f seconds", SystemClock::GetTimeInSeconds() - start );
}

void Native::OneTimeShutdown()
{
	OVR_LOG( "Native::OneTimeShutdown" );
}

String Native::GetExternalCacheDirectory( App * app )
{
#if defined( OVR_OS_ANDROID )
	jstring externalCacheDirectoryString = (jstring)app->GetJava()->Env->CallObjectMethod( app->GetJava()->ActivityObject, getExternalCacheDirectoryMethodId );

	const char *externalCacheDirectoryStringUTFChars = app->GetJava()->Env->GetStringUTFChars( externalCacheDirectoryString, NULL );
	String externalCacheDirectory = externalCacheDirectoryStringUTFChars;

	app->GetJava()->Env->ReleaseStringUTFChars( externalCacheDirectoryString, externalCacheDirectoryStringUTFChars );
	app->GetJava()->Env->DeleteLocalRef( externalCacheDirectoryString );

	return externalCacheDirectory;
#else
	return String();
#endif
}

bool Native::CreateVideoThumbnail( App *app, const char *videoFilePath, const char *outputFilePath, const int width, const int height )
{
	OVR_LOG( "CreateVideoThumbnail( %s, %s )", videoFilePath, outputFilePath );
#if defined( OVR_OS_ANDROID )
	jstring jstrVideoFilePath = app->GetJava()->Env->NewStringUTF( videoFilePath );
	jstring jstrOutputFilePath = app->GetJava()->Env->NewStringUTF( outputFilePath );

	jboolean result = app->GetJava()->Env->CallBooleanMethod( app->GetJava()->ActivityObject, createVideoThumbnailMethodId, jstrVideoFilePath, jstrOutputFilePath, width, height );

	app->GetJava()->Env->DeleteLocalRef( jstrVideoFilePath );
	app->GetJava()->Env->DeleteLocalRef( jstrOutputFilePath );

	OVR_LOG( "CreateVideoThumbnail( %s, %s )", videoFilePath, outputFilePath );

	return result;
#else
	return false;
#endif
}

bool Native::CheckForMovieResume( App *app, const char * movieName )
{
	OVR_LOG( "CheckForMovieResume( %s )", movieName );
#if defined( OVR_OS_ANDROID )
	jstring jstrMovieName = app->GetJava()->Env->NewStringUTF( movieName );
	jboolean result = app->GetJava()->Env->CallBooleanMethod( app->GetJava()->ActivityObject, checkForMovieResumeId, jstrMovieName );
	app->GetJava()->Env->DeleteLocalRef( jstrMovieName );

	return result;
#else
	return false;
#endif
}

bool Native::IsPlaying( App * app )
{
	OVR_LOG( "IsPlaying()" );
#if defined( OVR_OS_ANDROID )
	return app->GetJava()->Env->CallBooleanMethod( app->GetJava()->ActivityObject, isPlayingMethodId );
#else
	return false;
#endif
}

bool Native::IsPlaybackFinished( App * app )
{
#if defined( OVR_OS_ANDROID )
	jboolean result = app->GetJava()->Env->CallBooleanMethod( app->GetJava()->ActivityObject, isPlaybackFinishedMethodId );
	return ( result != 0 );
#else
	return false;
#endif
}

bool Native::HadPlaybackError( App * app )
{
#if defined( OVR_OS_ANDROID )
	jboolean result = app->GetJava()->Env->CallBooleanMethod( app->GetJava()->ActivityObject, hadPlaybackErrorMethodId );
	return ( result != 0 );
#else
	return false;
#endif
}

int Native::GetPosition( App * app )
{
	//OVR_LOG( "GetPosition()" );
#if defined( OVR_OS_ANDROID )
	return app->GetJava()->Env->CallIntMethod( app->GetJava()->ActivityObject, getPositionMethodId );
#else
	return 0;
#endif
}

int Native::GetDuration( App * app )
{
	OVR_LOG( "GetDuration()" );
#if defined( OVR_OS_ANDROID )
	return app->GetJava()->Env->CallIntMethod( app->GetJava()->ActivityObject, getDurationMethodId );
#else
	return 0;
#endif
}

void Native::SetPosition( App * app, int positionMS )
{
	OVR_LOG( "SetPosition()" );
#if defined( OVR_OS_ANDROID )
	app->GetJava()->Env->CallVoidMethod( app->GetJava()->ActivityObject, setPositionMethodId, positionMS );
#endif
}

void Native::SeekDelta( App * app, int deltaMS )
{
	OVR_LOG( "SeekDelta()" );
#if defined( OVR_OS_ANDROID )
	app->GetJava()->Env->CallVoidMethod( app->GetJava()->ActivityObject, seekDeltaMethodId, deltaMS );
#endif
}

void Native::StartMovie( App * app, const char * movieName, bool resumePlayback, bool isEncrypted, bool loop )
{
	OVR_LOG( "StartMovie( %s )", movieName );
#if defined( OVR_OS_ANDROID )
	jstring jstrMovieName = app->GetJava()->Env->NewStringUTF( movieName );
	app->GetJava()->Env->CallVoidMethod( app->GetJava()->ActivityObject, startMovieMethodId, jstrMovieName, resumePlayback, isEncrypted, loop );
	app->GetJava()->Env->DeleteLocalRef( jstrMovieName );
#endif
}

void Native::PauseMovie( App * app )
{
	OVR_LOG( "PauseMovie()" );
#if defined( OVR_OS_ANDROID )
	app->GetJava()->Env->CallVoidMethod( app->GetJava()->ActivityObject, pauseMovieMethodId );
#endif
}

void Native::ResumeMovie( App * app )
{
	OVR_LOG( "ResumeMovie()" );
#if defined( OVR_OS_ANDROID )
	app->GetJava()->Env->CallVoidMethod( app->GetJava()->ActivityObject, resumeMovieMethodId );
#endif
}

void Native::StopMovie( App * app )
{
	OVR_LOG( "StopMovie()" );
#if defined( OVR_OS_ANDROID )
	app->GetJava()->Env->CallVoidMethod( app->GetJava()->ActivityObject, stopMovieMethodId );
#endif
}

bool Native::TogglePlaying( App * app )
{
	OVR_LOG( "TogglePlaying()" );
#if defined( OVR_OS_ANDROID )
	return app->GetJava()->Env->CallBooleanMethod( app->GetJava()->ActivityObject, togglePlayingMethodId );
#else
	return false;
#endif
}

} // namespace OculusCinema
