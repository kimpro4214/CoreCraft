#pragma once

class FName
{
public:
	FName();
	FName(const char* name);
	FName(const string& name);

	bool operator==(const FName& other) const { return _index == other._index; }
	bool operator!=(const FName& other) const { return _index != other._index; }

	const string& ToString() const;
	bool IsNone() const { return _index == 0; }
	uint32 GetComparisonIndex() const { return _index; }

private:
	uint32 _index;
};

extern const FName NAME_None;

namespace std
{
	template<>
	struct hash<FName>
	{
		size_t operator()(const FName& name) const noexcept
		{
			return hash<uint32>()(name.GetComparisonIndex());
		}
	};
}
