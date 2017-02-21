#pragma  once
#include "pre.h"
#include <vector>
#include <string>

namespace lean{
//对象列表，可以减少对象频繁释放，申请的消耗
	template<class T> class ReferenceObjList
	{
	public:
		unsigned int add(const T &obj)
		{
			if (!_freeList.empty())
			{
				unsigned int index = _freeList.back();
				_freeList.pop_back();
				_objectList[index] = obj;
				return index + 1;
			}
			else
			{
				_objectList.push_back(obj);
				return (unsigned int)_objectList.size();
			}
		}

		void remove(unsigned int handle)
		{
			ASSERT(handle > 0 && handle <= _objectList.size());
			_objectList[handle - 1] = T();
			_freeList.push_back(handle - 1);

		}

		void clearAll()
		{
			_freeList.clear();
			_objectList.clear();
		}

		T &getRef(unsigned int handle)
		{
			ASSERT(handle > 0 && handle <= _objectList.size());
			return _objectList[handle - 1];
		}

	private:
		std::vector<T> _objectList;
		std::vector<unsigned int> _freeList;
	};
}
	