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

/******* Translated to the C language by N. Kyriazis  20 Aug 2003 ******
 *
 * Program NEC(input,tape5=input,output,tape11,tape12,tape13,tape14,
 * tape15,tape16,tape20,tape21)
 *
 * Numerical Electromagnetics Code (NEC2)  developed at Lawrence
 * Livermore lab., Livermore, CA.  (contact G. Burke at 415-422-8414
 * for problems with the NEC code. For problems with the vax implem-
 * entation, contact J. Breakall at 415-422-8196 or E. Domning at 415
 * 422-5936)
 * file created 4/11/80
 *
 *                ***********Notice**********
 * This computer code material was prepared as an account of work
 * sponsored by the United States government.  Neither the United
 * States nor the United States Department Of Energy, nor any of
 * their employees, nor any of their contractors, subcontractors,
 * or their employees, makes any warranty, express or implied, or
 * assumes any legal liability or responsibility for the accuracy,
 * completeness or usefulness of any information, apparatus, product
 * or process disclosed, or represents that its use would not infringe
 * privately-owned rights.
 *
 ***********************************************************************/

#include "xnec2c.h"

/* common  /crnt/ */
extern crnt_t crnt;

/* common  /data/ */
extern data_t data;

/* common  /dataj/ */
extern dataj_t dataj;

/*common  /ggrid/ */
extern ggrid_t ggrid;

/* common  /segj/ */
extern segj_t segj;

/* common  /vsorc/ */
extern vsorc_t vsorc;

/* common  /zload/ */
extern zload_t zload;

/*-------------------------------------------------------------------*/

/* fill incident field array for charge discontinuity voltage source */
void qdsrc( int is, complex long double v, complex long double *e )
{
  int i, jx, j, jp1, ipr, ij, i1;
  long double xi, yi, zi, ai, cabi, sabi, salpi, tx, ty, tz;
  complex long double curd, etk, ets, etc;

  is--;
  i= data.icon1[is];
  data.icon1[is]=0;
  tbf( is+1,0);
  data.icon1[is]= i;
  dataj.s= data.si[is]*.5l;
  curd= CCJ* v/(( logl(2.0l * dataj.s/ data.bi[is])-1.0l) *
	  ( segj.bx[segj.jsno-1] * cosl( TP* dataj.s) +
		segj.cx[segj.jsno-1] * sinl( TP* dataj.s))* data.wlam);
  vsorc.vqds[vsorc.nqds]= v;
  vsorc.iqds[vsorc.nqds]= is+1;
  vsorc.nqds++;

  for( jx = 0; jx < segj.jsno; jx++ )
  {
	j= segj.jco[jx]-1;
	jp1 = j+1;
	dataj.s= data.si[j];
	dataj.b= data.bi[j];
	dataj.xj= data.x[j];
	dataj.yj= data.y[j];
	dataj.zj= data.z[j];
	dataj.cabj= data.cab[j];
	dataj.sabj= data.sab[j];
	dataj.salpj= data.salp[j];

	if( dataj.iexk != 0)
	{
	  ipr= data.icon1[j];

	  if (ipr > PCHCON) dataj.ind1=2;
	  else if( ipr < 0 )
	  {
		ipr=- ipr;
		ipr--;
		if( -data.icon1[ipr-1] != jp1 )
		  dataj.ind1=2;
		else
		{
		  xi= fabsl( dataj.cabj* data.cab[ipr]+ dataj.sabj*
			  data.sab[ipr]+ dataj.salpj* data.salp[ipr]);
		  if( (xi < 0.999999l) ||
			  (fabsl(data.bi[ipr]/dataj.b-1.0l) > 1.0e-6l) )
			dataj.ind1=2;
		  else
			dataj.ind1=0;
		}
	  }  /* if( ipr < 0 ) */
	  else
		if( ipr == 0 )
		  dataj.ind1=1;
		else /* ipr > 0 */
		{
		  ipr--;
		  if( ipr != j )
		  {
			if( data.icon2[ipr] != jp1)
			  dataj.ind1=2;
			else
			{
			  xi= fabsl( dataj.cabj* data.cab[ipr]+ dataj.sabj*
				  data.sab[ipr]+ dataj.salpj* data.salp[ipr]);
			  if( (xi < 0.999999l) ||
				  (fabsl(data.bi[ipr]/dataj.b-1.0l) > 1.0e-6l) )
				dataj.ind1=2;
			  else
				dataj.ind1=0;
			}
		  } /* if( ipr != j ) */
		  else
		  {
			if( (dataj.cabj*dataj.cabj + dataj.sabj*dataj.sabj) > 1.0e-8l)
			  dataj.ind1=2;
			else
			  dataj.ind1=0;
		  }
		} /* else */

	  ipr= data.icon2[j];
	  if (ipr > PCHCON) dataj.ind2=2;
	  else if( ipr < 0 )
	  {
		ipr = -ipr;
		ipr--;
		if( -data.icon2[ipr] != jp1 )
		  dataj.ind1=2;
		else
		{
		  xi= fabsl( dataj.cabj* data.cab[ipr]+ dataj.sabj *
			  data.sab[ipr]+ dataj.salpj* data.salp[ipr]);
		  if( (xi < 0.999999l) ||
			  (fabsl(data.bi[ipr]/dataj.b-1.0l) > 1.0e-6l) )
			dataj.ind1=2;
		  else
			dataj.ind1=0;
		}
	  } /* if( ipr < 0 ) */
	  else
		if( ipr == 0 )
		  dataj.ind2=1;
		else /* ipr > 0 */
		{
		  ipr--;
		  if( ipr != j )
		  {
			if( data.icon1[ipr] != jp1)
			  dataj.ind2=2;
			else
			{
			  xi= fabsl( dataj.cabj* data.cab[ipr]+ dataj.sabj*
				  data.sab[ipr]+ dataj.salpj* data.salp[ipr]);
			  if( (xi < 0.9999990l) ||
				  (fabsl(data.bi[ipr]/dataj.b-1.0l) > 1.0e-6l) )
				dataj.ind2=2;
			  else
				dataj.ind2=0;
			}
		  } /* if( ipr != j )*/
		  else
		  {
			if( (dataj.cabj* dataj.cabj + dataj.sabj* dataj.sabj) > 1.0e-8l)
			  dataj.ind1=2;
			else
			  dataj.ind1=0;
		  }
		} /* else */

	} /* if( dataj.iexk != 0) */

	for( i = 0; i < data.n; i++ )
	{
	  ij= i- j;
	  xi= data.x[i];
	  yi= data.y[i];
	  zi= data.z[i];
	  ai= data.bi[i];
	  efld( xi, yi, zi, ai, ij);
	  cabi= data.cab[i];
	  sabi= data.sab[i];
	  salpi= data.salp[i];
	  etk= dataj.exk* cabi+ dataj.eyk* sabi+ dataj.ezk* salpi;
	  ets= dataj.exs* cabi+ dataj.eys* sabi+ dataj.ezs* salpi;
	  etc= dataj.exc* cabi+ dataj.eyc* sabi+ dataj.ezc* salpi;
	  e[i]= e[i]-( etk* segj.ax[jx] +
		  ets* segj.bx[jx]+ etc* segj.cx[jx])* curd;
	}

	if( data.m != 0)
	{
	  i1= data.n-1;
	  for( i = 0; i < data.m; i++ )
	  {
		xi= data.px[i];
		yi= data.py[i];
		zi= data.pz[i];
		hsfld( xi, yi, zi, 0.0l);
		i1++;
		tx= data.t2x[i];
		ty= data.t2y[i];
		tz= data.t2z[i];
		etk= dataj.exk* tx+ dataj.eyk* ty+ dataj.ezk* tz;
		ets= dataj.exs* tx+ dataj.eys* ty+ dataj.ezs* tz;
		etc= dataj.exc* tx+ dataj.eyc* ty+ dataj.ezc* tz;
		e[i1] += ( etk* segj.ax[jx]+ ets* segj.bx[jx]+
			etc* segj.cx[jx] )* curd* data.psalp[i];
		i1++;
		tx= data.t1x[i];
		ty= data.t1y[i];
		tz= data.t1z[i];
		etk= dataj.exk* tx+ dataj.eyk* ty+ dataj.ezk* tz;
		ets= dataj.exs* tx+ dataj.eys* ty+ dataj.ezs* tz;
		etc= dataj.exc* tx+ dataj.eyc* ty+ dataj.ezc* tz;
		e[i1] += ( etk* segj.ax[jx]+ ets* segj.bx[jx]+
			etc* segj.cx[jx])* curd* data.psalp[i];
	  }

	} /* if( m != 0) */

	if( zload.nload > 0 )
	  e[j] += zload.zarray[j]* curd*(segj.ax[jx]+ segj.cx[jx]);

  } /* for( jx = 0; jx < segj.jsno; jx++ ) */

  return;
}

