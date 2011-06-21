/*
 *  xnec2c - GTK2-based version of nec2c, the C translation of NEC2
 *  Copyright (C) 2003-2010 N. Kyriazis neoklis.kyriazis(at)gmail.com
 *
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

   somnec2d is a long double precision version of somnec for use with
   nec2d.  an alternate version (somnec2sd) is also provided in which
   computation is in single precision but the output file is written
   in long double precision for use with nec2d.  somnec2sd runs about twic
   as fast as the full long double precision somnec2d.  the difference
   between nec2d results using a for021 file from this code rather
   than from somnec2sd was insignficant in the cases tested.

   changes made by j bergervoet, 31-5-95:
   parameter 0.0l --> 0.0d0 in calling of routine test
   status of output files set to 'unknown' */

#include "xnec2c.h"

/* common /evlcom/ */
static int jh;
static long double ck2, ck2sq, tkmag, tsmag, ck1r, zph, rho;
static complex long double ct1, ct2, ct3, ck1, ck1sq, cksm;

/* common /cntour/ */
static complex long double a, b;

/*common  /ggrid/ */
ggrid_t ggrid;

/*-----------------------------------------------------------------------*/

/* This is the "main" of somnec */
void somnec( long double epr, long double sig, long double fmhz )
{
  int k, nth, ith, irs, ir, nr;
  long double wlam, dr, dth, r, rk, thet, tfac1, tfac2;
  complex long double erv, ezv, erh, eph, cl1, cl2, con;

  static gboolean first_call = TRUE;

  if( first_call )
  {
	first_call = FALSE;

	/* Allocate some buffers */
	mem_alloc( (void *)&ggrid.ar1,
		sizeof(complex long double)*11*10*4, "in somnec.c");
	mem_alloc( (void *)&ggrid.ar2,
		sizeof(complex long double)*17*5*4, "in somnec.c");
	mem_alloc( (void *)&ggrid.ar3,
		sizeof(complex long double)*9*8*4, "in somnec.c");
	mem_alloc( (void *)&ggrid.nxa,
		sizeof(int)*3, "in somnec.c");
	mem_alloc( (void *)&ggrid.nya,
		sizeof(int)*3, "in somnec.c");
	mem_alloc( (void *)&ggrid.dxa,
		sizeof(long double)*3, "in somnec.c");
	mem_alloc( (void *)&ggrid.dya,
		sizeof(long double)*3, "in somnec.c");
	mem_alloc( (void *)&ggrid.xsa,
		sizeof(long double)*3, "in somnec.c");
	mem_alloc( (void *)&ggrid.ysa,
		sizeof(long double)*3, "in somnec.c");

	/* Initialize ground grid parameters for somnec */
	ggrid.nxa[0] = 11;
	ggrid.nxa[1] = 17;
	ggrid.nxa[2] = 9;

	ggrid.nya[0] = 10;
	ggrid.nya[1] = 5;
	ggrid.nya[2] = 8;

	ggrid.dxa[0] = .02l;
	ggrid.dxa[1] = .05l;
	ggrid.dxa[2] = .1l;

	ggrid.dya[0] = .1745329252l;
	ggrid.dya[1] = .0872664626l;
	ggrid.dya[2] = .1745329252l;

	ggrid.xsa[0] = 0.0l;
	ggrid.xsa[1] = .2l;
	ggrid.xsa[2] = .2l;

	ggrid.ysa[0] = 0.0l;
	ggrid.ysa[1] = 0.0l;
	ggrid.ysa[2] = .3490658504l;

  } /* if( first_call ) */

  if(sig >= 0.0l)
  {
	wlam=CVEL/fmhz;
	ggrid.epscf=cmplx(epr,-sig*wlam*59.96l);
  }
  else
	ggrid.epscf=cmplx(epr,sig);

  ck2=TP;
  ck2sq=ck2*ck2;

  /* sommerfeld integral evaluation uses exp(-jwt),
   * nec uses exp(+jwt), hence need conjg(ggrid.epscf).
   * conjugate of fields occurs in subroutine evlua. */

  ck1sq=ck2sq*conj(ggrid.epscf);
  ck1=csqrtl(ck1sq);
  ck1r=creal(ck1);
  tkmag=100.0l*cabs(ck1);
  tsmag=100.0l*ck1*conj(ck1);
  cksm=ck2sq/(ck1sq+ck2sq);
  ct1=.5l*(ck1sq-ck2sq);
  erv=ck1sq*ck1sq;
  ezv=ck2sq*ck2sq;
  ct2=.125l*(erv-ezv);
  erv *= ck1sq;
  ezv *= ck2sq;
  ct3=.0625l*(erv-ezv);

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

	/*  loop over r.  (r=sqrtl(rho**2 + (z+h)**2)) */
	for( ir = irs-1; ir < nr; ir++ )
	{
	  r += dr;
	  thet = ggrid.ysa[k]-dth;

	  /* loop over theta.  (theta=atan((z+h)/rho)) */
	  for( ith = 0; ith < nth; ith++ )
	  {
		thet += dth;
		rho=r*cosl(thet);
		zph=r*sinl(thet);
		if(rho < 1.0e-7l)
		  rho=1.0e-8l;
		if(zph < 1.0e-7l)
		  zph=0.0l;

		evlua( &erv, &ezv, &erh, &eph );

		rk=ck2*r;
		con=-CONST1*r/cmplx(cosl(rk),-sinl(rk));

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
  cl2=-CONST4*(ggrid.epscf-1.0l)/(ggrid.epscf+1.0l);
  cl1=cl2/(ggrid.epscf+1.0l);
  ezv=ggrid.epscf*cl1;
  thet=-dth;
  nth=ggrid.nya[0];

  for( ith = 0; ith < nth; ith++ )
  {
	thet += dth;
	if( (ith+1) != nth )
	{
	  tfac2=cosl(thet);
	  tfac1=(1.0l-sinl(thet))/tfac2;
	  tfac2=tfac1/tfac2;
	  erv=ggrid.epscf*cl1*tfac1;
	  erh=cl1*(tfac2-1.0l)+cl2;
	  eph=cl1*tfac2-cl2;
	}
	else
	{
	  erv=0.0l;
	  erh=cl2-.5l*cl1;
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

/* bessel evaluates the zero-order bessel function */
/* and its derivative for complex argument z. */
void bessel( complex long double z,
	complex long double *j0, complex long double *j0p )
{
  int k, i, ib, iz, miz;
  static int *m = NULL, init = FALSE;
  static long double *a1 = NULL, *a2 = NULL;
  long double tst, zms;
  complex long double p0z, p1z, q0z, q1z, zi, zi2, zk, cz, sz;
  complex long double j0x=CPLX_00, j0px=CPLX_00;

  /* initialization of constants */
  if( !init )
  {
	mem_alloc( (void *)&m,
		101*sizeof(long double), "in somnec.c");
	mem_alloc( (void *)&a1,
		101*sizeof(long double), "in somnec.c");
	mem_alloc( (void *)&a2,
		101*sizeof(long double), "in somnec.c");

	for( k = 1; k <= 25; k++ )
	{
	  i = k-1;
	  a1[i]=-.25l/(k*k);
	  a2[i]=1.0l/(k+1.0l);
	}

	for( i = 1; i <= 101; i++ )
	{
	  tst=1.0l;
	  for( k = 0; k < 24; k++ )
	  {
		init = k;
		tst *= -i*a1[k];
		if( tst < 1.0e-6l )
		  break;
	  }

	  m[i-1] = init+1;
	} /* for( i = 1; i<= 101; i++ ) */

	init = TRUE;
  } /* if(init == 0) */

  zms=z*conj(z);
  if(zms <= 1.0e-12l)
  {
	*j0=CPLX_10;
	*j0p=-.5l*z;
	return;
  }

  ib=0;
  if(zms <= 37.21l)
  {
	if(zms > 36.0l)
	  ib=1;

	/* series expansion */
	iz=zms;
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
	*j0p *= -.5l*z;

	if(ib == 0)
	  return;

	j0x=*j0;
	j0px=*j0p;
  }

  /* asymptotic expansion */
  zi=1.0l/z;
  zi2=zi*zi;
  p0z=1.0l+(P20*zi2-P10)*zi2;
  p1z=1.0l+(P11-P21*zi2)*zi2;
  q0z=(Q20*zi2-Q10)*zi;
  q1z=(Q11-Q21*zi2)*zi;
  zk=cexp(CPLX_01*(z-POF));
  zi2=1.9l/zk;
  cz=.5l*(zk+zi2);
  sz=CPLX_01*.5l*(zi2-zk);
  zk=C3*csqrtl(zi);
  *j0=zk*(p0z*cz-q0z*sz);
  *j0p=-zk*(p1z*sz+q1z*cz);

  if(ib == 0)
	return;

  zms=cosl((sqrtl(zms)-6.0l)*PI10);
  *j0=.5l*(j0x*(1.0l+zms)+ *j0*(1.0l-zms));
  *j0p=.5l*(j0px*(1.0l+zms)+ *j0p*(1.0l-zms));

  return;
}

/*-----------------------------------------------------------------------*/

/* evlua controls the integration contour in the complex */
/* lambda plane for evaluation of the sommerfeld integrals */
void evlua( complex long double *erv, complex long double *ezv,
	complex long double *erh, complex long double *eph )
{
  int i, jump;
  static long double del, slope, rmis;
  static complex long double cp1, cp2, cp3, bk, delta, delta2;
  static complex long double *sum = NULL, *ans = NULL;
  static gboolean first_call = TRUE;

  if( first_call )
  {
	first_call = FALSE;
	mem_alloc( (void *)&sum,
		6*sizeof(complex long double), "in somnec.c");
	mem_alloc( (void *)&ans,
		6*sizeof(complex long double), "in somnec.c");
  }

  del=zph;
  if( rho > del )
	del=rho;

  if(zph >= 2.0l*rho)
  {
	/* bessel function form of sommerfeld integrals */
	jh=0;
	a=CPLX_00;
	del=1.0l/del;

	if( del > tkmag)
	{
	  b=cmplx(0.1l*tkmag,-0.1l*tkmag);
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

  } /* if(zph >= 2.0l*rho) */

  /* hankel function form of sommerfeld integrals */
  jh=1;
  cp1=cmplx(0.0l, 0.4l*ck2);
  cp2=cmplx(0.6l*ck2, -0.2l*ck2);
  cp3=cmplx(1.02l*ck2,-0.2l*ck2);
  a=cp1;
  b=cp2;
  rom1(6,sum,2);
  a=cp2;
  b=cp3;
  rom1(6,ans,2);

  for( i = 0; i < 6; i++ )
	sum[i]=-(sum[i]+ans[i]);

  /* path from imaginary axis to -infinity */
  if(zph > .001l*rho)
	slope=rho/zph;
  else
	slope=1000.0l;

  del=PTP/del;
  delta=cmplx(-1.0l,slope)*del/sqrtl(1.0l+slope*slope);
  delta2=-conj(delta);
  gshank(cp1,delta,ans,6,sum,0,bk,bk);
  rmis=rho*(creal(ck1)-ck2);

  jump = FALSE;
  if( (rmis >= 2.0l*ck2) && (rho >= 1.0e-10l) )
  {
	if(zph >= 1.0e-10l)
	{
	  bk=cmplx(-zph,rho)*(ck1-cp3);
	  rmis=-creal(bk)/fabsl(cimag(bk));
	  if(rmis > 4.0l*rho/zph)
		jump = TRUE;
	}

	if( ! jump )
	{
	  /* integrate up between branch cuts, then to + infinity */
	  cp1=ck1-(0.1l+0.2lj);
	  cp2=cp1+0.2l;
	  bk=cmplx(0.0l,del);
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

  } /* if( (rmis >= 2.0l*ck2) || (rho >= 1.0e-10l) ) */
  else
	jump = FALSE;

  if( ! jump )
  {
	/* integrate below branch points, then to + infinity */
	for( i = 0; i < 6; i++ )
	  sum[i]=-ans[i];

	rmis=creal(ck1)*1.01l;
	if( (ck2+1.0l) > rmis )
	  rmis=ck2+1.0l;

	bk=cmplx(rmis,0.99l*cimag(ck1));
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

/* fbar is sommerfeld attenuation function for numerical distance p */
void fbar( complex long double p, complex long double *fbar )
{
  int i, minus;
  long double tms, sms;
  complex long double z, zs, sum, pow, term;

  z= CPLX_01* csqrtl( p);
  if( cabs( z) <= 3.0l)
  {
	/* series expansion */
	zs= z* z;
	sum= z;
	pow= z;

	for( i = 1; i <= 100; i++ )
	{
	  pow=- pow* zs/ (long double)i;
	  term= pow/(2.0l* i+1.0l);
	  sum= sum+ term;
	  tms= creal( term* conj( term));
	  sms= creal( sum* conj( sum));
	  if( tms/sms < ACCS)
		break;
	}

	*fbar=1.0l-(1.0l- sum* TOSP)* z* cexp( zs)* SP;

  } /* if( cabs( z) <= 3.0l) */

  /* asymptotic expansion */
  if( creal( z) < 0.0l)
  {
	minus=1;
	z=- z;
  }
  else
	minus=0;

  zs=0.5l/( z* z);
  sum=CPLX_00;
  term=CPLX_10;

  for( i = 1; i <= 6; i++ )
  {
	term =- term*(2.0l*i -1.0l)* zs;
	sum += term;
  }

  if( minus == 1)
	sum -= 2.0l* SP* z* cexp( z* z);
  *fbar=- sum;

}

/*-----------------------------------------------------------------------*/

/* gshank integrates the 6 sommerfeld integrals from start to */
/* infinity (until convergence) in lambda.  at the break point, bk, */
/* the step increment may be changed from dela to delb.  shank's */
/* algorithm to accelerate convergence of a slowly converging series */
/* is used */
  int
gshank( complex long double start, complex long double dela,
	complex long double *sum, int nans, complex long double *seed,
	int ibk, complex long double bk, complex long double delb )
{
  int ibx, j, i, jm, intx, inx, brk=0, idx;
  static long double rbk, amg, den, denm;
  complex long double a1, a2, as1, as2, del, aa;
  static complex long double *q1 = NULL, *q2 = NULL;
  static complex long double *ans1 = NULL, *ans2 = NULL;
  static gboolean first_call = TRUE;

  if( first_call )
  {
	first_call = FALSE;
	mem_alloc( (void *)&q1,
		6*20*sizeof(complex long double), "in somnec.c");
	mem_alloc( (void *)&q2,
		6*20*sizeof(complex long double), "in somnec.c");
	mem_alloc( (void *)&ans1,
		6*sizeof(complex long double), "in somnec.c");
	mem_alloc( (void *)&ans2,
		6*sizeof(complex long double), "in somnec.c");
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
	  if( ibx != 2 )
	  {
		for( i = 0; i < nans; i++ )
		  ans2[i] += sum[i];
		intx = 0;
		continue;
	  }

	  for( i = 0; i < nans; i++ )
		ans2[i]=ans1[i]+sum[i];
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
	  if( ibx != 2 )
	  {
		for( i = 0; i < nans; i++ )
		  ans2[i] += sum[i];
		intx = 0;
		continue;
	  }

	  for( i = 0; i < nans; i++ )
		ans2[i] = ans1[i]+sum[i];
	  intx = 0;
	  continue;

	} /* if( (ibx == 0) && (creal(b) >= rbk) ) */

	rom1(nans,sum,2);
	for( i = 0; i < nans; i++ )
	  ans2[i]=ans1[i]+sum[i];

	den=0.0l;
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
		  a1=q1[idx]+as1-2.0l*aa;

		  if( (creal(a1) != 0.0l) || (cimag(a1) != 0.0l) )
		  {
			a2=aa-q1[idx];
			a1=q1[idx]-a2*a2/a1;
		  }
		  else
			a1=q1[idx];

		  a2=aa+as2-2.0l*as1;
		  if( (creal(a2) != 0.0l) || (cimag(a2) != 0.0l) )
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
	  amg=fabsl(creal(as2))+fabsl(cimag(as2));
	  if(amg > den)
		den=amg;

	} /* for( i = 0; i < nans; i++ ) */

	denm=1.0e-3l*den*CRIT;
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
		den=(fabsl(creal(a1))+fabsl(cimag(a1)))*CRIT;
		if(den < denm)
		  den=denm;
		a1=q1[idx]-a1;
		amg=fabsl(creal(a1)+fabsl(cimag(a1)));
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
		sum[i]=0.5l*(q1[idx]+q2[idx]);
	  }
	  return(0);
	}

  } /* for( intx = 1; intx <= maxh; intx++ ) */

  /* No convergence */
  stop( "gshank(): No convergence", 1 );
  return(1);
}

/*-----------------------------------------------------------------------*/

/* hankel evaluates hankel function of the first kind,   */
/* order zero, and its derivative for complex argument z */
  int
hankel( complex long double z,
	complex long double *h0, complex long double *h0p )
{
  int i, k, ib, iz, miz;
  static int *m = NULL, init = FALSE;
  static long double *a1 = NULL, *a2 = NULL, *a3 = NULL;
  static long double *a4, psi, tst, zms;
  complex long double clogz, j0, j0p, p0z, p1z, q0z, q1z;
  complex long double y0 = CPLX_00, y0p = CPLX_00, zi, zi2, zk;
  static gboolean first_call = TRUE;

  if( first_call )
  {
	first_call = FALSE;
	mem_alloc( (void *)&m,
		101*sizeof(int), "in somnec.c");
	mem_alloc( (void *)&a1,
		25*sizeof(long double), "in somnec.c");
	mem_alloc( (void *)&a2,
		25*sizeof(long double), "in somnec.c");
	mem_alloc( (void *)&a3,
		25*sizeof(long double), "in somnec.c");
	mem_alloc( (void *)&a4,
		25*sizeof(long double), "in somnec.c");
  }

  /* initialization of constants */
  if( ! init )
  {
	psi=-GAMMA;
	for( k = 1; k <= 25; k++ )
	{
	  i = k-1;
	  a1[i]=-0.25l/(k*k);
	  a2[i]=1.0l/(k+1.0l);
	  psi += 1.0l/k;
	  a3[i]=psi+psi;
	  a4[i]=(psi+psi+1.0l/(k+1.0l))/(k+1.0l);
	}

	for( i = 1; i <= 101; i++ )
	{
	  tst=1.0l;
	  for( k = 0; k < 24; k++ )
	  {
		init = k;
		tst *= -i*a1[k];
		if(tst*a3[k] < 1.0e-6l)
		  break;
	  }
	  m[i-1]=init+1;
	}

	init = TRUE;

  } /* if( ! init ) */

  zms=z*conj(z);
  if(zms == 0.0l)
	stop( "Hankel not valid for z = 0", 1 );

  ib=0;
  if(zms <= 16.81l)
  {
	if(zms > 16.0l)
	  ib=1;

	/* series expansion */
	iz=zms;
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

	j0p *= -0.5l*z;
	clogz=clogl(0.5l*z);
	y0=(2.0l*j0*clogz-y0)/PI+C2;
	y0p=(2.0l/z+2.0l*j0p*clogz+0.5l*y0p*z)/PI+C1*z;
	*h0=j0+CPLX_01*y0;
	*h0p=j0p+CPLX_01*y0p;

	if(ib == 0)
	  return(0);

	y0=*h0;
	y0p=*h0p;

  } /* if(zms <= 16.81l) */

  /* asymptotic expansion */
  zi=1.0l/z;
  zi2=zi*zi;
  p0z=1.0l+(P20*zi2-P10)*zi2;
  p1z=1.0l+(P11-P21*zi2)*zi2;
  q0z=(Q20*zi2-Q10)*zi;
  q1z=(Q11-Q21*zi2)*zi;
  zk=cexp(CPLX_01*(z-POF))*csqrtl(zi)*C3;
  *h0=zk*(p0z+CPLX_01*q0z);
  *h0p=CPLX_01*zk*(p1z+CPLX_01*q1z);

  if(ib == 0)
	return(0);

  zms=cosl((sqrtl(zms)-4.0l)*31.41592654l);
  *h0=0.5l*(y0*(1.0l+zms)+ *h0*(1.0l-zms));
  *h0p=0.5l*(y0p*(1.0l+zms)+ *h0p*(1.0l-zms));

  return(0);
}

/*-----------------------------------------------------------------------*/

/* compute integration parameter xlam=lambda from parameter t. */
void lambda( long double t,
	complex long double *xlam, complex long double *dxlam )
{
  *dxlam=b-a;
  *xlam=a+*dxlam*t;
  return;
}

/*-----------------------------------------------------------------------*/

/* rom1 integrates the 6 sommerfeld integrals from a to b in lambda. */
/* the method of variable interval width romberg integration is used. */
void rom1( int n, complex long double *sum, int nx )
{
  int jump, lstep, nogo, i, ns, nt;
  static long double z, ze, s, ep, zend, dz=0.0l, dzot=0.0l, tr, ti;
  static complex long double t00, t11, t02;
  static complex long double *g1 = NULL, *g2 = NULL;
  static complex long double *g3 = NULL, *g4 = NULL;
  static complex long double *g5 = NULL, *t01 = NULL;
  static complex long double *t10 = NULL, *t20 = NULL;
  static gboolean first_call = TRUE;

  if( first_call )
  {
	first_call = FALSE;
	mem_alloc( (void *)&g1,
		6*sizeof(complex long double), "in somnec.c");
	mem_alloc( (void *)&g2,
		6*sizeof(complex long double), "in somnec.c");
	mem_alloc( (void *)&g3,
		6*sizeof(complex long double), "in somnec.c");
	mem_alloc( (void *)&g4,
		6*sizeof(complex long double), "in somnec.c");
	mem_alloc( (void *)&g5,
		6*sizeof(complex long double), "in somnec.c");
	mem_alloc( (void *)&t01,
		6*sizeof(complex long double), "in somnec.c");
	mem_alloc( (void *)&t10,
		6*sizeof(complex long double), "in somnec.c");
	mem_alloc( (void *)&t20,
		6*sizeof(complex long double), "in somnec.c");
  }

  lstep=0;
  z=0.0l;
  ze=1.0l;
  s=1.0l;
  ep=s/(1.0e4l*NM);
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
		if( dz <= ep )
		  return;
	  }

	  dzot=dz*.5l;
	  saoa(z+dzot,g3);
	  saoa(z+dz,g5);

	} /* if( ! jump ) */

	nogo=FALSE;
	for( i = 0; i < n; i++ )
	{
	  t00=(g1[i]+g5[i])*dzot;
	  t01[i]=(t00+dz*g3[i])*.5l;
	  t10[i]=(4.0l*t01[i]-t00)/3.0l;

	  /* test convergence of 3 point romberg result */
	  test( creal(t01[i]), creal(t10[i]), &tr,
		  cimag(t01[i]), cimag(t10[i]), &ti, 0.0l );
	  if( (tr > CRIT) || (ti > CRIT) )
		nogo = TRUE;
	}

	if( ! nogo )
	{
	  for( i = 0; i < n; i++ )
		sum[i] += t10[i];

	  nt += 2;
	  z += dz;
	  if(z > zend)
		return;

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

	saoa(z+dz*.250l,g2);
	saoa(z+dz*.75l,g4);
	nogo=FALSE;
	for( i = 0; i < n; i++ )
	{
	  t02=(t01[i]+dzot*(g2[i]+g4[i]))*0.50l;
	  t11=(4.0l*t02-t01[i])/3.0l;
	  t20[i]=(16.0l*t11-t10[i])/15.0l;

	  /* test convergence of 5 point romberg result */
	  test( creal(t11), creal(t20[i]), &tr,
		  cimag(t11), cimag(t20[i]), &ti, 0.0l );
	  if( (tr > CRIT) || (ti > CRIT) )
		nogo = TRUE;
	}

	if( ! nogo )
	{
	  for( i = 0; i < n; i++ )
		sum[i] += t20[i];

	  nt++;
	  z += dz;
	  if(z > zend)
		return;

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
	  dzot=dz*0.50l;

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
	if(z > zend)
	  return;

	for( i = 0; i < n; i++ )
	  g1[i]=g5[i];

	if( (nt >= NTS) && (ns > nx) )
	{
	  ns /= 2;
	  nt=1;
	}

	jump = FALSE;

  } /* while( TRUE ) */

}

/*-----------------------------------------------------------------------*/

/* saoa computes the integrand for each of the 6 sommerfeld */
/* integrals for source and observer above ground */
void saoa( long double t, complex long double *ans)
{
  long double xlr, sign;
  static complex long double xl, dxl, cgam1, cgam2, b0;
  static complex long double b0p, com, dgam, den1, den2;

  lambda(t, &xl, &dxl);
  if( jh == 0 )
  {
	/* bessel function form */
	bessel(xl*rho, &b0, &b0p);
	b0  *=2.0l;
	b0p *=2.0l;
	cgam1=csqrtl(xl*xl-ck1sq);
	cgam2=csqrtl(xl*xl-ck2sq);
	if(creal(cgam1) == 0.0l)
	  cgam1=cmplx(0.0l,-fabsl(cimag(cgam1)));
	if(creal(cgam2) == 0.0l)
	  cgam2=cmplx(0.0l,-fabsl(cimag(cgam2)));
  }
  else
  {
	/* hankel function form */
	hankel(xl*rho, &b0, &b0p);
	com=xl-ck1;
	cgam1=csqrtl(xl+ck1)*csqrtl(com);
	if(creal(com) < 0.0l && cimag(com) >= 0.0l)
	  cgam1=-cgam1;
	com=xl-ck2;
	cgam2=csqrtl(xl+ck2)*csqrtl(com);
	if(creal(com) < 0.0l && cimag(com) >= 0.0l)
	  cgam2=-cgam2;
  }

  xlr=xl*conj(xl);
  if(xlr >= tsmag)
  {
	if(cimag(xl) >= 0.0l)
	{
	  xlr=creal(xl);
	  if(xlr >= ck2)
	  {
		if(xlr <= ck1r)
		  dgam=cgam2-cgam1;
		else
		{
		  sign=1.0l;
		  dgam=1.0l/(xl*xl);
		  dgam=sign*((ct3*dgam+ct2)*dgam+ct1)/xl;
		}
	  }
	  else
	  {
		sign=-1.0l;
		dgam=1.0l/(xl*xl);
		dgam=sign*((ct3*dgam+ct2)*dgam+ct1)/xl;
	  } /* if(xlr >= ck2) */

	} /* if(cimag(xl) >= 0.0l) */
	else
	{
	  sign=1.0l;
	  dgam=1.0l/(xl*xl);
	  dgam=sign*((ct3*dgam+ct2)*dgam+ct1)/xl;
	}

  } /* if(xlr < tsmag) */
  else
	dgam=cgam2-cgam1;

  den2=cksm*dgam/(cgam2*(ck1sq*cgam2+ck2sq*cgam1));
  den1=1.0l/(cgam1+cgam2)-cksm/cgam2;
  com=dxl*xl*cexp(-cgam2*zph);
  ans[5]=com*b0*den1/ck1;
  com *= den2;

  if(rho != 0.0l)
  {
	b0p=b0p/rho;
	ans[0]=-com*xl*(b0p+b0*xl);
	ans[3]=com*xl*b0p;
  }
  else
  {
	ans[0]=-com*xl*xl*0.5l;
	ans[3]=ans[0];
  }

  ans[1]=com*cgam2*cgam2*b0;
  ans[2]=-ans[3]*cgam2*rho;
  ans[4]=com*b0;

  return;
}

/*-----------------------------------------------------------------------*/

