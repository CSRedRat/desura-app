/*
Desura is the leading indie game distribution platform
Copyright (C) 2011 Mark Chandler (Desura Net Pty Ltd)

$LicenseInfo:firstyear=2014&license=lgpl$
Copyright (C) 2014, Linden Research, Inc.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation;
version 2.1 of the License only.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, see <http://www.gnu.org/licenses/>
or write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

Linden Research, Inc., 945 Battery Street, San Francisco, CA  94111  USA
$/LicenseInfo$
*/

#ifndef DESURA_WILDCARD_DELEGATE_H
#define DESURA_WILDCARD_DELEGATE_H
#ifdef _WIN32
#pragma once
#endif

#include "Event.h"
#include "wx_controls/guiDelegate.h"
#include "managers/WildcardManager.h"
#include "MainAppI.h"

extern MainAppI* g_pMainApp;

template <class TObj>
class WildCardDelegate : public DelegateI<WCSpecialInfo&>, public InvokeI
{
public:
	WildCardDelegate(TObj* t)
	{
		m_pObj = t;
		m_bCompleted = true;
		m_bCancel = false;
	}

	~WildCardDelegate()
	{
		while (!m_bCompleted)
			gcSleep(250);
	}

	void operator()(WCSpecialInfo& a)
	{
		if (a.handled || m_pObj->isStopped())
			return;

		m_bCompleted = false;
		a.processed = false;
	
		m_pWildCardInfo = &a;
		wxGuiDelegateEvent event((InvokeI*)this, m_pObj->GetId());
		m_pObj->GetEventHandler()->AddPendingEvent(event);

		while (!a.processed && !m_pObj->isStopped())
		{
			if (a.processed || m_bCancel)
				break;

			gcSleep(1000);
		}

		m_bCompleted = true;
	}

	virtual DelegateI<WCSpecialInfo&>* clone()
	{
		return new WildCardDelegate(m_pObj);
	}

	virtual void destroy()
	{
		delete this;
	}

	virtual void cancel()
	{
		m_bCancel = true;
	}

	virtual void invoke()
	{
		if (!m_pWildCardInfo)
			return;

		if (!m_bCancel)
			g_pMainApp->processWildCards(*m_pWildCardInfo, m_pObj);

		m_pWildCardInfo->processed = true;
	}

	virtual bool equals(DelegateI<WCSpecialInfo&>* di)
	{
		WildCardDelegate<TObj> *d = dynamic_cast<WildCardDelegate<TObj>*>(di);

		if (!d)
			return false;

		return (m_pObj == d->m_pObj);
	}

	volatile bool m_bCompleted;
	volatile bool m_bCancel;

	TObj* m_pObj;
	WCSpecialInfo* m_pWildCardInfo;
};

template <class TObj>
DelegateI<WCSpecialInfo&>* wcDelegate(TObj* pObj)
{
	gcPanel* pan = dynamic_cast<gcPanel*>(pObj);
	gcFrame* frm = dynamic_cast<gcFrame*>(pObj);

	if (!pan && !frm)
	{
		assert(false);
		return nullptr;
	}

	return new WildCardDelegate<TObj>(pObj);
}

#endif
