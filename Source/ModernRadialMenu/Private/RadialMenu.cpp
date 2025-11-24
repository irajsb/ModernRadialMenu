// Copyright 2023 Iraj Mohtasham aurelion.net 



#include "RadialMenu.h"


#include "RadialPanelSlot.h"

#include "Blueprint/SlateBlueprintLibrary.h"
#include "Components/Button.h"
#include "Components/CanvasPanelSlot.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

#define LOCTEXT_NAMESPACE "Titan"

URadialMenu::URadialMenu()
	: SelectorColor(0.f, 0.518556, 0.796875, 1),
	  BackgroundColor(0.019382, 0.019382, 0.019382, 1.000000),
	  DividerOpacity(1.f)
{
	bCanHaveMultipleChildren=true;
	const ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialDefault(
		TEXT("/ModernRadialMenu/RadialMenu/M_RadialMenu"));
	if (MaterialDefault.Succeeded())
	{
		RadialMenuBaseMaterial = MaterialDefault.Object;
	}
}

void URadialMenu::UpdateMaterial()
{
	if (!MaterialInstance && RadialMenuBaseMaterial)
	{
		MaterialInstance = UMaterialInstanceDynamic::Create(RadialMenuBaseMaterial, this);
	}
	if (MaterialInstance)
	{
		MaterialInstance->SetScalarParameterValue(FName("DividerCount"), Slots.Num());
		MaterialInstance->SetScalarParameterValue(FName("DividerOpacity"), DividerOpacity);
		MaterialInstance->SetVectorParameterValue(FName("BackGroundColor"), BackgroundColor);
		MaterialInstance->SetVectorParameterValue(FName("ForeGround"), SelectorColor);
		

		Brush.SetResourceObject(MaterialInstance);
	}
}

URadialPanelSlot* URadialMenu::AddChildToRadialMenu(UWidget* Content)
{
	if ( Content == nullptr )
	{
		return nullptr;
	}

	if ( !bCanHaveMultipleChildren && GetChildrenCount() > 0 )
	{
		return nullptr;
	}

	Content->RemoveFromParent();

	EObjectFlags NewObjectFlags = RF_Transactional;
	if (HasAnyFlags(RF_Transient))
	{
		NewObjectFlags |= RF_Transient;
	}

	URadialPanelSlot* PanelSlot = NewObject<URadialPanelSlot>(this, URadialPanelSlot::StaticClass(), NAME_None, NewObjectFlags);
	PanelSlot->Content = Content;
	PanelSlot->Parent = this;

	
	Content->Slot = PanelSlot;

	Slots.Add(PanelSlot);

	OnSlotAdded(PanelSlot);

	InvalidateLayoutAndVolatility();

	return PanelSlot;
}

TSharedRef<SWidget> URadialMenu::RebuildWidget()
{
	Overlay = SNew(SConstraintCanvas);

	RadialBox = SNew(SRadialBox);



	
	const TAttribute<const FSlateBrush*> ImageBinding = OPTIONAL_BINDING_CONVERT(
		FSlateBrush, Brush, const FSlateBrush*, ConvertImage);

	UpdateMaterial();
	BackgroundImage = SNew(SImage).Image(ImageBinding);
	Overlay->AddSlot()[BackgroundImage.ToSharedRef()].Anchors(FAnchors(0.0 + BackgroundPadding,
	                                                                   0.0f + BackgroundPadding,
	                                                                   1.f - BackgroundPadding,
	                                                                   1.f - BackgroundPadding)).AutoSize(true);


	Overlay->AddSlot()[RadialBox.ToSharedRef()].Anchors(
		                                            FAnchors(0.0 + OptionsPadding, 0.0f + OptionsPadding,
		                                                     1.f - OptionsPadding, 1.f - OptionsPadding)).
	                                            AutoSize(true);


	Overlay->SetOnMouseEnter(BIND_UOBJECT_DELEGATE(FNoReplyPointerEventHandler, URadialMenu::HandleOnMouseEnter));
	Overlay->SetOnMouseMove(BIND_UOBJECT_DELEGATE(FPointerEventHandler, URadialMenu::HandleMouseMove));
	Overlay->SetOnMouseLeave(BIND_UOBJECT_DELEGATE(FSimpleNoReplyPointerEventHandler, URadialMenu::HandleOnMouseLeave));

	Overlay->SetOnMouseButtonDown(BIND_UOBJECT_DELEGATE(FPointerEventHandler, URadialMenu::HandleMouseDown));
	Overlay->SetOnMouseButtonUp(BIND_UOBJECT_DELEGATE(FPointerEventHandler, URadialMenu::HandleMouseUp));

	BackgroundImage->SetRenderTransformPivot(FVector2D(0.5));
	BackgroundImage->SetRenderTransform(
		FSlateRenderTransform(FQuat2D(FMath::DegreesToRadians(BackgroundRotationOffset))));

	RadialBox->SetSectorCentralAngle(360.f);
	RadialBox->SetUseAllottedWidth(true);
	SetStartingAngle();
	RadialBox->SetDistributeItemsEvenly(true);
	RadialBox->SetVisibility(EVisibility::HitTestInvisible);


	for (UPanelSlot* PanelSlot : Slots)
	{
		PanelSlot->Parent = this;
		if (Overlay.IsValid())
		{
			CastChecked<URadialPanelSlot>(PanelSlot)->BuildSlot(RadialBox.ToSharedRef());
		}
	}

	if (HideSelectorWhenNotHovered && !(IsDesignTime()))
	{
		SetSelectorVisibility(false);
	}

	SetSelectorAngle(0.f);
	return Overlay.ToSharedRef();
}


