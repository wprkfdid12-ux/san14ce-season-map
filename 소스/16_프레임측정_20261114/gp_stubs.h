#define HOSTTEST 1
#define SEASON_TEST 1
#define SEASON_MEASURE 1
#define MS_START_DELAY 100000000
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include "season_rules_v26.c"

/* ---- Windows API stand-ins ---- */
static char dir[]="/tmp/claude-0/rt/";
static void u2a(const u16*w,char*o){while(*w){*o++=(char)(*w=='\\'?'/':*w<128?*w:'_');w++;}*o=0;}
void *VirtualAlloc(void*a,SIZE_T n,DWORD t,DWORD p){(void)a;(void)t;(void)p;void*m=mmap(0,n,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);return m==MAP_FAILED?0:m;}
BOOL VirtualFree(void*a,SIZE_T n,DWORD t){(void)a;(void)n;(void)t;return 1;}
DWORD GetModuleFileNameW(void*m,u16*p,DWORD n){(void)m;const char*s="/tmp/claude-0/rt/SAN14_SeasonRules.dll";DWORD i=0;for(;s[i]&&i<n-1;i++)p[i]=(u16)s[i];p[i]=0;return i;}
HANDLE CreateFileW(const u16*w,DWORD acc,DWORD sh,void*sa,DWORD disp,DWORD fl,HANDLE t){(void)sh;(void)sa;(void)fl;(void)t;char p[600];u2a(w,p);
 FILE*f=fopen(p,(acc&0x80000000)&&!(acc&0x40000000)&&!(acc&4)?"rb":"ab");if(!f)return (HANDLE)(i64)-1;return f;}
BOOL ReadFile(HANDLE h,void*b,DWORD n,DWORD*r,void*o){(void)o;*r=(DWORD)fread(b,1,n,(FILE*)h);return 1;}
BOOL WriteFile(HANDLE h,const void*b,DWORD n,DWORD*w,void*o){(void)o;*w=(DWORD)fwrite(b,1,n,(FILE*)h);return 1;}
BOOL CloseHandle(HANDLE h){if(h&&h!=(HANDLE)1&&h!=(HANDLE)2)fclose((FILE*)h);return 1;}
DWORD SetFilePointer(HANDLE h,long d,long*hi,DWORD m){(void)h;(void)d;(void)hi;(void)m;return 0;}
u64 GetTickCount64(void){return (u64)time(0)*1000;}
DWORD GetFileAttributesW(const u16*w){char p[600];u2a(w,p);struct stat st;return stat(p,&st)?0xFFFFFFFF:0x80;}
BOOL DeleteFileW(const u16*w){char p[600];u2a(w,p);return unlink(p)==0;}
BOOL FlushInstructionCache(HANDLE a,const void*b,SIZE_T n){(void)a;(void)b;(void)n;return 1;}
HANDLE GetCurrentProcess(void){return (HANDLE)-1;}
DWORD GetCurrentThreadId(void){return 1;}
void GetLocalTime(SYSTEMTIME*t){memset(t,0,sizeof(*t));}
#include <pthread.h>
HANDLE CreateThread(void*a,SIZE_T b,void*fn,void*arg,DWORD f,DWORD*id){(void)a;(void)b;(void)f;(void)id;pthread_t t;pthread_create(&t,0,(void*(*)(void*))fn,arg);return (HANDLE)1;}
void Sleep(DWORD ms){usleep(ms*1000);}
HANDLE CreateMutexW(void*a,BOOL b,const u16*n){(void)a;(void)b;(void)n;return (HANDLE)2;}
DWORD WaitForSingleObject(HANDLE h,DWORD ms){(void)h;(void)ms;return 0;}
BOOL ReleaseMutex(HANDLE h){(void)h;return 1;}
DWORD GetCurrentProcessId(void){return 7;}
static u8 *fake3dm; static float *ini_list;
void*GetModuleHandleW(const u16*n){ char a[600]; u2a(n,a); if(strstr(a,"d3d11")) return fake3dm; return 0; }
typedef struct { u8 *b; u64 n; u32 type; } Reg; static Reg regs[64]; static int nregs;
static void addreg(u8*b,u64 n,u32 t){ regs[nregs].b=b; regs[nregs].n=n; regs[nregs].type=t; nregs++; for(int i=nregs-1;i>0&&regs[i].b<regs[i-1].b;i--){Reg x=regs[i];regs[i]=regs[i-1];regs[i-1]=x;} }
SIZE_T VirtualQuery(const void*a,MBI*m,SIZE_T n){ (void)n; memset(m,0,sizeof(*m)); const u8*p=(const u8*)a;
  for(int i=0;i<nregs;i++){ if(p>=regs[i].b && p<regs[i].b+regs[i].n){ m->Base=regs[i].b; m->AllocBase=regs[i].b; m->Size=regs[i].n; m->State=0x1000; m->Protect=4; m->Type=regs[i].type; return sizeof(*m);} 
    if(p<regs[i].b){ m->Base=(void*)p; m->AllocBase=0; m->Size=(u64)(regs[i].b-p); m->State=0x10000; m->Protect=1; return sizeof(*m);} }
  return 0; }
BOOL ReadProcessMemory(HANDLE h,const void*a,void*b,SIZE_T n,SIZE_T*got){ (void)h; memcpy(b,a,n); if(got)*got=n; return 1; }
static volatile int present_run=1;
static void *present_thread(void*arg){ (void)arg; while(present_run){ ini_list[241]=ini_list[244]+1.0f; ini_list[400]+=1.0f; usleep(16000);} return 0; }
BOOL VirtualProtect(void*a,SIZE_T n,DWORD f,DWORD*o){(void)a;(void)n;(void)f;if(o)*o=0;return 1;}
BOOL QueryPerformanceCounter(i64*v){struct timespec ts;clock_gettime(CLOCK_MONOTONIC,&ts);*v=ts.tv_sec*1000000000LL+ts.tv_nsec;return 1;}
BOOL QueryPerformanceFrequency(i64*v){*v=1000000000LL;return 1;}
static int keyseq[4096]; static int nkeys;
u32 SendInput(u32 n,INPUT*in,int sz){(void)sz;for(u32 i=0;i<n;i++){ if(!(in[i].dwFlags&2)&&nkeys<4096)keyseq[nkeys++]=in[i].wVk; } return n;}
void *GetForegroundWindow(void){return (void*)1;}
DWORD GetWindowThreadProcessId(void*w,DWORD*pid){(void)w;*pid=7;return 1;}

