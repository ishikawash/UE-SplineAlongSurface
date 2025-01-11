#pragma once

#include "CoreMinimal.h"
#include "EditorScriptableInteractiveTool.h"
#include "SplineAlongSurfacePropertySet.generated.h"

class ASplineAlongSurface;

/**
 * 
 */
UCLASS()
class EDITORSPLINEALONGSURFACE_API USplineAlongSurfacePropertySet : public UEditorScriptableInteractiveToolPropertySet
{
	GENERATED_BODY()
	
protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<ASplineAlongSurface> SplineActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceFromSurface = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TangentLength = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHeightOfObstacle = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool OnlyPolyLine = false;
};
