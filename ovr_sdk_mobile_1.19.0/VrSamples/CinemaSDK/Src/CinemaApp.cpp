/************************************************************************************

Filename    :   CinemaApp.cpp
Content     :   
Created     :	6/17/2014
Authors     :   Jim Dosé

Copyright   :   Copyright (c) Facebook Technologies, LLC and its affiliates. All rights reserved.

This source code is licensed under the BSD-style license found in the
LICENSE file in the Cinema/ directory. An additional grant 
of patent rights can be found in the PATENTS file in the same directory.

*************************************************************************************/

#include "Kernel/OVR_String_Utils.h"
#include "CinemaApp.h"
#include "Native.h"
#include "CinemaStrings.h"
#include "OVR_Locale.h"

//=======================================================================================

namespace OculusCinema {

//==============================================================
// ovrGuiSoundEffectPlayer
class ovrGuiSoundEffectPlayer : public OvrGuiSys::SoundEffectPlayer
{
public:
	ovrGuiSoundEffectPlayer( ovrSoundEffectContext & context )
		: SoundEffectContext( context )
	{
	}

	virtual bool Has( const char * name ) const OVR_OVERRIDE { return SoundEffectContext.GetMapping().HasSound( name ); }
	virtual void Play( const char * name ) OVR_OVERRIDE { SoundEffectContext.Play( name ); }

private:
	ovrGuiSoundEffectPlayer  &	operator=( const ovrGuiSoundEffectPlayer & );
	ovrSoundEffectContext & SoundEffectContext;
};

CinemaApp::CinemaApp() :
	GuiSys( OvrGuiSys::Create() ),
	Locale( NULL ),
	CinemaStrings( NULL ),
	StartTime( 0 ),
	SceneMgr( *this ),
	ShaderMgr( *this ),
	ModelMgr( *this ),
	MovieMgr( *this ),
	InLobby( true ),
	AllowDebugControls( false ),
	SoundEffectContext( NULL ),
	SoundEffectPlayer( NULL ),
	VrFrame(),
	ViewMgr(),
	MoviePlayer( *this ),
	MovieSelectionMenu( *this ),
	TheaterSelectionMenu( *this ),
	ResumeMovieMenu( *this ),
	MessageQueue( 100 ),
	FrameCount( 0 ),
	CurrentMovie( NULL ),
	PlayList(),
	ShouldResumeMovie( false ),
	MovieFinishedPlaying( false ),
	MountState( true ),			// We assume that the device is mounted at start since we can only detect changes in mount state
	LastMountState( true ),
	UseSrgb( false )
{
}

CinemaApp::~CinemaApp()
{
	OVR_LOG( "--------------- ~CinemaApp() ---------------");

	delete SoundEffectPlayer;
	SoundEffectPlayer = NULL;

	delete SoundEffectContext;
	SoundEffectContext = NULL;

	Native::OneTimeShutdown();
	ShaderMgr.OneTimeShutdown();
	ModelMgr.OneTimeShutdown();
	SceneMgr.OneTimeShutdown();
	MovieMgr.OneTimeShutdown();
	MoviePlayer.OneTimeShutdown();
	MovieSelectionMenu.OneTimeShutdown();
	TheaterSelectionMenu.OneTimeShutdown();
	ResumeMovieMenu.OneTimeShutdown();
	ovrCinemaStrings::Destroy( *this, CinemaStrings );

	OvrGuiSys::Destroy( GuiSys );
}

void CinemaApp::Configure( ovrSettings & settings )
{
	// We need very little CPU for movie playing, but a fair amount of GPU.
	// The CPU clock should ramp up above the minimum when necessary.
	settings.CpuLevel = 1;
	settings.GpuLevel = 2;

	settings.UseSrgbFramebuffer = UseSrgb;

	// Default to 2x MSAA.
	settings.EyeBufferParms.colorFormat = COLOR_8888;
	//settings.EyeBufferParms.depthFormat = DEPTH_16;
	settings.EyeBufferParms.multisamples = 2;

	settings.RenderMode = RENDERMODE_MULTIVIEW;
}

void CinemaApp::EnteredVrMode( const ovrIntentType intentType, const char * intentFromPackage, const char * intentJSON, const char * intentURI )
{
	OVR_UNUSED( intentFromPackage );
	OVR_UNUSED( intentJSON );

	if ( intentType == INTENT_LAUNCH )
	{
		OVR_LOG( "--------------- CinemaApp OneTimeInit ---------------");

		const ovrJava * java = app->GetJava();
		SoundEffectContext = new ovrSoundEffectContext( *java->Env, java->ActivityObject );
		SoundEffectContext->Initialize( &app->GetFileSys() );
		SoundEffectPlayer = new ovrGuiSoundEffectPlayer( *SoundEffectContext );

		Locale = ovrLocale::Create( *java->Env, java->ActivityObject, "default", &app->GetFileSys() );

		String fontName;
		GetLocale().GetString( "@string/font_name", "efigs.fnt", fontName );
		GuiSys->Init( this->app, *SoundEffectPlayer, fontName.ToCStr(), &app->GetDebugLines() );

		GuiSys->GetGazeCursor().ShowCursor();
	
		StartTime = SystemClock::GetTimeInSeconds();

		Native::OneTimeInit( app, ActivityClass );

		CinemaStrings = ovrCinemaStrings::Create( *this );

		ShaderMgr.OneTimeInit( intentURI );
		ModelMgr.OneTimeInit( intentURI );
		SceneMgr.OneTimeInit( intentURI );
		MovieMgr.OneTimeInit( intentURI );
		MoviePlayer.OneTimeInit( intentURI );
		MovieSelectionMenu.OneTimeInit( intentURI );
		TheaterSelectionMenu.OneTimeInit( intentURI );
		ResumeMovieMenu.OneTimeInit( intentURI );

		ViewMgr.AddView( &MoviePlayer );
		ViewMgr.AddView( &MovieSelectionMenu );
		ViewMgr.AddView( &TheaterSelectionMenu );

		MovieSelection( true );

		OVR_LOG( "CinemaApp::OneTimeInit: %3.1f seconds", SystemClock::GetTimeInSeconds() - StartTime );
	}
	else if ( intentType == INTENT_NEW )
	{
	}

	OVR_LOG( "CinemaApp::EnteredVrMode" );
	// Clear cursor trails.
	GetGuiSys().GetGazeCursor().HideCursorForFrames( 10 );	
	ViewMgr.EnteredVrMode();
}

void CinemaApp::LeavingVrMode()
{
	OVR_LOG( "CinemaApp::LeavingVrMode" );
	ViewMgr.LeavingVrMode();
}

const char * CinemaApp::RetailDir( const char *dir ) const
{
	static char subDir[ 256 ];
	StringUtils::SPrintf( subDir, "%s/%s", SDCardDir( "RetailMedia" ), dir );
	return subDir;
}

const char * CinemaApp::ExternalRetailDir( const char *dir ) const
{
	static char subDir[ 256 ];
	StringUtils::SPrintf( subDir, "%s/%s", ExternalSDCardDir( "RetailMedia" ), dir );
	return subDir;
}

const char * CinemaApp::SDCardDir( const char *dir ) const
{
	static char subDir[256];
	String sdcardPath;
	const OvrStoragePaths & storagePaths = app->GetStoragePaths();
	storagePaths.GetPathIfValidPermission( EST_PRIMARY_EXTERNAL_STORAGE, EFT_ROOT, "", permissionFlags_t( PERMISSION_READ ), sdcardPath );
	StringUtils::SPrintf( subDir, "%s%s", sdcardPath.ToCStr(), dir );
	return subDir;
}

const char * CinemaApp::ExternalSDCardDir( const char *dir ) const
{
	static char subDir[256];
	String externalSdcardPath;
	const OvrStoragePaths & storagePaths = app->GetStoragePaths();
	storagePaths.GetPathIfValidPermission( EST_SECONDARY_EXTERNAL_STORAGE, EFT_ROOT, "", permissionFlags_t( PERMISSION_READ ), externalSdcardPath );
	StringUtils::SPrintf( subDir, "%s%s", externalSdcardPath.ToCStr(), dir );
	return subDir;
}

const char * CinemaApp::ExternalCacheDir( const char *dir ) const
{
	static char subDir[ 256 ];
	StringUtils::SPrintf( subDir, "%s/%s", Native::GetExternalCacheDirectory( app ).ToCStr(), dir );
	return subDir;
}

bool CinemaApp::IsExternalSDCardDir( const char *dir ) const
{
	const char * sdcardDir = ExternalSDCardDir( "" );
	const size_t l = strlen( sdcardDir );
	return ( 0 == strncmp( sdcardDir, dir, l ) );
}

bool CinemaApp::FileExists( const char * filename ) const
{
	FILE * f = fopen( filename, "r" );
	if ( f == NULL )
	{
		return false;
	}
	else
	{
		fclose( f );
		return true;
	}
}

bool CinemaApp::GetUseSrgb() const
{
	return UseSrgb && app->GetFramebufferIsSrgb();
}

void CinemaApp::SetPlaylist( const Array<const MovieDef *> &playList, const int nextMovie )
{
	PlayList = playList;

	OVR_ASSERT( nextMovie < PlayList.GetSizeI() );
	SetMovie( PlayList[ nextMovie ] );
}

void CinemaApp::SetMovie( const MovieDef *movie )
{
	OVR_LOG( "SetMovie( %s )", movie->Filename.ToCStr() );
	CurrentMovie = movie;
	MovieFinishedPlaying = false;
}

void CinemaApp::MovieLoaded( const int width, const int height, const int duration )
{
	MoviePlayer.MovieLoaded( width, height, duration );
}

const MovieDef * CinemaApp::GetNextMovie() const
{
	const MovieDef *next = NULL;
	if ( PlayList.GetSizeI() != 0 )
	{
		for ( int i = 0; i < PlayList.GetSizeI() - 1; i++ )
		{
			if ( PlayList[ i ] == CurrentMovie )
			{
				next = PlayList[ i + 1 ];
				break;
			}
		}

		if ( next == NULL )
		{
			next = PlayList[ 0 ];
		}
	}

	return next;
}

const MovieDef * CinemaApp::GetPreviousMovie() const
{
	const MovieDef *previous = NULL;
	if ( PlayList.GetSizeI() != 0 )
	{
		for( int i = 0; i < PlayList.GetSizeI(); i++ )
		{
			if ( PlayList[ i ] == CurrentMovie )
			{
				break;
			}
			previous = PlayList[ i ];
		}

		if ( previous == NULL )
		{
			previous = PlayList[ PlayList.GetSizeI() - 1 ];
		}
	}

	return previous;
}

void CinemaApp::StartMoviePlayback()
{
	if ( CurrentMovie != NULL )
	{
		MovieFinishedPlaying = false;
		Native::StartMovie( app, CurrentMovie->Filename.ToCStr(), ShouldResumeMovie, CurrentMovie->IsEncrypted, false );
		ShouldResumeMovie = false;
	}
}

void CinemaApp::ResumeMovieFromSavedLocation()
{
	OVR_LOG( "ResumeMovie");
	InLobby = false;
	ShouldResumeMovie = true;
	ViewMgr.OpenView( MoviePlayer );
}

void CinemaApp::PlayMovieFromBeginning()
{
	OVR_LOG( "PlayMovieFromBeginning");
	InLobby = false;
	ShouldResumeMovie = false;
	ViewMgr.OpenView( MoviePlayer );
}

void CinemaApp::ResumeOrRestartMovie()
{
	OVR_LOG( "StartMovie");
	if ( Native::CheckForMovieResume( app, CurrentMovie->Filename.ToCStr() ) )
	{
		OVR_LOG( "Open ResumeMovieMenu");
		ViewMgr.OpenView( ResumeMovieMenu );
	}
	else
	{
		PlayMovieFromBeginning();
	}
}

void CinemaApp::MovieFinished()
{
	InLobby = false;
	MovieFinishedPlaying = true;
	MovieSelectionMenu.SetMovieList( PlayList, GetNextMovie() );
	ViewMgr.OpenView( MovieSelectionMenu );
}

void CinemaApp::UnableToPlayMovie()
{
	InLobby = false;
	MovieSelectionMenu.SetError( GetCinemaStrings().Error_UnableToPlayMovie.ToCStr(), false, true );
	ViewMgr.OpenView( MovieSelectionMenu );
}

void CinemaApp::TheaterSelection()
{
	ViewMgr.OpenView( TheaterSelectionMenu );
}

void CinemaApp::MovieSelection( bool inLobby )
{
	InLobby = inLobby;
	ViewMgr.OpenView( MovieSelectionMenu );
}

bool CinemaApp::AllowTheaterSelection() const
{
	if ( CurrentMovie != NULL )
	{
		return CurrentMovie->AllowTheaterSelection;
	}

	return true;
}

bool CinemaApp::IsMovieFinished() const
{
	return MovieFinishedPlaying;
}


const SceneDef & CinemaApp::GetCurrentTheater() const
{
	return ModelMgr.GetTheater( TheaterSelectionMenu.GetSelectedTheater() );
}

bool CinemaApp::OnKeyEvent( const int keyCode, const int repeatCount, const KeyEventType eventType )
{
	if ( GuiSys->OnKeyEvent( keyCode, repeatCount, eventType ) )
	{
		return true;
	}

	return ViewMgr.OnKeyEvent( keyCode, repeatCount, eventType );
}

void CinemaApp::Command( const char * msg )
{
	if ( SceneMgr.Command( msg ) )
	{
		return;
	}
}

ovrFrameResult CinemaApp::Frame( const ovrFrameInput & vrFrame )
{
	FrameResult = ovrFrameResult();

	// process input events first because this mirrors the behavior when OnKeyEvent was
	// a virtual function on VrAppInterface and was called by VrAppFramework.
	for ( int i = 0; i < vrFrame.Input.NumKeyEvents; i++ )
	{
		const int keyCode = vrFrame.Input.KeyEvents[i].KeyCode;
		const int repeatCount = vrFrame.Input.KeyEvents[i].RepeatCount;
		const KeyEventType eventType = vrFrame.Input.KeyEvents[i].EventType;

		if ( OnKeyEvent( keyCode, repeatCount, eventType ) )
		{
			continue;   // consumed the event
		}
		// If nothing consumed the key and it's a short-press of the back key, then exit the application to OculusHome.
		if ( keyCode == OVR_KEY_BACK && eventType == KEY_EVENT_SHORT_PRESS )
		{
			app->ShowSystemUI( VRAPI_SYS_UI_CONFIRM_QUIT_MENU );
			continue;
		}           
	}
	// Process incoming messages until the queue is empty.
	for ( ; ; )
	{
		const char * msg = MessageQueue.GetNextMessage();
		if ( msg == NULL )
		{
			break;
		}
		Command( msg );
		free( (void *)msg );
	}

	FrameCount++;
	VrFrame = vrFrame;

	// update mount state
	LastMountState = MountState;
	MountState = vrFrame.DeviceStatus.HeadsetIsMounted;
	if ( HeadsetWasMounted() )
	{
		OVR_LOG( "Headset mounted" );
	}
	else if ( HeadsetWasUnmounted() )
	{
		OVR_LOG( "Headset unmounted" );
	}

	// The View handles setting the FrameResult and Parms.
	ViewMgr.Frame( vrFrame );

	// Update gui systems after the app frame, but before rendering anything.
	GuiSys->Frame( vrFrame, FrameResult.FrameMatrices.CenterView );

	//-------------------------------
	// Rendering
	//-------------------------------

	GuiSys->AppendSurfaceList( FrameResult.FrameMatrices.CenterView, &FrameResult.Surfaces );

	return FrameResult;
}

ovrCinemaStrings & CinemaApp::GetCinemaStrings() const
{
	return *CinemaStrings;
}

} // namespace OculusCinema

