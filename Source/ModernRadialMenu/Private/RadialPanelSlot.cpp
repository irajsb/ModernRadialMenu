// Copyright 2023 Iraj Mohtasham aurelion.net 


#include "RadialPanelSlot.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Components/Widget.h"

void URadialPanelSlot::ReleaseSlateResources(bool bReleaseChildren)
{
	
	Super::ReleaseSlateResources(bReleaseChildren);
	Slot= nullptr;
}

void URadialPanelSlot::BuildSlot(TSharedRef<SRadialBox> InRadialBox)
{
#if ENGINE_MAJOR_VERSION >4

	 auto Result= &InRadialBox->AddSlot().Expose(Slot)
			[
				Content == NULL ? SNullWidget::NullWidget : Content->TakeWidget()
			];

#else
	Slot = &InRadialBox->AddSlot()
		
	
		[
			Content == NULL ? SNullWidget::NullWidget : Content->TakeWidget()
		];


#endif
}
