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

#include "radiation.h"
#include "shared.h"

/* Radiation pattern data */

/*-----------------------------------------------------------------------*/

/* calculates the xyz components of the electric */
/* field due to surface currents */
static void
fflds( double rox, double roy, double roz,
	complex double *scur, complex double *ex,
	complex double *ey, complex double *ez )
{
  double *xs, *ys, *zs, *s;
  int j, i, k;
  double arg;
  complex double ct;

  xs = data.px;
  ys = data.py;
  zs = data.pz;
  s = data.pbi;

  *ex=CPLX_00;
  *ey=CPLX_00;
  *ez=CPLX_00;

  i= -1;
  for( j = 0; j < data.m; j++ )
  {
	i++;
	arg= M_2PI*( rox* xs[i]+ roy* ys[i]+ roz* zs[i]);
	ct= cmplx( cos( arg)* s[i], sin( arg)* s[i]);
	k=3*j;
	*ex += scur[k  ]* ct;
	*ey += scur[k+1]* ct;
	*ez += scur[k+2]* ct;
  }

  ct= rox* *ex+ roy* *ey+ roz* *ez;
  *ex= CONST4*( ct* rox- *ex);
  *ey= CONST4*( ct* roy- *ey);
  *ez= CONST4*( ct* roz- *ez);

  return;
}

/*-----------------------------------------------------------------------*/

