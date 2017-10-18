#include "../includes/PortableThread.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

namespace PortableThread
{
	class CCS
	{
	public:
#ifdef _WIN32
		CCS(CRITICAL_SECTION  * pSection)
#else
		CCS(pthread_mutex_t * pSection)
#endif
		 : m_pSection(pSection)
		{
#ifdef _WIN32
			::EnterCriticalSection(m_pSection);
#else
			pthread_mutex_unlock(m_pSection);
#endif
		}

		~CCS()
		{
#ifdef _WIN32
			::LeaveCriticalSection(m_pSection);
#else
			pthread_mutex_unlock(m_pSection);
#endif
		}

#ifdef _WIN32
		CRITICAL_SECTION * m_pSection;
#else
		pthread_mutex_t * m_pSection;
#endif
	};

	class CPortableThreadHandle
	{
	public:
		friend class CPortableThread;
#ifdef _WIN32
		HANDLE m_hThreadHandle;
		DWORD m_dwThreadId;
		CRITICAL_SECTION m_Section;
#else
		pthread_t m_hThreadHandle;
		pthread_mutex_t m_Section;
#endif

		CPortableThreadHandle()
		{
#ifdef _WIN32
			m_hThreadHandle = INVALID_HANDLE_VALUE;
			m_dwThreadId = 0;
			InitializeCriticalSection(&m_Section);
#else
			m_hThreadHandle = (pthread_t)-1;
			pthread_mutex_init(&m_Section, NULL);
#endif
		}

		~CPortableThreadHandle()
		{
#ifdef _WIN32
			DeleteCriticalSection(&m_Section);
#else
			pthread_mutex_destroy(&m_Section);
#endif
		}
	private:
#ifdef _WIN32
		static unsigned long __stdcall PortableThreadFunc( void * lpParam )
#else
		static void * PortableThreadFunc(void * lpParam)
#endif
		{
			CPortableThread * pThread = reinterpret_cast<CPortableThread *>(lpParam);

			pThread->DoThreadWork();

#ifdef _WIN32
			return pThread->GetThreadExitCode();
#else
			return reinterpret_cast<void *>(pThread->GetThreadExitCode());
#endif
		}
	};
}

PortableThread::CPortableThread::CPortableThread() :
m_pRunnable(NULL)
, m_nThreadExitCode(0)
, m_pRunningArgument(NULL)
{
	m_pThreadHandle = new PortableThread::CPortableThreadHandle();
}

PortableThread::CPortableThread::CPortableThread(IPortableRunnable * pRunnable) :
m_pRunnable(pRunnable)
, m_nThreadExitCode(0)
, m_pRunningArgument(NULL)
{
	m_pThreadHandle = new PortableThread::CPortableThreadHandle();
}

PortableThread::CPortableThread::~CPortableThread()
{
	Abort();

	delete m_pThreadHandle;
}

void PortableThread::CPortableThread::Start()
{
	CCS c(&m_pThreadHandle->m_Section);
#ifdef _WIN32
	if (m_pThreadHandle->m_hThreadHandle != INVALID_HANDLE_VALUE)
		return;

	m_pThreadHandle->m_hThreadHandle = CreateThread(
		NULL,              // default security attributes
		0,                 // use default stack size
		PortableThread::CPortableThreadHandle::PortableThreadFunc,          // thread function
		this,             // argument to thread function
		0,                 // use default creation flags
		&m_pThreadHandle->m_dwThreadId);   // returns the thread identifier
#else
	if (m_pThreadHandle->m_hThreadHandle != (pthread_t)-1)
		return;

	if (pthread_create(&m_pThreadHandle->m_hThreadHandle, NULL,
		PortableThread::CPortableThreadHandle::PortableThreadFunc,
		this))
	{
		m_pThreadHandle->m_hThreadHandle = (pthread_t)-1;
	}
#endif
}

void PortableThread::CPortableThread::Abort()
{
	CCS c(&m_pThreadHandle->m_Section);

#ifdef _WIN32
	if (m_pThreadHandle->m_hThreadHandle != INVALID_HANDLE_VALUE)
	{
		::TerminateThread(m_pThreadHandle->m_hThreadHandle, 0);
		::WaitForSingleObject(m_pThreadHandle->m_hThreadHandle, INFINITE);
		::CloseHandle(m_pThreadHandle->m_hThreadHandle);
		m_pThreadHandle->m_hThreadHandle = INVALID_HANDLE_VALUE;
	}
#else
	if (m_pThreadHandle->m_hThreadHandle != (pthread_t)-1)
	{
		void * val = 0;
		pthread_cancel(m_pThreadHandle->m_hThreadHandle);
		pthread_join(m_pThreadHandle->m_hThreadHandle, &val);
		m_pThreadHandle->m_hThreadHandle = (pthread_t)-1;
	}
#endif
}

void PortableThread::CPortableThread::DoThreadWork()
{
	if (m_pRunnable)
		m_nThreadExitCode = m_pRunnable->Run(m_pRunningArgument);
}

bool PortableThread::CPortableThread::Join(int nTimeout)
{
	CCS c(&m_pThreadHandle->m_Section);

#ifdef _WIN32
	if (m_pThreadHandle->m_hThreadHandle != INVALID_HANDLE_VALUE)
	{
		if (WAIT_TIMEOUT == ::WaitForSingleObject(m_pThreadHandle->m_hThreadHandle, nTimeout))
			return false;

		::CloseHandle(m_pThreadHandle->m_hThreadHandle);

		m_pThreadHandle->m_hThreadHandle = INVALID_HANDLE_VALUE;
	}
#else
    (void)nTimeout;
	if (m_pThreadHandle->m_hThreadHandle != (pthread_t)-1)
	{
		void * val = 0;
		pthread_join(m_pThreadHandle->m_hThreadHandle, &val);
		m_pThreadHandle->m_hThreadHandle = (pthread_t)-1;
	}
#endif
	return true;
}
