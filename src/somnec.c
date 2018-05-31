/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/* last change:  pgm   8 nov 2000    1:04 pm
   program somnec(input,output,tape21)

   program to generate nec interpolation grids for fields due to
   ground.  field components are computed by numerical evaluation
   of modified sommerfeld integrals.

   somnec2d is a double precision version of somnec for use with
   nec2d.  an alternate version (somnec2sd) is also provided in which
   computation is in single precision but the output file is written
   in double precision for use with nec2d.  somnec2sd runs about twic
   as fast as the full double precision somnec2d.  the difference
   between nec2d results using a for021 file from this code rather
   than from somnec2sd was insignficant in the cases tested.

   changes made by j bergervoet, 31-5-95:
   parameter 0.0 --> 0.0d0 in calling of routine test
   status of output files set to 'unknown' */

#include "somnec.h"
#include "shared.h"

/* common /evlcom/ */
static int jh;
static double ck2, ck2sq, tkmag, tsmag, ck1r, zph, rho;
static complex double ct1, ct2, ct3, ck1, ck1sq, cksm;

/* common /cntour/ */
static complex double a, b;

/*-----------------------------------------------------------------------*/

/* compute integration parameter xlam=lambda from parameter t. */
  static void
lambda( double t, complex double *xlam, complex double *dxlam )
{
  *dxlam=b-a;
  *xlam=a+*dxlam*t;
  return;
}

/*-----------------------------------------------------------------------*/

/* bessel evaluates the zero-order bessel function */
/* and its derivative for complex argument z. */
  static void
bessel( complex double z,
	complex double *j0, complex double *j0p )
{
  int k, ib;
  static int *m = NULL, init = FALSE;
  static double *a1 = NULL, *a2 = NULL;
  double zms;
  complex double p0z, p1z, q0z, q1z, zi, zi2, zk, cz, sz;
  complex double j0x=CPLX_00, j0px=CPLX_00;

  /* initialization of constants */
  if( !init )
  {
	int i;

	size_t mreq = 101 * sizeof(double);
	mem_alloc( (void **)&m, mreq, "in somnec.c");
	mem_alloc( (void **)&a1, mreq, "in somnec.c");
	mem_alloc( (void **)&a2, mreq, "in somnec.c");

	for( k = 1; k <= 25; k++ )
	{
	  i = k-1;
	  a1[i]=-.25/(k*k);
	  a2[i]=1.0/(k+1.0);
	}

	for( i = 1; i <= 101; i++ )
	{
	  double tst=1.0;
	  for( k = 0; k < 24; k++ )
	  {
		init = k;
		tst *= -i*a1[k];
		if( tst < 1.0e-6 )
		  break;
	  }

	  m[i-1] = init+1;
	} /* for( i = 1; i<= 101; i++ ) */

	init = TRUE;
  } /* if(init == 0) */

  zms=creal( z*conj(z) );
  if(zms <= 1.0e-12)
  {
	*j0=CPLX_10;
	*j0p=-.5*z;
	return;
  }

  ib=0;
  if(zms <= 37.21)
  {
	int iz, miz;

	if(zms > 36.0)
	  ib=1;

	/* series expansion */
	iz=(int)zms;
	miz=m[iz];
	*j0=CPLX_10;
	*j0p=*j0;
	zk=*j0;
	zi=z*z;

	for( k = 0; k < miz; k++ )
	{
	  zk *= a1[k]*zi;
	  *j0 += zk;
	  *j0p += a2[k]*zk;
	}
	*j0p *= -.5*z;

	if(ib == 0)
	  return;

	j0x=*j0;
	j0px=*j0p;
  }

  /* asymptotic expansion */
  zi=1.0/z;
  zi2=zi*zi;
  p0z=1.0+(P20*zi2-P10)*zi2;
  p1z=1.0+(P11-P21*zi2)*zi2;
  q0z=(Q20*zi2-Q10)*zi;
  q1z=(Q11-Q21*zi2)*zi;
  zk=cexp(CPLX_01*(z-POF));
  zi2=1.9/zk;
  cz=.5*(zk+zi2);
  sz=CPLX_01*.5*(zi2-zk);
  zk=C3*csqrt(zi);
  *j0=zk*(p0z*cz-q0z*sz);
  *j0p=-zk*(p1z*sz+q1z*cz);

  if(ib == 0)
	return;

  zms=cos((sqrt(zms)-6.0)*PI10);
  *j0=.5*(j0x*(1.0+zms)+ *j0*(1.0-zms));
  *j0p=.5*(j0px*(1.0+zms)+ *j0p*(1.0-zms));

  return;
}

