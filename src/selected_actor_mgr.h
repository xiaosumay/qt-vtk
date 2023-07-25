/** @file       selected_actor_mgr.h
 *  @brief
 *  @author     Mei Zhaorui
 *  @version    1.0
 *  @date       2023-07-25
 *  @copyright  Heli Co., Ltd. All rights reserved.
 */
#ifndef __TEST_VTK_SELECTED_ACTOR_MGR_H__
#define __TEST_VTK_SELECTED_ACTOR_MGR_H__

#include <unordered_map>

#include "vtkActor.h"
#include "vtkAssembly.h"
#include "vtkNew.h"

namespace lingxi::vtk
{

class SelectedActorMgr
{
public:
    SelectedActorMgr() = default;
    ~SelectedActorMgr();

    void AddActor(vtkActor *actor);
    void Reset();
    void Clear();

    void AddPosition(double x, double y, double z);

    double *GetCenter();

    bool Contain(vtkActor *);

    void RemoveFrom(vtkRenderer *);

private:
    std::unordered_map<vtkActor *, vtkProperty *> _selected_actors;
};

}  // namespace lingxi::vtk


#endif  // __TEST_VTK_SELECTED_ACTOR_MGR_H__
