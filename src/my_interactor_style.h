/** @file       my_interactor_style.h
 *  @brief
 *  @author     Mei Zhaorui
 *  @version    1.0
 *  @date       2023-07-25
 *  @copyright  Heli Co., Ltd. All rights reserved.
 */
#ifndef __TEST_VTK_MY_INTERACTOR_STYLE_H__
#define __TEST_VTK_MY_INTERACTOR_STYLE_H__

#include "selected_actor_mgr.h"

#include <vtkSmartPointer.h>

class vtkInteractorStyle;
class vtkUnsignedCharArray;
class vtkRenderWindowInteractor;

namespace lingxi::vtk
{

class MyInteractorStyle final
{
public:
    MyInteractorStyle();

    void OnLeftButtonDown();
    void OnMouseMove();
    void OnLeftButtonUp();

    void RemoveSelected();

    vtkRenderWindowInteractor *GetInteractor() { return _interactor; }
    void SetInteractor(vtkRenderWindowInteractor *interactor);

    bool IsSelectedActor(vtkActor *);
    void RemoveSelected(vtkActor *);

protected:
    void Pick();
    void RedrawRubberBand();

private:
    vtkRenderWindowInteractor *_interactor;
    vtkInteractorStyle *_interactor_style;
    bool _move_actor;
    bool _select_actor;

    int _start_position[2];
    int _end_position[2];

    vtkSmartPointer<vtkUnsignedCharArray> _pixel_array;
    lingxi::vtk::SelectedActorMgr _selected_actors;
};

}  // namespace lingxi::vtk

#endif  // __TEST_VTK_MY_INTERACTOR_STYLE_H__
