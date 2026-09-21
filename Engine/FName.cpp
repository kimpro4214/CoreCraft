#include "pch.h"
#include "FName.h"

namespace
{
	struct FNameTable
	{
		vector<string> names;
		unordered_map<string, uint32> lookup;

		FNameTable()
		{
			names.push_back("None");
			lookup.emplace("None", 0);
		}

		uint32 Intern(const string& name)
		{
			auto found = lookup.find(name);
			if (found != lookup.end())
				return found->second;

			uint32 index = static_cast<uint32>(names.size());
			names.push_back(name);
			lookup.emplace(name, index);
			return index;
		}
	};

	FNameTable& GetTable()
	{
		static FNameTable table;
		return table;
	}
}

const FName NAME_None;

FName::FName()
	: _index(0)
{
}

FName::FName(const char* name)
	: _index(GetTable().Intern(name))
{
}

FName::FName(const string& name)
	: _index(GetTable().Intern(name))
{
}

const string& FName::ToString() const
{
	return GetTable().names[_index];
}