/*-----------------------------------------------------------------------*/

/* hankel evaluates hankel function of the first kind,   */
/* order zero, and its derivative for complex argument z */
  static void
hankel( complex double z,
	complex double *h0, complex double *h0p )
{
  int k, ib;
  static int *m = NULL, init = FALSE;
  static double *a1 = NULL, *a2 = NULL, *a3 = NULL;
  static double *a4, psi, tst, zms;
  complex double clogz, j0, j0p, p0z, p1z, q0z, q1z;
  complex double y0 = CPLX_00, y0p = CPLX_00, zi, zi2, zk;
  static gboolean first_call = TRUE;

  if( first_call )
  {
	first_call = FALSE;
	size_t mreq = 101 * sizeof(int);
	mem_alloc( (void **)&m, mreq, "in somnec.c");
	mreq = 25 * sizeof(double);
	mem_alloc( (void **)&a1, mreq, "in somnec.c");
	mem_alloc( (void **)&a2, mreq, "in somnec.c");
	mem_alloc( (void **)&a3, mreq, "in somnec.c");
	mem_alloc( (void **)&a4, mreq, "in somnec.c");
  }

  /* initialization of constants */
  if( ! init )
  {
	int i;

	psi=-GAMMA;
	for( k = 1; k <= 25; k++ )
	{
	  i = k-1;
	  a1[i]=-0.25/(k*k);
	  a2[i]=1.0/(k+1.0);
	  psi += 1.0/k;
	  a3[i]=psi+psi;
	  a4[i]=(psi+psi+1.0/(k+1.0))/(k+1.0);
	}

	for( i = 1; i <= 101; i++ )
	{
	  tst=1.0;
	  for( k = 0; k < 24; k++ )
	  {
		init = k;
		tst *= -i*a1[k];
		if(tst*a3[k] < 1.0e-6)
		  break;
	  }
	  m[i-1]=init+1;
	}

	init = TRUE;

  } /* if( ! init ) */

  zms=creal( z*conj(z) );
  if(zms == 0.0)
  {
	fprintf( stderr, _("xnec2c: hankel(): Hankel not valid for z = 0\n") );
	Stop( _("hankel(): Hankel not valid for z = 0"), ERR_STOP );
  }

  ib=0;
  if(zms <= 16.81)
  {
	int iz, miz;

	if(zms > 16.0)
	  ib=1;

	/* series expansion */
	iz=(int)zms;
	miz=m[iz];
	j0=CPLX_10;
	j0p=j0;
	y0=CPLX_00;
	y0p=y0;
	zk=j0;
	zi=z*z;

	for( k = 0; k < miz; k++ )
	{
	  zk *= a1[k]*zi;
	  j0 += zk;
	  j0p += a2[k]*zk;
	  y0 += a3[k]*zk;
	  y0p += a4[k]*zk;
	}

	j0p *= -0.5*z;
	clogz=clog(0.5*z);
	y0=(2.0*j0*clogz-y0)/M_PI+C2;
	y0p=(2.0/z+2.0*j0p*clogz+0.5*y0p*z)/M_PI+C1*z;
	*h0=j0+CPLX_01*y0;
	*h0p=j0p+CPLX_01*y0p;

	if(ib == 0) return;

	y0=*h0;
	y0p=*h0p;
  } /* if(zms <= 16.81) */

  /* asymptotic expansion */
  zi=1.0/z;
  zi2=zi*zi;
  p0z=1.0+(P20*zi2-P10)*zi2;
  p1z=1.0+(P11-P21*zi2)*zi2;
  q0z=(Q20*zi2-Q10)*zi;
  q1z=(Q11-Q21*zi2)*zi;
  zk=cexp(CPLX_01*(z-POF))*csqrt(zi)*C3;
  *h0=zk*(p0z+CPLX_01*q0z);
  *h0p=CPLX_01*zk*(p1z+CPLX_01*q1z);

  if(ib == 0) return;

  zms=cos((sqrt(zms)-4.0)*31.41592654);
  *h0=0.5*(y0*(1.0+zms)+ *h0*(1.0-zms));
  *h0p=0.5*(y0p*(1.0+zms)+ *h0p*(1.0-zms));

  return;
}

/*-----------------------------------------------------------------------*/

/* saoa computes the integrand for each of the 6 sommerfeld */
/* integrals for source and observer above ground */
  static void
