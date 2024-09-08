/**
 * picobench - testes de desempenho para a Raspberry Pi Pico
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include "pico/stdlib.h"

void calculaPi(void);
void linpack_test(int n);
void wheatstones(void);

int main() {
    // Inicia stdio
    stdio_init_all();
    #ifdef LIB_PICO_STDIO_USB
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    #endif

    printf("Picobench v1.00\n");

    #if PICO_RP2040
      #pragma message("Running on RP2040 - ARM Cortex-M0+")
      printf("Running on RP2040 - ARM Cortex-M0+\n\n");
    #elif PICO_RP2350
      #if PICO_RISCV
        #pragma message("Running on RP2350 - RISC-V Hazard3")
        printf("Running on RP2350 - RISC-V Hazard3\n\n");
      #else
        #pragma message("Running on RP2350 - ARM Cortex-M3")
         printf("Running on RP2350 - ARM Cortex-M3\n\n");
      #endif
    #else
      #pragma message("Running on ???")
      printf("Running on ???\n");
    #endif
    
    // Teste de processamento de números interios
    calculaPi();

    // Teste de processamento de ponto flutuante precisão simples
    linpack_test(200);

    // Teste de processamento de ponto flutuante dupla precisão
    wheatstones();

    printf ("*** FIM ***\n");

    while(1) {
      sleep_ms(100);
    }
}

static inline uint32_t board_millis(void)
{
  return to_ms_since_boot(get_absolute_time());
}


/*
 * Calculo dos dígito de Pi
 * Usando o algoritimo Spigot de Rabinowitz e Wagon
 * Adaptação da versão compacta e obsfucada escrita por Dik T. Winter:
 * 
 * int a=10000,b,c=2800,d,e,f[2801],g;main(){for(;b-c;)f[b++]=a/5;
 * for(;d=0,g=c*2;c-=14,printf("%.4d",e+d/a),e=d%a)for(b=c;d+=f[b]*a,
 * f[b]=d%--g,d/=g--,--b;d*=b);}
 * 
 * Comentários nas declarações copiados de
 * https://stackoverflow.com/questions/4084571/implementing-the-spigot-algorithm-for-%CF%80-pi
 * 
 * Daniel Quadros junho/2021
 */
 
#define NDIGITS 10000           //max digits to compute
#define LEN (NDIGITS/4+1)*14   //nec. array length


// Cálculo dos dígitos do Pi
void calculaPi() {
    int32_t a = 10000;             //new base, 4 decimal digits
    int32_t b = 0;                 //nominator prev. base
    int32_t c = LEN;               //index
    int32_t d = 0;                 //accumulator and carry
    int32_t e = 0;                 //save previous 4 digits
    int32_t *f;                    //array of 4-digit-decimals
    int32_t g = 0;                 //denom previous base


  char dig[5] = "0000"; // para fazer o print
  int n = 0;            // para mudar de linha a cada 100 dígitos

  f = (int32_t *) malloc((LEN+1)*sizeof(int32_t));

  printf ("Calculando %d digitos de Pi\n", NDIGITS);
  uint32_t inicio = board_millis();

  c = LEN;
  for(b = 0; b < c; b++) {
    f[b] = a/5;
  }

  e = 0;
  for (; c > 0; c -= 14) {
      d = 0;
      g = c*2;
      b = c;
      for (;;) {
          d += f[b]*a;
          f[b] = d % --g;
          d /= g--;
          if (--b == 0) {
              break;
          }
          d *= b;
      }
      uint16_t val = e+d/a;
      dig[0] = (val / 1000) + '0'; 
      dig[1] = ((val / 100) % 10) + '0'; 
      dig[2] = ((val / 10) % 10) + '0'; 
      dig[3] = (val % 10) + '0'; 
      printf ("%s", dig);
      n += 4;
      if (n == 76) {
        printf("\n");
        n = 0;
      }
      e = d % a;
  }

  uint32_t duracao = board_millis() - inicio;
  free(f);
  printf ("\nTempo: %lu ms\n\n", duracao);
}

/*
**
** LINPACK.C        Linpack benchmark, calculates FLOPS.
**                  (FLoating Point Operations Per Second)
**
** Translated to C by Bonnie Toy 5/88
**
** Modified by Will Menninger, 10/93, with these features:
**  (modified on 2/25/94  to fix a problem with daxpy  for
**   unequal increments or equal increments not equal to 1.
**     Jack Dongarra)
**
** - Defaults to double precision.
** - Averages ROLLed and UNROLLed performance.
** - User selectable array sizes.
** - Automatically does enough repetitions to take at least 10 CPU seconds.
** - Prints machine precision.
** - ANSI prototyping.
**
** Modified by ict@nfinit.systems, 12/18, with these features:
**
** - Improved double precision defaulting to allow -DSP to work again
** - Can now take the array size as an argument for automation purposes
** - Main function return type changed to integer for automation purposes
** - Re-organized output for cleaner reports
**
** To compile:  cc -O -o linpack linpack.c -lm
**
**
** Modified by dqsoft.blogspot@gmail.com for Arduino, sept/24:
**
** - single precision as default
** - no interactive mode
** - arsize = 200
** - replace printf by Serial.print & Serial.println
*/


