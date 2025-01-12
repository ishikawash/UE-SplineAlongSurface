#pragma once

#include "CoreMinimal.h"
#include "BaseTools/EditorScriptableSingleClickTool.h"
#include "SplineAlongSurfaceDrawTool.generated.h"

class ASplineAlongSurface;

/**
 * 
 */
UCLASS()
class EDITORSPLINEALONGSURFACE_API USplineAlongSurfaceDrawTool : public UEditorScriptableSingleClickTool
{
	GENERATED_BODY()
	
public:

	USplineAlongSurfaceDrawTool();
	virtual void Setup() override;
	virtual void Shutdown(EToolShutdownType ShutdownType) override;
	virtual void OnHitByClick_Implementation(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers) override;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<ASplineAlongSurface> SplineActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<ASplineAlongSurface> SplineActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UScriptableInteractiveToolPropertySet> PropertySet;

	void HandleOnHitClickEvent(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers);

	UFUNCTION(BlueprintCallable, Category = "SplineAlongSurface|DrawTool")
	ASplineAlongSurface* SpawnSplineActor(const FVector& WorldLocation);

	UFUNCTION(BlueprintCallable, Category = "SplineAlongSurface|DrawTool")
	void InputSurfacePoint(const FVector& WorldLocation, const FVector& WorldNormal, bool bRunConstructionScripts = false);

	void InputSurfacePointWithoutTransaction(const FVector& WorldLocation, const FVector& WorldNormal, bool bRunConstructionScripts);

private:

	UFUNCTION()
	void OnSplineActorClassChanged(UScriptableInteractiveToolPropertySet* TargetPropertySet, FString PropertyName, UObject* NewValue);

	UFUNCTION()
	void OnDistanceFromSurfaceChanged(UScriptableInteractiveToolPropertySet* TargetPropertySet, FString PropertyName, double NewValue);

	UFUNCTION()
	void OnTangentLengthChanged(UScriptableInteractiveToolPropertySet* TargetPropertySet, FString PropertyName, double NewValue);

	UFUNCTION()
	void OnMaxHeightOfObstacleChanged(UScriptableInteractiveToolPropertySet* TargetPropertySet, FString PropertyName, double NewValue);

	UFUNCTION()
	void OnOnlyPolyLineChanged(UScriptableInteractiveToolPropertySet* TargetPropertySet, FString PropertyName, bool NewValue);
};