saoa( double t, complex double *ans)
{
  double xlr;
  static complex double xl, dxl, cgam1, cgam2, b0;
  static complex double b0p, com, dgam, den1, den2;

  lambda(t, &xl, &dxl);
  if( jh == 0 )
  {
	/* bessel function form */
	bessel(xl*rho, &b0, &b0p);
	b0  *=2.0;
	b0p *=2.0;
	cgam1=csqrt(xl*xl-ck1sq);
	cgam2=csqrt(xl*xl-ck2sq);
	if(creal(cgam1) == 0.0)
	  cgam1=cmplx(0.0,-fabs(cimag(cgam1)));
	if(creal(cgam2) == 0.0)
	  cgam2=cmplx(0.0,-fabs(cimag(cgam2)));
  }
  else
  {
	/* hankel function form */
	hankel(xl*rho, &b0, &b0p);
	com=xl-ck1;
	cgam1=csqrt(xl+ck1)*csqrt(com);
	if(creal(com) < 0.0 && cimag(com) >= 0.0)
	  cgam1=-cgam1;
	com=xl-ck2;
	cgam2=csqrt(xl+ck2)*csqrt(com);
	if(creal(com) < 0.0 && cimag(com) >= 0.0)
	  cgam2=-cgam2;
  }

  xlr=creal( xl*conj(xl) );
  if(xlr >= tsmag)
  {
	double sign;
	if(cimag(xl) >= 0.0)
	{
	  xlr=creal(xl);
	  if(xlr >= ck2)
	  {
		if(xlr <= ck1r)
		  dgam=cgam2-cgam1;
		else
		{
		  sign=1.0;
		  dgam=1.0/(xl*xl);
		  dgam=sign*((ct3*dgam+ct2)*dgam+ct1)/xl;
		}
	  }
	  else
	  {
		sign=-1.0;
		dgam=1.0/(xl*xl);
		dgam=sign*((ct3*dgam+ct2)*dgam+ct1)/xl;
	  } /* if(xlr >= ck2) */

	} /* if(cimag(xl) >= 0.0) */
	else
	{
	  sign=1.0;
	  dgam=1.0/(xl*xl);
	  dgam=sign*((ct3*dgam+ct2)*dgam+ct1)/xl;
	}

  } /* if(xlr < tsmag) */
  else dgam=cgam2-cgam1;

  den2=cksm*dgam/(cgam2*(ck1sq*cgam2+ck2sq*cgam1));
  den1=1.0/(cgam1+cgam2)-cksm/cgam2;
  com=dxl*xl*cexp(-cgam2*zph);
  ans[5]=com*b0*den1/ck1;
  com *= den2;

  if(rho != 0.0)
  {
	b0p=b0p/rho;
	ans[0]=-com*xl*(b0p+b0*xl);
	ans[3]=com*xl*b0p;
  }
  else
  {
	ans[0]=-com*xl*xl*0.5;
	ans[3]=ans[0];
  }

  ans[1]=com*cgam2*cgam2*b0;
  ans[2]=-ans[3]*cgam2*rho;
  ans[4]=com*b0;

  return;
}

/*-----------------------------------------------------------------------*/

/* rom1 integrates the 6 sommerfeld integrals from a to b in lambda. */
/* the method of variable interval width romberg integration is used. */
  static void
