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

#include "calculations.h"
#include "shared.h"

/*-------------------------------------------------------------------*/

/* compute basis function i */
  static void
tbf( int i, int icap )
{
  int ix, jcox, jcoxx, jend, iend, njun1=0, njun2, jsnop, jsnox;
  double pp, sdh, cdh, sd, omc, aj, pm=0, cd, ap, qp, qm, xxi;
  double d, sig; /*** also global ***/

  segj.jsno=0;
  pp=0.0;
  ix = i-1;
  jcox= data.icon1[ix];

  if( jcox > PCHCON)
	jcox= i;

  jend=-1;
  iend=-1;
  sig=-1.0;

  do
  {
	if( jcox != 0 )
	{
	  if( jcox < 0 )
		jcox= -jcox;
	  else
	  {
		sig= -sig;
		jend= -jend;
	  }

	  jcoxx = jcox-1;
	  segj.jsno++;
	  jsnox = segj.jsno-1;
	  segj.jco[jsnox]= jcox;
	  d= M_PI* data.si[jcoxx];
	  sdh= sin( d);
	  cdh= cos( d);
	  sd=2.0* sdh* cdh;

	  if( d <= 0.015)
	  {
		omc=4.0* d* d;
		omc=((1.3888889e-3* omc-4.1666666667e-2)* omc+.5)* omc;
	  }
	  else omc=1.0- cdh* cdh+ sdh* sdh;

	  aj=1.0/( log(1.0/( M_PI* data.bi[jcoxx]))-.577215664);
	  pp= pp- omc/ sd* aj;
	  segj.ax[jsnox]= aj/ sd* sig;
	  segj.bx[jsnox]= aj/(2.0* cdh);
	  segj.cx[jsnox]= -aj/(2.0* sdh)* sig;

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
				_("xnec2c: tbf(): segment connection error for segment %5d\n"), i );
			Stop( _("Segment connection error in tbf()"), ERR_STOP );
		  }
		}

	  } /* if( jcox != i) */
	  else segj.bx[jsnox] = -segj.bx[jsnox];

	  if( iend == 1) break;

	} /* if( jcox != 0 ) */

	pm= -pp;
	pp=0.0;
	njun1= segj.jsno;

	jcox= data.icon2[ix];
	if( jcox > PCHCON)
	  jcox= i;

	jend=1;
	iend=1;
	sig=-1.0;

  } /* do */
  while( jcox != 0 );

  njun2= segj.jsno- njun1;
  jsnop= segj.jsno;
  segj.jco[jsnop]= i;
  d= M_PI* data.si[ix];
  sdh= sin( d);
  cdh= cos( d);
  sd=2.0* sdh* cdh;
  cd= cdh* cdh- sdh* sdh;

  if( d <= 0.015)
  {
	omc=4.0* d* d;
	omc=((1.3888889e-3* omc-4.1666666667e-2)* omc+.5)* omc;
  }
  else omc=1.0- cd;

  ap=1.0/( log(1.0/( M_PI* data.bi[ix]))-.577215664);
  aj= ap;

  if( njun1 == 0)
  {
	if( njun2 == 0)
	{
	  segj.bx[jsnop]=0.0;

	  if( icap == 0)
		xxi=0.0;
	  else
	  {
		qp= M_PI* data.bi[ix];
		xxi= qp* qp;
		xxi= qp*(1.0-.5* xxi)/(1.0- xxi);
	  }

	  segj.cx[jsnop]=1.0/( cdh- xxi* sdh);
	  segj.jsno= jsnop+1;
	  segj.ax[jsnop]=-1.0;
	  return;

	} /* if( njun2 == 0) */

	if( icap == 0) xxi=0.0;
	else
	{
	  qp= M_PI* data.bi[ix];
	  xxi= qp* qp;
	  xxi= qp*(1.0-.5* xxi)/(1.0- xxi);
	}

	qp=-( omc+ xxi* sd)/( sd*( ap+ xxi* pp)+ cd*( xxi* ap- pp));
	d= cd- xxi* sd;
	segj.bx[jsnop]=( sdh+ ap* qp*( cdh- xxi* sdh))/ d;
	segj.cx[jsnop]=( cdh+ ap* qp*( sdh+ xxi* cdh))/ d;

	for( iend = 0; iend < njun2; iend++ )
	{
	  segj.ax[iend]= -segj.ax[iend]* qp;
	  segj.bx[iend]= segj.bx[iend]* qp;
	  segj.cx[iend]= -segj.cx[iend]* qp;
	}

	segj.jsno= jsnop+1;
	segj.ax[jsnop]=-1.0;
	return;

  } /* if( njun1 == 0) */

  if( njun2 == 0)
  {
	if( icap == 0)
	  xxi=0.0;
	else
	{
	  qm= M_PI* data.bi[ix];
	  xxi= qm* qm;
	  xxi= qm*(1.0-.5* xxi)/(1.0- xxi);
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
	segj.ax[jsnop]=-1.0;
	return;

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
	segj.ax[iend]= -segj.ax[iend]* qp;
	segj.bx[iend]= segj.bx[iend]* qp;
	segj.cx[iend]= -segj.cx[iend]* qp;
  }

  segj.jsno= jsnop+1;
  segj.ax[jsnop]=-1.0;

  return;
}