/*-----------------------------------------------------------------------*/

/* cabc computes coefficients of the constant (a), sine (b), and */
/* cosine (c) terms in the current interpolation functions for the */
/* current vector cur. */
void cabc( complex long double *curx)
{
  int i, is, j, jx, jco1, jco2;
  long double ar, ai, sh;
  complex long double curd, cs1, cs2;

  if( data.n != 0)
  {
	for( i = 0; i < data.n; i++ )
	{
	  crnt.air[i]=0.0l;
	  crnt.aii[i]=0.0l;
	  crnt.bir[i]=0.0l;
	  crnt.bii[i]=0.0l;
	  crnt.cir[i]=0.0l;
	  crnt.cii[i]=0.0l;
	}

	for( i = 0; i < data.n; i++ )
	{
	  ar= creall( curx[i]);
	  ai= cimagl( curx[i]);
	  tbf( i+1, 1 );

	  for( jx = 0; jx < segj.jsno; jx++ )
	  {
		j= segj.jco[jx]-1;
		crnt.air[j] += segj.ax[jx]* ar;
		crnt.aii[j] += segj.ax[jx]* ai;
		crnt.bir[j] += segj.bx[jx]* ar;
		crnt.bii[j] += segj.bx[jx]* ai;
		crnt.cir[j] += segj.cx[jx]* ar;
		crnt.cii[j] += segj.cx[jx]* ai;
	  }

	} /* for( i = 0; i < n; i++ ) */

	if( vsorc.nqds != 0)
	{
	  for( is = 0; is < vsorc.nqds; is++ )
	  {
		i= vsorc.iqds[is]-1;
		jx= data.icon1[i];
		data.icon1[i]=0;
		tbf(i+1,0);
		data.icon1[i]= jx;
		sh= data.si[i]*.5l;
		curd= CCJ* vsorc.vqds[is]/( (logl(2.0l* sh/ data.bi[i])-1.0l) *
			(segj.bx[segj.jsno-1]* cosl(TP* sh)+ segj.cx[segj.jsno-1] *
			 sinl(TP* sh))* data.wlam );
		ar= creall( curd);
		ai= cimagl( curd);

		for( jx = 0; jx < segj.jsno; jx++ )
		{
		  j= segj.jco[jx]-1;
		  crnt.air[j]= crnt.air[j]+ segj.ax[jx]* ar;
		  crnt.aii[j]= crnt.aii[j]+ segj.ax[jx]* ai;
		  crnt.bir[j]= crnt.bir[j]+ segj.bx[jx]* ar;
		  crnt.bii[j]= crnt.bii[j]+ segj.bx[jx]* ai;
		  crnt.cir[j]= crnt.cir[j]+ segj.cx[jx]* ar;
		  crnt.cii[j]= crnt.cii[j]+ segj.cx[jx]* ai;
		}

	  } /* for( is = 0; is < vsorc.nqds; is++ ) */

	} /* if( vsorc.nqds != 0) */

	for( i = 0; i < data.n; i++ )
	  curx[i]= cmplx( crnt.air[i]+crnt.cir[i], crnt.aii[i]+crnt.cii[i] );

  } /* if( n != 0) */

  if( data.m == 0)
	return;

  /* convert surface currents from */
  /* t1,t2 components to x,y,z components */
  jco1= data.np2m;
  jco2= jco1+ data.m;
  for( i = 1; i <= data.m; i++ )
  {
	jco1 -= 2;
	jco2 -= 3;
	cs1= curx[jco1];
	cs2= curx[jco1+1];
	curx[jco2]  = cs1* data.t1x[data.m-i]+ cs2* data.t2x[data.m-i];
	curx[jco2+1]= cs1* data.t1y[data.m-i]+ cs2* data.t2y[data.m-i];
	curx[jco2+2]= cs1* data.t1z[data.m-i]+ cs2* data.t2z[data.m-i];
  }

  return;
}

