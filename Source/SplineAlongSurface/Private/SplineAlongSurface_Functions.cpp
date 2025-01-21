#include "SplineAlongSurface_Functions.h"
#include "Components/SplineComponent.h"
#include "GeometryScript/ShapeFunctions.h"
#include "KismetTraceUtils.h"

namespace SplineAlongSurface::Private
{
    FTransform MakeTransformFromZXAxes(const FVector& Location, const FVector& ZAxis, const FVector& XAxis)
    {
        return UGeometryScriptLibrary_TransformFunctions::MakeTransformFromAxes(Location, ZAxis, XAxis);
    }

    FTransform MakeTransformFromZAxes(const FVector& Location, const FVector& ZAxis)
    {
        return UGeometryScriptLibrary_TransformFunctions::MakeTransformFromZAxis(Location, ZAxis);
    }

    FCollisionQueryParams MakeCollisionQueryParams(const UObject* WorldContextObject)
    {
        FCollisionQueryParams Params;
        const AActor* IgnoredActor = Cast<AActor>(WorldContextObject);
        if (IgnoredActor)
        {
            Params.AddIgnoredActor(IgnoredActor);
        }
        return Params;
    }
}

bool USplineAlongSurface_Functions::FindWayPointTransform(const FTransform& A, const FTransform& B, const FVector& UpA, const FVector& UpB, FTransform& C)
{
    FVector P1 = A.GetLocation();
    FVector P2 = B.GetLocation();
    FVector P2P1 = P2 - P1;
    FVector P3P1 = FVector::VectorPlaneProject(P2P1, UpA);
    float Dot = FVector::DotProduct(P2P1.GetSafeNormal(), P3P1.GetSafeNormal());
    if (Dot >= 0.999)
    {
        return false;
    }

    FVector P3 = A.GetLocation() + P3P1;

    float DihedralAngle = FMath::Acos(FVector::DotProduct(UpA, UpB));
    float R1 = FVector::Distance(P2, P3) / FMath::Sin(DihedralAngle);
    float R2 = FMath::Cos(DihedralAngle) * R1;
    FVector QP3 = R2 * (P1 - P3).GetSafeNormal();
    FVector Q = P3 + QP3;
    FVector QP2 = (P2 - Q).GetSafeNormal();
    C = SplineAlongSurface::Private::MakeTransformFromZXAxes(Q, UpB, QP2);
    return true;
}

void USplineAlongSurface_Functions::ComputeWayPoints(const TArray<FSurfacePoint>& SurfacePoints, TArray<FTransform>& OutWayPoints, float OffsetLength)
{
    if (SurfacePoints.Num() < 2)
    {
        return;
    }

    // [1] Create list of transforms.
    TArray<FTransform> WayPoints_A;
    for (int i = 1; i < SurfacePoints.Num(); i++)
    {
        const FSurfacePoint& P0 = SurfacePoints[i - 1];
        const FSurfacePoint& P1 = SurfacePoints[i];
        FVector Forward = (P1.Location - P0.Location).GetSafeNormal();
        FTransform WayPoint = SplineAlongSurface::Private::MakeTransformFromZXAxes(P0.Location, P0.Normal, Forward);
        WayPoints_A.Add(WayPoint);
    }
    {
        const FSurfacePoint& P = SurfacePoints.Last();
        FTransform WayPoint = SplineAlongSurface::Private::MakeTransformFromZAxes(P.Location, P.Normal);
        WayPoints_A.Add(WayPoint);
    }

    // [2] Move location of transform along normal.
    for (int i = 0; i < WayPoints_A.Num(); i++)
    {
        FTransform& WayPoint = WayPoints_A[i];
        FVector Location = WayPoint.GetLocation() + SurfacePoints[i].Normal * OffsetLength;
        WayPoint.SetLocation(Location);
    }

    // [3] Compute waypoint at corner.
    TArray<FTransform> WayPoints_B;
    for (int i = 0; i < WayPoints_A.Num() - 1; i++)
    {
        const FTransform& P0 = WayPoints_A[i];
        const FTransform& P1 = WayPoints_A[i + 1];
        WayPoints_B.Add(P0);

        const FVector& Up0 = SurfacePoints[i].Normal;
        const FVector& Up1 = SurfacePoints[i + 1].Normal;
        FTransform P2;
        if (FindWayPointTransform(P0, P1, Up0, Up1, P2))
        {
            WayPoints_B.Add(P2);
        }
    }
    {
        FTransform& WayPoint = WayPoints_A.Last();
        WayPoints_B.Add(WayPoint);
    }

    Swap(WayPoints_B, OutWayPoints);
}

