#include "RobloxModLoader/memory/module.hpp"

#include "RobloxModLoader/internal/common.hpp"
namespace rml::memory
{

	namespace
	{
#if defined(RML_WINDOWS)
		std::string win32_error_string(const DWORD err)
		{
			char buf[512] = {};

			if (const DWORD len = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, static_cast<DWORD>(sizeof buf), nullptr); len > 0)
			{
				std::string s(buf, len);
				while (!s.empty() && (s.back() == '\n' || s.back() == '\r'))
					s.pop_back();
				return s;
			}
			return std::format("(Win32 error {})", static_cast<unsigned>(err));
		}
#endif
	}

	module::module(std::string_view name) :range(nullptr, 0), m_name(name)
	{
		std::scoped_lock lk(m_mtx);
		try_get_module_locked();
	}

	module::module(std::filesystem::path path) :range(nullptr, 0), m_name(path.filename().string()), m_path(std::move(path))
	{
		std::scoped_lock lk(m_mtx);
		try_get_module_locked();
	}

	bool module::loaded() const noexcept
	{
		std::scoped_lock lk(m_mtx);
		return m_loaded;
	}

	size_t module::size() const noexcept
	{
		std::scoped_lock lk(m_mtx);
		return m_size;
	}

	handle module::get_export(std::string_view symbol_name) const
	{
		std::scoped_lock lk(m_mtx);
		if (!m_loaded)
			return handle(nullptr);

#if defined(RML_WINDOWS)
		const HMODULE mod = m_attached_handle ? static_cast<HMODULE>(m_attached_handle) : GetModuleHandleA(m_name.c_str());

		if (!mod)
			return handle(nullptr);

		return handle(reinterpret_cast<void*>(GetProcAddress(mod, symbol_name.data())));

#else
		void* sym = nullptr;

		if (m_attached_handle)
		{
			sym = dlsym(m_attached_handle, symbol_name.data());
		}
		else
		{
			void* h = dlopen(m_name.c_str(), RTLD_NOW | RTLD_NOLOAD);
			if (h)
			{
				sym = dlsym(h, symbol_name.data());
				dlclose(h);
			}
			else
			{
				sym = dlsym(RTLD_DEFAULT, symbol_name.data());
			}
		}

		return handle(sym);
#endif
	}

	bool module::wait_for_module(std::optional<std::chrono::steady_clock::duration> timeout)
	{
		using clock = std::chrono::steady_clock;

		const auto deadline = timeout ? std::make_optional(clock::now() + *timeout) : std::nullopt;

		LOG_DEBUG("Waiting for {}...", m_name);

		while (true)
		{
			{
				std::scoped_lock lk(m_mtx);
				if (try_get_module_locked())
					return true;
			}

			if (deadline && clock::now() >= *deadline)
				break;

			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		LOG_DEBUG("Timed out waiting for {}", m_name);
		return false;
	}

	std::expected<void, std::string> module::attach()
	{
		std::scoped_lock lk(m_mtx);

		if (m_attached_handle)
			return std::unexpected(std::format("Module '{}' already attached", m_name));

#if defined(RML_WINDOWS)
		HMODULE loaded = nullptr;

		if (m_path)
		{
			const std::string abs_path_str = std::filesystem::absolute(*m_path).string();
			const std::wstring abs_path_w = std::filesystem::absolute(*m_path).wstring();
			const std::wstring dll_dir_w = m_path->parent_path().wstring();

			const auto h_module = LoadLibraryExW(abs_path_w.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);

			const DWORD err = GetLastError();
			if (!h_module)
				return std::unexpected(std::format("Failed to attach module '{}' : Failed to load module '{}': {}", m_name, abs_path_str, win32_error_string(err)));

			loaded = h_module;
		}
		else
		{
			loaded = LoadLibraryA(m_name.c_str());
			if (!loaded)
			{
				const DWORD err = GetLastError();
				return std::unexpected(std::format("Failed to attach module '{}': {}", m_name, win32_error_string(err)));
			}
		}

		m_attached_handle = static_cast<void*>(loaded);
		m_attached_owner = true;
		m_loaded = true;
		m_base = handle(loaded);

		const auto* dos = m_base.as<const IMAGE_DOS_HEADER*>();
		const auto* nt = m_base.add(dos->e_lfanew).as<const IMAGE_NT_HEADERS*>();
		m_size = nt->OptionalHeader.SizeOfImage;

		return {};

#else
		const std::string load_target = m_path ? m_path->string() : m_name;

		const bool already_loaded = (dlopen(load_target.c_str(), RTLD_NOW | RTLD_NOLOAD) != nullptr);

		void* h = dlopen(load_target.c_str(), RTLD_NOW | RTLD_LOCAL);
		if (!h)
		{
			const char* err = dlerror();
			return std::unexpected(std::format("Failed to attach module '{}': {}", m_name, err ? err : "(unknown dlerror)"));
		}

		m_attached_handle = h;
		m_attached_owner = !already_loaded;

		try_get_module_locked();
		return {};
#endif
	}

	std::expected<void, std::string> module::detach()
	{
		std::scoped_lock lk(m_mtx);

		if (!m_attached_handle)
			return {};

#if defined(RML_WINDOWS)
		if (m_attached_owner && !FreeLibrary(static_cast<HMODULE>(m_attached_handle)))
		{
			const DWORD err = GetLastError();
			return std::unexpected(std::format("Failed to detach module '{}': {}", m_name, win32_error_string(err)));
		}
#else
		if (m_attached_owner && dlclose(m_attached_handle) != 0)
		{
			const char* err = dlerror();
			return std::unexpected(std::format("Failed to detach module '{}': {}", m_name, err ? err : "(unknown dlerror)"));
		}
#endif

		m_attached_handle = nullptr;
		m_attached_owner = false;
		reset_state_locked();
		return {};
	}

	void module::reset_state_locked() noexcept
	{
		m_loaded = false;
		m_base = handle(nullptr);
		m_size = 0;
	}

#if defined(RML_LINUX)
	int module::phdr_callback(struct ::dl_phdr_info* info, size_t /*sz*/, void* data) noexcept
	{
		auto* ctx = static_cast<PhdrSearchCtx*>(data);

		if (!info->dlpi_name || !ctx->search_name)
			return 0;
		if (!strstr(info->dlpi_name, ctx->search_name))
			return 0;

		std::uintptr_t min_vaddr = UINTPTR_MAX;
		std::uintptr_t max_vaddr = 0;

		for (int i = 0; i < info->dlpi_phnum; ++i)
		{
			const ElfW(Phdr) & ph = info->dlpi_phdr[i];
			if (ph.p_type != PT_LOAD)
				continue;

			const auto start = static_cast<std::uintptr_t>(ph.p_vaddr);
			const auto end = start + static_cast<std::uintptr_t>(ph.p_memsz);
			min_vaddr = std::min(min_vaddr, start);
			max_vaddr = std::max(max_vaddr, end);
		}

		if (min_vaddr == UINTPTR_MAX)
			min_vaddr = 0;

		ctx->found_base = static_cast<std::uintptr_t>(info->dlpi_addr) + min_vaddr;
		ctx->found_size = max_vaddr > min_vaddr ? (max_vaddr - min_vaddr) : 0;
		return 1;
	}
#endif

	bool module::try_get_module_locked()
	{
		if (m_loaded)
			return true;

#if defined(RML_WINDOWS)
		const HMODULE mod = GetModuleHandleA(m_name.c_str());
		if (!mod)
			return false;

		m_base = handle(mod);
		m_loaded = true;

		const auto* dos = m_base.as<const IMAGE_DOS_HEADER*>();
		const auto* nt = m_base.add(dos->e_lfanew).as<const IMAGE_NT_HEADERS*>();
		m_size = nt->OptionalHeader.SizeOfImage;

		return true;

#elif defined(RML_LINUX)
		PhdrSearchCtx ctx{m_name.c_str(), 0, 0};
		dl_iterate_phdr(phdr_callback, &ctx);

		if (ctx.found_base != 0)
		{
			m_loaded = true;
			m_base = handle(static_cast<std::uintptr_t>(ctx.found_base));
			m_size = ctx.found_size;
			return true;
		}

		void* h = dlopen(m_name.c_str(), RTLD_NOW | RTLD_NOLOAD);
		if (!h)
			return false;

		dlclose(h);
		m_loaded = true;
		m_base = handle(nullptr);
		m_size = 0;
		return true;

#elif defined(RML_MACOS)
		const uint32_t image_count = _dyld_image_count();

		for (uint32_t i = 0; i < image_count; ++i)
		{
			const char* image_name = _dyld_get_image_name(i);
			if (!image_name || !strstr(image_name, m_name.c_str()))
				continue;

			const auto* mh = reinterpret_cast<const mach_header_64*>(_dyld_get_image_header(i));
			const intptr_t slide = _dyld_get_image_vmaddr_slide(i);

			if (mh->magic != MH_MAGIC_64)
				continue;

			const auto* cmd = reinterpret_cast<const load_command*>(mh + 1);

			std::uintptr_t min_addr = UINTPTR_MAX;
			std::uintptr_t max_addr = 0;

			for (uint32_t ci = 0; ci < mh->ncmds; ++ci)
			{
				if (cmd->cmd == LC_SEGMENT_64)
				{
					const auto* seg = reinterpret_cast<const segment_command_64*>(cmd);
					if (seg->vmsize > 0)
					{
						const auto start = static_cast<std::uintptr_t>(seg->vmaddr) + static_cast<std::uintptr_t>(slide);
						const auto end = start + static_cast<std::uintptr_t>(seg->vmsize);
						min_addr = std::min(min_addr, start);
						max_addr = std::max(max_addr, end);
					}
				}
				cmd = reinterpret_cast<const load_command*>(reinterpret_cast<const char*>(cmd) + cmd->cmdsize);
			}

			if (min_addr == UINTPTR_MAX)
				min_addr = reinterpret_cast<std::uintptr_t>(mh) + static_cast<std::uintptr_t>(slide);

			m_loaded = true;
			m_base = handle(reinterpret_cast<void*>(min_addr));
			m_size = max_addr > min_addr ? (max_addr - min_addr) : 0;
			return true;
		}

		return false;

#else
		return false;
#endif
	}

} // namespace memory