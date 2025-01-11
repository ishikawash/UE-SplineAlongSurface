#include "SplineAlongSurfaceDrawTool.h"
#include "Components/SplineComponent.h"
#include "SplineAlongSurface_Functions.h"
#include "SplineAlongSurfaceActor.h"

void USplineAlongSurfaceDrawTool::Shutdown(EToolShutdownType ShutdownType)
{
    UScriptableInteractiveTool::Shutdown(ShutdownType);

    if (ShutdownType == EToolShutdownType::Cancel)
    {
        if (IsValid(SplineActor))
        {
            SplineActor->Destroy();
        }
    }
}

void USplineAlongSurfaceDrawTool::InputSurfacePoint(const FVector& WorldLocation, const FVector& WorldNormal, bool bRunConstructionScripts)
{
    if (!IsValid(SplineActor))
    {
        return;
    }

    FString Context;
    FText Description = FText::FromString(TEXT("Add SurfacePoint"));
    GEngine->BeginTransaction(*Context, Description, nullptr);
    {
        SplineActor->Modify();
        InputSurfacePointWithoutTransaction(WorldLocation, WorldNormal, bRunConstructionScripts);
    }
    GEngine->EndTransaction();
}

void USplineAlongSurfaceDrawTool::InputSurfacePointWithoutTransaction(const FVector& WorldLocation, const FVector& WorldNormal, bool bRunConstructionScripts)
{
    if (!IsValid(SplineActor))
    {
        return;
    }
    if (SplineActor->GetSpline() == nullptr)
    {
        return;
    }
 
    FTransform WorldTransform = SplineActor->GetSpline()->GetComponentToWorld();
    FVector LocalLocation = WorldTransform.InverseTransformPosition(WorldLocation);
    FVector LocalNormal = WorldTransform.InverseTransformVectorNoScale(WorldNormal);
    FSurfacePoint SurfacePoint = { LocalLocation, LocalNormal };
    SplineActor->GetSurfacePoints().Add(SurfacePoint);

    if (bRunConstructionScripts)
    {
        SplineActor->RerunConstructionScripts();
    }
}