bool USplineAlongSurface_Functions::DetectObstacle(const UObject* WorldContextObject, const FVector& WorldStart, const FVector& WorldEnd, const FVector& WorldUp, TArray<FSurfacePoint>& OutWayPoints, float OffsetLength)
{
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (World == nullptr)
    {
        return false;
    }
    const FCollisionQueryParams QueryParams = SplineAlongSurface::Private::MakeCollisionQueryParams(WorldContextObject);

    auto LineTrace = [World, &QueryParams](const FVector& Start, const FVector& End, FSurfacePoint& OutWayPoint) -> bool
    {
        FHitResult Hit;
        if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, QueryParams))
        {
            OutWayPoint.Location = Hit.Location;
            OutWayPoint.Normal = Hit.Normal;
            return true;
        }
        else
        {
            return false;
        }
    };

    TArray<FSurfacePoint> WayPoints;
    WayPoints.SetNum(3);
    if (!LineTrace(WorldStart, WorldEnd, WayPoints[0]))
    {
        return false;
    }
    if (!LineTrace(WorldEnd, WorldStart, WayPoints[2]))
    {
        return false;
    }
    if (WayPoints[0].Location.Equals(WayPoints[2].Location))
    {
        return false;
    }
    FVector EndAtCenter = (WayPoints[0].Location + WayPoints[2].Location) * 0.5f;
    FVector StartAtCenter = EndAtCenter + WorldUp * OffsetLength;
    if (!LineTrace(StartAtCenter, EndAtCenter, WayPoints[1]))
    {
        return false;
    }

    Swap(WayPoints, OutWayPoints);
    return true;
}

FSurfacePoint USplineAlongSurface_Functions::TransformFromWorldToLocal(const FSurfacePoint& InSurfacePoint, const FTransform& WorldTransform)
{
    FSurfacePoint OutSurfacePoint;
    OutSurfacePoint.Location = WorldTransform.InverseTransformPosition(InSurfacePoint.Location);
    OutSurfacePoint.Normal = WorldTransform.InverseTransformVectorNoScale(InSurfacePoint.Normal);
    return OutSurfacePoint;
}

bool USplineAlongSurface_Functions::TestTwoTransformDirections(const FTransform& A, const FTransform& B)
{
    FVector V0 = A.Rotator().Vector();
    FVector V1 = B.Rotator().Vector();
    float Dot = FVector::DotProduct(V0, V1);
    return (Dot >= 0.99);
}

void USplineAlongSurface_Functions::ComputeControlPointsAtCorner(const FTransform& A, const FTransform& B, TArray<FTransform>& OutControlPoints, float OffsetLength)
{
    TArray<FTransform> ControlPoints;
    {
        FVector RelativeLocation = A.Rotator().Vector() * OffsetLength * -1;
        FVector Location = B.GetLocation() + RelativeLocation;
        FTransform WayPoint(A.Rotator(), Location);
        ControlPoints.Add(WayPoint);
    }
    {
        FVector RelativeLocation = B.Rotator().Vector() * OffsetLength;
        FVector Location = B.GetLocation() + RelativeLocation;
        FTransform WayPoint(B.Rotator(), Location);
        ControlPoints.Add(WayPoint);
    }
    Swap(ControlPoints, OutControlPoints);
}