rom1( int n, complex double *sum, int nx )
{
  int jump, lstep, nogo, i, ns, nt;
  static double z, ze, s, ep, zend, dz=0.0, dzot=0.0, tr, ti;
  static complex double t00, t11, t02;
  static complex double *g1 = NULL, *g2 = NULL;
  static complex double *g3 = NULL, *g4 = NULL;
  static complex double *g5 = NULL, *t01 = NULL;
  static complex double *t10 = NULL, *t20 = NULL;
  static gboolean first_call = TRUE;

  if( first_call )
  {
	first_call = FALSE;
	size_t mreq = 6 * sizeof(complex double);
	mem_alloc( (void **)&g1, mreq, "in somnec.c");
	mem_alloc( (void **)&g2, mreq, "in somnec.c");
	mem_alloc( (void **)&g3, mreq, "in somnec.c");
	mem_alloc( (void **)&g4, mreq, "in somnec.c");
	mem_alloc( (void **)&g5, mreq, "in somnec.c");
	mem_alloc( (void **)&t01, mreq, "in somnec.c");
	mem_alloc( (void **)&t10, mreq, "in somnec.c");
	mem_alloc( (void **)&t20, mreq, "in somnec.c");
  }

  lstep=0;
  z=0.0;
  ze=1.0;
  s=1.0;
  ep=s/(1.0e4*NM);
  zend=ze-ep;
  for( i = 0; i < n; i++ )
	sum[i]=CPLX_00;
  ns=nx;
  nt=0;
  saoa(z,g1);

  jump = FALSE;
  while( TRUE )
  {
	if( ! jump )
	{
	  dz=s/ns;
	  if( (z+dz) > ze )
	  {
		dz=ze-z;
		if( dz <= ep ) return;
	  }

	  dzot=dz*.5;
	  saoa(z+dzot,g3);
	  saoa(z+dz,g5);

	} /* if( ! jump ) */

	nogo=FALSE;
	for( i = 0; i < n; i++ )
	{
	  t00=(g1[i]+g5[i])*dzot;
	  t01[i]=(t00+dz*g3[i])*.5;
	  t10[i]=(4.0*t01[i]-t00)/3.0;

	  /* test convergence of 3 point romberg result */
	  test( creal(t01[i]), creal(t10[i]), &tr,
		  cimag(t01[i]), cimag(t10[i]), &ti, 0.0 );
	  if( (tr > CRIT) || (ti > CRIT) )
		nogo = TRUE;
	}

	if( ! nogo )
	{
	  for( i = 0; i < n; i++ )
		sum[i] += t10[i];

	  nt += 2;
	  z += dz;
	  if(z > zend) return;

	  for( i = 0; i < n; i++ )
		g1[i]=g5[i];

	  if( (nt >= NTS) && (ns > nx) )
	  {
		ns=ns/2;
		nt=1;
	  }

	  jump = FALSE;
	  continue;

	} /* if( ! nogo ) */

	saoa(z+dz*.250,g2);
	saoa(z+dz*.75,g4);
	nogo=FALSE;
	for( i = 0; i < n; i++ )
	{
	  t02=(t01[i]+dzot*(g2[i]+g4[i]))*0.50;
	  t11=(4.0*t02-t01[i])/3.0;
	  t20[i]=(16.0*t11-t10[i])/15.0;

	  /* test convergence of 5 point romberg result */
	  test( creal(t11), creal(t20[i]), &tr,
		  cimag(t11), cimag(t20[i]), &ti, 0.0 );
	  if( (tr > CRIT) || (ti > CRIT) )
		nogo = TRUE;
	}

	if( ! nogo )
	{
	  for( i = 0; i < n; i++ )
		sum[i] += t20[i];

	  nt++;
	  z += dz;
	  if(z > zend) return;

	  for( i = 0; i < n; i++ )
		g1[i]=g5[i];

	  if( (nt >= NTS) && (ns > nx) )
	  {
		ns=ns/2;
		nt=1;
	  }

	  jump = FALSE;
	  continue;

	} /* if( ! nogo ) */

	nt=0;
	if(ns < NM)
	{
	  ns *= 2;
	  dz=s/ns;
	  dzot=dz*0.50;

	  for( i = 0; i < n; i++ )
	  {
		g5[i]=g3[i];
		g3[i]=g2[i];
	  }

	  jump = TRUE;
	  continue;

	} /* if(ns < nm) */

	if( ! lstep )
	{
	  lstep = TRUE;
	  lambda( z, &t00, &t11 );
	}

	for( i = 0; i < n; i++ )
	  sum[i] += t20[i];

	nt++;
	z += dz;
	if(z > zend) return;

	for( i = 0; i < n; i++ )
	  g1[i]=g5[i];

	jump = FALSE;

  } /* while( TRUE ) */

}

/*-----------------------------------------------------------------------*/

/* gshank integrates the 6 sommerfeld integrals from start to */
/* infinity (until convergence) in lambda.  at the break point, bk, */
/* the step increment may be changed from dela to delb.  shank's */
/* algorithm to accelerate convergence of a slowly converging series */
/* is used */
  static void
