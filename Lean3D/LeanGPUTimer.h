#pragma once
#include "LeanPrerequisites.h"
#include "glus.h"
#include <vector>

namespace Lean3D
{
	class LeanGPUTimer
	{
	public:
		LeanGPUTimer();
		~LeanGPUTimer();

		void beginQuery(uint32 frameID);
		void endQuery();
		bool updateResults();

		void reset();
		float getTimeMS() { return _time; }
	private:
		std::vector < uint32 >  _queryPool;
		uint32                  _numQueries;
		uint32                  _queryFrame;
		float                   _time;
		bool                    _activeQuery;
	};
}


