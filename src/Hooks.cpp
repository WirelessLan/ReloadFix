#include "Hooks.h"

#include "Utils.h"

namespace Hooks {
	struct BSAnimationGraphEvent {
	public:
		RE::TESObjectREFR* refr;
		RE::BSFixedString name;
		RE::BSFixedString args;
	};

	class BSAnimationGraphManager :
		public RE::BSTEventSink<BSAnimationGraphEvent>,
		public RE::BSIntrusiveRefCounted {
	public:
		// members
		RE::BSTArray<RE::BSTSmartPointer<RE::BSAnimationGraphChannel>>  boundChannels;			// 10
		RE::BSTArray<RE::BSTSmartPointer<RE::BSAnimationGraphChannel>>  bumpedChannels;			// 28
		RE::BSTSmallArray<RE::BSTSmartPointer<RE::BShkbAnimationGraph>> thirdPersonAnimGraphs;	// 40
		RE::BSTArray<uint64_t>											subManagers;			// 58
		RE::BSTArray<uint64_t>											unk70;					// 70
		RE::BSTArray<uint64_t>											unk88;					// 88
		RE::BSTArray<uint64_t>											unkA0;					// A0
		uint64_t														unkB8;					// B8
		RE::BSTSmartPointer<RE::BShkbAnimationGraph>					firstPersonAnimGraph;	// C0
	};

	template <class T>
	class hkArray {
	public:
		T* data;					// 00
		uint32_t size;				// 08
		uint32_t capacity;			// 0C
	};

	class hkStringPtr {
	public:
		// members
		const char* data;			// 00
	};

	class hkbClipGenerator {
	public:
		// members
		uint64_t					unk00[0x38 >> 3];			// 00
		hkStringPtr					animName;					// 38
		uint64_t					unk40[(0x90 - 0x40) >> 3];	// 40
		hkStringPtr					animPath;					// 90
	};

	class hkbBehaviorGraph {
	public:
		struct NodeData {
			hkbClipGenerator* clipGenerator;
			hkbClipGenerator* clipGenerator2;
			hkbBehaviorGraph* behaviorGraph;
		};

		// members
		uint64_t			unk00[0xE0 >> 3];		// 000
		hkArray<NodeData*>* activeNodes;			// 0E0
	};

	class BShkbAnimationGraph {
	public:
		// members
		uint64_t						unk00;							// 000
		RE::BSIntrusiveRefCounted		refCount;						// 008
		RE::BSTEventSource<RE::BSTransformDeltaEvent>	deltaEvent;		// 010
		RE::BSTEventSource<BSAnimationGraphEvent>	animGraphEvent;		// 068
		uint64_t									unkC0[(0x378 - 0xC0) >> 3];		// 0C0
		hkbBehaviorGraph* behaviourGraph;								// 378
	};

	bool IsReloading() {
		if (!Utils::IsWeaponDrawn())
			return false;

		RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
		if (!player || !player->currentProcess || !player->currentProcess->middleHigh || !player->currentProcess->middleHigh->animationGraphManager)
			return false;

		bool isFirstPerson = Utils::IsFirstPerson();
		BSAnimationGraphManager* animGraphManager = (BSAnimationGraphManager*)player->currentProcess->middleHigh->animationGraphManager.get();
		if ((isFirstPerson && !animGraphManager->firstPersonAnimGraph) || animGraphManager->thirdPersonAnimGraphs.empty())
			return false;

		BShkbAnimationGraph* animGraph = (BShkbAnimationGraph*)(isFirstPerson ?	animGraphManager->firstPersonAnimGraph.get() : animGraphManager->thirdPersonAnimGraphs[0].get());
		if (!animGraph)
			return false;

		hkbBehaviorGraph* behaviorGraph = animGraph->behaviourGraph;
		if (!behaviorGraph || !behaviorGraph->activeNodes || behaviorGraph->activeNodes->size == 0)
			return false;

		for (std::uint32_t ii = 0; ii < behaviorGraph->activeNodes->size; ii++) {
			if (!behaviorGraph->activeNodes->data[ii]->clipGenerator || !behaviorGraph->activeNodes->data[ii]->clipGenerator->animName.data)
				continue;

			if (strncmp(behaviorGraph->activeNodes->data[ii]->clipGenerator->animName.data, "WPNReload", strlen("WPNReload")) == 0)
				return true;
		}

		return false;
	}

