#pragma once 

namespace Utils {
	bool IsSprinting();
	bool IsFirstPerson();
	bool IsWeaponDrawn();
	bool IsVanityModeEnabled();
	void ToggleVanityMode(bool enable);
	void ToggleSprint(bool a_sprint);
}
