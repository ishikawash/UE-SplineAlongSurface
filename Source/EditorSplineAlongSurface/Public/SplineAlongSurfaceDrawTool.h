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

	virtual void Shutdown(EToolShutdownType ShutdownType) override;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<ASplineAlongSurface> SplineActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<ASplineAlongSurface> SplineActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UScriptableInteractiveToolPropertySet> PropertySet;

	UFUNCTION(BlueprintCallable, Category = "SplineAlongSurface|DrawTool")
	void InputSurfacePoint(const FVector& WorldLocation, const FVector& WorldNormal, bool bRunConstructionScripts = false);

	void InputSurfacePointWithoutTransaction(const FVector& WorldLocation, const FVector& WorldNormal, bool bRunConstructionScripts);
};