#define SP

#ifndef SP
#ifndef DP
#define DP
#endif
#endif

#ifdef SP
#define ZERO        0.0
#define ONE         1.0
#define THOUSAND    1000.0
#define PREC        "Single"
#define BASE10DIG   FLT_DIG

typedef float   REAL;
#endif

#ifdef DP
#define ZERO        0.0e0
#define ONE         1.0e0
#define THOUSAND    1000.0e0
#define PREC        "Double"
#define BASE10DIG   DBL_DIG

typedef double  REAL;
#endif

/* 2022-07-26: Macro defined for memreq variable to resolve warnings
 *             during malloc check
 */                                    
#define MEM_T long

static REAL linpack  (long nreps,int arsize);
static void matgen   (REAL *a,int lda,int n,REAL *b,REAL *norma);
static void dgefa    (REAL *a,int lda,int n,int *ipvt,int *info,int roll);
static void dgesl    (REAL *a,int lda,int n,int *ipvt,REAL *b,int job,int roll);
static void daxpy_r  (int n,REAL da,REAL *dx,int incx,REAL *dy,int incy);
static REAL ddot_r   (int n,REAL *dx,int incx,REAL *dy,int incy);
static void dscal_r  (int n,REAL da,REAL *dx,int incx);
static void daxpy_ur (int n,REAL da,REAL *dx,int incx,REAL *dy,int incy);
static REAL ddot_ur  (int n,REAL *dx,int incx,REAL *dy,int incy);
static void dscal_ur (int n,REAL da,REAL *dx,int incx);
static int  idamax   (int n,REAL *dx,int incx);
static REAL second   (void);

static void *mempool;

void linpack_test(int arsize)
{
  long    arsize2d,nreps;
  size_t  malloc_arg;
  MEM_T   memreq;

  arsize/=2;
  arsize*=2;
  arsize2d = (long)arsize*(long)arsize;
  memreq=arsize2d*sizeof(REAL)+(long)arsize*sizeof(REAL)+(long)arsize*sizeof(int);

  printf("LINPACK benchmark, precisao %s\n", PREC);
  printf("Precisao do programa: %d digitos\n", BASE10DIG);
  printf("Matriz %d x %d\n", arsize, arsize);
  printf("Memoria necessaria: %ld\n", memreq);

  malloc_arg=(size_t)memreq;
  if ((MEM_T)malloc_arg!=memreq || (mempool=malloc(malloc_arg))==NULL)
  {
    printf("Memoria insuficiente!\n");
    return;
  }

  printf("    Reps Time(s) DGEFA   DGESL  OVERHEAD    KFLOPS\n");
  printf("----------------------------------------------------\n");

  // Repete o teste até passar de 10 segundos
  nreps=1;
  while (linpack(nreps,arsize)<10.)
      nreps*=2;
  free(mempool);

  printf("\n");
}

static REAL linpack(long nreps,int arsize)
    {
    REAL  *a,*b;
    REAL   norma,t1,kflops,tdgesl,tdgefa,totalt,toverhead,ops;
    int   *ipvt,n,info,lda;
    long   i,arsize2d;

    lda = arsize;
    n = arsize/2;
    arsize2d = (long)arsize*(long)arsize;
    ops=((2.0*n*n*n)/3.0+2.0*n*n);
    a=(REAL *)mempool;
    b=a+arsize2d;
    ipvt=(int *)&b[arsize];
    tdgesl=0;
    tdgefa=0;
    totalt=second();
    for (i=0;i<nreps;i++)
  {
  matgen(a,lda,n,b,&norma);
  t1 = second();
  dgefa(a,lda,n,ipvt,&info,1);
  tdgefa += second()-t1;
  t1 = second();
  dgesl(a,lda,n,ipvt,b,0,1);
  tdgesl += second()-t1;
  }
    for (i=0;i<nreps;i++)
  {
  matgen(a,lda,n,b,&norma);
  t1 = second();
  dgefa(a,lda,n,ipvt,&info,0);
  tdgefa += second()-t1;
  t1 = second();
  dgesl(a,lda,n,ipvt,b,0,0);
  tdgesl += second()-t1;
  }
    totalt=second()-totalt;
    if (totalt<0.5 || tdgefa+tdgesl<0.2)
  return(0.);
    kflops=2.*nreps*ops/(1000.*(tdgefa+tdgesl));
    toverhead=totalt-tdgefa-tdgesl;
    if (tdgefa<0.)
  tdgefa=0.;
    if (tdgesl<0.)
  tdgesl=0.;
    if (toverhead<0.)
  toverhead=0.;

    printf("%8ld %6.2f %6.2f%% %6.2f%% %6.2f%%  %9.3f\n",
            nreps,totalt,100.*tdgefa/totalt,
            100.*tdgesl/totalt,100.*toverhead/totalt,
            kflops);
    
    return(totalt);
    }


