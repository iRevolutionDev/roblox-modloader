#pragma once

#include "rml/dumper/core/error.hpp"
#include "rml/dumper/schema/layout_set.hpp"

#include <expected>
#include <memory>
#include <ostream>
#include <string_view>
#include <vector>

namespace rml::dumper::emit
{
	class Emitter
	{
	public:
		virtual ~Emitter() = default;

		[[nodiscard]] virtual std::string_view id() const = 0;
		[[nodiscard]] virtual std::string_view default_filename() const = 0;
		[[nodiscard]] virtual std::expected<void, Error> emit(const schema::LayoutSet& layouts,
		                                                      std::ostream& out) const = 0;
	};

	class TextEmitter : public Emitter
	{
	protected:
		static void write_banner(std::ostream& out, const schema::LayoutSet& layouts, std::string_view comment);
	};

	class EmitterRegistry
	{
	public:
		[[nodiscard]] std::unique_ptr<Emitter> create(std::string_view id) const;
		[[nodiscard]] std::vector<std::unique_ptr<Emitter>> create_all() const;
		[[nodiscard]] static std::vector<std::string_view> ids();
	};
}
