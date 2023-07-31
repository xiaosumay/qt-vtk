/** @file       lxInteractorStyle.cpp
 *  @brief
 *  @author     Mei Zhaorui
 *  @version    1.0
 *  @date       2023-07-28
 *  @copyright  Heli Co., Ltd. All rights reserved.
 */
#include "lxInteractorStyle.h"

#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>
#include <vtkUnsignedCharArray.h>
#include <vtkPropPicker.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkRenderer.h>

#include <cctype>

vtkStandardNewMacro(lxInteractorStyle);
namespace
{
enum
{
    VTKIS_RUBBERBANDPICK = 1000,
    VTKIS_ACTORMOVING
};

}  // namespace

struct lxInteractorStyle::DataImpl
{
    int state;
    double motion_factor;

    int start_position[2];
    int end_position[2];

    vtkActor *last_actor;
    double motion_vector[3];
    int selected_area[4];

    vtkNew<vtkUnsignedCharArray> pixel_array;
    vtkNew<vtkPropPicker> prop_picker;
};

lxInteractorStyle::lxInteractorStyle()
    : _self(new DataImpl{})
{
    _self->motion_factor = 10.0;
}

lxInteractorStyle::~lxInteractorStyle() {}

//----------------------------------------------------------------------------
void lxInteractorStyle::OnMouseMove()
{
    int x = Interactor->GetEventPosition()[0];
    int y = Interactor->GetEventPosition()[1];
    FindPokedRenderer(x, y);

    switch (State)
    {
    case VTKIS_ROTATE:
        Rotate();
        break;

    case VTKIS_PAN:
        Pan();
        break;

    case VTKIS_DOLLY:
        Dolly();
        break;

    case VTKIS_SPIN:
        Spin();
        break;

    case VTKIS_RUBBERBANDPICK:
        RedrawRubberBand();
        break;

    case VTKIS_ACTORMOVING:
        MoveActor();
        break;
    }

    InvokeEvent(vtkCommand::InteractionEvent, nullptr);
}

//----------------------------------------------------------------------------
void lxInteractorStyle::OnLeftButtonDown()
{
    FindPokedRenderer(Interactor->GetEventPosition()[0], Interactor->GetEventPosition()[1]);
    if (CurrentRenderer == nullptr)
    {
        return;
    }

    GrabFocus(EventCallbackCommand);
    if (Interactor->GetShiftKey())
    {
        StartRotate();
    }
    else if (Interactor->GetAltKey())
    {
        StartRubberBandPick();
    }
    else
    {
        if (Interactor->GetControlKey())
        {
            if (IsOnActor())
            {
                StartMoveActor();
            }
            else
            {
                StartSpin();
            }
        }
        else if (IsOnActor())
        {
            StartMoveActor();
        }
        else
        {
            StartPan();
        }
    }
}

//----------------------------------------------------------------------------
void lxInteractorStyle::OnLeftButtonUp()
{
    switch (State)
    {
    case VTKIS_DOLLY:
        EndDolly();
        break;

    case VTKIS_PAN:
        EndPan();
        break;

    case VTKIS_SPIN:
        EndSpin();
        break;

    case VTKIS_ROTATE:
        EndRotate();
        break;

    case VTKIS_RUBBERBANDPICK:
        EndRubberBandPick();
        break;

    case VTKIS_ACTORMOVING:
        EndMoveActor();
        break;
    }

    if (Interactor)
    {
        ReleaseFocus();
    }
}

//----------------------------------------------------------------------------
void lxInteractorStyle::OnRightButtonDown()
{
#if 0
    FindPokedRenderer(Interactor->GetEventPosition()[0], Interactor->GetEventPosition()[1]);
    if (CurrentRenderer == nullptr)
    {
        return;
    }

    GrabFocus(EventCallbackCommand);
    StartDolly();
#endif
}

//----------------------------------------------------------------------------
void lxInteractorStyle::OnRightButtonUp()
{
#if 0
    switch (State)
    {
    case VTKIS_DOLLY:
        EndDolly();

        if (Interactor)
        {
            ReleaseFocus();
        }
        break;
    }
#else
    InvokeEvent(LxEventIdsEx::kRightButtonUpEvent, GetInteractor());
#endif
}

