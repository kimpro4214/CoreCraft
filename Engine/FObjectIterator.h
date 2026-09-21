#pragma once
#include "UObject.h"

template<typename T>
class FObjectIterator
{
public:
	FObjectIterator()
		: _objects(UObject::GetAllUObjects()), _index(0)
	{
		static_assert(is_base_of_v<UObject, T>, "T must derive from UObject");
		Advance();
	}

	explicit operator bool() const { return _index < _objects.size(); }

	T* operator*() const { return static_cast<T*>(_objects[_index]); }
	T* operator->() const { return static_cast<T*>(_objects[_index]); }

	FObjectIterator& operator++()
	{
		++_index;
		Advance();
		return *this;
	}

private:
	void Advance()
	{
		while (_index < _objects.size() && !IsA<T>(_objects[_index]))
			++_index;
	}

private:
	// 순회 도중 생성/파괴로 목록이 바뀌어도 안전하도록 시작 시점의 스냅샷을 따로 들고 있는다.
	vector<UObject*> _objects;
	size_t _index;
};
