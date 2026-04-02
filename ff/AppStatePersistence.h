# pragma once
# include "AppData.h"

[[nodiscard]] AppData LoadAppDataFromDisk();
[[nodiscard]] bool SaveAppDataToDisk(const AppData& data);