//----------------------------------------------------------------------------
void lxInteractorStyle::OnMouseWheelForward()
{
    FindPokedRenderer(Interactor->GetEventPosition()[0], Interactor->GetEventPosition()[1]);
    if (CurrentRenderer == nullptr)
    {
        return;
    }

    GrabFocus(EventCallbackCommand);
    StartDolly();
    double factor = _self->motion_factor * 0.2 * MouseWheelMotionFactor;
    dolly(pow(1.1, factor));
    EndDolly();
    ReleaseFocus();
}

//----------------------------------------------------------------------------
void lxInteractorStyle::OnMouseWheelBackward()
{
    FindPokedRenderer(Interactor->GetEventPosition()[0], Interactor->GetEventPosition()[1]);
    if (CurrentRenderer == nullptr)
    {
        return;
    }

    GrabFocus(EventCallbackCommand);
    StartDolly();
    double factor = _self->motion_factor * -0.2 * MouseWheelMotionFactor;
    dolly(pow(1.1, factor));
    EndDolly();
    ReleaseFocus();
}

void lxInteractorStyle::OnChar()
{
    vtkRenderWindowInteractor *rwi = Interactor;

    switch (rwi->GetKeyCode())
    {
    case 'p':
    case 'P':
    {
        FindPokedRenderer(rwi->GetEventPosition()[0], rwi->GetEventPosition()[1]);
        if (CurrentRenderer == nullptr) return;

        auto camera = CurrentRenderer->GetActiveCamera();

        camera->SetParallelProjection(!camera->GetParallelProjection());
    }
    break;
    case 'r':
    case 'R':
    {
        Superclass::OnChar();

        FindPokedRenderer(rwi->GetEventPosition()[0], rwi->GetEventPosition()[1]);
        if (CurrentRenderer == nullptr) return;

        auto camera = CurrentRenderer->GetActiveCamera();

        camera->ParallelProjectionOff();
    }
    break;
    }
}

//----------------------------------------------------------------------------
void lxInteractorStyle::OnTimer()
{
    vtkRenderWindowInteractor *rwi = this->Interactor;

    switch (this->State)
    {
    case VTKIS_NONE:
    case VTKIS_TIMER:
        rwi->Render();
        break;
    default:
        break;
    }
}

//----------------------------------------------------------------------------
void lxInteractorStyle::Rotate()
{
    if (CurrentRenderer == nullptr)
    {
        return;
    }

    vtkRenderWindowInteractor *rwi = Interactor;

    int dx = rwi->GetEventPosition()[0] - rwi->GetLastEventPosition()[0];
    int dy = rwi->GetEventPosition()[1] - rwi->GetLastEventPosition()[1];

    int *size = CurrentRenderer->GetRenderWindow()->GetSize();

    double delta_elevation = -20.0 / size[1];
    double delta_azimuth = -20.0 / size[0];

    double rxf = dx * delta_azimuth * _self->motion_factor;
    double ryf = dy * delta_elevation * _self->motion_factor;

    vtkCamera *camera = CurrentRenderer->GetActiveCamera();
    camera->Azimuth(rxf);
    camera->Elevation(ryf);
    camera->OrthogonalizeViewUp();

    if (AutoAdjustCameraClippingRange)
    {
        CurrentRenderer->ResetCameraClippingRange();
    }

    if (rwi->GetLightFollowCamera())
    {
        CurrentRenderer->UpdateLightsGeometryToFollowCamera();
    }

    rwi->Render();
}

//----------------------------------------------------------------------------
void lxInteractorStyle::Spin()
{
    if (CurrentRenderer == nullptr)
    {
        return;
    }

    vtkRenderWindowInteractor *rwi = Interactor;

    double *center = CurrentRenderer->GetCenter();

    double newAngle = vtkMath::DegreesFromRadians(
        atan2(rwi->GetEventPosition()[1] - center[1], rwi->GetEventPosition()[0] - center[0]));

    double oldAngle = vtkMath::DegreesFromRadians(
        atan2(rwi->GetLastEventPosition()[1] - center[1], rwi->GetLastEventPosition()[0] - center[0]));

    vtkCamera *camera = CurrentRenderer->GetActiveCamera();
    camera->Roll(newAngle - oldAngle);
    camera->OrthogonalizeViewUp();

    rwi->Render();
}