void URadialMenu::OnSlotAdded(UPanelSlot* InSlot)
{

	// Add the child to the live canvas if it already exists
	if (Overlay.IsValid())
	{
		CastChecked<URadialPanelSlot>(InSlot)->BuildSlot(RadialBox.ToSharedRef());
		SetStartingAngle();
	}
	UpdateMaterial();
}

void URadialMenu::OnSlotRemoved(UPanelSlot* InSlot)
{
	// Remove the widget from the live slot if it exists.
	if (Overlay.IsValid() && InSlot->Content)
	{
		const TSharedPtr<SWidget> Widget = InSlot->Content->GetCachedWidget();
		if (Widget.IsValid())
		{
			RadialBox->RemoveSlot(Widget.ToSharedRef());
		}
	}
}

void URadialMenu::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	Overlay.Reset();
	RadialBox.Reset();
	BackgroundImage.Reset();
}


void URadialMenu::SetSelectorColor(FLinearColor InSelectorColor)
{
	SelectorColor = InSelectorColor;
}

void URadialMenu::SetSelectorAngle(float InputDegrees)
{
	SelectorAngle = InputDegrees;
	if(Slots.Num()==0)
	{
		SelectorAngle=0.f;
		return;
	}
	if (MaterialInstance)
	{
		if (SelectorAngle < 0)
		{
			SelectorAngle = 360 + SelectorAngle;
		}

		SelectorAngle += (StartingAngle - 90.f);

		if (SnapSelector)
		{
			const auto SlotAngle = 360 / Slots.Num();
			SelectorAngle = FMath::RoundToInt(SelectorAngle / SlotAngle) * SlotAngle;
		}

		SelectorAngle -= (StartingAngle - 90.f);

		if (SelectorAngle < 0)
		{
			SelectorAngle = 360 + SelectorAngle;
		}

		const float RotatedSelectorRotation = SelectorAngle - BackgroundRotationOffset < 0
			                                      ? 360 + SelectorAngle - BackgroundRotationOffset
			                                      : SelectorAngle - BackgroundRotationOffset;

		MaterialInstance->SetScalarParameterValue(FName("CursorDegree"),
		                                          UKismetMathLibrary::MapRangeClamped(
			                                          RotatedSelectorRotation, 0, 360.f, PI, PI * -1));

		SelectorAngle += (StartingAngle - 90.f);
	}
}

void URadialMenu::SetSelectorAngleByIndex(const int Index)
{
	SetSelectorAngle(360 / Slots.Num() * Index);
}

void URadialMenu::SetSelectorAngleFromViewportPosition(UObject* WorldContextObject, FVector2D InPos)
{
	FVector2D PixelPos, ViewportPos;
	USlateBlueprintLibrary::LocalToViewport(WorldContextObject, GetCachedGeometry(),
	                                        GetCachedGeometry().GetLocalSize() / 2, PixelPos, ViewportPos);
	const FVector2D NormalizedVector = ((InPos - ViewportPos) * FVector2D(1.f, -1.f)).GetSafeNormal();
	const FVector2D UpVector(0.f, 1.f);
	const float Radians = FMath::Atan2(FVector2D::CrossProduct(NormalizedVector, UpVector),
	                                   FVector2D::DotProduct(NormalizedVector, UpVector));
	SetSelectorAngle(FMath::RadiansToDegrees(Radians));
}

int URadialMenu::GetElementUnderSelector(UWidget*& Widget)
{
	if(Slots.Num()<1)
	{
		return -1;
	}
	const auto SlotAngle = 360 / Slots.Num();

	int Index = FMath::RoundToInt(SelectorAngle / SlotAngle);

	if (Index == Slots.Num())
	{
		Index = 0;
	}
	if (Index != 0)
	{
		Index = Slots.Num() - Index;
	}
	if (!Slots.IsValidIndex(Index))
	{
		return Index;
	}
	Widget = Slots[Index]->Content;
	return Index;
}

