#pragma once

namespace PortableThread
{
	class IPortableRunnable
	{
	public:
		virtual unsigned long Run(void * pArgument) = 0;
	};

	class CPortableThreadHandle;
	class CPortableThread
	{
	private:
		friend class CPortableThreadHandle;

		CPortableThread(const CPortableThread & thread) = delete;
		CPortableThread & operator = (const CPortableThread & thread) = delete;

	public:
		CPortableThread();
		CPortableThread(IPortableRunnable * pRunnable);

		virtual ~CPortableThread();

	public:
		virtual void Start();
		virtual void Abort();
		virtual bool Join(int nTimeout = -1); //true join success, false timeout
		virtual unsigned long GetThreadExitCode() const { return m_nThreadExitCode; }

		virtual void * GetRunningArgument() const { return m_pRunningArgument; }
		virtual void SetRunningArgument(void * pArgument) { m_pRunningArgument = pArgument; }

		virtual IPortableRunnable * GetPortableRunnable() const { return m_pRunnable; }
		virtual void SetPortableRunnable(IPortableRunnable * pRunnable) { m_pRunnable = pRunnable; }

	protected:
		virtual void DoThreadWork();

	protected:
		IPortableRunnable * m_pRunnable;
		unsigned long m_nThreadExitCode;
		void * m_pRunningArgument;

	private:
		CPortableThreadHandle * m_pThreadHandle;
	};
}
