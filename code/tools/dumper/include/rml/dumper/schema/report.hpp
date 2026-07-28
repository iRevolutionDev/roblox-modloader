#pragma once

#include <string>
#include <vector>

namespace rml::dumper::schema
{
	class Report
	{
	public:
		struct Entry
		{
			std::string struct_name;
			std::string field_name;
			std::string reason;
		};

		void record_recovered(std::string struct_name, std::string field_name, std::string probe);
		void record_fixed(std::string struct_name, std::string field_name, std::string reason);
		void record_failure(std::string struct_name, std::string field_name, std::string reason);
		void record_note(std::string struct_name, std::string reason);

		[[nodiscard]] std::size_t recovered_count() const { return m_recovered.size(); }
		[[nodiscard]] std::size_t fixed_count() const { return m_fixed.size(); }

		[[nodiscard]] const std::vector<Entry>& recovered() const { return m_recovered; }
		[[nodiscard]] const std::vector<Entry>& fixed() const { return m_fixed; }
		[[nodiscard]] const std::vector<Entry>& failures() const { return m_failures; }
		[[nodiscard]] const std::vector<Entry>& notes() const { return m_notes; }

		[[nodiscard]] bool has_failures() const { return !m_failures.empty(); }
		[[nodiscard]] std::string summary() const;

	private:
		std::vector<Entry> m_recovered;
		std::vector<Entry> m_fixed;
		std::vector<Entry> m_failures;
		std::vector<Entry> m_notes;
	};
}