void USplineAlongSurface_Functions::CleanupRedundantControlPoints(const TArray<FTransform>& InControlPoints, TArray<FTransform>& OutControlPoints)
{
    if (InControlPoints.Num() < 2)
    {
        return;
    }

    // [1] Find two points that are close to each other and eliminate one of them.
    TArray<FTransform> ControlPoints_A;
    for (int i = 0; i < InControlPoints.Num() - 1; i++)
    {
        FVector P0 = InControlPoints[i].GetLocation();
        FVector P1 = InControlPoints[i + 1].GetLocation();
        if (FVector::Distance(P0, P1) >= 1.0f)
        {
            ControlPoints_A.Add(InControlPoints[i]);
        }
    }
    ControlPoints_A.Add(InControlPoints.Last());

    if (ControlPoints_A.Num() < 2)
    {
        Swap(ControlPoints_A, OutControlPoints);
        return;
    }

    // [2] Merge the two points whose positions have been reversed.
    TArray<FTransform> ControlPoints_B;
    for (int i = 0; i < ControlPoints_A.Num() - 1; i++)
    {
        FVector P0 = ControlPoints_A[i].Rotator().Vector();
        FVector P1 = (ControlPoints_A[i + 1].GetLocation() - ControlPoints_A[i].GetLocation()).GetSafeNormal();
        if (FVector::DotProduct(P0, P1) >= 0.0f)
        {
            ControlPoints_B.Add(ControlPoints_A[i]);
        }
        else
        {
            FVector Location = (ControlPoints_A[i].GetLocation() + ControlPoints_A[i + 1].GetLocation()) * 0.5f;
            FTransform ControlPoint(ControlPoints_A[i].Rotator(), Location);
            ControlPoints_B.Add(ControlPoint);
            i++;
        }
    }
    ControlPoints_B.Add(ControlPoints_A.Last());

    Swap(ControlPoints_B, OutControlPoints);
}

bool USplineAlongSurface_Functions::ComputePolyLinePoints(USceneComponent* Scene, const TArray<FSurfacePoint>& SurfacePoints, TArray<FTransform>& OutLinePoints, float DistanceFromSurface, float MaxHeightOfObstacle)
{
    if (SurfacePoints.Num() < 2)
    {
        return false;
    }

    TArray<FTransform> LinePoints_A;
    ComputeWayPoints(SurfacePoints, LinePoints_A, DistanceFromSurface);

    const FTransform& WorldTransform = Scene->GetComponentToWorld();
    TArray<FTransform> LinePoints_B;
    {
        for (int K = 0; K < LinePoints_A.Num() - 1; K++)
        {
            FTransform& P0 = LinePoints_A[K];
            FTransform& P1 = LinePoints_A[K + 1];
            LinePoints_B.Add(P0);

            FVector WorldStart = WorldTransform.TransformPosition(P0.GetLocation());
            FVector WorldEnd = WorldTransform.TransformPosition(P1.GetLocation());
            FVector WorldUp = FRotationMatrix(P0.Rotator()).GetScaledAxis(EAxis::Z);
            TArray<FSurfacePoint> WorldWayPoints;
            bool Found = DetectObstacle(Scene, WorldStart, WorldEnd, WorldUp, WorldWayPoints, MaxHeightOfObstacle);
            if (!Found)
            {
                continue;
            }

            TArray<FSurfacePoint> LocalWayPoints;
            for (const FSurfacePoint& WorldWayPoint : WorldWayPoints)
            {
                FSurfacePoint LocalWayPoint = TransformFromWorldToLocal(WorldWayPoint, WorldTransform);
                LocalWayPoints.Add(LocalWayPoint);
            }

            TArray<FTransform> WayPointTransforms;
            ComputeWayPoints(LocalWayPoints, WayPointTransforms, DistanceFromSurface);
            for (FTransform& WayPointTransform : WayPointTransforms)
            {
                LinePoints_B.Add(WayPointTransform);
            }
            {
                FTransform A = LinePoints_B.Last();
                FTransform B = LinePoints_A[K];
                FTransform C = FTransform(B.Rotator(), A.GetLocation());
                LinePoints_B[LinePoints_B.Num() - 1] = C;
            }
        }

        {
            FTransform LinePoint = LinePoints_A.Last();
            LinePoints_B.Add(LinePoint);
        }
    }

    Swap(LinePoints_B, OutLinePoints);

    return true;
}