/*-----------------------------------------------------------------------*/

/* function db10 returns db for magnitude (field) */
long double db10( long double x )
{
  if( x < 1.0e-20l )
	return( -999.99l );

  return( 10.0l * log10l(x) );
}

/*-----------------------------------------------------------------------*/

/* function db20 returns db for mag**2 (power) i */
long double db20( long double x )
{
  if( x < 1.0e-20l )
	return( -999.99l );

  return( 20.0l * log10l(x) );
}

/*-----------------------------------------------------------------------*/

/* intrp uses bivariate cubic interpolation to obtain */
/* the values of 4 functions at the point (x,y). */
void intrp( long double x, long double y, complex long double *f1,
	complex long double *f2, complex long double *f3, complex long double *f4 )
{
  static int ix, iy, ixs=-10, iys=-10, igrs=-10, ixeg=0, iyeg=0;
  static int nxm2, nym2, nxms, nyms, nd, ndp;
  static int *nda = NULL, *ndpa = NULL;
  int igr, iadd, iadz, i, k, jump;
  static long double dx = 1.0l, dy = 1.0l, xs = 0.0l, ys = 0.0l, xz, yz;
  long double xx, yy;
  static complex long double a[4][4], b[4][4], c[4][4], d[4][4];
  complex long double p1=CPLX_00, p2=CPLX_00, p3=CPLX_00, p4=CPLX_00;
  complex long double fx1, fx2, fx3, fx4;
  static gboolean first_call = TRUE;

  if( first_call )
  {
	first_call = FALSE;
	mem_alloc( (void *)&nda,  3*sizeof(int), "in calculations.c");
	mem_alloc( (void *)&ndpa, 3*sizeof(int), "in calculations.c");
	nda[0] = 11; nda[1] = 17; nda[2] = 9;
	ndpa[0] = 110; ndpa[1] = 85; ndpa[2] = 72;
  }

  jump = FALSE;
  if( (x < xs) || (y < ys) )
	jump = TRUE;
  else
  {
	ix= (int)(( x- xs)/ dx)+1;
	iy= (int)(( y- ys)/ dy)+1;
  }

  /* if point lies in same 4 by 4 point region */
  /* as previous point, old values are reused. */
  if( (ix < ixeg) ||
	  (iy < iyeg) ||
	  (abs(ix- ixs) >= 2) ||
	  (abs(iy- iys) >= 2) ||
	  jump )
  {
	/* determine correct grid and grid region */
	if( x <= ggrid.xsa[1])
	  igr=0;
	else
	{
	  if( y > ggrid.ysa[2])
		igr=2;
	  else
		igr=1;
	}

	if( igr != igrs)
	{
	  igrs= igr;
	  dx= ggrid.dxa[igrs];
	  dy= ggrid.dya[igrs];
	  xs= ggrid.xsa[igrs];
	  ys= ggrid.ysa[igrs];
	  nxm2= ggrid.nxa[igrs]-2;
	  nym2= ggrid.nya[igrs]-2;
	  nxms=(( nxm2+1)/3)*3+1;
	  nyms=(( nym2+1)/3)*3+1;
	  nd= nda[igrs];
	  ndp= ndpa[igrs];
	  ix= (int)(( x- xs)/ dx)+1;
	  iy= (int)(( y- ys)/ dy)+1;

	} /* if( igr != igrs) */

	ixs=(( ix-1)/3)*3+2;
	if( ixs < 2)
	  ixs=2;
	ixeg=-10000;

	if( ixs > nxm2)
	{
	  ixs= nxm2;
	  ixeg= nxms;
	}

	iys=(( iy-1)/3)*3+2;
	if( iys < 2)
	  iys=2;
	iyeg=-10000;

	if( iys > nym2)
	{
	  iys= nym2;
	  iyeg= nyms;
	}

	/* compute coefficients of 4 cubic polynomials in x for */
	/* the 4 grid values of y for each of the 4 functions */
	iadz= ixs+( iys-3)* nd- ndp;
	for( k = 0; k < 4; k++ )
	{
	  iadz += ndp;
	  iadd = iadz;

	  for( i = 0; i < 4; i++ )
	  {
		iadd += nd;

		switch( igrs )
		{
		  case 0:
			p1= ggrid.ar1[iadd-2];
			p2= ggrid.ar1[iadd-1];
			p3= ggrid.ar1[iadd];
			p4= ggrid.ar1[iadd+1];
			break;

		  case 1:
			p1= ggrid.ar2[iadd-2];
			p2= ggrid.ar2[iadd-1];
			p3= ggrid.ar2[iadd];
			p4= ggrid.ar2[iadd+1];
			break;

		  case 2:
			p1= ggrid.ar3[iadd-2];
			p2= ggrid.ar3[iadd-1];
			p3= ggrid.ar3[iadd];
			p4= ggrid.ar3[iadd+1];

		} /* switch( igrs ) */

		a[i][k]=( p4- p1+3.0l*( p2- p3))*.1666666667l;
		b[i][k]=( p1-2.0l* p2+ p3)*.5;
		c[i][k]= p3-(2.0l* p1+3.0l* p2+ p4)*.1666666667l;
		d[i][k]= p2;

	  } /* for( i = 0; i < 4; i++ ) */

	} /* for( k = 0; k < 4; k++ ) */

	xz=( ixs-1)* dx+ xs;
	yz=( iys-1)* dy+ ys;

  } /* if( (abs(ix- ixs) >= 2) || */

  /* evaluate polymomials in x and use cubic */
  /* interpolation in y for each of the 4 functions. */
  xx=( x- xz)/ dx;
  yy=( y- yz)/ dy;
  fx1=(( a[0][0]* xx+ b[0][0])* xx+ c[0][0])* xx+ d[0][0];
  fx2=(( a[1][0]* xx+ b[1][0])* xx+ c[1][0])* xx+ d[1][0];
  fx3=(( a[2][0]* xx+ b[2][0])* xx+ c[2][0])* xx+ d[2][0];
  fx4=(( a[3][0]* xx+ b[3][0])* xx+ c[3][0])* xx+ d[3][0];
  p1= fx4- fx1+3.0l*( fx2- fx3);
  p2=3.0l*( fx1-2.0l* fx2+ fx3);
  p3=6.0l* fx3-2.0l* fx1-3.0l* fx2- fx4;
  *f1=(( p1* yy+ p2)* yy+ p3)* yy*.1666666667l+ fx2;
  fx1=(( a[0][1]* xx+ b[0][1])* xx+ c[0][1])* xx+ d[0][1];
  fx2=(( a[1][1]* xx+ b[1][1])* xx+ c[1][1])* xx+ d[1][1];
  fx3=(( a[2][1]* xx+ b[2][1])* xx+ c[2][1])* xx+ d[2][1];
  fx4=(( a[3][1]* xx+ b[3][1])* xx+ c[3][1])* xx+ d[3][1];
  p1= fx4- fx1+3.0l*( fx2- fx3);
  p2=3.0l*( fx1-2.0l* fx2+ fx3);
  p3=6.0l* fx3-2.0l* fx1-3.0l* fx2- fx4;
  *f2=(( p1* yy+ p2)* yy+ p3)* yy*.1666666667l+ fx2;
  fx1=(( a[0][2]* xx+ b[0][2])* xx+ c[0][2])* xx+ d[0][2];
  fx2=(( a[1][2]* xx+ b[1][2])* xx+ c[1][2])* xx+ d[1][2];
  fx3=(( a[2][2]* xx+ b[2][2])* xx+ c[2][2])* xx+ d[2][2];
  fx4=(( a[3][2]* xx+ b[3][2])* xx+ c[3][2])* xx+ d[3][2];
  p1= fx4- fx1+3.0l*( fx2- fx3);
  p2=3.0l*( fx1-2.0l* fx2+ fx3);
  p3=6.0l* fx3-2.0l* fx1-3.0l* fx2- fx4;
  *f3=(( p1* yy+ p2)* yy+ p3)* yy*.1666666667l+ fx2;
  fx1=(( a[0][3]* xx+ b[0][3])* xx+ c[0][3])* xx+ d[0][3];
  fx2=(( a[1][3]* xx+ b[1][3])* xx+ c[1][3])* xx+ d[1][3];
  fx3=(( a[2][3]* xx+ b[2][3])* xx+ c[2][3])* xx+ d[2][3];
  fx4=(( a[3][3]* xx+ b[3][3])* xx+ c[3][3])* xx+ d[3][3];
  p1= fx4- fx1+3.0l*( fx2- fx3);
  p2=3.0l*( fx1-2.0l* fx2+ fx3);
  p3=6.0l* fx3-2.0l* fx1-3.0l* fx2- fx4;
  *f4=(( p1* yy+ p2)* yy+ p3)* yy*.1666666667l+ fx2;

  return;
}

