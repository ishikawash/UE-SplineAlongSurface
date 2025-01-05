#include "SplineAlongSurfaceActor.h"
#include "Components/SplineComponent.h"

ASplineAlongSurface::ASplineAlongSurface()
{
	PrimaryActorTick.bCanEverTick = false;

    USceneComponent* SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent0"));
    SetRootComponent(SceneComponent);

    Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline0"));
    Spline->SetupAttachment(RootComponent);
}

void ASplineAlongSurface::BuildSpline()
{
    USplineAlongSurface_Functions::BuildSplineAlongSurface(Spline, SurfacePoints, SplineAlongSurfaceParams);
}