//----------------------------------------------------------------------------
void lxInteractorStyle::Pan()
{
    if (CurrentRenderer == nullptr)
    {
        return;
    }

    vtkRenderWindowInteractor *rwi = Interactor;

    double viewFocus[4], focalDepth, viewPoint[3];
    double newPickPoint[4], oldPickPoint[4], motionVector[3];

    // Calculate the focal depth since we'll be using it a lot

    vtkCamera *camera = CurrentRenderer->GetActiveCamera();
    camera->GetFocalPoint(viewFocus);
    ComputeWorldToDisplay(viewFocus[0], viewFocus[1], viewFocus[2], viewFocus);
    focalDepth = viewFocus[2];

    ComputeDisplayToWorld(rwi->GetEventPosition()[0], rwi->GetEventPosition()[1], focalDepth, newPickPoint);

    // Has to recalc old mouse point since the viewport has moved,
    // so can't move it outside the loop

    ComputeDisplayToWorld(rwi->GetLastEventPosition()[0], rwi->GetLastEventPosition()[1], focalDepth, oldPickPoint);

    // Camera motion is reversed

    motionVector[0] = oldPickPoint[0] - newPickPoint[0];
    motionVector[1] = oldPickPoint[1] - newPickPoint[1];
    motionVector[2] = oldPickPoint[2] - newPickPoint[2];

    camera->GetFocalPoint(viewFocus);
    camera->GetPosition(viewPoint);
    camera->SetFocalPoint(motionVector[0] + viewFocus[0],
        motionVector[1] + viewFocus[1],
        motionVector[2] + viewFocus[2]);

    camera->SetPosition(motionVector[0] + viewPoint[0], motionVector[1] + viewPoint[1], motionVector[2] + viewPoint[2]);

    if (rwi->GetLightFollowCamera())
    {
        CurrentRenderer->UpdateLightsGeometryToFollowCamera();
    }

    rwi->Render();
}

//----------------------------------------------------------------------------
void lxInteractorStyle::Dolly()
{
    if (CurrentRenderer == nullptr)
    {
        return;
    }

    vtkRenderWindowInteractor *rwi = Interactor;
    double *center = CurrentRenderer->GetCenter();
    int dy = rwi->GetEventPosition()[1] - rwi->GetLastEventPosition()[1];
    double dyf = _self->motion_factor * dy / center[1];
    dolly(pow(1.1, dyf));
}

//----------------------------------------------------------------------------
void lxInteractorStyle::StartMoveActor()
{
    StartState(VTKIS_ACTORMOVING);
    InvokeEvent(LxEventIdsEx::kSingleActorClickedEvent, _self->last_actor);
}

//----------------------------------------------------------------------------
void lxInteractorStyle::EndMoveActor()
{
    _self->last_actor = nullptr;
    StopState();
}

//----------------------------------------------------------------------------
void lxInteractorStyle::dolly(double factor)
{
    if (CurrentRenderer == nullptr)
    {
        return;
    }

    vtkCamera *camera = CurrentRenderer->GetActiveCamera();
    if (camera->GetParallelProjection())
    {
        camera->SetParallelScale(camera->GetParallelScale() / factor);
    }
    else
    {
        camera->Dolly(factor);
        if (AutoAdjustCameraClippingRange)
        {
            CurrentRenderer->ResetCameraClippingRange();
        }
    }

    if (Interactor->GetLightFollowCamera())
    {
        CurrentRenderer->UpdateLightsGeometryToFollowCamera();
    }

    Interactor->Render();
}

//----------------------------------------------------------------------------
void lxInteractorStyle::PrintSelf(ostream &os, vtkIndent indent)
{
    Superclass::PrintSelf(os, indent);
    os << indent << "MotionFactor: " << _self->motion_factor << "\n";
}