gshank( complex double start, complex double dela,
	complex double *sum, int nans, complex double *seed,
	int ibk, complex double bk, complex double delb )
{
  int ibx, j, i, jm, intx, inx, brk=0, idx;
  static double rbk, amg, den, denm;
  complex double a1, a2, as1, as2, del, aa;
  static complex double *q1 = NULL, *q2 = NULL;
  static complex double *ans1 = NULL, *ans2 = NULL;
  static gboolean first_call = TRUE;

  if( first_call )
  {
	first_call = FALSE;
	size_t mreq = 6 * 20 * sizeof(complex double);
	mem_alloc( (void **)&q1, mreq, "in somnec.c");
	mem_alloc( (void **)&q2, mreq, "in somnec.c");
	mreq = 6 * sizeof(complex double);
	mem_alloc( (void **)&ans1, mreq, "in somnec.c");
	mem_alloc( (void **)&ans2, mreq, "in somnec.c");
  }

  rbk=creal(bk);
  del=dela;
  if(ibk == 0)
	ibx=1;
  else
	ibx=0;

  for( i = 0; i < nans; i++ )
	ans2[i]=seed[i];

  b=start;
  for( intx = 1; intx <= MAXH; intx++ )
  {
	inx=intx-1;
	a=b;
	b += del;

	if( (ibx == 0) && (creal(b) >= rbk) )
	{
	  /* hit break point.  reset seed and start over. */
	  ibx=1;
	  b=bk;
	  del=delb;
	  rom1(nans,sum,2);
	  for( i = 0; i < nans; i++ )
		ans2[i] += sum[i];
	  intx = 0;
	  continue;
	} /* if( (ibx == 0) && (creal(b) >= rbk) ) */

	rom1(nans,sum,2);
	for( i = 0; i < nans; i++ )
	  ans1[i] = ans2[i]+sum[i];
	a=b;
	b += del;

	if( (ibx == 0) && (creal(b) >= rbk) )
	{
	  /* hit break point.  reset seed and start over. */
	  ibx=2;
	  b=bk;
	  del=delb;
	  rom1(nans,sum,2);
	  for( i = 0; i < nans; i++ )
		ans2[i] = ans1[i]+sum[i];
	  intx = 0;
	  continue;

	} /* if( (ibx == 0) && (creal(b) >= rbk) ) */

	rom1(nans,sum,2);
	for( i = 0; i < nans; i++ )
	  ans2[i]=ans1[i]+sum[i];

	den=0.0;
	for( i = 0; i < nans; i++ )
	{
	  as1=ans1[i];
	  as2=ans2[i];

	  if(intx >= 2)
	  {
		for( j = 1; j < intx; j++ )
		{
		  jm=j-1;
		  idx = i + 6*jm;
		  aa=q2[idx];
		  a1=q1[idx]+as1-2.0*aa;

		  if( (creal(a1) != 0.0) || (cimag(a1) != 0.0) )
		  {
			a2=aa-q1[idx];
			a1=q1[idx]-a2*a2/a1;
		  }
		  else a1=q1[idx];

		  a2=aa+as2-2.0*as1;
		  if( (creal(a2) != 0.0) || (cimag(a2) != 0.0) )
			a2=aa-(as1-aa)*(as1-aa)/a2;
		  else
			a2=aa;

		  q1[idx]=as1;
		  q2[idx]=as2;
		  as1=a1;
		  as2=a2;

		} /* for( j = 1; i < intx; i++ ) */

	  } /* if(intx >= 2) */

	  idx = i + 6*(intx-1);
	  q1[idx]=as1;
	  q2[idx]=as2;
	  amg=fabs(creal(as2))+fabs(cimag(as2));
	  if(amg > den)
		den=amg;

	} /* for( i = 0; i < nans; i++ ) */

	denm=1.0e-3*den*CRIT;
	jm=intx-3;
	if(jm < 1)
	  jm=1;

	for( j = jm-1; j < intx; j++ )
	{
	  brk = FALSE;
	  for( i = 0; i < nans; i++ )
	  {
		idx = i +6*j;
		a1=q2[idx];
		den=(fabs(creal(a1))+fabs(cimag(a1)))*CRIT;
		if(den < denm)
		  den=denm;
		a1=q1[idx]-a1;
		amg=fabs(creal(a1)+fabs(cimag(a1)));
		if(amg > den)
		{
		  brk = TRUE;
		  break;
		}

	  } /* for( i = 0; i < nans; i++ ) */

	  if( brk ) break;

	} /* for( j = jm-1; j < intx; j++ ) */

	if( ! brk )
	{
	  for( i = 0; i < nans; i++ )
	  {
		idx = i + 6*inx;
		sum[i]=0.5*(q1[idx]+q2[idx]);
	  }
	  return;
	}

  } /* for( intx = 1; intx <= maxh; intx++ ) */

  /* No convergence */
  fprintf( stderr, _("xnec2c: gshank(): No convergence\n") );
  Stop( _("gshank(): No convergencn"), ERR_STOP );
  return;
}

