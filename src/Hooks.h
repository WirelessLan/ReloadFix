#pragma once

#include "Utils.h"

namespace Hooks {
	using _ButtonEventHandler = void (*)(void*, RE::ButtonEvent*);
	REL::Relocation<uintptr_t> ReadyWeaponHandler_Target(REL::ID(549666), 0x40);
	_ButtonEventHandler ReadyWeaponHandler_Original;

	REL::Relocation<uintptr_t> TogglePOV_Target(REL::ID(1040085), 0x40);
	_ButtonEventHandler TogglePOV_Original;

	REL::Relocation<uintptr_t> TogglePOV_FirstToThird_Target(REL::ID(246953), 0x40);
	_ButtonEventHandler TogglePOV_FirstToThird_Original;

	REL::Relocation<uintptr_t> TogglePOV_ThirdToFirst_Target(REL::ID(728085), 0x40);
	_ButtonEventHandler TogglePOV_ThirdToFirst_Original;

	struct BSAnimationGraphEvent {
	public:
		RE::TESObjectREFR* refr;
		RE::BSFixedString name;
		RE::BSFixedString args;
	};

	using _PlayerAnimGraphEvent_ReceiveEvent = RE::BSEventNotifyControl (*)(void* arg1, BSAnimationGraphEvent* evn, void* dispatcher);
	REL::Relocation<uintptr_t> PlayerAnimGraphEvent_ReceiveEvent_Target(REL::ID(1542933), 0x8);
	_PlayerAnimGraphEvent_ReceiveEvent PlayerAnimGraphEvent_ReceiveEvent_Original;

	REL::Relocation<uintptr_t> ShouldRestartReloading_Target(REL::ID(601228), 0x446);

	REL::Relocation<float*> minCurrentZoom(REL::ID(1011622));

	uint32_t g_reloadStackSize;

	bool IsReloading() {
		return g_reloadStackSize > 0;
	}

	void ClearReloadStack() {
		g_reloadStackSize = 0;
	}

	void ReadyWeaponHandler_Hook(void* arg1, RE::ButtonEvent* event) {
		if (Utils::IsButtonPressed(event) && Utils::IsWeaponDrawn() && Utils::IsSprinting() && Utils::IsWeaponReloadable() && !IsReloading())
			g_reloadStackSize++;

		ReadyWeaponHandler_Original(arg1, event);
	}

	void TogglePOV_Hook(void* arg1, RE::ButtonEvent* event) {
		if (IsReloading())
			return;

		if (Utils::IsFirstPerson() && Utils::IsWeaponDrawn()) {
			if (!Utils::IsVanityModeEnabled())
				Utils::ToggleVanityMode(true);
			return;
		}

		TogglePOV_Original(arg1, event);
	}

	void TogglePOV_FirstToThird_Hook(void* arg1, RE::ButtonEvent* event) {
		if (IsReloading())
			return;

		TogglePOV_FirstToThird_Original(arg1, event);
	}

	void TogglePOV_ThirdToFirst_Hook(RE::ThirdPersonState* tpState, RE::ButtonEvent* event) {
		if (IsReloading()) {
			if (event->strUserEvent == "TogglePOV")
				return;

			if (event->strUserEvent == "ZoomIn") {
				if (tpState && tpState->currentZoomOffset <= *minCurrentZoom)
					return;
			}
		}

		TogglePOV_ThirdToFirst_Original(tpState, event);
	}

	RE::BSEventNotifyControl PlayerAnimGraphEvent_ReceiveEvent_Hook(void* arg1, BSAnimationGraphEvent* evn, void* dispatcher) {
		if (evn->name == "reloadState") {
			if (evn->args == "Enter")
				g_reloadStackSize++;
		}
		else if (evn->name == "initiateStart") {
			g_reloadStackSize = 0;
		}

		return PlayerAnimGraphEvent_ReceiveEvent_Original(arg1, evn, dispatcher);
	}

	void Install(bool bPreventTogglePOVDuringReload, bool bPreventReloadAfterTogglePOV) {
		if (bPreventTogglePOVDuringReload) {
			ReadyWeaponHandler_Original = *(_ButtonEventHandler*)(ReadyWeaponHandler_Target.get());
			REL::safe_write(ReadyWeaponHandler_Target.address(), (uintptr_t)ReadyWeaponHandler_Hook);

			TogglePOV_Original = *(_ButtonEventHandler*)(TogglePOV_Target.get());
			REL::safe_write(TogglePOV_Target.address(), (uintptr_t)TogglePOV_Hook);

			TogglePOV_FirstToThird_Original = *(_ButtonEventHandler*)(TogglePOV_FirstToThird_Target.get());
			REL::safe_write(TogglePOV_FirstToThird_Target.address(), (uintptr_t)TogglePOV_FirstToThird_Hook);

			TogglePOV_ThirdToFirst_Original = *(_ButtonEventHandler*)(TogglePOV_ThirdToFirst_Target.get());
			REL::safe_write(TogglePOV_ThirdToFirst_Target.address(), (uintptr_t)TogglePOV_ThirdToFirst_Hook);

			PlayerAnimGraphEvent_ReceiveEvent_Original = *(_PlayerAnimGraphEvent_ReceiveEvent*)(PlayerAnimGraphEvent_ReceiveEvent_Target.get());
			REL::safe_write(PlayerAnimGraphEvent_ReceiveEvent_Target.address(), (uintptr_t)PlayerAnimGraphEvent_ReceiveEvent_Hook);
		}

		if (bPreventReloadAfterTogglePOV) {
			uint8_t buf[] = { 0x40, 0x30, 0xFF, 0x90 };	// xor dil, dil; nop;
			REL::safe_write(ShouldRestartReloading_Target.address(), buf, sizeof(buf));
		}
	}
}
