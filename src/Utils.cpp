#include "Utils.h"

namespace Utils {
	bool IsSprinting() {
		RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
		if (!player)
			return false;

		return player->moveMode & 0x0100;
	}

	bool IsFirstPerson() {
		RE::PlayerCamera* pCam = RE::PlayerCamera::GetSingleton();
		if (!pCam)
			return false;

		return pCam->currentState == pCam->cameraStates[RE::CameraState::kFirstPerson];
	}

	bool IsWeaponDrawn() {
		RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
		if (!player)
			return false;

		return player->GetWeaponMagicDrawn();
	}

	bool IsVanityModeEnabled() {
		RE::PlayerControls* pCon = RE::PlayerControls::GetSingleton();
		if (!pCon)
			return false;

		return pCon->data.vanityModeEnabled;
	}

	void ToggleVanityMode(bool enable) {
		RE::PlayerControls* pCon = RE::PlayerControls::GetSingleton();
		if (!pCon)
			return;

		pCon->data.vanityModeEnabled = enable;
	}

	bool PerformAction(RE::Actor* a_actor, std::uint32_t a_actionIndex, RE::TESObjectREFR* a_ref) {
		using func_t = decltype(&PerformAction);
		REL::Relocation<func_t> func{ REL::ID(445541) };
		return func(a_actor, a_actionIndex, a_ref);
	}

	void ToggleSprint(bool a_sprint) {
		RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
		if (!player)
			return;

		// Sneak -> Stand
		if (a_sprint && player->stance == 0x01)
			PerformAction(player, 0x30, nullptr);
		
		player->sprintToggled = a_sprint;
	}
}
