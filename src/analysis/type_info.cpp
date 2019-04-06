#include <analysis/type_info.h>

namespace lac::an
{
	TypeInfo::TypeInfo() = default;

	TypeInfo::TypeInfo(Type type)
		: type(type)
	{
	
	}
} // namespace lac::an