/*-----------------------------------------------------------------------*/

/* intx performs numerical integration of exp(jkr)/r by the method of */
/* variable interval width romberg integration.  the integrand value */
/* is supplied by subroutine gf. */
void intx( long double el1, long double el2, long double b,
	int ij, long double *sgr, long double *sgi )
{
  int ns, nt;
  int nx = 1, nma = 65536, nts = 4;
  int flag = TRUE;
  long double z, s, ze, fnm, ep, zend, fns, dz=0.0l, zp;
  long double t00r, g1r, g5r, t00i, g1i, g5i, t01r, g3r;
  long double t01i, g3i, t10r, t10i, te1i, te1r, t02r;
  long double g2r, g4r, t02i, g2i, g4i, t11r, t11i, t20r;
  long double  t20i, te2i, te2r, rx = 1.0e-4l, dzot=0.0l;

  z= el1;
  ze= el2;
  if( ij == 0)
	ze=0.0l;
  s= ze- z;
  fnm= nma;
  ep= s/(10.0l* fnm);
  zend= ze- ep;
  *sgr=0.0l;
  *sgi=0.0l;
  ns= nx;
  nt=0;
  gf( z, &g1r, &g1i);

  while( TRUE )
  {
	if( flag )
	{
	  fns= ns;
	  dz= s/ fns;
	  zp= z+ dz;

	  if( zp > ze)
	  {
		dz= ze- z;
		if( fabsl(dz) <= ep)
		{
		  /* add contribution of near singularity for diagonal term */
		  if(ij == 0)
		  {
			*sgr=2.0l*( *sgr+ logl(( sqrtl( b* b+ s* s)+ s)/ b));
			*sgi=2.0l* *sgi;
		  }
		  return;
		}

	  } /* if( zp > ze) */

	  dzot= dz*.5l;
	  zp= z+ dzot;
	  gf( zp, &g3r, &g3i);
	  zp= z+ dz;
	  gf( zp, &g5r, &g5i);

	} /* if( flag ) */

	t00r=( g1r+ g5r)* dzot;
	t00i=( g1i+ g5i)* dzot;
	t01r=( t00r+ dz* g3r)*0.5l;
	t01i=( t00i+ dz* g3i)*0.5l;
	t10r=(4.0l* t01r- t00r)/3.0l;
	t10i=(4.0l* t01i- t00i)/3.0l;

	/* test convergence of 3 point romberg result. */
	test( t01r, t10r, &te1r, t01i, t10i, &te1i, 0.0l);
	if( (te1i <= rx) && (te1r <= rx) )
	{
	  *sgr= *sgr+ t10r;
	  *sgi= *sgi+ t10i;
	  nt += 2;

	  z += dz;
	  if( z >= zend)
	  {
		/* add contribution of near singularity for diagonal term */
		if(ij == 0)
		{
		  *sgr=2.0l*( *sgr+ logl(( sqrtl( b* b+ s* s)+ s)/ b));
		  *sgi=2.0l* *sgi;
		}
		return;
	  }

	  g1r= g5r;
	  g1i= g5i;
	  if( nt >= nts)
		if( ns > nx)
		{
		  /* Double step size */
		  ns= ns/2;
		  nt=1;
		}
	  flag = TRUE;
	  continue;

	} /* if( (te1i <= rx) && (te1r <= rx) ) */

	zp= z+ dz*0.25l;
	gf( zp, &g2r, &g2i);
	zp= z+ dz*0.75l;
	gf( zp, &g4r, &g4i);
	t02r=( t01r+ dzot*( g2r+ g4r))*0.5l;
	t02i=( t01i+ dzot*( g2i+ g4i))*0.5l;
	t11r=(4.0l* t02r- t01r)/3.0l;
	t11i=(4.0l* t02i- t01i)/3.0l;
	t20r=(16.0l* t11r- t10r)/15.0l;
	t20i=(16.0l* t11i- t10i)/15.0l;

	/* test convergence of 5 point romberg result. */
	test( t11r, t20r, &te2r, t11i, t20i, &te2i, 0.0l);
	if( (te2i > rx) || (te2r > rx) )
	{
	  nt=0;
	  if( ns >= nma)
		fprintf( stderr,
			"xnec2c: step size limited at z= %10.5LF\n", z );
	  else
	  {
		/* halve step size */
		ns= ns*2;
		fns= ns;
		dz= s/ fns;
		dzot= dz*0.5l;
		g5r= g3r;
		g5i= g3i;
		g3r= g2r;
		g3i= g2i;

		flag = FALSE;
		continue;
	  }

	} /* if( (te2i > rx) || (te2r > rx) ) */

	*sgr= *sgr+ t20r;
	*sgi= *sgi+ t20i;
	nt++;

	z += dz;
	if( z >= zend)
	{
	  /* add contribution of near singularity for diagonal term */
	  if(ij == 0)
	  {
		*sgr=2.0l*( *sgr+ logl(( sqrtl( b* b+ s* s)+ s)/ b));
		*sgi=2.0l* *sgi;
	  }
	  return;
	}

	g1r= g5r;
	g1i= g5i;
	if( nt >= nts)
	  if( ns > nx)
	  {
		/* Double step size */
		ns= ns/2;
		nt=1;
	  }
	flag = TRUE;

  } /* while( TRUE ) */

}