/*
** For matgen,
** We would like to declare a[][lda], but c does not allow it.  In this
** function, references to a[i][j] are written a[lda*i+j].
*/
static void matgen(REAL *a,int lda,int n,REAL *b,REAL *norma)

    {
    int init,i,j;

    init = 1325;
    *norma = 0.0;
    for (j = 0; j < n; j++)
  for (i = 0; i < n; i++)
      {
      init = (int)((long)3125*(long)init % 65536L);
      a[lda*j+i] = (init - 32768.0)/16384.0;
      *norma = (a[lda*j+i] > *norma) ? a[lda*j+i] : *norma;
      }
    for (i = 0; i < n; i++)
  b[i] = 0.0;
    for (j = 0; j < n; j++)
  for (i = 0; i < n; i++)
      b[i] = b[i] + a[lda*j+i];
    }


/*
**
** DGEFA benchmark
**
** We would like to declare a[][lda], but c does not allow it.  In this
** function, references to a[i][j] are written a[lda*i+j].
**
**   dgefa factors a double precision matrix by gaussian elimination.
**
**   dgefa is usually called by dgeco, but it can be called
**   directly with a saving in time if  rcond  is not needed.
**   (time for dgeco) = (1 + 9/n)*(time for dgefa) .
**
**   on entry
**
**      a       REAL precision[n][lda]
**              the matrix to be factored.
**
**      lda     integer
**              the leading dimension of the array  a .
**
**      n       integer
**              the order of the matrix  a .
**
**   on return
**
**      a       an upper triangular matrix and the multipliers
**              which were used to obtain it.
**              the factorization can be written  a = l*u  where
**              l  is a product of permutation and unit lower
**              triangular matrices and  u  is upper triangular.
**
**      ipvt    integer[n]
**              an integer vector of pivot indices.
**
**      info    integer
**              = 0  normal value.
**              = k  if  u[k][k] .eq. 0.0 .  this is not an error
**                   condition for this subroutine, but it does
**                   indicate that dgesl or dgedi will divide by zero
**                   if called.  use  rcond  in dgeco for a reliable
**                   indication of singularity.
**
**   linpack. this version dated 08/14/78 .
**   cleve moler, university of New Mexico, argonne national lab.
**
**   functions
**
**   blas daxpy,dscal,idamax
**
*/
static void dgefa(REAL *a,int lda,int n,int *ipvt,int *info,int roll)

    {
    REAL t;
    int j,k,kp1,l,nm1;

    /* gaussian elimination with partial pivoting */

    if (roll)
  {
  *info = 0;
  nm1 = n - 1;
  if (nm1 >=  0)
      for (k = 0; k < nm1; k++)
    {
    kp1 = k + 1;

    /* find l = pivot index */

    l = idamax(n-k,&a[lda*k+k],1) + k;
    ipvt[k] = l;

    /* zero pivot implies this column already
       triangularized */

    if (a[lda*k+l] != ZERO)
        {

        /* interchange if necessary */

        if (l != k)
      {
      t = a[lda*k+l];
      a[lda*k+l] = a[lda*k+k];
      a[lda*k+k] = t;
      }

        /* compute multipliers */

        t = -ONE/a[lda*k+k];
        dscal_r(n-(k+1),t,&a[lda*k+k+1],1);

        /* row elimination with column indexing */

        for (j = kp1; j < n; j++)
      {
      t = a[lda*j+l];
      if (l != k)
          {
          a[lda*j+l] = a[lda*j+k];
          a[lda*j+k] = t;
          }
      daxpy_r(n-(k+1),t,&a[lda*k+k+1],1,&a[lda*j+k+1],1);
      }
        }
    else
        (*info) = k;
    }
  ipvt[n-1] = n-1;
  if (a[lda*(n-1)+(n-1)] == ZERO)
      (*info) = n-1;
  }
    else
  {
  *info = 0;
  nm1 = n - 1;
  if (nm1 >=  0)
      for (k = 0; k < nm1; k++)
    {
    kp1 = k + 1;

    /* find l = pivot index */

    l = idamax(n-k,&a[lda*k+k],1) + k;
    ipvt[k] = l;

    /* zero pivot implies this column already
       triangularized */

    if (a[lda*k+l] != ZERO)
        {

        /* interchange if necessary */

        if (l != k)
      {
      t = a[lda*k+l];
      a[lda*k+l] = a[lda*k+k];
      a[lda*k+k] = t;
      }

        /* compute multipliers */

        t = -ONE/a[lda*k+k];
        dscal_ur(n-(k+1),t,&a[lda*k+k+1],1);

        /* row elimination with column indexing */

        for (j = kp1; j < n; j++)
      {
      t = a[lda*j+l];
      if (l != k)
          {
          a[lda*j+l] = a[lda*j+k];
          a[lda*j+k] = t;
          }
      daxpy_ur(n-(k+1),t,&a[lda*k+k+1],1,&a[lda*j+k+1],1);
      }
        }
    else
        (*info) = k;
    }
  ipvt[n-1] = n-1;
  if (a[lda*(n-1)+(n-1)] == ZERO)
      (*info) = n-1;
  }
    }