/* ffld calculates the far zone radiated electric fields, */
/* the factor exp(j*k*r)/(r/lamda) not included */
static void
ffld( double thet, double phi,
	complex double *eth, complex double *eph )
{
  int k, i, ip, jump;
  double phx, phy, roz, rozs, thx, thy, thz, rox, roy;
  double tthet=0.0, darg=0.0, omega, el, sill, top, bot, a;
  double too, boo, b, c, d, rr, ri, arg, dr, rfl, rrz;
  complex double cix=CPLX_00, ciy=CPLX_00, ciz=CPLX_00, ccx=CPLX_00;
  complex double ccy=CPLX_00, ccz=CPLX_00, exa, cdp;
  complex double zrsin, rrv=CPLX_00, rrh=CPLX_00, rrv1=CPLX_00;
  complex double rrh1=CPLX_00, rrv2=CPLX_00, rrh2=CPLX_00;
  complex double tix, tiy, tiz, zscrn, ex=CPLX_00;
  complex double ey=CPLX_00, ez=CPLX_00, gx, gy, gz;

  phx= -sin( phi);
  phy= cos( phi);
  roz= cos( thet);
  rozs= roz;
  thx= roz* phy;
  thy= -roz* phx;
  thz= -sin( thet);
  rox= -thz* phy;
  roy= thz* phx;

  jump = FALSE;
  if( data.n != 0)
  {
	/* loop for structure image if any */
	/* calculation of reflection coeffecients */
	for( k = 0; k < gnd.ksymp; k++ )
	{
	  if( k != 0 )
	  {
		/* for perfect ground */
		if( gnd.iperf == 1)
		{
		  rrv=-CPLX_10;
		  rrh=-CPLX_10;
		}
		else
		{
		  /* for infinite planar ground */
		  zrsin= csqrt(1.0- gnd.zrati* gnd.zrati* thz* thz);
		  rrv=-( roz- gnd.zrati* zrsin)/( roz+ gnd.zrati* zrsin);
		  rrh=( gnd.zrati* roz- zrsin)/( gnd.zrati* roz+ zrsin);
		} /* if( gnd.iperf == 1) */

		/* for the cliff problem, two reflection coefficients calculated */
		if( gnd.ifar > 1)
		{
		  rrv1= rrv;
		  rrh1= rrh;
		  tthet= tan( thet);

		  if( gnd.ifar != 4)
		  {
			zrsin= csqrt(1.0- gnd.zrati2* gnd.zrati2* thz* thz);
			rrv2=-( roz- gnd.zrati2* zrsin)/( roz+ gnd.zrati2* zrsin);
			rrh2=( gnd.zrati2* roz- zrsin)/( gnd.zrati2* roz+ zrsin);
			darg= -M_2PI*2.0* gnd.ch* roz;
		  }
		} /* if( gnd.ifar > 1) */

		roz= -roz;
		ccx= cix;
		ccy= ciy;
		ccz= ciz;

	  } /* if( k != 0 ) */

	  cix=CPLX_00;
	  ciy=CPLX_00;
	  ciz=CPLX_00;

	  /* loop over structure segments */
	  for( i = 0; i < data.n; i++ )
	  {
		omega=-( rox* data.cab[i] +	roy* data.sab[i]+ roz* data.salp[i]);
		el= M_PI* data.si[i];
		sill= omega* el;
		top= el+ sill;
		bot= el- sill;

		if( fabs( omega) >= 1.0e-7)
		  a=2.0* sin( sill)/ omega;
		else
		  a=(2.0- omega* omega* el* el/3.0)* el;

		if( fabs( top) >= 1.0e-7)
		  too= sin( top)/ top;
		else
		  too=1.0- top* top/6.0;

		if( fabs( bot) >= 1.0e-7)
		  boo= sin( bot)/ bot;
		else
		  boo=1.0- bot* bot/6.0;

		b= el*( boo- too);
		c= el*( boo+ too);
		rr= a* crnt.air[i]+ b* crnt.bii[i]+ c* crnt.cir[i];
		ri= a* crnt.aii[i]- b* crnt.bir[i]+ c* crnt.cii[i];
		arg= M_2PI*( data.x[i]* rox+ data.y[i]* roy+ data.z[i]* roz);

		if( (k != 1) || (gnd.ifar < 2) )
		{
		  /* summation for far field integral */
		  exa= cmplx( cos( arg), sin( arg))* cmplx( rr, ri);
		  cix= cix+ exa* data.cab[i];
		  ciy= ciy+ exa* data.sab[i];
		  ciz= ciz+ exa* data.salp[i];
		  continue;
		}

		/* calculation of image contribution */
		/* in cliff and ground screen problems */

		/* specular point distance */
		dr= data.z[i]* tthet;
		d= dr* phy+ data.x[i];
		if( gnd.ifar == 2)
		{
		  if(( gnd.cl- d) > 0.0)
		  {
			rrv= rrv1;
			rrh= rrh1;
		  }
		  else
		  {
			rrv= rrv2;
			rrh= rrh2;
			arg= arg+ darg;
		  }
		} /* if( gnd.ifar == 2) */
		else
		{
		  d= sqrt( d*d + (data.y[i]-dr*phx)*(data.y[i]-dr*phx) );
		  if( gnd.ifar == 3)
		  {
			if(( gnd.cl- d) > 0.0)
			{
			  rrv= rrv1;
			  rrh= rrh1;
			}
			else
			{
			  rrv= rrv2;
			  rrh= rrh2;
			  arg= arg+ darg;
			}
		  } /* if( gnd.ifar == 3) */
		  else
		  {
			if(( gnd.scrwl- d) >= 0.0)
			{
			  /* radial wire ground screen reflection coefficient */
			  d= d+ gnd.t2;
			  zscrn= gnd.t1* d* log( d/ gnd.t2);
			  zscrn=( zscrn* gnd.zrati)/( ETA* gnd.zrati+ zscrn);
			  zrsin= csqrt(1.0- zscrn* zscrn* thz* thz);
			  rrv=( roz+ zscrn* zrsin)/(- roz+ zscrn* zrsin);
			  rrh=( zscrn* roz+ zrsin)/( zscrn* roz- zrsin);
			} /* if(( gnd.scrwl- d) < 0.0) */
			else
			{
			  if( gnd.ifar == 4)
			  {
				rrv= rrv1;
				rrh= rrh1;
			  } /* if( gnd.ifar == 4) */
			  else
			  {
				if( gnd.ifar == 5)
				  d= dr* phy+ data.x[i];

				if(( gnd.cl- d) > 0.0)
				{
				  rrv= rrv1;
				  rrh= rrh1;
				}
				else
				{
				  rrv= rrv2;
				  rrh= rrh2;
				  arg= arg+ darg;
				} /* if(( gnd.cl- d) > 0.0) */

			  } /* if( gnd.ifar == 4) */

			} /* if(( gnd.scrwl- d) < 0.0) */

		  } /* if( gnd.ifar == 3) */

		} /* if( gnd.ifar == 2) */

		/* contribution of each image segment modified by */
		/* reflection coef, for cliff and ground screen problems */
		exa= cmplx( cos( arg), sin( arg))* cmplx( rr, ri);
		tix= exa* data.cab[i];
		tiy= exa* data.sab[i];
		tiz= exa* data.salp[i];
		cdp=( tix* phx+ tiy* phy)*( rrh- rrv);
		cix= cix+ tix* rrv+ cdp* phx;
		ciy= ciy+ tiy* rrv+ cdp* phy;
		ciz= ciz- tiz* rrv;

	  } /* for( i = 0; i < n; i++ ) */

	  if( k == 0 )
		continue;

	  /* calculation of contribution of
	   * structure image for infinite ground */
	  if( gnd.ifar < 2)
	  {
		cdp=( cix* phx+ ciy* phy)*( rrh- rrv);
		cix= ccx+ cix* rrv+ cdp* phx;
		ciy= ccy+ ciy* rrv+ cdp* phy;
		ciz= ccz- ciz* rrv;
	  }
	  else
	  {
		cix= cix+ ccx;
		ciy= ciy+ ccy;
		ciz= ciz+ ccz;
	  }

	} /* for( k=0; k < gnd.ksymp; k++ ) */

	if( data.m > 0)
	  jump = TRUE;
	else
	{
	  *eth = ( cix * thx + ciy * thy + ciz * thz ) * CONST3;
	  *eph = ( cix * phx + ciy * phy ) * CONST3;
	  return;
	}

  } /* if( n != 0) */

  if( ! jump )
  {
	cix=CPLX_00;
	ciy=CPLX_00;
	ciz=CPLX_00;
  }

  /* electric field components */
  roz= rozs;
  rfl=-1.0;
  for( ip = 0; ip < gnd.ksymp; ip++ )
  {
	rfl= -rfl;
	rrz= roz* rfl;
	fflds( rox, roy, rrz, &crnt.cur[data.n], &gx, &gy, &gz);

	if( ip != 1 )
	{
	  ex= gx;
	  ey= gy;
	  ez= gz;
	  continue;
	}

	if( gnd.iperf == 1)
	{
	  gx= -gx;
	  gy= -gy;
	  gz= -gz;
	}
	else
	{
	  rrv= csqrt(1.0- gnd.zrati* gnd.zrati* thz* thz);
	  rrh= gnd.zrati* roz;
	  rrh=( rrh- rrv)/( rrh+ rrv);
	  rrv= gnd.zrati* rrv;
	  rrv=-( roz- rrv)/( roz+ rrv);
	  *eth=( gx* phx+ gy* phy)*( rrh- rrv);
	  gx= gx* rrv+ *eth* phx;
	  gy= gy* rrv+ *eth* phy;
	  gz= gz* rrv;

	} /* if( gnd.iperf == 1) */

	ex= ex+ gx;
	ey= ey+ gy;
	ez= ez- gz;

  } /* for( ip = 0; ip < gnd.ksymp; ip++ ) */

  ex = ex + cix * CONST3;
  ey = ey + ciy * CONST3;
  ez = ez + ciz * CONST3;

  *eth = ex * thx + ey * thy + ez * thz;
  *eph = ex * phx + ey * phy;

  return;
}

