// Copyright 2024 Iraj Mohtasham aurelion.net 

#pragma once

#include "Modules/ModuleManager.h"

class FModernRadialMenuModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
