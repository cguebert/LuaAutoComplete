#include <analysis/type_info.h>

namespace lac::an
{
	Parameter::Parameter(const Parameter& other)
		: Parameter(other.name, *other.type)
	{
	}

	Parameter::Parameter(std::string_view name)
		: Parameter(name, Type::unknown)
	{
	}

	Parameter::Parameter(std::string_view name, const TypeInfo& type)
		: name(name)
		, type(std::make_unique<TypeInfo>(type))
	{
	}

	TypeInfo::TypeInfo() = default;

	TypeInfo::TypeInfo(Type type)
		: type(type)
	{
	}

	TypeInfo TypeInfo::convert(Type destination) const
	{
		if (type == destination)
			return destination;

		if (type == Type::error)
			return Type::error;
		if (type == Type::unknown)
			return destination;

		switch (destination)
		{
		case Type::boolean:
			return Type::boolean;

		case Type::number:
			if (type == Type::number || type == Type::string)
				return Type::number;
			return Type::error;

		case Type::string:
			if (type == Type::string || type == Type::number)
				return Type::string;
			return Type::error;

		default:
			return Type::error;
		}
	}

	TypeInfo::operator bool() const
	{
		return type != Type::error;
	}

} // namespace lac::an
