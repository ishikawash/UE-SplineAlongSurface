#include "SplineAlongSurfaceDrawTool.h"
#include "Components/SplineComponent.h"
#include "GeometryScript/ShapeFunctions.h"
#include "Kismet/KismetSystemLibrary.h"
#include "SplineAlongSurface_Functions.h"
#include "SplineAlongSurfaceActor.h"
#include "SplineAlongSurfacePropertySet.h"

namespace SplineAlongSurfaceDrawTool::Private
{
    void GetRayStartEnd(FRay Ray, FVector& StartPoint, FVector& EndPoint)
    {
        UGeometryScriptLibrary_RayFunctions::GetRayStartEnd(Ray, 0.0, 0.0, StartPoint, EndPoint);
    }
}

USplineAlongSurfaceDrawTool::USplineAlongSurfaceDrawTool()
{
    ToolName = FText::FromString(TEXT("Draw Spline Along Surface"));
    ToolTooltip = ToolName;
    ToolShutdownType = EScriptableToolShutdownType::AcceptCancel;
}

void USplineAlongSurfaceDrawTool::Setup()
{
    UEditorScriptableSingleClickTool::Setup();

    FString Identifier = TEXT("DrawToolSettings");
    EToolsFrameworkOutcomePins Outcome;
    PropertySet = AddPropertySetOfType(USplineAlongSurfacePropertySet::StaticClass(), Identifier, Outcome);
    if (PropertySet == nullptr)
    {
        return;
    }

    {
        FToolObjectPropertyModifiedDelegate EventObserver;
        EventObserver.BindUFunction(this, FName(TEXT("OnSplineActorClassChanged")));
        WatchObjectProperty(PropertySet, TEXT("SplineActorClass"), EventObserver);
    }
    {
        FToolFloatPropertyModifiedDelegate EventObserver;
        EventObserver.BindUFunction(this, FName(TEXT("OnDistanceFromSurfaceChanged")));
        WatchFloatProperty(PropertySet, TEXT("DistanceFromSurface"), EventObserver);
    }
    {
        FToolFloatPropertyModifiedDelegate EventObserver;
        EventObserver.BindUFunction(this, FName(TEXT("OnTangentLengthChanged")));
        WatchFloatProperty(PropertySet, TEXT("TangentLength"), EventObserver);
    }
    {
        FToolFloatPropertyModifiedDelegate EventObserver;
        EventObserver.BindUFunction(this, FName(TEXT("OnMaxHeightOfObstacleChanged")));
        WatchFloatProperty(PropertySet, TEXT("MaxHeightOfObstacle"), EventObserver);
    }
    {
        FToolBoolPropertyModifiedDelegate EventObserver;
        EventObserver.BindUFunction(this, FName(TEXT("OnOnlyPolyLineChanged")));
        WatchBoolProperty(PropertySet, TEXT("OnlyPolyLine"), EventObserver);
    }
}

void USplineAlongSurfaceDrawTool::Shutdown(EToolShutdownType ShutdownType)
{
    UEditorScriptableSingleClickTool::Shutdown(ShutdownType);

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
    HandleOnHitClickEvent(ClickPos, Modifiers);
}

void USplineAlongSurfaceDrawTool::HandleOnHitClickEvent(FInputDeviceRay ClickPos, const FScriptableToolModifierStates& Modifiers)
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
    if (SplineActor->GetSplineComponent() == nullptr)
    {
        return;
    }
 
    FTransform WorldTransform = SplineActor->GetSplineComponent()->GetComponentToWorld();
    FVector LocalLocation = WorldTransform.InverseTransformPosition(WorldLocation);
    FVector LocalNormal = WorldTransform.InverseTransformVectorNoScale(WorldNormal);
    FSurfacePoint SurfacePoint = { LocalLocation, LocalNormal };
    SplineActor->GetSurfacePoints().Add(SurfacePoint);

    if (bRunConstructionScripts)
    {
        SplineActor->RerunConstructionScripts();
    }
}

void USplineAlongSurfaceDrawTool::OnSplineActorClassChanged(UScriptableInteractiveToolPropertySet* TargetPropertySet, FString PropertyName, UObject* NewValue)
{
    UClass* NewClass = Cast<UClass>(NewValue);
    SplineActorClass = NewClass;
}

void USplineAlongSurfaceDrawTool::OnDistanceFromSurfaceChanged(UScriptableInteractiveToolPropertySet* TargetPropertySet, FString PropertyName, double NewValue)
{
    if (IsValid(SplineActor))
    {
        FSplineAlongSurfaceParams& Params = SplineActor->GetParams();
        Params.DistanceFromSurface = NewValue;

        SplineActor->RerunConstructionScripts();
    }
}

void USplineAlongSurfaceDrawTool::OnTangentLengthChanged(UScriptableInteractiveToolPropertySet* TargetPropertySet, FString PropertyName, double NewValue)
{
    if (IsValid(SplineActor))
    {
        FSplineAlongSurfaceParams& Params = SplineActor->GetParams();
        Params.TangentLength = NewValue;

        SplineActor->RerunConstructionScripts();
    }
}

void USplineAlongSurfaceDrawTool::OnMaxHeightOfObstacleChanged(UScriptableInteractiveToolPropertySet* TargetPropertySet, FString PropertyName, double NewValue)
{
    if (IsValid(SplineActor))
    {
        FSplineAlongSurfaceParams& Params = SplineActor->GetParams();
        Params.MaxHeightOfObstacle = NewValue;

        SplineActor->RerunConstructionScripts();
    }
}

void USplineAlongSurfaceDrawTool::OnOnlyPolyLineChanged(UScriptableInteractiveToolPropertySet* TargetPropertySet, FString PropertyName, bool NewValue)
{
    if (IsValid(SplineActor))
    {
        FSplineAlongSurfaceParams& Params = SplineActor->GetParams();
        Params.bOnlyPolyLine = NewValue;

        SplineActor->RerunConstructionScripts();
    }
}
