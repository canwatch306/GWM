#include "stdafx.h"
#include "CLedState.h"

IMPLEMENT_DYNAMIC(CLedState, CStatic)

CLedState::CLedState()
	: m_led_on(false)
{

}

CLedState::~CLedState()
{
}

BEGIN_MESSAGE_MAP(CLedState, CStatic)
	ON_WM_PAINT()
	ON_WM_ENABLE()
END_MESSAGE_MAP()


void CLedState::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: �ڴ˴������Ϣ����������
	CRect rectClient;
	GetClientRect(rectClient);

	COLORREF clr = m_led_on ? RGB(255, 0, 0) : RGB(0, 0, 0);
	if (!IsWindowEnabled())
		clr = RGB(192, 192, 192);

	CBrush brush;
	brush.CreateSolidBrush(clr);
	dc.FillRect(&rectClient, &brush);
}


void CLedState::OnEnable(BOOL bEnable)
{
	CStatic::OnEnable(bEnable);

	Invalidate();
}
