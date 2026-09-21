#pragma once
#include "FName.h"

class UClass
{
public:
	UClass(const FName& name, UClass* super)
		: _name(name), _super(super)
	{
	}

	const FName& GetName() const { return _name; }
	UClass* GetSuper() const { return _super; }

	bool IsChildOf(const UClass* other) const
	{
		for (const UClass* current = this; current != nullptr; current = current->_super)
		{
			if (current == other)
				return true;
		}
		return false;
	}

private:
	FName _name;
	UClass* _super;
};

#define DECLARE_UCLASS(ThisClass, SuperClass)								\
public:																		\
	using Super = SuperClass;												\
	static UClass* StaticClass()											\
	{																		\
		static UClass s_class(FName(#ThisClass), SuperClass::StaticClass());	\
		return &s_class;													\
	}																		\
	virtual UClass* GetClass() const override { return ThisClass::StaticClass(); }

using UObjectId = uint64;

class UObject : public enable_shared_from_this<UObject>
{
public:
	UObject();
	virtual ~UObject();

	static UClass* StaticClass();
	virtual UClass* GetClass() const { return StaticClass(); }

	const FName& GetName() const { return _name; }
	void SetName(const FName& name) { _name = name; }
	UObjectId GetUniqueId() const { return _id; }

	static const vector<UObject*>& GetAllUObjects();

protected:
	FName _name;

private:
	UObjectId _id;
};

template<typename T>
bool IsA(const UObject* object)
{
	return object != nullptr && object->GetClass()->IsChildOf(T::StaticClass());
}

template<typename T>
shared_ptr<T> Cast(const shared_ptr<UObject>& object)
{
	if (object && IsA<T>(object.get()))
		return static_pointer_cast<T>(object);
	return nullptr;
}
