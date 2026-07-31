#pragma once

#include <chrono>
#include <filesystem>
#include <functional>
#include <stop_token>
#include <string>
#include <thread>
#include <vector>

namespace rml::filesystem
{
	class DirectoryWatcher
	{
	public:
		struct Root
		{
			std::string label;
			std::filesystem::path path;
		};

		struct Options
		{
			std::chrono::milliseconds poll_interval{500};
			std::chrono::milliseconds settle{250};
			std::vector<std::string> extensions;
		};

		using Callback = std::function<void(const std::string& label)>;

		DirectoryWatcher() = default;
		~DirectoryWatcher();

		DirectoryWatcher(const DirectoryWatcher&) = delete;
		DirectoryWatcher& operator=(const DirectoryWatcher&) = delete;
		DirectoryWatcher(DirectoryWatcher&&) = delete;
		DirectoryWatcher& operator=(DirectoryWatcher&&) = delete;

		void start(std::vector<Root> roots, Options options, Callback on_changed);
		void stop();

		[[nodiscard]] bool running() const noexcept { return m_thread.joinable(); }

	private:
		struct Tracked
		{
			Root root;
			std::size_t fingerprint{0};
			bool dirty{false};
			std::chrono::steady_clock::time_point changed_at{};
		};

		static std::size_t fingerprint_of(const std::filesystem::path& root, const std::vector<std::string>& extensions);

		static void run(const std::stop_token& stop_token, std::vector<Tracked> tracked, const Options& options,
		                const Callback& on_changed);

		std::jthread m_thread;
	};
}
