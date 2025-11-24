// Copyright 2023 Iraj Mohtasham aurelion.net 

#pragma once

#include "CoreMinimal.h"
#include "Components/PanelSlot.h"
#include "Widgets/Layout/SRadialBox.h"
#include "RadialPanelSlot.generated.h"

/**
 * 
 */

UCLASS()
class MODERNRADIALMENU_API URadialPanelSlot : public UPanelSlot
{
	GENERATED_BODY()

public:
	SRadialBox::FSlot* Slot;

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	/** Builds the underlying FSlot for the Slate layout panel. */
	void BuildSlot(TSharedRef<SRadialBox> InRadialBox);
};
