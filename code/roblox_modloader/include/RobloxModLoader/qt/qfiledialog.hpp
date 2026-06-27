#pragma once

#include "RobloxModLoader/rml_export.hpp"

#include <string>
#include <string_view>

namespace rml::qt
{
	class QWidget;

	class RML_EXPORT QFileDialog
	{
	public:
		[[nodiscard]] static std::string get_open_file_name(QWidget* parent = nullptr, std::string_view caption = {}, std::string_view dir = {}, std::string_view filter = {});
	};
}
