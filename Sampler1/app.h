#ifndef _app_H_
#define _app_H_

#include "Lean3D.h"
#include <sstream>
#include <string>


class Application
{
public:
	Application( const std::string &appPath );
	
	void setKeyState( int key, bool state ) { _prevKeys[key] = _keys[key]; _keys[key] = state; }

	const char *getTitle() { return "Knight - Lean3D Sample"; }
	
	bool init();
	void mainLoop( float fps );
	void release();
	void resize( int width, int height );

	void keyStateHandler();
	void mouseMoveEvent( float dX, float dY );

private:
	bool               _keys[320], _prevKeys[320];
	
	float              _x, _y, _z, _rx, _ry;  // Viewer position and orientation
	float              _velocity;  // Velocity for movement
	float              _curFPS;
	std::stringstream  _text;

	int                _statMode;
	int                _freezeMode;
	bool               _debugViewMode, _wireframeMode;
	float              _animTime, _weight;
	
	// Engine objects
	ResHandle             _fontMatRes, _panelMatRes;
	ResHandle             _pipeRes, _logoMatRes, _hdrPipeRes, _forwardPipeRes;
	NodeHandle            _cam, _knight, _particleSys;

	std::string        _contentDir;
};

#endif // _app_H_
