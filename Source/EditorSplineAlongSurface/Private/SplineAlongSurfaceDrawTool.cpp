#include "SplineAlongSurfaceDrawTool.h"
#include "Components/SplineComponent.h"
#include "GeometryScript/ShapeFunctions.h"
#include "Kismet/KismetSystemLibrary.h"
#include "SplineAlongSurface_Functions.h"
#include "SplineAlongSurfaceActor.h"

namespace SplineAlongSurfaceDrawTool::Private
{
    void GetRayStartEnd(FRay Ray, FVector& StartPoint, FVector& EndPoint)
    {
        UGeometryScriptLibrary_RayFunctions::GetRayStartEnd(Ray, 0.0, 0.0, StartPoint, EndPoint);
    }
}

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

void USplineAlongSurfaceDrawTool::OnHitByClick_Implementation(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers)
{
    FVector Start;
    FVector End;
    SplineAlongSurfaceDrawTool::Private::GetRayStartEnd(ClickPos.WorldRay, Start, End);

    FHitResult Hit;
    if (!GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
    {
        return;
    }

#if ENABLE_DRAW_DEBUG
    {
        FVector LineStart = Hit.Location;
        FVector LineEnd = Hit.Location + Hit.Normal * 50.f;
        float ArrowSize = 0.f;
        FLinearColor LineColor(0.f, 0.f, 1.f);
        float Duration = 5.f;
        float Thickness = 2.f;
        DrawDebugDirectionalArrow(GetWorld(), LineStart, LineEnd, ArrowSize, LineColor.ToFColor(true), false, Duration, SDPG_World, Thickness);
    }
#endif

    if (SpawnSplineActor(Hit.Location))
    {
        InputSurfacePoint(Hit.Location, Hit.Normal, true);
    }
}

ASplineAlongSurface* USplineAlongSurfaceDrawTool::SpawnSplineActor(const FVector& WorldLocation)
{
    if (IsValid(SplineActor))
    {
        return SplineActor;
    }
    if (!IsValid(SplineActorClass))
    {
        return nullptr;
    }
    
    FTransform WorldTransform;
    WorldTransform.SetLocation(WorldLocation);
    SplineActor = GetWorld()->SpawnActor<ASplineAlongSurface>(SplineActorClass.Get(), WorldTransform);
    return SplineActor;
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