/*-----------------------------------------------------------------------*/

/* gfld computes the radiated field including ground wave. */
static void
gfld( double rho, double phi, double rz,
	complex double *eth, complex double *epi,
	complex double *erd, complex double ux, int ksymp )
{
  int i, k;
  double b, r, thet, arg, phx, phy, rx, ry;
  double dx, dy, dz, rix, riy, rhs, rhp;
  double rhx, rhy, calp, cbet, sbet, cph;
  double sph, el, rfl, riz, thx, thy, thz;
  double rxyz, rnx, rny, rnz, omega, sill;
  double top, bot, a, too, boo, c, rr, ri;
  complex double cix, ciy, ciz, exa, erv;
  complex double ezv, erh, eph, ezh, ex, ey;

  r= sqrt( rho*rho+ rz*rz );
  if( (ksymp == 1) || (cabs(ux) > .5) || (r > 1.0e5) )
  {
	/* computation of space wave only */
	if( rz >= 1.0e-20)
	  thet= atan( rho/ rz);
	else
	  thet= M_PI*.5;

	ffld( thet, phi, eth, epi);
	arg= -M_2PI* r;
	exa= cmplx( cos( arg), sin( arg))/ r;
	*eth= *eth* exa;
	*epi= *epi* exa;
	*erd=CPLX_00;
	return;
  } /* if( (ksymp == 1) && (cabs(ux) > .5) && (r > 1.0e5) ) */

  /* computation of space and ground waves. */
  gwav.u= ux;
  gwav.u2= gwav.u* gwav.u;
  phx= -sin( phi);
  phy= cos( phi);
  rx= rho* phy;
  ry= -rho* phx;
  cix=CPLX_00;
  ciy=CPLX_00;
  ciz=CPLX_00;

  /* summation of field from individual segments */
  for( i = 0; i < data.n; i++ )
  {
	dx= data.cab[i];
	dy= data.sab[i];
	dz= data.salp[i];
	rix= rx- data.x[i];
	riy= ry- data.y[i];
	rhs= rix* rix+ riy* riy;
	rhp= sqrt( rhs);

	if( rhp >= 1.0e-6)
	{
	  rhx= rix/ rhp;
	  rhy= riy/ rhp;
	}
	else
	{
	  rhx=1.0;
	  rhy=0.0;
	}

	calp=1.0- dz* dz;
	if( calp >= 1.0e-6)
	{
	  calp= sqrt( calp);
	  cbet= dx/ calp;
	  sbet= dy/ calp;
	  cph= rhx* cbet+ rhy* sbet;
	  sph= rhy* cbet- rhx* sbet;
	}
	else
	{
	  cph= rhx;
	  sph= rhy;
	}

	el= M_PI* data.si[i];
	rfl=-1.0;

	/* integration of (current)*(phase factor)
	 * over segment and image for constant,
	 * sine, and cosine current distributions */
	for( k = 0; k < 2; k++ )
	{
	  rfl= -rfl;
	  riz= rz- data.z[i]* rfl;
	  rxyz= sqrt( rix* rix+ riy* riy+ riz* riz);
	  rnx= rix/ rxyz;
	  rny= riy/ rxyz;
	  rnz= riz/ rxyz;
	  omega=-( rnx* dx+ rny* dy+ rnz* dz* rfl);
	  sill= omega* el;
	  top= el+ sill;
	  bot= el- sill;

	  if( fabs( omega) >= 1.0e-7)
		a=2.0* sin( sill)/ omega;
	  else
		a=(2.0- omega* omega* el* el/3.0)* el;

	  if( fabs( top) >= 1.0e-7)
		too= sin( top)/ top;
	  else
		too=1.0- top* top/6.0;

	  if( fabs( bot) >= 1.0e-7)
		boo= sin( bot)/ bot;
	  else
		boo=1.0- bot* bot/6.0;

	  b= el*( boo- too);
	  c= el*( boo+ too);
	  rr= a* crnt.air[i] +
		b* crnt.bii[i]+ c* crnt.cir[i];
	  ri= a* crnt.aii[i] -
		b* crnt.bir[i]+ c* crnt.cii[i];
	  arg= M_2PI*( data.x[i] *
		  rnx+ data.y[i]* rny+ data.z[i]* rnz* rfl);
	  exa= cmplx( cos( arg), sin( arg))* cmplx( rr, ri)/ M_2PI;

	  if( k != 1 )
	  {
		gwav.xx1= exa;
		gwav.r1= rxyz;
		gwav.zmh= riz;
		continue;
	  }

	  gwav.xx2= exa;
	  gwav.r2= rxyz;
	  gwav.zph= riz;

	} /* for( k = 0; k < 2; k++ ) */

	/* call subroutine to compute the field */
	/* of segment including ground wave. */
	gwave( &erv, &ezv, &erh, &ezh, &eph);
	erh= erh* cph* calp+ erv* dz;
	eph= eph* sph* calp;
	ezh= ezh* cph* calp+ ezv* dz;
	ex= erh* rhx- eph* rhy;
	ey= erh* rhy+ eph* rhx;
	cix= cix+ ex;
	ciy= ciy+ ey;
	ciz= ciz+ ezh;

  } /* for( i = 0; i < n; i++ ) */

  arg= -M_2PI* r;
  exa= cmplx( cos( arg), sin( arg));
  cix= cix* exa;
  ciy= ciy* exa;
  ciz= ciz* exa;
  rnx= rx/ r;
  rny= ry/ r;
  rnz= rz/ r;
  thx= rnz* phy;
  thy= -rnz* phx;
  thz= -rho/ r;
  *eth= cix* thx+ ciy* thy+ ciz* thz;
  *epi= cix* phx+ ciy* phy;
  *erd= cix* rnx+ ciy* rny+ ciz* rnz;

  return;
}

