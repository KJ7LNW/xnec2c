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

  Program NEC(input,tape5=input,output,tape11,tape12,tape13,tape14,
  tape15,tape16,tape20,tape21)

  Numerical Electromagnetics Code (NEC2)  developed at Lawrence
  Livermore lab., Livermore, CA.  (contact G. Burke at 415-422-8414
  for problems with the NEC code. For problems with the vax implem-
  entation, contact J. Breakall at 415-422-8196 or E. Domning at 415
  422-5936)
  file created 4/11/80.

 ***********Notice**********
 This computer code material was prepared as an account of work
 sponsored by the United States government.  Neither the United
 States nor the United States Department Of Energy, nor any of
 their employees, nor any of their contractors, subcontractors,
 or their employees, makes any warranty, express or implied, or
 assumes any legal liability or responsibility for the accuracy,
 completeness or usefulness of any information, apparatus, product
 or process disclosed, or represents that its use would not infringe
 privately-owned rights.

 ***********************************************************************/

#include "ground.h"
#include "shared.h"

/*-------------------------------------------------------------------*/

/* segment to obtain the total field due to ground.  the method of */
/* variable interval width romberg integration is used.  there are 9 */
/* field components - the x, y, and z components due to constant, */
/* sine, and cosine current distributions. */
  void
rom2( double a, double b,
	complex double *sum, double dmin )
{
  int i, ns, nt, flag = TRUE;
  int nts = 4, nx = 1, n = 9;
  double ze, ep, zend, dz=0.0, dzot=0.0, tmag1, tmag2, tr, ti;
  double z, s; /***also global***/
  double rx = 1.0e-4;
  complex double t00, t02, t11;
  static complex double *g1 = NULL, *g2 = NULL;
  static complex double *g3 = NULL, *g4 = NULL;
  static complex double *t01 = NULL, *t10 = NULL;
  static complex double *t20 = NULL, *g5 = NULL;
  static gboolean first_call = TRUE;

  if( first_call )
  {
	first_call = FALSE;
	size_t mreq = 9 * sizeof(complex double);
	mem_alloc( (void **)&g1, mreq, "in ground.c");
	mem_alloc( (void **)&g2, mreq, "in ground.c");
	mem_alloc( (void **)&g3, mreq, "in ground.c");
	mem_alloc( (void **)&g4, mreq, "in ground.c");
	mem_alloc( (void **)&g5, mreq, "in ground.c");
	mem_alloc( (void **)&t01, mreq, "in ground.c");
	mem_alloc( (void **)&t10, mreq, "in ground.c");
	mem_alloc( (void **)&t20, mreq, "in ground.c");
  }

  z= a;
  ze= b;
  s= b- a;

  if( s < 0.0)
  {
	fprintf( stderr, _("xnec2c: b less than a in rom2\n") );
	Stop( _("rom2(): b less than a"), ERR_STOP );
  }

  ep= s/(1.0e4* data.npm);
  zend= ze- ep;

  for( i = 0; i < n; i++ )
	sum[i]=CPLX_00;

  ns= nx;
  nt=0;
  sflds( z, g1);

  while( TRUE )
  {
	if( flag )
	{
	  dz= s/ ns;
	  if( z+ dz > ze)
	  {
		dz= ze- z;
		if( dz <= ep) return;
	  }

	  dzot= dz*.5;
	  sflds( z+ dzot, g3);
	  sflds( z+ dz, g5);

	} /* if( flag ) */

	tmag1=0.0;
	tmag2=0.0;

	/* evaluate 3 point romberg result and test convergence. */
	for( i = 0; i < n; i++ )
	{
	  t00=( g1[i]+ g5[i])* dzot;
	  t01[i]=( t00+ dz* g3[i])*.5;
	  t10[i]=(4.0* t01[i]- t00)/3.0;
	  if( i > 2)
		continue;

	  tr= creal( t01[i]);
	  ti= cimag( t01[i]);
	  tmag1= tmag1+ tr* tr+ ti* ti;
	  tr= creal( t10[i]);
	  ti= cimag( t10[i]);
	  tmag2= tmag2+ tr* tr+ ti* ti;

	} /* for( i = 0; i < n; i++ ) */

	tmag1= sqrt( tmag1);
	tmag2= sqrt( tmag2);
	test( tmag1, tmag2, &tr, 0.0, 0.0, &ti, dmin);

	if( tr <= rx)
	{
	  for( i = 0; i < n; i++ )
		sum[i] += t10[i];
	  nt += 2;

	  z += dz;
	  if( z > zend)
		return;

	  for( i = 0; i < n; i++ )
		g1[i]= g5[i];

	  if( (nt >= nts) && (ns > nx) )
	  {
		ns= ns/2;
		nt=1;
	  }
	  flag = TRUE;
	  continue;

	} /* if( tr <= rx) */

	sflds( z+ dz*.25, g2);
	sflds( z+ dz*.75, g4);
	tmag1=0.0;
	tmag2=0.0;

	/* evaluate 5 point romberg result and test convergence. */
	for( i = 0; i < n; i++ )
	{
	  t02=( t01[i]+ dzot*( g2[i]+ g4[i]))*.5;
	  t11=( 4.0 * t02- t01[i] )/3.0;
	  t20[i]=(16.0* t11- t10[i])/15.0;
	  if( i > 2)
		continue;

	  tr= creal( t11);
	  ti= cimag( t11);
	  tmag1= tmag1+ tr* tr+ ti* ti;
	  tr= creal( t20[i]);
	  ti= cimag( t20[i]);
	  tmag2= tmag2+ tr* tr+ ti* ti;

	} /* for( i = 0; i < n; i++ ) */

	tmag1= sqrt( tmag1);
	tmag2= sqrt( tmag2);
	test( tmag1, tmag2, &tr, 0.0,0.0, &ti, dmin);

	if( tr > rx)
	{
	  nt=0;
	  if( ns < data.npm )
	  {
		ns= ns*2;
		dz= s/ ns;
		dzot= dz*.5;

		for( i = 0; i < n; i++ )
		{
		  g5[i]= g3[i];
		  g3[i]= g2[i];
		}

		flag=FALSE;
		continue;

	  } /* if( ns < npm) */

	  fprintf( stderr,
		  _("xnec2c: rom2 -- step size limited at z = %12.5E\n"), z );

	} /* if( tr > rx) */

	for( i = 0; i < n; i++ )
	  sum[i]= sum[i]+ t20[i];
	nt= nt+1;

	z= z+ dz;
	if( z > zend)
	  return;

	for( i = 0; i < n; i++ )
	  g1[i]= g5[i];

	flag = TRUE;
	if( (nt < nts) || (ns <= nx) )
	  continue;

	ns= ns/2;
	nt=1;

  } /* while( TRUE ) */

}

