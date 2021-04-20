// Visual Studio 2017 version.

#include "stdafx.h"
#include "Performance2.h"
#include "pimp.h"

#include <iostream>
#include <list>
#include <future>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Timer - used to established precise timings for code.
class TIMER
{
	LARGE_INTEGER t_;

	__int64 current_time_;

	public:
		TIMER()	// Default constructor. Initialises this timer with the current value of the hi-res CPU timer.
		{
			QueryPerformanceCounter(&t_);
			current_time_ = t_.QuadPart;
		}

		TIMER(const TIMER &ct)	// Copy constructor.
		{
			current_time_ = ct.current_time_;
		}

		TIMER& operator=(const TIMER &ct)	// Copy assignment.
		{
			current_time_ = ct.current_time_;
			return *this;
		}

		TIMER& operator=(const __int64 &n)	// Overloaded copy assignment.
		{
			current_time_ = n;
			return *this;
		}

		~TIMER() {}		// Destructor.

		static __int64 get_frequency()
		{
			LARGE_INTEGER frequency;
			QueryPerformanceFrequency(&frequency); 
			return frequency.QuadPart;
		}

		__int64 get_time() const
		{
			return current_time_;
		}

		void get_current_time()
		{
			QueryPerformanceCounter(&t_);
			current_time_ = t_.QuadPart;
		}

		inline bool operator==(const TIMER &ct) const
		{
			return current_time_ == ct.current_time_;
		}

		inline bool operator!=(const TIMER &ct) const
		{
			return current_time_ != ct.current_time_;
		}

		__int64 operator-(const TIMER &ct) const		// Subtract a TIMER from this one - return the result.
		{
			return current_time_ - ct.current_time_;
		}

		inline bool operator>(const TIMER &ct) const
		{
			return current_time_ > ct.current_time_;
		}

		inline bool operator<(const TIMER &ct) const
		{
			return current_time_ < ct.current_time_;
		}

		inline bool operator<=(const TIMER &ct) const
		{
			return current_time_ <= ct.current_time_;
		}

		inline bool operator>=(const TIMER &ct) const
		{
			return current_time_ >= ct.current_time_;
		}
};

CWinApp theApp;  // The one and only application object
using namespace std;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// initialize Microsoft Foundation Classes, and print an error if failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		_tprintf(_T("Fatal Error: MFC initialization failed\n"));
		nRetCode = 1;
	}
	else
	{
		// Application starts here...

		// Time the application's execution time.
		TIMER start;	// DO NOT CHANGE THIS LINE. Timing will start here.

		//--------------------------------------------------------------------------------------
		// Insert your code from here...

		// Pushing a new scope here so the future destructors
		// will wait until the threads close before showing the timer
        {
            WIN32_FIND_DATAA ffd;
            HANDLE h_find;
			string target_dir;
            list<pimp::Image> images;

			// Async returns need to be stored outside of the
			// for loop where async is called because if async is called
			// without storing its return the future is created inside
			// of the scope of the loop and stops the loop when the destructor
			// is called until the task is completed (so no parallelization)
            list<future<void>> futures;

			if (argc != 2)
			{
				target_dir = "..\\SampleImages\\*.JP*G";
			}
			else
			{
                target_dir = (char*)argv[1];
			}	

            cout << "Input files: " << target_dir.c_str() << endl;

            h_find = FindFirstFileA(target_dir.c_str(), &ffd);
            if (INVALID_HANDLE_VALUE == h_find)
            {
				cout << "Error finding files" << endl;
				return 1;
            }

            do
            {
                string fname(ffd.cFileName);
                int dotpos = fname.find_last_of(".");
                fname = fname.substr(0, dotpos);
                images.emplace_back(fname);
            } while (FindNextFileA(h_find, &ffd));

			CreateDirectoryA("..\\Output", 0);
			cout << "Output files: ..\\Output\\*.PNG" << endl;

            for (auto &img : images)
            {
#if PIMP_MULTITHREADING == 1
                futures.emplace_back(async(launch::async, pimp::Image::ProcessImg, img));
#else
                pimp::Image::ProcessImg(img);
#endif
            }

        }
		pimp::Close();

		//-------------------------------------------------------------------------------------------------------
		// How long did it take?...   DO NOT CHANGE FROM HERE...
		
		TIMER end;

		TIMER elapsed;
		
		elapsed = end - start;

		__int64 ticks_per_second = start.get_frequency();

		// Display the resulting time...

		double elapsed_seconds = (double)elapsed.get_time() / (double)ticks_per_second;

        cout << "Elapsed time (seconds): " << elapsed_seconds << endl;

        // std::cin was not working on my machine
        system("pause");
	}

	return nRetCode;
}