/*
**
** DGESL benchmark
**
** We would like to declare a[][lda], but c does not allow it.  In this
** function, references to a[i][j] are written a[lda*i+j].
**
**   dgesl solves the double precision system
**   a * x = b  or  trans(a) * x = b
**   using the factors computed by dgeco or dgefa.
**
**   on entry
**
**      a       double precision[n][lda]
**              the output from dgeco or dgefa.
**
**      lda     integer
**              the leading dimension of the array  a .
**
**      n       integer
**              the order of the matrix  a .
**
**      ipvt    integer[n]
**              the pivot vector from dgeco or dgefa.
**
**      b       double precision[n]
**              the right hand side vector.
**
**      job     integer
**              = 0         to solve  a*x = b ,
**              = nonzero   to solve  trans(a)*x = b  where
**                          trans(a)  is the transpose.
**
**  on return
**
**      b       the solution vector  x .
**
**   error condition
**
**      a division by zero will occur if the input factor contains a
**      zero on the diagonal.  technically this indicates singularity
**      but it is often caused by improper arguments or improper
**      setting of lda .  it will not occur if the subroutines are
**      called correctly and if dgeco has set rcond .gt. 0.0
**      or dgefa has set info .eq. 0 .
**
**   to compute  inverse(a) * c  where  c  is a matrix
**   with  p  columns
**         dgeco(a,lda,n,ipvt,rcond,z)
**         if (!rcond is too small){
**              for (j=0,j<p,j++)
**                      dgesl(a,lda,n,ipvt,c[j][0],0);
**         }
**
**   linpack. this version dated 08/14/78 .
**   cleve moler, university of new mexico, argonne national lab.
**
**   functions
**
**   blas daxpy,ddot
*/
static void dgesl(REAL *a,int lda,int n,int *ipvt,REAL *b,int job,int roll)

    {
    REAL    t;
    int     k,kb,l,nm1;

    if (roll)
  {
  nm1 = n - 1;
  if (job == 0)
      {

      /* job = 0 , solve  a * x = b   */
      /* first solve  l*y = b         */

      if (nm1 >= 1)
    for (k = 0; k < nm1; k++)
        {
        l = ipvt[k];
        t = b[l];
        if (l != k)
      {
      b[l] = b[k];
      b[k] = t;
      }
        daxpy_r(n-(k+1),t,&a[lda*k+k+1],1,&b[k+1],1);
        }

      /* now solve  u*x = y */

      for (kb = 0; kb < n; kb++)
    {
    k = n - (kb + 1);
    b[k] = b[k]/a[lda*k+k];
    t = -b[k];
    daxpy_r(k,t,&a[lda*k+0],1,&b[0],1);
    }
      }
  else
      {

      /* job = nonzero, solve  trans(a) * x = b  */
      /* first solve  trans(u)*y = b             */

      for (k = 0; k < n; k++)
    {
    t = ddot_r(k,&a[lda*k+0],1,&b[0],1);
    b[k] = (b[k] - t)/a[lda*k+k];
    }

      /* now solve trans(l)*x = y     */

      if (nm1 >= 1)
    for (kb = 1; kb < nm1; kb++)
        {
        k = n - (kb+1);
        b[k] = b[k] + ddot_r(n-(k+1),&a[lda*k+k+1],1,&b[k+1],1);
        l = ipvt[k];
        if (l != k)
      {
      t = b[l];
      b[l] = b[k];
      b[k] = t;
      }
        }
      }
  }
    else
  {
  nm1 = n - 1;
  if (job == 0)
      {

      /* job = 0 , solve  a * x = b   */
      /* first solve  l*y = b         */

      if (nm1 >= 1)
    for (k = 0; k < nm1; k++)
        {
        l = ipvt[k];
        t = b[l];
        if (l != k)
      {
      b[l] = b[k];
      b[k] = t;
      }
        daxpy_ur(n-(k+1),t,&a[lda*k+k+1],1,&b[k+1],1);
        }

      /* now solve  u*x = y */

      for (kb = 0; kb < n; kb++)
    {
    k = n - (kb + 1);
    b[k] = b[k]/a[lda*k+k];
    t = -b[k];
    daxpy_ur(k,t,&a[lda*k+0],1,&b[0],1);
    }
      }
  else
      {

      /* job = nonzero, solve  trans(a) * x = b  */
      /* first solve  trans(u)*y = b             */

      for (k = 0; k < n; k++)
    {
    t = ddot_ur(k,&a[lda*k+0],1,&b[0],1);
    b[k] = (b[k] - t)/a[lda*k+k];
    }

      /* now solve trans(l)*x = y     */

      if (nm1 >= 1)
    for (kb = 1; kb < nm1; kb++)
        {
        k = n - (kb+1);
        b[k] = b[k] + ddot_ur(n-(k+1),&a[lda*k+k+1],1,&b[k+1],1);
        l = ipvt[k];
        if (l != k)
      {
      t = b[l];
      b[l] = b[k];
      b[k] = t;
      }
        }
      }
  }
    }



