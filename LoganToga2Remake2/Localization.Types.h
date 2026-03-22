#pragma once

#include "Remake2Common.h"

using AppLanguage = String;

namespace Localization
{
	struct LanguageDefinition
	{
		AppLanguage language;
		String displayName;
		String resourcePath;
		int32 sortOrder = 1000;
		Array<String> persistenceAliases;
	};
}
