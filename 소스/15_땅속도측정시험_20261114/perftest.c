#define HOSTTEST 1
#define SEASON_TEST 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include "/home/claude/rules/season_rules_v26.c"

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
static u64 fake_now=1000; u64 GetTickCount64(void){return fake_now;}
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

/* ---- fake game ---- */
static u8 *img, *G, *hexes, *date; static int nrep;
static int GAMEABI mock_time(void){ date[0x38]+=2; if(date[0x38]>=24){date[0x38]=0;(*(u32*)(date+0x450))++;date[0x37]++;if(date[0x37]>30){date[0x37]=1;date[0x36]++;if(date[0x36]>12){date[0x36]=1;(*(u16*)(date+0x34))++;}}} return 1;}
static void* GAMEABI mock_mgr(void){return (void*)0x1234;}
static u8* GAMEABI mock_hist(void*m,int kind,int dist,int cell,int flag,int dt,const u16*title,const u16*body,int sp,int a,int b,int c,int d){
 (void)m;(void)dist;(void)body;(void)a;(void)b;(void)c;(void)d;nrep++;
 char o[800],*p=o;p=putf8(p,title,250);*p=0;printf("  REPORT kind=%d imp=%d cell=%d date=%d speaker=%d: %s\n",kind,flag,cell,dt,sp,o);return 0;}
static float GAMEABI mock_speed(void*a,void*f,int x,int y,void*c){(void)a;(void)f;(void)x;(void)y;(void)c;return 100.0f;}
static void jmpabs(u8*at,void*to){at[0]=0xFF;at[1]=0x25;*(u32*)(at+2)=0;*(u64*)(at+6)=(u64)to;}
static long nread; static i32 GAMEABI m_readable(const void*p,UPTR n){(void)n;nread++;return p!=0;}
static int nhooks;
static i32 GAMEABI m_install(const char*id,const S14_Hook*h,u32 n){for(u32 i=0;i<n;i++){if(memcmp(h[i].target,h[i].expected,h[i].length)){printf("hook %s: expected mismatch\n",id);return 0;}}
 for(u32 i=0;i<n;i++)memcpy(h[i].target,h[i].replacement,h[i].length);nhooks++;printf("hook %s installed\n",id);return 1;}
static u8*relayp;
static u8* GAMEABI m_relay(const void*t,const void*d){(void)t;u8*r=relayp;relayp+=16;jmpabs(r,(void*)d);return r;}
static void GAMEABI m_free(void*p){(void)p;}
static void GAMEABI m_log(const char*s){printf("core log: %s\n",s);}

static u8 fh[0x800]; static void* fhvt[24];
static u8* GAMEABI mock_look(u8*d,int id){(void)d;(void)id;return fh;}
static void GAMEABI mock_setfood(u8*f,int v){*(i32*)(f+0x38)=v; printf("  SETFOOD -> %d\n",v);}
static int nsetfire; static void GAMEABI mock_setfire(u8*h,int v){ if(v<0)v=0; if(v>720)v=720; *(u16*)(h+0x16)=(u16)v; nsetfire++; }

/* ---- v26k32 tile info mocks: the panel's layout boxes with the game's own SetSize / SetPos arithmetic, the game's box maker / hanger /
   text / size functions as stand-ins at their own addresses, and the text style objects running the GAME'S OWN style method code
   (official 0x140af0dc0 -> 0x140aec920, rollback 0x140aefff0 -> 0x140aebb60, bytes copied from the unpacked exe; memset -> libc).
   A box's style object sits at node+0x190 (block at +0x198) as in the game's text node (0x140aca380); +0x248 points at it. ---- */
typedef struct { u8 b[0x400]; } MNode; static u8 mtf[64][0x40]; static MNode mn[64]; static u16 mtxt[64][700]; static u8 *mpanel; static int nmn=16;
static int mock_nomake; static u8 *mvt[32]; static u8 mownerv[0x400]; static void *mboxvt[32]; static int mbox_n, mbox_room, mmade;
typedef void (GAMEABI *MGet)(void*,void*);
static int mock_way(u8*fo){ if(!fo) return 2; u8 buf[0x240] __attribute__((aligned(16))); memset(buf,0,sizeof buf); (*(MGet*)(*(u8**)fo+13*8))(fo,buf); return *(int*)(buf+0x70); }
static void* GAMEABI mock_memset(void*p,int v,SIZE_T n){ return memset(p,v,n); }
static int real_copy(const char*path,u32 rva,u32 n){ static u8*D; static char*dp; static long dn; if(!D||strcmp(dp,path)){ FILE*f=fopen(path,"rb"); if(!f) return 0; fseek(f,0,SEEK_END); dn=ftell(f); fseek(f,0,SEEK_SET); free(D); D=malloc(dn); fread(D,1,dn,f); fclose(f); dp=strdup(path); }
  u32 pe=*(u32*)(D+0x3c); int ns=*(u16*)(D+pe+6), opt=*(u16*)(D+pe+20); for(int i=0;i<ns;i++){ u8*s=D+pe+24+opt+40*i; u32 va=*(u32*)(s+12), rs=*(u32*)(s+16), ro=*(u32*)(s+20); if(rva>=va&&rva+n<=va+rs){ memcpy(img+rva,D+ro+rva-va,n); return 1; } } return 0; }
static u32 m_settext_rva;
static int mk_idx(u8*nd){ for(int i=0;i<nmn;i++) if(mn[i].b==nd) return i; return -1; }
static void m_store(u8*nd,const u16*s){ int k=mk_idx(nd); if(k<0) return; int i=0; while(s&&s[i]&&i<698){ mtxt[k][i]=s[i]; i++; } mtxt[k][i]=0;
  if(mock_way(*(u8**)(nd+0x248))!=3){ int j=0; for(int q=0;mtxt[k][q];q++) if(mtxt[k][q]!='\n') mtxt[k][j++]=mtxt[k][q]; mtxt[k][j]=0; } }   /* as the game: ways 1/2 take the line breaks out */
