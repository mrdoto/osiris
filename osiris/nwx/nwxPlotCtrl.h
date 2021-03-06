/*
* ===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
*
*  FileName: nwxPlotCtrl.h
*  Author:   Douglas Hoffman
*
*   need to remove m_pTimer and inherit nwxTimerReceiver 5/27/16
*
*/
#ifndef __NWX_PLOT_CTRL_H__
#define __NWX_PLOT_CTRL_H__

#include <wx/dc.h>
#include <wx/stattext.h>
#include <wx/colour.h>
#include <wx/timer.h>
#include <wx/tooltip.h>
#include <wx/scrolbar.h>
#include <wx/panel.h>
#ifdef __WXMSW__
#pragma warning( push )
#pragma warning( disable : 4267 )
#endif
#include <wx/plotctrl/plotctrl.h>
#ifdef __WXMSW__
#pragma warning( pop )
#endif

#include "nwx/nwxPlotDrawerLabel.h"
#include "nwx/nwxPlotDrawerShade.h"

#define __TOOLTIP_TO_FRAME__ 0
//  tried to set the tooltip parent to the frame
//  but it created more problmes than it solved
//  set to 1 if attempted again in the future

class WXDLLIMPEXP_PLOTCTRL nwxPlotCtrl : public wxPlotCtrl
{
public:
  nwxPlotCtrl() : wxPlotCtrl(),
    m_Labels(this),
    m_XLabels(this),
    m_Xshade(this),
    m_fontLabel(8,wxFONTFAMILY_DEFAULT,
        wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL),
    m_PositionMouse(-1,-1),
    m_pTimer(NULL),
    m_nDisableToolTip(0),
    m_bClear(false),
    m_bStopTimer(false),
    m_nTimeHere(0),
    m_pToolTip(NULL),
    m_pToolText(NULL),
    m_pLastXLabel(NULL),
    m_pToolTipParent(NULL)
  { }
  nwxPlotCtrl( wxWindow *parent, wxWindowID win_id = wxID_ANY,
        const wxPoint &pos = wxDefaultPosition,
        const wxSize &size = wxDefaultSize,
        wxPlotCtrlAxis_Type flags = wxPLOTCTRL_DEFAULT,
        const wxString& name = wxT("wxPlotCtrl") ) :
    wxPlotCtrl( parent, win_id, pos, size, flags, name),
      m_Labels(this),
      m_XLabels(this),
      m_Xshade(this),
      m_fontLabel(8,wxFONTFAMILY_DEFAULT,
        wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL),
      m_PositionMouse(0,0),
      m_pTimer(NULL),
      m_nDisableToolTip(0),
      m_bClear(false),
      m_bStopTimer(false),
      m_nTimeHere(0),
      m_pToolTip(NULL),
      m_pLastXLabel(NULL),
      m_pToolTipParent(NULL)
      {
        _Init();
      }
  virtual ~nwxPlotCtrl();

  bool Create( wxWindow *parent, wxWindowID id = wxID_ANY,
                 const wxPoint &pos = wxDefaultPosition,
                 const wxSize &size = wxDefaultSize,
                 wxPlotCtrlAxis_Type flags = wxPLOTCTRL_DEFAULT,
                 const wxString& name = wxT("wxPlotCtrl") )
  {
    bool bRtn = wxPlotCtrl::Create(parent,id,pos,size,flags,name);
    if(bRtn)
    {
      _Init();
    }
    return bRtn;
  }
  int GetAreaMouseCursor()
  {
    return m_area_mouse_cursorid;
  }
  void AddLabel(const nwxPointLabel &x)
  {
    m_Labels.AddLabel(x);
  }
  void AddXLabel(const nwxPointLabel &x)
  {
    m_XLabels.AddLabel(x);
  }
  void AddXShade(const nwxPlotShade &x)
  {
    m_Xshade.AddArea(x);
  }
  void RemoveLabel(const nwxPointLabel &x)
  {
    m_Labels.RemoveLabel(x);
  }
  void RemoveXLabel(const nwxPointLabel &x)
  {
    m_XLabels.RemoveLabel(x);
  }
  void RemoveAllLabels()
  {
    m_Labels.Clear();
    m_XLabels.Clear();
    m_Xshade.Clear();
  }
  void SetLabelFont(const wxFont &x)
  {
    m_fontLabel = x;
  }
  const wxFont &GetLabelFont() const
  {
    return m_fontLabel;
  }
  bool IsTimerRunning()
  {
    return (m_pTimer != NULL) && m_pTimer->IsRunning();
  }
  void StartTimer()
  {
    m_bStopTimer = false;
    if(m_pTimer == NULL)
    {
      m_pTimer = new wxTimer(this,wxID_ANY);
    }
    if(!m_pTimer->IsRunning())
    {
      m_pTimer->Start(TIMER_ELAPSE, false);
    }
  }
  void StopTimer()
  {
    m_bStopTimer = true;
  }
  void ShowScrollbars(bool b);

