#pragma once
namespace RBX {

	// Fail:	Stop the hit test - don't bore any further down
	// Ignore:	Keep testing
	// Hit:		Found something

	struct RobloxHitTestFilter {
	public:
		void* _vtable;
	};

} // namespace