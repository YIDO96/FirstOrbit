#include "pch.h"
#include "UIBase.h"

#include "Core/GameInstance.h"
#include "GameFramework/Texture.h"

bool UIBase::s_activeSpaceIsPanel = false;

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

	// mousePos는 윈도우 픽셀 좌표, UI는 논리 해상도 기준이라 비율 보정 필요.
	// 패널 소속이면 패널 뷰포트 오프셋을 뺀 뒤 패널 자체 비율로, 아니면 게임 뷰포트 비율로 보정한다.
	Vector2 scaledMouse;
	if (_usePanelSpace)
	{
		RECT panelRect = GAME.GetPanelViewportRect();
		Vector2 local = mousePos - Vector2((float)panelRect.left, (float)panelRect.top);
		Vector2 ratio = GAME.GetPanelRectRatio();   // x/y 스케일이 다를 수 있어서 각각 나눔
		scaledMouse = Vector2(local.x / ratio.x, local.y / ratio.y);
	}
	else
	{
		scaledMouse = mousePos / GAME.GetRectRatio();
	}

	// Rect 범위 안에 마우스 좌표가 들어왔는지 검사
	bool isInsideX = (scaledMouse.x >= pos.x && scaledMouse.x <= pos.x + size.x);
	bool isInsideY = (scaledMouse.y >= pos.y && scaledMouse.y <= pos.y + size.y);

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
	_usePanelSpace = s_activeSpaceIsPanel;

	// 패널 소속이면 GWinSizeX(게임 뷰포트 폭) 대신 GImGuiPanelWidth(패널 폭)를 기준으로 앵커를 잡는다.
	float refW = _usePanelSpace ? (float)GImGuiPanelWidth : (float)GWinSizeX;
	float refH = (float)GWinSizeY;   // 패널도 세로는 게임 버퍼와 같은 GWinSizeY를 쓴다

	switch (_anchor)
	{
	case EAnchor::LeftTop:		_anchorX = 0;			_anchorY = 0;			break;
	case EAnchor::Top:			_anchorX = refW / 2;	_anchorY = 0;			break;
	case EAnchor::RightTop:		_anchorX = refW;		_anchorY = 0;			break;
	case EAnchor::Left:			_anchorX = 0;			_anchorY = refH / 2;	break;
	case EAnchor::Center:		_anchorX = refW / 2;	_anchorY = refH / 2;	break;
	case EAnchor::Right:		_anchorX = refW;		_anchorY = refH / 2;	break;
	case EAnchor::LeftBottom:	_anchorX = 0;			_anchorY = refH;		break;
	case EAnchor::Bottom:		_anchorX = refW / 2;	_anchorY = refH;		break;
	case EAnchor::RightBottom:	_anchorX = refW;		_anchorY = refH;		break;
	default:																	break;
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