static void GAMEABI mock_settext(void*layout,int idx,const u16*s,int flag){ (void)flag; u8**a=*(u8***)((u8*)layout+8); int n=*(int*)((u8*)layout+0x10); if(idx<0||idx>=n)return; m_store(a[idx],s); }
static void GAMEABI mock_setpos(u8*nd,int x,int y){ int w=*(short*)(nd+0xf0),h=*(short*)(nd+0xf2); u8 pv=nd[0x10c]; float sx=*(float*)(nd+0xf4),sy=*(float*)(nd+0xf8);
  int ox=(int)((float)((pv>>4)*w)*sx*0.5f), oy=(int)((float)((pv&15)*h)*sy*0.5f); u8*tf=*(u8**)(nd+0xe8); *(float*)(tf+0x30)=(float)(ox+x); *(float*)(tf+0x34)=-(float)(oy+y); }
static void GAMEABI mock_setsize(u8*nd,int w,int h){ int ow=*(short*)(nd+0xf0),oh=*(short*)(nd+0xf2); u8 pv=nd[0x10c]; float sx=*(float*)(nd+0xf4),sy=*(float*)(nd+0xf8); u8*tf=*(u8**)(nd+0xe8);
  int l=(int)*(float*)(tf+0x30)-(int)((float)((pv>>4)*ow)*sx*0.5f); int t=-((int)*(float*)(tf+0x34)+(int)((float)((pv&15)*oh)*sy*0.5f));
  *(short*)(nd+0xf0)=(short)w; *(short*)(nd+0xf2)=(short)h; mock_setpos(nd,l,t); }
static u8 mstyle[64][0x100]; static int nstyle;
static void mk_style(u8*nd){ u8*s=nd+0x198; memset(s,0,0x78); *(i64*)(s+0)=-1; *(i64*)(s+8)=-1; *(float*)(s+0x10)=1; *(float*)(s+0x14)=1; *(float*)(s+0x18)=1; *(float*)(s+0x1c)=1; *(short*)(s+0x20)=-1; *(u16*)(s+0x28)=0x111;   /* 0x140aec7d0 */
  u8*fo=mstyle[nstyle++]; memset(fo,0,0x100); *(u8***)fo=mvt; memcpy(fo+8,s,0x78); *(u8**)(nd+0x248)=fo; }   /* the drawing copy (0x140acbfe0: a separate object, its block copied from node+0x198) */
static u8* GAMEABI mock_inner(u8*nd,int w){ return w?0:nd+0x198; }
static void mk_node(int i,int kind,int l,int t,int w,int h,u8 pv){ u8*nd=mn[i].b; memset(nd,0,0x400); *(void**)nd=img+(kind==2?0x1685ed8:0x1400000); *(u8**)(nd+0xe8)=mtf[i]; *(float*)(nd+0xf4)=1; *(float*)(nd+0xf8)=1; nd[0x10c]=pv; nd[0x112]=(u8)kind; *(short*)(nd+0xf0)=(short)w; *(short*)(nd+0xf2)=(short)h; *(int*)(nd+0x10)=1; mock_setpos(nd,l,t); if(kind==2) mk_style(nd); }
/* the game's box maker / hanger / text / size (stand-ins) */
static u8* GAMEABI mock_create(u8*owner,const char*name,int type,u16*wh,int z){ (void)z; if(mock_nomake||owner!=mownerv||type!=2||nmn>=64||mbox_n>=mbox_room) return 0; int i=nmn++; mk_node(i,2,0,0,wh[0],wh[1],0); mbox_n++; mmade++; printf("    (game) made text box \"%s\" %dx%d\n",name,wh[0],wh[1]); return mn[i].b; }
static void GAMEABI mock_attach(u8*parent,u8*nd,int yes,void*z){ (void)yes; (void)z; *(u8**)(nd+0xd0)=parent; (*(int*)(nd+0x10))++; }
static void GAMEABI mock_nsettext(u8*nd,const u16*s){ m_store(nd,s); }
static void GAMEABI mock_fontsz(u8*nd,int sz){ u8*fo=*(u8**)(nd+0x248); *(float*)(fo+8+0x10)=sz/36.0f; *(float*)(fo+8+0x14)=sz/24.0f; }
static int GAMEABI mbox_count(void*box){ (void)box; return mbox_n; }
static void GAMEABI mbox_reserve(void*box,i64 n){ (void)box; mbox_room+=(int)n; }   /* vt+0x60 adds room (0x140ac8660) */
static i64 GAMEABI mbox_roomf(void*box){ (void)box; return mbox_room; }
static int mfill_morale;
static void GAMEABI mock_fill(u8*panel){ static const u16 e[1]={0}; static const u16 mo[]={0xC0AC,0xAE30,0xAC10,0xC18C,'(',0xC18C,')',0};
  typedef void (GAMEABI *ST)(void*,int,const u16*,int); ST st=(ST)(img+m_settext_rva); st(panel+0x130,0xe,e,1); st(panel+0x130,0xf,e,1); if(mfill_morale) st(panel+0x130,0xf,mo,1); }
