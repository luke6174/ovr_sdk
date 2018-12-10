/************************************************************************************

Filename    :   ModelManager.h
Content     :
Created     :	7/3/2014
Authors     :   Jim Dosé

Copyright   :   Copyright (c) Facebook Technologies, LLC and its affiliates. All rights reserved.

This source code is licensed under the BSD-style license found in the
LICENSE file in the Cinema/ directory. An additional grant 
of patent rights can be found in the PATENTS file in the same directory.

*************************************************************************************/

#if !defined( ModelManager_h )
#define ModelManager_h

#include "ModelFile.h"
#include "Kernel/OVR_String.h"
#include "Kernel/OVR_Array.h"

using namespace OVR;

namespace OculusCinema {

class CinemaApp;

class SceneDef
{
public:
						SceneDef() : 
							SceneModel( NULL ),
							Filename(),
							UseFreeScreen( false ),
							UseSeats( false ),
							UseDynamicProgram( false ), 
							Loaded( false ) { }

	ModelFile *			SceneModel;
	String				Filename;
	GlTexture			IconTexture;
	bool				UseFreeScreen;
	bool 				UseSeats;
	bool 				UseDynamicProgram;
	bool				Loaded;
};

class ModelManager
{
public:
						ModelManager( CinemaApp &cinema );
						~ModelManager();

	void				OneTimeInit( const char * launchIntent );
	void				OneTimeShutdown();

	int					GetTheaterCount() const { return Theaters.GetSizeI(); }
	const SceneDef & 	GetTheater( int index ) const;

public:
	CinemaApp &			Cinema;

	Array<SceneDef *>	Theaters;
	SceneDef *			BoxOffice;
	SceneDef *			VoidScene;

	String				LaunchIntent;

	ModelFile *			DefaultSceneModel;

private:
	ModelManager &		operator=( const ModelManager & );

	void 				LoadModels();
	void 				ScanDirectoryForScenes( const char * directory, bool useDynamicProgram, Array<SceneDef *> &scenes ) const;
	SceneDef *			LoadScene( const char * filename, bool useDynamicProgram, bool loadFromApplicationPackage ) const;
};

} // namespace OculusCinema

#endif // ModelManager_h
