#pragma once

#include <windows.h>

namespace Lean3D
{
	//计时器
	class LeanTimer
	{
	public:
		LeanTimer();

		void setEnabled(bool enabled)
		{
			if (enabled && !_enabled)
			{
				_startTime = getTime();
				_enabled = true;
			}
			else if ( !enabled && _enabled)
			{
				double endTime = getTime();
				_elapsedTime += endTime - _startTime;
				_enabled = false;
			}
		}

		void reset()
		{
			_elapsedTime = 0;
			if (_enabled) _startTime = getTime();
		}

		//精度毫秒
		float getElapsedTime()
		{
			if ( _enabled )
			{
				double endTime = getTime();
				_elapsedTime += endTime - _startTime;
				_startTime = endTime;
			}
			return (float)_elapsedTime;
		}

	protected:

		double getTime()
		{
			//绑定到计时器指定CPU
			DWORD_PTR preThreadAffMask = SetThreadAffinityMask(GetCurrentThread(), _affMask);

			LARGE_INTEGER curTick;
			QueryPerformanceCounter(&curTick);

			//还原CPU绑定
			SetThreadAffinityMask(GetCurrentThread(), preThreadAffMask);

			// time = 计数 / 频率 * 1000
			return (double)curTick.QuadPart / (double)_timerFreq.QuadPart * 1000.0f; //精确到毫秒
		}

	protected:
		double _startTime;
		double _elapsedTime;
		LARGE_INTEGER _timerFreq;
		DWORD_PTR _affMask;
		bool _enabled;

	};

	LeanTimer::LeanTimer() : _elapsedTime(0), _enabled(false)
	{
		//找个可用CPU
		DWORD_PTR usedCpuMask, sysMask;

		GetProcessAffinityMask(GetCurrentProcess(), &usedCpuMask, &sysMask);
		
		_affMask = 1;
		while ((_affMask & usedCpuMask) == 0) _affMask <<= 1;

		//返回前面的亲缘性屏蔽 查询完CPU频率后 还原
		DWORD_PTR preThreadAffMask = SetThreadAffinityMask(GetCurrentThread(), _affMask);
		QueryPerformanceFrequency(&_timerFreq);
		SetThreadAffinityMask(GetCurrentThread(), preThreadAffMask);
	}

	//定时触发定时器
	class LeanItrvalTrgTimer : LeanTimer
	{
	public:
		LeanItrvalTrgTimer();
		~LeanItrvalTrgTimer();

	private:

	};

	LeanItrvalTrgTimer::LeanItrvalTrgTimer()
	{
	}

	LeanItrvalTrgTimer::~LeanItrvalTrgTimer()
	{
	}
}