//----------------------------------------------------------------------------
void lxInteractorStyle::StartRubberBandPick()
{
    if (State != VTKIS_NONE)
    {
        return;
    }
    StartState(VTKIS_RUBBERBANDPICK);

    vtkRenderWindow *ren_win = GetInteractor()->GetRenderWindow();

    _self->start_position[0] = GetInteractor()->GetEventPosition()[0];
    _self->start_position[1] = GetInteractor()->GetEventPosition()[1];
    _self->end_position[0] = _self->start_position[0];
    _self->end_position[1] = _self->start_position[1];

    int *size = ren_win->GetSize();
    _self->pixel_array->SetNumberOfTuples(size[0] * size[1]);

    ren_win->GetRGBACharPixelData(0, 0, size[0] - 1, size[1] - 1, 1, _self->pixel_array);
}

//----------------------------------------------------------------------------
void lxInteractorStyle::EndRubberBandPick()
{
    if (State != VTKIS_RUBBERBANDPICK)
    {
        return;
    }
    StopState();

    int *size = GetInteractor()->GetRenderWindow()->GetSize();

    /* clang-format off */
    int min[2], max[2];
    min[0] = _self->start_position[0] <= _self->end_position[0] ? _self->start_position[0] : _self->end_position[0];
    if (min[0] < 0) { min[0] = 0; }
    if (min[0] >= size[0]) { min[0] = size[0] - 2; }

    min[1] = _self->start_position[1] <= _self->end_position[1] ? _self->start_position[1] : _self->end_position[1];
    if (min[1] < 0) { min[1] = 0; }
    if (min[1] >= size[1]) { min[1] = size[1] - 2; }

    max[0] = _self->end_position[0] > _self->start_position[0] ? _self->end_position[0] : _self->start_position[0];
    if (max[0] < 0) { max[0] = 0; }
    if (max[0] >= size[0]) { max[0] = size[0] - 2; }

    max[1] = _self->end_position[1] > _self->start_position[1] ? _self->end_position[1] : _self->start_position[1];
    if (max[1] < 0) { max[1] = 0; }
    if (max[1] >= size[1]) { max[1] = size[1] - 2; }
    /* clang-format on */

    _self->selected_area[0] = min[0];
    _self->selected_area[1] = min[1];
    _self->selected_area[2] = max[0];
    _self->selected_area[3] = max[1];

    InvokeEvent(LxEventIdsEx::kSelectedAreaEvent, _self->selected_area);
}

