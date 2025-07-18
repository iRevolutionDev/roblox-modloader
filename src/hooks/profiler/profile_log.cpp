#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/hooking/hooking.hpp"

#include "tracy/TracyC.h"
#include <unordered_map>
#include <mutex>
#include <thread>
#include <stack>
#include <shared_mutex>
#include <atomic>
#include <array>

#include "tracy/Tracy.hpp"

typedef uint64_t MicroProfileToken;
typedef uint16_t MicroProfileGroupId;

#define MICROPROFILE_MAX_COUNTERS 512
#define MICROPROFILE_MAX_COUNTER_NAME_CHARS (MICROPROFILE_MAX_COUNTERS*16)
#define MICROPROFILE_MAX_TIMERS 1024
#define MICROPROFILE_MAX_GROUPS 48
#define MICROPROFILE_MAX_CATEGORIES 16
#define MICROPROFILE_MAX_GRAPHS 5
#define MICROPROFILE_GRAPH_HISTORY 128
#define MICROPROFILE_BUFFER_SIZE ((MICROPROFILE_PER_THREAD_BUFFER_SIZE)/sizeof(MicroProfileLogEntry))
#define MICROPROFILE_MAX_CONTEXT_SWITCH_THREADS 256
#define MICROPROFILE_STACK_MAX 32
#define MICROPROFILE_ANIM_DELAY_PRC 0.5f
#define MICROPROFILE_GAP_TIME 50

#define MICROPROFILE_NAME_MAX_LEN 64

#define MP_LOG_GPU_EXTRA 0x4
#define MP_LOG_LABEL 0x3
#define MP_LOG_META 0x2
#define MP_LOG_ENTER 0x1
#define MP_LOG_LEAVE 0x0

enum MicroProfileTokenType {
	MicroProfileTokenTypeCpu,
	MicroProfileTokenTypeGpu,
};

struct MicroProfileCategory {
	char name[MICROPROFILE_NAME_MAX_LEN];
	uint64_t nGroupMask;
	uint64_t nGroupMask2;
	uint64_t nGroupMask3;
};

struct MicroProfileGroupInfo {
	char name[MICROPROFILE_NAME_MAX_LEN];
	uint32_t name_len;
	uint32_t group_index;
	uint32_t num_timers;
	uint32_t max_timer_name_len;
	uint32_t color;
	uint32_t category;
	MicroProfileTokenType Type;
};

enum MicroProfileDumpType {
	MicroProfileDumpTypeHtml,
	MicroProfileDumpTypeCsv
};

struct MicroProfileTimerInfo {
	MicroProfileToken token;
	uint32_t timer_index;
	uint32_t group_index;
	char name[MICROPROFILE_NAME_MAX_LEN];
	uint32_t name_len;
	uint32_t color;
	bool graph;
};

struct MicroProfile {
	char padding[0x738];

	MicroProfileGroupInfo group_info[MICROPROFILE_MAX_GROUPS];
	MicroProfileTimerInfo timer_info[MICROPROFILE_MAX_TIMERS];
	uint8_t timer_to_group[MICROPROFILE_MAX_TIMERS];
};

inline uint16_t get_timer_index(const MicroProfileToken token) { return token & 0xffff; }

std::string get_profile_name_from_token(const MicroProfileToken token) {
	const auto p_profile = reinterpret_cast<MicroProfile *>(
		reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr)) + 0xA6CB9E8);

	if (const auto timer_index = get_timer_index(token); timer_index < MICROPROFILE_MAX_TIMERS) {
		return std::string(p_profile->timer_info[timer_index].name, p_profile->timer_info[timer_index].name_len);
	}

	return {};
}

MicroProfileTimerInfo get_profile_from_token(const MicroProfileToken token) {
	const auto p_profile = reinterpret_cast<MicroProfile *>(
		reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr)) + 0xA6CB9E8);

	if (const auto timer_index = get_timer_index(token); timer_index < MICROPROFILE_MAX_TIMERS) {
		return p_profile->timer_info[timer_index];
	}

	return {};
}

