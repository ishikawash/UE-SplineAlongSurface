#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SplineAlongSurface_Functions.h"
#include "SplineAlongSurfaceActor.generated.h"

UCLASS()
class SPLINEALONGSURFACE_API ASplineAlongSurface : public AActor
{
	GENERATED_BODY()
	
public:
	ASplineAlongSurface();

	USplineComponent* GetSplineComponent() { return Spline.Get(); }

	TArray<FSurfacePoint>& GetSurfacePoints() { return SurfacePoints; }

	FSplineAlongSurfaceParams& GetParams() { return SplineAlongSurfaceParams; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<USplineComponent> Spline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSurfacePoint> SurfacePoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSplineAlongSurfaceParams SplineAlongSurfaceParams;

	UFUNCTION(BlueprintCallable, Category="SplineAlongSurface|Actor")
	void BuildSpline();
};