static void tile_setup(void){ const TIVer*v=0; for(int i=0;i<3;i++) if(TIV[i].stamp==gver->stamp) v=&TIV[i]; if(!v) return; tiv=v;
  int off_ver=gver->stamp==0x6a983c25u, new_ver=gver->stamp==0x6ad06f53u; m_settext_rva=new_ver?0x9332b0:off_ver?0x930d20:0x9301e0;
  jmpabs(img+m_settext_rva,(void*)mock_settext); jmpabs(img+v->setsize,(void*)mock_setsize); jmpabs(img+v->setpos,(void*)mock_setpos);
  jmpabs(img+v->create,(void*)mock_create); jmpabs(img+v->attach,(void*)mock_attach); jmpabs(img+v->nsettext,(void*)mock_nsettext); jmpabs(img+v->fontsz,(void*)mock_fontsz);
  u8*f=img+v->fill; const u8 fb[7]={0x40,0x53,0x41,0x54,0x41,0x5C,0x5B}; memcpy(f,fb,7); jmpabs(f+7,(void*)mock_fill); *(u8**)(img+v->slot)=f;
  mpanel=calloc(1,0x400); u8*lay=mpanel+0x130; static u8* arr[16]; static u8 desc[16*0x30]; for(int i=0;i<16;i++) arr[i]=mn[i].b; *(u8**)(lay+8)=(u8*)arr; *(int*)(lay+0x10)=16; *(u8**)lay=desc;
  /* the boxes as the game's own record has them (official exe, 1920 x 1080) */
  mk_node(0,0,200,-540,960,1080,0); mk_node(1,3,640,996,300,72,0); mk_node(2,0,654,1002,94,32,0); mk_node(3,3,0,0,94,32,0); mk_node(4,1,5,3,32,32,0x11); mk_node(5,2,34,2,56,28,0);
  mk_node(6,0,771,1002,66,32,0); mk_node(7,3,0,0,66,32,0); mk_node(8,1,5,3,32,32,0x11); mk_node(9,2,34,2,28,28,0);
  mk_node(10,0,860,1002,66,32,0); mk_node(11,3,0,0,66,32,0); mk_node(12,1,2,0,32,32,0x11); mk_node(13,2,34,2,28,28,0);
  mk_node(14,2,654,1036,240,24,0); mk_node(15,2,782,1036,144,24,0); for(int i=0;i<16;i++) *(u32*)(mn[i].b+0x104)=0x1B1AD;
  for(int i=1;i<16;i++) *(u8**)(mn[i].b+0xd0)=mn[0].b; *(u8**)(mn[3].b+0xd0)=mn[2].b; *(u8**)(mn[4].b+0xd0)=mn[2].b; *(u8**)(mn[5].b+0xd0)=mn[2].b;
  *(u8**)(mpanel+0xa0)=mn[0].b;
  /* the box 0xe look as the game has it: size factors 0.62 / 0.8 (not 1), a colour, the draw way 2 */
  for(int i=14;i<16;i++){ u8*s=mn[i].b+0x198; *(float*)(s+0x10)=0.62f; *(float*)(s+0x14)=0.80f; *(u32*)(s+0)=0xffe8e0d0; *(int*)(s+0x2c)=2; memcpy(*(u8**)(mn[i].b+0x248)+8,s,0x78); }
  jmpabs(img+0x28f0400,(void*)mock_inner); *(void**)(img+0x1685ed8+16*8)=img+0x28f0400;
  /* the owner the panel code makes boxes in: [[panel+0x98]+0x10], its node list at +0x18 (count vt+0x18, room vt+0x60) */
  { static u8 o98[0x40]; *(u8**)(mpanel+0x98)=o98; *(u8**)(o98+0x10)=mownerv; *(void**)mownerv=img+0x1500000; for(int i=0;i<32;i++) mboxvt[i]=img+0x28f0100; jmpabs(img+0x28f0200,(void*)mbox_count); jmpabs(img+0x28f0300,(void*)mbox_reserve); jmpabs(img+0x28f0500,(void*)mbox_roomf); mboxvt[3]=img+0x28f0200; mboxvt[4]=img+0x28f0500; mboxvt[12]=img+0x28f0300;
    *(void***)(mownerv+0x18)=mboxvt; mbox_n=40; mbox_room=40; addreg(o98,0x40,0x20000); addreg(mownerv,0x400,0x20000); }
  { const char*exe=new_ver?"/tmp/claude-0/ref/SAN14CE_KO_20261105_해제본.exe":off_ver?"/tmp/claude-0/ref/SAN14CE_KO_1.0.1_해제본.exe":"/home/claude/port/rb_KO.unpacked.exe";
    u32 get=new_ver?0xaf3b00:off_ver?0xaf0dc0:0xaefff0, inner=new_ver?0xaef640:off_ver?0xaec920:0xaebb60;
    if(!real_copy(exe,get,0x30)||!real_copy(exe,inner,0x140)){ printf("REAL CODE NOT LOADED\n"); exit(1); }
    for(int i=0;i<0x20;i++) if(img[inner+i]==0xE8){ u32 t=inner+i+5+*(i32*)(img+inner+i+1); jmpabs(img+t,(void*)mock_memset); printf("real style method %x -> %x (memset %x) from %s\n",get,inner,t,exe); break; }
    for(int i=0;i<32;i++) mvt[i]=img+get;
    printf("style objects: method rva %x, box 0xe draw way %d\n",get,mock_way(*(u8**)(mn[14].b+0x248))); addreg(mvt,sizeof mvt,0x20000); }
  addreg(mpanel,0x400,0x20000); addreg((u8*)mn,sizeof mn,0x20000); addreg((u8*)mstyle,sizeof mstyle,0x20000); }
static void tile_show(int c,const char*what){ typedef void (GAMEABI *FL)(void*); FL fl=*(FL*)(img+tiv->slot); *(u8**)(mpanel+0x178)=hexes+c*32; mfill_morale=(c%3)==0; fl(mpanel);
  char o[3000],*p=o; p+=sprintf(p,"[%s] cell %d terr %d: line2 left \"",what,c,hexes[c*32+0x10]); p=putf8(p,mtxt[0xe],200); p+=sprintf(p,"\" right \""); p=putf8(p,mtxt[0xf],200); *p=0; printf("%s\"\n",o);
  for(int i=16;i<nmn;i++){ u8*nd=mn[i].b; if(!mtxt[i][0]) continue; TIRect r=ti_rect(nd); u8*s=nd+0x198; u8*s2=*(u8**)(nd+0x248)+8; if(memcmp(s,s2,0x78)) printf("    !! line box %d: drawing copy differs from the node block\n",i-13); char q[800],*z=q; z=putf8(z,mtxt[i],300); *z=0;
    printf("    line box %d at %d,%d %dx%d size %.2f/%.2f colour %08x way %d parent %s: \"%s\"  (est %d of %d)\n",i-13,r.l,r.t,r.w,r.h,*(float*)(s+0x10),*(float*)(s+0x14),*(u32*)s,mock_way(*(u8**)(nd+0x248)),*(u8**)(nd+0xd0)==mn[0].b?"root":"OTHER",q,ti_est(mtxt[i]),272); }
  int ids[4]={0xe,0xf,1,2}; for(int k=0;k<4;k++){ TIRect r=ti_rect(mn[ids[k]].b); printf("    box %d: left %d top %d w %d h %d\n",ids[k],r.l,r.t,r.w,r.h);} }