/*-----------------------------------------------------------------------*/

/* evlua controls the integration contour in the complex */
/* lambda plane for evaluation of the sommerfeld integrals */
  static void
evlua( complex double *erv, complex double *ezv,
	complex double *erh, complex double *eph )
{
  int i, jump;
  static double del, slope, rmis;
  static complex double cp1, cp2, cp3, bk, delta, delta2;
  static complex double *sum = NULL, *ans = NULL;
  static gboolean first_call = TRUE;

  if( first_call )
  {
	first_call = FALSE;
	size_t mreq = 6 * sizeof(complex double);
	mem_alloc( (void **)&sum, mreq, "in somnec.c");
	mem_alloc( (void **)&ans, mreq, "in somnec.c");
  }

  del=zph;
  if( rho > del )
	del=rho;

  if(zph >= 2.0*rho)
  {
	/* bessel function form of sommerfeld integrals */
	jh=0;
	a=CPLX_00;
	del=1.0/del;

	if( del > tkmag)
	{
	  b=cmplx(0.1*tkmag,-0.1*tkmag);
	  rom1(6,sum,2);
	  a=b;
	  b=cmplx(del,-del);
	  rom1 (6,ans,2);
	  for( i = 0; i < 6; i++ )
		sum[i] += ans[i];
	}
	else
	{
	  b=cmplx(del,-del);
	  rom1(6,sum,2);
	}

	delta=PTP*del;
	gshank(b,delta,ans,6,sum,0,b,b);
	ans[5] *= ck1;

	/* conjugate since nec uses exp(+jwt) */
	*erv=conj(ck1sq*ans[2]);
	*ezv=conj(ck1sq*(ans[1]+ck2sq*ans[4]));
	*erh=conj(ck2sq*(ans[0]+ans[5]));
	*eph=-conj(ck2sq*(ans[3]+ans[5]));

	return;
  } /* if(zph >= 2.0*rho) */

  /* hankel function form of sommerfeld integrals */
  jh=1;
  cp1=cmplx(0.0, 0.4*ck2);
  cp2=cmplx(0.6*ck2, -0.2*ck2);
  cp3=cmplx(1.02*ck2,-0.2*ck2);
  a=cp1;
  b=cp2;
  rom1(6,sum,2);
  a=cp2;
  b=cp3;
  rom1(6,ans,2);

  for( i = 0; i < 6; i++ )
	sum[i]=-(sum[i]+ans[i]);

  /* path from imaginary axis to -infinity */
  if(zph > .001*rho)
	slope=rho/zph;
  else
	slope=1000.0;

  del=PTP/del;
  delta=cmplx(-1.0,slope)*del/sqrt(1.0+slope*slope);
  delta2=-conj(delta);
  gshank(cp1,delta,ans,6,sum,0,bk,bk);
  rmis=rho*(creal(ck1)-ck2);

  jump = FALSE;
  if( (rmis >= 2.0*ck2) && (rho >= 1.0e-10) )
  {
	if(zph >= 1.0e-10)
	{
	  bk=cmplx(-zph,rho)*(ck1-cp3);
	  rmis=-creal(bk)/fabs(cimag(bk));
	  if(rmis > 4.0*rho/zph)
		jump = TRUE;
	}

	if( ! jump )
	{
	  /* integrate up between branch cuts, then to + infinity */
	  cp1=ck1-(0.1+I*0.2);
	  cp2=cp1+0.2;
	  bk=cmplx(0.0,del);
	  gshank(cp1,bk,sum,6,ans,0,bk,bk);
	  a=cp1;
	  b=cp2;
	  rom1(6,ans,1);
	  for( i = 0; i < 6; i++ )
		ans[i] -= sum[i];

	  gshank(cp3,bk,sum,6,ans,0,bk,bk);
	  gshank(cp2,delta2,ans,6,sum,0,bk,bk);
	}

	jump = TRUE;

  } /* if( (rmis >= 2.0*ck2) || (rho >= 1.0e-10) ) */
  else jump = FALSE;

  if( ! jump )
  {
	/* integrate below branch points, then to + infinity */
	for( i = 0; i < 6; i++ )
	  sum[i]=-ans[i];

	rmis=creal(ck1)*1.01;
	if( (ck2+1.0) > rmis )
	  rmis=ck2+1.0;

	bk=cmplx(rmis,0.99*cimag(ck1));
	delta=bk-cp3;
	delta *= del/cabs(delta);
	gshank(cp3,delta,ans,6,sum,1,bk,delta2);

  } /* if( ! jump ) */

  ans[5] *= ck1;

  /* conjugate since nec uses exp(+jwt) */
  *erv=conj(ck1sq*ans[2]);
  *ezv=conj(ck1sq*(ans[1]+ck2sq*ans[4]));
  *erh=conj(ck2sq*(ans[0]+ans[5]));
  *eph=-conj(ck2sq*(ans[3]+ans[5]));

  return;
}

