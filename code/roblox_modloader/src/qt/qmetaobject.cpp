#include "RobloxModLoader/qt/qmetaobject.hpp"

#include "RobloxModLoader/qt/qt_module.hpp"

namespace rml::qt
{
	const char* QMetaObject::className() const
	{
		static const auto fn = detail::core<const char* (*)(const void*)>("?className@QMetaObject@@QEBAPEBDXZ");
		return fn ? fn(this) : "";
	}
}
