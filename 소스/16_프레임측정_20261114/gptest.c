#include "gp_stubs.h"
#include "gptest_main.c"
#ifndef HAVE_LL
void *LoadLibraryW(const u16 *n){(void)n;return 0;}
void *GetProcAddress(void *m,const char *n){(void)m;(void)n;return 0;}
#endif
/* ---- a fake device and context (the two tables the profiler patches) ---- */
typedef struct { void **vt; int kind; int refs; u8 cat; } FObj;           /* kind 1 shader, 2 timestamp, 3 disjoint */
static void *dev_vt[64], *ctx_vt[128]; static FObj fdev = { dev_vt, 0, 1, 0 }, fctx = { ctx_vt, 0, 1, 0 };
static double gpu_now_ms; static FObj *bound; static u64 q_val[20000]; static int q_ready_after[20000]; static int frame_no; static int nq_made;
static FObj qobj[20000]; static FObj shaders[16]; static void *obj_vt[8];
static u32 f_release(void *o){ ((FObj*)o)->refs--; return 0; }
static long f_createps(void *d, const void *bc, SIZE_T len, void *l, void **out){ (void)d;(void)l; static int n; FObj *s=&shaders[n++]; s->vt=obj_vt; s->kind=1; s->refs=1; s->cat=((const u8*)bc)[len-1]; *out=s; return 0; }
static long f_createquery(void *d, const u32 *desc, void **q){ (void)d; if(nq_made>=20000) return -1; FObj*o=&qobj[nq_made]; o->vt=obj_vt; o->kind=desc[0]==3?3:2; o->refs=nq_made; *q=o; nq_made++; return 0; }
static void f_getimm(void *d, void **c){ (void)d; *c=&fctx; fctx.refs++; }
static double cost_of(u8 c){ return c==1?2.0:c==2?0.5:c==7?1.25:0.05; }
static void f_draw_common(void){ gpu_now_ms += bound ? cost_of(bound->cat) : 0.01; }
static int draws_seen; 
static void f_drawindexed(void*c,u32 n,u32 s,i32 b){(void)c;(void)n;(void)s;(void)b;draws_seen++;f_draw_common();}
static void f_draw(void*c,u32 n,u32 s){(void)c;(void)n;(void)s;draws_seen++;f_draw_common();}
static void f_drawii(void*c,u32 n,u32 i,u32 s,i32 b,u32 si){(void)c;(void)n;(void)i;(void)s;(void)b;(void)si;draws_seen++;f_draw_common();}
static void f_drawi(void*c,u32 n,u32 i,u32 s,u32 si){(void)c;(void)n;(void)i;(void)s;(void)si;draws_seen++;f_draw_common();}
static int begins, ends;
static void f_begin(void*c,void*q){(void)c;(void)q;begins++;}
static void f_end(void*c,void*q){(void)c; FObj*o=(FObj*)q; int i=(int)(o-qobj); ends++; if(o->kind==2){ q_val[i]=(u64)(gpu_now_ms*1000.0); } q_ready_after[i]=frame_no+2; }
static long f_getdata(void*c,void*q,void*data,u32 sz,u32 fl){(void)c;(void)fl; FObj*o=(FObj*)q; int i=(int)(o-qobj); if(frame_no<q_ready_after[i]) return 1;
  if(o->kind==3){ struct{u64 f;int d;int p;}*dj=data; if(sz<16) return -1; dj->f=1000000; dj->d=0; return 0; } *(u64*)data=q_val[i]; return 0; }