/*-----------------------------------------------------------------------*/

/* returns smallest of two arguments */
int min( int a, int b )
{
  if( a < b )
	return(a);
  else
	return(b);
}

/*-----------------------------------------------------------------------*/

/* test for convergence in numerical integration */
void test( long double f1r, long double f2r, long double *tr,
	long double f1i, long double f2i, long double *ti, long double dmin )
{
  long double den;

  den= fabsl( f2r);
  *tr= fabsl( f2i);

  if( den < *tr)
	den= *tr;
  if( den < dmin)
	den= dmin;

  if( den < 1.0e-37l)
  {
	*tr=0.0l;
	*ti=0.0l;
	return;
  }

  *tr= fabsl(( f1r- f2r)/ den);
  *ti= fabsl(( f1i- f2i)/ den);

  return;
}

/*-----------------------------------------------------------------------*/

/* compute component of basis function i on segment is. */
  int
sbf( int i, int is, long double *aa, long double *bb, long double *cc )
{
  int ix, jsno, june, jcox, jcoxx, jend, iend, njun1=0, njun2;
  long double d, sig, pp, sdh, cdh, sd, omc;
  long double aj, pm=0, cd, ap, qp, qm, xxi;

  *aa=0.0l;
  *bb=0.0l;
  *cc=0.0l;
  june=0;
  jsno=0;
  pp=0.0l;
  ix=i-1;

  jcox= data.icon1[ix];
  if( jcox > PCHCON)
	jcox= i;
  jcoxx = jcox-1;

  jend=-1;
  iend=-1;
  sig=-1.0l;

  do
  {
	if( jcox != 0 )
	{
	  if( jcox < 0 )
		jcox=- jcox;
	  else
	  {
		sig=- sig;
		jend=- jend;
	  }

	  jcoxx = jcox-1;
	  jsno++;
	  d= PI* data.si[jcoxx];
	  sdh= sinl( d);
	  cdh= cosl( d);
	  sd=2.0l* sdh* cdh;

	  if( d <= 0.015l)
	  {
		omc=4.0l* d* d;
		omc=((1.3888889e-3l* omc -4.1666666667e-2l)* omc +.5l)* omc;
	  }
	  else
		omc=1.0l- cdh* cdh+ sdh* sdh;

	  aj=1.0l/( logl(1.0l/( PI* data.bi[jcoxx]))-.577215664l);
	  pp -= omc/ sd* aj;

	  if( jcox == is)
	  {
		*aa= aj/ sd* sig;
		*bb= aj/(2.0l* cdh);
		*cc=- aj/(2.0l* sdh)* sig;
		june= iend;
	  }

	  if( jcox != i )
	  {
		if( jend != 1)
		  jcox= data.icon1[jcoxx];
		else
		  jcox= data.icon2[jcoxx];

		if( abs(jcox) != i )
		{
		  if( jcox == 0 )
		  {
			fprintf( stderr,
				"xnec2c: sbf() - segment connection"
				" error for segment %d\n", i );
			stop( "Segment connection error", 1 );
		  }
		  else
			continue;
		}

	  } /* if( jcox != i ) */
	  else
		if( jcox == is)
		  *bb=- *bb;

	  if( iend == 1)
		break;

	} /* if( jcox != 0 ) */

	pm=- pp;
	pp=0.0l;
	njun1= jsno;

	jcox= data.icon2[ix];
	if( jcox > PCHCON)
	  jcox= i;

	jend=1;
	iend=1;
	sig=-1.0l;

  } /* do */
  while( jcox != 0 );

  njun2= jsno- njun1;
  d= PI* data.si[ix];
  sdh= sinl( d);
  cdh= cosl( d);
  sd=2.0l* sdh* cdh;
  cd= cdh* cdh- sdh* sdh;

  if( d <= 0.015l)
  {
	omc=4.0l* d* d;
	omc=((1.3888889e-3l* omc -4.1666666667e-2l)* omc +.5l)* omc;
  }
  else
	omc=1.0l- cd;

  ap=1.0l/( logl(1.0l/( PI* data.bi[ix])) -.577215664l);
  aj= ap;

  if( njun1 == 0)
  {
	if( njun2 == 0)
	{
	  *aa =-1.0l;
	  qp= PI* data.bi[ix];
	  xxi= qp* qp;
	  xxi= qp*(1.0l-.5l* xxi)/(1.0l- xxi);
	  *cc=1.0l/( cdh- xxi* sdh);
	  return(0);
	}

	qp= PI* data.bi[ix];
	xxi= qp* qp;
	xxi= qp*(1.0l-.5l* xxi)/(1.0l- xxi);
	qp=-( omc+ xxi* sd)/( sd*( ap+ xxi* pp)+ cd*( xxi* ap- pp));

	if( june == 1)
	{
	  *aa=- *aa* qp;
	  *bb=  *bb* qp;
	  *cc=- *cc* qp;
	  if( i != is)
		return(0);
	}

	*aa -= 1.0l;
	d = cd - xxi * sd;
	*bb += (sdh + ap * qp * (cdh - xxi * sdh)) / d;
	*cc += (cdh + ap * qp * (sdh + xxi * cdh)) / d;
	return(0);

  } /* if( njun1 == 0) */

  if( njun2 == 0)
  {
	qm= PI* data.bi[ix];
	xxi= qm* qm;
	xxi= qm*(1.0l-.5l* xxi)/(1.0l- xxi);
	qm=( omc+ xxi* sd)/( sd*( aj- xxi* pm)+ cd*( pm+ xxi* aj));

	if( june == -1)
	{
	  *aa= *aa* qm;
	  *bb= *bb* qm;
	  *cc= *cc* qm;
	  if( i != is)
		return(0);
	}

	*aa -= 1.0l;
	d= cd- xxi* sd;
	*bb += ( aj* qm*( cdh- xxi* sdh)- sdh)/ d;
	*cc += ( cdh- aj* qm*( sdh+ xxi* cdh))/ d;
	return(0);

  } /* if( njun2 == 0) */

  qp= sd*( pm* pp+ aj* ap)+ cd*( pm* ap- pp* aj);
  qm=( ap* omc- pp* sd)/ qp;
  qp=-( aj* omc+ pm* sd)/ qp;

  if( june != 0 )
  {
	if( june < 0 )
	{
	  *aa= *aa* qm;
	  *bb= *bb* qm;
	  *cc= *cc* qm;
	}
	else
	{
	  *aa=- *aa* qp;
	  *bb= *bb* qp;
	  *cc=- *cc* qp;
	}

	if( i != is)
	  return(0);

  } /* if( june != 0 ) */

  *aa -= 1.0l;
  *bb += ( aj* qm+ ap* qp)* sdh/ sd;
  *cc += ( aj* qm- ap* qp)* cdh/ sd;

  return(0);
}

