#pragma once

namespace rml::qt
{
	class QMetaObject
	{
	public:
		[[nodiscard]] const char* className() const;
	};
}
