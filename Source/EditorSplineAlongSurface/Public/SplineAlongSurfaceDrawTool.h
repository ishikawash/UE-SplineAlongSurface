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
	
protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<ASplineAlongSurface> SplineActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UScriptableInteractiveToolPropertySet> PropertySet;
};
