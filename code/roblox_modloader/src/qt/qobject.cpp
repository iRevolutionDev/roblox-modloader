#include "RobloxModLoader/qt/qobject.hpp"

#include "RobloxModLoader/qt/qmetaobject.hpp"
#include "RobloxModLoader/qt/qt_module.hpp"

namespace rml::qt
{
	const char* QObject::class_name() const
	{
		const QMetaObject* meta = metaObject();
		const char* name = meta ? meta->className() : nullptr;
		return name ? name : "";
	}

	bool QObject::inherits(const char* class_name) const
	{
		static const auto fn = detail::core<bool (*)(const void*, const char*)>("?inherits@QObject@@QEBA_NPEBD@Z");
		return class_name && fn && fn(this, class_name);
	}
}
