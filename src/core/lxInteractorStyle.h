/** @file       lxInteractorStyle.h
 *  @brief
 *  @author     Mei Zhaorui
 *  @version    1.0
 *  @date       2023-07-28
 *  @copyright  Heli Co., Ltd. All rights reserved.
 */
#ifndef __TEST_VTK_LXINTERACTORSTYLE_H__
#define __TEST_VTK_LXINTERACTORSTYLE_H__

#include <vtkInteractorStyle.h>
#include <vtkCommand.h>

struct LxEventIdsEx
{
    enum
    {
        kSelectedAreaEvent = vtkCommand::UserEvent + 1,
        kActorMoveDeltaEvent,
        kSingleActorClickedEvent,
        kRightButtonUpEvent,
    };
};

class lxInteractorStyle final : public vtkInteractorStyle
{
    vtkTypeMacro(lxInteractorStyle, vtkInteractorStyle);

public:
    static lxInteractorStyle *New();
    void PrintSelf(ostream &os, vtkIndent indent) final;

    void OnMouseMove() final;
    void OnLeftButtonDown() final;
    void OnLeftButtonUp() final;
    void OnRightButtonDown() final;
    void OnRightButtonUp() final;
    void OnMouseWheelForward() final;
    void OnMouseWheelBackward() final;

    void OnChar() final;
    void OnTimer() final;

    void Rotate() final;
    void Spin() final;
    void Pan() final;
    void Dolly() final;

    void StartMoveActor();
    void EndMoveActor();
    void MoveActor();

    void StartRubberBandPick();
    void EndRubberBandPick();
    void RedrawRubberBand();

protected:
    lxInteractorStyle();
    ~lxInteractorStyle() final;

    void dolly(double factor);
    bool IsOnActor();

private:
    lxInteractorStyle(const lxInteractorStyle &) = delete;
    void operator=(const lxInteractorStyle &) = delete;

private:
    struct DataImpl;
    std::unique_ptr<DataImpl> _self;
};

#endif  // __TEST_VTK_LXINTERACTORSTYLE_H__