static void f_psget(void*c,void**ps,void**ci,u32*n){(void)c;(void)ci;(void)n; *ps=bound; if(bound) bound->refs++; }
static void make_bin(const char *name, u8 sum0){ char p[300]; snprintf(p,sizeof p,"/tmp/claude-0/rt/ShaderFixes/%s",name); FILE*f=fopen(p,"wb"); u8 h[24]={'D','X','B','C'}; for(int i=0;i<16;i++)h[4+i]=(u8)(sum0+i); fwrite(h,1,24,f); fclose(f); }
static void *bc_for(u8 sum0, u8 tag, SIZE_T *len){ static u8 b[16][64]; static int n; u8*x=b[n++]; x[0]='D';x[1]='X';x[2]='B';x[3]='C'; for(int i=0;i<16;i++)x[4+i]=(u8)(sum0+i); x[63]=tag; *len=64; return x; }
/* a fake SeasonSync image: PE header + one writable section with the creating function's pointer */
static u8 fake_ss[0x3000];
static i32 GAMEABI t_readable(const void*p,UPTR n){(void)p;(void)n;return 1;}
static S14_CoreAPI tapi;
int main(void){ setvbuf(stdout,0,_IONBF,0); tapi.Readable=t_readable; api=&tapi;
  system("mkdir -p /tmp/claude-0/rt/ShaderFixes");
  self_module=(void*)1;
  make_bin("d3e1f71e809bac74-ps_replace.bin",0x10); make_bin("9addb6cb884c7a7c-ps_replace.bin",0x30); make_bin("SeasonRain.ps_5_0.8000.bin",0x50);
  dev_vt[2]=(void*)f_release; dev_vt[15]=(void*)f_createps; dev_vt[24]=(void*)f_createquery; dev_vt[40]=(void*)f_getimm;
  ctx_vt[2]=(void*)f_release; ctx_vt[12]=(void*)f_drawindexed; ctx_vt[13]=(void*)f_draw; ctx_vt[20]=(void*)f_drawii; ctx_vt[21]=(void*)f_drawi;
  ctx_vt[27]=(void*)f_begin; ctx_vt[28]=(void*)f_end; ctx_vt[29]=(void*)f_getdata; ctx_vt[74]=(void*)f_psget; obj_vt[2]=(void*)f_release;
  /* 1. the pointer swap in SeasonSync's data */
  { u8*img=fake_ss; *(u32*)(img+0x3c)=0x80; *(u32*)(img+0x80)=0x4550; *(u16*)(img+0x86)=1; *(u16*)(img+0x94)=0xF0; u8*sh=img+0x80+24+0xF0; *(u32*)(sh+8)=0x1000; *(u32*)(sh+12)=0x1000; *(u32*)(sh+36)=0xC0000040u;
    addreg(img,sizeof fake_ss,0x1000000);
    *(u64*)(img+0x1008)=0x1234; int r1=gp_swap_ptr(img,(void*)0x1234,(void*)0x5678); u64 v=*(u64*)(img+0x1008);
    *(u64*)(img+0x1010)=0x9999; *(u64*)(img+0x1018)=0x9999; int r2=gp_swap_ptr(img,(void*)0x9999,(void*)0x1);
    int r3=gp_swap_ptr(img,(void*)0x4444,(void*)0x1);
    printf("swap: one %d (value %llx) two %d none %d -> %s\n",r1,(unsigned long long)v,r2,r3,(r1==1&&v==0x5678&&r2==-2&&r3==0)?"ok":"BAD"); }
  /* 2. the device: tables patched, shaders recognised */
  gp_device(&fdev,0);
  printf("state %d, files %d, ctx %p (want %p), patched draw %s createps %s\n",gp_state,gp_files_found,gp_ctx,(void*)&fctx, ctx_vt[12]==(void*)gp_DrawIndexed?"yes":"no", dev_vt[15]==(void*)gp_CreatePS?"yes":"no");
  ID3D11:;
  SIZE_T L; void *ps_near,*ps_far,*ps_rain,*ps_game; 
  void *bc=bc_for(0x10,1,&L); ((GP_CreatePS)dev_vt[15])(&fdev,bc,L,0,&ps_near);
  bc=bc_for(0x30,2,&L); ((GP_CreatePS)dev_vt[15])(&fdev,bc,L,0,&ps_far);
  bc=bc_for(0x50,7,&L); ((GP_CreatePS)dev_vt[15])(&fdev,bc,L,0,&ps_rain);
  bc=bc_for(0x70,0,&L); ((GP_CreatePS)dev_vt[15])(&fdev,bc,L,0,&ps_game);
  printf("recognised: near %d far %d rain %d game %d\n",gp_ps_get(ps_near),gp_ps_get(ps_far),gp_ps_get(ps_rain),gp_ps_get(ps_game));
  /* 3. frames: 10 near draws (2 ms each), 4 far (0.5), 300 game draws (0.05), rain on odd frames (1.25) */
  static float fc=0; gp_fcp=&fc; gp_next=0;
  void (*DI)(void*,u32,u32,i32)=(void*)ctx_vt[12]; void (*D)(void*,u32,u32)=(void*)ctx_vt[13]; void (*DII)(void*,u32,u32,u32,i32,u32)=(void*)ctx_vt[20];
  int sampled_before=0;
  for(frame_no=1;frame_no<=400;frame_no++){ fc+=1.0f; if(gp_next>GetTickCount64()) {} if(!gp_samp && !gp_pend && (frame_no%7)==0) gp_next=0;
    bound=(FObj*)ps_game; for(int i=0;i<150;i++) D(&fctx,3,0);
    bound=(FObj*)ps_near; for(int i=0;i<10;i++) DI(&fctx,30,0,0);
    bound=(FObj*)ps_far; for(int i=0;i<4;i++) DII(&fctx,30,1,0,0,0);
    bound=(FObj*)ps_game; for(int i=0;i<150;i++) D(&fctx,3,0);
    if(frame_no&1){ bound=(FObj*)ps_rain; D(&fctx,3,0); }
    gpu_now_ms+=3.0; /* idle gap between frames: must not be counted */ }
  (void)sampled_before;
  for(int g=0;g<3;g++){ GPACC*a=&gp_acc[g]; if(!a->frames) continue; printf("group %d frames %llu draws/frame %llu near %.3f (%llu) far %.3f (%llu) rain %.3f (%llu) game %.3f\n",g,(unsigned long long)a->frames,(unsigned long long)(a->draws/a->frames),
     a->ms[1]/a->frames,(unsigned long long)(a->dr[1]/a->frames),a->ms[2]/a->frames,(unsigned long long)(a->dr[2]/a->frames),a->ms[7]/a->frames,(unsigned long long)(a->dr[7]/a->frames),a->ms[0]/a->frames); }
  GPACC*a1=&gp_acc[1],*a2=&gp_acc[2]; int ok = a1->frames>5 && a2->frames>5 && fabs(a1->ms[1]/a1->frames-20.0)<0.01 && fabs(a1->ms[2]/a1->frames-2.0)<0.01 && a1->dr[7]==0 && fabs(a2->ms[7]/a2->frames-1.25)<0.01 && a1->draws/a1->frames==314 && a2->draws/a2->frames==315 && a1->ms[0]==0;
  printf("refs: near %d far %d (balanced = 1) | lost %llu disjoint %llu partial %llu tried %llu\n",((FObj*)ps_near)->refs,((FObj*)ps_far)->refs,(unsigned long long)gp_lost,(unsigned long long)gp_disjoint,(unsigned long long)gp_partial,(unsigned long long)gp_frames_tried);
  ok = ok && ((FObj*)ps_near)->refs==1 && ((FObj*)ps_far)->refs==1;
  /* 4. another context of the same kind (a deferred one) is passed through untouched */
  { FObj other={ctx_vt,0,1,0}; int d0=draws_seen, e0=ends; bound=(FObj*)ps_near; gp_next=0; fc+=1; DI(&other,3,0,0); DI(&other,3,0,0); printf("other context: draws %d marks %d -> %s\n",draws_seen-d0,ends-e0,(draws_seen-d0==2&&ends==e0)?"ok":"BAD"); ok = ok && draws_seen-d0==2 && ends==e0; }
  /* 5. the summary text */
  { FILE*f=fopen("/tmp/claude-0/rt/gpsum.txt","wb"); gp_summary(f); fclose(f); system("cat /tmp/claude-0/rt/gpsum.txt"); }
  /* 6. the sampler's file with made-up samples: modules, GPU use, the clock */
  { ms_mod=calloc(MS_MODS,sizeof(MSMOD)); ms_ht=calloc(MS_HT,sizeof(MSENT)); ct_ht=calloc(CT_HT,sizeof(CTENT)); ag_ht=calloc(AG_HT,sizeof(AGENT));
    const char *nm[4]={"san14ce_ko.exe","3dmigoto-d3d11.dll","d3d11.dll","nvwgf2umx.dll"}; for(int i=0;i<4;i++){ ms_mod[i].base=0x10000000ULL*(i+1); ms_mod[i].end=ms_mod[i].base+0x1000000; strcpy(ms_mod[i].name,nm[i]); } ms_nmod=4;
    for(int k=0;k<1000;k++){ int mi=k%10<6?0:k%10<8?1:k%10<9?2:3; MSMST*e=ms_mstat(mi); e->ex[1][0]++; e->in[1][0]++; if(mi!=0){ MSMST*g=ms_mstat(0); g->in[1][0]++; } ms_slot_samples[1][0]++; if(k%10==9) ms_wait[1][0]++; }
    ms_mstat(-1)->ex[1][0]+=3;
    ms_gbusy[1]=4300; ms_gwin[1]=10000; ms_gfr[1]=900; ms_gn[1]=5; ms_frames[1]=900; ms_front_frames[1]=300; ms_pdh_state=1;
    const char *pp="/tmp/claude-0/rt/SeasonMeasure.txt"; for(int i=0;pp[i];i++) ms_path[i]=(u16)pp[i]; ms_write_summary(); system("cat /tmp/claude-0/rt/SeasonMeasure.txt | head -40"); }
  printf("GPU CLOCK TEST %s\n", ok?"ok":"BAD");
  return 0; }