/*-----------------------------------------------------------------------*/

/* This is the "main" of somnec */
  void
somnec( double epr, double sig, double fmhz )
{
  int k, nth, ith, irs, ir, nr;
  double wlam, dr, dth=0.0, r, rk, thet, tfac1, tfac2;
  complex double erv, ezv, erh, eph, cl1, cl2, con;

  static gboolean first_call = TRUE;

  if( first_call )
  {
	first_call = FALSE;

	/* Allocate some buffers */
	size_t mreq = sizeof(complex double) * 11 * 10 * 4;
	mem_alloc( (void **)&ggrid.ar1, mreq, "in somnec.c");
	mreq = sizeof(complex double) * 17 * 5 * 4;
	mem_alloc( (void **)&ggrid.ar2, mreq, "in somnec.c");
	mreq = sizeof(complex double) * 9 * 8 * 4;
	mem_alloc( (void **)&ggrid.ar3, mreq, "in somnec.c");

	mreq = sizeof(int) * 3;
	mem_alloc( (void **)&ggrid.nxa, mreq, "in somnec.c");
	mem_alloc( (void **)&ggrid.nya, mreq, "in somnec.c");

	mreq = sizeof(double) * 3;
	mem_alloc( (void **)&ggrid.dxa, mreq, "in somnec.c");
	mem_alloc( (void **)&ggrid.dya, mreq, "in somnec.c");
	mem_alloc( (void **)&ggrid.xsa, mreq, "in somnec.c");
	mem_alloc( (void **)&ggrid.ysa, mreq, "in somnec.c");

	/* Initialize ground grid parameters for somnec */
	ggrid.nxa[0] = 11;
	ggrid.nxa[1] = 17;
	ggrid.nxa[2] = 9;

	ggrid.nya[0] = 10;
	ggrid.nya[1] = 5;
	ggrid.nya[2] = 8;

	ggrid.dxa[0] = .02;
	ggrid.dxa[1] = .05;
	ggrid.dxa[2] = .1;

	ggrid.dya[0] = .1745329252;
	ggrid.dya[1] = .0872664626;
	ggrid.dya[2] = .1745329252;

	ggrid.xsa[0] = 0.0;
	ggrid.xsa[1] = .2;
	ggrid.xsa[2] = .2;

	ggrid.ysa[0] = 0.0;
	ggrid.ysa[1] = 0.0;
	ggrid.ysa[2] = .3490658504;

  } /* if( first_call ) */

  if(sig >= 0.0)
  {
	wlam=CVEL/fmhz;
	ggrid.epscf=cmplx(epr,-sig*wlam*59.96);
  }
  else ggrid.epscf=cmplx(epr,sig);

  ck2=M_2PI;
  ck2sq=ck2*ck2;

  /* sommerfeld integral evaluation uses exp(-jwt),
   * nec uses exp(+jwt), hence need conjg(ggrid.epscf).
   * conjugate of fields occurs in subroutine evlua. */

  ck1sq=ck2sq*conj(ggrid.epscf);
  ck1=csqrt(ck1sq);
  ck1r=creal(ck1);
  tkmag=100.0*cabs(ck1);
  tsmag=100.0*creal( ck1*conj(ck1) );
  cksm=ck2sq/(ck1sq+ck2sq);
  ct1=.5*(ck1sq-ck2sq);
  erv=ck1sq*ck1sq;
  ezv=ck2sq*ck2sq;
  ct2=.125*(erv-ezv);
  erv *= ck1sq;
  ezv *= ck2sq;
  ct3=.0625*(erv-ezv);

  /* loop over 3 grid regions */
  for( k = 0; k < 3; k++ )
  {
	nr=ggrid.nxa[k];
	nth=ggrid.nya[k];
	dr=ggrid.dxa[k];
	dth=ggrid.dya[k];
	r=ggrid.xsa[k]-dr;
	irs=1;
	if(k == 0)
	{
	  r=ggrid.xsa[k];
	  irs=2;
	}

	/*  loop over r.  (r=sqrt(rho**2 + (z+h)**2)) */
	for( ir = irs-1; ir < nr; ir++ )
	{
	  r += dr;
	  thet = ggrid.ysa[k]-dth;

	  /* loop over theta.  (theta=atan((z+h)/rho)) */
	  for( ith = 0; ith < nth; ith++ )
	  {
		thet += dth;
		rho=r*cos(thet);
		zph=r*sin(thet);
		if(rho < 1.0e-7)
		  rho=1.0e-8;
		if(zph < 1.0e-7)
		  zph=0.0;

		evlua( &erv, &ezv, &erh, &eph );

		rk=ck2*r;
		con=-CONST1*r/cmplx(cos(rk),-sin(rk));

		switch( k )
		{
		  case 0:
			ggrid.ar1[ir+ith*11+  0]=erv*con;
			ggrid.ar1[ir+ith*11+110]=ezv*con;
			ggrid.ar1[ir+ith*11+220]=erh*con;
			ggrid.ar1[ir+ith*11+330]=eph*con;
			break;

		  case 1:
			ggrid.ar2[ir+ith*17+  0]=erv*con;
			ggrid.ar2[ir+ith*17+ 85]=ezv*con;
			ggrid.ar2[ir+ith*17+170]=erh*con;
			ggrid.ar2[ir+ith*17+255]=eph*con;
			break;

		  case 2:
			ggrid.ar3[ir+ith*9+  0]=erv*con;
			ggrid.ar3[ir+ith*9+ 72]=ezv*con;
			ggrid.ar3[ir+ith*9+144]=erh*con;
			ggrid.ar3[ir+ith*9+216]=eph*con;

		} /* switch( k ) */

	  } /* for( ith = 0; ith < nth; ith++ ) */

	} /* for( ir = irs-1; ir < nr; ir++; ) */

  } /* for( k = 0; k < 3; k++; ) */

  /* fill grid 1 for r equal to zero. */
  cl2=-CONST4*(ggrid.epscf-1.0)/(ggrid.epscf+1.0);
  cl1=cl2/(ggrid.epscf+1.0);
  ezv=ggrid.epscf*cl1;
  thet=-dth;
  nth=ggrid.nya[0];

  for( ith = 0; ith < nth; ith++ )
  {
	thet += dth;
	if( (ith+1) != nth )
	{
	  tfac2=cos(thet);
	  tfac1=(1.0-sin(thet))/tfac2;
	  tfac2=tfac1/tfac2;
	  erv=ggrid.epscf*cl1*tfac1;
	  erh=cl1*(tfac2-1.0)+cl2;
	  eph=cl1*tfac2-cl2;
	}
	else
	{
	  erv=0.0;
	  erh=cl2-.5*cl1;
	  eph=-erh;
	}

	ggrid.ar1[0+ith*11+  0]=erv;
	ggrid.ar1[0+ith*11+110]=ezv;
	ggrid.ar1[0+ith*11+220]=erh;
	ggrid.ar1[0+ith*11+330]=eph;
  }

  return;
}

