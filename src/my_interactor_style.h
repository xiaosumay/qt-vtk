/** @file       my_interactor_style.h
 *  @brief
 *  @author     Mei Zhaorui
 *  @version    1.0
 *  @date       2023-07-25
 *  @copyright  Heli Co., Ltd. All rights reserved.
 */
#ifndef __TEST_VTK_MY_INTERACTOR_STYLE_H__
#define __TEST_VTK_MY_INTERACTOR_STYLE_H__

#include <QObject>

#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkRenderWindow.h>
#include <vtkPropPicker.h>
#include <vtkTransform.h>
#include <vtkProperty.h>
#include <vtkAreaPicker.h>
#include <vtkObjectFactory.h>
#include <vtkProp3DCollection.h>

#include "selected_actor_mgr.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkUnsignedCharArray.h"

namespace lingxi::vtk
{

class MyInteractorStyle final : public QObject, public vtkInteractorStyleTrackballCamera
{
    Q_OBJECT
    vtkTypeMacro(MyInteractorStyle, vtkInteractorStyleTrackballCamera);

signals:
    void statusRenderer(bool);

public:
    static MyInteractorStyle* New();
    MyInteractorStyle();

    void OnLeftButtonDown() override;
    void OnMouseMove() override;
    void OnLeftButtonUp() override;

    void RemoveSelected();
    
protected:
    void Pick();
    void RedrawRubberBand();

private:
    bool _move_actor;
    bool _select_actor;

    int _start_position[2];
    int _end_position[2];

    vtkNew<vtkUnsignedCharArray> _pixel_array;
    lingxi::vtk::SelectedActorMgr _selected_actors;
};

}  // namespace lingxi::vtk

#endif  // __TEST_VTK_MY_INTERACTOR_STYLE_H__
