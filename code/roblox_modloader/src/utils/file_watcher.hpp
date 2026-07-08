#pragma once

#include <chrono>
#include <filesystem>
#include <functional>
#include <mutex>
#include <stop_token>
#include <thread>

namespace rml::utils
{
	std::filesystem::file_time_type read_last_write_time(const std::filesystem::path& path);

	class FileWatcher
	{
	public:
		using Callback = std::function<bool()>;

		FileWatcher() = default;
		~FileWatcher();

		FileWatcher(const FileWatcher&) = delete;
		FileWatcher& operator=(const FileWatcher&) = delete;

		FileWatcher(FileWatcher&&) = delete;
		FileWatcher& operator=(FileWatcher&&) = delete;

		void start(std::filesystem::path path, std::chrono::milliseconds poll_interval, Callback on_changed);
		void stop();

		void acknowledge(std::filesystem::file_time_type write_time);

	private:
		void run(const std::stop_token& stop_token, const std::filesystem::path& path, std::chrono::milliseconds poll_interval, const Callback& on_changed);

		std::jthread m_thread;
		std::mutex m_baseline_mutex;
		std::filesystem::file_time_type m_baseline{};
	};
}