/*-----------------------------------------------------------------------*/

/* compute radiation pattern, gain, normalized gain */
void
rdpat( void )
{
  int kth, kph, isens;
  double  prad, gcon, gcop;
  double phi, pha, thet;
  double tha, ethm2, ethm;
  double etha, ephm2, ephm, epha, tilta, emajr2, eminr2;
  double dfaz, axrat, dfaz2, cdfaz, tstor1=0.0, tstor2;
  double gnmn, stilta, gnmj, gnv, gnh, gtot;
  complex double eth, eph, erd;
  int idx, pol; /* Gain buffer and pol type index */
  double gain;


  if( gnd.ifar != 4 )
  {
	gnd.cl= fpat.clt/ data.wlam;
	gnd.ch= fpat.cht/ data.wlam;
	gnd.zrati2= csqrt(1.0/ cmplx(fpat.epsr2,- fpat.sig2* data.wlam*59.96));
  }

  /* Calculate radiation pattern data */
  /*** For applied voltage excitation ***/
  if( (fpat.ixtyp == 0) || (fpat.ixtyp == 5) )
  {
	gcop= data.wlam* data.wlam* M_2PI/(376.73* fpat.pinr);
	prad= fpat.pinr- fpat.ploss- fpat.pnlr;
	gcon= gcop;
	if( fpat.ipd != 0)
	  gcon *= fpat.pinr/ prad;
  }
  else if( fpat.ixtyp == 4) /*** For elementary current source ***/
  {
	fpat.pinr=394.510* calc_data.xpr6*
	  calc_data.xpr6* data.wlam* data.wlam;
	gcop= data.wlam* data.wlam*M_2PI/(376.73* fpat.pinr);
	prad= fpat.pinr- fpat.ploss- fpat.pnlr;
	gcon= gcop;
	if( fpat.ipd != 0)
	  gcon= gcon* fpat.pinr/ prad;
  }
  else gcon=4.0* M_PI/(1.0+ calc_data.xpr6* calc_data.xpr6);
  /*** Incident field source ***/

  phi  = fpat.phis - fpat.dph;

  /*** Save radiation pattern data ***/
  /* Prime max and min gains and index */
  for( pol = 0; pol < NUM_POL; pol++ )
  {
	rad_pattern[calc_data.fstep].max_gain[pol] = -10000.0;
	rad_pattern[calc_data.fstep].min_gain[pol] =  10000.0;
	rad_pattern[calc_data.fstep].max_gain_idx[pol] = 0;
	rad_pattern[calc_data.fstep].min_gain_idx[pol] = 0;
	rad_pattern[calc_data.fstep].max_gain_tht[pol] = 0;
	rad_pattern[calc_data.fstep].max_gain_phi[pol] = 0;
  }

  /* Signal new rad pattern data */
  SetFlag( DRAW_NEW_RDPAT );

  /* Step over theta and phi angles */
  idx = 0;
  for( kph = 1; kph <= fpat.nph; kph++ )
  {
	phi += fpat.dph;
	pha= phi* TORAD;
	thet= fpat.thets - fpat.dth;

	for( kth = 1; kth <= fpat.nth; kth++ )
	{
	  thet += fpat.dth;

	  if( (gnd.ksymp == 2) && (thet > 90.01) && (gnd.ifar != 1) )
	  {
		Gtk_Widget_Destroy( rdpattern_window );
		fprintf( stderr, "xnec2c: rdpat(): Theta > 90 deg with ground specified\n"
			"Please check RP card data and correct\n" );
		Stop( _("rdpat(): Theta > 90 deg with ground specified\n"
			  "Please check RP card data and correct"), ERR_STOP );
	  }

	  tha= thet* TORAD;
	  if( gnd.ifar != 1)
		ffld( tha, pha, &eth, &eph);
	  else
	  {
		gfld( fpat.rfld/data.wlam, pha, thet/data.wlam,
			&eth, &eph, &erd, gnd.zrati, gnd.ksymp);
	  }

	  ethm2= creal( eth* conj( eth));
	  ethm= sqrt( ethm2);
	  etha= cang( eth);
	  ephm2= creal( eph* conj( eph));
	  ephm= sqrt( ephm2);
	  epha= cang( eph);

	  /* elliptical polarization calc. */
	  if( gnd.ifar != 1)
	  {
		if( (ethm2 <= 1.0e-20) && (ephm2 <= 1.0e-20) )
		{
		  tilta=0.0;
		  emajr2=0.0;
		  eminr2=0.0;
		  axrat=0.0;
		  isens= 0;
		}
		else
		{
		  dfaz= epha- etha;
		  if( epha >= 0.0)
			dfaz2= dfaz-360.0;
		  else
			dfaz2= dfaz+360.0;

		  if( fabs(dfaz) > fabs(dfaz2) )
			dfaz= dfaz2;

		  cdfaz= cos( dfaz* TORAD);
		  tstor1= ethm2- ephm2;
		  tstor2=2.0* ephm* ethm* cdfaz;
		  tilta=atan2( tstor2, tstor1)/2.0;
		  stilta= sin( tilta);
		  tstor1= tstor1* stilta* stilta;
		  tstor2= tstor2* stilta* cos( tilta);
		  emajr2= -tstor1+ tstor2+ ethm2;
		  eminr2= tstor1- tstor2+ ephm2;
		  if( eminr2 < 0.0)	eminr2=0.0;

		  axrat= sqrt( eminr2/ emajr2);
		  if( axrat <= 1.0e-5)
			isens= 1;
		  else if( dfaz <= 0.0)
			isens= 2;
		  else
			isens= 3;

		} /* if( (ethm2 <= 1.0e-20) && (ephm2 <= 1.0e-20) ) */

		gnmj= db10( gcon* emajr2);
		gnmn= db10( gcon* eminr2);
		gnv = db10( gcon* ethm2);
		gnh = db10( gcon* ephm2);
		gtot= db10( gcon* (ethm2+ ephm2) );

		switch( fpat.inor )
		{
		  case 0:
			tstor1= gtot;
			break;

		  case 1:
			tstor1= gnmj;
			break;

		  case 2:
			tstor1= gnmn;
			break;

		  case 3:
			tstor1= gnv;
			break;

		  case 4:
			tstor1= gnh;
			break;

		  case 5:
			tstor1= gtot;
		}

		/* Save rad pattern gains */
		rad_pattern[calc_data.fstep].gtot[idx] = tstor1;

		/* Save axial ratio, tilt and pol sense */
		if( isens == 2 )
		  rad_pattern[calc_data.fstep].axrt[idx] = -axrat;
		else
		  rad_pattern[calc_data.fstep].axrt[idx] = axrat;
		rad_pattern[calc_data.fstep].tilt[idx] = tilta;
		rad_pattern[calc_data.fstep].sens[idx] = isens;

		/* Find and save max value of gain and direction */
		for( pol = 0; pol < NUM_POL; pol++ )
		{
		  gain = rad_pattern[calc_data.fstep].gtot[idx] +
			Polarization_Factor( pol, calc_data.fstep, idx);
		  if( gain < -999.99 ) gain = -999.99;

		  /* Find and save max value of gain and direction */
		  if( rad_pattern[calc_data.fstep].max_gain[pol] < gain )
		  {
			rad_pattern[calc_data.fstep].max_gain[pol]     = gain;
			rad_pattern[calc_data.fstep].max_gain_tht[pol] = thet;
			rad_pattern[calc_data.fstep].max_gain_phi[pol] = phi;
			rad_pattern[calc_data.fstep].max_gain_idx[pol] = idx;
		  }

		  /* Find and save min value of gain and buffer idx */
		  if( rad_pattern[calc_data.fstep].min_gain[pol] > gain )
		  {
			rad_pattern[calc_data.fstep].min_gain[pol]     = gain;
			rad_pattern[calc_data.fstep].min_gain_idx[pol] = idx;
		  }

		} /* for( pol = 0; pol < NUM_POL; pol++ ) */

		idx++;
		continue;
	  } /* if( gnd.ifar != 1) */

	} /* for( kth = 1; kth <= fpat.nth; kth++ ) */
  } /* for( kph = 1; kph <= fpat.nph; kph++ ) */

  return;

} /* void rdpat() */

/*-----------------------------------------------------------------------*/

