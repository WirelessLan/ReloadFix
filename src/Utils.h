#pragma once 

namespace Utils {
	bool IsSprinting();
	bool IsFirstPerson();
	bool IsWeaponDrawn();
	bool IsAutoMove();
	bool IsVanityModeEnabled();
	bool IsWeaponReloadable();
	bool IsButtonPressed(RE::ButtonEvent* a_btnEvent, float a_heldDownSecs);
	bool IsHoldingButton(RE::ButtonEvent* a_btnEvent, float a_heldDownSecs);
	void ToggleVanityMode(bool enable);
	void ToggleSprint(bool a_sprint);
}