/*-----------------------------------------------------------------------*/

/* sfldx returns the field due to ground for a current element on */
/* the source segment at t relative to the segment center. */
  void
sflds( double t, complex double *e )
{
  double xt, yt, zt, rhx, rhy, rhs, rho, phx, phy;
  double cph, sph, zphs, r2s, rk, sfac, thet;
  complex double eph, erv, ezv, erh, ezh;
  complex double er, et, hrv, hzv, hrh;

  xt= dataj.xj+ t* dataj.cabj;
  yt= dataj.yj+ t* dataj.sabj;
  zt= dataj.zj+ t* dataj.salpj;
  rhx= incom.xo- xt;
  rhy= incom.yo- yt;
  rhs= rhx* rhx+ rhy* rhy;
  rho= sqrt( rhs);

  if( rho <= 0.0)
  {
	rhx=1.0;
	rhy=0.0;
	phx=0.0;
	phy=1.0;
  }
  else
  {
	rhx= rhx/ rho;
	rhy= rhy/ rho;
	phx= -rhy;
	phy= rhx;
  }

  cph= rhx* incom.xsn+ rhy* incom.ysn;
  sph= rhy* incom.xsn- rhx* incom.ysn;

  if( fabs( cph) < 1.0e-10)
	cph=0.0;
  if( fabs( sph) < 1.0e-10)
	sph=0.0;

  gwav.zph= incom.zo+ zt;
  zphs= gwav.zph* gwav.zph;
  r2s= rhs+ zphs;
  gwav.r2= sqrt( r2s);
  rk= gwav.r2* M_2PI;
  gwav.xx2= cmplx( cos( rk),- sin( rk));

  /* use norton approximation for field due to ground.
   * current is lumped at segment center with current moment
   * for constant, sine or cosine distribution. */
  if( incom.isnor != 1)
  {
	gwav.zmh=1.0;
	gwav.r1=1.0;
	gwav.xx1=0.0;
	gwave( &erv, &ezv, &erh, &ezh, &eph);

	et=-CONST1* gnd.frati* gwav.xx2/( r2s* gwav.r2);
	er=2.0* et* cmplx(1.0, rk);
	et= et* cmplx(1.0 - rk* rk, rk);
	hrv=( er+ et)* rho* gwav.zph/ r2s;
	hzv=( zphs* er- rhs* et)/ r2s;
	hrh=( rhs* er- zphs* et)/ r2s;
	erv= erv- hrv;
	ezv= ezv- hzv;
	erh= erh+ hrh;
	ezh= ezh+ hrv;
	eph= eph+ et;
	erv= erv* dataj.salpj;
	ezv= ezv* dataj.salpj;
	erh= erh* incom.sn* cph;
	ezh= ezh* incom.sn* cph;
	eph= eph* incom.sn* sph;
	erh= erv+ erh;
	e[0]=( erh* rhx+ eph* phx)* dataj.s;
	e[1]=( erh* rhy+ eph* phy)* dataj.s;
	e[2]=( ezv+ ezh)* dataj.s;
	e[3]=0.0;
	e[4]=0.0;
	e[5]=0.0;
	sfac= M_PI* dataj.s;
	sfac= sin( sfac)/ sfac;
	e[6]= e[0]* sfac;
	e[7]= e[1]* sfac;
	e[8]= e[2]* sfac;

	return;
  } /* if( smat.isnor != 1) */

  /* interpolate in sommerfeld field tables */
  if( rho >= 1.0e-12)
	thet= atan( gwav.zph/ rho);
  else
	thet= M_PI_2;

  /* combine vertical and horizontal components and convert */
  /* to x,y,z components. multiply by exp(-jkr)/r. */
  intrp( gwav.r2, thet, &erv, &ezv, &erh, &eph );
  gwav.xx2= gwav.xx2/ gwav.r2;
  sfac= incom.sn* cph;
  erh= gwav.xx2*( dataj.salpj* erv+ sfac* erh);
  ezh= gwav.xx2*( dataj.salpj* ezv- sfac* erv);
  /* x,y,z fields for constant current */
  eph= incom.sn* sph* gwav.xx2* eph;
  e[0]= erh* rhx+ eph* phx;
  e[1]= erh* rhy+ eph* phy;
  e[2]= ezh;
  /* x,y,z fields for sine current */
  rk= M_2PI* t;
  sfac= sin( rk);
  e[3]= e[0]* sfac;
  e[4]= e[1]* sfac;
  /* x,y,z fields for cosine current */
  e[5]= e[2]* sfac;
  sfac= cos( rk);
  e[6]= e[0]* sfac;
  e[7]= e[1]* sfac;
  e[8]= e[2]* sfac;

  return;
}

/*-----------------------------------------------------------------------*/

