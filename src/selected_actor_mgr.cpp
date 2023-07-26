/** @file       selected_actor_mgr.cpp
 *  @brief
 *  @author     Mei Zhaorui
 *  @version    1.0
 *  @date       2023-07-25
 *  @copyright  Heli Co., Ltd. All rights reserved.
 */
#include "selected_actor_mgr.h"

#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>

#include <algorithm>

namespace lingxi::vtk
{

SelectedActorMgr::~SelectedActorMgr()
{
    Clear();
}

void SelectedActorMgr::AddActor(vtkActor* actor)
{
    auto property = vtkProperty::New();

    property->DeepCopy(actor->GetProperty());

    _selected_actors.insert({actor, property});

    actor->GetProperty()->SetColor(1.0, 0.0, 0.0);
    actor->GetProperty()->SetDiffuse(1.0);
    actor->GetProperty()->SetSpecular(0.0);
}

void SelectedActorMgr::RemoveActor(vtkActor* actor)
{
    for (auto it = _selected_actors.cbegin(); it != _selected_actors.cend(); ++it)
    {
        auto [item, property] = *it;

        if (item == actor)
        {
            actor->GetProperty()->DeepCopy(property);
            _selected_actors.erase(it);
            break;
        }
    }
}

void SelectedActorMgr::Reset()
{
    for (auto [actor, property] : _selected_actors) { actor->GetProperty()->DeepCopy(property); }
}

void SelectedActorMgr::Clear()
{
    for (auto [actor, property] : _selected_actors) { property->Delete(); }
    _selected_actors.clear();
}

void SelectedActorMgr::AddPosition(double x, double y, double z)
{
    for (auto [actor, property] : _selected_actors) { actor->AddPosition(x, y, z); }
}

double* SelectedActorMgr::GetCenter()
{
    return _selected_actors.empty() ? nullptr : _selected_actors.begin()->first->GetCenter();
}

bool SelectedActorMgr::Contain(vtkActor* actor)
{
    return std::find_if(_selected_actors.cbegin(),
               _selected_actors.cend(),
               [actor](const auto& item) { return item.first == actor; }) != _selected_actors.cend();
}

void SelectedActorMgr::RemoveFrom(vtkRenderer* renderer)
{
    for (auto [actor, property] : _selected_actors) { renderer->RemoveActor(actor); }
    Clear();
}

}  // namespace lingxi::vtk
