/************************************************************************************

Filename    :   CinemaApp.h
Content     :   
Created     :	6/17/2014
Authors     :   Jim Dosé

Copyright   :   Copyright (c) Facebook Technologies, LLC and its affiliates. All rights reserved.

This source code is licensed under the BSD-style license found in the
LICENSE file in the Cinema/ directory. An additional grant 
of patent rights can be found in the PATENTS file in the same directory.

*************************************************************************************/

#include "App.h"
#include "ShaderManager.h"
#include "ModelManager.h"
#include "SceneManager.h"
#include "ViewManager.h"
#include "MovieManager.h"
#include "MoviePlayerView.h"
#include "MovieSelectionView.h"
#include "TheaterSelectionView.h"
#include "ResumeMovieView.h"
#include "GuiSys.h"
#include "SoundEffectContext.h"
#include <memory>

using namespace OVR;

namespace OVR {
	class ovrLocale;
}

namespace OculusCinema {

class ovrCinemaStrings;

class CinemaApp : public OVR::VrAppInterface
{
public:
							CinemaApp();
	virtual					~CinemaApp();

	virtual void			Configure( ovrSettings & settings );
	virtual void			EnteredVrMode( const ovrIntentType intentType, const char * intentFromPackage, const char * intentJSON, const char * intentURI );
	virtual void			LeavingVrMode();
	virtual bool 			OnKeyEvent( const int keyCode, const int repeatCount, const KeyEventType eventType );
	virtual ovrFrameResult	Frame( const ovrFrameInput & vrFrame );

	OvrGuiSys &				GetGuiSys() { return *GuiSys; }
	ovrLocale &				GetLocale() { return *Locale; }
	ovrMessageQueue &		GetMessageQueue() { return MessageQueue; }

	const ovrFrameInput &	GetFrame() const { return VrFrame; }
	ovrFrameResult &		GetFrameResult() { return FrameResult; }

	void			    	SetPlaylist( const Array<const MovieDef *> &playList, const int nextMovie );
	void			    	SetMovie( const MovieDef * nextMovie );
	void 					MovieLoaded( const int width, const int height, const int duration );

	const MovieDef *		GetCurrentMovie() const { return CurrentMovie; }
	const MovieDef *		GetNextMovie() const;
	const MovieDef *		GetPreviousMovie() const;

	const SceneDef & 		GetCurrentTheater() const;

	void 					StartMoviePlayback();
	void 					ResumeMovieFromSavedLocation();
	void					PlayMovieFromBeginning();
	void 					ResumeOrRestartMovie();
	void 					TheaterSelection();
	void 					MovieSelection( bool inLobby );
	void					MovieFinished();
	void					UnableToPlayMovie();

	bool 					AllowTheaterSelection() const;
	bool 					IsMovieFinished() const;

	const char *			RetailDir( const char *dir ) const;
	const char *			ExternalRetailDir( const char *dir ) const;
	const char *			SDCardDir( const char *dir ) const;
	const char * 			ExternalSDCardDir( const char *dir ) const;
	const char * 			ExternalCacheDir( const char *dir ) const;
	bool 					IsExternalSDCardDir( const char *dir ) const;
	bool 					FileExists( const char *filename ) const;

	bool					HeadsetWasMounted() const { return ( MountState == true ) && ( LastMountState == false ); }
	bool					HeadsetWasUnmounted() const { return ( MountState == false ) && ( LastMountState == true ); }
	bool					HeadsetMountStateChanged() const { return ( MountState != LastMountState ); }
	bool					HeadsetMountState() const { return MountState; }

	bool					GetUseSrgb() const;

	ovrSoundEffectContext & GetSoundEffectContext() { return *SoundEffectContext; }
	ovrCinemaStrings &		GetCinemaStrings() const;

public:
	OvrGuiSys *				GuiSys;
	ovrLocale *				Locale;
	ovrCinemaStrings *		CinemaStrings;
	double					StartTime;

	jclass					MainActivityClass;	// need to look up from main thread

	SceneManager			SceneMgr;
	ShaderManager 			ShaderMgr;
	ModelManager 			ModelMgr;
	MovieManager 			MovieMgr;

	bool					InLobby;
	bool					AllowDebugControls;

private:
	ovrSoundEffectContext * SoundEffectContext;
	OvrGuiSys::SoundEffectPlayer * SoundEffectPlayer;

	ovrFrameInput			VrFrame;
	ovrFrameResult			FrameResult;

	ViewManager				ViewMgr;
	MoviePlayerView			MoviePlayer;
	MovieSelectionView		MovieSelectionMenu;
	TheaterSelectionView	TheaterSelectionMenu;
	ResumeMovieView			ResumeMovieMenu;

	ovrMessageQueue			MessageQueue;

	int						FrameCount;

	const MovieDef *		CurrentMovie;
	Array<const MovieDef *> PlayList;

	bool					ShouldResumeMovie;
	bool					MovieFinishedPlaying;

	bool					MountState;
	bool					LastMountState;

	bool					UseSrgb;

private:
	void 					Command( const char * msg );
};

} // namespace OculusCinema
