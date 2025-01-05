#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SplineAlongSurface_Functions.generated.h"

class USplineComponent;

USTRUCT(BlueprintType)
struct SPLINEALONGSURFACE_API FSurfacePoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = { 0.f, 0.f, 0.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Normal = { 0.f, 0.f, 1.f };
};

USTRUCT(BlueprintType)
struct SPLINEALONGSURFACE_API FSplineAlongSurfaceParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceFromSurface = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TangentLength = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHeightOfObstacle = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOnlyPolyLine = false;
};

UCLASS()
class SPLINEALONGSURFACE_API USplineAlongSurface_Functions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	static bool FindWayPointTransform(const FTransform& A, const FTransform& B, const FVector& UpA, const FVector& UpB, FTransform& C);

	static void ComputeWayPoints(const TArray<FSurfacePoint>& SurfacePoints, TArray<FTransform>& OutWayPoints, float OffsetLength);

	static bool ComputePolyLinePoints(USceneComponent* Scene, const TArray<FSurfacePoint>& SurfacePoints, TArray<FTransform>& OutLinePoints, float DistanceFromSurface, float MaxHeightOfObstacle);

	static bool DetectObstacle(const UObject* WorldContextObject, const FVector& WorldStart, const FVector& WorldEnd, const FVector& WorldUp, TArray<FSurfacePoint>& OutWayPoints, float OffsetLength);

	static FSurfacePoint TransformFromWorldToLocal(const FSurfacePoint& InSurfacePoint, const FTransform& WorldTransform);

	static bool TestTwoTransformDirections(const FTransform& A, const FTransform& B);

	static void ComputeControlPointsAtCorner(const FTransform& A, const FTransform& B, TArray<FTransform>& OutControlPoints, float OffsetLength);

	static void CleanupRedundantControlPoints(const TArray<FTransform>& InControlPoints, TArray<FTransform>& OutControlPoints);

	static bool ComputeControlPoints(const TArray<FTransform>& LinePoints, TArray<FTransform>& OutControlPoints, float TangentLength);

	static void BuildPolyLine(USplineComponent* Spline, const TArray<FSurfacePoint>& SurfacePoints, float DistanceFromSurface, float MaxHeightOfObstacle);

	static void BuildCurve(USplineComponent* Spline, const TArray<FTransform>& LinePoints, float TangentLength);

public:

	UFUNCTION(BlueprintCallable, Category = "SplineAlongSurface|Functions")
	static void BuildSplineAlongSurface(USplineComponent* Spline, const TArray<FSurfacePoint>& SurfacePoints, const FSplineAlongSurfaceParams& Params);

	UFUNCTION(BlueprintCallable, Category = "SplineAlongSurface|Functions")
	static void RunConstructionScripts(AActor* Actor);
};
