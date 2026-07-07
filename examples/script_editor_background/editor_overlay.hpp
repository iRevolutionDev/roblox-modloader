#pragma once

#include "settings.hpp"

#include <RobloxModLoader/qt/qmovie.hpp>
#include <RobloxModLoader/qt/qpixmap.hpp>
#include <atomic>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

namespace script_editor_bg
{
	class EditorOverlay
	{
	public:
		explicit EditorOverlay(std::filesystem::path mod_directory);
		~EditorOverlay();

		EditorOverlay(const EditorOverlay&) = delete;
		EditorOverlay& operator=(const EditorOverlay&) = delete;

		void apply(const BackgroundSettings& settings);
		void hook_open_editors();
		static void repaint_editors();
		void unhook_all();

		[[nodiscard]] std::size_t hooked_class_count() const;

	private:
		using paint_fn = void (*)(void* self, void* event);

		static void paint_detour(void* self, void* event);
		void render(void* editor) const;

		void ensure_source(const BackgroundSettings& settings) const;
		void rebuild_display(const BackgroundSettings& settings) const;
		void teardown_source() const;
		void on_movie_frame() const;

		[[nodiscard]] static bool is_animated_extension(const std::filesystem::path& path);
		[[nodiscard]] static rml::qt::QPixmap blur_pixmap(const rml::qt::QPixmap& source, double blur);

		[[nodiscard]] static std::size_t paint_slot();
		void hook_editor_class(void* editor_handle);

		std::filesystem::path m_mod_directory;
		std::atomic<std::shared_ptr<const BackgroundSettings>> m_settings;

		std::unordered_map<void**, paint_fn> m_originals;

		mutable std::string m_source_key;
		mutable bool m_animated{false};
		mutable rml::qt::QtOwned<rml::qt::QMovie> m_movie;
		mutable rml::qt::QPixmap m_frame;
		mutable rml::qt::QPixmap m_display;
		mutable double m_display_blur{-1.0};
		mutable bool m_frame_dirty{true};
	};
}