/*-----------------------------------------------------------------------*/

/* fill incident field array for charge discontinuity voltage source */
void qdsrc( int is, complex double v, complex double *e )
{
  int i, jx, j, jp1, ipr, ij, i1;
  double xi, yi, zi, ai, cabi, sabi, salpi, tx, ty, tz;
  complex double curd, etk, ets, etc;

  is--;
  i= data.icon1[is];
  data.icon1[is]=0;
  tbf( is+1,0);
  data.icon1[is]= i;
  dataj.s= data.si[is]*.5;
  curd= CCJ* v/(( log(2.0 * dataj.s/ data.bi[is])-1.0) *
	  ( segj.bx[segj.jsno-1] * cos( M_2PI* dataj.s) +
		segj.cx[segj.jsno-1] * sin( M_2PI* dataj.s))* data.wlam);
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
		ipr= -ipr;
		ipr--;
		if( -data.icon1[ipr-1] != jp1 )
		  dataj.ind1=2;
		else
		{
		  xi= fabs( dataj.cabj* data.cab[ipr]+ dataj.sabj*
			  data.sab[ipr]+ dataj.salpj* data.salp[ipr]);
		  if( (xi < 0.999999) ||
			  (fabs(data.bi[ipr]/dataj.b-1.0) > 1.0e-6) )
			dataj.ind1=2;
		  else
			dataj.ind1=0;
		}
	  }  /* if( ipr < 0 ) */
	  else if( ipr == 0 )
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
			xi= fabs( dataj.cabj* data.cab[ipr]+ dataj.sabj*
				data.sab[ipr]+ dataj.salpj* data.salp[ipr]);
			if( (xi < 0.999999) ||
				(fabs(data.bi[ipr]/dataj.b-1.0) > 1.0e-6) )
			  dataj.ind1=2;
			else
			  dataj.ind1=0;
		  }
		} /* if( ipr != j ) */
		else
		{
		  if( (dataj.cabj*dataj.cabj + dataj.sabj*dataj.sabj) > 1.0e-8)
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
		  xi= fabs( dataj.cabj* data.cab[ipr]+ dataj.sabj *
			  data.sab[ipr]+ dataj.salpj* data.salp[ipr]);
		  if( (xi < 0.999999) ||
			  (fabs(data.bi[ipr]/dataj.b-1.0) > 1.0e-6) )
			dataj.ind1=2;
		  else
			dataj.ind1=0;
		}
	  } /* if( ipr < 0 ) */
	  else if( ipr == 0 )
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
			xi= fabs( dataj.cabj* data.cab[ipr]+ dataj.sabj*
				data.sab[ipr]+ dataj.salpj* data.salp[ipr]);
			if( (xi < 0.9999990) ||
				(fabs(data.bi[ipr]/dataj.b-1.0) > 1.0e-6) )
			  dataj.ind2=2;
			else
			  dataj.ind2=0;
		  }
		} /* if( ipr != j )*/
		else
		{
		  if( (dataj.cabj* dataj.cabj + dataj.sabj* dataj.sabj) > 1.0e-8)
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
		hsfld( xi, yi, zi, 0.0);
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
void cabc( complex double *curx)
{
  int i, is, j, jx, jco1, jco2;
  double ar, ai, sh;
  complex double curd, cs1, cs2;

  if( data.n != 0)
  {
	for( i = 0; i < data.n; i++ )
	{
	  crnt.air[i]=0.0;
	  crnt.aii[i]=0.0;
	  crnt.bir[i]=0.0;
	  crnt.bii[i]=0.0;
	  crnt.cir[i]=0.0;
	  crnt.cii[i]=0.0;
	}

	for( i = 0; i < data.n; i++ )
	{
	  ar= creal( curx[i]);
	  ai= cimag( curx[i]);
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
		sh= data.si[i]*.5;
		curd= CCJ* vsorc.vqds[is]/( (log(2.0* sh/ data.bi[i])-1.0) *
			(segj.bx[segj.jsno-1]* cos(M_2PI* sh)+ segj.cx[segj.jsno-1] *
			 sin(M_2PI* sh))* data.wlam );
		ar= creal( curd);
		ai= cimag( curd);

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
double db10( double x )
{
  if( x < 1.0e-20 )
	return( -999.99 );

  return( 10.0 * log10(x) );
}

/*-----------------------------------------------------------------------*/

/* function db20 returns db for mag**2 (power) i */
double db20( double x )
{
  if( x < 1.0e-20 )
	return( -999.99 );

  return( 20.0 * log10(x) );
}

/*-----------------------------------------------------------------------*/

/* intrp uses bivariate cubic interpolation to obtain */
/* the values of 4 functions at the point (x,y). */
void intrp( double x, double y, complex double *f1,
	complex double *f2, complex double *f3, complex double *f4 )
{
  static int ix, iy, ixs=-10, iys=-10, igrs=-10, ixeg=0, iyeg=0;
  static int nxm2, nym2, nxms, nyms, nd, ndp;
  static int *nda = NULL, *ndpa = NULL;
  int jump;
  static double dx = 1.0, dy = 1.0, xs = 0.0, ys = 0.0, xz, yz;
  double xx, yy;
  static complex double a[4][4], b[4][4], c[4][4], d[4][4];
  complex double p1=CPLX_00, p2=CPLX_00, p3=CPLX_00, p4=CPLX_00;
  complex double fx1, fx2, fx3, fx4;
  static gboolean first_call = TRUE;

  if( first_call )
  {
	first_call = FALSE;
	size_t mreq = 3*sizeof(int);
	mem_alloc( (void **)&nda,  mreq, _("in calculations.c"));
	mem_alloc( (void **)&ndpa, mreq, _("in calculations.c"));
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
	int igr, iadd, iadz, i, k;

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

		a[i][k]=( p4- p1+3.0*( p2- p3))*.1666666667;
		b[i][k]=( p1-2.0* p2+ p3)*.5;
		c[i][k]= p3-(2.0* p1+3.0* p2+ p4)*.1666666667;
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
  p1= fx4- fx1+3.0*( fx2- fx3);
  p2=3.0*( fx1-2.0* fx2+ fx3);
  p3=6.0* fx3-2.0* fx1-3.0* fx2- fx4;
  *f1=(( p1* yy+ p2)* yy+ p3)* yy*.1666666667+ fx2;
  fx1=(( a[0][1]* xx+ b[0][1])* xx+ c[0][1])* xx+ d[0][1];
  fx2=(( a[1][1]* xx+ b[1][1])* xx+ c[1][1])* xx+ d[1][1];
  fx3=(( a[2][1]* xx+ b[2][1])* xx+ c[2][1])* xx+ d[2][1];
  fx4=(( a[3][1]* xx+ b[3][1])* xx+ c[3][1])* xx+ d[3][1];
  p1= fx4- fx1+3.0*( fx2- fx3);
  p2=3.0*( fx1-2.0* fx2+ fx3);
  p3=6.0* fx3-2.0* fx1-3.0* fx2- fx4;
  *f2=(( p1* yy+ p2)* yy+ p3)* yy*.1666666667+ fx2;
  fx1=(( a[0][2]* xx+ b[0][2])* xx+ c[0][2])* xx+ d[0][2];
  fx2=(( a[1][2]* xx+ b[1][2])* xx+ c[1][2])* xx+ d[1][2];
  fx3=(( a[2][2]* xx+ b[2][2])* xx+ c[2][2])* xx+ d[2][2];
  fx4=(( a[3][2]* xx+ b[3][2])* xx+ c[3][2])* xx+ d[3][2];
  p1= fx4- fx1+3.0*( fx2- fx3);
  p2=3.0*( fx1-2.0* fx2+ fx3);
  p3=6.0* fx3-2.0* fx1-3.0* fx2- fx4;
  *f3=(( p1* yy+ p2)* yy+ p3)* yy*.1666666667+ fx2;
  fx1=(( a[0][3]* xx+ b[0][3])* xx+ c[0][3])* xx+ d[0][3];
  fx2=(( a[1][3]* xx+ b[1][3])* xx+ c[1][3])* xx+ d[1][3];
  fx3=(( a[2][3]* xx+ b[2][3])* xx+ c[2][3])* xx+ d[2][3];
  fx4=(( a[3][3]* xx+ b[3][3])* xx+ c[3][3])* xx+ d[3][3];
  p1= fx4- fx1+3.0*( fx2- fx3);
  p2=3.0*( fx1-2.0* fx2+ fx3);
  p3=6.0* fx3-2.0* fx1-3.0* fx2- fx4;
  *f4=(( p1* yy+ p2)* yy+ p3)* yy*.1666666667+ fx2;

  return;
}

/*-----------------------------------------------------------------------*/

/* intx performs numerical integration of exp(jkr)/r by the method of */
/* variable interval width romberg integration.  the integrand value */
/* is supplied by subroutine gf. */
void intx( double el1, double el2, double b,
	int ij, double *sgr, double *sgi )
{
  int ns, nt;
  int nx = 1, nma = 65536, nts = 4;
  int flag = TRUE;
  double z, s, ze, fnm, ep, zend, fns, dz=0.0, zp;
  double t00r, g1r, g5r=0.0, t00i, g1i, g5i=0.0, t01r, g3r=0.0;
  double t01i, g3i=0.0, t10r, t10i, te1i, te1r, t02r;
  double g2r, g4r, t02i, g2i, g4i, t11r, t11i, t20r;
  double  t20i, te2i, te2r, rx = 1.0e-4, dzot=0.0;

  z= el1;
  ze= el2;
  if( ij == 0)
	ze=0.0;
  s= ze- z;
  fnm= nma;
  ep= s/(10.0* fnm);
  zend= ze- ep;
  *sgr=0.0;
  *sgi=0.0;
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
		if( fabs(dz) <= ep)
		{
		  /* add contribution of near singularity for diagonal term */
		  if(ij == 0)
		  {
			*sgr=2.0*( *sgr+ log(( sqrt( b* b+ s* s)+ s)/ b));
			*sgi=2.0* *sgi;
		  }
		  return;
		}

	  } /* if( zp > ze) */

	  dzot= dz*.5;
	  zp= z+ dzot;
	  gf( zp, &g3r, &g3i);
	  zp= z+ dz;
	  gf( zp, &g5r, &g5i);

	} /* if( flag ) */

	t00r=( g1r+ g5r)* dzot;
	t00i=( g1i+ g5i)* dzot;
	t01r=( t00r+ dz* g3r)*0.5;
	t01i=( t00i+ dz* g3i)*0.5;
	t10r=(4.0* t01r- t00r)/3.0;
	t10i=(4.0* t01i- t00i)/3.0;

	/* test convergence of 3 point romberg result. */
	test( t01r, t10r, &te1r, t01i, t10i, &te1i, 0.0);
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
		  *sgr=2.0*( *sgr+ log(( sqrt( b* b+ s* s)+ s)/ b));
		  *sgi=2.0* *sgi;
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

	zp= z+ dz*0.25;
	gf( zp, &g2r, &g2i);
	zp= z+ dz*0.75;
	gf( zp, &g4r, &g4i);
	t02r=( t01r+ dzot*( g2r+ g4r))*0.5;
	t02i=( t01i+ dzot*( g2i+ g4i))*0.5;
	t11r=(4.0* t02r- t01r)/3.0;
	t11i=(4.0* t02i- t01i)/3.0;
	t20r=(16.0* t11r- t10r)/15.0;
	t20i=(16.0* t11i- t10i)/15.0;

	/* test convergence of 5 point romberg result. */
	test( t11r, t20r, &te2r, t11i, t20i, &te2i, 0.0);
	if( (te2i > rx) || (te2r > rx) )
	{
	  nt=0;
	  if( ns >= nma)
		fprintf( stderr,
			_("xnec2c: step size limited at z= %10.5f\n"), z );
	  else
	  {
		/* halve step size */
		ns= ns*2;
		fns= ns;
		dz= s/ fns;
		dzot= dz*0.5;
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
		*sgr=2.0*( *sgr+ log(( sqrt( b* b+ s* s)+ s)/ b));
		*sgi=2.0* *sgi;
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
	return( a );
  else
	return( b );
}

/*-----------------------------------------------------------------------*/

/* test for convergence in numerical integration */
void test( double f1r, double f2r, double *tr,
	double f1i, double f2i, double *ti, double dmin )
{
  double den;

  den= fabs( f2r);
  *tr= fabs( f2i);

  if( den < *tr)
	den= *tr;
  if( den < dmin)
	den= dmin;

  if( den < 1.0e-37)
  {
	*tr=0.0;
	*ti=0.0;
	return;
  }

  *tr= fabs(( f1r- f2r)/ den);
  *ti= fabs(( f1i- f2i)/ den);

  return;
}

/*-----------------------------------------------------------------------*/

/* compute component of basis function i on segment is. */
  static void
sbf( int i, int is, double *aa, double *bb, double *cc )
{
  int ix, jsno, june, jcox, jcoxx, jend, iend, njun1=0, njun2;
  double d, sig, pp, sdh, cdh, sd, omc;
  double aj, pm=0, cd, ap, qp, qm, xxi;

  *aa=0.0;
  *bb=0.0;
  *cc=0.0;
  june=0;
  jsno=0;
  pp=0.0;
  ix=i-1;

  jcox= data.icon1[ix];
  if( jcox > PCHCON)
	jcox= i;

  jend=-1;
  iend=-1;
  sig=-1.0;

  do
  {
	if( jcox != 0 )
	{
	  if( jcox < 0 )
		jcox= -jcox;
	  else
	  {
		sig= -sig;
		jend= -jend;
	  }

	  jcoxx = jcox-1;
	  jsno++;
	  d= M_PI* data.si[jcoxx];
	  sdh= sin( d);
	  cdh= cos( d);
	  sd=2.0* sdh* cdh;

	  if( d <= 0.015)
	  {
		omc=4.0* d* d;
		omc=((1.3888889e-3* omc -4.1666666667e-2)* omc +.5)* omc;
	  }
	  else omc=1.0- cdh* cdh+ sdh* sdh;

	  aj=1.0/( log(1.0/( M_PI* data.bi[jcoxx]))-.577215664);
	  pp -= omc/ sd* aj;

	  if( jcox == is)
	  {
		*aa= aj/ sd* sig;
		*bb= aj/(2.0* cdh);
		*cc= -aj/(2.0* sdh)* sig;
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
				_("xnec2c: sbf(): segment connection error for segment %d\n"), i );
			Stop( _("Segment connection error in sbf()"), ERR_STOP );
		  }
		  else continue;
		}

	  } /* if( jcox != i ) */
	  else if( jcox == is)
		  *bb= -*bb;

	  if( iend == 1) break;

	} /* if( jcox != 0 ) */

	pm= -pp;
	pp=0.0;
	njun1= jsno;

	jcox= data.icon2[ix];
	if( jcox > PCHCON)
	  jcox= i;

	jend=1;
	iend=1;
	sig=-1.0;

  } /* do */
  while( jcox != 0 );

  njun2= jsno- njun1;
  d= M_PI* data.si[ix];
  sdh= sin( d);
  cdh= cos( d);
  sd=2.0* sdh* cdh;
  cd= cdh* cdh- sdh* sdh;

  if( d <= 0.015)
  {
	omc=4.0* d* d;
	omc=((1.3888889e-3* omc -4.1666666667e-2)* omc +.5)* omc;
  }
  else omc=1.0- cd;

  ap=1.0/( log(1.0/( M_PI* data.bi[ix])) -.577215664);
  aj= ap;

  if( njun1 == 0)
  {
	if( njun2 == 0)
	{
	  *aa =-1.0;
	  qp= M_PI* data.bi[ix];
	  xxi= qp* qp;
	  xxi= qp*(1.0-.5* xxi)/(1.0- xxi);
	  *cc=1.0/( cdh- xxi* sdh);
	  return;
	}

	qp= M_PI* data.bi[ix];
	xxi= qp* qp;
	xxi= qp*(1.0-.5* xxi)/(1.0- xxi);
	qp=-( omc+ xxi* sd)/( sd*( ap+ xxi* pp)+ cd*( xxi* ap- pp));

	if( june == 1)
	{
	  *aa= -*aa* qp;
	  *bb=  *bb* qp;
	  *cc= -*cc* qp;
	  if( i != is)
		return;
	}

	*aa -= 1.0;
	d = cd - xxi * sd;
	*bb += (sdh + ap * qp * (cdh - xxi * sdh)) / d;
	*cc += (cdh + ap * qp * (sdh + xxi * cdh)) / d;
	return;

  } /* if( njun1 == 0) */

  if( njun2 == 0)
  {
	qm= M_PI* data.bi[ix];
	xxi= qm* qm;
	xxi= qm*(1.0-.5* xxi)/(1.0- xxi);
	qm=( omc+ xxi* sd)/( sd*( aj- xxi* pm)+ cd*( pm+ xxi* aj));

	if( june == -1)
	{
	  *aa= *aa* qm;
	  *bb= *bb* qm;
	  *cc= *cc* qm;
	  if( i != is)
		return;
	}

	*aa -= 1.0;
	d= cd- xxi* sd;
	*bb += ( aj* qm*( cdh- xxi* sdh)- sdh)/ d;
	*cc += ( cdh- aj* qm*( sdh+ xxi* cdh))/ d;
	return;

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
	  *aa= -*aa* qp;
	  *bb= *bb* qp;
	  *cc= -*cc* qp;
	}

	if( i != is)
	  return;

  } /* if( june != 0 ) */

  *aa -= 1.0;
  *bb += ( aj* qm+ ap* qp)* sdh/ sd;
  *cc += ( aj* qm- ap* qp)* cdh/ sd;

  return;
}

/*-----------------------------------------------------------------------*/

/* compute the components of all basis functions on segment j */
  void
trio( int j )
{
  int jcox, jcoxx, jsnox, jx, jend=0, iend=0;

  segj.jsno=0;
  jx = j-1;
  jcox= data.icon1[jx];

  if( jcox <= PCHCON)
  {
	jend=-1;
	iend=-1;
  }

  if( (jcox == 0) || (jcox > PCHCON) )
  {
	jcox= data.icon2[jx];

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
		size_t mreq = (size_t)segj.maxcon * sizeof(int);
		mem_realloc( (void **)&segj.jco, mreq, _("in calculations.c") );
		mreq = (size_t)segj.maxcon * sizeof(double);
		mem_realloc( (void **) &segj.ax, mreq, _("in calculations.c") );
		mem_realloc( (void **) &segj.bx, mreq, _("in calculations.c") );
		mem_realloc( (void **) &segj.cx, mreq, _("in calculations.c") );
	  }

	  sbf( j, j, &segj.ax[jsnox], &segj.bx[jsnox], &segj.cx[jsnox]);
	  segj.jco[jsnox]= j;
	  return;
	}

  } /* if( (jcox == 0) || (jcox > PCHCON) ) */

  do
  {
	if( jcox < 0 )
	  jcox= -jcox;
	else
	  jend= -jend;
	jcoxx = jcox-1;

	if( jcox != j)
	{
	  jsnox = segj.jsno;
	  segj.jsno++;

	  /* Allocate to connections buffers */
	  if( segj.jsno >= segj.maxcon )
	  {
		segj.maxcon = segj.jsno +1;
		size_t mreq = (size_t)segj.maxcon * sizeof(int);
		mem_realloc( (void **)&segj.jco, mreq, _("in calculations.c") );
		mreq = (size_t)segj.maxcon * sizeof(double);
		mem_realloc( (void **) &segj.ax, mreq, _("in calculations.c") );
		mem_realloc( (void **) &segj.bx, mreq, _("in calculations.c") );
		mem_realloc( (void **) &segj.cx, mreq, _("in calculations.c") );
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
			_("xnec2c: trio(): segment connention error for segment %5d\n"), j );
		Stop( _("Segment connention error in trio()"), ERR_STOP );
	  }
	  else continue;

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
	size_t mreq = (size_t)segj.maxcon * sizeof(int);
	mem_realloc( (void **)&segj.jco, mreq, _("in calculations.c") );
	mreq = (size_t)segj.maxcon * sizeof(double);
	mem_realloc( (void **) &segj.ax, mreq, _("in calculations.c") );
	mem_realloc( (void **) &segj.bx, mreq, _("in calculations.c") );
	mem_realloc( (void **) &segj.cx, mreq, _("in calculations.c") );
  }

  sbf( j, j, &segj.ax[jsnox], &segj.bx[jsnox], &segj.cx[jsnox]);
  segj.jco[jsnox]= j;

  return;
}

