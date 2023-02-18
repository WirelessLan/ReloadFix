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

	void ToggleSprint(bool a_sprint) {
		RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
		if (!player)
			return;

		player->sprintToggled = a_sprint;
	}
}
