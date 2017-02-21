#pragma once

#include <windows.h>

namespace Lean3D
{
	//��ʱ��
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

		//���Ⱥ���
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
			//�󶨵���ʱ��ָ��CPU
			DWORD_PTR preThreadAffMask = SetThreadAffinityMask(GetCurrentThread(), _affMask);

			LARGE_INTEGER curTick;
			QueryPerformanceCounter(&curTick);

			//��ԭCPU��
			SetThreadAffinityMask(GetCurrentThread(), preThreadAffMask);

			// time = ���� / Ƶ�� * 1000
			return (double)curTick.QuadPart / (double)_timerFreq.QuadPart * 1000.0f; //��ȷ������
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
		//�Ҹ�����CPU
		DWORD_PTR usedCpuMask, sysMask;

		GetProcessAffinityMask(GetCurrentProcess(), &usedCpuMask, &sysMask);
		
		_affMask = 1;
		while ((_affMask & usedCpuMask) == 0) _affMask <<= 1;

		//����ǰ�����Ե������ ��ѯ��CPUƵ�ʺ� ��ԭ
		DWORD_PTR preThreadAffMask = SetThreadAffinityMask(GetCurrentThread(), _affMask);
		QueryPerformanceFrequency(&_timerFreq);
		SetThreadAffinityMask(GetCurrentThread(), preThreadAffMask);
	}

	//��ʱ������ʱ��
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

