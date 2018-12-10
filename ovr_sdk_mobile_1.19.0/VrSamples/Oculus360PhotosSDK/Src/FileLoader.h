/************************************************************************************

Filename    :   FileLoader.h
Content     :   
Created     :   August 13, 2014
Authors     :   John Carmack

Copyright   :   Copyright (c) Facebook Technologies, LLC and its affiliates. All rights reserved.

This source code is licensed under the BSD-style license found in the
LICENSE file in the Oculus360Photos/ directory. An additional grant 
of patent rights can be found in the PATENTS file in the same directory.

************************************************************************************/

#include "MessageQueue.h"

namespace OVR {

class App;
class Oculus360Photos;

void InitFileQueue( App * app, Oculus360Photos * photos);

extern ovrMessageQueue	Queue1;

}