static int GAMEABI mock_rc(int base,int flag,void*cell,void*unit,void*a5,void*a6,void*a7){(void)flag;(void)cell;(void)unit;(void)a5;(void)a6;(void)a7;return base;}
int ht_main(int argc,char**argv){ setvbuf(stdout,0,_IONBF,0);
 int test=argc>1 && atoi(argv[1])!=0;
 use_version(getenv("HOSTVER")?atoi(getenv("HOSTVER")):0); printf("host test table: %s\n", gver->name);
 system("mkdir -p /tmp/claude-0/rt/SAN14Mods/SeasonRules && rm -f /tmp/claude-0/rt/SAN14Mods/SeasonRules/SeasonRules.log && cp SeasonRules_v26.dat /tmp/claude-0/rt/SAN14Mods/SeasonRules/SeasonRules.dat");

 img=mmap(0,0x2a10000,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
 G=calloc(1,0x100000); *(u8**)(img+RVA_GAMEDB)=G;
 hexes=calloc(48400,32); u8 terr[48400]; FILE*f=fopen("hex_terr.bin","rb"); fread(terr,1,48400,f); fclose(f);
 for(int c=0;c<48400;c++){hexes[c*32+0x10]=terr[c]; *(u8**)(G+DB_HEX+8*c)=hexes+c*32;}
 date=calloc(1,0x460); *(u8**)(G+DB_DATE)=date; *(u16*)(date+0x34)=218; date[0x36]=2; date[0x37]=21; *(u32*)(date+0x450)=90770;
 /* bases on every city / gate / village cell (first 400) */
 int nb=0; for(int c=0;c<48400&&nb<500;c++) if(terr[c]>=15&&terr[c]<=18){ u8*b=calloc(1,0x200); *(u8**)b=img+VT_FOOT; *(u16*)(b+0x3a)=(u16)c; b[0x54]=90; *(u16*)(b+0x44)=480; b[0x34]=5; b[0x35]=5;
   if(terr[c]>=16){u8*bd=calloc(1,0x100); *(u16*)(bd+0x10)=(u16)c; *(u16*)(bd+0x14)=3000; *(u8**)(G+DB_BUILD+8*(nb+1))=bd;}
   u16 nm[4]={0xC2E0,0xC57C,(u16)('0'+nb%10),0}; memcpy(b+0x10,nm,8); *(u8**)(G+DB_FOOT+8*(++nb))=b; }
 printf("bases %d\n",nb);
 /* code stubs */
 { void**vtp=(void**)(img+VT_FOOTHOLD); vtp[20]=img+RVA_GETFOOD; vtp[21]=(void*)mock_setfood; *(void**)fh=vtp; } *(i32*)(fh+0x38)=12000; *(u8**)(G+DB_FOOTHOLD+8*5)=fh;
 { u8*t=img+RVA_TIME_FN; static u8 dummy[0x1000]; t[0]=0x48;t[1]=0x8b;t[2]=0x05;*(i32*)(t+3)=0x40-7; *(u64*)(t+0x40)=(u64)dummy;   /* v26k41: the time step's own start as the entry hook expects it */
   t[7]=0x48;t[8]=0x8b;t[9]=0x88;*(i32*)(t+10)=0; jmpabs(t+14,(void*)mock_time); } jmpabs(img+RVA_HISTMGR,(void*)mock_mgr); jmpabs(img+RVA_HISTORY,(void*)mock_hist);
 static const u16 empty[1]={0}; *(const u16**)(img+RVA_EMPTYSTR)=empty;
 u8*site=img+RVA_TIME_SITE; site[-4]=0x48;site[-3]=0x83;site[-2]=0xEC;site[-1]=0x28; site[0]=0xE8; *(i32*)(site+1)=(i32)(RVA_TIME_FN-RVA_TIME_SITE-5); site[5]=0x48;site[6]=0x83;site[7]=0xC4;site[8]=0x28;site[9]=0xC3;
 u8*sp=img+RVA_SPEED_FN;
 if(gv_new){ u8 head[8]={0x40,0x57,0x41,0x55,0x41,0x56,0x41,0x57}; memcpy(sp,head,8); u8 pops[7]={0x41,0x5F,0x41,0x5E,0x41,0x5D,0x5F}; memcpy(sp+8,pops,7); jmpabs(sp+15,(void*)mock_speed); }   /* v26k41: the 2026-11-05 start */
 else { u8 head[7]={0x40,0x53,0x56,0x41,0x55,0x41,0x56}; memcpy(sp,head,7); u8 pops[6]={0x41,0x5E,0x41,0x5D,0x5E,0x5B}; memcpy(sp+7,pops,6); jmpabs(sp+13,(void*)mock_speed); }
 { u8 *fr=img+RVA_FIRERATE; const u8 pro[13]={0x40,0x56,0x41,0x54,0x41,0x57,0x48,0x81,0xec,0xb0,0x02,0x00,0x00}; memcpy(fr,pro,13);
   const u8 body[]={0x48,0x81,0xc4,0xb0,0x02,0x00,0x00, 0x41,0x5F, 0x41,0x5C, 0x5E, 0xB8,0x3C,0x00,0x00,0x00, 0xC3}; memcpy(fr+13,body,sizeof body); }
 { u8 *sf=img+RVA_SETHEXFIRE; const u8 pro[12]={0x48,0x89,0x5c,0x24,0x08,0x57,0x48,0x83,0xec,0x20,0x8b,0xda}; memcpy(sf,pro,12);
   const u8 body[]={0x48,0x83,0xc4,0x20, 0x5f, 0x48,0x8b,0x5c,0x24,0x08}; memcpy(sf+12,body,sizeof body); jmpabs(sf+12+sizeof body,(void*)mock_setfire); }
 relayp=img+0x2900000;
 static S14_Resolved res; res.image_size=0x29ff000; res.image=img;
 static S14_CoreAPI api_; api_.size=sizeof(api_); api_.abi=0x00010000; api_.resolved=&res; api_.Readable=m_readable; api_.InstallHooks=m_install; api_.MakeRelay=m_relay; api_.FreeRelay=m_free; api_.Log=m_log;
 self_module=0;
 fake3dm=mmap(0,0x32b000,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
 *(u32*)(fake3dm+0x3c)=0x80; *(u32*)(fake3dm+0x80)=0x4550; *(u32*)(fake3dm+0x88)=0x5c9b4a3e; *(u32*)(fake3dm+0x80+0x50)=0x32b000;
 { static const u32 RV[4]={0xf7f43,0xf7f4f,0xf7f56,0x8638f}; static const u8 BY[4][7]={{0x48,0x8b,0x15,0x9e,0x39,0x1f,0x00},{0x4c,0x8b,0x82,0xa0,0x0b,0x00,0x00},{0x4c,0x2b,0x82,0x98,0x0b,0x00,0x00},{0x48,0x8b,0x0d,0x52,0x55,0x26,0x00}}; for(int i=0;i<4;i++) memcpy(fake3dm+RV[i],BY[i],7); }
 { u8 *Gm=calloc(1,0x1000); ini_list=calloc(7460*4,4); *(u8**)(fake3dm+0x2eb8e8)=Gm; *(float**)(Gm+0xb98)=ini_list; *(float**)(Gm+0xba0)=ini_list+7460*4; ini_list[240]=7171.0f; }
 { pthread_t pt; pthread_create(&pt,0,present_thread,0); }
 if(test){ FILE*ff=fopen("/tmp/claude-0/rt/SAN14Mods/SeasonRules/SeasonRules_test.flag","w"); fputs("test",ff); fclose(ff); }
 { u8 *tb=calloc(1,0x1000); int D[8]={16,24,12,16,12,12,13,21},C[8]={600,1000,300,1200,500,1500,500,800},E[8]={400,600,300,400,300,300,500,800};
   for(int k=0;k<8;k++){ u8*r=tb+0x100+k*0x30; *(u16*)(r+8)=(u16)D[k]; *(u16*)(r+10)=(u16)C[k]; *(u16*)(r+12)=(u16)E[k]; }
   u16 s1[]={0xD1A0,0xC131,0}, s2[]={0xC11D,0xBCBD,0}; memcpy(tb+0x800,s1,6); memcpy(tb+0x900,s2,6); addreg(tb,0x1000,0x20000); }
 addreg(img,0x29ff000,0x1000000);
 if(getenv("TITEST")) { if(getenv("NOMAKE")) mock_nomake=1; tile_setup(); }
 u32 ok=SAN14_ModInitV1(&api_); { char a1[600],a2[600]; u2a(logpath,a1); u2a(datpath,a2); printf("paths [%s] [%s]\n",a1,a2);} printf("init %u hooks %d readable so far %ld\n",ok,nhooks,nread);
 int na=0;
 /* armies: some on freezing river cells, fire/flood/slide cells */
 srand(5);
 for(int k=0;k<nf&&na<40;k+=17){ int c=fcells[k]; u8*a=calloc(1,0x200);*(u8**)a=img+VT_ARMY;*(u16*)(a+0x16)=5000;a[0x1a]=90;*(u16*)(a+0x2a)=(u16)c;*(u16*)(a+0x12)=(u16)(100+na);*(u8**)(G+DB_ARMY+8*(++na))=a; }
 for(int c=0;c<48400&&na<200;c+=7) if(firez[c]==1||slidez[c]==1||(floodz[c]>>4)==1){ u8*a=calloc(1,0x200);*(u8**)a=img+VT_ARMY;*(u16*)(a+0x16)=5000;a[0x1a]=90;*(u16*)(a+0x2a)=(u16)c;*(u16*)(a+0x12)=(u16)(100+na);*(u8**)(G+DB_ARMY+8*(++na))=a; }
 printf("armies %d\n",na);


 /* some burning game hexes (화계) in the first rows of the map, to see the rain shorten them */
 for(int c=3000;c<3600;c+=7) *(u16*)(hexes+c*32+0x16)=200;
 typedef int (GAMEABI *Step)(void); Step step=(Step)(site-4); typedef float (GAMEABI *Sp)(void*,void*,int,int,void*); Sp spd=(Sp)sp;
 typedef int (GAMEABI *FR)(void*,void*,int); FR fr=(FR)(img+RVA_FIRERATE);
 int lastm=-1; int days=argc>2?atoi(argv[2]):400; int quiet=argc>3;
 for(int s=0;s<12*days;s++){ step(); if(test&&s==3)usleep(2500000); if(test&&s==60)usleep(1500000); if(argc>4&&s%12==0)usleep(300000);
   if(date[0x36]!=lastm){ lastm=date[0x36];
     int fr_=0,bl=0,rn=0,r3=0,wt=0,cr=0,bn=0,fd=0,sn=0; for(int c=0;c<48400;c++){ if(hexes[c*32+0x10]==13&&terr[c]==14)fr_++; if(hexes[c*32+0x10]==19&&terr[c]!=19)bl++; if(rain[c])rn++; if(rain[c]==3)r3++; if(wet[c]>60)wt++; if(crack[c])cr++; if(burning[c])bn++; if(flooded[c])fd++; if(cSnow[c])sn++; }
     u8 a[0x200]; memset(a,0,sizeof a); *(u8**)a=img+VT_ARMY; int rc=-1,rc2=-1; for(int c=0;c<48400;c++){ if(rc<0&&rain[c]==3&&terr[c]==1)rc=c; if(rc2<0&&rain[c]==3&&terr[c]==3)rc2=c; }
     float sr=rc>=0?spd(a,0,0,0,hexes+rc*32):-1, sr2=rc2>=0?spd(a,0,0,0,hexes+rc2*32):-1;
     int frr=rc>=0?fr(0,hexes+rc*32,0):-1; int frd=fr(0,hexes+5000*32,0);
     { static FILE*rd; if(!rd&&getenv("RAINDUMP")) rd=fopen(getenv("RAINDUMP"),"wb"); if(rd){ fwrite(rain,1,48400,rd); fflush(rd);} }
     if(!quiet) printf("%d/%d t%d: ice %d blocked %d rain %d (heavy %d) wet %d crack %d burning %d flooded %d snow %d | speed heavy-rain plain %.0f forest %.0f | firerate dry %d heavy-rain %d | setfire calls %d\n",*(u16*)(date+0x34),date[0x36],g_turn,fr_,bl,rn,r3,wt,cr,bn,fd,sn,sr,sr2,frd,frr,nsetfire);
   } }
 printf("reports %d  readable calls %ld\n",nrep,nread);
 if(getenv("TITEST")){ printf("tile info on %d\n",ti_on);
   int pick[12]; const char*nm[12]={"nothing","snow cover","rain","downpour+snow","burning","flooded","quaked","landslide","crack+fuel","ash on slide road","mud","ice"}; for(int i=0;i<12;i++)pick[i]=-1;
   for(int c=0;c<48400;c++){ if(pick[0]<0&&!rain[c]&&!cSnow[c]&&!burning[c]&&!flooded[c]&&!quaked[c]&&!mud[c]&&!ash[c]&&!crack[c]&&mine[c]==0&&terr[c]==1) pick[0]=c;
     if(pick[1]<0&&cSnow[c]&&!rain[c])pick[1]=c; if(pick[2]<0&&rain[c]==2&&!snowing[c])pick[2]=c; if(pick[3]<0&&rain[c]==3&&snowing[c]&&cSnow[c])pick[3]=c; if(pick[4]<0&&burning[c])pick[4]=c; if(pick[5]<0&&flooded[c])pick[5]=c;
     if(pick[6]<0&&quaked[c])pick[6]=c; if(pick[7]<0&&mine[c]==2)pick[7]=c; if(pick[8]<0&&crack[c]&&fuelT[c])pick[8]=c; if(pick[9]<0&&ash[c]&&slidez[c]&&!near_gate[c])pick[9]=c; if(pick[10]<0&&mud[c])pick[10]=c; if(pick[11]<0&&mine[c]==1)pick[11]=c; }
   for(int i=1;i<12;i++) if(pick[i]>=0) tile_show(pick[i],nm[i]); else printf("[%s] none in this run\n",nm[i]);
   if(pick[0]>=0) tile_show(pick[0],"nothing (restored?)");
   for(int i=1;i<12;i++) if(pick[i]>=0){ tile_show(pick[i],nm[i]); break; }
   if(pick[0]>=0) tile_show(pick[0],"nothing again");
   if(test){ int gc[6]={-1,-1,-1,-1,-1,-1}; const char*gn[6]={"TEST thin snow","TEST deep snow","TEST river ice","TEST fresh mud","TEST drying mud","TEST burnt slope"};
     for(int i=0;i<ntcov;i++){ int c=tcov_c[i]; if(tcov_l[i]==1&&gc[0]<0)gc[0]=c; if(tcov_l[i]==2&&gc[1]<0)gc[1]=c; }
     for(int i=0;i<ntice;i++) if(mine[tice_c[i]]==1){gc[2]=tice_c[i];break;}
     for(int i=0;i<ntmud;i++){ int c=tmud_c[i]; if(tmud_l[i]==2&&gc[3]<0)gc[3]=c; if(tmud_l[i]==1&&gc[4]<0)gc[4]=c; }
     if(ntash) gc[5]=tash_c[0]; { int ok=0; for(int i=0;i<ntash;i++) ok+=ash[tash_c[i]]&&ashLeft[tash_c[i]]==8; printf("burnt slope cells %d, held %d\n",ntash,ok); }
     int iceOk=0; for(int i=0;i<ntice;i++) iceOk+=mine[tice_c[i]]==1; int snOk=0; for(int i=0;i<ntcov;i++) snOk+=cSnow[tcov_c[i]]==tcov_l[i]; int mdOk=0; for(int i=0;i<ntmud;i++) mdOk+=mud[tmud_c[i]]==tmud_l[i];
     printf("forced ground after %d days: snow %d/%d held, ice %d/%d frozen, mud %d/%d held\n",days,snOk,ntcov,iceOk,ntice,mdOk,ntmud);
     for(int k=0;k<6;k++) if(gc[k]>=0){ printf("  %s cell %d wx %d cstate %d\n",gn[k],gc[k],cell_wx(gc[k],g_turn),cstate[gc[k]]); tile_show(gc[k],gn[k]); } else printf("[%s] none\n",gn[k]); } }

 if(getenv("TITEST")&&getenv("TICOMBO")){ printf("---- combos (states set by hand on plain cells away from everything) ----\n");
   int base=0; for(int c=30000;c<48000;c++){ int ok=terr[c]==1&&fuelT[c]&&cCR[c]>0; for(int k=0;k<6&&ok;k++){ int q=nb6(c,k); if(q<0||terr[q]!=1) ok=0; } if(ok){ base=c; break; } }
   int river=-1; for(int k=0;k<nf;k++){ int c=fcells[k]; if(vterr[c]==T_GREATRIVER){ river=c; break; } }
   #define CLR(c) do{ rain[c]=0; snowing[c]=0; burning[c]=0; flooded[c]=0; quaked[c]=0; mine[c]=0; mud[c]=0; ash[c]=0; crack[c]=0; cSnow[c]=0; for(int k_=0;k_<6;k_++){int q_=nb6(c,k_); if(q_>=0) crack[q_]=0;} for(int i_=0;i_<12;i_++) fires[i_].active=0; }while(0)
   #define FIRE(c,NN) do{ fires[0].active=1; fires[0].n=NN; for(int j_=0;j_<NN;j_++) fires[0].cells[j_]=(u16)(j_==0?c:c+220*5+j_); burning[c]=1; }while(0)
   int c=base; printf("plain cell %d (terr %d fuel %d cCR %d slidez %d)\n",c,terr[c],fuelT[c],cCR[c],slidez?slidez[c]:-1);
   CLR(c); crack[c]=1; rain[c]=1; FIRE(c,15); quaked[c]=2|(3<<2); tile_show(c,"가뭄+가랑비+큰 산불+강진 (넷 겹침)");
   CLR(c); cSnow[c]=2; rain[c]=3; snowing[c]=1; quaked[c]=2|(3<<2); mine[c]=2; tile_show(c,"깊은 눈+눈보라+강진+산사태");
   CLR(c); rain[c]=3; flooded[c]=3; quaked[c]=2|(3<<2); tile_show(c,"폭우+대홍수+강진 (셋)");
   CLR(c); crack[c]=1; FIRE(c,40); tile_show(c,"가뭄+대산불");
   CLR(c); crack[c]=1; tile_show(c,"가뭄만");
   CLR(c); { int q=nb6(c,0); crack[q]=1; } tile_show(c,"옆 칸만 가뭄 (가뭄 근처)");
   CLR(c); quaked[c]=1|(2<<2); tile_show(c,"지진만");
   CLR(c); quaked[c]=3|(4<<2); rain[c]=2; tile_show(c,"대지진+비");
   CLR(c); mine[c]=2; tile_show(c,"산사태만");
   CLR(c); ash[c]=1; cSnow[c]=2; tile_show(c,"잿더미+깊은 눈");
   CLR(c); mud[c]=1; cSnow[c]=1; rain[c]=2; snowing[c]=1; tile_show(c,"진흙+얕은 눈+눈");
   CLR(c); rain[c]=3; FIRE(c,5); tile_show(c,"폭우 맞는 산불");
   CLR(c); rain[c]=2; snowing[c]=1; tile_show(c,"진눈깨비? (눈 rt2)");
   CLR(c); rain[c]=1; snowing[c]=1; tile_show(c,"진눈깨비 (rt1)");
   if(river>=0){ int r=river; CLR(r); mine[r]=1; cFrz[r>>3]|=(u8)(1<<(r&7)); cFrzN[r>>3]&=(u8)~(1<<(r&7)); tile_show(r,"언 강 (다음 턴 녹음)");
     CLR(r); mine[r]=1; cFrz[r>>3]|=(u8)(1<<(r&7)); cFrzN[r>>3]|=(u8)(1<<(r&7)); tile_show(r,"언 강 (계속 얼어 있음)"); CLR(r); }
   CLR(c); tile_show(c,"아무것도 없음"); }
 if(getenv("RCTEST")){ /* v26k38c: the route cost table and the wrapper */
   int n1=0,nr2=0,nr3=0,nice=0,nq=0,nfl=0,bad=0; for(int c=0;c<48400;c++){ float m=rc_mul[c]; if(m==1.0f){n1++;continue;}
     float fn=spd_cell_nr(c,g_turn); float want=(rain[c]>=3?1.6f:rain[c]==2?1.3f:1.0f)/(fn<1.0f?fn:1.0f); if(fabsf(m-want)>1e-4f||m<1.0f) bad++;
     if(rain[c]==2)nr2++; if(rain[c]>=3)nr3++; if(mine[c]==1)nice++; if(quaked[c])nq++; if(flooded[c])nfl++; }
   printf("RCTEST table: plain %d, dearer: rain %d downpour %d ice %d shaken %d flooded %d, wrong %d\n",n1,nr2,nr3,nice,nq,nfl,bad);
   int cr=-1,ci=-1,cq=-1; for(int c=0;c<48400;c++){ if(cr<0&&rain[c]==2&&rc_mul[c]>1.29f&&rc_mul[c]<1.31f)cr=c; if(ci<0&&mine[c]==1)ci=c; if(cq<0&&quaked[c])cq=c; }
   rc_orig=mock_rc; rc_ok=1; spd_build(); int cs[3]={cr,ci,cq}; const char*nm[3]={"rain","ice","shaken"};
   for(int k=0;k<3;k++){ if(cs[k]<0){printf("RCTEST %s none\n",nm[k]);continue;} int r=RouteCost(1000,1,hexes+cs[k]*32,(void*)1,0,0,0); rc_off=1; int r0=RouteCost(1000,1,hexes+cs[k]*32,(void*)1,0,0,0); rc_off=0;
     printf("RCTEST %s cell %d mul %.3f: cost new %d old %d\n",nm[k],cs[k],rc_mul[cs[k]],r,r0); }
   { int r=RouteCost(1000,1,0,(void*)1,0,0,0); printf("RCTEST null cell %d\n",r); int pc=-1; for(int c=0;c<48400;c++) if(rc_mul[c]==1.0f){pc=c;break;} printf("RCTEST plain cell %d\n",RouteCost(1000,1,hexes+pc*32,(void*)1,0,0,0)); }
   rc_ok=0; }
 /* armies after */
 int hurt_n=0; for(int i=1;i<=na;i++){u8*a=*(u8**)(G+DB_ARMY+8*i); if(*(u16*)(a+0x16)!=5000||a[0x1a]!=90)hurt_n++; if(*(u16*)(a+0x16)<500||a[0x1a]<10)printf("FLOOR BROKEN army %d\n",i);} printf("armies hurt %d\n",hurt_n);
 int lowo=0; for(int i=1;i<=nb;i++){u8*b=*(u8**)(G+DB_FOOT+8*i); if(b[0x54]!=90)lowo++; if(*(u16*)(b+0x44)!=480)printf("WRONG FIELD touched\n");} printf("bases with lower order %d\n",lowo); int le=0; for(int i=1;i<=3000;i++){u8*bd=*(u8**)(G+DB_BUILD+8*i); if(bd&&*(u16*)(bd+0x14)!=3000)le++;} printf("buildings with lower endurance %d\n",le);
 usleep(2500000);
 /* emulate the d3dx.ini key logic (v25.4 protocol) */
 { double acc=0,val=0,ev[16]={0},ec[16]={0}; int slot=0,dmode=0,evn=0,commits=0;
   for(int i=0;i<nkeys;i++){ int vk=keyseq[i];
     if(vk==0xFC){dmode=1;acc=0;}
     else if(vk==0xF9){dmode=0;acc=0;}
     else if(vk>=0x7C&&vk<=0x85){ if(dmode) acc=acc*10+(vk-0x7C); }
     else if(vk==0xF6){ if(dmode){slot=(int)acc;acc=0;} }
     else if(vk==0xF7){ if(dmode){int q=(int)acc/1000; if(q<=16 && (int)acc%1000==(q*37+11)%97) evn=q; else printf("  evn REJECTED %.0f\n",acc); acc=0;} }
     else if(vk==0x86){ if(dmode){val=acc;acc=0;} }
     else if(vk==0x87){ if(dmode&&slot<16){ if((long)acc==((long)val+slot*7919L)%9973){ev[slot]=val;ec[slot]=acc;commits++;} else printf("  slot %d REJECTED\n",slot); acc=0;} } }
   for(int k=0;k<evn;k++){ if(ev[k]<1) continue; unsigned v=(unsigned)ev[k],ds=(v+k*7919u)%9973u;
     int code=v/48400,cell=v%48400,ty=code/36,pp=code%36; printf("  map slot %d: type %d p %d cell %d check %s\n",k,ty,pp,cell,ds==(unsigned)ec[k]?"ok":"BAD"); }
   printf("key presses %d, slot commits %d, slots in use %d\n",nkeys,commits,evn); }
 { printf("direct list: marker %.0f echo %.0f bankcount %.0f on %.0f\n",ini_list[240],ini_list[241],ini_list[242],ini_list[243]);
   int bc=(int)ini_list[242], bank=bc/256, n=bc%256; for(int k=0;k<n;k++){ unsigned v=(unsigned)ini_list[(64+bank*16)*4+k]; if(!v)continue; printf("  direct slot %d: type %u p %u cell %u\n",k,(v/48400)/36,(v/48400)%36,v%48400);} }
 { int cnt[16]={0}; for(int c=0;c<48400;c++){ int q=c/5,k=c%5; float f=ini_list[IP_STATE*4+q]; unsigned v=(unsigned)f; int nib=(v>>(4*k))&15; cnt[nib]++; }
   printf("map state nibbles: none %d burning %d burnt %d/%d flood %d/%d/%d slide %d crack %d\n",cnt[0],cnt[1],cnt[2],cnt[3],cnt[4],cnt[5],cnt[6],cnt[7],cnt[9]);
   int rg[4]={0}; for(int i=0;i<55*55;i++){ unsigned v=(unsigned)ini_list[IP_RAIN*4+i]; int tier=(v/1000)%10; rg[tier]++; } printf("map rain grid tiers: %d %d %d %d; ctrl level %.0f ver %.0f valid %.0f frames %.0f\n",rg[0],rg[1],rg[2],rg[3],ini_list[IP_CTRL*4+1],ini_list[IP_CTRL*4+2],ini_list[IP_CTRL*4+3],ini_list[IP_CTRL*4]);
   printf("nodes dT[40] %.2f mult[40] %.2f\n",ini_list[IP_DT*4+40],ini_list[IP_MULT*4+40]); }
 { printf("grid %.1f %.4f %.1f %.1f\n",ini_list[IP_GRID*4],ini_list[IP_GRID*4+1],ini_list[IP_GRID*4+2],ini_list[IP_GRID*4+3]); int bad=0,nr=0,ns=0,nc=0;
   for(int c=0;c<48400;c++){ unsigned v=(unsigned)ini_list[IP_WX*4+c/3]; int b=(v>>(8*(c%3)))&255; if(b!=cell_wx(c,cur_turn<0?0:cur_turn))bad++; if(b&3)nr++; if(b&4)ns++; if((b>>3)&3)nc++; }
   printf("wx layer: mismatches %d, rain cells %d, snowing %d, lying snow %d\n",bad,nr,ns,nc); }
 present_run=0;
 return 0;
}

/* v26k53: the ground speed test's way switching - simulated frames of a known length per way */
int main(void){ setvbuf(stdout,0,_IONBF,0);
  system("mkdir -p /tmp/claude-0/rt/SAN14Mods/SeasonRules && rm -f /tmp/claude-0/rt/SAN14Mods/SeasonRules/SeasonRules.log");
  { const char *lp="/tmp/claude-0/rt/SAN14Mods/SeasonRules/SeasonRules.log"; int i=0; for(;lp[i];i++) logpath[i]=(u16)lp[i]; logpath[i]=0; }
  static S14_CoreAPI fake; memset(&fake,0,sizeof fake); fake.Readable=m_readable; api=&fake;
  static float L[8192*4]; static u8 Gm[0x1000]; static u8 *mod; mod=calloc(1,0x2eb8f0+16);
  *(u8**)(mod+0x2eb8e8)=Gm; *(float**)(Gm+0xb98)=L; *(float**)(Gm+0xba0)=L+8192*4; dl_mod=mod; dl_state=1; day_tick_at=1;
  const double ms[4]={10.0, 8.0, 4.0, 6.5}; int seen[4]={0}; double fa=0;
  for (long f=0; f<400000 && pf_state!=2; f++){
    int w=(int)(L[98*4]+0.5); if(w<0||w>3){printf("BAD way %d\n",w);return 1;} seen[w]++;
    fa+=ms[w]; fake_now=1000+(u64)fa; L[IP_CTRL*4]+=1.0f; perf_ab(); }
  printf("state %d final x98 %.0f frames per way %d %d %d %d\n", pf_state, L[98*4], seen[0], seen[1], seen[2], seen[3]);
  system("grep -a 'perf' /tmp/claude-0/rt/SAN14Mods/SeasonRules/SeasonRules.log | tail -6");
  return 0; }
