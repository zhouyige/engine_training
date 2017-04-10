#include "app.h"
#include "Lean3D.h"
#include "Lean3DUtils.h"
#include <math.h>
#include <iomanip>

using namespace std;

inline float degToRad( float f ) 
{
	return f * (3.1415926f / 180.0f);
}


Application::Application( const std::string &appPath )
{
	for( unsigned int i = 0; i < 320; ++i )
	{	
		_keys[i] = false;
		_prevKeys[i] = false;
	}

	_x = 5; _y = 3; _z = 19; _rx = 7; _ry = 15; _velocity = 10.0f;
	_curFPS = 30;

	_statMode = 0;
	_freezeMode = 0; _debugViewMode = false; _wireframeMode = false;
	_animTime = 0; _weight = 1.0f;
	_cam = 0;

	_contentDir = appPath + "../Content";
}


bool Application::init()
{	
	if( !LeanInit() )
	{	
		h3dutDumpMessages();
		return false;
	}

	LeanSetOption( H3DOptions::LoadTextures, 1 );
	LeanSetOption( H3DOptions::TexCompression, 0 );
	LeanSetOption( H3DOptions::FastAnimation, 0 );
	LeanSetOption( H3DOptions::MaxAnisotropy, 4 );
	LeanSetOption( H3DOptions::SampleCount, 8);
	LeanSetOption( H3DOptions::ShadowMapSize, 2048 );


	_hdrPipeRes = LeanAddResource( H3DResTypes::Pipeline, "pipelines/hdr.pipeline.xml", 0 );
	_forwardPipeRes = LeanAddResource( H3DResTypes::Pipeline, "pipelines/forward.pipeline.xml", 0 );

	_fontMatRes = LeanAddResource( H3DResTypes::Material, "overlays/font.material.xml", 0 );
	_panelMatRes = LeanAddResource( H3DResTypes::Material, "overlays/panel.material.xml", 0 );
	_logoMatRes = LeanAddResource( H3DResTypes::Material, "overlays/logo.material.xml", 0 );

	ResHandle envRes = LeanAddResource( H3DResTypes::SceneGraph, "models/sphere/sphere.scene.xml", 0 );

	ResHandle knightRes = LeanAddResource( H3DResTypes::SceneGraph, "models/knight/knight.scene.xml", 0 );
	ResHandle knightAnim1Res = LeanAddResource( H3DResTypes::Animation, "animations/knight_order.anim", 0 );
	ResHandle knightAnim2Res = LeanAddResource( H3DResTypes::Animation, "animations/knight_attack.anim", 0 ); 
		
	ResHandle skyboxRes = LeanAddResource(H3DResTypes::SceneGraph, "models/skybox/skybox.scene.xml", 0);
	
	ResHandle particleSysRes = LeanAddResource( H3DResTypes::SceneGraph, "particles/particleSys1/particleSys1.scene.xml", 0 );
	

	h3dutLoadResourcesFromDisk( _contentDir.c_str() );


	_cam = LeanAddCameraNode( H3DRootNode, "Camera", _hdrPipeRes );

	NodeHandle env = LeanAddNodes( H3DRootNode, envRes );
	LeanSetNodeTransform( env, 0, -20, 0, 0, 0, 0, 20, 20, 20 );

	_knight = LeanAddNodes( H3DRootNode, knightRes );

	LeanSetNodeTransform( _knight, 0, 0, 0, 0, 180, 0, 0.1f, 0.1f, 0.1f );
	LeanSetupModelAnimStage( _knight, 0, knightAnim1Res, 0, "", false );
	LeanSetupModelAnimStage( _knight, 1, knightAnim2Res, 0, "", false );

	NodeHandle _skybox = LeanAddNodes(H3DRootNode, skyboxRes);
	LeanSetNodeTransform(_skybox, 0, 0, 0, 0, 180, 0, 100.0f, 100.0f,100.0f);

	LeanFindNodes( _knight, "Bip01_R_Hand", H3DNodeTypes::Joint );
	NodeHandle hand = LeanGetNodeFindResult( 0 );
	_particleSys = LeanAddNodes( hand, particleSysRes );
	LeanSetNodeTransform( _particleSys, 4, 5, 0, 90, 0, 0, 1, 1, 1 );


	NodeHandle light = LeanAddLightNode( H3DRootNode, "Light1", 0, "LIGHTING", "SHADOWMAP" );
	LeanSetNodeTransform( light, 0, 15, 10, -60, 0, 0, 1, 1, 1 );
	LeanSetNodeParamF( light, H3DLight::RadiusF, 0, 30 );
	LeanSetNodeParamF( light, H3DLight::FovF, 0, 90 );
	LeanSetNodeParamI( light, H3DLight::ShadowMapCountI, 1 );
	LeanSetNodeParamF( light, H3DLight::ShadowMapBiasF, 0, 0.01f );
	LeanSetNodeParamF( light, H3DLight::ColorF3, 0, 1.0f );
	LeanSetNodeParamF( light, H3DLight::ColorF3, 1, 0.8f );
	LeanSetNodeParamF( light, H3DLight::ColorF3, 2, 0.7f );
	LeanSetNodeParamF( light, H3DLight::ColorMultiplierF, 0, 1.0f );

	NodeHandle matRes = LeanFindResource( H3DResTypes::Material, "pipelines/postHDR.material.xml" );
	LeanSetMaterialUniform( matRes, "hdrExposure", 2.5f, 0, 0, 0 );
	LeanSetMaterialUniform( matRes, "hdrBrightThres", 0.5f, 0, 0, 0 );
	LeanSetMaterialUniform( matRes, "hdrBrightOffset", 0.08f, 0, 0, 0 );
	
	return true;
}