  bool AreScrollbarsShown()
  {
    bool b =
      (m_xAxisScrollbar != NULL) &&
      (m_xAxisScrollbar->IsShown());
    return b;
  }

  virtual void OnClickXLabel(const nwxPointLabel &x, const wxPoint &pt);
  virtual void OnClickLabel(const nwxPointLabel &x, const wxPoint &pt);
  void SetupToolTip();
  void ClearToolTip()
  {
    if( (m_pToolTip != NULL) && m_pToolTip->IsShown() )
    {
      m_bClear = true; // timer will take it from here
      StartTimer();
    }
  }
  void DisableToolTip()
  {
    m_nDisableToolTip++;
    if(m_nDisableToolTip == 1) { ClearToolTip(); }
  }
  void EnableToolTip()
  {
    m_nDisableToolTip--;
    if(!m_nDisableToolTip) { StartTimer(); }
  }

  void OnViewChanged(wxPlotCtrlEvent &e);
  void nwxOnMouse(wxMouseEvent &event);
  void nwxOnTimer(wxTimerEvent &e);

  virtual void DrawAreaWindow( wxDC *dc, const wxRect& rect );
  virtual void DrawPlotCtrl( wxDC *dc );
  virtual void ProcessAreaEVT_MOUSE_EVENTS( wxMouseEvent &event );
  void DrawEntirePlot( wxDC *dc, const wxRect &boundingRect, double dpi = 72 );
  //wxString GetToolTipText(const wxPoint &pos);

#if __TOOLTIP_TO_FRAME__
  wxWindow *GetToolTipParent()
  {
    if(m_pToolTipParent == NULL)
    {
      m_pToolTipParent = this;
      //m_pToolTipParent = _FindFrameParent();
    }
    return m_pToolTipParent;
  }
  void SetToolTipParent(wxWindow *p)
  {
    if(m_pToolTip == NULL)
    {
      m_pToolTipParent = p;
    }
  }


  // DrawEntirePlot is a wrapper around DrawWholePlot
private:
  void _OffsetToolTipPosition(wxPoint *ppt);
#endif
  wxWindow *_FindFrameParent();

private:
  static const int TIMER_ELAPSE;
  static const unsigned int TIMER_COUNT;
  void _SetupCursor(const nwxPointLabel *pLabel);
  void _ClearToolTip();
  bool _OnMouseDown(wxMouseEvent &e); // called from nwxOnMouse
  bool _OnMouseUp(wxMouseEvent &e);   //
  bool _OnMouseDownXLabel(wxMouseEvent &e); // called from nwxOnMouse
  bool _OnMouseUpXLabel(wxMouseEvent &e);   //
  void SendContextMenuEvent(wxMouseEvent &e);
  void ProcessMousePosition(const wxPoint &pt);
  void _ResetMousePosition()
  {
    m_PositionMouse.x = -1;
    m_PositionMouse.y = -1;
  }
  bool CursorInPlotArea()
  {
    bool bRtn = false;
    if( (m_PositionMouse.x >= 0) && (m_PositionMouse.y >= 0) )
    {
      int x,y;
      GetPlotArea()->GetSize(&x,&y);
      bRtn = (m_PositionMouse.x <= x) &&
        (m_PositionMouse.y <= y);
    }
    return bRtn;
  }
  void _Init();
#ifdef __WXDEBUG__
  void _LogMouseUp(const wxPoint &pt,const nwxPointLabel *pLabel);
#endif
  nwxPlotDrawerLabel m_Labels;
  nwxPlotDrawerXLabel m_XLabels;
  nwxPlotDrawerXShade m_Xshade;
  wxFont m_fontLabel;
  wxPoint m_PositionMouse;
  wxTimer *m_pTimer;
  int m_nDisableToolTip;
  bool m_bClear;
  bool m_bStopTimer;
  unsigned int m_nTimeHere;
  wxPanel *m_pToolTip;
  wxStaticText *m_pToolText;
  const nwxPointLabel *m_pLastXLabel;
  const nwxPointLabel *m_pLastLabel;
  wxWindow *m_pToolTipParent;
  wxCursor m_cursorDefault;
  wxCursor m_cursorAreaDefault;
  wxBitmap m_bmActiveBackup;
  wxBitmap m_bmInactiveBackup;
  wxBitmap m_bmClear;

  DECLARE_CLASS(nwxPlotCtrl)
  DECLARE_EVENT_TABLE()
};

class nwxPlotControlToolTipDisabler
{
public:
  nwxPlotControlToolTipDisabler(nwxPlotCtrl *p) : m_pPlotCtrl(p)
  {
    m_pPlotCtrl->DisableToolTip();
  }
  nwxPlotControlToolTipDisabler(const nwxPlotControlToolTipDisabler &x) : m_pPlotCtrl(x.m_pPlotCtrl)
  {
    m_pPlotCtrl->DisableToolTip();
  }
  virtual ~nwxPlotControlToolTipDisabler()
  {
    m_pPlotCtrl->EnableToolTip();
  }
private:
  mutable nwxPlotCtrl *m_pPlotCtrl;
};


#endif
