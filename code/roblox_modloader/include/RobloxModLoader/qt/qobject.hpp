#pragma once

#include "RobloxModLoader/rml_export.hpp"
#include "RobloxModLoader/util/layout_assert.hpp"

namespace rml::qt
{
	class QMetaObject;

	class RML_EXPORT QObject
	{
	public:
		virtual const QMetaObject* metaObject() const = 0;
		virtual void* qt_metacast(const char* class_name) = 0;
		virtual int qt_metacall(int call, int id, void** args) = 0;
		virtual ~QObject() = default;
		virtual bool event(void* e) = 0;
		virtual bool eventFilter(void* watched, void* e) = 0;
		virtual void timerEvent(void* e) = 0;
		virtual void childEvent(void* e) = 0;
		virtual void customEvent(void* e) = 0;
		virtual void connectNotify(const void* signal) = 0;
		virtual void disconnectNotify(const void* signal) = 0;

		[[nodiscard]] const char* class_name() const;
		[[nodiscard]] bool inherits(const char* class_name) const;

		[[nodiscard]] void* handle()
		{
			return this;
		}
		[[nodiscard]] const void* handle() const
		{
			return this;
		}

	protected:
		void* d_ptr;

	private:
		RML_LAYOUT_GUARD_BEGIN()
			RML_ASSERT_LAYOUT_OFFSET(QObject, d_ptr, sizeof(void*));
		RML_LAYOUT_GUARD_END()
	};
}
