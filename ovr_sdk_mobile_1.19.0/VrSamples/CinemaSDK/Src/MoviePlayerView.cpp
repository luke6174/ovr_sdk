/************************************************************************************

Filename    :   MoviePlayerView.cpp
Content     :
Created     :	6/17/2014
Authors     :   Jim Dosé

Copyright   :   Copyright (c) Facebook Technologies, LLC and its affiliates. All rights reserved.

This source code is licensed under the BSD-style license found in the
LICENSE file in the Cinema/ directory. An additional grant 
of patent rights can be found in the PATENTS file in the same directory.

*************************************************************************************/

#include "OVR_Input.h"
#include "CinemaApp.h"
#include "Native.h"
#include "VRMenuMgr.h"
#include "GuiSys.h"
#include "Kernel/OVR_String_Utils.h"

#include "CinemaStrings.h"

namespace OculusCinema
{

const int MoviePlayerView::MaxSeekSpeed = 5;
const int MoviePlayerView::ScrubBarWidth = 516;

const double MoviePlayerView::GazeTimeTimeout = 4;

MoviePlayerView::MoviePlayerView( CinemaApp &cinema ) :
	View( "MoviePlayerView" ),
	Cinema( cinema ),
	uiActive( false ),
	RepositionScreen( false ),
	SeekSpeed( 0 ),
	PlaybackPos( 0 ),
	NextSeekTime( 0 ),
	BackgroundTintTexture(),
	RWTexture(),
	RWHoverTexture(),
	RWPressedTexture(),
	FFTexture(),
	FFHoverTexture(),
	FFPressedTexture(),
	PlayTexture(),
	PlayHoverTexture(),
	PlayPressedTexture(),
	PauseTexture(),
	PauseHoverTexture(),
	PausePressedTexture(),
	CarouselTexture(),
	CarouselHoverTexture(),
	CarouselPressedTexture(),
	SeekbarBackgroundTexture(),
	SeekbarProgressTexture(),
	SeekPosition(),
	SeekFF2x(),
	SeekFF4x(),
	SeekFF8x(),
	SeekFF16x(),
	SeekFF32x(),
	SeekRW2x(),
	SeekRW4x(),
	SeekRW8x(),
	SeekRW16x(),
	SeekRW32x(),
	MoveScreenMenu( NULL ),
	MoveScreenLabel( Cinema.GetGuiSys() ),
	MoveScreenAlpha(),
	PlaybackControlsMenu( NULL ),
	PlaybackControlsPosition( Cinema.GetGuiSys() ),
	PlaybackControlsScale( Cinema.GetGuiSys() ),
	MovieTitleLabel( Cinema.GetGuiSys() ),
	SeekIcon( Cinema.GetGuiSys() ),
	ControlsBackground( Cinema.GetGuiSys() ),
	GazeTimer(),
	RewindButton( Cinema.GetGuiSys() ),
	PlayButton( Cinema.GetGuiSys() ),
	FastForwardButton( Cinema.GetGuiSys() ),
	CarouselButton( Cinema.GetGuiSys() ),
	SeekbarBackground( Cinema.GetGuiSys() ),
	SeekbarProgress( Cinema.GetGuiSys() ),
	ScrubBar(),
	CurrentTime( Cinema.GetGuiSys() ),
	SeekTime( Cinema.GetGuiSys() ),
	BackgroundClicked( false ),
	UIOpened( false )

{
}

MoviePlayerView::~MoviePlayerView()
{
}

//=========================================================================================

void MoviePlayerView::OneTimeInit( const char * launchIntent )
{
	OVR_LOG( "MoviePlayerView::OneTimeInit" );

	OVR_UNUSED( launchIntent );

	const double start =  SystemClock::GetTimeInSeconds();

	CreateMenu( Cinema.GetGuiSys() );

	OVR_LOG( "MoviePlayerView::OneTimeInit: %3.1f seconds", SystemClock::GetTimeInSeconds() - start );
}

void MoviePlayerView::OneTimeShutdown()
{
	OVR_LOG( "MoviePlayerView::OneTimeShutdown" );
}

float PixelScale( const float x )
{
	return x * VRMenuObject::DEFAULT_TEXEL_SCALE;
}

Vector3f PixelPos( const float x, const float y, const float z )
{
	return Vector3f( PixelScale( x ), PixelScale( y ), PixelScale( z ) );
}

void PlayPressedCallback( UIButton *button, void *object )
{
	OVR_UNUSED( button );
	( ( MoviePlayerView * )object )->TogglePlayback();
}

void RewindPressedCallback( UIButton *button, void *object )
{
	OVR_UNUSED( button );
	( ( MoviePlayerView * )object )->RewindPressed();
}

void FastForwardPressedCallback( UIButton *button, void *object )
{
	OVR_UNUSED( button );
	( ( MoviePlayerView * )object )->FastForwardPressed();
}

void CarouselPressedCallback( UIButton *button, void *object )
{
	OVR_UNUSED( button );
	( ( MoviePlayerView * )object )->CarouselPressed();
}

void ScrubBarCallback( ScrubBarComponent *scrubbar, void *object, const float progress )
{
	OVR_UNUSED( scrubbar );
	( ( MoviePlayerView * )object )->ScrubBarClicked( progress );
}

void MoviePlayerView::CreateMenu( OvrGuiSys & guiSys )
{
	OVR_UNUSED( guiSys );

	BackgroundTintTexture.LoadTextureFromApplicationPackage( "assets/backgroundTint.png" );

	RWTexture.LoadTextureFromApplicationPackage( "assets/img_btn_rw.png" );
	RWHoverTexture.LoadTextureFromApplicationPackage( "assets/img_btn_rw_hover.png" );
	RWPressedTexture.LoadTextureFromApplicationPackage( "assets/img_btn_rw_pressed.png" );

	FFTexture.LoadTextureFromApplicationPackage( "assets/img_btn_ff.png" );
	FFHoverTexture.LoadTextureFromApplicationPackage( "assets/img_btn_ff_hover.png" );
	FFPressedTexture.LoadTextureFromApplicationPackage( "assets/img_btn_ff_pressed.png" );

	PlayTexture.LoadTextureFromApplicationPackage( "assets/img_btn_play.png" );
	PlayHoverTexture.LoadTextureFromApplicationPackage( "assets/img_btn_play_hover.png" );
	PlayPressedTexture.LoadTextureFromApplicationPackage( "assets/img_btn_play_pressed.png" );

	PauseTexture.LoadTextureFromApplicationPackage( "assets/img_btn_pause.png" );
	PauseHoverTexture.LoadTextureFromApplicationPackage( "assets/img_btn_pause_hover.png" );
	PausePressedTexture.LoadTextureFromApplicationPackage( "assets/img_btn_pause_pressed.png" );

	CarouselTexture.LoadTextureFromApplicationPackage( "assets/img_btn_carousel.png" );
	CarouselHoverTexture.LoadTextureFromApplicationPackage( "assets/img_btn_carousel_hover.png" );
	CarouselPressedTexture.LoadTextureFromApplicationPackage( "assets/img_btn_carousel_pressed.png" );

	SeekbarBackgroundTexture.LoadTextureFromApplicationPackage( "assets/img_seekbar_background.png" );
	SeekbarProgressTexture.LoadTextureFromApplicationPackage( "assets/img_seekbar_progress_blue.png" );

	SeekPosition.LoadTextureFromApplicationPackage( "assets/img_seek_position.png" );

	SeekFF2x.LoadTextureFromApplicationPackage( "assets/img_seek_ff2x.png" );
	SeekFF4x.LoadTextureFromApplicationPackage( "assets/img_seek_ff4x.png" );
	SeekFF8x.LoadTextureFromApplicationPackage( "assets/img_seek_ff8x.png" );
	SeekFF16x.LoadTextureFromApplicationPackage( "assets/img_seek_ff16x.png" );
	SeekFF32x.LoadTextureFromApplicationPackage( "assets/img_seek_ff32x.png" );

	SeekRW2x.LoadTextureFromApplicationPackage( "assets/img_seek_rw2x.png" );
	SeekRW4x.LoadTextureFromApplicationPackage( "assets/img_seek_rw4x.png" );
	SeekRW8x.LoadTextureFromApplicationPackage( "assets/img_seek_rw8x.png" );
	SeekRW16x.LoadTextureFromApplicationPackage( "assets/img_seek_rw16x.png" );
	SeekRW32x.LoadTextureFromApplicationPackage( "assets/img_seek_rw32x.png" );

    // ==============================================================================
    //
    // reorient message
    //
	MoveScreenMenu = new UIMenu( Cinema.GetGuiSys() );
	MoveScreenMenu->Create( "MoviePlayerMenu" );
	MoveScreenMenu->SetFlags( VRMenuFlags_t( VRMENU_FLAG_TRACK_GAZE ) | VRMenuFlags_t( VRMENU_FLAG_BACK_KEY_DOESNT_EXIT ) );

	MoveScreenLabel.AddToMenu( MoveScreenMenu, NULL );
    MoveScreenLabel.SetLocalPose( Quatf( Vector3f( 0.0f, 1.0f, 0.0f ), 0.0f ), Vector3f( 0.0f, 0.0f, -1.8f ) );
    MoveScreenLabel.GetMenuObject()->AddFlags( VRMenuObjectFlags_t( VRMENUOBJECT_DONT_HIT_ALL ) );
    MoveScreenLabel.SetFontScale( 0.5f );
    MoveScreenLabel.SetText( Cinema.GetCinemaStrings().MoviePlayer_Reorient );
    MoveScreenLabel.SetTextOffset( Vector3f( 0.0f, -24 * VRMenuObject::DEFAULT_TEXEL_SCALE, 0.0f ) );  // offset to be below gaze cursor
    MoveScreenLabel.SetVisible( false );

    // ==============================================================================
    //
    // Playback controls
    //
    PlaybackControlsMenu = new UIMenu( Cinema.GetGuiSys() );
    PlaybackControlsMenu->Create( "PlaybackControlsMenu" );
    PlaybackControlsMenu->SetFlags( VRMenuFlags_t( VRMENU_FLAG_BACK_KEY_DOESNT_EXIT ) );

    PlaybackControlsPosition.AddToMenu( PlaybackControlsMenu );
    PlaybackControlsScale.AddToMenu( PlaybackControlsMenu, &PlaybackControlsPosition );
    PlaybackControlsScale.SetLocalPosition( Vector3f( 0.0f, 0.0f, 0.05f ) );
    PlaybackControlsScale.SetImage( 0, SURFACE_TEXTURE_DIFFUSE, BackgroundTintTexture, 1080, 1080 );

	// ==============================================================================
    //
    // movie title
    //
    MovieTitleLabel.AddToMenu( PlaybackControlsMenu, &PlaybackControlsScale );
    MovieTitleLabel.SetLocalPosition( PixelPos( 0, 266, 0 ) );
    MovieTitleLabel.SetFontScale( 1.4f );
    MovieTitleLabel.SetText( "" );
    MovieTitleLabel.SetTextOffset( Vector3f( 0.0f, 0.0f, 0.01f ) );
    MovieTitleLabel.SetImage( 0, SURFACE_TEXTURE_DIFFUSE, BackgroundTintTexture, 320, 120 );

	// ==============================================================================
    //
    // seek icon
    //
    SeekIcon.AddToMenu( PlaybackControlsMenu, &PlaybackControlsScale );
    SeekIcon.SetLocalPosition( PixelPos( 0, 0, 0 ) );
    SeekIcon.SetLocalScale( Vector3f( 2.0f ) );
    SetSeekIcon( 0 );

    // ==============================================================================
    //
    // controls
    //
    ControlsBackground.AddToMenu( PlaybackControlsMenu, &PlaybackControlsScale );
	ControlsBackground.AddFlags( VRMENUOBJECT_RENDER_HIERARCHY_ORDER );
    ControlsBackground.SetLocalPosition( PixelPos( 0, -288, 0 ) );
    ControlsBackground.SetImage( 0, SURFACE_TEXTURE_DIFFUSE, BackgroundTintTexture, 1004, 168 );
    ControlsBackground.AddComponent( &GazeTimer );

    RewindButton.AddToMenu( PlaybackControlsMenu, &ControlsBackground );
    RewindButton.SetLocalPosition( PixelPos( -448, 0, 1 ) );
    RewindButton.SetLocalScale( Vector3f( 2.0f ) );
    RewindButton.SetButtonImages( RWTexture, RWHoverTexture, RWPressedTexture );
    RewindButton.SetOnClick( RewindPressedCallback, this );

	FastForwardButton.AddToMenu( PlaybackControlsMenu, &ControlsBackground );
	FastForwardButton.SetLocalPosition( PixelPos( -234, 0, 1 ) );
	FastForwardButton.SetLocalScale( Vector3f( 2.0f ) );
	FastForwardButton.SetButtonImages( FFTexture, FFHoverTexture, FFPressedTexture );
	FastForwardButton.SetOnClick( FastForwardPressedCallback, this );
	FastForwardButton.GetMenuObject()->SetLocalBoundsExpand( Vector3f::ZERO, PixelPos( -20, 0, 0 ) );

	// playbutton created after fast forward button to fix z issues
    PlayButton.AddToMenu( PlaybackControlsMenu, &ControlsBackground );
    PlayButton.SetLocalPosition( PixelPos( -341, 0, 2 ) );
    PlayButton.SetLocalScale( Vector3f( 2.0f ) );
    PlayButton.SetButtonImages( PauseTexture, PauseHoverTexture, PausePressedTexture );
    PlayButton.SetOnClick( PlayPressedCallback, this );

	CarouselButton.AddToMenu( PlaybackControlsMenu, &ControlsBackground );
	CarouselButton.SetLocalPosition( PixelPos( 418, 0, 1 ) );
	CarouselButton.SetLocalScale( Vector3f( 2.0f ) );
	CarouselButton.SetButtonImages( CarouselTexture, CarouselHoverTexture, CarouselPressedTexture );
	CarouselButton.SetOnClick( CarouselPressedCallback, this );
	CarouselButton.GetMenuObject()->SetLocalBoundsExpand( PixelPos( 20, 0, 0 ), Vector3f::ZERO );

	SeekbarBackground.AddToMenu( PlaybackControlsMenu, &ControlsBackground );
	SeekbarBackground.SetLocalPosition( PixelPos( 78, 0, 2 ) );
	SeekbarBackground.SetColor( Vector4f( 0.5333f, 0.5333f, 0.5333f, 1.0f ) );
	SeekbarBackground.SetImage( 0, SURFACE_TEXTURE_DIFFUSE, SeekbarBackgroundTexture, static_cast<float>( ScrubBarWidth + 6 ), 46.0f );
	SeekbarBackground.AddComponent( &ScrubBar );

	SeekbarProgress.AddToMenu( PlaybackControlsMenu, &SeekbarBackground );
	SeekbarProgress.SetLocalPosition( PixelPos( 0, 0, 1 ) );
	SeekbarProgress.SetImage( 0, SURFACE_TEXTURE_DIFFUSE, SeekbarProgressTexture, static_cast<float>( ScrubBarWidth ), 40.0f );
	SeekbarProgress.GetMenuObject()->AddFlags( VRMenuObjectFlags_t( VRMENUOBJECT_DONT_HIT_ALL ) );

	CurrentTime.AddToMenu( PlaybackControlsMenu, &SeekbarBackground );
	CurrentTime.SetLocalPosition( PixelPos( -234, 52, 2 ) );
	CurrentTime.SetLocalScale( Vector3f( 1.0f ) );
	CurrentTime.SetImage( 0, SURFACE_TEXTURE_DIFFUSE, SeekPosition );
	CurrentTime.SetText( "2:33:33" );
	CurrentTime.SetTextOffset( PixelPos( 0, 6, 1 ) );
	CurrentTime.SetFontScale( 0.71f );
	CurrentTime.SetColor( Vector4f( 0 / 255.0f, 93 / 255.0f, 219 / 255.0f, 1.0f ) );
	CurrentTime.SetTextColor( Vector4f( 1.0f, 1.0f, 1.0f, 1.0f ) );
	CurrentTime.GetMenuObject()->AddFlags( VRMenuObjectFlags_t( VRMENUOBJECT_DONT_HIT_ALL ) );

	SeekTime.AddToMenu( PlaybackControlsMenu, &SeekbarBackground );
	SeekTime.SetLocalPosition( PixelPos( -34, 52, 4 ) );
	SeekTime.SetLocalScale( Vector3f( 1.0f ) );
	SeekTime.SetImage( 0, SURFACE_TEXTURE_DIFFUSE, SeekPosition );
	SeekTime.SetText( "2:33:33" );
	SeekTime.SetTextOffset( PixelPos( 0, 6, 1 ) );
	SeekTime.SetFontScale( 0.71f );
	SeekTime.SetColor( Vector4f( 47.0f / 255.0f, 70 / 255.0f, 89 / 255.0f, 1.0f ) );
	SeekTime.SetTextColor( Vector4f( 1.0f, 1.0f, 1.0f, 1.0f ) );
	SeekTime.GetMenuObject()->AddFlags( VRMenuObjectFlags_t( VRMENUOBJECT_DONT_HIT_ALL ) );

	ScrubBar.SetWidgets( &SeekbarBackground, &SeekbarProgress, &CurrentTime, &SeekTime, ScrubBarWidth );
	ScrubBar.SetOnClick( ScrubBarCallback, this );
}

void MoviePlayerView::SetSeekIcon( const int seekSpeed )
{
	const UITexture * texture = NULL;

	switch( seekSpeed )
	{
		case -5 : texture = &SeekRW32x; break;
		case -4 : texture = &SeekRW16x; break;
		case -3 : texture = &SeekRW8x; break;
		case -2 : texture = &SeekRW4x; break;
		case -1 : texture = &SeekRW2x; break;

		default:
		case 0 : texture = NULL; break;

		case 1 : texture = &SeekFF2x; break;
		case 2 : texture = &SeekFF4x; break;
		case 3 : texture = &SeekFF8x; break;
		case 4 : texture = &SeekFF16x; break;
		case 5 : texture = &SeekFF32x; break;
	}

	if ( texture == NULL )
	{
	    SeekIcon.SetVisible( false );
	}
	else
	{
	    SeekIcon.SetVisible( true );
	    SeekIcon.SetImage( 0, SURFACE_TEXTURE_DIFFUSE, *texture );
	}
}

void MoviePlayerView::OnOpen()
{
	OVR_LOG( "OnOpen" );
	CurViewState = VIEWSTATE_OPEN;

	Cinema.SceneMgr.ClearMovie();

	SeekSpeed = 0;
	PlaybackPos = 0;
	NextSeekTime = 0;

	SetSeekIcon( SeekSpeed );

	ScrubBar.SetProgress( 0.0f );

	RepositionScreen = false;
	MoveScreenAlpha.Set( 0, 0, 0, 0.0f );

	HideUI();
	Cinema.SceneMgr.LightsOff( 1.5f );

	Cinema.StartMoviePlayback();

	MovieTitleLabel.SetText( Cinema.GetCurrentMovie()->Title );
	Bounds3f titleBounds = MovieTitleLabel.GetTextLocalBounds( Cinema.GetGuiSys().GetDefaultFont() ) * VRMenuObject::TEXELS_PER_METER;
	MovieTitleLabel.SetImage( 0, SURFACE_TEXTURE_DIFFUSE, BackgroundTintTexture, titleBounds.GetSize().x + 88, titleBounds.GetSize().y + 32 );

	PlayButton.SetButtonImages( PauseTexture, PauseHoverTexture, PausePressedTexture );
}

void MoviePlayerView::OnClose()
{
	OVR_LOG( "OnClose" );
	CurViewState = VIEWSTATE_CLOSED;
	HideUI();
	Cinema.GetGuiSys().GetGazeCursor().ShowCursor();

	if ( MoveScreenMenu->IsOpen() )
	{
		MoveScreenLabel.SetVisible( false );
		MoveScreenMenu->Close();
	}

	Cinema.SceneMgr.ClearMovie();

	if ( Cinema.SceneMgr.VoidedScene )
	{
		Cinema.SceneMgr.SetSceneModel( Cinema.GetCurrentTheater() );
		Cinema.SceneMgr.PutScreenInFront();
	}
}

void MoviePlayerView::EnteredVrMode()
{
	OVR_LOG( "EnteredVrMode" );
	Cinema.MovieSelection( false );
}

void MoviePlayerView::LeavingVrMode()
{
	OVR_LOG( "LeavingVrMode" );
	Native::StopMovie( Cinema.app );
}

void MoviePlayerView::MovieLoaded( const int width, const int height, const int duration )
{
	OVR_UNUSED2( width, height );
	ScrubBar.SetDuration( duration );
}

void MoviePlayerView::BackPressed()
{
	OVR_LOG( "BackPressed" );
	HideUI();
	if ( Cinema.AllowTheaterSelection() )
	{
		OVR_LOG( "Opening TheaterSelection" );
		Cinema.TheaterSelection();
	}
	else
	{
		OVR_LOG( "Opening MovieSelection" );
		Cinema.MovieSelection( true );
	}
}

bool MoviePlayerView::OnKeyEvent( const int keyCode, const int repeatCount, const KeyEventType eventType )
{
	OVR_UNUSED( repeatCount );

	switch ( keyCode )
	{
		case OVR_KEY_BACK:
		{
			switch ( eventType )
			{
				case KEY_EVENT_SHORT_PRESS:
				OVR_LOG( "KEY_EVENT_SHORT_PRESS" );
				BackPressed();
				return true;
				break;
				default:
				//OVR_LOG( "unexpected back key state %i", eventType );
				break;
			}
		}
		break;
		case OVR_KEY_MEDIA_NEXT_TRACK:
			if ( eventType == KEY_EVENT_UP )
			{
				Cinema.SetMovie( Cinema.GetNextMovie() );
				Cinema.ResumeOrRestartMovie();
			}
			break;
		case OVR_KEY_MEDIA_PREV_TRACK:
			if ( eventType == KEY_EVENT_UP )
			{
				Cinema.SetMovie( Cinema.GetPreviousMovie() );
				Cinema.ResumeOrRestartMovie();
			}
			break;
		break;
	}
	return false;
}

//=========================================================================================

static bool InsideUnit( const Vector2f v )
{
	return v.x > -1.0f && v.x < 1.0f && v.y > -1.0f && v.y < 1.0f;
}

void MoviePlayerView::ShowUI()
{
	OVR_LOG( "ShowUI" );
	Cinema.SceneMgr.ForceMono = true;
	Cinema.GetGuiSys().GetGazeCursor().ShowCursor();

	PlaybackControlsMenu->Open();
	GazeTimer.SetGazeTime();

	PlaybackControlsScale.SetLocalScale( Vector3f( Cinema.SceneMgr.GetScreenSize().y * ( 500.0f / 1080.0f ) ) );
	PlaybackControlsPosition.SetLocalPose( Cinema.SceneMgr.GetScreenPose() );

	uiActive = true;
}

void MoviePlayerView::HideUI()
{
	OVR_LOG( "HideUI" );
	PlaybackControlsMenu->Close();

	Cinema.GetGuiSys().GetGazeCursor().HideCursor();
	Cinema.SceneMgr.ForceMono = false;
	uiActive = false;

	SeekSpeed = 0;
	PlaybackPos = 0;
	NextSeekTime = 0;

	BackgroundClicked = false;

	SetSeekIcon( SeekSpeed );
}

void MoviePlayerView::CheckDebugControls( const ovrFrameInput & vrFrame )
{
	if ( !Cinema.AllowDebugControls )
	{
		return;
	}

#if 0
	if ( !( vrFrame.Input.buttonState & BUTTON_Y ) )
	{
		Cinema.SceneMgr.ChangeSeats( vrFrame );
	}
#endif

	if ( vrFrame.Input.buttonPressed & BUTTON_X )
	{
		Cinema.SceneMgr.ToggleLights( 0.5f );
	}

	// Press Y to toggle FreeScreen mode, while holding the scale and distance can be adjusted
	if ( vrFrame.Input.buttonPressed & BUTTON_Y )
	{
		Cinema.SceneMgr.FreeScreenActive = !Cinema.SceneMgr.FreeScreenActive || Cinema.SceneMgr.SceneInfo.UseFreeScreen;
		Cinema.SceneMgr.PutScreenInFront();
	}

	if ( Cinema.SceneMgr.FreeScreenActive && ( vrFrame.Input.buttonState & BUTTON_Y ) )
	{
		Cinema.SceneMgr.FreeScreenDistance -= vrFrame.Input.sticks[0][1] * vrFrame.DeltaSeconds * 3;
		Cinema.SceneMgr.FreeScreenDistance = OVR::Alg::Clamp( Cinema.SceneMgr.FreeScreenDistance, 1.0f, 4.0f );
		Cinema.SceneMgr.FreeScreenScale += vrFrame.Input.sticks[0][0] * vrFrame.DeltaSeconds * 3;
		Cinema.SceneMgr.FreeScreenScale = OVR::Alg::Clamp( Cinema.SceneMgr.FreeScreenScale, 1.0f, 4.0f );
	}
}

static Vector3f	MatrixOrigin( const Matrix4f & m )
{
	return Vector3f( -m.M[0][3], -m.M[1][3], -m.M[2][3] );
}

static Vector3f	MatrixForward( const Matrix4f & m )
{
	return Vector3f( -m.M[2][0], -m.M[2][1], -m.M[2][2] );
}

// -1 to 1 range on screenMatrix, returns -2,-2 if looking away from the screen
Vector2f MoviePlayerView::GazeCoordinatesOnScreen( const Matrix4f & viewMatrix, const Matrix4f screenMatrix ) const
{
	// project along -Z in the viewMatrix onto the Z = 0 plane of screenMatrix
	const Vector3f viewForward = MatrixForward( viewMatrix ).Normalized();

	Vector3f screenForward;
	if ( Cinema.SceneMgr.FreeScreenActive )
	{
		// FIXME: free screen matrix is inverted compared to bounds screen matrix.
		screenForward = -Vector3f( screenMatrix.M[0][2], screenMatrix.M[1][2], screenMatrix.M[2][2] ).Normalized();
	}
	else
	{
		screenForward = -MatrixForward( screenMatrix ).Normalized();
	}

	const float approach = viewForward.Dot( screenForward );
	if ( approach <= 0.1f )
	{
		// looking away
		return Vector2f( -2.0f, -2.0f );
	}

	const Matrix4f panelInvert = screenMatrix.Inverted();
	const Matrix4f viewInvert = viewMatrix.Inverted();

	const Vector3f viewOrigin = viewInvert.Transform( Vector3f( 0.0f ) );
	const Vector3f panelOrigin = MatrixOrigin( screenMatrix );

	// Should we disallow using panels from behind?
	const float d = panelOrigin.Dot( screenForward );
	const float t = -( viewOrigin.Dot( screenForward ) + d ) / approach;

	const Vector3f impact = viewOrigin + viewForward * t;
	const Vector3f localCoordinate = panelInvert.Transform( impact );

	return Vector2f( localCoordinate.x, localCoordinate.y );
}

void MoviePlayerView::CheckInput( const ovrFrameInput & vrFrame )
{
	if ( Cinema.HeadsetWasMounted() )
	{
		Cinema.MovieSelection( false );
	}
	else if ( Cinema.HeadsetWasUnmounted() )
	{
		PauseMovie();
	}

	if ( !uiActive && !RepositionScreen )
	{
		if ( ( vrFrame.Input.buttonPressed & BUTTON_A ) || ( ( vrFrame.Input.buttonReleased & BUTTON_TOUCH ) && !( vrFrame.Input.buttonState & BUTTON_TOUCH_WAS_SWIPE ) ) )
		{
			// open ui if it's not visible
			Cinema.GetGuiSys().GetSoundEffectPlayer().Play( "touch_up" );
			ShowUI();

			// ignore button A or touchpad until release so we don't close the UI immediately after opening it
			UIOpened = true;
		}
	}

	if ( vrFrame.Input.buttonPressed & ( BUTTON_DPAD_LEFT | BUTTON_SWIPE_BACK ) )
	{
		if ( ( vrFrame.Input.buttonPressed & BUTTON_DPAD_LEFT ) || !GazeTimer.IsFocused() )
		{
			ShowUI();
			if ( SeekSpeed == 0 )
			{
				PauseMovie();
			}

			SeekSpeed--;
			if ( ( SeekSpeed == 0 ) || ( SeekSpeed < -MaxSeekSpeed ) )
			{
				SeekSpeed = 0;
				PlayMovie();
			}
			SetSeekIcon( SeekSpeed );

			Cinema.GetGuiSys().GetSoundEffectPlayer().Play( "touch_up" );
		}
	}

	if ( vrFrame.Input.buttonPressed & ( BUTTON_DPAD_RIGHT | BUTTON_SWIPE_FORWARD ) )
	{
		if ( ( vrFrame.Input.buttonPressed & BUTTON_DPAD_RIGHT ) || !GazeTimer.IsFocused() )
		{
			ShowUI();
			if ( SeekSpeed == 0 )
			{
				PauseMovie();
			}

			SeekSpeed++;
			if ( ( SeekSpeed == 0 ) || ( SeekSpeed > MaxSeekSpeed ) )
			{
				SeekSpeed = 0;
				PlayMovie();
			}
			SetSeekIcon( SeekSpeed );

			Cinema.GetGuiSys().GetSoundEffectPlayer().Play( "touch_up" );
		}
	}

	if ( Cinema.SceneMgr.FreeScreenActive )
	{
		const Vector2f screenCursor = GazeCoordinatesOnScreen( Cinema.SceneMgr.Scene.GetCenterEyeViewMatrix(), Cinema.SceneMgr.ScreenMatrix() );
		bool onscreen = false;
		if ( InsideUnit( screenCursor ) )
		{
			onscreen = true;
		}
		else if ( uiActive )
		{
			onscreen = GazeTimer.IsFocused();
		}

		if ( !onscreen )
		{
			// outside of screen, so show reposition message
			const double now = vrapi_GetTimeInSeconds();
			float alpha = static_cast<float>( MoveScreenAlpha.Value( now ) );
			if ( alpha > 0.0f )
			{
				MoveScreenLabel.SetVisible( true );
				MoveScreenLabel.SetTextColor( Vector4f( alpha ) );
			}

			if ( vrFrame.Input.buttonPressed & ( BUTTON_A | BUTTON_TOUCH ) )
			{
				RepositionScreen = true;
			}
		}
		else
		{
			// onscreen, so hide message
			const double now = vrapi_GetTimeInSeconds();
			MoveScreenAlpha.Set( now, -1.0f, now + 1.0f, 1.0f );
			MoveScreenLabel.SetVisible( false );
		}
	}

	// while we're holding down the button or touchpad, reposition screen
	if ( RepositionScreen )
	{
		if ( vrFrame.Input.buttonState & ( BUTTON_A | BUTTON_TOUCH ) )
		{
			Cinema.SceneMgr.PutScreenInFront();
		}
		else
		{
			RepositionScreen = false;
		}
	}

	if ( vrFrame.Input.buttonPressed & BUTTON_START )
	{
		TogglePlayback();
	}

	if ( vrFrame.Input.buttonPressed & BUTTON_SELECT )
	{
		// movie select
		Cinema.GetGuiSys().GetSoundEffectPlayer().Play( "touch_up" );
		Cinema.MovieSelection( false );
	}

	if ( vrFrame.Input.buttonPressed & BUTTON_B )
	{
		if ( !uiActive )
		{
			BackPressed();
		}
		else
		{
			OVR_LOG( "User pressed button 2" );
			Cinema.GetGuiSys().GetSoundEffectPlayer().Play( "touch_up" );
			HideUI();
			PlayMovie();
		}
	}
}

void MoviePlayerView::TogglePlayback()
{
	const bool isPlaying = Native::IsPlaying( Cinema.app );
	if ( isPlaying )
	{
		PauseMovie();
	}
	else
	{
		PlayMovie();
	}
}

void MoviePlayerView::PauseMovie()
{
	Native::PauseMovie( Cinema.app );
	PlaybackPos = Native::GetPosition( Cinema.app );
	PlayButton.SetButtonImages( PlayTexture, PlayHoverTexture, PlayPressedTexture );
}

void MoviePlayerView::PlayMovie()
{
	SeekSpeed = 0;
	SetSeekIcon( SeekSpeed );
	Native::ResumeMovie( Cinema.app );
	PlayButton.SetButtonImages( PauseTexture, PauseHoverTexture, PausePressedTexture );
}

void MoviePlayerView::RewindPressed()
{
	// rewind
	if ( SeekSpeed == 0 )
	{
		PauseMovie();
	}

	SeekSpeed--;
	if ( ( SeekSpeed == 0 ) || ( SeekSpeed < -MaxSeekSpeed ) )
	{
		SeekSpeed = 0;
		PlayMovie();
	}
	SetSeekIcon( SeekSpeed );
}

void MoviePlayerView::FastForwardPressed()
{
	// fast forward
	if ( SeekSpeed == 0 )
	{
		PauseMovie();
	}

	SeekSpeed++;
	if ( ( SeekSpeed == 0 ) || ( SeekSpeed > MaxSeekSpeed ) )
	{
		SeekSpeed = 0;
		PlayMovie();
	}
	SetSeekIcon( SeekSpeed );
}

void MoviePlayerView::CarouselPressed()
{
	Cinema.MovieSelection( false );
}

void MoviePlayerView::ScrubBarClicked( const float progress )
{
	// if we're rw/ff'ing, then stop and resume playback
	if ( SeekSpeed != 0 )
	{
		SeekSpeed = 0;
		PlayMovie();
		SetSeekIcon( SeekSpeed );
		NextSeekTime = 0;
	}

	// choke off the amount position changes we send to the media player
	const double now = vrapi_GetTimeInSeconds();
	if ( now <= NextSeekTime )
	{
		return;
	}

	const int position = static_cast<int>( Cinema.SceneMgr.MovieDuration * progress );
	Native::SetPosition( Cinema.app, position );

	ScrubBar.SetProgress( progress );

	NextSeekTime = vrapi_GetTimeInSeconds() + 0.1;
}

void MoviePlayerView::UpdateUI( const ovrFrameInput & vrFrame )
{
	if ( uiActive )
	{
		double timeSinceLastGaze = vrapi_GetTimeInSeconds() - GazeTimer.GetLastGazeTime();
		if ( !ScrubBar.IsScrubbing() && ( SeekSpeed == 0 ) && ( timeSinceLastGaze > GazeTimeTimeout ) )
		{
			OVR_LOG( "Gaze timeout" );
			HideUI();
		}

		// if we press the touchpad or a button outside of the playback controls, then close the UI
		if ( ( ( vrFrame.Input.buttonPressed & BUTTON_A ) != 0 ) || ( ( vrFrame.Input.buttonPressed & BUTTON_TOUCH ) != 0 ) )
		{
			// ignore button A or touchpad until release so we don't close the UI immediately after opening it
			BackgroundClicked = !GazeTimer.IsFocused() && !UIOpened;
		}

		if ( ( ( vrFrame.Input.buttonReleased & BUTTON_A ) != 0 ) ||
			( ( ( vrFrame.Input.buttonReleased & BUTTON_TOUCH ) != 0 ) && ( ( vrFrame.Input.buttonState & BUTTON_TOUCH_WAS_SWIPE ) == 0 ) )	)
		{
			if ( !GazeTimer.IsFocused() && BackgroundClicked )
			{
				OVR_LOG( "Clicked outside playback controls" );
				Cinema.GetGuiSys().GetSoundEffectPlayer().Play( "touch_up" );
				HideUI();
				PlayMovie();
			}
			BackgroundClicked = false;
		}

		if ( Cinema.SceneMgr.MovieDuration > 0 )
		{
			const int currentPosition = Native::GetPosition( Cinema.app );
			float progress = ( float )currentPosition / ( float )Cinema.SceneMgr.MovieDuration;
			ScrubBar.SetProgress( progress );
		}

		if ( Cinema.SceneMgr.FreeScreenActive )
		{
			// update the screen position & size;
			PlaybackControlsScale.SetLocalScale( Vector3f( Cinema.SceneMgr.GetScreenSize().y * ( 500.0f / 1080.0f ) ) );
			PlaybackControlsPosition.SetLocalPose( Cinema.SceneMgr.GetScreenPose() );
		}
	}

	// clear the flag for ignoring button A or touchpad until release
	UIOpened = false;
}

/*
 * Frame()
 *
 * App override
 */
void MoviePlayerView::Frame( const ovrFrameInput & vrFrame )
{
	// Drop to 2x MSAA during playback, people should be focused
	// on the high quality screen.
	ovrEyeBufferParms eyeBufferParms = Cinema.app->GetEyeBufferParms();
	eyeBufferParms.multisamples = 2;
	Cinema.app->SetEyeBufferParms( eyeBufferParms );

	if ( Native::HadPlaybackError( Cinema.app ) )
	{
		OVR_LOG( "Playback failed" );
		Cinema.UnableToPlayMovie();
	}
	else if ( Native::IsPlaybackFinished( Cinema.app ) )
	{
		OVR_LOG( "Playback finished" );
		Cinema.MovieFinished();
	}

	CheckInput( vrFrame );
	CheckDebugControls( vrFrame );
	UpdateUI( vrFrame );

	if ( Cinema.SceneMgr.FreeScreenActive && !MoveScreenMenu->IsOpen() )
	{
		MoveScreenMenu->Open();
	}
	else if ( !Cinema.SceneMgr.FreeScreenActive && MoveScreenMenu->IsOpen() )
	{
		MoveScreenMenu->Close();
	}

	if ( SeekSpeed != 0 )
	{
		const double now = vrapi_GetTimeInSeconds();
		if ( now > NextSeekTime )
		{
			int PlaybackSpeed = ( SeekSpeed < 0 ) ? -( 1 << -SeekSpeed ) : ( 1 << SeekSpeed );
			PlaybackPos += 250 * PlaybackSpeed;
			Native::SetPosition( Cinema.app, PlaybackPos );
			NextSeekTime = now + 0.25;
		}
	}

	Cinema.SceneMgr.Frame( vrFrame );
}

/*************************************************************************************/

ControlsGazeTimer::ControlsGazeTimer() :
	VRMenuComponent( VRMenuEventFlags_t( VRMENU_EVENT_FRAME_UPDATE ) |
			VRMENU_EVENT_FOCUS_GAINED |
            VRMENU_EVENT_FOCUS_LOST ),
    LastGazeTime( 0 ),
    HasFocus( false )

{
}

void ControlsGazeTimer::SetGazeTime()
{
	LastGazeTime = vrapi_GetTimeInSeconds();
}

eMsgStatus ControlsGazeTimer::OnEvent_Impl( OvrGuiSys & guiSys, ovrFrameInput const & vrFrame,
        VRMenuObject * self, VRMenuEvent const & event )
{
	OVR_UNUSED( guiSys );
	OVR_UNUSED( vrFrame );
	OVR_UNUSED( self );

    switch( event.EventType )
    {
    	case VRMENU_EVENT_FRAME_UPDATE:
    		if ( HasFocus )
    		{
    			LastGazeTime = vrapi_GetTimeInSeconds();
    		}
    		return MSG_STATUS_ALIVE;
        case VRMENU_EVENT_FOCUS_GAINED:
        	HasFocus = true;
        	LastGazeTime = vrapi_GetTimeInSeconds();
    		return MSG_STATUS_ALIVE;
        case VRMENU_EVENT_FOCUS_LOST:
        	HasFocus = false;
    		return MSG_STATUS_ALIVE;
        default:
            OVR_ASSERT( !"Event flags mismatch!" );
            return MSG_STATUS_ALIVE;
    }
}

/*************************************************************************************/

ScrubBarComponent::ScrubBarComponent() :
	VRMenuComponent( VRMenuEventFlags_t( VRMENU_EVENT_TOUCH_DOWN ) |
		VRMENU_EVENT_TOUCH_DOWN |
		VRMENU_EVENT_FRAME_UPDATE |
		VRMENU_EVENT_FOCUS_GAINED |
        VRMENU_EVENT_FOCUS_LOST ),
	HasFocus( false ),
	TouchDown( false ),
	Progress( 0.0f ),
	Duration( 0 ),
	Background( NULL ),
	ScrubBar( NULL ),
	CurrentTime( NULL ),
	SeekTime( NULL ),
	OnClickFunction( NULL ),
	OnClickObject( NULL )

{
}

void ScrubBarComponent::SetDuration( const int duration )
{
	Duration = duration;

	SetProgress( Progress );
}

void ScrubBarComponent::SetOnClick( void ( *callback )( ScrubBarComponent *, void *, float ), void *object )
{
	OnClickFunction = callback;
	OnClickObject = object;
}

void ScrubBarComponent::SetWidgets( UIObject *background, UIObject *scrubBar, UILabel *currentTime, UILabel *seekTime, const int scrubBarWidth )
{
	Background 		= background;
	ScrubBar 		= scrubBar;
	CurrentTime 	= currentTime;
	SeekTime 		= seekTime;
	ScrubBarWidth	= scrubBarWidth;

	SeekTime->SetVisible( false );
}

void ScrubBarComponent::SetProgress( const float progress )
{
	Progress = progress;
	const float seekwidth = static_cast<float>( ScrubBarWidth * progress );

	Vector3f pos = ScrubBar->GetLocalPosition();
	pos.x = PixelScale( ( ScrubBarWidth - seekwidth ) * -0.5f );
	ScrubBar->SetLocalPosition( pos );
	ScrubBar->SetSurfaceDims( 0, Vector2f( seekwidth, 40.0f ) );
	ScrubBar->RegenerateSurfaceGeometry( 0, false );

	pos = CurrentTime->GetLocalPosition();
	pos.x = PixelScale( ScrubBarWidth * -0.5f + seekwidth );
	CurrentTime->SetLocalPosition( pos );
	SetTimeText( CurrentTime, static_cast<int>( Duration * progress ) );
}

void ScrubBarComponent::SetTimeText( UILabel *label, const int time )
{
	int seconds = time / 1000;
	int minutes = seconds / 60;
	int hours = minutes / 60;
	seconds = seconds % 60;
	minutes = minutes % 60;

	if ( hours > 0 )
	{
		label->SetText( StringUtils::Va( "%d:%02d:%02d", hours, minutes, seconds ) );
	}
	else if ( minutes > 0 )
	{
		label->SetText( StringUtils::Va( "%d:%02d", minutes, seconds ) );
	}
	else
	{
		label->SetText( StringUtils::Va( "0:%02d", seconds ) );
	}
}

eMsgStatus ScrubBarComponent::OnEvent_Impl( OvrGuiSys & guiSys, ovrFrameInput const & vrFrame,
        VRMenuObject * self, VRMenuEvent const & event )
{
    switch( event.EventType )
    {
		case VRMENU_EVENT_FOCUS_GAINED:
			HasFocus = true;
			return MSG_STATUS_ALIVE;

		case VRMENU_EVENT_FOCUS_LOST:
			HasFocus = false;
			return MSG_STATUS_ALIVE;

    	case VRMENU_EVENT_TOUCH_DOWN:
    		TouchDown = true;
    		OnClick( guiSys, vrFrame, event );
    		return MSG_STATUS_ALIVE;

    	case VRMENU_EVENT_FRAME_UPDATE:
    		return OnFrame( guiSys, vrFrame, self, event );

        default:
            OVR_ASSERT( !"Event flags mismatch!" );
            return MSG_STATUS_ALIVE;
    }
}

eMsgStatus ScrubBarComponent::OnFrame( OvrGuiSys & guiSys, ovrFrameInput const & vrFrame,
        VRMenuObject * self, VRMenuEvent const & event )
{
	OVR_UNUSED( vrFrame );
	OVR_UNUSED( self );

	if ( TouchDown )
	{
		if ( ( vrFrame.Input.buttonState & ( BUTTON_A | BUTTON_TOUCH ) ) != 0 )
		{
			OnClick( guiSys, vrFrame, event );
		}
		else
		{
			TouchDown = false;
		}
	}

	SeekTime->SetVisible( HasFocus );
	if ( HasFocus )
	{
		Vector3f hitPos = event.HitResult.RayStart + event.HitResult.RayDir * event.HitResult.t;

		// move hit position into local space
		const Posef modelPose = Background->GetWorldPose();
		Vector3f localHit = modelPose.Rotation.Inverted().Rotate( hitPos - modelPose.Translation );

		Bounds3f bounds = Background->GetMenuObject()->GetLocalBounds(guiSys.GetDefaultFont() ) * Background->GetParent()->GetWorldScale();
		const float progress = ( localHit.x - bounds.GetMins().x ) / bounds.GetSize().x;

		if ( ( progress >= 0.0f ) && ( progress <= 1.0f ) )
		{
			const float seekwidth = ScrubBarWidth * progress;
			Vector3f pos = SeekTime->GetLocalPosition();
			pos.x = PixelScale( ScrubBarWidth * -0.5f + seekwidth );
			SeekTime->SetLocalPosition( pos );

			SetTimeText( SeekTime, static_cast<int>( Duration * progress ) );
		}
	}

	return MSG_STATUS_ALIVE;
}

void ScrubBarComponent::OnClick( OvrGuiSys & guiSys, ovrFrameInput const & vrFrame, VRMenuEvent const & event )
{
	if ( OnClickFunction == NULL )
	{
		return;
	}

	Vector3f hitPos = event.HitResult.RayStart + event.HitResult.RayDir * event.HitResult.t;

	// move hit position into local space
	const Posef modelPose = Background->GetWorldPose();
	Vector3f localHit = modelPose.Rotation.Inverted().Rotate( hitPos - modelPose.Translation );

	Bounds3f bounds = Background->GetMenuObject()->GetLocalBounds( guiSys.GetDefaultFont() ) * Background->GetParent()->GetWorldScale();
	const float progress = ( localHit.x - bounds.GetMins().x ) / bounds.GetSize().x;
	if ( ( progress >= 0.0f ) && ( progress <= 1.0f ) )
	{
		( *OnClickFunction )( this, OnClickObject, progress );
	}
}

} // namespace OculusCinema
