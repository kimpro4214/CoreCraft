#include "pch.h"
#include "UObject.h"

namespace
{
	atomic<UObjectId> GNextUObjectId = 1;
}

UObject::UObject()
	: _id(GNextUObjectId.fetch_add(1))
{
}

UClass* UObject::StaticClass()
{
	static UClass s_class(FName("UObject"), nullptr);
	return &s_class;
}