void Application::mainLoop( float fps )
{
	_curFPS = fps;
	
	LeanSetOption( H3DOptions::DebugViewMode, _debugViewMode ? 1.0f : 0.0f );
	LeanSetOption( H3DOptions::WireframeMode, _wireframeMode ? 1.0f : 0.0f );
	
	if( !_freezeMode )
	{
		_animTime += 1.0f / _curFPS;

		LeanSetModelAnimParams( _knight, 0, _animTime * 24.0f, _weight );
		LeanSetModelAnimParams( _knight, 1, _animTime * 24.0f, 1.0f - _weight );
		LeanUpdateModel( _knight, H3DModelUpdateFlags::Animation | H3DModelUpdateFlags::Geometry );

		unsigned int cnt = LeanFindNodes( _particleSys, "", H3DNodeTypes::Emitter );
		for( unsigned int i = 0; i < cnt; ++i )
			LeanUpdateEmitter( LeanGetNodeFindResult( i ), 1.0f / _curFPS );
	}
	
	LeanSetNodeTransform( _cam, _x, _y, _z, _rx ,_ry, 0, 1, 1, 1 );
	
	h3dutShowFrameStats( _fontMatRes, _panelMatRes, _statMode );
	if( _statMode > 0 )
	{	
		_text.str( "" );
		_text << fixed << setprecision( 2 ) << "Weight: " << _weight;
		h3dutShowText( _text.str().c_str(), 0.03f, 0.24f, 0.026f, 1, 1, 1, _fontMatRes );
	}

	const float ww = (float)LeanGetNodeParamI( _cam, H3DCamera::ViewportWidthI ) /
	                 (float)LeanGetNodeParamI( _cam, H3DCamera::ViewportHeightI );
	const float ovLogo[] = { ww-0.2f, 0.8f, 0, 1,  ww-0.2f, 1, 0, 0,  ww, 1, 1, 0,  ww, 0.8f, 1, 1 };
	
	LeanShowOverlays( ovLogo, 4, 1.f, 1.f, 1.f, 1.f, _logoMatRes, 0 );
	
	LeanRender( _cam );

	LeanFinalizeFrame();

	LeanClearOverlays();

	h3dutDumpMessages();
}