/*-----------------------------------------------------------------------*/

/* compute basis function i */
  int
tbf( int i, int icap )
{
  int ix, jcox, jcoxx, jend, iend, njun1=0, njun2, jsnop, jsnox;
  long double pp, sdh, cdh, sd, omc, aj, pm=0, cd, ap, qp, qm, xxi;
  long double d, sig; /*** also global ***/

  segj.jsno=0;
  pp=0.0l;
  ix = i-1;
  jcox= data.icon1[ix];

  if( jcox > PCHCON)
	jcox= i;

  jend=-1;
  iend=-1;
  sig=-1.0l;

  do
  {
	if( jcox != 0 )
	{
	  if( jcox < 0 )
		jcox=- jcox;
	  else
	  {
		sig=- sig;
		jend=- jend;
	  }

	  jcoxx = jcox-1;
	  segj.jsno++;
	  jsnox = segj.jsno-1;
	  segj.jco[jsnox]= jcox;
	  d= PI* data.si[jcoxx];
	  sdh= sinl( d);
	  cdh= cosl( d);
	  sd=2.0l* sdh* cdh;

	  if( d <= 0.015l)
	  {
		omc=4.0l* d* d;
		omc=((1.3888889e-3l* omc-4.1666666667e-2l)* omc+.5l)* omc;
	  }
	  else
		omc=1.0l- cdh* cdh+ sdh* sdh;

	  aj=1.0l/( logl(1.0l/( PI* data.bi[jcoxx]))-.577215664l);
	  pp= pp- omc/ sd* aj;
	  segj.ax[jsnox]= aj/ sd* sig;
	  segj.bx[jsnox]= aj/(2.0l* cdh);
	  segj.cx[jsnox]=- aj/(2.0l* sdh)* sig;

	  if( jcox != i)
	  {
		if( jend == 1)
		  jcox= data.icon2[jcoxx];
		else
		  jcox= data.icon1[jcoxx];

		if( abs(jcox) != i )
		{
		  if( jcox != 0 )
			continue;
		  else
		  {
			fprintf( stderr,
				"xnec2c: tbf() - segment connection"
				" error for segment %5d\n", i );
			stop( "Segment connection error", 1 );
		  }
		}

	  } /* if( jcox != i) */
	  else
		segj.bx[jsnox] =- segj.bx[jsnox];

	  if( iend == 1)
		break;

	} /* if( jcox != 0 ) */

	pm=- pp;
	pp=0.0l;
	njun1= segj.jsno;

	jcox= data.icon2[ix];
	if( jcox > PCHCON)
	  jcox= i;

	jend=1;
	iend=1;
	sig=-1.0l;

  } /* do */
  while( jcox != 0 );

  njun2= segj.jsno- njun1;
  jsnop= segj.jsno;
  segj.jco[jsnop]= i;
  d= PI* data.si[ix];
  sdh= sinl( d);
  cdh= cosl( d);
  sd=2.0l* sdh* cdh;
  cd= cdh* cdh- sdh* sdh;

  if( d <= 0.015l)
  {
	omc=4.0l* d* d;
	omc=((1.3888889e-3l* omc-4.1666666667e-2l)* omc+.5l)* omc;
  }
  else
	omc=1.0l- cd;

  ap=1.0l/( logl(1.0l/( PI* data.bi[ix]))-.577215664l);
  aj= ap;

  if( njun1 == 0)
  {
	if( njun2 == 0)
	{
	  segj.bx[jsnop]=0.0l;

	  if( icap == 0)
		xxi=0.0l;
	  else
	  {
		qp= PI* data.bi[ix];
		xxi= qp* qp;
		xxi= qp*(1.0l-.5l* xxi)/(1.0l- xxi);
	  }

	  segj.cx[jsnop]=1.0l/( cdh- xxi* sdh);
	  segj.jsno= jsnop+1;
	  segj.ax[jsnop]=-1.0l;
	  return(0);

	} /* if( njun2 == 0) */

	if( icap == 0)
	  xxi=0.0l;
	else
	{
	  qp= PI* data.bi[ix];
	  xxi= qp* qp;
	  xxi= qp*(1.0l-.5l* xxi)/(1.0l- xxi);
	}

	qp=-( omc+ xxi* sd)/( sd*( ap+ xxi* pp)+ cd*( xxi* ap- pp));
	d= cd- xxi* sd;
	segj.bx[jsnop]=( sdh+ ap* qp*( cdh- xxi* sdh))/ d;
	segj.cx[jsnop]=( cdh+ ap* qp*( sdh+ xxi* cdh))/ d;

	for( iend = 0; iend < njun2; iend++ )
	{
	  segj.ax[iend]=- segj.ax[iend]* qp;
	  segj.bx[iend]= segj.bx[iend]* qp;
	  segj.cx[iend]=- segj.cx[iend]* qp;
	}

	segj.jsno= jsnop+1;
	segj.ax[jsnop]=-1.0l;
	return(0);

  } /* if( njun1 == 0) */

  if( njun2 == 0)
  {
	if( icap == 0)
	  xxi=0.0l;
	else
	{
	  qm= PI* data.bi[ix];
	  xxi= qm* qm;
	  xxi= qm*(1.0l-.5l* xxi)/(1.0l- xxi);
	}

	qm=( omc+ xxi* sd)/( sd*( aj- xxi* pm)+ cd*( pm+ xxi* aj));
	d= cd- xxi* sd;
	segj.bx[jsnop]=( aj* qm*( cdh- xxi* sdh)- sdh)/ d;
	segj.cx[jsnop]=( cdh- aj* qm*( sdh+ xxi* cdh))/ d;

	for( iend = 0; iend < njun1; iend++ )
	{
	  segj.ax[iend]= segj.ax[iend]* qm;
	  segj.bx[iend]= segj.bx[iend]* qm;
	  segj.cx[iend]= segj.cx[iend]* qm;
	}

	segj.jsno= jsnop+1;
	segj.ax[jsnop]=-1.0l;
	return(0);

  } /* if( njun2 == 0) */

  qp= sd*( pm* pp+ aj* ap)+ cd*( pm* ap- pp* aj);
  qm=( ap* omc- pp* sd)/ qp;
  qp=-( aj* omc+ pm* sd)/ qp;
  segj.bx[jsnop]=( aj* qm+ ap* qp)* sdh/ sd;
  segj.cx[jsnop]=( aj* qm- ap* qp)* cdh/ sd;

  for( iend = 0; iend < njun1; iend++ )
  {
	segj.ax[iend]= segj.ax[iend]* qm;
	segj.bx[iend]= segj.bx[iend]* qm;
	segj.cx[iend]= segj.cx[iend]* qm;
  }

  jend= njun1;
  for( iend = jend; iend < segj.jsno; iend++ )
  {
	segj.ax[iend]=- segj.ax[iend]* qp;
	segj.bx[iend]= segj.bx[iend]* qp;
	segj.cx[iend]=- segj.cx[iend]* qp;
  }

  segj.jsno= jsnop+1;
  segj.ax[jsnop]=-1.0l;

  return(0);
}

