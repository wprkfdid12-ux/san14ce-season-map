/* stubs the measure build needs */
HANDLE CreateToolhelp32Snapshot(DWORD a, DWORD b){(void)a;(void)b;return (HANDLE)(i64)-1;}
BOOL Thread32First(HANDLE h, THREADENTRY32*t){(void)h;(void)t;return 0;} BOOL Thread32Next(HANDLE h, THREADENTRY32*t){(void)h;(void)t;return 0;}
BOOL Module32FirstW(HANDLE h, MODULEENTRY32W*m){(void)h;(void)m;return 0;} BOOL Module32NextW(HANDLE h, MODULEENTRY32W*m){(void)h;(void)m;return 0;}
HANDLE OpenThread(DWORD a, BOOL b, DWORD c){(void)a;(void)b;(void)c;return 0;}
DWORD SuspendThread(HANDLE h){(void)h;return (DWORD)-1;} DWORD ResumeThread(HANDLE h){(void)h;return 0;}
BOOL GetThreadContext(HANDLE h, void*c){(void)h;(void)c;return 0;} BOOL GetThreadTimes(HANDLE h,u64*a,u64*b,u64*c,u64*d){(void)h;(void)a;(void)b;(void)c;(void)d;return 0;}
BOOL SetThreadPriority(HANDLE h,int p){(void)h;(void)p;return 1;} HANDLE GetCurrentThread(void){return (HANDLE)3;}
u32 GetSystemDirectoryW(u16*b,u32 n){const char*s="C:\\Windows\\System32";u32 i=0;for(;s[i];i++)b[i]=(u16)s[i];b[i]=0;return i;}
