#include "editor_overlay.hpp"

#include <RobloxModLoader/memory/vtable.hpp>
#include <RobloxModLoader/qt/qapplication.hpp>
#include <RobloxModLoader/qt/qobject.hpp>
#include <RobloxModLoader/qt/qpainter.hpp>
#include <RobloxModLoader/qt/qrect.hpp>
#include <RobloxModLoader/qt/qwidget.hpp>
#include <algorithm>
#include <string_view>
#include <utility>
#include <windows.h>

namespace script_editor_bg
{
	EditorOverlay* g_active = nullptr;

	void** vtable_of(void* object)
	{
		return *static_cast<void***>(object);
	}

	bool is_script_editor(const rml::qt::QObject* widget)
	{
		const std::string_view name = widget->class_name();
		return name == "StudioScriptEditor" || name == "RBX::ScriptEditor::ScriptEditor";
	}

	std::pair<int, int> anchor(const Alignment alignment, const int w, const int h, const int vw, const int vh)
	{
		int x = (vw - w) / 2;
		int y = (vh - h) / 2;

		switch (alignment)
		{
		case Alignment::TopLeft:
			x = 0;
			y = 0;
			break;
		case Alignment::Top: y = 0; break;
		case Alignment::TopRight:
			x = vw - w;
			y = 0;
			break;
		case Alignment::Left: x = 0; break;
		case Alignment::Right: x = vw - w; break;
		case Alignment::BottomLeft:
			x = 0;
			y = vh - h;
			break;
		case Alignment::Bottom: y = vh - h; break;
		case Alignment::BottomRight:
			x = vw - w;
			y = vh - h;
			break;
		case Alignment::Center: break;
		}

		return {x, y};
	}

	EditorOverlay::EditorOverlay(std::filesystem::path mod_directory) :
	    m_mod_directory(std::move(mod_directory))
	{
		m_settings.store(std::make_shared<const BackgroundSettings>(), std::memory_order_release);
		g_active = this;
	}

	EditorOverlay::~EditorOverlay()
	{
		unhook_all();
		if (g_active == this)
			g_active = nullptr;
	}

	std::size_t EditorOverlay::paint_slot()
	{
		static const std::size_t slot = memory::virtual_index(&rml::qt::QWidget::paintEvent);
		return slot;
	}

	void EditorOverlay::apply(const BackgroundSettings& settings)
	{
		m_settings.store(std::make_shared<const BackgroundSettings>(settings), std::memory_order_release);
	}

	void EditorOverlay::hook_open_editors()
	{
		for (rml::qt::QWidget* editor : rml::qt::QApplication::all_widgets())
			if (is_script_editor(editor))
				hook_editor_class(editor->handle());
	}

	void EditorOverlay::repaint_editors()
	{
		for (rml::qt::QWidget* editor : rml::qt::QApplication::all_widgets())
			if (is_script_editor(editor))
				editor->update();
	}

	void EditorOverlay::hook_editor_class(void* editor_handle)
	{
		if (paint_slot() == static_cast<std::size_t>(-1))
			return;

		void** const vtable = vtable_of(editor_handle);
		if (m_originals.contains(vtable))
			return;

		void** const slot = vtable + paint_slot();
		DWORD protection = 0;
		if (!VirtualProtect(slot, sizeof(void*), PAGE_READWRITE, &protection))
			return;

		m_originals.emplace(vtable, reinterpret_cast<paint_fn>(*slot));
		*slot = reinterpret_cast<void*>(&paint_detour);
		VirtualProtect(slot, sizeof(void*), protection, &protection);
	}

	void EditorOverlay::unhook_all()
	{
		for (const auto& [vtable, original] : m_originals)
		{
			void** const slot = vtable + paint_slot();
			DWORD protection = 0;
			if (!VirtualProtect(slot, sizeof(void*), PAGE_READWRITE, &protection))
				continue;
			*slot = reinterpret_cast<void*>(original);
			VirtualProtect(slot, sizeof(void*), protection, &protection);
		}
		m_originals.clear();
	}

	std::size_t EditorOverlay::hooked_class_count() const
	{
		return m_originals.size();
	}

	void EditorOverlay::paint_detour(void* self, void* event)
	{
		EditorOverlay* const overlay = g_active;

		paint_fn original = nullptr;
		if (overlay)
			if (const auto it = overlay->m_originals.find(vtable_of(self)); it != overlay->m_originals.end())
				original = it->second;

		if (overlay)
			overlay->render(self);

		if (original)
			original(self, event);
	}

	void EditorOverlay::render(void* editor) const
	{
		const std::shared_ptr<const BackgroundSettings> settings = m_settings.load(std::memory_order_acquire);
		if (!settings || !settings->enabled)
			return;

		std::filesystem::path image_path(settings->image);
		if (image_path.is_relative())
			image_path = m_mod_directory / image_path;

		if (const std::string key = image_path.generic_string(); key != m_pixmap_key)
		{
			m_pixmap = rml::qt::QPixmap(image_path.generic_string());
			m_pixmap_key = key;
		}

		if (!m_pixmap.loaded())
			return;

		const auto* const widget = static_cast<rml::qt::QWidget*>(editor);
		const rml::qt::QWidget* const viewport = widget->viewport();
		if (!viewport)
			return;

		const int vw = viewport->width();
		const int vh = viewport->height();
		const int pw = m_pixmap.width();
		const int ph = m_pixmap.height();
		if (vw <= 0 || vh <= 0 || pw <= 0 || ph <= 0)
			return;

		rml::qt::QPainter painter(*viewport);
		painter.set_opacity(settings->opacity);

		switch (settings->scale_mode)
		{
		case ScaleMode::Stretch:
		{
			painter.draw_pixmap(rml::qt::QRect(0, 0, vw, vh), m_pixmap);
			break;
		}
		case ScaleMode::Tile:
		{
			for (int y = 0; y < vh; y += ph)
				for (int x = 0; x < vw; x += pw)
					painter.draw_pixmap(x, y, m_pixmap);
			break;
		}
		case ScaleMode::Center:
		{
			const auto [x, y] = anchor(settings->alignment, pw, ph, vw, vh);
			painter.draw_pixmap(x, y, m_pixmap);
			break;
		}
		case ScaleMode::Fit:
		case ScaleMode::Fill:
		{
			const double sx = static_cast<double>(vw) / pw;
			const double sy = static_cast<double>(vh) / ph;
			const double scale = settings->scale_mode == ScaleMode::Fit ? std::min(sx, sy) : std::max(sx, sy);
			const int tw = std::max(1, static_cast<int>(std::lround(pw * scale)));
			const int th = std::max(1, static_cast<int>(std::lround(ph * scale)));
			const auto [x, y] = anchor(settings->alignment, tw, th, vw, vh);
			painter.draw_pixmap(rml::qt::QRect(x, y, tw, th), m_pixmap);
			break;
		}
		}
	}
}
