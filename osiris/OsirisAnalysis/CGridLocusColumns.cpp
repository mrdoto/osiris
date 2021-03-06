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
*  FileName: CGridLocusColumns.cpp
*  Author:   Douglas Hoffman
*
*/
#include "mainApp.h"
#include "nwx/nwxGrid.h"
#include "CKitList.h"
#include "CKitColors.h"
#include "CLabSettings.h"
#include "CGridLocusColumns.h"
#include <wx/settings.h>

// bool to int
#define BINT(b) ((b) ? 1 : 0)

bool CGridLocusColumns::SetupKit(
  wxGrid *pGrid,
  const wxString &sKitName, 
  bool bDefault, bool bILS, 
  bool bAllowAmel,
  bool bSetError,
  int *pnILScolumn,
  unsigned int *pnILSchannel)
{
  vector<wxString> vs;
  if(bDefault)
  {
    const wxChar *psDefault = 
      bILS ? wxS("Default*") : wxS("Default");
    vs.push_back(psDefault);
  }
  bool bRtn = SetupKit(
    pGrid,sKitName,vs,bILS,bAllowAmel,bSetError,pnILScolumn,pnILSchannel);
  return bRtn;
}
bool CGridLocusColumns::SetupKit(
  wxGrid *pGrid,
  const wxString &sKitName, 
  const vector<wxString> &vs, 
  bool bILS, 
  bool bAllowAmel,
  bool bSetError,
  int *pnILScolumn,
  unsigned int *pnILSchannel)
{
  wxString sProblem;
  CPersistKitList *pKit = mainApp::GetKitList();
  const CLocusNameList *pLocus = 
    pKit->GetLocusNameList(sKitName);
  if(sKitName.IsEmpty())
  {
    sProblem = "Marker set name is not specified.";
  }
  else if(pLocus == NULL)
  {
    sProblem = "Marker set, ";
    sProblem.Append(sKitName);
    sProblem.Append(", is unknown");
  }
  else if(pLocus->size() < 1)
  {
    sProblem = "Marker set, ";
    sProblem.Append(sKitName);
    sProblem.Append(", does not have any loci");
  }
  if(!sProblem.IsEmpty())
  {
    if(bSetError)
    {
      nwxGrid::SetError(pGrid,sProblem);
    }
  }
  else
  {
    vector<unsigned int> vnChannels;
    vector<int> vnCounts;
    set<unsigned int> snChannelsUsed;
    wxFont fnBold = pGrid->GetDefaultCellFont();

    int nBeforeLoci = (int)vs.size();
    int nCols = (int)(pLocus->size()) + nBeforeLoci + BINT(bILS);
    int nRemove = 0;
    int nCol;
    int nCount = 1;
    unsigned int nChannel;
    unsigned int nLastChannel = 0;  // invalid channel name for default column
    unsigned int nILSchannel = 0;
    const CSingleKitColors *pKitColors;
    const CLocusNameChannel *plc;
    pKitColors = mainApp::GetKitColors()->GetKitColors(sKitName);

    pGrid->SetMargins(0, wxSystemSettings::GetMetric(wxSYS_HSCROLL_Y) + 4);
    fnBold.SetWeight(wxFONTWEIGHT_BOLD);
    nwxGrid::SetColCount(pGrid,nCols);
    for(nCol = 0; nCol < nCols; nCol++)
    {
      pGrid->SetCellSize(0,nCol,1,1);
      //pGrid->SetReadOnly(0,0,true);
    }
    vector<wxString>::const_iterator itrs = vs.begin();
    for(nCol = 0; nCol < nBeforeLoci; nCol++)
    {
      pGrid->SetCellBackgroundColour(0,nCol,
        pGrid->GetGridLineColour());
      pGrid->SetCellValue(0,nCol,wxEmptyString);
      pGrid->SetColLabelValue(nCol, *itrs);
      //pGrid->SetReadOnly(0,nCol,true);
      ++itrs;
    }
    // nCol = nBeforeLoci; // redundant

    // Set up column labels

    vnCounts.reserve(CHANNEL_MAX);
    vnChannels.reserve(CHANNEL_MAX);
    for(CLocusNameList::const_iterator itr = pLocus->begin();
      itr != pLocus->end();
      ++itr)
    {
      plc = *itr;
      const wxString &sName(plc->GetName());
      if(bAllowAmel || !CLabLocus::IsAmel(sName))
      {
        nChannel = plc->GetChannel();
        if(nChannel != nLastChannel)
        {
          if(nLastChannel > 0)
          {
            vnChannels.push_back(nLastChannel);
            vnCounts.push_back(nCount);
          }
          snChannelsUsed.insert(nChannel);
          nCount = 1;
          nLastChannel = nChannel;
        }
        else
        {
          nCount++;
        }
        pGrid->SetColLabelValue(nCol,plc->GetName());
        nCol++;
      }
      else
      {
        nRemove++;
      }
    }
    if(nRemove)
    {
      nCols -= nRemove;
      nwxGrid::SetColCount(pGrid,nCols);
    }
    vnChannels.push_back(nLastChannel);
    vnCounts.push_back(nCount);
    if(bILS)
    {
      // figure out which channnel is ILS

      int nChannelCount = 
        pKit->GetChannelCount(sKitName);
      for(nChannel = (unsigned int) nChannelCount;
        nChannel > 0;
        --nChannel)
      {
        if(snChannelsUsed.find(nChannel) ==
          snChannelsUsed.end())
        {
          nILSchannel = nChannel; // unused channel, assume ILS
          if(pnILSchannel != NULL)
          {
            *pnILSchannel = nChannel;
          }
          nChannel = 1; // loop exit
        }
      }

      // ILS Column Label
      pGrid->SetColLabelValue(nCol,"ILS*");
      vnChannels.push_back(nILSchannel);
      vnCounts.push_back(1);
      if(pnILScolumn != NULL)
      {
        *pnILScolumn = nCol;
      }
    }

    // column labels are set
    pGrid->SetRowLabelValue(0,"Channel");

    // now set channel cells in top row 
    // and colors throughout the table

    size_t nSize = vnChannels.size();
    size_t i;
    nCol = nBeforeLoci;
    const CChannelColors *pChannelColors;
    wxString sLabel;
    bool bColorSet = false;

    // set cell colors and set channel numbers/names in
    // first row

    for(i = 0; i < nSize; i++)
    {
      bColorSet = false;
      nChannel = vnChannels.at(i);
      nCount = vnCounts.at(i);
      if(nCount > 1)
      {
        pGrid->SetCellSize(0,nCol,1,nCount);
      }
      if(nChannel > 0)
      {
        pChannelColors = 
          (pKitColors == NULL)
          ? NULL
          : pKitColors->GetColorChannel((unsigned int)nChannel);
        const wxChar *psDyeLabel = NULL;
        if(pChannelColors != NULL)
        {
          // int k;
          pGrid->SetCellTextColour(0,nCol,*wxWHITE);
          pGrid->SetCellBackgroundColour(
              0,nCol,pChannelColors->GetColorAnalyzed());
          psDyeLabel = (const wxChar *)(pChannelColors->GetDyeName());
          bColorSet = true;
        }
        FORMAT_CHANNEL_DYE(&sLabel,(unsigned int) nChannel, psDyeLabel);
        pGrid->SetCellFont(0,nCol,fnBold);
        pGrid->SetCellValue(0,nCol,sLabel);
        pGrid->SetCellAlignment(0,nCol,wxALIGN_CENTRE, wxALIGN_CENTRE);
      }
      else
      {
        pGrid->SetCellValue(0,nCol,wxEmptyString);
      }
      if(!bColorSet)
      {
        pGrid->SetCellTextColour(0,nCol,pGrid->GetDefaultCellTextColour());
        pGrid->SetCellBackgroundColour(
          0,nCol,pGrid->GetDefaultCellBackgroundColour());
      }
      nCol += nCount;
      nwxGrid::SetRowReadOnly(pGrid,0,true);
    }
  }
  return sProblem.IsEmpty();
}

void CGridLocusColumns::SetReadOnly(wxGrid *pGrid, bool b)
{
  nwxGrid::SetAllReadOnly(pGrid,b);
  if(!b)
  {
    nwxGrid::SetRowReadOnly(pGrid,0,true);
  }
/*
  //  10/20/2015 OS-271 - no functional change, replaces code below
  // with SetRowReadOnly() above
    // keep top row readonly

    int nCols = pGrid->GetNumberCols();
    for(int i = 0; i < nCols; i++)
    {
      pGrid->SetReadOnly(0,i,true);
    }
  }
*/
}