	bool g_isSprintQueued = false;

	void ClearVariables() {
		g_isSprintQueued = false;
	}

	template<std::uint64_t id, std::ptrdiff_t diff>
	class TogglePOV_FirstToThirdHook {
	public:
		static void Install() {
			REL::Relocation<std::uintptr_t> target(REL::ID(id), diff);
			origFunc = *(func_t*)(target.get());
			REL::safe_write(target.get(), (std::uintptr_t)ProcessHook);
		}
	private:
		using func_t = void (*)(void*, RE::ButtonEvent*);

		static void ProcessHook(void* arg1, RE::ButtonEvent* a_event) {
			if (IsReloading())
				return;

			origFunc(arg1, a_event);
		}

		inline static func_t origFunc;
	};

	template<std::uint64_t id, std::uint64_t minCurrentZoom_id, std::ptrdiff_t diff>
	class TogglePOV_ThirdToFirstHook {
	public:
		static void Install() {
			REL::Relocation<std::uintptr_t> target(REL::ID(id), diff);
			origFunc = *(func_t*)(target.get());
			REL::safe_write(target.get(), (std::uintptr_t)ProcessHook);
		}
	private:
		using func_t = void (*)(RE::ThirdPersonState*, RE::ButtonEvent*);

		static void ProcessHook(RE::ThirdPersonState* a_tpState, RE::ButtonEvent* a_event) {
			if (a_event->value == 0.0f && a_tpState && !a_tpState->freeRotationEnabled)
				a_tpState->freeRotation.x = 0.0f;

			if (IsReloading()) {
				if (a_event->strUserEvent == "TogglePOV") {
					return;
				}
				else if (a_event->strUserEvent == "ZoomIn") {
					float minCurrentZoom = *REL::Relocation<float*>(REL::ID(minCurrentZoom_id));
					if (a_tpState && a_tpState->currentZoomOffset <= minCurrentZoom)
						return;
				}
			}

			origFunc(a_tpState, a_event);
		}

		inline static func_t origFunc;
	};

	template<std::uint64_t id, std::ptrdiff_t diff>
	class ReadyWeaponHandlerHook {
	public:
		static void Install() {
			REL::Relocation<std::uintptr_t> target(REL::ID(id), diff);
			origFunc = *(func_t*)(target.get());
			REL::safe_write(target.get(), (std::uintptr_t)ProcessHook);
		}
	private:
		using func_t = void (*)(void*, RE::ButtonEvent*);

		static void ProcessHook(void* arg1, RE::ButtonEvent* a_event) {
			origFunc(arg1, a_event);

			if (Utils::IsSprinting() && IsReloading()) {
				g_isSprintQueued = true;
				Utils::ToggleSprint(false);
			}
		}

		inline static func_t origFunc;
	};

	template<std::uint64_t id, std::ptrdiff_t diff>
	class MovementHandlerHook {
	public:
		static void Install() {
			REL::Relocation<std::uintptr_t> target(REL::ID(id), diff);
			origFunc = *(func_t*)(target.get());
			REL::safe_write(target.get(), (std::uintptr_t)ProcessHook);
		}
	private:
		using func_t = void (*)(void*, RE::ButtonEvent*);

		static void ProcessHook(void* arg1, RE::ButtonEvent* a_event) {
			if (g_isSprintQueued) {
				if (a_event->strUserEvent != "Forward" || a_event->value == 0.0f)
					g_isSprintQueued = false;
			}

			origFunc(arg1, a_event);
		}

