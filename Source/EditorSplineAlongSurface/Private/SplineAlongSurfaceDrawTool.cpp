#include "SplineAlongSurfaceDrawTool.h"
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