/*-----------------------------------------------------------------------*/

/* compute the components of all basis functions on segment j */
  int
trio( int j )
{
  int jcox, jcoxx, jsnox, jx, jend=0, iend=0;

  segj.jsno=0;
  jx = j-1;
  jcox= data.icon1[jx];
  jcoxx = jcox-1;

  if( jcox <= PCHCON)
  {
	jend=-1;
	iend=-1;
  }

  if( (jcox == 0) || (jcox > PCHCON) )
  {
	jcox= data.icon2[jx];
	jcoxx = jcox-1;

	if( jcox <= PCHCON)
	{
	  jend=1;
	  iend=1;
	}

	if( jcox == 0 || (jcox > PCHCON) )
	{
	  jsnox = segj.jsno;
	  segj.jsno++;

	  /* Allocate to connections buffers */
	  if( segj.jsno >= segj.maxcon )
	  {
		segj.maxcon = segj.jsno +1;
		mem_realloc( (void *)&segj.jco,
			segj.maxcon * sizeof(int), "in calculations.c" );
		mem_realloc( (void *) &segj.ax,
			segj.maxcon * sizeof(long double), "in calculations.c" );
		mem_realloc( (void *) &segj.bx,
			segj.maxcon * sizeof(long double), "in calculations.c" );
		mem_realloc( (void *) &segj.cx,
			segj.maxcon * sizeof(long double), "in calculations.c" );
	  }

	  sbf( j, j, &segj.ax[jsnox], &segj.bx[jsnox], &segj.cx[jsnox]);
	  segj.jco[jsnox]= j;
	  return(0);
	}

  } /* if( (jcox == 0) || (jcox > PCHCON) ) */

  do
  {
	if( jcox < 0 )
	  jcox=- jcox;
	else
	  jend=- jend;
	jcoxx = jcox-1;

	if( jcox != j)
	{
	  jsnox = segj.jsno;
	  segj.jsno++;

	  /* Allocate to connections buffers */
	  if( segj.jsno >= segj.maxcon )
	  {
		segj.maxcon = segj.jsno +1;
		mem_realloc( (void *)&segj.jco,
			segj.maxcon * sizeof(int), "in calculations.c" );
		mem_realloc( (void *) &segj.ax,
			segj.maxcon * sizeof(long double), "in calculations.c" );
		mem_realloc( (void *) &segj.bx,
			segj.maxcon * sizeof(long double), "in calculations.c" );
		mem_realloc( (void *) &segj.cx,
			segj.maxcon * sizeof(long double), "in calculations.c" );
	  }

	  sbf( jcox, j, &segj.ax[jsnox], &segj.bx[jsnox], &segj.cx[jsnox]);
	  segj.jco[jsnox]= jcox;

	  if( jend != 1)
		jcox= data.icon1[jcoxx];
	  else
		jcox= data.icon2[jcoxx];

	  if( jcox == 0 )
	  {
		fprintf( stderr,
			"xnec2c: trio() - segment connention"
			" error for segment %5d\n", j );
		stop( "Segment connention error", 1 );
	  }
	  else
		continue;

	} /* if( jcox != j) */

	if( iend == 1)
	  break;

	jcox= data.icon2[jx];

	if( jcox > PCHCON)
	  break;

	jend=1;
	iend=1;

  } /* do */
  while( jcox != 0 );

  jsnox = segj.jsno;
  segj.jsno++;

  /* Allocate to connections buffers */
  if( segj.jsno >= segj.maxcon )
  {
	segj.maxcon = segj.jsno +1;
	mem_realloc( (void *)&segj.jco,
		segj.maxcon * sizeof(int), "in calculations.c" );
	mem_realloc( (void *) &segj.ax,
		segj.maxcon * sizeof(long double), "in calculations.c" );
	mem_realloc( (void *) &segj.bx,
		segj.maxcon * sizeof(long double), "in calculations.c" );
	mem_realloc( (void *) &segj.cx,
		segj.maxcon * sizeof(long double), "in calculations.c" );
  }

  sbf( j, j, &segj.ax[jsnox], &segj.bx[jsnox], &segj.cx[jsnox]);
  segj.jco[jsnox]= j;

  return(0);

}

