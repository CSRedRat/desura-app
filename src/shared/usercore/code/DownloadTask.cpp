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

#include "Common.h"
#include "DownloadTask.h"
#include "UserTasks.h"
#include "User.h"

#include "McfManager.h"
#include "MCFDownloadProviders.h"

#include "ToolManager.h"
#ifdef WIN32
#include "GameExplorerManager.h"
#endif

namespace UserCore
{
namespace ItemTask
{

DownloadTask::DownloadTask(UserCore::Item::ItemHandle* handle, const char* mcfPath) : BaseItemTask(UserCore::Item::ITEM_STAGE::STAGE_DOWNLOAD, "Download", handle)
{
	onErrorEvent += delegate(this, &DownloadTask::onError);
	m_bInError = false;

	m_ToolTTID = -1;
	m_bToolDownloadComplete = false;

	m_szMcfPath = mcfPath;
	m_bInitFinished = false;
}

DownloadTask::~DownloadTask()
{
	clearEvents();
}

void DownloadTask::doRun()
{
	UserCore::Item::ItemInfo* pItem = getItemInfo();

	if (!pItem)
		throw gcException(ERR_BADID);

	m_hMCFile->setFile(m_szMcfPath.c_str());
	m_hMCFile->parseMCF();

	m_hMCFile->getErrorEvent() += delegate(&onErrorEvent);
	m_hMCFile->getProgEvent() += delegate(this, &DownloadTask::onProgress);
	m_hMCFile->getNewProvider() += delegate(this, &DownloadTask::onNewProvider);

	auto dp = std::make_shared<MCFDownloadProviders>(getWebCore(), getUserCore()->getUserId());
	MCFDownloadProviders::forceLoad(m_hMCFile, dp);

	if (isStopped())
		return;

	startToolDownload();

	m_hMCFile->dlFilesFromWeb();

	onComplete(m_szMcfPath);
}

void DownloadTask::startToolDownload()
{
	//dont download tools for preorders just yet
	if (getItemInfo()->getCurrentBranch()->isPreOrder())
		return;

	std::vector<DesuraId> toolList;
	getItemInfo()->getCurrentBranch()->getToolList(toolList);

	if (toolList.size() == 0)
		return;

	auto pToolManager = getUserCore()->getToolManager();

	if (!pToolManager->areAllToolsValid(toolList))
	{
		pToolManager->reloadTools(getItemId());
		getItemInfo()->getCurrentBranch()->getToolList(toolList);

		if (!pToolManager->areAllToolsValid(toolList))
			throw gcException(ERR_INVALID, "Tool ids cannot be resolved into tools.");
	}

	UserCore::Misc::ToolTransaction* tt = new UserCore::Misc::ToolTransaction();

	tt->onCompleteEvent += delegate(this, &DownloadTask::onToolComplete);
	tt->toolsList = toolList;
	
	m_ToolTTID = getUserCore()->getToolManager()->downloadTools(tt);
}

void DownloadTask::onToolComplete()
{
	m_bToolDownloadComplete = true;
}

void DownloadTask::onComplete(gcString &savePath)
{
	bool hasError = m_bInError || isStopped();

	if (m_bToolDownloadComplete)
	{
		getUserCore()->getToolManager()->removeTransaction(m_ToolTTID, hasError);
		m_ToolTTID = UINT_MAX;
	}

	if (hasError)
	{
		getItemHandle()->completeStage(true);
		return;
	}

	onCompleteStrEvent(savePath);

	if (getItemInfo()->getCurrentBranch()->isPreOrder())
	{
		getItemInfo()->addSFlag(UserCore::Item::ItemInfoI::STATUS_PRELOADED);
		getItemInfo()->delSFlag(UserCore::Item::ItemInfoI::STATUS_DOWNLOADING);
		getItemHandle()->completeStage(true);
	}
	else if (m_ToolTTID != UINT_MAX)
	{
		UserCore::Misc::ToolTransaction* tt = new UserCore::Misc::ToolTransaction();
		getUserCore()->getToolManager()->updateTransaction(m_ToolTTID, tt);

		getItemHandle()->goToStageDownloadTools(m_ToolTTID, savePath.c_str(), getMcfBranch(), getMcfBuild());
	}
	else
	{
		//mirrored in download tool item task. Make sure to update it as well
		if (HasAllFlags(getItemInfo()->getStatus(), UserCore::Item::ItemInfoI::STATUS_INSTALLCOMPLEX))
		{
			getItemHandle()->goToStageInstallComplex(getMcfBranch(), getMcfBuild());
		}
		else
		{
			getItemHandle()->goToStageInstall(savePath.c_str(), getMcfBranch());
		}
	}
}


void DownloadTask::onPause()
{
	m_hMCFile->pause();
}

void DownloadTask::onUnpause()
{
	m_hMCFile->unpause();
}

void DownloadTask::clearEvents()
{
	UserCore::User* pUser = dynamic_cast<UserCore::User*>(getUserCore());

	if (pUser)
		pUser->getBDManager()->cancelDownloadBannerHooks(this);
}

void DownloadTask::onStop()
{
	BaseItemTask::onStop();
	clearEvents();
}

void DownloadTask::onProgress(MCFCore::Misc::ProgressInfo& p)
{
	MCFCore::Misc::ProgressInfo pOut = p;
	onMcfProgressEvent(pOut);

	if (p.flag & MCFCore::Misc::ProgressInfo::FLAG_INITFINISHED)
	{
		m_bInitFinished = true;
		getItemHandle()->setPausable(true);
	}
	else if (p.flag & MCFCore::Misc::ProgressInfo::FLAG_FINALIZING)
	{
		getItemHandle()->setPausable(false);
	}

	if (getItemInfo() && m_bInitFinished)
	{
		if (getItemInfo()->isUpdating())
		{
			//for updating downloading is the first 50%
			getItemInfo()->setPercent(p.percent/2);
		}
		else
		{
			getItemInfo()->setPercent(p.percent);
		}
	}
}

void DownloadTask::onBannerComplete(MCFCore::Misc::DownloadProvider &info)
{
	if (isStopped())
		return;

	UserCore::Misc::GuiDownloadProvider gdp(MCFCore::Misc::DownloadProvider::ADD, info);
	onNewProviderEvent(gdp);
}

void DownloadTask::onNewProvider(MCFCore::Misc::DP_s& dp)
{
	if (isStopped())
		return;

	if (!dp.provider)
		return;

	if (dp.action == MCFCore::Misc::DownloadProvider::ADD)
	{
		UserCore::User* pUser = dynamic_cast<UserCore::User*>(getUserCore());

		if (pUser)
			pUser->getBDManager()->downloadBanner(this, *dp.provider);
	}
	else if (dp.action == MCFCore::Misc::DownloadProvider::REMOVE)
	{
		UserCore::Misc::GuiDownloadProvider gdp(MCFCore::Misc::DownloadProvider::REMOVE, *dp.provider);
		onNewProviderEvent(gdp);
	}
}

void DownloadTask::onError(gcException &e)
{
	Warning(gcString("Error in MCF download: {0}\n", e));
	getItemHandle()->setPausable(false);

	if (!getItemHandle()->shouldPauseOnError())
	{	
		m_bInError = true;
		getItemHandle()->resetStage(true);
	}
	else
	{
		getItemHandle()->setPaused(true, true);
	}
}

void DownloadTask::cancel()
{
	getItemHandle()->setPausable(false);
	onStop();
	getItemHandle()->resetStage(true);
}




}
}
