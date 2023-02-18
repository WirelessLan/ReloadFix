#pragma once

namespace Hooks {
	void ClearVariables();
	void Install(bool bPreventTogglePOVDuringReload, bool bPreventReloadAfterTogglePOV, bool bPreventSprintReloading);
}
