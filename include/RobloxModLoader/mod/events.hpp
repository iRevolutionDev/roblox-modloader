#pragma once
#include "RobloxModLoader/common.hpp"
#include <functional>
#include <vector>
#include <memory>
#include <unordered_map>

namespace events {
    struct EventBase {
        virtual ~EventBase() = default;

        bool cancelled = false;
    };

    struct AuthenticationEvent final : EventBase {
        uint64_t *thisPtr;
        uint64_t docPanelProvider;
        uint64_t qImageProvider;

        AuthenticationEvent(uint64_t *_this, const uint64_t doc_panel, const uint64_t q_image)
            : thisPtr(_this), docPanelProvider(doc_panel), qImageProvider(q_image) {
        }
    };

    class EventManager {
    public:
        EventManager();
        ~EventManager();

        template<typename T>
        using EventHandler = std::function<void(T &)>;

        template<typename T>
        void registerHandler(EventHandler<T> handler) {
            auto wrappedHandler = [handler](std::shared_ptr<EventBase> e) {
                if (auto event = std::dynamic_pointer_cast<T>(e)) {
                    handler(*event);
                }
            };
            handlers[typeid(T).hash_code()].push_back(wrappedHandler);
        }

        template<typename T>
        void emit(T &event) {
            auto baseEvent = std::make_shared<T>(event);
            const auto typeHash = typeid(T).hash_code();
            if (const auto it = handlers.find(typeHash); it != handlers.end()) {
                for (const auto &handler: it->second) {
                    if (!event.cancelled) {
                        handler(baseEvent);
                    }
                }
            }
        }

    private:
        using HandlerFunc = std::function<void(std::shared_ptr<EventBase>)>;
        std::unordered_map<size_t, std::vector<HandlerFunc> > handlers;
    };

    inline EventManager *g_event_manager{};
}
