/** @file       my_interactor_style.cpp
 *  @brief
 *  @author     Mei Zhaorui
 *  @version    1.0
 *  @date       2023-07-25
 *  @copyright  Heli Co., Ltd. All rights reserved.
 */
#include "my_interactor_style.h"

namespace lingxi::vtk
{

vtkStandardNewMacro(MyInteractorStyle);

MyInteractorStyle::MyInteractorStyle()
{
    _move_actor = false;
    _start_position[0] = _start_position[1] = 0;
    _end_position[0] = _end_position[1] = 0;
    _select_actor = false;

    _pixel_array->Initialize();
    _pixel_array->SetNumberOfComponents(4);
}

void MyInteractorStyle::OnLeftButtonDown()
{
    // std::cout << "Pressed left mouse button." << std::endl;

    if (GetInteractor()->GetAltKey())
    {
        // std::cout << "alt key pressed" << std::endl;

        emit statusRenderer(false);

        _select_actor = true;

        vtkRenderWindow* ren_win = Interactor->GetRenderWindow();

        _start_position[0] = Interactor->GetEventPosition()[0];
        _start_position[1] = Interactor->GetEventPosition()[1];
        _end_position[0] = _start_position[0];
        _end_position[1] = _start_position[1];

        int* size = ren_win->GetSize();
        _pixel_array->SetNumberOfTuples(size[0] * size[1]);

        ren_win->GetRGBACharPixelData(0, 0, size[0] - 1, size[1] - 1, 1, _pixel_array);
    }
    else
    {
        auto event_pos = GetInteractor()->GetEventPosition();
        // 注册拾取点函数
        auto picker = vtkSmartPointer<vtkPropPicker>::New();
        picker->Pick(event_pos[0], event_pos[1], 0.0, GetDefaultRenderer());

        auto actor = picker->GetActor();
        if (actor)
        {
            _move_actor = true;
            if (!_selected_actors.Contain(actor))
            {
                _selected_actors.Reset();
                _selected_actors.Clear();
                _selected_actors.AddActor(actor);
            }
        }
        else
        {
            //
            Superclass::OnLeftButtonDown();
        }
    }
}

void MyInteractorStyle::OnMouseMove()
{
    if (_move_actor)
    {
        double* obj_center = _selected_actors.GetCenter();

        double disp_obj_center[3], new_pick_point[4], old_pick_point[4], motion_vector[3];
        ComputeWorldToDisplay(obj_center[0], obj_center[1], obj_center[2], disp_obj_center);

        auto pos = GetInteractor()->GetEventPosition();
        ComputeDisplayToWorld(pos[0], pos[1], disp_obj_center[2], new_pick_point);

        pos = GetInteractor()->GetLastEventPosition();
        ComputeDisplayToWorld(pos[0], pos[1], disp_obj_center[2], old_pick_point);

        motion_vector[0] = new_pick_point[0] - old_pick_point[0];
        motion_vector[1] = new_pick_point[1] - old_pick_point[1];
        motion_vector[2] = new_pick_point[2] - old_pick_point[2];

        _selected_actors.AddPosition(motion_vector[0], motion_vector[1], motion_vector[2]);
    }
    else if (_select_actor)
    {
        _end_position[0] = Interactor->GetEventPosition()[0];
        _end_position[1] = Interactor->GetEventPosition()[1];

        int* size = Interactor->GetRenderWindow()->GetSize();
        if (_end_position[0] > (size[0] - 1)) { _end_position[0] = size[0] - 1; }
        if (_end_position[0] < 0) { _end_position[0] = 0; }
        if (_end_position[1] > (size[1] - 1)) { _end_position[1] = size[1] - 1; }
        if (_end_position[1] < 0) { _end_position[1] = 0; }

        RedrawRubberBand();
    }
    else { Superclass::OnMouseMove(); }
}

void MyInteractorStyle::OnLeftButtonUp()
{
    if (_move_actor) { _move_actor = false; }

    if (_select_actor)
    {
        _select_actor = false;
        emit statusRenderer(true);
        Pick();
    }

    Superclass::OnLeftButtonUp();
}

void MyInteractorStyle::RemoveSelected()
{
    _selected_actors.RemoveFrom(GetDefaultRenderer());
}

void MyInteractorStyle::RedrawRubberBand()
{
    // update the rubber band on the screen
    int* size = Interactor->GetRenderWindow()->GetSize();

    vtkNew<vtkUnsignedCharArray> tmp_pixel_array;
    tmp_pixel_array->DeepCopy(_pixel_array);

    unsigned char* pixels = tmp_pixel_array->GetPointer(0);

    int min[2], max[2];

    min[0] = _start_position[0] <= _end_position[0] ? _start_position[0] : _end_position[0];
    if (min[0] < 0) { min[0] = 0; }
    if (min[0] >= size[0]) { min[0] = size[0] - 1; }

    min[1] = _start_position[1] <= _end_position[1] ? _start_position[1] : _end_position[1];
    if (min[1] < 0) { min[1] = 0; }
    if (min[1] >= size[1]) { min[1] = size[1] - 1; }

    max[0] = _end_position[0] > _start_position[0] ? _end_position[0] : _start_position[0];
    if (max[0] < 0) { max[0] = 0; }
    if (max[0] >= size[0]) { max[0] = size[0] - 1; }

    max[1] = _end_position[1] > _start_position[1] ? _end_position[1] : _start_position[1];
    if (max[1] < 0) { max[1] = 0; }
    if (max[1] >= size[1]) { max[1] = size[1] - 1; }

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

    Interactor->GetRenderWindow()->SetRGBACharPixelData(0, 0, size[0] - 1, size[1] - 1, pixels, 0);
    Interactor->GetRenderWindow()->Frame();
}

void MyInteractorStyle::Pick()
{
    int* size = Interactor->GetRenderWindow()->GetSize();
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

    vtkNew<vtkAreaPicker> picker;
    picker->AreaPick(min[0], min[1], max[0], max[1], GetDefaultRenderer());

    vtkProp3DCollection* props = picker->GetProp3Ds();
    props->InitTraversal();

    _selected_actors.Reset();
    _selected_actors.Clear();
    for (vtkIdType i = 0; i < props->GetNumberOfItems(); i++)
    {
        vtkActor* actor = vtkActor::SafeDownCast(props->GetNextProp3D());
        if (!actor) continue;

        _selected_actors.AddActor(actor);
    }
}

}  // namespace lingxi::vtk
