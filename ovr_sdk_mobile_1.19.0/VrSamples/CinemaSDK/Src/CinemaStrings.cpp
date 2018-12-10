/************************************************************************************

Filename    :   CinemaStrings.cpp
Content     :	Text strings used by app.  Located in one place to make translation easier.
Created     :	9/30/2014
Authors     :   Jim Dosé

Copyright   :   Copyright (c) Facebook Technologies, LLC and its affiliates. All rights reserved.

This source code is licensed under the BSD-style license found in the
LICENSE file in the Cinema/ directory. An additional grant 
of patent rights can be found in the PATENTS file in the same directory.

*************************************************************************************/

#include "CinemaStrings.h"
#include "OVR_Locale.h"
#include "CinemaApp.h"

namespace OculusCinema
{

void ovrCinemaStrings::OneTimeInit( CinemaApp & cinema )
{
	OVR_LOG( "ovrCinemaStrings::OneTimeInit" );

	cinema.GetLocale().GetString( "@string/Category_Trailers", 		"@string/Category_Trailers", 		Category_Trailers );
	cinema.GetLocale().GetString( "@string/Category_MyVideos", 		"@string/Category_MyVideos", 		Category_MyVideos );
	cinema.GetLocale().GetString( "@string/MovieSelection_Resume",	"@string/MovieSelection_Resume",	MovieSelection_Resume );
	cinema.GetLocale().GetString( "@string/MovieSelection_Next", 	"@string/MovieSelection_Next", 		MovieSelection_Next );
	cinema.GetLocale().GetString( "@string/ResumeMenu_Title", 		"@string/ResumeMenu_Title", 		ResumeMenu_Title );
	cinema.GetLocale().GetString( "@string/ResumeMenu_Resume", 		"@string/ResumeMenu_Resume", 		ResumeMenu_Resume );
	cinema.GetLocale().GetString( "@string/ResumeMenu_Restart", 	"@string/ResumeMenu_Restart", 		ResumeMenu_Restart );
	cinema.GetLocale().GetString( "@string/TheaterSelection_Title", "@string/TheaterSelection_Title", 	TheaterSelection_Title );

	cinema.GetLocale().GetString( "@string/Error_NoVideosOnPhone", 	"@string/Error_NoVideosOnPhone", 	Error_NoVideosOnPhone );

	cinema.GetLocale().GetString( "@string/Error_NoVideosInMyVideos", "@string/Error_NoVideosInMyVideos", Error_NoVideosInMyVideos );

	cinema.GetLocale().GetString( "@string/Error_UnableToPlayMovie", "@string/Error_UnableToPlayMovie",	Error_UnableToPlayMovie );

	cinema.GetLocale().GetString( "@string/MoviePlayer_Reorient", 	"@string/MoviePlayer_Reorient", 	MoviePlayer_Reorient );
}

ovrCinemaStrings *	ovrCinemaStrings::Create( CinemaApp & cinema )
{
	ovrCinemaStrings * cs = new ovrCinemaStrings;
	cs->OneTimeInit( cinema );
	return cs;
}

void ovrCinemaStrings::Destroy( CinemaApp & cinema, ovrCinemaStrings * & strings )
{
	OVR_UNUSED( cinema );

	delete strings;
	strings = NULL;
}

} // namespace OculusCinema
