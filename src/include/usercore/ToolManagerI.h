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


#ifndef DESURA_TOOLMANAGERI_H
#define DESURA_TOOLMANAGERI_H
#ifdef _WIN32
#pragma once
#endif

typedef uint32 ToolTransactionId;

namespace XML
{
	class gcXMLElement;
}

namespace UserCore
{
	namespace Misc
	{
		class ToolProgress
		{
		public:
			ToolProgress()
			{
				percent = 0;
				total = 0;
				done = 0;
			}

			uint32 percent;
			uint32 total;
			uint32 done;
		};

		class ToolTransaction
		{
		public:
			EventV onCompleteEvent;
			EventV onStartIPCEvent;

			Event<DesuraId> onStartInstallEvent;
			Event<ToolProgress> onProgressEvent;
			Event<gcException> onErrorEvent;

			std::vector<DesuraId> toolsList;
		};
	}
	
	namespace Item
	{
		class ItemInfo;
	}

	class ToolManagerI
	{
	public:
		//! Removes an install or download transaction
		//!
		virtual void removeTransaction(ToolTransactionId ttid, bool forced)=0;

		//! Downloads all required tools
		//!
		//! @param transaction Transaction information (tool manager will own it)
		//! @param downloadSize download size
		//! @return transaction id
		//!
		virtual ToolTransactionId downloadTools(Misc::ToolTransaction* transaction)=0;

		//! Installs all required tools
		//!
		//! @param transaction Transaction information (tool manager will own it)
		//! @return transaction id
		//!
		virtual ToolTransactionId installTools(Misc::ToolTransaction* transaction)=0;

		//! Updates the event callbacks of a transaction (i.e. will be the same as the new events). Will ignore tool list
		//!
		//! @param ttid TOol Transaction id
		//! @param transaction Transaction information (tool manager will own it)
		//!
		virtual bool updateTransaction(ToolTransactionId ttid, Misc::ToolTransaction* transaction)=0;

		//! Parse xml from an item
		//!
		//! @param toolinfoNode Tool info node in item xml
		//!
		virtual void parseXml(const XML::gcXMLElement &toolinfoNode)=0;

		//! Checks to see if all tool ids are valid tools
		virtual bool areAllToolsValid(std::vector<DesuraId> &list)=0;

		//! Checks to see if all tools are downloaded ready to be installed or installed
		virtual bool areAllToolsDownloaded(std::vector<DesuraId> &list)=0;

		//! Checks to see if all tools are installed
		virtual bool areAllToolsInstalled(std::vector<DesuraId> &list)=0;

		//! Saves items to db
		//!
		virtual void saveItems()=0;

		//! Gets the tool name 
		//!
		virtual std::string getToolName(DesuraId toolId)=0;

		//! checks the item install script and finds the tools that the item will use
		//!
		virtual void findJSTools(UserCore::Item::ItemInfo* item)=0;

		//! init the script engine. Must be called before findJSTools is called. Can be called on different threads
		//!
		virtual bool initJSEngine()=0;

		//! Clean up the script engine. Can be called while active call to findJSTools
		//!
		virtual void destroyJSEngine()=0;

		//! Marks all tools as not installed
		//!
		virtual void invalidateTools(std::vector<DesuraId> &list)=0;
		
	#ifdef NIX
		//! Sym link all the tools to the item lib folder
		//!
		virtual void symLinkTools(std::vector<DesuraId> &list, const char* path)=0;
		
		//! Does this group have a speacial non installable tool
		//!
		virtual int hasNonInstallableTool(std::vector<DesuraId> &list)=0;
	#endif	

		virtual void reloadTools(DesuraId id)=0;	
	};
}

#endif //DESURA_TOOLMANAGERI_H