void Application::release()
{
	LeanRelease();
}


void Application::resize( int width, int height )
{
	LeanSetNodeParamI( _cam, H3DCamera::ViewportXI, 0 );
	LeanSetNodeParamI( _cam, H3DCamera::ViewportYI, 0 );
	LeanSetNodeParamI( _cam, H3DCamera::ViewportWidthI, width );
	LeanSetNodeParamI( _cam, H3DCamera::ViewportHeightI, height );
	
	LeanSetupCameraView( _cam, 45.0f, (float)width / height, 0.1f, 1000.0f );
	LeanResizePipelineBuffers( _hdrPipeRes, width, height );
	LeanResizePipelineBuffers( _forwardPipeRes, width, height );
}


void Application::keyStateHandler()
{

	if( _keys[32] && !_prevKeys[32] )  // Space
	{
		if( ++_freezeMode == 3 ) _freezeMode = 0;
	}

	if( _keys[260] && !_prevKeys[260] )  // F3
	{
		if( LeanGetNodeParamI( _cam, H3DCamera::PipeResI ) == _hdrPipeRes )
			LeanSetNodeParamI( _cam, H3DCamera::PipeResI, _forwardPipeRes );
		else
			LeanSetNodeParamI( _cam, H3DCamera::PipeResI, _hdrPipeRes );
	}
	
	if( _keys[264] && !_prevKeys[264] )  // F7
		_debugViewMode = !_debugViewMode;

	if( _keys[265] && !_prevKeys[265] )  // F8
		_wireframeMode = !_wireframeMode;
	
	if( _keys[263] && !_prevKeys[263] )  // F6
	{
		_statMode += 1;
		if( _statMode > H3DUTMaxStatMode ) _statMode = 0;
	}
	

	if( _freezeMode != 2 )
	{
		float curVel = _velocity / _curFPS;
		
		if( _keys[287] ) curVel *= 5;	// LShift
		
		if( _keys['W'] )
		{
			// Move forward
			_x -= sinf( degToRad( _ry ) ) * cosf( -degToRad( _rx ) ) * curVel;
			_y -= sinf( -degToRad( _rx ) ) * curVel;
			_z -= cosf( degToRad( _ry ) ) * cosf( -degToRad( _rx ) ) * curVel;
		}
		if( _keys['S'] )
		{
			// Move backward
			_x += sinf( degToRad( _ry ) ) * cosf( -degToRad( _rx ) ) * curVel;
			_y += sinf( -degToRad( _rx ) ) * curVel;
			_z += cosf( degToRad( _ry ) ) * cosf( -degToRad( _rx ) ) * curVel;
		}
		if( _keys['A'] )
		{
			// Strafe left
			_x += sinf( degToRad( _ry - 90) ) * curVel;
			_z += cosf( degToRad( _ry - 90 ) ) * curVel;
		}
		if( _keys['D'] )
		{
			// Strafe right
			_x += sinf( degToRad( _ry + 90 ) ) * curVel;
			_z += cosf( degToRad( _ry + 90 ) ) * curVel;
		}
		if( _keys['1'] )
		{
			// Change blend weight
			_weight += 2 / _curFPS;
			if( _weight > 1 ) _weight = 1;
		}
		if( _keys['2'] )
		{
			// Change blend weight
			_weight -= 2 / _curFPS;
			if( _weight < 0 ) _weight = 0;
		}
	}
}


void Application::mouseMoveEvent( float dX, float dY )
{
	if( _freezeMode == 2 ) return;
	
	_ry -= dX / 100 * 30;
	
	_rx += dY / 100 * 30;
	if( _rx > 90 ) _rx = 90; 
	if( _rx < -90 ) _rx = -90;
}
