/** @file       vtkInteractorStyleTrackballCameraEx.cpp
 *  @brief
 *  @author     Mei Zhaorui
 *  @version    1.0
 *  @date       2023-07-26
 *  @copyright  Heli Co., Ltd. All rights reserved.
 */
#include "vtkInteractorStyleTrackballCameraEx.h"

#include <vtkObjectFactory.h>
#include <vtkUnsignedCharArray.h>
#include <vtkPropPicker.h>
#include "vtkActor.h"
#include "vtkPointPicker.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkProp3DCollection.h"

vtkStandardNewMacro(vtkInteractorStyleTrackballCameraEx);

struct vtkInteractorStyleTrackballCameraEx::DataImpl
{
    bool click_moving_action;
    bool select_area_action;

    int start_position[2];
    int end_position[2];

    vtkActor* last_actor;
    double motion_vector[3];
    int selected_area[4];

    vtkNew<vtkUnsignedCharArray> pixel_array;
    vtkNew<vtkPropPicker> prop_picker;
};

vtkInteractorStyleTrackballCameraEx::vtkInteractorStyleTrackballCameraEx()
    : _self(new DataImpl{})
{
    _self->click_moving_action = false;
    _self->select_area_action = false;
    _self->last_actor = nullptr;

    memset(_self->start_position, '\0', sizeof(_self->start_position));
    memset(_self->end_position, '\0', sizeof(_self->end_position));
    memset(_self->motion_vector, '\0', sizeof(_self->motion_vector));
    memset(_self->selected_area, '\0', sizeof(_self->selected_area));

    _self->pixel_array->Initialize();
    _self->pixel_array->SetNumberOfComponents(4);
}

void vtkInteractorStyleTrackballCameraEx::OnLeftButtonDown()
{
    if (GetInteractor()->GetAltKey())
    {
        _self->select_area_action = true;

        vtkRenderWindow* ren_win = GetInteractor()->GetRenderWindow();

        _self->start_position[0] = GetInteractor()->GetEventPosition()[0];
        _self->start_position[1] = GetInteractor()->GetEventPosition()[1];
        _self->end_position[0] = _self->start_position[0];
        _self->end_position[1] = _self->start_position[1];

        int* size = ren_win->GetSize();
        _self->pixel_array->SetNumberOfTuples(size[0] * size[1]);

        ren_win->GetRGBACharPixelData(0, 0, size[0] - 1, size[1] - 1, 1, _self->pixel_array);

        InvokeEvent(LxEventIdsEx::kSelectedAreaStartEvent);
    }
    else
    {
        auto event_pos = GetInteractor()->GetEventPosition();

        _self->prop_picker->Pick(event_pos[0], event_pos[1], 0.0, GetDefaultRenderer());
        _self->last_actor = _self->prop_picker->GetActor();
        if (_self->last_actor)
        {
            _self->click_moving_action = true;
            InvokeEvent(LxEventIdsEx::kSingleActorClickedEvent, _self->last_actor);
        }
        else
        {
            Superclass::OnLeftButtonDown();  //
        }
    }
}

void vtkInteractorStyleTrackballCameraEx::OnMouseMove()
{
    if (_self->click_moving_action && _self->last_actor)
    {
        double* c_pos = _self->last_actor->GetCenter();
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

        GetInteractor()->GetRenderWindow()->Render();
    }
    else if (_self->select_area_action)
    {
        /* clang-format off */
        auto pos = GetInteractor()->GetEventPosition();
        _self->end_position[0] = pos[0];
        _self->end_position[1] = pos[1];

        int* size = GetInteractor()->GetRenderWindow()->GetSize();
        if (_self->end_position[0] > (size[0] - 1)) { _self->end_position[0] = size[0] - 1; }
        if (_self->end_position[0] < 0) { _self->end_position[0] = 0; }
        if (_self->end_position[1] > (size[1] - 1)) { _self->end_position[1] = size[1] - 1; }
        if (_self->end_position[1] < 0) { _self->end_position[1] = 0; }
        /* clang-format on */

        redrawRubberBand();
    }
    else
    {
        Superclass::OnMouseMove();  //
    }
}

void vtkInteractorStyleTrackballCameraEx::OnLeftButtonUp()
{
    if (_self->click_moving_action)
    {
        _self->click_moving_action = false;
        _self->last_actor = nullptr;
    }

    if (_self->select_area_action)
    {
        _self->select_area_action = false;
        pick();
    }

    Superclass::OnLeftButtonUp();
}

void vtkInteractorStyleTrackballCameraEx::redrawRubberBand()
{
    auto ren_win = GetInteractor()->GetRenderWindow();
    int* size = ren_win->GetSize();

    vtkNew<vtkUnsignedCharArray> tmp_pixel_array;
    tmp_pixel_array->DeepCopy(_self->pixel_array);

    unsigned char* pixels = tmp_pixel_array->GetPointer(0);

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

void vtkInteractorStyleTrackballCameraEx::pick()
{
    int* size = GetInteractor()->GetRenderWindow()->GetSize();

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

    InvokeEvent( LxEventIdsEx::kSelectedAreaEndEvent, _self->selected_area);
}

void vtkInteractorStyleTrackballCameraEx::OnRightButtonDown()
{
    /// ignore event
}

void vtkInteractorStyleTrackballCameraEx::OnRightButtonUp()
{
    InvokeEvent(LxEventIdsEx::kRightButtonUpEvent, GetInteractor());
}