/*
** Constant times a vector plus a vector.
** Jack Dongarra, linpack, 3/11/78.
** ROLLED version
*/
static void daxpy_r(int n,REAL da,REAL *dx,int incx,REAL *dy,int incy)

    {
    int i,ix,iy;

    if (n <= 0)
  return;
    if (da == ZERO)
  return;

    if (incx != 1 || incy != 1)
  {

  /* code for unequal increments or equal increments != 1 */

  ix = 1;
  iy = 1;
  if(incx < 0) ix = (-n+1)*incx + 1;
  if(incy < 0)iy = (-n+1)*incy + 1;
  for (i = 0;i < n; i++)
      {
      dy[iy] = dy[iy] + da*dx[ix];
      ix = ix + incx;
      iy = iy + incy;
      }
  return;
  }

    /* code for both increments equal to 1 */

    for (i = 0;i < n; i++)
  dy[i] = dy[i] + da*dx[i];
    }


/*
** Forms the dot product of two vectors.
** Jack Dongarra, linpack, 3/11/78.
** ROLLED version
*/
static REAL ddot_r(int n,REAL *dx,int incx,REAL *dy,int incy)

    {
    REAL dtemp;
    int i,ix,iy;

    dtemp = ZERO;

    if (n <= 0)
  return(ZERO);

    if (incx != 1 || incy != 1)
  {

  /* code for unequal increments or equal increments != 1 */

  ix = 0;
  iy = 0;
  if (incx < 0) ix = (-n+1)*incx;
  if (incy < 0) iy = (-n+1)*incy;
  for (i = 0;i < n; i++)
      {
      dtemp = dtemp + dx[ix]*dy[iy];
      ix = ix + incx;
      iy = iy + incy;
      }
  return(dtemp);
  }

    /* code for both increments equal to 1 */

    for (i=0;i < n; i++)
  dtemp = dtemp + dx[i]*dy[i];
    return(dtemp);
    }


/*
** Scales a vector by a constant.
** Jack Dongarra, linpack, 3/11/78.
** ROLLED version
*/
static void dscal_r(int n,REAL da,REAL *dx,int incx)

    {
    int i,nincx;

    if (n <= 0)
  return;
    if (incx != 1)
  {

  /* code for increment not equal to 1 */

  nincx = n*incx;
  for (i = 0; i < nincx; i = i + incx)
      dx[i] = da*dx[i];
  return;
  }

    /* code for increment equal to 1 */

    for (i = 0; i < n; i++)
  dx[i] = da*dx[i];
    }


/*
** constant times a vector plus a vector.
** Jack Dongarra, linpack, 3/11/78.
** UNROLLED version
*/
static void daxpy_ur(int n,REAL da,REAL *dx,int incx,REAL *dy,int incy)

    {
    int i,ix,iy,m;

    if (n <= 0)
  return;
    if (da == ZERO)
  return;

    if (incx != 1 || incy != 1)
  {

  /* code for unequal increments or equal increments != 1 */

  ix = 1;
  iy = 1;
  if(incx < 0) ix = (-n+1)*incx + 1;
  if(incy < 0)iy = (-n+1)*incy + 1;
  for (i = 0;i < n; i++)
      {
      dy[iy] = dy[iy] + da*dx[ix];
      ix = ix + incx;
      iy = iy + incy;
      }
  return;
  }

    /* code for both increments equal to 1 */

    m = n % 4;
    if ( m != 0)
  {
  for (i = 0; i < m; i++)
      dy[i] = dy[i] + da*dx[i];
  if (n < 4)
      return;
  }
    for (i = m; i < n; i = i + 4)
  {
  dy[i] = dy[i] + da*dx[i];
  dy[i+1] = dy[i+1] + da*dx[i+1];
  dy[i+2] = dy[i+2] + da*dx[i+2];
  dy[i+3] = dy[i+3] + da*dx[i+3];
  }
    }


