#include "pch.h"
#include "Widget.h"

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
