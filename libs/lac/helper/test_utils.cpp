#include <lac/helper/test_utils.h>

std::ostream& operator<<(std::ostream& os, const std::type_info& value)
{
	os << value.name();
	return os;
}