/*
** Forms the dot product of two vectors.
** Jack Dongarra, linpack, 3/11/78.
** UNROLLED version
*/
static REAL ddot_ur(int n,REAL *dx,int incx,REAL *dy,int incy)

    {
    REAL dtemp;
    int i,ix,iy,m;

    dtemp = ZERO;

    if (n <= 0)
  return(ZERO);

    if (incx != 1 || incy != 1)
  {

  /* code for unequal increments or equal increments != 1 */

  ix = 0;
  iy = 0;
  if (incx < 0) ix = (-n+1)*incx;
  if (incy < 0) iy = (-n+1)*incy;
  for (i = 0;i < n; i++)
      {
      dtemp = dtemp + dx[ix]*dy[iy];
      ix = ix + incx;
      iy = iy + incy;
      }
  return(dtemp);
  }

    /* code for both increments equal to 1 */

    m = n % 5;
    if (m != 0)
  {
  for (i = 0; i < m; i++)
      dtemp = dtemp + dx[i]*dy[i];
  if (n < 5)
      return(dtemp);
  }
    for (i = m; i < n; i = i + 5)
  {
  dtemp = dtemp + dx[i]*dy[i] +
  dx[i+1]*dy[i+1] + dx[i+2]*dy[i+2] +
  dx[i+3]*dy[i+3] + dx[i+4]*dy[i+4];
  }
    return(dtemp);
    }


/*
** Scales a vector by a constant.
** Jack Dongarra, linpack, 3/11/78.
** UNROLLED version
*/
static void dscal_ur(int n,REAL da,REAL *dx,int incx)

    {
    int i,m,nincx;

    if (n <= 0)
  return;
    if (incx != 1)
  {

  /* code for increment not equal to 1 */

  nincx = n*incx;
  for (i = 0; i < nincx; i = i + incx)
      dx[i] = da*dx[i];
  return;
  }

    /* code for increment equal to 1 */

    m = n % 5;
    if (m != 0)
  {
  for (i = 0; i < m; i++)
      dx[i] = da*dx[i];
  if (n < 5)
      return;
  }
    for (i = m; i < n; i = i + 5)
  {
  dx[i] = da*dx[i];
  dx[i+1] = da*dx[i+1];
  dx[i+2] = da*dx[i+2];
  dx[i+3] = da*dx[i+3];
  dx[i+4] = da*dx[i+4];
  }
    }


/*
** Finds the index of element having max. absolute value.
** Jack Dongarra, linpack, 3/11/78.
*/
static int idamax(int n,REAL *dx,int incx)

    {
    REAL dmax;
    int i, ix, itemp;

    if (n < 1)
  return(-1);
    if (n ==1 )
  return(0);
    if(incx != 1)
  {

  /* code for increment not equal to 1 */

  ix = 1;
  dmax = fabs((double)dx[0]);
  ix = ix + incx;
  for (i = 1; i < n; i++)
      {
      if(fabs((double)dx[ix]) > dmax)
    {
    itemp = i;
    dmax = fabs((double)dx[ix]);
    }
      ix = ix + incx;
      }
  }
    else
  {

  /* code for increment equal to 1 */

  itemp = 0;
  dmax = fabs((double)dx[0]);
  for (i = 1; i < n; i++)
      if(fabs((double)dx[i]) > dmax)
    {
    itemp = i;
    dmax = fabs((double)dx[i]);
    }
  }
    return (itemp);
    }


static REAL second(void)
{
    return (REAL) (board_millis()/THOUSAND);
}



/*
 * {module name} Whetstone
 *
 * {module description}
 *
 * Copyright (C) {YEAR} Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/

/*
 * C Converted Whetstone Double Precision Benchmark
 *		Version 1.2	22 March 1998
 *
 *	(c) Copyright 1998 Painter Engineering, Inc.
 *		All Rights Reserved.
 *
 *		Permission is granted to use, duplicate, and
 *		publish this text and program as long as it
 *		includes this entire comment block and limited
 *		rights reference.
 *
 * Converted by Rich Painter, Painter Engineering, Inc. based on the
 * www.netlib.org benchmark/whetstoned version obtained 16 March 1998.
 *
 * A novel approach was used here to keep the look and feel of the
 * FORTRAN version.  Altering the FORTRAN-based array indices,
 * starting at element 1, to start at element 0 for C, would require
 * numerous changes, including decrementing the variable indices by 1.
 * Instead, the array E1[] was declared 1 element larger in C.  This
 * allows the FORTRAN index range to function without any literal or
 * variable indices changes.  The array element E1[0] is simply never
 * used and does not alter the benchmark results.
 *
 * The major FORTRAN comment blocks were retained to minimize
 * differences between versions.  Modules N5 and N12, like in the
 * FORTRAN version, have been eliminated here.
 *
 * An optional command-line argument has been provided [-c] to
 * offer continuous repetition of the entire benchmark.
 * An optional argument for setting an alternate LOOP count is also
 * provided.  Define PRINTOUT to cause the POUT() function to print
 * outputs at various stages.  Final timing measurements should be
 * made with the PRINTOUT undefined.
 *
 * Questions and comments may be directed to the author at
 *			r.painter@ieee.org
 */
