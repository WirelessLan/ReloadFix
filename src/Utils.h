#pragma once 

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

	struct LOAD_DATA {
		struct Data {
			uint64_t unk00;
			RE::Actor* actor;
		};

		void* arg1;
		void* arg2;
	};

	bool IsWeaponReloadable() {
		using func_t = bool (*)(LOAD_DATA*, RE::EquippedItem*);
		REL::Relocation<func_t> func{ REL::ID(1089596) };

		RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
		if (!player || !player->currentProcess || !player->currentProcess->middleHigh)
			return false;

		if (player->currentProcess->middleHigh->equippedItems.empty())
			return false;

		RE::EquippedItem* equipData = nullptr;
		for (RE::EquippedItem& item : player->currentProcess->middleHigh->equippedItems) {
			if (item.equipIndex.index == 0) {
				equipData = &item;
				break;
			}
		}
		if (!equipData)
			return false;

		LOAD_DATA::Data data = { 0 };
		data.actor = player;

		LOAD_DATA loadData = { &data.unk00, &data.actor };

		return !func(&loadData, equipData);
	}

	bool IsButtonPressed(RE::ButtonEvent* btnEvent) {
		if (btnEvent->value == 0.0f && btnEvent->heldDownSecs < 0.4f)
			return true;
		return false;
	}
}