MicroProfileGroupInfo get_group_from_index(const uint32_t group_index) {
	const auto p_profile = reinterpret_cast<MicroProfile *>(
		reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr)) + 0xA6CB9E8);

	if (group_index < MICROPROFILE_MAX_GROUPS) {
		return p_profile->group_info[group_index];
	}

	return {};
}

#define MP_LOG_TICK_MASK  0x0000ffffffffffff
#define MP_LOG_INDEX_MASK 0x1fff000000000000
#define MP_LOG_BEGIN_MASK 0xe000000000000000

struct ProfileCache {
	MicroProfileToken token;
	std::string name;
	bool valid;
	uint32_t color;
};

static thread_local ProfileCache t_profile_cache = {0, "", false};
static thread_local uint64_t t_thread_id_hash = 0;

constexpr size_t MAX_CONTEXTS_PER_THREAD = 64;

struct ThreadLocalContexts {
	std::array<TracyCZoneCtx, MAX_CONTEXTS_PER_THREAD> contexts;
	std::array<uint64_t, MAX_CONTEXTS_PER_THREAD> keys;
	std::atomic<size_t> depth{0};

	void push_context(const uint64_t key, const TracyCZoneCtx ctx) {
		if (const auto current_depth = depth.load(std::memory_order_relaxed); current_depth < MAX_CONTEXTS_PER_THREAD) {
			contexts[current_depth] = ctx;
			keys[current_depth] = key;
			depth.store(current_depth + 1, std::memory_order_release);
		}
	}

	std::optional<TracyCZoneCtx> pop_context(const uint64_t key) {
		if (const auto current_depth = depth.load(std::memory_order_relaxed); current_depth > 0) {
			if (const auto new_depth = current_depth - 1; keys[new_depth] == key) {
				depth.store(new_depth, std::memory_order_release);
				return contexts[new_depth];
			}
		}
		return std::nullopt;
	}

	size_t get_depth() const {
		return depth.load(std::memory_order_relaxed);
	}
};

static thread_local ThreadLocalContexts t_contexts;

uintptr_t hooks::profile_log(uintptr_t token, uint64_t tick, uint64_t begin, uintptr_t *log) {
	const auto result = hooking::get_original<&hooks::profile_log>()(token, tick, begin, log);

	if (!token) {
		return result;
	}

	// Ignore sleep token
	if (token == 131077) {
		return result;
	}

	// GPU extra doesn't support yet ¯\_(ツ)_/¯
	if (begin == MP_LOG_GPU_EXTRA || begin == MP_LOG_LABEL || begin == MP_LOG_META) {
		return result;
	}

	try {
		if (t_thread_id_hash == 0) {
			t_thread_id_hash = std::hash<std::thread::id>{}(std::this_thread::get_id());
		}

		const auto context_key = (t_thread_id_hash << 32) | static_cast<uint32_t>(token);

		if (begin == MP_LOG_LEAVE) {
			if (const auto ctx = t_contexts.pop_context(context_key)) {
				TracyCZoneEnd(ctx.value());
			}
			return result;
		}

		if (begin == MP_LOG_ENTER) {
			if (t_contexts.get_depth() > 32) {
				return result;
			}

			if (t_profile_cache.token != token || !t_profile_cache.valid) {
				const auto profile = get_profile_from_token(token);
				if (profile.name_len == 0 || profile.name_len > 256) {
					return result;
				}

				t_profile_cache.token = token;
				t_profile_cache.name.assign(profile.name, profile.name_len);
				t_profile_cache.valid = true;
				t_profile_cache.color = profile.color;
			}

			if (t_profile_cache.name.empty()) {
				return result;
			}

			//TracyCZone(ctx, true)
			/*TracyCZoneName(ctx, t_profile_cache.name.c_str(), t_profile_cache.name.size());
			TracyCZoneColor(ctx, t_profile_cache.color);

			t_contexts.push_context(context_key, ctx);*/
		}
	} catch (...) {
		return result;
	}

	return result;
}
