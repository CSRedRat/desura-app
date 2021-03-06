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

#ifndef DESURA_SAVEMCFTHREAD_H
#define DESURA_SAVEMCFTHREAD_H
#ifdef _WIN32
#pragma once
#endif

#include "Common.h"
#include "BaseMCFThread.h"
#include <atomic>

class CourgetteInstance;

namespace MCFCore
{
	namespace Thread
	{

		class SMTWorkerInfo;

		//! Save mcf thread controller. Zips the local files into a mcf
		//!
		class SMTController : public MCFCore::Thread::BaseMCFThread
		{
		public:
			//! Constructor
			//!
			//! @param num Number of workers
			//! @param caller Parent Mcf
			//!
			SMTController(uint16 num, MCFCore::MCF* caller);
			~SMTController();

			//! Gets a new task for a worker
			//!
			//! @param id Worker id
			//! @return File to process
			//!
			std::shared_ptr<MCFCore::MCFFile> newTask(uint32 id);

			//! Report a worker has completed its task
			//!
			//! @param id Worker id
			//!
			void endTask(uint32 id);

			//! Gets a worker status
			//!
			//! @param id Worker id
			//! @return Worker status
			//!
			MCFThreadStatus getStatus(uint32 id);

			//! Reports an error from a worker thread
			//!
			//! @param id Worker id
			//! @param e Exception that occured
			//!
			void reportError(uint32 id, gcException &e);

			//! Report progress from a worker
			//!
			//! @param id Worker id
			//! @param ammount Ammount completed
			//!
			void reportProgress(uint32 id, uint64 ammount);

		protected:
			void run();
			void onPause();
			void onStop();

			//! Finds a Worker given a worker id
			//!
			//! @param id worker id
			//! @return Worker
			//!
			SMTWorkerInfo* findWorker(uint32 id);

			//! Fills the list of files needed to be saved
			//!
			void fillFileList();

			//! Are all workers completed
			//!
			//! @return True if all completed, false if not
			//!
			bool workersDone();

			std::vector<SMTWorkerInfo*> createWorkers();

			//! Makes the worker threads
			//!
			//! @return True if completed, false if error
			//!
			bool initWorkers();

			//! Perform post processing on the mcf (merging parts together)
			//!
			void postProcessing();


		private:
			const std::vector<SMTWorkerInfo*> m_vWorkerList;

			std::atomic<uint32> m_iRunningWorkers;
			bool m_bCreateDiff = false;

			::Thread::WaitCondition m_WaitCond;
		};
	}
}

#endif //DESURA_SAVEMCFTHREAD_H
