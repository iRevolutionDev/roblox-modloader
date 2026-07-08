#include "file_watcher.hpp"

namespace rml::utils
{
	using namespace std::chrono_literals;

	std::filesystem::file_time_type read_last_write_time(const std::filesystem::path& path)
	{
		std::error_code ec;
		const auto time = std::filesystem::last_write_time(path, ec);
		return ec ? std::filesystem::file_time_type{} : time;
	}

	FileWatcher::~FileWatcher()
	{
		stop();
	}

	void FileWatcher::start(std::filesystem::path path, const std::chrono::milliseconds poll_interval, Callback on_changed)
	{
		stop();

		{
			std::lock_guard lock(m_baseline_mutex);
			m_baseline = read_last_write_time(path);
		}

		m_thread = std::jthread([this, path = std::move(path), poll_interval, on_changed = std::move(on_changed)](const std::stop_token& stop_token) {
			run(stop_token, path, poll_interval, on_changed);
		});
	}

	void FileWatcher::stop()
	{
		if (!m_thread.joinable())
			return;

		m_thread.request_stop();
		m_thread.join();
	}

	void FileWatcher::acknowledge(const std::filesystem::file_time_type write_time)
	{
		std::lock_guard lock(m_baseline_mutex);
		m_baseline = write_time;
	}

	void FileWatcher::run(const std::stop_token& stop_token, const std::filesystem::path& path, const std::chrono::milliseconds poll_interval, const Callback& on_changed)
	{
		constexpr auto step = 125ms;

		while (!stop_token.stop_requested())
		{
			for (auto elapsed = 0ms; elapsed < poll_interval && !stop_token.stop_requested(); elapsed += step)
				std::this_thread::sleep_for(step);

			if (stop_token.stop_requested())
				break;

			const auto current = read_last_write_time(path);

			std::filesystem::file_time_type baseline;
			{
				std::lock_guard lock(m_baseline_mutex);
				baseline = m_baseline;
			}

			if (current == std::filesystem::file_time_type{} || current == baseline)
				continue;

			if (on_changed && on_changed())
			{
				std::lock_guard lock(m_baseline_mutex);
				if (m_baseline < current)
					m_baseline = current;
			}
		}
	}
}
