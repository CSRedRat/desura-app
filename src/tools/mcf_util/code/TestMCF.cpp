/*
Desura is the leading indie game distribution platform
Copyright (C) 2013 Mark Chandler (Desura Net Pty Ltd)

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
#include "UtilFunction.h"
#include "umcf/UMcf.h"


class TestHttpDownload : public UtilFunction
{
public:
	virtual uint32 getNumArgs()
	{
		return 3;
	}

	virtual const char* getArgDesc(size_t index)
	{
		if (index == 2)
			return "Dest Mcf";

		if (index == 1)
			return "Verify Folder";

		return "Url of mcf";
	}

	virtual const char* getFullArg()
	{
		return "httptest";
	}

	virtual const char getShortArg()
	{
		return 'a';
	}

	virtual const char* getDescription()
	{
		return "Test http download of a mcf";
	}

	virtual int performAction(std::vector<std::string> &args)
	{
		MCFCore::MCFI* mcfSrc = mcfFactory();

		mcfSrc->setFile(args[2].c_str());
		mcfSrc->setWorkerCount(1);
	
		mcfSrc->dlHeaderFromHttp(args[0].c_str());

		int res = mcfSrc->verifyInstall(args[1].c_str(), true);

		mcfSrc->getProgEvent() += delegate((UtilFunction*)this, &UtilFunction::printProgress);
		mcfSrc->getErrorEvent() += delegate((UtilFunction*)this, &UtilFunction::mcfError);
		mcfSrc->dlFilesFromHttp(args[0].c_str());

		mcfDelFactory(mcfSrc);
		return res;
	}
};


class TestUpdate : public UtilFunction
{
public:
	virtual uint32 getNumArgs()
	{
		return 2;
	}

	virtual const char* getArgDesc(size_t index)
	{
		if (index == 0)
			return "Src Folder";

		return "Dest Mcf";
	}

	virtual const char* getFullArg()
	{
		return "testupdate";
	}

	virtual const char getShortArg()
	{
		return 't';
	}

	virtual const char* getDescription()
	{
		return "Tests a update mcf to make sure its correct and will work";
	}

	virtual int performAction(std::vector<std::string> &args)
	{
		int res = 0;

		UMcf* m_pUMcf = new UMcf();
		m_pUMcf->setFile(gcWString(args[0]).c_str());
		m_pUMcf->parseMCF();

		//make sure patch is newer than currently installed version
		if (!m_pUMcf->isUpdateNewer())
		{
			//dont do shit. :P
			res = 2;
		}
		else if (m_pUMcf->isValidInstaller())
		{
			//deleteDumps();
		
			//m_pOldMcf = new umcf();
			//m_pOldMcf->loadFromFile(UPDATEXML);

			//DeleteFile(UPDATEXML);
			m_pUMcf->install(gcWString(args[1]).c_str());
			m_pUMcf->dumpXml(UPDATEXML_W);

			//m_pUMcf->removeOldFiles(m_pOldMcf, src);
			//m_pUMcf->setRegValues();
			res = 3;
		}
		else
		{
			//DeleteFile(src);
			printf("The MCF file %s is an invalid installer.", args[0].c_str());
			res = 4;
		}

		//safe_delete(m_pOldMcf);
		safe_delete(m_pUMcf);

		//remove patch
		//DeleteFile(m_szMCFPath);
		return res;
	}
};


class TestDiffUpdate : public UtilFunction
{
public:
	virtual uint32 getNumArgs()
	{
		return 3;
	}

	virtual const char* getArgDesc(size_t index)
	{
		if (index == 1)
			return "Install Folder";
		else if (index == 0)
			return "Download Url";

		return "Dest Mcf";
	}

	virtual const char* getFullArg()
	{
		return "testdiffupdate";
	}

	virtual const char getShortArg()
	{
		return 'k';
	}

	virtual const char* getDescription()
	{
		return "Tests downloading an update using diffs";
	}

	virtual int performAction(std::vector<std::string> &args)
	{
		MCFCore::MCFI* mcfSrc = mcfFactory();

		mcfSrc->setFile(args[2].c_str());
		mcfSrc->setWorkerCount(1);
	
		mcfSrc->dlHeaderFromHttp(args[0].c_str());
		mcfSrc->verifyInstall(args[1].c_str(), true, true);

		mcfSrc->getProgEvent() += delegate((UtilFunction*)this, &UtilFunction::printProgress);
		mcfSrc->getErrorEvent() += delegate((UtilFunction*)this, &UtilFunction::mcfError);
		mcfSrc->dlFilesFromHttp(args[0].c_str(), args[1].c_str());

		mcfDelFactory(mcfSrc);
		return 0;
	}
};


REG_FUNCTION(TestHttpDownload)
REG_FUNCTION(TestUpdate)
REG_FUNCTION(TestDiffUpdate)
