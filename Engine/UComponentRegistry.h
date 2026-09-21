#pragma once
#include "UActorComponent.h"

class UComponentRegistry
{
	DECLARE_SINGLE(UComponentRegistry);
public:
	using Factory = function<shared_ptr<UActorComponent>()>;

	template<typename T>
	void Register()
	{
		static_assert(is_base_of_v<UActorComponent, T>, "T must derive from UActorComponent");

		FName name = T::StaticClass()->GetName();
		if (_factories.find(name) != _factories.end())
			return;

		_factories[name] = [] { return static_pointer_cast<UActorComponent>(make_shared<T>()); };
		_names.push_back(name);
	}

	shared_ptr<UActorComponent> Create(const FName& name) const
	{
		auto found = _factories.find(name);
		return found != _factories.end() ? found->second() : nullptr;
	}

	const vector<FName>& GetRegisteredNames() const { return _names; }

private:
	unordered_map<FName, Factory> _factories;
	vector<FName> _names;
};