/*
C**********************************************************************
C     Benchmark #2 -- Double  Precision Whetstone (A001)
C
C     o	This is a REAL*8 version of
C	the Whetstone benchmark program.
C
C     o	DO-loop semantics are ANSI-66 compatible.
C
C     o	Final measurements are to be made with all
C	WRITE statements and FORMAT sttements removed.
C
C**********************************************************************   
*/

//#define PRINTOUT

/* map the FORTRAN math functions, etc. to the C versions */
#define DSIN	sin
#define DCOS	cos
#define DATAN	atan
#define DLOG	log
#define DEXP	exp
#define DSQRT	sqrt
#define IF		if

/* function prototypes */
void POUT(long N, long J, long K, double X1, double X2, double X3, double X4);
void PA(double E[]);
void P0(void);
void P3(double X, double Y, double *Z);

/*
  COMMON T,T1,T2,E1(4),J,K,L
*/
double T,T1_X,T2_X,E1[5];
int J,K,L;

void wheatstones() {
  /* used in the FORTRAN version */
  long I;
  long N1, N2, N3, N4, N6, N7, N8, N9, N10, N11;
  double X1,X2,X3,X4,X,Y,Z;
  long LOOP;
  int II, JJ;

  /* added for this version */
  long loopstart;
  long startsec, finisec;
  float KIPS;

  printf("Whetstone benchmark\n");

  loopstart = 10000;		/* see the note about LOOP below */

/*
C
C	Start benchmark timing at this point.
C
*/
  startsec = board_millis();

  #ifdef PRINTOUT
  printf("Inicio: %ld\n", startsec);
  #endif

/*
C
C	The actual benchmark starts here.
C
*/
  T  = .499975;
  T1_X = 0.50025;
  T2_X = 2.0;
/*
C
C	With loopcount LOOP=10, one million Whetstone instructions
C	will be executed in EACH MAJOR LOOP..A MAJOR LOOP IS EXECUTED
C	'II' TIMES TO INCREASE WALL-CLOCK TIMING ACCURACY.
C
  LOOP = 1000;
*/
  LOOP = loopstart;
  II   = 1;

  JJ = 1;

IILOOP:
  N1  = 0;
  N2  = 12 * LOOP;
  N3  = 14 * LOOP;
  N4  = 345 * LOOP;
  N6  = 210 * LOOP;
  N7  = 32 * LOOP;
  N8  = 899 * LOOP;
  N9  = 616 * LOOP;
  N10 = 0;
  N11 = 93 * LOOP;
/*
C
C	Module 1: Simple identifiers
C
*/
  X1  =  1.0;
  X2  = -1.0;
  X3  = -1.0;
  X4  = -1.0;

  for (I = 1; I <= N1; I++) {
      X1 = (X1 + X2 + X3 - X4) * T;
      X2 = (X1 + X2 - X3 + X4) * T;
      X3 = (X1 - X2 + X3 + X4) * T;
      X4 = (-X1+ X2 + X3 + X4) * T;
  }
#ifdef PRINTOUT
  IF (JJ==II)POUT(N1,N1,N1,X1,X2,X3,X4);
#endif
  
/*
C
C	Module 2: Array elements
C
*/
  E1[1] =  1.0;
  E1[2] = -1.0;
  E1[3] = -1.0;
  E1[4] = -1.0;

  for (I = 1; I <= N2; I++) {
      E1[1] = ( E1[1] + E1[2] + E1[3] - E1[4]) * T;
      E1[2] = ( E1[1] + E1[2] - E1[3] + E1[4]) * T;
      E1[3] = ( E1[1] - E1[2] + E1[3] + E1[4]) * T;
      E1[4] = (-E1[1] + E1[2] + E1[3] + E1[4]) * T;
  }

#ifdef PRINTOUT
  IF (JJ==II)POUT(N2,N3,N2,E1[1],E1[2],E1[3],E1[4]);
#endif

/*
C
C	Module 3: Array as parameter
C
*/
  for (I = 1; I <= N3; I++)
    PA(E1);

#ifdef PRINTOUT
  IF (JJ==II)POUT(N3,N2,N2,E1[1],E1[2],E1[3],E1[4]);
#endif

/*
C
C	Module 4: Conditional jumps
C
*/
  J = 1;
  for (I = 1; I <= N4; I++) {
    if (J == 1)
      J = 2;
    else
      J = 3;

    if (J > 2)
      J = 0;
    else
      J = 1;

    if (J < 1)
      J = 1;
    else
      J = 0;
  }

#ifdef PRINTOUT
  IF (JJ==II)POUT(N4,J,J,X1,X2,X3,X4);
#endif

/*
C
C	Module 5: Omitted
C 	Module 6: Integer arithmetic
C
*/

  J = 1;
  K = 2;
  L = 3;

  for (I = 1; I <= N6; I++) {
      J = J * (K-J) * (L-K);
      K = L * K - (L-J) * K;
      L = (L-K) * (K+J);
      E1[L-1] = J + K + L;
      E1[K-1] = J * K * L;
  }

#ifdef PRINTOUT
  IF (JJ==II)POUT(N6,J,K,E1[1],E1[2],E1[3],E1[4]);
#endif

/*
C
C	Module 7: Trigonometric functions
C
*/
  X = 0.5;
  Y = 0.5;

  for (I = 1; I <= N7; I++) {
    X = T * DATAN(T2_X*DSIN(X)*DCOS(X)/(DCOS(X+Y)+DCOS(X-Y)-1.0));
    Y = T * DATAN(T2_X*DSIN(Y)*DCOS(Y)/(DCOS(X+Y)+DCOS(X-Y)-1.0));
  }

#ifdef PRINTOUT
  IF (JJ==II)POUT(N7,J,K,X,X,Y,Y);
#endif

/*
C
C	Module 8: Procedure calls
C
*/
  X = 1.0;
  Y = 1.0;
  Z = 1.0;

  for (I = 1; I <= N8; I++)
    P3(X,Y,&Z);

#ifdef PRINTOUT
  IF (JJ==II)POUT(N8,J,K,X,Y,Z,Z);
#endif

/*
C
C	Module 9: Array references
C
*/
  J = 1;
  K = 2;
  L = 3;
  E1[1] = 1.0;
  E1[2] = 2.0;
  E1[3] = 3.0;

  for (I = 1; I <= N9; I++)
    P0();

#ifdef PRINTOUT
  IF (JJ==II)POUT(N9,J,K,E1[1],E1[2],E1[3],E1[4]);
#endif

/*
C
C	Module 10: Integer arithmetic
C
*/
  J = 2;
  K = 3;

  for (I = 1; I <= N10; I++) {
      J = J + K;
      K = J + K;
      J = K - J;
      K = K - J - J;
  }

#ifdef PRINTOUT
  IF (JJ==II)POUT(N10,J,K,X1,X2,X3,X4);
#endif

/*
C
C	Module 11: Standard functions
C
*/
  X = 0.75;

  for (I = 1; I <= N11; I++)
    X = DSQRT(DEXP(DLOG(X)/T1_X));

#ifdef PRINTOUT
  IF (JJ==II)POUT(N11,J,K,X,X,X,X);
#endif

/*
C
C      THIS IS THE END OF THE MAJOR LOOP.
C
*/
  if (++JJ <= II)
    goto IILOOP;

/*
C
C      Stop benchmark timing at this point.
C
*/
  //finisec = time(0);
  finisec = board_millis();

/*
C----------------------------------------------------------------
C      Performance in Whetstone KIP's per second is given by
C
C	(100*LOOP*II)/TIME
C
C      where TIME is in seconds.
C--------------------------------------------------------------------
*/
  if (finisec-startsec <= 0) {
    printf("Insufficient duration- Increase the LOOP count\n");
    return;
  }
  float duration = (finisec-startsec) / 1000.0;

  printf("Loops: %ld, Iterations: %d, Duration %f sec.\n", LOOP, II, duration);

  KIPS = (100.0*LOOP*II)/duration;
  printf("C Converted Double Precision Whetstones: ");
  if (KIPS >= 1000.0) {
    printf ("%.1f MIPS\n", KIPS/1000.0);
  }	else {
    printf ("%.1f KIPS\n", KIPS);
  }
 
}

void
PA(double E[])
{
  J = 0;

L10:
  E[1] = ( E[1] + E[2] + E[3] - E[4]) * T;
  E[2] = ( E[1] + E[2] - E[3] + E[4]) * T;
  E[3] = ( E[1] - E[2] + E[3] + E[4]) * T;
  E[4] = (-E[1] + E[2] + E[3] + E[4]) / T2_X;
  J += 1;

  if (J < 6)
    goto L10;
}

void
P0(void)
{
  E1[J] = E1[K];
  E1[K] = E1[L];
  E1[L] = E1[J];
}

 __attribute__ ((noinline)) void
P3(double X, double Y, double *Z)
{
  double X1, Y1;

  X1 = X;
  Y1 = Y;
  X1 = T * (X1 + Y1);
  Y1 = T * (X1 + Y1);
  *Z  = (X1 + Y1) / T2_X;
}

#ifdef PRINTOUT
void
POUT(long N, long J, long K, double X1, double X2, double X3, double X4)
{
  printf("%7ld: %7ld %7ld %7ld %12.4e %12.4e %12.4e %12.4e\n",
            board_millis(), N, J, K, X1, X2, X3, X4);
}
#endif
