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

#include "network.h"
#include "shared.h"

/*-------------------------------------------------------------------*/

/* subroutine netwk solves for structure currents for a given */
/* excitation including the effect of non-radiating networks if */
/* present. */
void
netwk( complex double *cmx, int *ip, complex double *einc )
{
  int *ipnt = NULL, *nteqa = NULL, *ntsca = NULL;
  int nteq=0, ntsc=0, j, ndimn;
  int neqt, irow1=0, i, isc1=0;
  double pwr;
  complex double *vsrc = NULL, *rhs = NULL, *cmn = NULL;
  complex double *rhnt = NULL, *rhnx = NULL, ymit, vlt, cux;
  size_t mreq;

  netcx.pin=0.0;
  netcx.pnls=0.0;
  neqt= netcx.neq+ netcx.neq2;
  ndimn = j = (2*netcx.nonet + vsorc.nsant);

  /* Allocate network buffers */
  if( netcx.nonet > 0 )
  {
	mreq = (size_t)data.np3m * sizeof(complex double);
	mem_alloc( (void **)&rhs, mreq, "in network.c");

	mreq = (size_t)j * sizeof(complex double);
	mem_alloc( (void **)&rhnt, mreq, "in network.c");
	mem_alloc( (void **)&rhnx, mreq, "in network.c");
	mem_alloc( (void **)&cmn,  mreq * (size_t)j, "in network.c");

	mreq = (size_t)j * sizeof(int);
	mem_alloc( (void **)&ntsca, mreq, "in network.c");
	mem_alloc( (void **)&nteqa, mreq, "in network.c");
	mem_alloc( (void **)&ipnt,  mreq, "in network.c");

	mreq = (size_t)vsorc.nsant * sizeof(complex double);
	mem_alloc( (void **)&vsrc, mreq, "in network.c");
  }
  else if( netcx.masym != 0)
  {
	mreq = (size_t)j * sizeof(int);
	mem_alloc( (void **)&ipnt,  mreq, "in network.c");
  }

  /* Signal new and valid current data */
  crnt.newer = crnt.valid = 1;

  if( netcx.ntsol == 0)
  {
	int nseg1;

	/* compute relative matrix asymmetry */
	if( netcx.masym != 0)
	{
	  irow1=0;
	  if( netcx.nonet > 0)
	  {
		for( i = 0; i < netcx.nonet; i++ )
		{
		  nseg1= netcx.iseg1[i];
		  for( isc1 = 0; isc1 < 2; isc1++ )
		  {
			if( irow1 == 0)
			{
			  ipnt[irow1]= nseg1;
			  nseg1= netcx.iseg2[i];
			  irow1++;
			  continue;
			}

			for( j = 0; j < irow1; j++ )
			  if( nseg1 == ipnt[j])
				break;

			if( j == irow1 )
			{
			  ipnt[irow1]= nseg1;
			  irow1++;
			}

			nseg1= netcx.iseg2[i];

		  } /* for( isc1 = 0; isc1 < 2; isc1++ ) */

		} /* for( i = 0; i < netcx.nonet; i++ ) */

	  } /* if( netcx.nonet > 0) */

	  if( vsorc.nsant != 0)
	  {
		for( i = 0; i < vsorc.nsant; i++ )
		{
		  nseg1= vsorc.isant[i];
		  if( irow1 == 0)
		  {
			ipnt[irow1]= nseg1;
			irow1++;
			continue;
		  }

		  for( j = 0; j < irow1; j++ )
			if( nseg1 == ipnt[j])
			  break;

		  if( j == irow1 )
		  {
			ipnt[irow1]= nseg1;
			irow1++;
		  }

		} /* for( i = 0; i < vsorc.nsant; i++ ) */

	  } /* if( vsorc.nsant != 0) */

	  if( irow1 >= 2 )
	  {
		double asmx;

		for( i = 0; i < irow1; i++ )
		{
		  isc1= ipnt[i]-1;
		  asmx= data.si[isc1];

		  for( j = 0; j < neqt; j++ )
			rhs[j] = CPLX_00;

		  rhs[isc1] = CPLX_10;
		  solves( cmx, ip, rhs, netcx.neq, 1,
			  data.np, data.n, data.mp, data.m);
		  cabc( rhs);

		  for( j = 0; j < irow1; j++ )
		  {
			isc1= ipnt[j]-1;
			cmn[j+i*ndimn]= rhs[isc1]/ asmx;
		  }

		} /* for( i = 0; i < irow1; i++ ) */

		asmx=0.0;

		for( i = 1; i < irow1; i++ )
		{
		  isc1= i;
		  for( j = 0; j < isc1; j++ )
		  {
			cux= cmn[i+j*ndimn];
			pwr= cabs(( cux- cmn[j+i*ndimn])/ cux);

			if( pwr < asmx)
			  continue;

			asmx= pwr;
			nteq= ipnt[i];
			ntsc= ipnt[j];

		  } /* for( j = 0; j < isc1; j++ ) */

		} /* for( i = 1; i < irow1; i++ ) */

	  } /* if( irow1 >= 2) */

	} /* if( netcx.masym != 0) */

	/* solution of network equations */
	if( netcx.nonet > 0)
	{
	  for( i = 0; i < ndimn; i++ )
	  {
		rhnx[i]=CPLX_00;
		for( j = 0; j < ndimn; j++ )
		  cmn[j+i*ndimn]=CPLX_00;
	  }

	  /* sort network and source data and */
	  /* assign equation numbers to segments */
	  nteq=0;
	  ntsc=0;

	  for( j = 0; j < netcx.nonet; j++ )
	  {
		int  jump1, jump2, nseg2, isc2=0, irow2=0;
		double y11r, y11i, y12r, y12i, y22r, y22i;

		nseg1= netcx.iseg1[j];
		nseg2= netcx.iseg2[j];

		if( netcx.ntyp[j] <= 1)
		{
		  y11r= netcx.x11r[j];
		  y11i= netcx.x11i[j];
		  y12r= netcx.x12r[j];
		  y12i= netcx.x12i[j];
		  y22r= netcx.x22r[j];
		  y22i= netcx.x22i[j];
		}
		else
		{
		  y22r= M_2PI* netcx.x11i[j]/ data.wlam;
		  y12r=0.0;
		  y12i=1.0/( netcx.x11r[j]* sin( y22r));
		  y11r= netcx.x12r[j];
		  y11i= -y12i* cos( y22r);
		  y22r= netcx.x22r[j];
		  y22i= y11i+ netcx.x22i[j];
		  y11i= y11i+ netcx.x12i[j];

		  if( netcx.ntyp[j] != 2)
		  {
			y12r= -y12r;
			y12i= -y12i;
		  }

		} /* if( netcx.ntyp[j] <= 1) */

		jump1 = FALSE;
		if( vsorc.nsant != 0)
		{
		  for( i = 0; i < vsorc.nsant; i++ )
			if( nseg1 == vsorc.isant[i])
			{
			  isc1 = i;
			  jump1 = TRUE;
			  break;
			}
		} /* if( vsorc.nsant != 0) */

		jump2 = FALSE;
		if( ! jump1 )
		{
		  isc1=-1;

		  if( nteq != 0)
		  {
			for( i = 0; i < nteq; i++ )
			  if( nseg1 == nteqa[i])
			  {
				irow1 = i;
				jump2 = TRUE;
				break;
			  }

		  } /* if( nteq != 0) */

		  if( ! jump2 )
		  {
			irow1= nteq;
			nteqa[nteq]= nseg1;
			nteq++;
		  }

		} /* if( ! jump1 ) */
		else
		{
		  if( ntsc != 0)
		  {
			for( i = 0; i < ntsc; i++ )
			{
			  if( nseg1 == ntsca[i])
			  {
				irow1 = ndimn- (i+1);
				jump2 = TRUE;
				break;
			  }
			}

		  } /* if( ntsc != 0) */

		  if( ! jump2 )
		  {
			irow1= ndimn- (ntsc+1);
			ntsca[ntsc]= nseg1;
			vsrc[ntsc]= vsorc.vsant[isc1];
			ntsc++;
		  }

		} /* if( ! jump1 ) */

		jump1 = FALSE;
		if( vsorc.nsant != 0)
		{
		  for( i = 0; i < vsorc.nsant; i++ )
		  {
			if( nseg2 == vsorc.isant[i])
			{
			  isc2= i;
			  jump1 = TRUE;
			  break;
			}
		  }

		} /* if( vsorc.nsant != 0) */

		jump2 = FALSE;
		if( ! jump1 )
		{
		  isc2=-1;

		  if( nteq != 0)
		  {
			for( i = 0; i < nteq; i++ )
			  if( nseg2 == nteqa[i])
			  {
				irow2= i;
				jump2 = TRUE;
				break;
			  }

		  } /* if( nteq != 0) */

		  if( ! jump2 )
		  {
			irow2= nteq;
			nteqa[nteq]= nseg2;
			nteq++;
		  }

		}  /* if( ! jump1 ) */
		else
		{
		  if( ntsc != 0)
		  {
			for( i = 0; i < ntsc; i++ )
			  if( nseg2 == ntsca[i])
			  {
				irow2 = ndimn- (i+1);
				jump2 = TRUE;
				break;
			  }

		  } /* if( ntsc != 0) */

		  if( ! jump2 )
		  {
			irow2= ndimn- (ntsc+1);
			ntsca[ntsc]= nseg2;
			vsrc[ntsc]= vsorc.vsant[isc2];
			ntsc++;
		  }

		} /* if( ! jump1 ) */

		/* fill network equation matrix and right hand side vector with */
		/* network short-circuit admittance matrix coefficients. */
		if( isc1 == -1)
		{
		  cmn[irow1+irow1*ndimn] -=
			cmplx( y11r, y11i)* data.si[nseg1-1];
		  cmn[irow1+irow2*ndimn] -=
			cmplx( y12r, y12i)* data.si[nseg1-1];
		}
		else
		{
		  rhnx[irow1] += cmplx( y11r, y11i) *
			vsorc.vsant[isc1]/data.wlam;
		  rhnx[irow2] += cmplx( y12r, y12i) *
			vsorc.vsant[isc1]/data.wlam;
		}

		if( isc2 == -1)
		{
		  cmn[irow2+irow2*ndimn] -=
			cmplx( y22r, y22i)* data.si[nseg2-1];
		  cmn[irow2+irow1*ndimn] -=
			cmplx( y12r, y12i)* data.si[nseg2-1];
		}
		else
		{
		  rhnx[irow1] += cmplx( y12r, y12i) *
			vsorc.vsant[isc2]/data.wlam;
		  rhnx[irow2] += cmplx( y22r, y22i) *
			vsorc.vsant[isc2]/data.wlam;
		}

	  } /* for( j = 0; j < netcx.nonet; j++ ) */

	  /* add interaction matrix admittance */
	  /* elements to network equation matrix */
	  for( i = 0; i < nteq; i++ )
	  {
		for( j = 0; j < neqt; j++ )
		  rhs[j] = CPLX_00;

		irow1= nteqa[i]-1;
		rhs[irow1]=CPLX_10;
		solves( cmx, ip, rhs, netcx.neq, 1,
			data.np, data.n, data.mp, data.m);
		cabc( rhs);

		for( j = 0; j < nteq; j++ )
		{
		  irow1= nteqa[j]-1;
		  cmn[i+j*ndimn] += rhs[irow1];
		}

	  } /* for( i = 0; i < nteq; i++ ) */

	  /* factor network equation matrix */
	  factr( nteq, cmn, ipnt, ndimn);

	} /* if( netcx.nonet != 0) */

  } /* if( netcx.ntsol != 0) */

  if( netcx.nonet > 0)
  {
	/* add to network equation right hand side */
	/* the terms due to element interactions */
	for( i = 0; i < neqt; i++ )
	  rhs[i]= einc[i];

	solves( cmx, ip, rhs, netcx.neq, 1,
		data.np, data.n, data.mp, data.m);
	cabc( rhs);

	for( i = 0; i < nteq; i++ )
	{
	  irow1= nteqa[i]-1;
	  rhnt[i]= rhnx[i]+ rhs[irow1];
	}

	/* solve network equations */
	solve( nteq, cmn, ipnt, rhnt, ndimn);

	/* add fields due to network voltages to electric fields */
	/* applied to structure and solve for induced current */
	for( i = 0; i < nteq; i++ )
	{
	  irow1= nteqa[i]-1;
	  einc[irow1] -= rhnt[i];
	}

	solves( cmx, ip, einc, netcx.neq, 1,
		data.np, data.n, data.mp, data.m);
	cabc( einc);

	for( i = 0; i < nteq; i++ )
	{
	  irow1= nteqa[i]-1;
	  vlt= rhnt[i]* data.si[irow1]* data.wlam;
	  cux= einc[irow1]* data.wlam;
	  netcx.zped= vlt/ cux;
	  pwr=.5* creal( vlt* conj( cux));
	  netcx.pnls= netcx.pnls- pwr;
	}

	if( ntsc != 0)
	{
	  for( i = 0; i < ntsc; i++ )
	  {
		irow1= ntsca[i]-1;
		vlt= vsrc[i];
		cux= einc[irow1]* data.wlam;
		netcx.zped= vlt/ cux;
		pwr=.5* creal( vlt* conj( cux));
		netcx.pnls= netcx.pnls- pwr;
	  } /* for( i = 0; i < ntsc; i++ ) */

	} /* if( ntsc != 0) */

  } /* if( netcx.nonet != 0) */
  else
  {
	/* solve for currents when no networks are present */
	solves( cmx, ip, einc, netcx.neq, 1,
		data.np, data.n, data.mp, data.m);
	cabc( einc);
	ntsc=0;
  }

  if( (vsorc.nsant+vsorc.nvqd) == 0)
  {
	/* Free network buffers */
	free_ptr( (void **)&ipnt );
	free_ptr( (void **)&nteqa );
	free_ptr( (void **)&ntsca );
	free_ptr( (void **)&vsrc );
	free_ptr( (void **)&rhs );
	free_ptr( (void **)&cmn );
	free_ptr( (void **)&rhnt );
	free_ptr( (void **)&rhnx );
	return;
  }

  if( vsorc.nsant != 0)
  {
	for( i = 0; i < vsorc.nsant; i++ )
	{
	  isc1= vsorc.isant[i]-1;
	  vlt= vsorc.vsant[i];

	  if( ntsc == 0)
	  {
		cux= einc[isc1]* data.wlam;
		irow1=0;
	  }
	  else
	  {
		for( j = 0; j < ntsc; j++ )
		  if( ntsca[j] == isc1+1)
			break;

		irow1= ndimn- (j+1);
		cux= rhnx[irow1];
		for( j = 0; j < nteq; j++ )
		  cux -= cmn[j+irow1*ndimn]*rhnt[j];
		cux=(einc[isc1]+ cux)* data.wlam;
		irow1++;

	  } /* if( ntsc == 0) */

	  netcx.zped= vlt/ cux;
	  pwr=.5* creal( vlt* conj( cux));
	  netcx.pin= netcx.pin+ pwr;

	  if( irow1 != 0)
		netcx.pnls= netcx.pnls+ pwr;
	} /* for( i = 0; i < vsorc.nsant; i++ ) */

  } /* if( vsorc.nsant != 0) */

  if( vsorc.nvqd != 0)
	for( i = 0; i < vsorc.nvqd; i++ )
	{
	  isc1= vsorc.ivqd[i]-1;
	  vlt= vsorc.vqd[i];
	  cux= cmplx( crnt.air[isc1], crnt.aii[isc1]);
	  ymit= cmplx( crnt.bir[isc1], crnt.bii[isc1]);
	  netcx.zped= cmplx( crnt.cir[isc1], crnt.cii[isc1]);
	  pwr= data.si[isc1]* M_2PI*.5;
	  cux=( cux- ymit* sin( pwr) +
		  netcx.zped* cos( pwr))* data.wlam;
	  netcx.zped= vlt/ cux;
	  pwr=.5* creal( vlt* conj( cux));
	  netcx.pin= netcx.pin+ pwr;
	} /* for( i = 0; i < vsorc.nvqd; i++ ) */

  /* Free network buffers */
  free_ptr( (void **)&ipnt );
  free_ptr( (void **)&nteqa );
  free_ptr( (void **)&ntsca );
  free_ptr( (void **)&vsrc );
  free_ptr( (void **)&rhs );
  free_ptr( (void **)&cmn );
  free_ptr( (void **)&rhnt );
  free_ptr( (void **)&rhnx );

  return;
}