/*-----------------------------------------------------------------------*/

/* fbar is sommerfeld attenuation function for numerical distance p */
  void
fbar( complex double p, complex double *fbar )
{
  int i, minus;
  double tms, sms;
  complex double z, zs, sum, pow, term;

  z= CPLX_01* csqrt( p);
  if( cabs( z) <= 3.0)
  {
	/* series expansion */
	zs= z* z;
	sum= z;
	pow= z;

	for( i = 1; i <= 100; i++ )
	{
	  pow= -pow* zs/ (double)i;
	  term= pow/(2.0* i+1.0);
	  sum= sum+ term;
	  tms= creal( term* conj( term));
	  sms= creal( sum* conj( sum));
	  if( tms/sms < ACCS)
		break;
	}

	*fbar=1.0-(1.0- sum* TOSP)* z* cexp( zs)* SP;

  } /* if( cabs( z) <= 3.0) */

  /* asymptotic expansion */
  if( creal( z) < 0.0)
  {
	minus=1;
	z= -z;
  }
  else minus=0;

  zs=0.5/( z* z);
  sum=CPLX_00;
  term=CPLX_10;

  for( i = 1; i <= 6; i++ )
  {
	term = -term*(2.0*i -1.0)* zs;
	sum += term;
  }

  if( minus == 1)
	sum -= 2.0* SP* z* cexp( z* z);
  *fbar= -sum;

}

/*-----------------------------------------------------------------------*/

