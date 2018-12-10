/************************************************************************************

Filename    :   MovieSelectionComponent.h
Content     :   Menu component for the movie selection menu.
Created     :   October 8, 2014
Authors     :   Jim Dosé

Copyright   :   Copyright (c) Facebook Technologies, LLC and its affiliates. All rights reserved.

This source code is licensed under the BSD-style license found in the
LICENSE file in the Cinema/ directory. An additional grant 
of patent rights can be found in the PATENTS file in the same directory.

*************************************************************************************/

#include "VRMenuComponent.h"

#if !defined( MovieSelectionComponent_h )
#define MovieSelectionComponent_h

using namespace OVR;

namespace OculusCinema {

class MovieSelectionView;

//==============================================================
// MovieSelectionComponent
class MovieSelectionComponent : public VRMenuComponent
{
public:
							MovieSelectionComponent( MovieSelectionView *view );

private:
    ovrSoundLimiter			Sound;
    MovieSelectionView *	CallbackView;

private:
    virtual eMsgStatus      OnEvent_Impl( OvrGuiSys & guiSys, ovrFrameInput const & vrFrame,
                                    VRMenuObject * self, VRMenuEvent const & event );

    eMsgStatus              Frame( OvrGuiSys & guiSys, ovrFrameInput const & vrFrame,
                                    VRMenuObject * self, VRMenuEvent const & event );
    eMsgStatus              FocusGained( OvrGuiSys & guiSys, ovrFrameInput const & vrFrame,
                                    VRMenuObject * self, VRMenuEvent const & event );
    eMsgStatus              FocusLost( OvrGuiSys & guiSys, ovrFrameInput const & vrFrame,
                                    VRMenuObject * self, VRMenuEvent const & event );
};

} // namespace OculusCinema

#endif // MovieSelectionComponent_h
