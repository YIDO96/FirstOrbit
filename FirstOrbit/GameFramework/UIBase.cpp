#include "pch.h"
#include "UIBase.h"

#include "GameFramework/Texture.h"

void UIBase::RecalculateFinalPos()
{
	_finalX = _x - (_width * _pivotRatioX);
	_finalY = _y - (_height * _pivotRatioY);
}

bool UIBase::IsHoverInUI(Vector2 mousePos)
{
	// Texture에서 최종 계산된 Position과 Size를 가져옵니다.
	// (클래스 내부 멤버 변수명에 맞게 GetPos(), GetSize() 등을 수정해 주세요)
	Vector2 pos = GetPos();   // 또는 texture->GetFinalPos()
	Vector2 size = GetSize();

	// Rect 범위 안에 마우스 좌표가 들어왔는지 검사
	bool isInsideX = (mousePos.x >= pos.x && mousePos.x <= pos.x + size.x);
	bool isInsideY = (mousePos.y >= pos.y && mousePos.y <= pos.y + size.y);

	return isInsideX && isInsideY;
}

void UIBase::SetPos(float x, float y)
{
	_x = _anchorX + x;
	_y = _anchorY + y;

	// Anchor 위치 + Local 위치 - (자신 크기 * Pivot 비율)
	RecalculateFinalPos();
}

void UIBase::SetSize(float w, float h)
{
	_width = w; _height = h;
	RecalculateFinalPos();
}

void UIBase::SetAnchor(EAnchor anchor)
{
	_anchor = anchor;

	switch (_anchor)
	{
	case EAnchor::LeftTop:		_anchorX = 0;				_anchorY = 0;				break;
	case EAnchor::Top:			_anchorX = GWinSizeX / 2;	_anchorY = 0;				break;
	case EAnchor::RightTop:		_anchorX = GWinSizeX;		_anchorY = 0;				break;
	case EAnchor::Left:			_anchorX = 0;				_anchorY = GWinSizeY / 2;	break;
	case EAnchor::Center:		_anchorX = GWinSizeX / 2;	_anchorY = GWinSizeY / 2;	break;
	case EAnchor::Right:		_anchorX = GWinSizeX;		_anchorY = GWinSizeY / 2;	break;
	case EAnchor::LeftBottom:	_anchorX = 0;				_anchorY = GWinSizeY;		break;
	case EAnchor::Bottom:		_anchorX = GWinSizeX / 2;	_anchorY = GWinSizeY;		break;
	case EAnchor::RightBottom:	_anchorX = GWinSizeX;		_anchorY = GWinSizeY;		break;
	default:																			break;
	}
}

void UIBase::SetParentAnchor(EParentAnchor parentanchor, UIBase* parent)
{
	_parentAnchor = parentanchor;

	switch (_parentAnchor)
	{
	case EParentAnchor::LeftTop:		_anchorX = parent->_finalX;							_anchorY = parent->_finalY;							break;
	case EParentAnchor::Top:			_anchorX = parent->_finalX + parent->_width / 2;	_anchorY = parent->_finalY;							break;
	case EParentAnchor::RightTop:		_anchorX = parent->_finalX + parent->_width;		_anchorY = parent->_finalY;							break;
	case EParentAnchor::Left:			_anchorX = parent->_finalX;							_anchorY = parent->_finalY + parent->_height / 2;	break;
	case EParentAnchor::Center:			_anchorX = parent->_finalX + parent->_width / 2;	_anchorY = parent->_finalY + parent->_height / 2;	break;
	case EParentAnchor::Right:			_anchorX = parent->_finalX + parent->_width;		_anchorY = parent->_finalY + parent->_height / 2;	break;
	case EParentAnchor::LeftBottom:		_anchorX = parent->_finalX;							_anchorY = parent->_finalY + parent->_height;		break;
	case EParentAnchor::Bottom:			_anchorX = parent->_finalX + parent->_width / 2;	_anchorY = parent->_finalY + parent->_height;		break;
	case EParentAnchor::RightBottom:	_anchorX = parent->_finalX + parent->_width;		_anchorY = parent->_finalY + parent->_height;		break;
	default:																																	break;
	}
}

void UIBase::SetPivot(EPivot pivot)
{
	_pivot = pivot;

	switch (_pivot)
	{
	case EPivot::LeftTop:     _pivotRatioX = 0.0f; _pivotRatioY = 0.0f; break;
	case EPivot::Top:         _pivotRatioX = 0.5f; _pivotRatioY = 0.0f; break;
	case EPivot::RightTop:    _pivotRatioX = 1.0f; _pivotRatioY = 0.0f; break;
	case EPivot::Left:        _pivotRatioX = 0.0f; _pivotRatioY = 0.5f; break;
	case EPivot::Center:      _pivotRatioX = 0.5f; _pivotRatioY = 0.5f; break;
	case EPivot::Right:       _pivotRatioX = 1.0f; _pivotRatioY = 0.5f; break;
	case EPivot::LeftBottom:  _pivotRatioX = 0.0f; _pivotRatioY = 1.0f; break;
	case EPivot::Bottom:      _pivotRatioX = 0.5f; _pivotRatioY = 1.0f; break;
	case EPivot::RightBottom: _pivotRatioX = 1.0f; _pivotRatioY = 1.0f; break;
	}
}