#include "RobloxModLoader/qt/qfiledialog.hpp"

#include "RobloxModLoader/qt/qstring.hpp"
#include "RobloxModLoader/qt/qt_module.hpp"

namespace rml::qt
{
	std::string QFileDialog::get_open_file_name(QWidget* parent, const std::string_view caption, const std::string_view dir, const std::string_view filter)
	{
		static const auto fn = detail::widgets<void (*)(void*, void*, const void*, const void*, const void*, void*, int)>("?getOpenFileName@QFileDialog@@SA?AVQString@@PEAVQWidget@@AEBV2@11PEAV2@V?$QFlags@W4Option@QFileDialog@@@@@Z");
		if (!fn)
			return {};

		const QString caption_str(caption);
		const QString dir_str(dir);
		const QString filter_str(filter);

		QString result;
		fn(result.storage(), parent, caption_str.data(), dir_str.data(), filter_str.data(), nullptr, 0);
		return result.to_utf8();
	}
}
