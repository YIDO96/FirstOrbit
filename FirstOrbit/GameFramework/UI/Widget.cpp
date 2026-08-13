#include "pch.h"
#include "Widget.h"

#include "Core/WorldManager.h"

Widget::~Widget()
{
    for (auto* child : _children)
        delete child;
    _children.clear();
}

void Widget::Init()
{
    //for (auto* child : _children)
    //    child->Init();

    _ownerWorld = WORLD.GetWorld();
}

void Widget::Update(float deltaTime)
{
    if (!_isActive) return;






    for (auto* child : _children)
        child->Update(deltaTime);
}

void Widget::Render(HDC hdc)
{
    if (!_isActive) return;
    for (auto* child : _children)
        child->Render(hdc);
}

bool Widget::IsMouseOverUI(Vector2 mousePos) const
{
    for (UIBase* child : _children)
    {
        if (child->IsHoverInUI(mousePos))
            return true;
    }
    return false;
}