		inline static func_t origFunc;
	};

	template<std::uint64_t id, std::ptrdiff_t diff>
	class SprintHandlerHook {
	public:
		static void Install() {
			REL::Relocation<std::uintptr_t> target(REL::ID(id), diff);
			origFunc = *(func_t*)(target.get());
			REL::safe_write(target.get(), (std::uintptr_t)ProcessHook);
		}
	private:
		using func_t = void (*)(void*, RE::ButtonEvent*);

		static void ProcessHook(void* arg1, RE::ButtonEvent* a_event) {
			if (IsReloading()) {
				g_isSprintQueued = true;
				return;
			}

			origFunc(arg1, a_event);
		}

		inline static func_t origFunc;
	};

	template<std::uint64_t id, std::ptrdiff_t diff>
	class BSAnimationGraphEvent_ProcessEventHook {
	public:
		static void Install() {
			REL::Relocation<std::uintptr_t> target(REL::ID(id), diff);
			origFunc = *(func_t*)(target.get());
			REL::safe_write(target.get(), (std::uintptr_t)ProcessHook);
		}
	private:
		using func_t = RE::BSEventNotifyControl(*)(void*, BSAnimationGraphEvent*, void*);

		static RE::BSEventNotifyControl ProcessHook(void* arg1, BSAnimationGraphEvent* a_event, void* a_dispatcher) {
			if (a_event->name == "reloadState" && a_event->args == "Exit") {
				if (g_isSprintQueued && !IsReloading()) {
					g_isSprintQueued = false;
					if (!Utils::IsSprinting())
						Utils::ToggleSprint(true);
				}
			}

			return origFunc(arg1, a_event, a_dispatcher);
		}

		inline static func_t origFunc;
	};

	template<std::uint64_t id, std::ptrdiff_t diff>
	class MenuOpenCloseEvent_ProcessEventHook {
	public:
		static void Install() {
			REL::Relocation<std::uintptr_t> target(REL::ID(id), diff);
			origFunc = *(func_t*)(target.get());
			REL::safe_write(target.get(), (std::uintptr_t)ProcessHook);
		}
	private:
		using func_t = RE::BSEventNotifyControl(*)(void*, RE::MenuOpenCloseEvent*, void*);

		static RE::BSEventNotifyControl ProcessHook(void* arg1, RE::MenuOpenCloseEvent* a_event, void* a_dispatcher) {
			if (a_event->menuName == "LoadingMenu" && a_event->opening)
				g_isSprintQueued = false;

			return origFunc(arg1, a_event, a_dispatcher);
		}

		inline static func_t origFunc;
	};

	void Install(bool bPreventTogglePOVDuringReload, bool bPreventReloadAfterTogglePOV, bool bPreventSprintReloading) {
		if (bPreventTogglePOVDuringReload) {
			TogglePOV_FirstToThirdHook<246953, 0x40>::Install();
			TogglePOV_ThirdToFirstHook<728085, 1011622, 0x40>::Install();
		}

		if (bPreventSprintReloading) {
			ReadyWeaponHandlerHook<549666, 0x40>::Install();
			MovementHandlerHook<577025, 0x40>::Install();
			SprintHandlerHook<1095200, 0x40>::Install();
			BSAnimationGraphEvent_ProcessEventHook<1542933, 0x08>::Install();
			MenuOpenCloseEvent_ProcessEventHook<267421, 0x08>::Install();
		}

		if (bPreventReloadAfterTogglePOV) {
			// xor dil, dil;
			// nop;
			uint8_t buf[] = { 0x40, 0x30, 0xFF, 0x90 };
			REL::Relocation<uintptr_t> target(REL::ID(601228), 0x446);
			REL::safe_write(target.get(), buf, sizeof(buf));
		}
	}
}