/*-----------------------------------------------------------------------*/

/* cang returns the phase angle of a complex number in degrees. */
double cang( complex double z )
{
  return( carg(z)*TODEG );
}

/*-----------------------------------------------------------------------*/

/* zint computes the internal impedance of a circular wire */
  void
zint( double sigl, double rolam, complex double *zint )
{
#define cc1	 ( 6.0e-7     + I*1.9e-6)
#define cc2	 (-3.4e-6     + I*5.1e-6)
#define cc3	 (-2.52e-5    + I*0.0)
#define cc4	 (-9.06e-5    - I*9.01e-5)
#define cc5	 ( 0.0        - I*9.765e-4)
#define cc6	 (.0110486    - I*0.0110485)
#define cc7	 ( 0.0        - I*0.3926991)
#define cc8	 ( 1.6e-6     - I*3.2e-6)
#define cc9	 ( 1.17e-5    - I*2.4e-6)
#define cc10 ( 3.46e-5    + I*3.38e-5)
#define cc11 ( 5.0e-7     + I*2.452e-4)
#define cc12 (-1.3813e-3  + I*1.3811e-3)
#define cc13 (-6.25001e-2 - I*1.0e-7)
#define cc14 (.7071068    + I*0.7071068)
#define cn	cc14

#define th(d) ( (((((cc1*(d)+cc2)*(d)+cc3)*(d)+cc4)*(d)+cc5)*(d)+cc6)*(d) + cc7 )
#define ph(d) ( (((((cc8*(d)+cc9)*(d)+cc10)*(d)+cc11)*(d)+cc12)*(d)+cc13)*(d)+cc14 )
#define f(d)  ( csqrt(M_PI_2/(d))*cexp(-cn*(d)+th(-8.0/x)) )
#define g(d)  ( cexp(cn*(d)+th(8.0/x))/csqrt(M_2PI*(d)) )

  double x;
  double tpcmu = 2.368705e+3;
  double cmotp = 60.0;
  complex double br1, br2;

  x= sqrt( tpcmu* sigl)* rolam;
  if( x <= 110.0)
  {
	if( x <= 8.0)
	{
	  double y, s, ber, bei;

	  y= x/8.0;
	  y= y* y;
	  s= y* y;

	  ber=((((((-9.01e-6* s+1.22552e-3)* s-.08349609)* s+ 2.6419140)*
			  s-32.363456)* s+113.77778)* s-64.0)* s+1.0;

	  bei=((((((1.1346e-4* s-.01103667)* s+.52185615)* s-10.567658)*
			  s+72.817777)* s-113.77778)* s+16.0)* y;

	  br1= cmplx( ber, bei);

	  ber=(((((((-3.94e-6* s+4.5957e-4)* s-.02609253)* s+ .66047849)*
				s-6.0681481)* s+14.222222)* s-4.0)* y)* x;

	  bei=((((((4.609e-5* s-3.79386e-3)* s+.14677204)* s- 2.3116751)*
			  s+11.377778)* s-10.666667)* s+.5)* x;

	  br2= cmplx( ber, bei);
	  br1= br1/ br2;
	  *zint= CPLX_01* sqrt( cmotp/sigl )* br1/ rolam;

	} /* if( x <= 8.0) */
	else
	{
	  br2= CPLX_01* f(x)/ M_PI;
	  br1= g( x)+ br2;
	  br2= g( x)* ph(8.0/ x)- br2* ph(-8.0/ x);
	  br1= br1/ br2;
	  *zint= CPLX_01* sqrt( cmotp/ sigl)* br1/ rolam;
	}

  } /* if( x <= 110.0) */
  else
  {
	br1= cmplx(.70710678,-.70710678);
	*zint= CPLX_01* sqrt( cmotp/ sigl)* br1/ rolam;
  }
}

/*-----------------------------------------------------------------------*/
