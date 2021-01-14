// To use, simply add bs-utils.so to your Android.mk, and #include utils.hpp
#include "utils.hpp"
#include "utils-logging.hpp"
#include "beatsaber-hook/shared/utils/logging.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/utils.h"
#include <stdlib.h>

MAKE_HOOK_OFFSETLESS(PlatformLeaderboardsModel_UploadScore, void, Il2CppObject *self, Il2CppObject* scoreData, Il2CppObject* completionHandler) {
    if (!bs_utils::Submission::getEnabled()) {
        getLogger().debug("Blocking vanilla score processing!");
        return;
    }
    getLogger().debug("Allowing vanilla score processing!");
    PlatformLeaderboardsModel_UploadScore(self, scoreData, completionHandler);
}

namespace bs_utils {
    std::unordered_set<DisablingModInfo, DisablingModInfoHash> Submission::disablingMods;
    bool Submission::enabled = true;
    bool Submission::initialized = false;

    void Submission::init() {
        if (!initialized) {
            INSTALL_HOOK_OFFSETLESS(getLogger(), PlatformLeaderboardsModel_UploadScore, il2cpp_utils::FindMethodUnsafe("", "PlatformLeaderboardsModel", "UploadScore", 2));
            initialized = true;
        }
    }

    void Submission::enable(const ModInfo& info) {
        init();
        auto itr = disablingMods.find(info);
        if (itr != disablingMods.end()) {
            disablingMods.erase(itr);
        }
        if (disablingMods.empty() && !enabled) {
            // Re-enable score submission
            getLogger().info("Mod: %s is enabling score submission!", info.id.c_str());
            setenv("disable_ss_upload", "0", true);
            enabled = true;
        }
    }
    void Submission::disable(const ModInfo& info) {
        init();
        if (disablingMods.find(info) != disablingMods.end()) {
            // Don't disable again if this mod has already disabled it once.
            getLogger().info("Mod: %s is disabling score submission multiple times!", info.id.c_str());
            return;
        }
        disablingMods.insert(info);
        if (enabled) {
            // Disable score submission
            getLogger().info("Mod: %s is disabling score submission!", info.id.c_str());
            setenv("disable_ss_upload", "1", true);
            // this will also disable vanilla score submission via main.cpp
            enabled = false;
        }
    }
    const std::unordered_set<DisablingModInfo, DisablingModInfoHash> Submission::getDisablingMods() {
        return disablingMods;
    }
    bool Submission::getEnabled() {
        return enabled;
    }
}
