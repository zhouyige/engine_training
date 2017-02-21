#include "LeanGPUTimer.h"

namespace Lean3D
{
	LeanGPUTimer::LeanGPUTimer() : _numQueries(0), _queryFrame(0), _time(0), _activeQuery(false)
	{
		reset();
	}


	LeanGPUTimer::~LeanGPUTimer()
	{
		if (!_queryPool.empty())
		{
			glDeleteQueries((uint32)_queryPool.size(), &_queryPool[0]);
		}
	}

	void LeanGPUTimer::beginQuery(uint32 frameID)
	{
		ASSERT(!_activeQuery);

		if (_queryFrame != frameID)
		{
			if (!updateResults()) return;
			
			_queryFrame = frameID;
			_numQueries = 0;
		}

		uint32 queryObj[2];
		if (_numQueries++ * 2 == _queryPool.size())
		{
			glGenQueries(2, queryObj);
			_queryPool.push_back(queryObj[0]);
			_queryPool.push_back(queryObj[1]);
		}
		else
		{
			queryObj[0] = _queryPool[(_numQueries - 1) * 2];
		}

		_activeQuery = true;
		glQueryCounter(queryObj[0], GL_TIMESTAMP);
	}

	void LeanGPUTimer::endQuery()
	{
		if (_activeQuery)
		{
			glQueryCounter(_queryPool[_numQueries * 2 - 1], GL_TIMESTAMP);
			_activeQuery = false;
		}
	}

	bool LeanGPUTimer::updateResults()
	{
		if (_numQueries == 0)
		{
			_time = 0;
			return true;
		}

		//确认上次查询
		GLint available;
		glGetQueryObjectiv(_queryPool[_numQueries * 2 - 1], GL_QUERY_RESULT_AVAILABLE, &available);
		if (!available) return false;
		
		//计时
		GLint64 timeStart = 0, timeEnd = 0, timeAccum = 0;
		for (uint32 i = 0; i < _numQueries; ++i)
		{
			glGetQueryObjecti64v(_queryPool[i * 2], GL_QUERY_RESULT, &timeStart);
			glGetQueryObjecti64v(_queryPool[i * 2 + 1], GL_QUERY_RESULT, &timeEnd);
			timeAccum += timeEnd - timeStart;
		}
		_time = (float)((double)timeAccum / 1000000.0);
		return true;
	}

	void LeanGPUTimer::reset()
	{
		_time = 0.0f;
	}

}

