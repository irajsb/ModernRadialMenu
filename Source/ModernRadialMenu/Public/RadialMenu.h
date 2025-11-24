// Copyright 2023 Iraj Mohtasham aurelion.net 

#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/PanelWidget.h"
#include "Widgets/Layout/SRadialBox.h"
#include "Widgets/Images/SImage.h"
#include "RadialMenu.generated.h"



class URadialPanelSlot;
class SRadialMenu;
/**
 * A radial menu with dynamically scaling background and borders
 * Supports adding buttons to simulate hovering effect
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPointerEventDelegate, const FGeometry&, Geometry, const FPointerEvent&,
                                             PointerEvent);

UCLASS()
class MODERNRADIALMENU_API URadialMenu : public UPanelWidget
{
	GENERATED_BODY()

	virtual TSharedRef<SWidget> RebuildWidget() override;

public:
	URadialMenu();

	void UpdateMaterial();
	UFUNCTION(BlueprintCallable, Category="Panel")
	URadialPanelSlot* AddChildToRadialMenu(UWidget* Content);
	
	// You can alter the material but keep the variable names the same 
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category="RadialMenu")
	UMaterialInterface* RadialMenuBaseMaterial;
	//Foreground Color
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="RadialMenu")
	FLinearColor SelectorColor;
	//Background color 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="RadialMenu")
	FLinearColor BackgroundColor;
	//Divider opacity between each element
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="RadialMenu")
	float DividerOpacity;
	//Padding for options relative to widget size
	UPROPERTY(EditAnywhere, meta=(UIMin="0.0", UIMax="1.0"), Category="RadialMenu")
	float OptionsPadding;
	//Background padding  relative to widget size
	UPROPERTY(EditAnywhere, meta=(UIMin="0.0", UIMax="1.0"), Category="RadialMenu")
	float BackgroundPadding;
	//Should selector snap instead of transitioning smoothly
	UPROPERTY(EditAnywhere, Category="RadialMenu")
	bool SnapSelector;
	//Selector will not be visible if not hovered with mouse
	UPROPERTY(EditAnywhere, Category="RadialMenu")
	bool HideSelectorWhenNotHovered;
	//Should rotate background 
	UPROPERTY(EditAnywhere, meta=(ClampMin="0", ClampMax="90", UIMin="0", UIMax="90"), Category="RadialMenu")
	float BackgroundRotationOffset = 0.f;

	//Setters
	UFUNCTION(BlueprintCallable, Category="RadialMenu")
	void SetSelectorColor(FLinearColor InSelectorColor);

	//Set angle on selector 0-360 starting from top and clock-wise
	UFUNCTION(BlueprintCallable, Category="RadialMenu")
	void SetSelectorAngle(float InputDegrees);
	// Set selector angle based on an item index
	UFUNCTION(BlueprintCallable, Category="RadialMenu")
	void SetSelectorAngleByIndex(int Index);
	//Calculate Angle from viewport position( if doesn't work as expected divide or multiply pos by viewport scale )
	UFUNCTION(BlueprintCallable, meta=( WorldContext="WorldContextObject" ), Category="RadialMenu")
	void SetSelectorAngleFromViewportPosition(UObject* WorldContextObject, FVector2D InPos);



	
	//Get element under selector
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="RadialMenu")
	int GetElementUnderSelector(UWidget*& Widget);
	UFUNCTION(BlueprintCallable, Category="RadialMenu")
	void SetSelectorVisibility(bool In);

	// UPanelWidget
	virtual UClass* GetSlotClass() const override;
	virtual void OnSlotAdded(UPanelSlot* InSlot) override;
	virtual void OnSlotRemoved(UPanelSlot* InSlot) override;
	/** Translates the bound brush data and assigns it to the cached brush used by this widget. */
	const FSlateBrush* ConvertImage(TAttribute<FSlateBrush> InImageAsset) const;
	// End UPanelWidget

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	float SelectorAngle;
	float StartingAngle;
	UPROPERTY()
	UButton* HoveredButton;
	TSharedPtr<SConstraintCanvas> Overlay;
	TSharedPtr<SRadialBox> RadialBox;
	TSharedPtr<SImage> BackgroundImage;

	/** Image to draw */
	UPROPERTY()
	FSlateBrush Brush;

	/** A bindable delegate for the Image. */
	UPROPERTY()
	FGetSlateBrush BrushDelegate;
	UPROPERTY()
	UMaterialInstanceDynamic* MaterialInstance = nullptr;


	//events
	UPROPERTY(BlueprintAssignable)
	FPointerEventDelegate MouseDown;
	UPROPERTY(BlueprintAssignable)
	FPointerEventDelegate MouseMove;
	UPROPERTY(BlueprintAssignable)
	FPointerEventDelegate MouseUp;

	UPROPERTY(BlueprintAssignable)
	FPointerEventDelegate TouchStart;
	UPROPERTY(BlueprintAssignable)
	FPointerEventDelegate TouchMove;
	UPROPERTY(BlueprintAssignable)
	FPointerEventDelegate TouchEnd;
	

	static void SwapButtonStyle(UButton* Button);

	
	void HandleOnMouseEnter(const FGeometry& Geometry, const FPointerEvent& Event);

	FReply HandleMouseMove(const FGeometry& Geometry, const FPointerEvent& Event);

	FReply HandleMouseDown(const FGeometry& Geometry, const FPointerEvent& Event) const;
	FReply HandleMouseUp(const FGeometry& Geometry, const FPointerEvent& Event) const;

	void SetStartingAngle();
	virtual void SynchronizeProperties() override;
	void HandleOnMouseLeave(const FPointerEvent&);

#if WITH_EDITOR
	
	virtual const FText GetPaletteCategory() override;
#endif
};
