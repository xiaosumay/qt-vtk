/** @file       vtkInteractorStyleTrackballCameraEx.h
 *  @brief
 *  @author     Mei Zhaorui
 *  @version    1.0
 *  @date       2023-07-26
 *  @copyright  Heli Co., Ltd. All rights reserved.
 */
#ifndef __TEST_VTK_VTKINTERACTORSTYLETRACKBALLCAMERAEX_H__
#define __TEST_VTK_VTKINTERACTORSTYLETRACKBALLCAMERAEX_H__

#include <memory>

#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkCommand.h>

struct LxEventIdsEx
{
    enum
    {
        kSelectedAreaStartEvent = vtkCommand::UserEvent + 1,
        kSelectedAreaEndEvent,
        kActorMoveDeltaEvent,
        kSingleActorClickedEvent,
        kRightButtonUpEvent,
    };
};

class vtkInteractorStyleTrackballCameraEx final : public vtkInteractorStyleTrackballCamera
{
    vtkTypeMacro(vtkInteractorStyleTrackballCameraEx, vtkInteractorStyleTrackballCamera);

public:
    vtkInteractorStyleTrackballCameraEx();

    static vtkInteractorStyleTrackballCameraEx *New();

    void OnLeftButtonDown() final;
    void OnLeftButtonUp() final;

    void OnMouseMove() final;

    void OnRightButtonDown() final;
    void OnRightButtonUp() final;

private:
    void pick();
    void redrawRubberBand();

private:
    struct DataImpl;
    std::unique_ptr<DataImpl> _self;
};

#endif  // __TEST_VTK_VTKINTERACTORSTYLETRACKBALLCAMERAEX_H__