void URadialMenu::SetSelectorVisibility(const bool In)
{
	
	UE_LOG(LogTemp,Log,TEXT("Visiblity %d"),In)
	if (MaterialInstance)
	{
		MaterialInstance->SetVectorParameterValue(FName("ForeGround"),  In ? SelectorColor : BackgroundColor);
	}
}

UClass* URadialMenu::GetSlotClass() const
{
	return URadialPanelSlot::StaticClass();
}

const FSlateBrush* URadialMenu::ConvertImage(TAttribute<FSlateBrush> InImageAsset) const
{
	URadialMenu* MutableThis = const_cast<URadialMenu*>(this);
	MutableThis->Brush = InImageAsset.Get();

	return &Brush;
}


void URadialMenu::HandleOnMouseLeave(const FPointerEvent&)
{
	if (HideSelectorWhenNotHovered)
	{
		SetSelectorVisibility(false);
	}
	if (HoveredButton)
	{
		SwapButtonStyle(HoveredButton);
		HoveredButton = nullptr;
	}
}


#if WITH_EDITOR
const FText URadialMenu::GetPaletteCategory()
{
	return LOCTEXT("Titan","Titan");
	
}

#endif

void URadialMenu::SwapButtonStyle(UButton* Button)
{
#if ENGINE_MAJOR_VERSION>4 && ENGINE_MINOR_VERSION>1
	auto Style=Button->GetStyle();
	const auto Normal = Style.Normal;
	Style.Normal = Style.Hovered;
	Style.Hovered = Normal;
	Button->SetStyle(Style);
#else
	const auto Normal = Button->WidgetStyle.Normal;
	Button->WidgetStyle.Normal = Button->WidgetStyle.Hovered;
	Button->WidgetStyle.Hovered = Normal;
	Button->SetStyle(Button->WidgetStyle);
#endif
	
}



void URadialMenu::HandleOnMouseEnter(const FGeometry& Geometry, const FPointerEvent& Event)
{
	
	if (HideSelectorWhenNotHovered)
	{
		SetSelectorVisibility(true);
	}
}

FReply URadialMenu::HandleMouseMove(const FGeometry& Geometry, const FPointerEvent& Event)
{
	UWidget* Widget;
	GetElementUnderSelector(Widget);
	UButton* NewlyHoveredButton = Cast<UButton>(Widget);
	if (HoveredButton == Widget)
	{
		return FReply::Unhandled();
	}
	if (NewlyHoveredButton)
	{
		//reset 
		if (HoveredButton)
		{
			SwapButtonStyle(HoveredButton);
		}
		SwapButtonStyle(NewlyHoveredButton);
		HoveredButton = NewlyHoveredButton;
	}


	return FReply::Unhandled();
}

FReply URadialMenu::HandleMouseDown(const FGeometry& Geometry, const FPointerEvent& Event) const
{
	MouseDown.Broadcast(Geometry, Event);
	return FReply::Handled();
}

FReply URadialMenu::HandleMouseUp(const FGeometry& Geometry, const FPointerEvent& Event) const
{
	MouseUp.Broadcast(Geometry, Event);
	return FReply::Handled();
}

void URadialMenu::SetStartingAngle()
{
	StartingAngle = Slots.Num() % 2 != 0 ? 90.f : 90.f + ((360.f / Slots.Num()) / 2.f);
	StartingAngle -= BackgroundRotationOffset;
	RadialBox->SetStartingAngle(StartingAngle);
}

void URadialMenu::SynchronizeProperties()
{
	Super::SynchronizeProperties();
#if WITH_EDITOR
	if(IsDesignTime())
	{
		RebuildWidget();
	}
#endif
	if (MaterialInstance)
	{
		MaterialInstance->SetScalarParameterValue(FName("DividerCount"), Slots.Num());
		MaterialInstance->SetScalarParameterValue(FName("DividerOpacity"), DividerOpacity);
		MaterialInstance->SetVectorParameterValue(FName("BackGroundColor"), BackgroundColor);
		MaterialInstance->SetVectorParameterValue(FName("ForeGround"), SelectorColor);
	
		SetSelectorAngle(0.f);
		Brush.SetResourceObject(MaterialInstance);
	}
	BackgroundImage->SetRenderTransform(
		FSlateRenderTransform(FQuat2D(FMath::DegreesToRadians(BackgroundRotationOffset))));

	SetStartingAngle();
}

#undef LOCTEXT_NAMESPACE