/*-----------------------------------------------------------------------*/

/* load calculates the impedance of specified */
/* segments for various types of loading */
  void
load( int *ldtyp, int *ldtag, int *ldtagf, int *ldtagt,
	double *zlr, double *zli, double *zlc )
{
  int i, istep, istepx, l1, l2, ldtags, jump, ichk;
  complex double zt=CPLX_00, tpcj;

  tpcj = (0.0+I*1.883698955e+9);

  /* initialize d array, used for temporary */
  /* storage of loading information. */
  for( i = 0; i < data.n; i++ )
	zload.zarray[i]=CPLX_00;

  istep=0;

  /* cycle over loading cards */
  while( TRUE )
  {
	istepx = istep;
	istep++;

	if( istep > zload.nload)
	{
	  smat.nop = data.n/data.np;
	  if( smat.nop == 1) return;

	  for( i = 0; i < data.np; i++ )
	  {
		zt = zload.zarray[i];
		l1 = i;

		for( l2 = 1; l2 < smat.nop; l2++ )
		{
		  l1 += data.np;
		  zload.zarray[l1]= zt;
		}
	  }

	  return;
	} /* if( istep > zload.nload) */

	if( ldtyp[istepx] > 5 )
	{
	  fprintf( stderr,
		  _("xnec2c: load(): improper load type chosen,"
		  " requested type is %d\n"), ldtyp[istepx] );
	  Stop( _("load(): Improper load type chose"), ERR_STOP );
	}

	/* search segments for proper itags */
	ldtags= ldtag[istepx];
	jump= ldtyp[istepx]+1;
	ichk=0;
	l1= 1;
	l2= data.n;

	if( ldtags == 0)
	{
	  if( (ldtagf[istepx] != 0) || (ldtagt[istepx] != 0) )
	  {
		l1= ldtagf[istepx];
		l2= ldtagt[istepx];

	  } /* if( (ldtagf[istepx] != 0) || (ldtagt[istepx] != 0) ) */

	} /* if( ldtags == 0) */

	for( i = l1-1; i < l2; i++ )
	{
	  if( ldtags != 0)
	  {
		if( ldtags != data.itag[i])
		  continue;

		if( ldtagf[istepx] != 0)
		{
		  ichk++;
		  if( (ichk < ldtagf[istepx]) || (ichk > ldtagt[istepx]) )
			continue;
		}
		else ichk=1;

	  } /* if( ldtags != 0) */
	  else ichk=1;

	  /* calculation of lamda*imped. per unit length, */
	  /* jump to appropriate section for loading type */
	  switch( jump )
	  {
		case 1:
		  zt= zlr[istepx]/ data.si[i] +
			tpcj* zli[istepx]/( data.si[i]* data.wlam);
		  if( fabs( zlc[istepx]) > 1.0e-20)
			zt += data.wlam/( tpcj* data.si[i]* zlc[istepx]);
		  break;

		case 2:
		  zt= tpcj* data.si[i]* zlc[istepx]/ data.wlam;
		  if( fabs( zli[istepx]) > 1.0e-20)
			zt += data.si[i]* data.wlam/( tpcj* zli[istepx]);
		  if( fabs( zlr[istepx]) > 1.0e-20)
			zt += data.si[i]/ zlr[istepx];
		  zt=1.0/ zt;
		  break;

		case 3:
		  zt= zlr[istepx]* data.wlam+ tpcj* zli[istepx];
		  if( fabs( zlc[istepx]) > 1.0e-20)
			zt += 1.0/( tpcj* data.si[i]* data.si[i]* zlc[istepx]);
		  break;

		case 4:
		  zt= tpcj* data.si[i]* data.si[i]* zlc[istepx];
		  if( fabs( zli[istepx]) > 1.0e-20)
			zt += 1.0/( tpcj* zli[istepx]);
		  if( fabs( zlr[istepx]) > 1.0e-20)
			zt += 1.0/( zlr[istepx]* data.wlam);
		  zt=1.0/ zt;
		  break;

		case 5:
		  zt= cmplx( zlr[istepx], zli[istepx])/ data.si[i];
		  break;

		case 6:
		  zint( zlr[istepx]* data.wlam, data.bi[i], &zt );

	  } /* switch( jump ) */

	  zload.zarray[i] += zt;
	} /* for( i = l1-1; i < l2; i++ ) */

	if( ichk == 0 )
	{
	  fprintf( stderr,
		  _("xnec2c: load(): loading data card error,"
		  " no segment has an itag = %d\n"), ldtags );
	  Stop( _("load(): Loading data card\n"
			"Tag number mismatch error"), ERR_STOP );
	}

  } /* while( TRUE ) */

}

/*-----------------------------------------------------------------------*/

