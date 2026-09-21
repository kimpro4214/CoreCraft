#include "pch.h"
#include "UObject.h"

namespace
{
	atomic<UObjectId> GNextUObjectId = 1;

	vector<UObject*>& GetRegistry()
	{
		static vector<UObject*> registry;
		return registry;
	}
}

UObject::UObject()
	: _id(GNextUObjectId.fetch_add(1))
{
	GetRegistry().push_back(this);
}

UObject::~UObject()
{
	vector<UObject*>& registry = GetRegistry();
	registry.erase(remove(registry.begin(), registry.end(), this), registry.end());
}

UClass* UObject::StaticClass()
{
	static UClass s_class(FName("UObject"), nullptr);
	return &s_class;
}

const vector<UObject*>& UObject::GetAllUObjects()
{
	return GetRegistry();
}
