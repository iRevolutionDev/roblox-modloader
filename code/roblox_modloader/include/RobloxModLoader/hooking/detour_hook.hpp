#pragma once

#include <expected>
#include <string>

#include "RobloxModLoader/hooking/i_hook.hpp"
#include "RobloxModLoader/rml_export.hpp"

class RML_EXPORT detour_hook : public rml::IHook {
public:
    explicit detour_hook();

    explicit detour_hook(const std::string &name, void *detour);

    explicit detour_hook(const std::string &name, void *target, void *detour);

    ~detour_hook() noexcept override;

    detour_hook(detour_hook &&that) = delete;

    detour_hook &operator=(detour_hook &&that) = delete;

    detour_hook(detour_hook const &) = delete;

    detour_hook &operator=(detour_hook const &) = delete;

    void set_instance(const std::string &name, void *detour);

    void set_instance(const std::string &name, void *target, void *detour);

    void set_target_and_create_hook(void *target);

    [[nodiscard]] std::expected<void, rml::HookError> enable() override;

    [[nodiscard]] std::expected<void, rml::HookError> disable() override;

    [[nodiscard]] bool is_enabled() const override;

    template<typename T>
    T get_original() {
        return static_cast<T>(m_original);
    }

    void **get_original_ptr() {
        return &m_original;
    }

    void fix_hook_address();

private:
    void create_hook();

    std::string m_name;
    void *m_original{};
    void *m_target{};
    void *m_detour{};
    bool m_enabled{};
};
