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

*  FileName: CProcess.cpp
*  Author:   Douglas Hoffman
*
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "nwx/stdb.h"
#include <string>
#include "nwx/stde.h"

#include "CProcess.h"
#include <wx/timer.h>
#include <wx/utils.h>
#ifndef __WXMSW__
#include <sys/wait.h>
#endif

bool CProcess::Run(char **argv)
{
  bool bRtn = !m_bRunning;
  if(bRtn)
  {
    Init();
    m_nPID = ::wxExecute(argv,wxEXEC_ASYNC,this);
    if(!m_nPID)
    {
      m_nExitStatus = -1;
      m_bRunning = false; 
      m_bFailed = true;
    }
    else
    {
      m_bFailed = false;
      m_bRunning = true;
    }
    bRtn = !m_bFailed;
  }
  return bRtn;
}
CProcess::~CProcess()
{
  if(m_bRunning)
  {
    Detach();
  }
}
void CProcess::Cancel()
{
  if(m_nPID && m_bRunning && !m_bKilled)
  {
    m_bKilled = true;
    wxProcess::Kill((int)m_nPID,wxSIGKILL, wxKILL_CHILDREN);
  }
}


void CProcess::OnTerminate(int nPID,int nStatus)
{
  // this is called as an internal event because it is a virtual
  // function that has been overridden and it also called from
  // the parent window as the result of a wxProcessEvent
  //
  // The reason for the redundancy is that sometimes the
  // macintosh version does not detect when the process exits.
  //
  if(nPID == m_nPID)
  {
    m_nExitStatus = nStatus;
    m_bRunning = false;
    m_nPID = 0;
  }
}

size_t CProcess::ProcessIO(size_t nLimit)
{
  size_t nRtn = 0;
  bool bInputClosed = false;
  if(!m_bPaused)
  {
    if(!IsInputOpened())
    {
      if(m_sBuffer.Len())
      {
        m_sBuffer += "\n";
        ProcessLine(m_sBuffer.utf8_str(),m_sBuffer.Len(),false);
        m_sBuffer.Empty();
      }
      bInputClosed = true;
    }
    else 
    {
      while(IsInputAvailable() && (nRtn < nLimit))
      {
        nRtn += ProcessIO(GetInputStream(),m_sBuffer,false);
      }
    }
    if(!IsErrorOpened())
    {
      if(m_sBufferError.Len())
      {
        m_sBufferError += "\n";
        ProcessLine(m_sBufferError.utf8_str(),m_sBufferError.Len(),true);
        m_sBufferError.Empty();
      }
      if(bInputClosed && m_bRunning)
      {
    	  m_bRunning = false;
    	      // we are sometimes not notified when process ends
#ifndef __WXMSW__
    	  // need to clean up zombie because wx sometimes
    	  // fails to do this on the macintosh
    	  int nStatLoc;
    	  pid_t nPID;
    	  nPID = waitpid((pid_t)m_nPID,&nStatLoc,0);
    	  OnTerminate(m_nPID,nStatLoc);
#endif
      }
    }
    else 
    {
      while(IsErrorAvailable() && (nRtn < nLimit))
      {
        nRtn += ProcessIO(GetErrorStream(),m_sBufferError,true);
      }
    }
  }
  return nRtn;
}
bool CProcess::IsErrorOpened() const
{
  wxInputStream *pIn = GetErrorStream();
  bool bRtn = pIn && (pIn->GetLastError() != wxSTREAM_EOF);
  return bRtn;
}


size_t CProcess::ProcessIO(
  wxInputStream *pIn, 
  wxString &sLine,
  bool bErrStream)
{
  size_t nRtn = 0;
  size_t nLen;
  char *pBuffer;
  char *pEnd;
  const int BUFFER_SIZE = 512;
  char sBuffer[BUFFER_SIZE + 1];
  char EOL = '\n';
  if(pIn->CanRead())
  {
    nRtn = pIn->Read(sBuffer,BUFFER_SIZE).LastRead();
    sBuffer[nRtn] = 0;
    if(nRtn)
    {
      pBuffer = &sBuffer[0];
      for(pEnd = strchr(pBuffer,EOL);
          pEnd != NULL;
          pEnd = strchr(pBuffer,EOL))
      {
        *pEnd = 0;
        sLine.Append(pBuffer);
        sLine.Append(wxChar(EOL));
        *pEnd = EOL; // restore
        nLen = sLine.Len();
        ProcessLine(sLine.utf8_str(), nLen, bErrStream);
        sLine.Empty();
        pBuffer = pEnd;
        pBuffer++;
      }
      sLine += pBuffer;
    }
  }
  return nRtn;
}
