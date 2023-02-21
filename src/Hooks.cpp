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

	class hkbBehaviorGraph;

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

	using _ButtonEventHandler = void (*)(void*, RE::ButtonEvent*);
	using _PlayerAnimGraphEvent_ReceiveEvent = RE::BSEventNotifyControl(*)(void*, BSAnimationGraphEvent*, void*);
	using _MenuOpenCloseEvent_ReceiveEvent = RE::BSEventNotifyControl(*)(void*, RE::MenuOpenCloseEvent*, void*);

	REL::Relocation<uintptr_t> ReadyWeaponHandler_Target(REL::ID(549666), 0x40);
	_ButtonEventHandler ReadyWeaponHandler_Original;

	REL::Relocation<uintptr_t> TogglePOV_FirstToThird_Target(REL::ID(246953), 0x40);
	_ButtonEventHandler TogglePOV_FirstToThird_Original;

	REL::Relocation<uintptr_t> TogglePOV_ThirdToFirst_Target(REL::ID(728085), 0x40);
	_ButtonEventHandler TogglePOV_ThirdToFirst_Original;

	REL::Relocation<uintptr_t> MovementHandler_Target(REL::ID(577025), 0x40);
	_ButtonEventHandler MovementHandler_Original;

	REL::Relocation<uintptr_t> SprintHandler_Target(REL::ID(1095200), 0x40);
	_ButtonEventHandler SprintHandler_Original;

	REL::Relocation<uintptr_t> PlayerAnimGraphEvent_ReceiveEvent_Target(REL::ID(1542933), 0x8);
	_PlayerAnimGraphEvent_ReceiveEvent PlayerAnimGraphEvent_ReceiveEvent_Original;

	REL::Relocation<uintptr_t> MenuOpenCloseEvent_ReceiveEvent_Target(REL::ID(267421), 0x08);
	_MenuOpenCloseEvent_ReceiveEvent MenuOpenCloseEvent_ReceiveEvent_Original;

	REL::Relocation<uintptr_t> ShouldRestartReloading_Target(REL::ID(601228), 0x446);

	REL::Relocation<float*> minCurrentZoom(REL::ID(1011622));

	bool g_preventSprintReloading;
	bool g_isSprintQueued;

	bool IsReloading() {
		RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
		if (!player || !player->currentProcess || !player->currentProcess->middleHigh || !player->currentProcess->middleHigh->animationGraphManager)
			return false;

		BSAnimationGraphManager* animGraphManager = (BSAnimationGraphManager*)player->currentProcess->middleHigh->animationGraphManager.get();
		if ((Utils::IsFirstPerson() && !animGraphManager->firstPersonAnimGraph) || animGraphManager->thirdPersonAnimGraphs.empty())
			return false;

		BShkbAnimationGraph* animGraph = 
			(BShkbAnimationGraph*)(Utils::IsFirstPerson() ? 
				animGraphManager->firstPersonAnimGraph.get() : animGraphManager->thirdPersonAnimGraphs[0].get());
		if (!animGraph)
			return false;

		hkbBehaviorGraph* behaviorGraph = animGraph->behaviourGraph;
		if (!behaviorGraph || !behaviorGraph->activeNodes || behaviorGraph->activeNodes->size == 0)
			return false;

		if (!behaviorGraph->activeNodes->data[0]->clipGenerator || !behaviorGraph->activeNodes->data[0]->clipGenerator->animName.data)
			return false;

		return strncmp(behaviorGraph->activeNodes->data[0]->clipGenerator->animName.data, "WPNReload", strlen("WPNReload")) == 0;
	}

	void ClearVariables() {
		g_isSprintQueued = false;
	}

	void ReadyWeaponHandler_Hook(void* arg1, RE::ButtonEvent* event) {
		ReadyWeaponHandler_Original(arg1, event);

		if (g_preventSprintReloading && Utils::IsSprinting() && IsReloading()) {
			g_isSprintQueued = true;
			Utils::ToggleSprint(false);
		}
	}

	void TogglePOV_FirstToThird_Hook(void* arg1, RE::ButtonEvent* event) {
		if (IsReloading())
			return;

		TogglePOV_FirstToThird_Original(arg1, event);
	}

	void TogglePOV_ThirdToFirst_Hook(RE::ThirdPersonState* tpState, RE::ButtonEvent* event) {
		if (event->value == 0.0f && tpState && !tpState->freeRotationEnabled)
			tpState->freeRotation.x = 0.0f;

		if (IsReloading()) {
			if (event->strUserEvent == "TogglePOV") {
				return;
			}
			else if (event->strUserEvent == "ZoomIn") {
				if (tpState && tpState->currentZoomOffset <= *minCurrentZoom)
					return;
			}
		}

		TogglePOV_ThirdToFirst_Original(tpState, event);
	}

	void MovementHandler_Hook(void* arg1, RE::ButtonEvent* event) {
		if (g_isSprintQueued) {
			if (event->strUserEvent != "Forward" || event->value == 0.0f)
				g_isSprintQueued = false;
		}

		MovementHandler_Original(arg1, event);
	}

	void SprintHandler_Hook(void* arg1, RE::ButtonEvent* event) {
		if (IsReloading()) {
			g_isSprintQueued = true;
			return;
		}

		SprintHandler_Original(arg1, event);
	}

	RE::BSEventNotifyControl PlayerAnimGraphEvent_ReceiveEvent_Hook(void* arg1, BSAnimationGraphEvent* evn, void* dispatcher) {
		if (evn->name == "reloadState" && evn->args == "Exit") {
			if (g_preventSprintReloading && g_isSprintQueued && !IsReloading()) {
				g_isSprintQueued = false;
				if (!Utils::IsSprinting())
					Utils::ToggleSprint(true);
			}
		}

		return PlayerAnimGraphEvent_ReceiveEvent_Original(arg1, evn, dispatcher);
	}

	RE::BSEventNotifyControl MenuOpenCloseEvent_ReceiveEvent_Hook(void* arg1, RE::MenuOpenCloseEvent* evn, void* dispatcher) {
		if (evn->menuName == "LoadingMenu" && evn->opening)
			g_isSprintQueued = false;

		return MenuOpenCloseEvent_ReceiveEvent_Original(arg1, evn, dispatcher);
	}

	void Install(bool bPreventTogglePOVDuringReload, bool bPreventReloadAfterTogglePOV, bool bPreventSprintReloading) {
		if (bPreventTogglePOVDuringReload || bPreventSprintReloading) {
			ReadyWeaponHandler_Original = *(_ButtonEventHandler*)(ReadyWeaponHandler_Target.get());
			REL::safe_write(ReadyWeaponHandler_Target.address(), (uintptr_t)ReadyWeaponHandler_Hook);

			PlayerAnimGraphEvent_ReceiveEvent_Original = *(_PlayerAnimGraphEvent_ReceiveEvent*)(PlayerAnimGraphEvent_ReceiveEvent_Target.get());
			REL::safe_write(PlayerAnimGraphEvent_ReceiveEvent_Target.address(), (uintptr_t)PlayerAnimGraphEvent_ReceiveEvent_Hook);
		}

		if (bPreventTogglePOVDuringReload) {
			TogglePOV_FirstToThird_Original = *(_ButtonEventHandler*)(TogglePOV_FirstToThird_Target.get());
			REL::safe_write(TogglePOV_FirstToThird_Target.address(), (uintptr_t)TogglePOV_FirstToThird_Hook);

			TogglePOV_ThirdToFirst_Original = *(_ButtonEventHandler*)(TogglePOV_ThirdToFirst_Target.get());
			REL::safe_write(TogglePOV_ThirdToFirst_Target.address(), (uintptr_t)TogglePOV_ThirdToFirst_Hook);
		}

		if (bPreventSprintReloading) {
			g_preventSprintReloading = true;

			MovementHandler_Original = *(_ButtonEventHandler*)(MovementHandler_Target.get());
			REL::safe_write(MovementHandler_Target.address(), (uintptr_t)MovementHandler_Hook);

			SprintHandler_Original = *(_ButtonEventHandler*)(SprintHandler_Target.get());
			REL::safe_write(SprintHandler_Target.address(), (uintptr_t)SprintHandler_Hook);

			MenuOpenCloseEvent_ReceiveEvent_Original = *(_MenuOpenCloseEvent_ReceiveEvent*)(MenuOpenCloseEvent_ReceiveEvent_Target.get());
			REL::safe_write(MenuOpenCloseEvent_ReceiveEvent_Target.address(), (uintptr_t)MenuOpenCloseEvent_ReceiveEvent_Hook);
		}

		if (bPreventReloadAfterTogglePOV) {
			// xor dil, dil;
			// nop;
			uint8_t buf[] = { 0x40, 0x30, 0xFF, 0x90 };
			REL::safe_write(ShouldRestartReloading_Target.address(), buf, sizeof(buf));
		}
	}
}