/*-----------------------------------------------------------------------*/

/* cang returns the phase angle of a complex number in degrees. */
long double cang( complex long double z )
{
  return( cargl(z)*TD );
}

/*-----------------------------------------------------------------------*/

/* zint computes the internal impedance of a circular wire */
  void
zint( long double sigl, long double rolam, complex long double *zint )
{
#define cc1	 ( 6.0e-7l     + 1.9e-6lj)
#define cc2	 (-3.4e-6l     + 5.1e-6lj)
#define cc3	 (-2.52e-5l    + 0.0lj)
#define cc4	 (-9.06e-5l    - 9.01e-5lj)
#define cc5	 ( 0.0l        - 9.765e-4lj)
#define cc6	 (.0110486l    - .0110485lj)
#define cc7	 ( 0.0l        - .3926991lj)
#define cc8	 ( 1.6e-6l     - 3.2e-6lj)
#define cc9	 ( 1.17e-5l    - 2.4e-6lj)
#define cc10 ( 3.46e-5l    + 3.38e-5lj)
#define cc11 ( 5.0e-7l     + 2.452e-4lj)
#define cc12 (-1.3813e-3l  + 1.3811e-3lj)
#define cc13 (-6.25001e-2l - 1.0e-7lj)
#define cc14 (.7071068l    + .7071068lj)
#define cn	cc14

#define th(d) ( (((((cc1*(d)+cc2)*(d)+cc3)*(d)+cc4)*(d)+cc5)*(d)+cc6)*(d) + cc7 )
#define ph(d) ( (((((cc8*(d)+cc9)*(d)+cc10)*(d)+cc11)*(d)+cc12)*(d)+cc13)*(d)+cc14 )
#define f(d)  ( csqrtl(POT/(d))*cexpl(-cn*(d)+th(-8.0l/x)) )
#define g(d)  ( cexpl(cn*(d)+th(8.0l/x))/csqrtl(TP*(d)) )

  long double x, y, s, ber, bei;
  long double tpcmu = 2.368705e+3l;
  long double cmotp = 60.0l;
  complex long double br1, br2;

  x= sqrtl( tpcmu* sigl)* rolam;
  if( x <= 110.0l)
  {
	if( x <= 8.0l)
	{
	  y= x/8.0l;
	  y= y* y;
	  s= y* y;

	  ber=((((((-9.01e-6l* s+1.22552e-3l)* s-.08349609l)* s+ 2.6419140l)*
			  s-32.363456l)* s+113.77778l)* s-64.0l)* s+1.0l;

	  bei=((((((1.1346e-4l* s-.01103667l)* s+.52185615l)* s-10.567658l)*
			  s+72.817777l)* s-113.77778l)* s+16.0l)* y;

	  br1= cmplx( ber, bei);

	  ber=(((((((-3.94e-6l* s+4.5957e-4l)* s-.02609253l)* s+ .66047849l)*
				s-6.0681481l)* s+14.222222l)* s-4.0l)* y)* x;

	  bei=((((((4.609e-5l* s-3.79386e-3l)* s+.14677204l)* s- 2.3116751l)*
			  s+11.377778l)* s-10.666667l)* s+.5l)* x;

	  br2= cmplx( ber, bei);
	  br1= br1/ br2;
	  *zint= CPLX_01* sqrtl( cmotp/sigl )* br1/ rolam;

	} /* if( x <= 8.0l) */
	else
	{
	  br2= CPLX_01* f(x)/ PI;
	  br1= g( x)+ br2;
	  br2= g( x)* ph(8.0l/ x)- br2* ph(-8.0l/ x);
	  br1= br1/ br2;
	  *zint= CPLX_01* sqrtl( cmotp/ sigl)* br1/ rolam;
	}

  } /* if( x <= 110.0l) */
  else
  {
	br1= cmplx(.70710678l,-.70710678l);
	*zint= CPLX_01* sqrtl( cmotp/ sigl)* br1/ rolam;
  }
}

/*-----------------------------------------------------------------------*/
