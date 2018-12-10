/************************************************************************************

Filename    :   ViewManager.cpp
Content     :
Created     :	6/17/2014
Authors     :   Jim Dosé

Copyright   :   Copyright (c) Facebook Technologies, LLC and its affiliates. All rights reserved.

This source code is licensed under the BSD-style license found in the
LICENSE file in the Cinema/ directory. An additional grant 
of patent rights can be found in the PATENTS file in the same directory.

*************************************************************************************/

#include "ViewManager.h"
#include "App.h"

namespace OculusCinema {

ViewManager::ViewManager() :
	Views(),
	CurrentView( NULL ),
	NextView( NULL ),
	ClosedCurrent( false )

{
}

void ViewManager::AddView( View * view )
{
	OVR_LOG( "AddView: %s", view->name );
	Views.PushBack( view );
}

void ViewManager::RemoveView( View * view )
{
	for ( UPInt i = 0; i < Views.GetSize(); i++ )
	{
		if ( Views[ i ] == view )
		{
			Views.RemoveAt( i );
			return;
		}
	}

	// view wasn't in the array
	OVR_ASSERT( false );
	OVR_LOG( "RemoveView: view not in array" );
}

void ViewManager::OpenView( View & view )
{
	OVR_LOG( "OpenView: %s", view.name );
	NextView = &view;
	ClosedCurrent = false;
}

void ViewManager::CloseView()
{
	if ( CurrentView != NULL )
	{
		OVR_LOG( "CloseView: %s", CurrentView->name );
		CurrentView->OnClose();
	}
}

void ViewManager::EnteredVrMode()
{
	if ( CurrentView != NULL )
	{
		OVR_LOG( "EnteredVrMode: %s", CurrentView->name );
		CurrentView->EnteredVrMode();
	}
}

void ViewManager::LeavingVrMode()
{
	if ( CurrentView != NULL )
	{
		OVR_LOG( "LeavingVrMode: %s", CurrentView->name );
		CurrentView->LeavingVrMode();
	}
}

bool ViewManager::OnKeyEvent( const int keyCode, const int repeatCount, const KeyEventType eventType )
{
	if ( ( CurrentView != NULL ) && !CurrentView->IsClosed() )
	{
		return CurrentView->OnKeyEvent( keyCode, repeatCount, eventType );
	}
	else
	{
		return false;
	}
}

void ViewManager::Frame( const ovrFrameInput & vrFrame )
{
	if ( ( NextView != NULL ) && ( CurrentView != NULL ) && !ClosedCurrent )
	{
		OVR_LOG( "OnClose: %s", CurrentView->name );
		CurrentView->OnClose();
		ClosedCurrent = true;
	}

	if ( ( CurrentView == NULL ) || ( CurrentView->IsClosed() ) )
	{
		CurrentView = NextView;
		NextView = NULL;
		ClosedCurrent = false;

		if ( CurrentView != NULL )
		{
			OVR_LOG( "OnOpen: %s", CurrentView->name );
			CurrentView->OnOpen();
		}
	}

	if ( CurrentView != NULL )
	{
		CurrentView->Frame( vrFrame );
	}
}

} // namespace OculusCinema
