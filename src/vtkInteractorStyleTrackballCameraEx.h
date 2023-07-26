/** @file       vtkInteractorStyleTrackballCameraEx.h
 *  @brief
 *  @author     Mei Zhaorui
 *  @version    1.0
 *  @date       2023-07-26
 *  @copyright  Heli Co., Ltd. All rights reserved.
 */
#ifndef __TEST_VTK_VTKINTERACTORSTYLETRACKBALLCAMERAEX_H__
#define __TEST_VTK_VTKINTERACTORSTYLETRACKBALLCAMERAEX_H__

#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkSmartPointer.h>
#include <vtkCommand.h>

class vtkUnsignedCharArray;

enum LxEventIdsEx
{
    kSelectedAreaEvent = vtkCommand::UserEvent + 1,
    kActorMoveEvent,
    kSingleActorClickedEvent,
};

class vtkInteractorStyleTrackballCameraEx : public vtkInteractorStyleTrackballCamera
{
    vtkTypeMacro(vtkInteractorStyleTrackballCameraEx, vtkInteractorStyleTrackballCamera);

public:
    vtkInteractorStyleTrackballCameraEx();

    static vtkInteractorStyleTrackballCameraEx *New();

    void OnLeftButtonDown() override;
    void OnMouseMove() override;
    void OnLeftButtonUp() override;

private:
    void Pick();
    void RedrawRubberBand();

private:
    bool _move_actor;
    bool _select_actor;

    int _start_position[2]{};
    int _end_position[2]{};

    vtkActor *_last_actor{};
    double _motion_vector[3]{};
    int _selected_area[4]{};
    vtkSmartPointer<vtkUnsignedCharArray> _pixel_array;
};

#endif  // __TEST_VTK_VTKINTERACTORSTYLETRACKBALLCAMERAEX_H__