bool USplineAlongSurface_Functions::ComputeControlPoints(const TArray<FTransform>& LinePoints, TArray<FTransform>& OutControlPoints, float TangentLength)
{
    if (LinePoints.Num() < 2)
    {
        return false;
    }

    TArray<FTransform> ControlPoints_A;
    {
        ControlPoints_A.Add(LinePoints[0]);

        for (int i = 1; i < LinePoints.Num(); i++)
        {
            const FTransform& P0 = LinePoints[i - 1];
            const FTransform& P1 = LinePoints[i];
            if (TestTwoTransformDirections(P0, P1))
            {
                ControlPoints_A.Add(P1);
            }
            else
            {
                TArray<FTransform> ControlPoints;
                ComputeControlPointsAtCorner(P0, P1, ControlPoints, TangentLength);
                ControlPoints_A.Append(ControlPoints);
            }
        }
    }

    TArray<FTransform> ControlPoints_B;
    CleanupRedundantControlPoints(ControlPoints_A, ControlPoints_B);

    Swap(ControlPoints_B, OutControlPoints);

    return true;
}

void USplineAlongSurface_Functions::BuildPolyLine(USplineComponent* Spline, const TArray<FSurfacePoint>& SurfacePoints, float DistanceFromSurface, float MaxHeightOfObstacle)
{
    Spline->ClearSplinePoints();

    TArray<FTransform> LinePoints;
    if (!ComputePolyLinePoints(Spline, SurfacePoints, LinePoints, DistanceFromSurface, MaxHeightOfObstacle))
    {
        return;
    }

    for (int i = 0; i < LinePoints.Num(); i++)
    {
        FTransform& LinePoint = LinePoints[i];
        FVector Location = LinePoint.GetLocation();
        FRotator Rotation = LinePoint.Rotator();
        Spline->AddSplinePointAtIndex(Location, i, ESplineCoordinateSpace::Type::Local, false);
        Spline->SetRotationAtSplinePoint(i, Rotation, ESplineCoordinateSpace::Type::Local, false);
        Spline->SetSplinePointType(i, ESplinePointType::Type::Linear, false);
    }

    Spline->UpdateSpline();
}

void USplineAlongSurface_Functions::BuildCurve(USplineComponent* Spline, const TArray<FTransform>& LinePoints, float TangentLength)
{
    Spline->ClearSplinePoints();

    TArray<FTransform> ControlPoints;
    if (!ComputeControlPoints(LinePoints, ControlPoints, TangentLength))
    {
        return;
    }

    for (int i = 0; i < ControlPoints.Num(); i++)
    {
        FTransform& ControlPoint = ControlPoints[i];
        FVector Location = ControlPoint.GetLocation();
        FRotator Rotation = ControlPoint.Rotator();
        Spline->AddSplinePointAtIndex(Location, i, ESplineCoordinateSpace::Type::Local, false);
        Spline->SetRotationAtSplinePoint(i, Rotation, ESplineCoordinateSpace::Type::Local, false);
        Spline->SetSplinePointType(i, ESplinePointType::Type::CurveCustomTangent, false);

        FVector Tangent = Rotation.Vector() * TangentLength;
        Spline->SetTangentAtSplinePoint(i, Tangent, ESplineCoordinateSpace::Type::Local, false);
    }

    Spline->UpdateSpline();
}

void USplineAlongSurface_Functions::BuildSplineAlongSurface(USplineComponent* Spline, const TArray<FSurfacePoint>& SurfacePoints, const FSplineAlongSurfaceParams& Params)
{
    BuildPolyLine(Spline, SurfacePoints, Params.DistanceFromSurface, Params.MaxHeightOfObstacle);
    if (Params.bOnlyPolyLine)
    {
        return;
    }

    TArray<FTransform> PolyLinePoints;
    int SplinePointNum = Spline->GetNumberOfSplinePoints();
    for (int i = 0; i < SplinePointNum; i++)
    {
        FSplinePoint SplinePoint = Spline->GetSplinePointAt(i, ESplineCoordinateSpace::Type::Local);
        FTransform PolyLinePoint(SplinePoint.Rotation, SplinePoint.Position, SplinePoint.Scale);
        PolyLinePoints.Add(PolyLinePoint);
    }

    BuildCurve(Spline, PolyLinePoints, Params.TangentLength);
}