//----------------------------------------------------------------------------
void lxInteractorStyle::RedrawRubberBand()
{
    auto ren_win = GetInteractor()->GetRenderWindow();
    int *size = ren_win->GetSize();

    /* clang-format off */
    auto pos = GetInteractor()->GetEventPosition();
    _self->end_position[0] = pos[0];
    _self->end_position[1] = pos[1];

    if (_self->end_position[0] > (size[0] - 1)) { _self->end_position[0] = size[0] - 1; }
    if (_self->end_position[0] < 0) { _self->end_position[0] = 0; }
    if (_self->end_position[1] > (size[1] - 1)) { _self->end_position[1] = size[1] - 1; }
    if (_self->end_position[1] < 0) { _self->end_position[1] = 0; }
    /* clang-format on */

    vtkNew<vtkUnsignedCharArray> tmp_pixel_array;
    tmp_pixel_array->DeepCopy(_self->pixel_array);

    unsigned char *pixels = tmp_pixel_array->GetPointer(0);

    /* clang-format off */
    int min[2], max[2];
    min[0] = _self->start_position[0] <= _self->end_position[0] ? _self->start_position[0] : _self->end_position[0];
    if (min[0] < 0) { min[0] = 0; }
    if (min[0] >= size[0]) { min[0] = size[0] - 1; }

    min[1] = _self->start_position[1] <= _self->end_position[1] ? _self->start_position[1] : _self->end_position[1];
    if (min[1] < 0) { min[1] = 0; }
    if (min[1] >= size[1]) { min[1] = size[1] - 1; }

    max[0] = _self->end_position[0] > _self->start_position[0] ? _self->end_position[0] : _self->start_position[0];
    if (max[0] < 0) {  max[0] = 0; }
    if (max[0] >= size[0]) { max[0] = size[0] - 1; }

    max[1] = _self->end_position[1] > _self->start_position[1] ? _self->end_position[1] : _self->start_position[1];
    if (max[1] < 0) { max[1] = 0; }
    if (max[1] >= size[1]) { max[1] = size[1] - 1;  }
    /* clang-format on */

    int i;
    for (i = min[0]; i <= max[0]; i++)
    {
        pixels[4 * (min[1] * size[0] + i)] = 255 ^ pixels[4 * (min[1] * size[0] + i)];
        pixels[4 * (min[1] * size[0] + i) + 1] = 255 ^ pixels[4 * (min[1] * size[0] + i) + 1];
        pixels[4 * (min[1] * size[0] + i) + 2] = 255 ^ pixels[4 * (min[1] * size[0] + i) + 2];
        pixels[4 * (max[1] * size[0] + i)] = 255 ^ pixels[4 * (max[1] * size[0] + i)];
        pixels[4 * (max[1] * size[0] + i) + 1] = 255 ^ pixels[4 * (max[1] * size[0] + i) + 1];
        pixels[4 * (max[1] * size[0] + i) + 2] = 255 ^ pixels[4 * (max[1] * size[0] + i) + 2];
    }
    for (i = min[1] + 1; i < max[1]; i++)
    {
        pixels[4 * (i * size[0] + min[0])] = 255 ^ pixels[4 * (i * size[0] + min[0])];
        pixels[4 * (i * size[0] + min[0]) + 1] = 255 ^ pixels[4 * (i * size[0] + min[0]) + 1];
        pixels[4 * (i * size[0] + min[0]) + 2] = 255 ^ pixels[4 * (i * size[0] + min[0]) + 2];
        pixels[4 * (i * size[0] + max[0])] = 255 ^ pixels[4 * (i * size[0] + max[0])];
        pixels[4 * (i * size[0] + max[0]) + 1] = 255 ^ pixels[4 * (i * size[0] + max[0]) + 1];
        pixels[4 * (i * size[0] + max[0]) + 2] = 255 ^ pixels[4 * (i * size[0] + max[0]) + 2];
    }

    ren_win->SetRGBACharPixelData(0, 0, size[0] - 1, size[1] - 1, pixels, 0);
    ren_win->Frame();
}

//----------------------------------------------------------------------------
bool lxInteractorStyle::IsOnActor()
{
    auto event_pos = GetInteractor()->GetEventPosition();

    _self->prop_picker->Pick(event_pos[0], event_pos[1], 0.0, GetDefaultRenderer());
    _self->last_actor = _self->prop_picker->GetActor();

    return _self->last_actor;
}

//----------------------------------------------------------------------------
void lxInteractorStyle::MoveActor()
{
    vtkRenderWindowInteractor *rwi = Interactor;

    if (_self->last_actor)
    {
        double *c_pos = _self->last_actor->GetCenter();
        double disp_obj_center[3], new_pick_point[4], old_pick_point[4];

        ComputeWorldToDisplay(c_pos[0], c_pos[1], c_pos[2], disp_obj_center);

        auto pos = GetInteractor()->GetEventPosition();
        ComputeDisplayToWorld(pos[0], pos[1], disp_obj_center[2], new_pick_point);

        pos = GetInteractor()->GetLastEventPosition();
        ComputeDisplayToWorld(pos[0], pos[1], disp_obj_center[2], old_pick_point);

        _self->motion_vector[0] = new_pick_point[0] - old_pick_point[0];
        _self->motion_vector[1] = new_pick_point[1] - old_pick_point[1];
        _self->motion_vector[2] = new_pick_point[2] - old_pick_point[2];

        InvokeEvent(LxEventIdsEx::kActorMoveDeltaEvent, _self->motion_vector);

        rwi->Render();
    }
}
