/************************************************************************************

Filename    :   CinemaStrings.h
Content     :	Text strings used by app.  Located in one place to make translation easier.
Created     :	9/30/2014
Authors     :   Jim Dosé

Copyright   :   Copyright (c) Facebook Technologies, LLC and its affiliates. All rights reserved.

This source code is licensed under the BSD-style license found in the
LICENSE file in the Cinema/ directory. An additional grant 
of patent rights can be found in the PATENTS file in the same directory.

*************************************************************************************/

#if !defined( CinemaStrings_h )
#define CinemaStrings_h

#include "Kernel/OVR_String.h"

using namespace OVR;

namespace OculusCinema {

class CinemaApp;

class ovrCinemaStrings {
public:
	static ovrCinemaStrings *	Create( CinemaApp & cinema );
	static void					Destroy( CinemaApp & cinema, ovrCinemaStrings * & strings );

	void		OneTimeInit( CinemaApp &cinema );

	String		Category_Trailers;
	String		Category_MyVideos;

	String		MovieSelection_Resume;
	String		MovieSelection_Next;

	String		ResumeMenu_Title;
	String		ResumeMenu_Resume;
	String		ResumeMenu_Restart;

	String		TheaterSelection_Title;

	String		Error_NoVideosOnPhone;
	String		Error_NoVideosInMyVideos;
	String		Error_UnableToPlayMovie;

	String		MoviePlayer_Reorient;
};

} // namespace OculusCinema

#endif // CinemaStrings_h
