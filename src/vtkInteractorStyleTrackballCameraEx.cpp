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
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkProp3DCollection.h"

vtkStandardNewMacro(vtkInteractorStyleTrackballCameraEx);

vtkInteractorStyleTrackballCameraEx::vtkInteractorStyleTrackballCameraEx()
    : _move_actor(false)
    , _select_actor(false)
    , _pixel_array(vtkSmartPointer<vtkUnsignedCharArray>::New())
{
    _pixel_array->Initialize();
    _pixel_array->SetNumberOfComponents(4);
}

void vtkInteractorStyleTrackballCameraEx::OnLeftButtonDown()
{
    if (GetInteractor()->GetAltKey())
    {
        _select_actor = true;

        vtkRenderWindow* ren_win = GetInteractor()->GetRenderWindow();

        _start_position[0] = GetInteractor()->GetEventPosition()[0];
        _start_position[1] = GetInteractor()->GetEventPosition()[1];
        _end_position[0] = _start_position[0];
        _end_position[1] = _start_position[1];

        int* size = ren_win->GetSize();
        _pixel_array->SetNumberOfTuples(size[0] * size[1]);

        ren_win->GetRGBACharPixelData(0, 0, size[0] - 1, size[1] - 1, 1, _pixel_array);
    }
    else
    {
        auto event_pos = GetInteractor()->GetEventPosition();
        auto picker = vtkSmartPointer<vtkPropPicker>::New();
        picker->Pick(event_pos[0], event_pos[1], 0.0, GetInteractor()->GetInteractorStyle()->GetDefaultRenderer());

        _last_actor = picker->GetActor();
        if (_last_actor)
        {
            _move_actor = true;
            Superclass::InvokeEvent(LxEventIdsEx::kSingleActorClickedEvent, GetInteractor());
        }
        else { Superclass::OnLeftButtonDown(); }
    }
}

void vtkInteractorStyleTrackballCameraEx::OnMouseMove()
{
    if (_move_actor && _last_actor)
    {
        double* c_pos = _last_actor->GetCenter();

        auto renderer = GetInteractor()->GetInteractorStyle()->GetDefaultRenderer();

        double disp_obj_center[3], new_pick_point[4], old_pick_point[4];
        vtkInteractorObserver::ComputeWorldToDisplay(renderer, c_pos[0], c_pos[1], c_pos[2], disp_obj_center);

        auto pos = GetInteractor()->GetEventPosition();
        vtkInteractorObserver::ComputeDisplayToWorld(renderer, pos[0], pos[1], disp_obj_center[2], new_pick_point);

        pos = GetInteractor()->GetLastEventPosition();
        vtkInteractorObserver::ComputeDisplayToWorld(renderer, pos[0], pos[1], disp_obj_center[2], old_pick_point);

        _motion_vector[0] = new_pick_point[0] - old_pick_point[0];
        _motion_vector[1] = new_pick_point[1] - old_pick_point[1];
        _motion_vector[2] = new_pick_point[2] - old_pick_point[2];

        InvokeEvent(LxEventIdsEx::kActorMoveEvent, _motion_vector);

        GetInteractor()->GetRenderWindow()->Render();
    }
    else if (_select_actor)
    {
        /* clang-format off */
        _end_position[0] = GetInteractor()->GetEventPosition()[0];
        _end_position[1] = GetInteractor()->GetEventPosition()[1];

        int* size = GetInteractor()->GetRenderWindow()->GetSize();
        if (_end_position[0] > (size[0] - 1)) { _end_position[0] = size[0] - 1; }
        if (_end_position[0] < 0) { _end_position[0] = 0; }
        if (_end_position[1] > (size[1] - 1)) { _end_position[1] = size[1] - 1; }
        if (_end_position[1] < 0) { _end_position[1] = 0; }
        /* clang-format on */

        RedrawRubberBand();
    }
    else { Superclass ::OnMouseMove(); }
}

void vtkInteractorStyleTrackballCameraEx::OnLeftButtonUp()
{
    if (_move_actor) { _move_actor = false; }

    if (_select_actor)
    {
        _select_actor = false;
        Pick();
    }

    Superclass::OnLeftButtonUp();
}

void vtkInteractorStyleTrackballCameraEx::RedrawRubberBand()
{
    // update the rubber band on the screen
    int* size = GetInteractor()->GetRenderWindow()->GetSize();

    vtkNew<vtkUnsignedCharArray> tmp_pixel_array;
    tmp_pixel_array->DeepCopy(_pixel_array);

    unsigned char* pixels = tmp_pixel_array->GetPointer(0);

    /* clang-format off */
    int min[2], max[2];
    min[0] = _start_position[0] <= _end_position[0] ? _start_position[0] : _end_position[0];
    if (min[0] < 0) { min[0] = 0; }
    if (min[0] >= size[0]) { min[0] = size[0] - 1; }

    min[1] = _start_position[1] <= _end_position[1] ? _start_position[1] : _end_position[1];
    if (min[1] < 0) { min[1] = 0; }
    if (min[1] >= size[1]) { min[1] = size[1] - 1; }

    max[0] = _end_position[0] > _start_position[0] ? _end_position[0] : _start_position[0];
    if (max[0] < 0) {  max[0] = 0; }
    if (max[0] >= size[0]) { max[0] = size[0] - 1; }

    max[1] = _end_position[1] > _start_position[1] ? _end_position[1] : _start_position[1];
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

    GetInteractor()->GetRenderWindow()->SetRGBACharPixelData(0, 0, size[0] - 1, size[1] - 1, pixels, 0);
    GetInteractor()->GetRenderWindow()->Frame();
}

void vtkInteractorStyleTrackballCameraEx::Pick()
{
    int* size = GetInteractor()->GetRenderWindow()->GetSize();

    /* clang-format off */
    int min[2], max[2];
    min[0] = _start_position[0] <= _end_position[0] ? _start_position[0] : _end_position[0];
    if (min[0] < 0) { min[0] = 0; }
    if (min[0] >= size[0]) { min[0] = size[0] - 2; }

    min[1] = _start_position[1] <= _end_position[1] ? _start_position[1] : _end_position[1];
    if (min[1] < 0) { min[1] = 0; }
    if (min[1] >= size[1]) { min[1] = size[1] - 2; }

    max[0] = _end_position[0] > _start_position[0] ? _end_position[0] : _start_position[0];
    if (max[0] < 0) { max[0] = 0; }
    if (max[0] >= size[0]) { max[0] = size[0] - 2; }

    max[1] = _end_position[1] > _start_position[1] ? _end_position[1] : _start_position[1];
    if (max[1] < 0) { max[1] = 0; }
    if (max[1] >= size[1]) { max[1] = size[1] - 2; }
    /* clang-format on */

    _selected_area[0] = min[0];
    _selected_area[1] = min[1];
    _selected_area[2] = max[0];
    _selected_area[3] = max[1];

    InvokeEvent(LxEventIdsEx::kSelectedAreaEvent, _selected_area);
}