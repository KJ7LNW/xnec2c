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

#include "geometry.h"
#include "shared.h"

/*-------------------------------------------------------------------*/

/* arc generates segment geometry data for an arc of ns segments */
  gboolean
arc( int itg, int ns, double rada,
	double ang1, double ang2, double rad )
{
  int ist;

  ist= data.n;
  data.n += ns;
  data.np= data.n;
  data.mp= data.m;
  data.ipsym=0;

  if( ns < 1)
  {
	fprintf( stderr,
		_("xnec2c: arc(): number of segments less than 1 (ns < 1)\n") );
	Stop( _("arc(): Number of segments < 1"), ERR_OK );
	return( FALSE );
  }

  if( fabs( ang2- ang1) < 360.00001)
  {
	int i;
	double ang, dang, xs1, xs2, zs1, zs2;
	size_t mreq;

	/* Reallocate tags buffer */
	mreq = (size_t)(data.n + data.m) * sizeof(int);
	mem_realloc( (void **)&data.itag, mreq, "in geometry.c" );

	/* Reallocate wire buffers */
	mreq = (size_t)data.n * sizeof(double);
	mem_realloc( (void **)&data.x1, mreq, "in geometry.c" );
	mem_realloc( (void **)&data.y1, mreq, "in geometry.c" );
	mem_realloc( (void **)&data.z1, mreq, "in geometry.c" );
	mem_realloc( (void **)&data.x2, mreq, "in geometry.c" );
	mem_realloc( (void **)&data.y2, mreq, "in geometry.c" );
	mem_realloc( (void **)&data.z2, mreq, "in geometry.c" );
	mem_realloc( (void **)&data.bi, mreq, "in geometry.c" );

	ang= ang1* TORAD;
	dang=( ang2- ang1)* TORAD/ ns;
	xs1= rada* cos( ang);
	zs1= rada* sin( ang);

	for( i = ist; i < data.n; i++ )
	{
	  ang += dang;
	  xs2= rada* cos( ang);
	  zs2= rada* sin( ang);
	  data.x1[i]= xs1;

	  data.y1[i]=0.0;
	  data.z1[i]= zs1;
	  data.x2[i]= xs2;
	  data.y2[i]=0.0;
	  data.z2[i]= zs2;
	  xs1= xs2;
	  zs1= zs2;
	  data.bi[i]= rad;
	  data.itag[i]= itg;

	} /* for( i = ist; i < data.n; i++ ) */

  } /* if( fabs( ang2- ang1) < 360.00001) */
  else
  {
	fprintf( stderr,
		_("xnec2c: arc(): arc angle exceeds 360 degrees\n") );
	Stop( _("arc(): Arc angle exceeds 360 degrees"), ERR_OK );
	return( FALSE );
  }

  return( TRUE );
}

/*-----------------------------------------------------------------------*/

/*** this function was an 'entry point' (part of) 'patch()' ***/
  static void
subph( int nx, int ny )
{
  int mia, ix, iy, mi;
  size_t mreq;
  double xs, ys, zs, xa, xst, s1x, s1y;
  double s1z, s2x, s2y, s2z, saln, xt, yt;

  /* Reallocate patch buffers */
  if( ny == 0 ) data.m += 3;
  else data.m += 4;

  mreq = (size_t)data.m * sizeof(double);
  mem_realloc( (void **)&data.px,  mreq, "in geometry.c" );
  mem_realloc( (void **)&data.py,  mreq, "in geometry.c" );
  mem_realloc( (void **)&data.pz,  mreq, "in geometry.c" );
  mem_realloc( (void **)&data.t1x, mreq, "in geometry.c" );
  mem_realloc( (void **)&data.t1y, mreq, "in geometry.c" );
  mem_realloc( (void **)&data.t1z, mreq, "in geometry.c" );
  mem_realloc( (void **)&data.t2x, mreq, "in geometry.c" );
  mem_realloc( (void **)&data.t2y, mreq, "in geometry.c" );
  mem_realloc( (void **)&data.t2z, mreq, "in geometry.c" );
  mem_realloc( (void **)&data.pbi, mreq, "in geometry.c" );
  mem_realloc( (void **)&data.psalp, mreq, "in geometry.c" );
  if( !CHILD )
  {
	mreq = (size_t)(data.n + 2 * data.m) * sizeof(Segment_t);
	mem_realloc( (void **)&structure_segs, mreq, "in geometry.c" );
  }

  /* Shift patches to make room for new ones */
  if( (ny == 0) && (nx != data.m) )
  {
	for( iy = data.m-1; iy > nx+2; iy-- )
	{
	  ix = iy-3;
	  data.px[iy]= data.px[ix];
	  data.py[iy]= data.py[ix];
	  data.pz[iy]= data.pz[ix];
	  data.pbi[iy]= data.pbi[ix];
	  data.psalp[iy]= data.psalp[ix];
	  data.t1x[iy]= data.t1x[ix];
	  data.t1y[iy]= data.t1y[ix];
	  data.t1z[iy]= data.t1z[ix];
	  data.t2x[iy]= data.t2x[ix];
	  data.t2y[iy]= data.t2y[ix];
	  data.t2z[iy]= data.t2z[ix];
	}

  } /* if( (ny == 0) || (nx != m) ) */

  /* divide patch for connection */
  mi= nx-1;
  xs= data.px[mi];
  ys= data.py[mi];
  zs= data.pz[mi];
  xa= data.pbi[mi]/4.0;
  xst= sqrt( xa)/2.0;
  s1x= data.t1x[mi];
  s1y= data.t1y[mi];
  s1z= data.t1z[mi];
  s2x= data.t2x[mi];
  s2y= data.t2y[mi];
  s2z= data.t2z[mi];
  saln= data.psalp[mi];
  xt= xst;
  yt= xst;

  if( ny == 0)
	mia= mi;
  else
  {
	data.mp++;
	mia= data.m-1;
  }

  for( ix = 1; ix <= 4; ix++ )
  {
	data.px[mia]= xs+ xt* s1x+ yt* s2x;
	data.py[mia]= ys+ xt* s1y+ yt* s2y;
	data.pz[mia]= zs+ xt* s1z+ yt* s2z;
	data.pbi[mia]= xa;
	data.t1x[mia]= s1x;
	data.t1y[mia]= s1y;
	data.t1z[mia]= s1z;
	data.t2x[mia]= s2x;
	data.t2y[mia]= s2y;
	data.t2z[mia]= s2z;
	data.psalp[mia]= saln;

	if( ix == 2)
	  yt= -yt;

	if( (ix == 1) || (ix == 3) )
	  xt= -xt;

	mia++;
  }

  if( nx <= data.mp)
	data.mp += 3;

  if( ny > 0 )
	data.pz[mi]=10000.0;

  /* Process new patches created */
  if( ! CHILD )
	New_Patch_Data();

  return;
}

/*-----------------------------------------------------------------------*/

/* connect sets up segment connection data in arrays icon1 and */
/* icon2 by searching for segment ends that are in contact. */
  gboolean
conect( int ignd )
{
  int i, iz, ic, j, jx, ix, ixx, iseg, iend, jend, jump;
  double sep=0.0, xi1, yi1, zi1, xi2, yi2, zi2;
  double slen, xa, ya, za, xs, ys, zs;
  size_t mreq;

  segj.maxcon = 1;

  if( ignd != 0)
  {
	if( data.ipsym == 2)
	{
	  data.np=2* data.np;
	  data.mp=2* data.mp;
	}

	if( abs(data.ipsym) > 2 )
	{
	  data.np= data.n;
	  data.mp= data.m;
	}

	/*** possibly should be error condition?? **/
	if( data.np > data.n)
	{
	  fprintf( stderr, "xnec2c: conect(): np > n\n" );
	  Stop( _("Error in conect(): np > n"), ERR_OK );
	  return( FALSE );
	}

	if( (data.np == data.n) && (data.mp == data.m) )
	  data.ipsym=0;

  } /* if( ignd != 0) */

  if( data.n != 0)
  {
	/* Allocate memory to connections */
	mreq = (size_t)(data.n + data.m) * sizeof(int);
	mem_realloc( (void **)&data.icon1, mreq, "in geometry.c" );
	mem_realloc( (void **)&data.icon2, mreq, "in geometry.c" );

	for( i = 0; i < data.n; i++ )
	{
	  data.icon1[i] = data.icon2[i] = 0;
	  iz = i+1;
	  xi1= data.x1[i];
	  yi1= data.y1[i];
	  zi1= data.z1[i];
	  xi2= data.x2[i];
	  yi2= data.y2[i];
	  zi2= data.z2[i];
	  slen= sqrt( (xi2- xi1)*(xi2- xi1) + (yi2- yi1) *
		  (yi2- yi1) + (zi2- zi1)*(zi2- zi1) ) * SMIN;

	  /* determine connection data for end 1 of segment. */
	  jump = FALSE;
	  if( ignd > 0)
	  {
		if( zi1 <= -slen)
		{
		  fprintf( stderr,
			  _("xnec2c: conect(): geometry data error\n"
			  "Segment %d extends below ground\n"), iz );
		  Stop( _("conect(): Geometry data error\n"
				"Segment extends below ground"), ERR_OK );
		  return( FALSE );
		}

		if( zi1 <= slen)
		{
		  data.icon1[i]= iz;
		  data.z1[i]=0.0;
		  jump = TRUE;

		} /* if( zi1 <= slen) */

	  } /* if( ignd > 0) */

	  if( ! jump )
	  {
		ic= i;
		for( j = 1; j < data.n; j++)
		{
		  ic++;
		  if( ic >= data.n)
			ic=0;

		  sep= fabs( xi1- data.x1[ic]) +
			fabs(yi1- data.y1[ic])+ fabs(zi1- data.z1[ic]);
		  if( sep <= slen)
		  {
			data.icon1[i]= -(ic+1);
			break;
		  }

		  sep= fabs( xi1- data.x2[ic]) +
			fabs(yi1- data.y2[ic])+ fabs(zi1- data.z2[ic]);
		  if( sep <= slen)
		  {
			data.icon1[i]= (ic+1);
			break;
		  }

		} /* for( j = 1; j < data.n; j++) */

	  } /* if( ! jump ) */

	  /* determine connection data for end 2 of segment. */
	  if( (ignd > 0) || jump )
	  {
		if( zi2 <= -slen)
		{
		  fprintf( stderr,
			  _("xnec2c: conect(): geometry data error\n"
			  "segment %d extends below ground\n"), iz );
		  Stop( _("conect(): Geometry data error\n"
				"Segment extends below ground"), ERR_OK );
		  return( FALSE );
		}

		if( zi2 <= slen)
		{
		  if( data.icon1[i] == iz )
		  {
			fprintf( stderr,
				_("xnec2c: conect(): geometry data error\n"
				"segment %d lies in ground plane\n"), iz );
			Stop( _("conect(): Geometry data error\n"
				  "Segment lies in ground plane"), ERR_OK );
			return( FALSE );
		  }

		  data.icon2[i]= iz;
		  data.z2[i]=0.0;
		  continue;

		} /* if( zi2 <= slen) */

	  } /* if( ignd > 0) */

	  ic= i;
	  for( j = 1; j < data.n; j++ )
	  {
		ic++;
		if( ic >= data.n)
		  ic=0;

		sep= fabs(xi2- data.x1[ic]) +
		  fabs(yi2- data.y1[ic])+ fabs(zi2- data.z1[ic]);
		if( sep <= slen)
		{
		  data.icon2[i]= (ic+1);
		  break;
		}

		sep= fabs(xi2- data.x2[ic]) +
		  fabs(yi2- data.y2[ic])+ fabs(zi2- data.z2[ic]);
		if( sep <= slen)
		{
		  data.icon2[i]= -(ic+1);
		  break;
		}

	  } /* for( j = 1; j < data.n; j++ ) */

	} /* for( i = 0; i < data.n; i++ ) */

	/* find wire-surface connections for new patches */
	if( data.m != 0)
	{
	  ix = -1;
	  i = 0;
	  while( ++i <= data.m )
	  {
		ix++;
		xs= data.px[ix];
		ys= data.py[ix];
		zs= data.pz[ix];

		for( iseg = 0; iseg < data.n; iseg++ )
		{
		  xi1= data.x1[iseg];
		  yi1= data.y1[iseg];
		  zi1= data.z1[iseg];
		  xi2= data.x2[iseg];
		  yi2= data.y2[iseg];
		  zi2= data.z2[iseg];

		  /* for first end of segment */
		  slen=( fabs(xi2- xi1) +
			  fabs(yi2- yi1)+ fabs(zi2- zi1))* SMIN;
		  sep= fabs(xi1- xs)+ fabs(yi1- ys)+ fabs(zi1- zs);

		  /* connection - divide patch into 4
		   * patches at present array loc. */
		  if( sep <= slen)
		  {
			data.icon1[iseg]=PCHCON+ i;
			ic=0;
			subph( i, ic );
			break;
		  }

		  sep= fabs(xi2- xs)+ fabs(yi2- ys)+ fabs(zi2- zs);
		  if( sep <= slen)
		  {
			data.icon2[iseg]=PCHCON+ i;
			ic=0;
			subph( i, ic );
			break;
		  }

		} /* for( iseg = 0; iseg < data.n; iseg++ ) */

	  } /* while( ++i <= data.m ) */

	} /* if( data.m != 0) */

  } /* if( data.n != 0) */

  iseg=( data.n+ data.m)/( data.np+ data.mp);
  if( iseg != 1)
  {
	/*** may be error condition?? ***/
	if( data.ipsym == 0 )
	{
	  fprintf( stderr, "xnec2c: conect(): ipsym = 0\n" );
	  Stop( _("conect(): Error: ipsym = 0"), ERR_OK );
	  return( FALSE );
	}
  } /* if( iseg != 1) */

  /* No wire segments */
  if( data.n <= 0) return( TRUE );

  /* Allocate to connection buffers */
  mreq = (size_t)segj.maxcon * sizeof(int);
  mem_realloc( (void **)&segj.jco, mreq, "in geometry.c" );

  /* Adjust connected segment ends to exactly coincide.
   * Also find old seg. connecting to new segment */
  iseg = 0;
  for( j = 0; j < data.n; j++ )
  {
	jx = j+1;
	iend=-1;
	jend=-1;
	ix= data.icon1[j];
	ic=1;
	segj.jco[0]= -jx;
	xa= data.x1[j];
	ya= data.y1[j];
	za= data.z1[j];

	while( TRUE )
	{
	  if( (ix != 0) && (ix != (j+1)) && (ix <= PCHCON) )
	  {
		do
		{
		  if( ix < 0 )
			ix= -ix;
		  else
			jend= -jend;

		  jump = FALSE;

		  if( ix == jx )
			break;

		  if( ix < jx )
		  {
			jump = TRUE;
			break;
		  }

		  /* Record max. no. of connections */
		  ic++;
		  if( ic >= segj.maxcon )
		  {
			segj.maxcon = ic+1;
			mreq = (size_t)segj.maxcon * sizeof(int);
			mem_realloc( (void **)&segj.jco, mreq, "in geometry.c" );
		  }
		  segj.jco[ic-1]= ix* jend;

		  ixx = ix-1;
		  if( jend != 1)
		  {
			xa= xa+ data.x1[ixx];
			ya= ya+ data.y1[ixx];
			za= za+ data.z1[ixx];
			ix= data.icon1[ixx];
			continue;
		  }

		  xa= xa+ data.x2[ixx];
		  ya= ya+ data.y2[ixx];
		  za= za+ data.z2[ixx];
		  ix= data.icon2[ixx];

		} /* do */
		while( ix != 0 );

		if( jump && (iend == 1) ) break;
		else if( jump )
		{
		  iend=1;
		  jend=1;
		  ix= data.icon2[j];
		  ic=1;
		  segj.jco[0]= jx;
		  xa= data.x2[j];
		  ya= data.y2[j];
		  za= data.z2[j];
		  continue;
		}

		sep= (double)ic;
		xa= xa/ sep;
		ya= ya/ sep;
		za= za/ sep;

		for( i = 0; i < ic; i++ )
		{
		  ix= segj.jco[i];
		  if( ix <= 0)
		  {
			ix= -ix;
			ixx = ix-1;
			data.x1[ixx]= xa;
			data.y1[ixx]= ya;
			data.z1[ixx]= za;
			continue;
		  }

		  ixx = ix-1;
		  data.x2[ixx]= xa;
		  data.y2[ixx]= ya;
		  data.z2[ixx]= za;

		} /* for( i = 0; i < ic; i++ ) */

		if( ic >= 3)
		  iseg++;

	  } /*if( (ix != 0) && (ix != j) && (ix <= PCHCON) ) */

	  if( iend == 1) break;

	  iend=1;
	  jend=1;
	  ix= data.icon2[j];
	  ic=1;
	  segj.jco[0]= jx;
	  xa= data.x2[j];
	  ya= data.y2[j];
	  za= data.z2[j];

	} /* while( TRUE ) */

  } /* for( j = 0; j < data.n; j++ ) */

  mreq = (size_t)segj.maxcon * sizeof(double);
  mem_realloc( (void **)&segj.ax, mreq, "in geometry.c" );
  mem_realloc( (void **)&segj.bx, mreq, "in geometry.c" );
  mem_realloc( (void **)&segj.cx, mreq, "in geometry.c" );

  return( TRUE );
}

/*-----------------------------------------------------------------------*/

/* subroutine helix generates segment geometry */
/* data for a helix of ns segments */
  void
helix(
	double tsp, double hl,
	double a1, double b1,
	double a2, double b2,
	double rad, int ns, int itg )
{
  int ist, i;
  size_t mreq;
  double z, zinc;

  if( ns < 1) return;

  ist = data.n;
  data.n += ns;
  data.np = data.n;
  data.mp = data.m;
  data.ipsym = 0;
  if( b1 == 0.0) b1 = a1;
  if( b2 == 0.0) b2 = a2;
  if( hl < 0.0 )
  {
	hl  = -hl;
	tsp = -tsp;
  }

  /* Reallocate tags buffer */
  mreq = (size_t)data.n * sizeof(int);
  mem_realloc( (void **)&data.itag, mreq, "in geometry.c" );

  /* Reallocate wire buffers */
  mreq = (size_t)data.n * sizeof(double);
  mem_realloc( (void **)&data.x1, mreq, "in geometry.c" );
  mem_realloc( (void **)&data.y1, mreq, "in geometry.c" );
  mem_realloc( (void **)&data.z1, mreq, "in geometry.c" );
  mem_realloc( (void **)&data.x2, mreq, "in geometry.c" );
  mem_realloc( (void **)&data.y2, mreq, "in geometry.c" );
  mem_realloc( (void **)&data.z2, mreq, "in geometry.c" );
  mem_realloc( (void **)&data.bi, mreq, "in geometry.c" );

  /* For a proper helix */
  if( (hl != 0.0) && (tsp != 0.0) )
  {
	z = 0.0;
	zinc = hl / (double)ns;
	for( i = ist; i < data.n; i++ )
	{
	  data.bi[i] = rad;
	  data.itag[i] = itg;
	  data.z1[i] = z;
	  z += zinc;
	  data.z2[i] = z;

	  if( a2 == a1)
	  {
		data.x1[i]= a1* cos(M_2PI* data.z1[i]/ tsp);
		data.y1[i]= b1* sin(M_2PI* data.z1[i]/ tsp);
		data.x2[i]= a1* cos(M_2PI* data.z2[i]/ tsp);
		data.y2[i]= b1* sin(M_2PI* data.z2[i]/ tsp);
	  }
	  else
	  {
		data.x1[i]=( a1+( a2- a1)* data.z1[i] / hl)*
		  cos(M_2PI* data.z1[i]/ tsp);
		data.y1[i]=( b1+( b2- b1)* data.z1[i] / hl)*
		  sin(M_2PI* data.z1[i]/ tsp);
		data.x2[i]=( a1+( a2- a1)* data.z2[i] / hl)*
		  cos(M_2PI* data.z2[i]/ tsp);
		data.y2[i]=( b1+( b2- b1)* data.z2[i] / hl)*
		  sin(M_2PI* data.z2[i]/ tsp);
	  } /* if( a2 == a1) */

	} /* for( i = ist; i < data.n; i++ ) */
  } /* if( (hl != 0.0) && (tsp != 0.0) ) */
  else /* A spiral */
  {
	double
	  phi  = 0.0,
	  dphi = M_2PI / tsp,
	  da   = (a1 - a2) / (double)ns,
	  db   = (b1 - b2) / (double)ns;

	for( i = ist; i < data.n; i++ )
	{
	  data.bi[i] = rad;
	  data.itag[i] = itg;
	  data.z1[i] = 0.0;
	  data.z2[i] = 0.0;

	  data.x1[i]= a1 * cos( phi );
	  data.y1[i]= b1 * sin( phi );
	  a1 -= da;
	  b1 -= db;
	  phi += dphi;
	  data.x2[i]= a1 * cos( phi );
	  data.y2[i]= b1 * sin( phi );

	} /* for( i = ist; i < data.n; i++ ) */
  }

  return;
}

/*-----------------------------------------------------------------------*/

/* isegno returns the segment number of the mth segment having the */
/* tag number itagi.  if itagi=0 segment number m is returned. */
  int
isegno( int itagi, int mx)
{
  int icnt, iseg;

  if( mx <= 0)
  {
	fprintf( stderr,
		"xnec2c: isegno(): check data, parameter specifying segment\n"
		"position in a group of equal tags must not be zero\n" );
	Stop( _("isegno(): Parameter specifying segment\n"
		  "position in a group of equal tags is zero"), ERR_OK );
	return( -1 );
  }

  icnt=0;
  if( itagi == 0)
  {
	iseg = mx;
	return( iseg );
  }

  if( data.n > 0)
  {
	int i;

	for( i = 0; i < data.n; i++ )
	{
	  if( data.itag[i] != itagi )
		continue;

	  icnt++;
	  if( icnt == mx)
	  {
		iseg= i+1;
		return( iseg );
	  }

	} /* for( i = 0; i < data.n; i++ ) */

  } /* if( data.n > 0) */

  fprintf( stderr,
	  _("xnec2c: isegno(): no segment has an itag of %d\n"),  itagi );
  Stop( _("isegno(): Segment tag number error"), ERR_OK );

  return( -1 );
}

/*-----------------------------------------------------------------------*/

/* subroutine move moves the structure with respect to its */
/* coordinate system or reproduces structure in new positions. */
/* structure is rotated about x,y,z axes by rox,roy,roz */
/* respectively, then shifted by xs,ys,zs */
  gboolean
move( double rox, double roy,
	double roz, double xs, double ys,
	double zs, int its, int nrpt, int itgi )
{
  int nrp, ix, i1, k, i;
  size_t mreq;
  double sps, cps, sth, cth, sph, cph, xx, xy;
  double xz, yx, yy, yz, zx, zy, zz, xi, yi, zi;

  if( fabs( rox)+ fabs( roy) > 1.0e-10)
	data.ipsym= data.ipsym*3;

  sps= sin( rox);
  cps= cos( rox);
  sth= sin( roy);
  cth= cos( roy);
  sph= sin( roz);
  cph= cos( roz);
  xx= cph* cth;
  xy= cph* sth* sps- sph* cps;
  xz= cph* sth* cps+ sph* sps;
  yx= sph* cth;
  yy= sph* sth* sps+ cph* cps;
  yz= sph* sth* cps- cph* sps;
  zx= -sth;
  zy= cth* sps;
  zz= cth* cps;

  if( nrpt == 0) nrp=1;
  else nrp= nrpt;

  ix=1;
  if( data.n > 0)
  {
	int ir;

	i1= isegno( its, 1);
	if( i1 < 0 ) return( FALSE ); /* my addition, error */
	if( i1 < 1) i1= 1;

	ix= i1;
	if( nrpt == 0) k= i1-1;
	else
	{
	  k= data.n;
	  /* Reallocate tags buffer */
	  mreq = (size_t)(data.n + data.m + (data.n + 1 - i1) * nrpt) * sizeof(int);
	  mem_realloc( (void **)&data.itag, mreq, "in geometry.c" );

	  /* Reallocate wire buffers */
	  mreq = (size_t)(data.n + (data.n + 1 - i1) * nrpt) * sizeof(double);
	  mem_realloc( (void **)&data.x1, mreq, "in geometry.c" );
	  mem_realloc( (void **)&data.y1, mreq, "in geometry.c" );
	  mem_realloc( (void **)&data.z1, mreq, "in geometry.c" );
	  mem_realloc( (void **)&data.x2, mreq, "in geometry.c" );
	  mem_realloc( (void **)&data.y2, mreq, "in geometry.c" );
	  mem_realloc( (void **)&data.z2, mreq, "in geometry.c" );
	  mem_realloc( (void **)&data.bi, mreq, "in geometry.c" );
	}

	for( ir = 0; ir < nrp; ir++ )
	{
	  for( i = i1-1; i < data.n; i++ )
	  {
		xi= data.x1[i];
		yi= data.y1[i];
		zi= data.z1[i];
		data.x1[k]= xi* xx+ yi* xy+ zi* xz+ xs;
		data.y1[k]= xi* yx+ yi* yy+ zi* yz+ ys;
		data.z1[k]= xi* zx+ yi* zy+ zi* zz+ zs;
		xi= data.x2[i];
		yi= data.y2[i];
		zi= data.z2[i];
		data.x2[k]= xi* xx+ yi* xy+ zi* xz+ xs;
		data.y2[k]= xi* yx+ yi* yy+ zi* yz+ ys;
		data.z2[k]= xi* zx+ yi* zy+ zi* zz+ zs;
		data.bi[k]= data.bi[i];
		data.itag[k]= data.itag[i];
		if( data.itag[i] != 0)
		  data.itag[k]= data.itag[i]+ itgi;

		k++;

	  } /* for( i = i1; i < data.n; i++ ) */

	  i1= data.n+1;
	  data.n= k;

	} /* for( ir = 0; ir < nrp; ir++ ) */

  } /* if( data.n >= n2) */

  if( data.m > 0)
  {
	int ii;
	i1 = 0;
	if( nrpt == 0) k= 0;
	else k = data.m;

	/* Reallocate patch buffers */
	mreq = (size_t)(data.m * (nrpt + 1)) * sizeof(double);
	mem_realloc( (void **)&data.px,  mreq, "in geometry.c" );
	mem_realloc( (void **)&data.py,  mreq, "in geometry.c" );
	mem_realloc( (void **)&data.pz,  mreq, "in geometry.c" );
	mem_realloc( (void **)&data.t1x, mreq, "in geometry.c" );
	mem_realloc( (void **)&data.t1y, mreq, "in geometry.c" );
	mem_realloc( (void **)&data.t1z, mreq, "in geometry.c" );
	mem_realloc( (void **)&data.t2x, mreq, "in geometry.c" );
	mem_realloc( (void **)&data.t2y, mreq, "in geometry.c" );
	mem_realloc( (void **)&data.t2z, mreq, "in geometry.c" );
	mem_realloc( (void **)&data.pbi, mreq, "in geometry.c" );
	mem_realloc( (void **)&data.psalp, mreq, "in geometry.c" );

	for( ii = 0; ii < nrp; ii++ )
	{
	  for( i = i1; i < data.m; i++ )
	  {
		xi= data.px[i];
		yi= data.py[i];
		zi= data.pz[i];
		data.px[k]= xi* xx+ yi* xy+ zi* xz+ xs;
		data.py[k]= xi* yx+ yi* yy+ zi* yz+ ys;
		data.pz[k]= xi* zx+ yi* zy+ zi* zz+ zs;
		xi= data.t1x[i];
		yi= data.t1y[i];
		zi= data.t1z[i];
		data.t1x[k]= xi* xx+ yi* xy+ zi* xz;
		data.t1y[k]= xi* yx+ yi* yy+ zi* yz;
		data.t1z[k]= xi* zx+ yi* zy+ zi* zz;
		xi= data.t2x[i];
		yi= data.t2y[i];
		zi= data.t2z[i];
		data.t2x[k]= xi* xx+ yi* xy+ zi* xz;
		data.t2y[k]= xi* yx+ yi* yy+ zi* yz;
		data.t2z[k]= xi* zx+ yi* zy+ zi* zz;
		data.psalp[k]= data.psalp[i];
		data.pbi[k]= data.pbi[i];
		k++;

	  } /* for( i = i1; i < data.m; i++ ) */

	  i1= data.m;
	  data.m = k;

	} /* for( ii = 0; ii < nrp; ii++ ) */

  } /* if( data.m >= m2) */

  if( (nrpt == 0) && (ix == 1) )
	return( TRUE );

  data.np= data.n;
  data.mp= data.m;
  data.ipsym=0;

  return( TRUE );
}

/*-----------------------------------------------------------------------*/

/* patch generates and modifies patch geometry data */
  gboolean
patch(
	int nx, int ny,
	double ax1, double ay1, double az1,
	double ax2, double ay2, double az2,
	double ax3, double ay3, double az3,
	double ax4, double ay4, double az4 )
{
  int mi, ntp;
  size_t mreq;
  double s1x=0.0, s1y=0.0, s1z=0.0;
  double s2x=0.0, s2y=0.0, s2z=0.0, xst=0.0;
  double znv, xnv, ynv, xa, xn2, yn2;
  double zn2;

  /* new patches.  for nx=0, ny=1,2,3,4 patch is (respectively) */;
  /* arbitrary, rectagular, triangular, or quadrilateral. */
  /* for nx and ny  > 0 a rectangular surface is produced with */
  /* nx by ny rectangular patches. */

  data.m++;
  mi= data.m-1;

  /* Reallocate patch buffers */
  mreq = (size_t)data.m * sizeof(double);
  mem_realloc( (void **)&data.px,  mreq, "in geometry.c" );
  mem_realloc( (void **)&data.py,  mreq, "in geometry.c" );
  mem_realloc( (void **)&data.pz,  mreq, "in geometry.c" );
  mem_realloc( (void **)&data.t1x, mreq, "in geometry.c" );
  mem_realloc( (void **)&data.t1y, mreq, "in geometry.c" );
  mem_realloc( (void **)&data.t1z, mreq, "in geometry.c" );
  mem_realloc( (void **)&data.t2x, mreq, "in geometry.c" );
  mem_realloc( (void **)&data.t2y, mreq, "in geometry.c");
  mem_realloc( (void **)&data.t2z, mreq, "in geometry.c" );
  mem_realloc( (void **)&data.pbi, mreq, "in geometry.c" );
  mem_realloc( (void **)&data.psalp, mreq, "in geometry.c" );

  if( nx > 0) ntp=2;
  else ntp= ny;

  if( ntp <= 1)
  {
	data.px[mi]= ax1;
	data.py[mi]= ay1;
	data.pz[mi]= az1;
	data.pbi[mi]= az2;
	znv= cos( ax2);
	xnv= znv* cos( ay2);
	ynv= znv* sin( ay2);
	znv= sin( ax2);
	xa= sqrt( xnv* xnv+ ynv* ynv);

	if( xa >= 1.0e-6)
	{
	  data.t1x[mi]= -ynv/ xa;
	  data.t1y[mi]= xnv/ xa;
	  data.t1z[mi]=0.0;
	}
	else
	{
	  data.t1x[mi]=1.0;
	  data.t1y[mi]=0.0;
	  data.t1z[mi]=0.0;
	}

  } /* if( ntp <= 1) */
  else
  {
	s1x= ax2- ax1;
	s1y= ay2- ay1;
	s1z= az2- az1;
	s2x= ax3- ax2;
	s2y= ay3- ay2;
	s2z= az3- az2;

	if( nx != 0)
	{
	  s1x= s1x/ nx;
	  s1y= s1y/ nx;
	  s1z= s1z/ nx;
	  s2x= s2x/ ny;
	  s2y= s2y/ ny;
	  s2z= s2z/ ny;
	}

	xnv= s1y* s2z- s1z* s2y;
	ynv= s1z* s2x- s1x* s2z;
	znv= s1x* s2y- s1y* s2x;
	xa= sqrt( xnv* xnv+ ynv* ynv+ znv* znv);
	xnv= xnv/ xa;
	ynv= ynv/ xa;
	znv= znv/ xa;
	xst= sqrt( s1x* s1x+ s1y* s1y+ s1z* s1z);
	data.t1x[mi]= s1x/ xst;
	data.t1y[mi]= s1y/ xst;
	data.t1z[mi]= s1z/ xst;

	if( ntp <= 2)
	{
	  data.px[mi]= ax1+.5*( s1x+ s2x);
	  data.py[mi]= ay1+.5*( s1y+ s2y);
	  data.pz[mi]= az1+.5*( s1z+ s2z);
	  data.pbi[mi]= xa;
	}
	else
	{
	  if( ntp != 4)
	  {
		data.px[mi]=( ax1+ ax2+ ax3)/3.0;
		data.py[mi]=( ay1+ ay2+ ay3)/3.0;
		data.pz[mi]=( az1+ az2+ az3)/3.0;
		data.pbi[mi]=.5* xa;
	  }
	  else
	  {
		s1x= ax3- ax1;
		s1y= ay3- ay1;
		s1z= az3- az1;
		s2x= ax4- ax1;
		s2y= ay4- ay1;
		s2z= az4- az1;
		xn2= s1y* s2z- s1z* s2y;
		yn2= s1z* s2x- s1x* s2z;
		zn2= s1x* s2y- s1y* s2x;
		xst= sqrt( xn2* xn2+ yn2* yn2+ zn2* zn2);
		double salpn=1.0/(3.0*( xa+ xst));
		data.px[mi]=( xa*( ax1+ ax2+ ax3) +
			xst*( ax1+ ax3+ ax4))* salpn;
		data.py[mi]=( xa*( ay1+ ay2+ ay3) +
			xst*( ay1+ ay3+ ay4))* salpn;
		data.pz[mi]=( xa*( az1+ az2+ az3) +
			xst*( az1+ az3+ az4))* salpn;
		data.pbi[mi]=.5*( xa+ xst);
		s1x=( xnv* xn2+ ynv* yn2+ znv* zn2)/ xst;

		if( s1x <= 0.9998)
		{
		  fprintf( stderr,
			  _("xnec2c: patch(): corners of quadrilateral\n"
			  "patch do not lie in a plane\n") );
		  Stop( _("patch(): Corners of quadrilateral\n"
				"patch do not lie in a plane"), ERR_OK );
		  return( FALSE );
		}

	  } /* if( ntp != 4) */

	} /* if( ntp <= 2) */

  } /* if( ntp <= 1) */

  data.t2x[mi]= ynv* data.t1z[mi]- znv* data.t1y[mi];
  data.t2y[mi]= znv* data.t1x[mi]- xnv* data.t1z[mi];
  data.t2z[mi]= xnv* data.t1y[mi]- ynv* data.t1x[mi];
  data.psalp[mi]=1.0;

  if( nx != 0)
  {
	int  iy, ix;
	double xs, ys, zs, xt, yt, zt;

	data.m += nx*ny-1;
	/* Reallocate patch buffers */
	mreq = (size_t)data.m * sizeof(double);
	mem_realloc( (void **)&data.px,  mreq, "in geometry.c" );
	mem_realloc( (void **)&data.py,  mreq, "in geometry.c" );
	mem_realloc( (void **)&data.pz,  mreq, "in geometry.c" );
	mem_realloc( (void **)&data.t1x, mreq, "in geometry.c" );
	mem_realloc( (void **)&data.t1y, mreq, "in geometry.c" );
	mem_realloc( (void **)&data.t1z, mreq, "in geometry.c" );
	mem_realloc( (void **)&data.t2x, mreq, "in geometry.c" );
	mem_realloc( (void **)&data.t2y, mreq, "in geometry.c" );
	mem_realloc( (void **)&data.t2z, mreq, "in geometry.c" );
	mem_realloc( (void **)&data.pbi, mreq, "in geometry.c" );
	mem_realloc( (void **)&data.psalp, mreq, "in geometry.c" );

	xn2= data.px[mi]- s1x- s2x;
	yn2= data.py[mi]- s1y- s2y;
	zn2= data.pz[mi]- s1z- s2z;
	xs= data.t1x[mi];
	ys= data.t1y[mi];
	zs= data.t1z[mi];
	xt= data.t2x[mi];
	yt= data.t2y[mi];
	zt= data.t2z[mi];

	for( iy = 0; iy < ny; iy++ )
	{
	  xn2 += s2x;
	  yn2 += s2y;
	  zn2 += s2z;

	  for( ix = 1; ix <= nx; ix++ )
	  {
		xst= (double)ix;
		data.px[mi]= xn2+ xst* s1x;
		data.py[mi]= yn2+ xst* s1y;
		data.pz[mi]= zn2+ xst* s1z;
		data.pbi[mi]= xa;
		data.psalp[mi]=1.0;
		data.t1x[mi]= xs;
		data.t1y[mi]= ys;
		data.t1z[mi]= zs;
		data.t2x[mi]= xt;
		data.t2y[mi]= yt;
		data.t2z[mi]= zt;
		mi++;
	  } /* for( ix = 0; ix < nx; ix++ ) */

	} /* for( iy = 0; iy < ny; iy++ ) */

  } /* if( nx != 0) */

  data.ipsym=0;
  data.np= data.n;
  data.mp= data.m;

  return( TRUE );
}

/*-----------------------------------------------------------------------*/

/* reflc reflects partial structure along x,y, or z axes */
/* or rotates structure to complete a symmetric structure. */
  gboolean
reflc( int ix, int iy, int iz, int iti, int nop )
{
  int i, nx, itagi, k;
  size_t mreq;
  double e1, e2, fnop, sam, cs, ss, xk, yk;

  if( nop == 0) return( TRUE );

  data.np= data.n;
  data.mp= data.m;
  data.ipsym=0;

  if( ix >= 0)
  {
	data.ipsym=1;

	/* reflect along z axis */
	if( iz != 0)
	{
	  data.ipsym=2;

	  if( data.n > 0 )
	  {
		/* Reallocate tags buffer */
		mreq = (size_t)(2 * data.n + data.m) * sizeof(int);
		mem_realloc( (void **)&data.itag, mreq, "in geometry.c" );

		/* Reallocate wire buffers */
		mreq = (size_t)(2 * data.n) * sizeof(double);
		mem_realloc( (void **)&data.x1, mreq, "in geometry.c" );
		mem_realloc( (void **)&data.y1, mreq, "in geometry.c" );
		mem_realloc( (void **)&data.z1, mreq, "in geometry.c" );
		mem_realloc( (void **)&data.x2, mreq, "in geometry.c" );
		mem_realloc( (void **)&data.y2, mreq, "in geometry.c" );
		mem_realloc( (void **)&data.z2, mreq, "in geometry.c" );
		mem_realloc( (void **)&data.bi, mreq, "in geometry.c" );

		for( i = 0; i < data.n; i++ )
		{
		  nx= i+ data.n;
		  e1= data.z1[i];
		  e2= data.z2[i];

		  if( (fabs(e1)+fabs(e2) <= 1.0e-5) || (e1*e2 < -1.0e-6) )
		  {
			fprintf( stderr,
				_("xnec2c: reflc(): geometry data error\n"
				"segment %d lies in plane of symmetry\n"), i+1 );
			Stop( _("reflc(): Geometry data error\n"
				  "Segment lies in plane of symmetry"), ERR_OK );
			return( FALSE );
		  }

		  data.x1[nx]= data.x1[i];
		  data.y1[nx]= data.y1[i];
		  data.z1[nx]= -e1;
		  data.x2[nx]= data.x2[i];
		  data.y2[nx]= data.y2[i];
		  data.z2[nx]= -e2;
		  itagi= data.itag[i];

		  if( itagi == 0)
			data.itag[nx]=0;
		  if( itagi != 0)
			data.itag[nx]= itagi+ iti;

		  data.bi[nx]= data.bi[i];

		} /* for( i = 0; i < data.n; i++ ) */

		data.n= data.n*2;
		iti= iti*2;

	  } /* if( data.n > 0) */

	  if( data.m > 0 )
	  {
		/* Reallocate patch buffers */
		mreq = (size_t)(2 * data.m) * sizeof(double);
		mem_realloc( (void **)&data.px,  mreq, "in geometry.c" );
		mem_realloc( (void **)&data.py,  mreq, "in geometry.c" );
		mem_realloc( (void **)&data.pz,  mreq, "in geometry.c" );
		mem_realloc( (void **)&data.t1x, mreq, "in geometry.c" );
		mem_realloc( (void **)&data.t1y, mreq, "in geometry.c" );
		mem_realloc( (void **)&data.t1z, mreq, "in geometry.c" );
		mem_realloc( (void **)&data.t2x, mreq, "in geometry.c" );
		mem_realloc( (void **)&data.t2y, mreq, "in geometry.c" );
		mem_realloc( (void **)&data.t2z, mreq, "in geometry.c" );
		mem_realloc( (void **)&data.pbi, mreq, "in geometry.c" );
		mem_realloc( (void **)&data.psalp, mreq, "in geometry.c" );

		for( i = 0; i < data.m; i++ )
		{
		  nx = i+data.m;
		  if( fabs(data.pz[i]) <= 1.0e-10)
		  {
			fprintf( stderr,
				_("xnec2c: reflc(): geometry data error\n"
				"patch %d lies in plane of symmetry\n"), i+1 );
			Stop( _("reflc(): Geometry data error\n"
				  "Patch lies in plane of symmetry"), ERR_OK );
			return( FALSE );
		  }

		  data.px[nx]= data.px[i];
		  data.py[nx]= data.py[i];
		  data.pz[nx]= -data.pz[i];
		  data.t1x[nx]= data.t1x[i];
		  data.t1y[nx]= data.t1y[i];
		  data.t1z[nx]= -data.t1z[i];
		  data.t2x[nx]= data.t2x[i];
		  data.t2y[nx]= data.t2y[i];
		  data.t2z[nx]= -data.t2z[i];
		  data.psalp[nx]= -data.psalp[i];
		  data.pbi[nx]= data.pbi[i];
		}

		data.m= data.m*2;

	  } /* if( data.m >= m2) */

	} /* if( iz != 0) */

	/* reflect along y axis */
	if( iy != 0)
	{
	  if( data.n > 0)
	  {
		/* Reallocate tags buffer */
		mreq = (size_t)(2 * data.n) * sizeof(int);
		mem_realloc( (void **)&data.itag, mreq, "in geometry.c" );
		/* Reallocate wire buffers */
		mreq = (size_t)(2 * data.n) * sizeof(double);
		mem_realloc( (void **)&data.x1, mreq, "in geometry.c" );
		mem_realloc( (void **)&data.y1, mreq, "in geometry.c" );
		mem_realloc( (void **)&data.z1, mreq, "in geometry.c" );
		mem_realloc( (void **)&data.x2, mreq, "in geometry.c" );
		mem_realloc( (void **)&data.y2, mreq, "in geometry.c" );
		mem_realloc( (void **)&data.z2, mreq, "in geometry.c" );
		mem_realloc( (void **)&data.bi, mreq, "in geometry.c" );

		for( i = 0; i < data.n; i++ )
		{
		  nx= i+ data.n;
		  e1= data.y1[i];
		  e2= data.y2[i];

		  if( (fabs(e1)+fabs(e2) <= 1.0e-5) || (e1*e2 < -1.0e-6) )
		  {
			fprintf( stderr,
				_("xnec2c: reflc(): geometry data error\n"
				"segment %d lies in plane of symmetry\n"), i+1 );
			Stop( _("reflc(): Geometry data error\n"
				  "Segment lies in plane of symmetry"), ERR_OK );
			return( FALSE );
		  }

		  data.x1[nx]= data.x1[i];
		  data.y1[nx]= -e1;
		  data.z1[nx]= data.z1[i];
		  data.x2[nx]= data.x2[i];
		  data.y2[nx]= -e2;
		  data.z2[nx]= data.z2[i];
		  itagi= data.itag[i];

		  if( itagi == 0)
			data.itag[nx]=0;
		  if( itagi != 0)
			data.itag[nx]= itagi+ iti;

		  data.bi[nx]= data.bi[i];

		} /* for( i = n2-1; i < data.n; i++ ) */

		data.n= data.n*2;
		iti= iti*2;

	  } /* if( data.n >= n2) */

	  if( data.m > 0 )
	  {
		/* Reallocate patch buffers */
		mreq = (size_t)(2 * data.m) * sizeof(double);
		mem_realloc( (void **)&data.px,  mreq, "in geometry.c" );
		mem_realloc( (void **)&data.py,  mreq, "in geometry.c" );
		mem_realloc( (void **)&data.pz,  mreq, "in geometry.c" );
		mem_realloc( (void **)&data.t1x, mreq, "in geometry.c" );
		mem_realloc( (void **)&data.t1y, mreq, "in geometry.c" );
		mem_realloc( (void **)&data.t1z, mreq, "in geometry.c" );
		mem_realloc( (void **)&data.t2x, mreq, "in geometry.c" );
		mem_realloc( (void **)&data.t2y, mreq, "in geometry.c" );
		mem_realloc( (void **)&data.t2z, mreq, "in geometry.c" );
		mem_realloc( (void **)&data.pbi, mreq, "in geometry.c" );
		mem_realloc( (void **)&data.psalp, mreq, "in geometry.c" );

		for( i = 0; i < data.m; i++ )
		{
		  nx= i+data.m;
		  if( fabs( data.py[i]) <= 1.0e-10)
		  {
			fprintf( stderr,
				_("xnec2c: reflc(): geometry data error\n"
				"patch %d lies in plane of symmetry\n"), i+1 );
			Stop( _("reflc(): Geometry data error\n"
				  "Patch lies in plane of symmetry"), ERR_OK );
			return( FALSE );
		  }

		  data.px[nx]= data.px[i];
		  data.py[nx]= -data.py[i];
		  data.pz[nx]= data.pz[i];
		  data.t1x[nx]= data.t1x[i];
		  data.t1y[nx]= -data.t1y[i];
		  data.t1z[nx]= data.t1z[i];
		  data.t2x[nx]= data.t2x[i];
		  data.t2y[nx]= -data.t2y[i];
		  data.t2z[nx]= data.t2z[i];
		  data.psalp[nx]= -data.psalp[i];
		  data.pbi[nx]= data.pbi[i];

		} /* for( i = m2; i <= data.m; i++ ) */

		data.m= data.m*2;

	  } /* if( data.m >= m2) */

	} /* if( iy != 0) */

	/* reflect along x axis */
	if( ix == 0 ) return( TRUE );

	if( data.n > 0 )
	{
	  /* Reallocate tags buffer */
	  mreq = (size_t)(2 * data.n) * sizeof(int);
	  mem_realloc( (void **)&data.itag, mreq, "in geometry.c" );

	  /* Reallocate wire buffers */
	  mreq = (size_t)(2 * data.n) * sizeof(double);
	  mem_realloc( (void **)&data.x1, mreq, "in geometry.c" );
	  mem_realloc( (void **)&data.y1, mreq, "in geometry.c" );
	  mem_realloc( (void **)&data.z1, mreq, "in geometry.c" );
	  mem_realloc( (void **)&data.x2, mreq, "in geometry.c" );
	  mem_realloc( (void **)&data.y2, mreq, "in geometry.c" );
	  mem_realloc( (void **)&data.z2, mreq, "in geometry.c" );
	  mem_realloc( (void **)&data.bi, mreq, "in geometry.c" );

	  for( i = 0; i < data.n; i++ )
	  {
		nx= i+ data.n;
		e1= data.x1[i];
		e2= data.x2[i];

		if( (fabs(e1)+fabs(e2) <= 1.0e-5) || (e1*e2 < -1.0e-6) )
		{
		  fprintf( stderr,
			  _("xnec2c: reflc(): geometry data error\n"
			  "segment %d lies in plane of symmetry\n"), i+1 );
		  Stop( _("reflc(): Geometry data error\n"
				"Segment lies in plane of symmetry"), ERR_OK );
		  return( FALSE );
		}

		data.x1[nx]= -e1;
		data.y1[nx]= data.y1[i];
		data.z1[nx]= data.z1[i];
		data.x2[nx]= -e2;
		data.y2[nx]= data.y2[i];
		data.z2[nx]= data.z2[i];
		itagi= data.itag[i];

		if( itagi == 0)
		  data.itag[nx]=0;
		if( itagi != 0)
		  data.itag[nx]= itagi+ iti;

		data.bi[nx]= data.bi[i];
	  }

	  data.n= data.n*2;

	} /* if( data.n > 0) */

	if( data.m == 0 ) return( TRUE );

	/* Reallocate patch buffers */
	mreq = (size_t)(2 * data.m) * sizeof(double);
	mem_realloc( (void **)&data.px,  mreq, "in geometry.c" );
	mem_realloc( (void **)&data.py,  mreq, "in geometry.c" );
	mem_realloc( (void **)&data.pz,  mreq, "in geometry.c" );
	mem_realloc( (void **)&data.t1x, mreq, "in geometry.c" );
	mem_realloc( (void **)&data.t1y, mreq, "in geometry.c" );
	mem_realloc( (void **)&data.t1z, mreq, "in geometry.c" );
	mem_realloc( (void **)&data.t2x, mreq, "in geometry.c" );
	mem_realloc( (void **)&data.t2y, mreq, "in geometry.c" );
	mem_realloc( (void **)&data.t2z, mreq, "in geometry.c" );
	mem_realloc( (void **)&data.pbi, mreq, "in geometry.c" );
	mem_realloc( (void **)&data.psalp, mreq, "in geometry.c" );

	for( i = 0; i < data.m; i++ )
	{
	  nx= i+data.m;
	  if( fabs( data.px[i]) <= 1.0e-10)
	  {
		fprintf( stderr,
			_("xnec2c: reflc(): geometry data error\n"
			"patch %d lies in plane of symmetry\n"), i+1 );
		Stop( _("reflc(): Geometry data error\n"
			  "Patch lies in plane of symmetry"), ERR_OK );
		return( FALSE );
	  }

	  data.px[nx]= -data.px[i];
	  data.py[nx]= data.py[i];
	  data.pz[nx]= data.pz[i];
	  data.t1x[nx]= -data.t1x[i];
	  data.t1y[nx]= data.t1y[i];
	  data.t1z[nx]= data.t1z[i];
	  data.t2x[nx]= -data.t2x[i];
	  data.t2y[nx]= data.t2y[i];
	  data.t2z[nx]= data.t2z[i];
	  data.psalp[nx]= -data.psalp[i];
	  data.pbi[nx]= data.pbi[i];
	}

	data.m= data.m*2;
	return( TRUE );

  } /* if( ix >= 0) */

  /* reproduce structure with rotation to form cylindrical structure */
  fnop= (double)nop;
  data.ipsym=-1;
  sam=M_2PI/ fnop;
  cs= cos( sam);
  ss= sin( sam);

  if( data.n > 0)
  {
	data.n *= nop;
	nx= data.np;

	/* Reallocate tags buffer */
	mreq = (size_t)data.n * sizeof(int);
	mem_realloc( (void **)&data.itag, mreq, "in geometry.c" );

	/* Reallocate wire buffers */
	mreq = (size_t)data.n * sizeof(double);
	mem_realloc( (void **)&data.x1, mreq, "in geometry.c" );
	mem_realloc( (void **)&data.y1, mreq, "in geometry.c" );
	mem_realloc( (void **)&data.z1, mreq, "in geometry.c" );
	mem_realloc( (void **)&data.x2, mreq, "in geometry.c" );
	mem_realloc( (void **)&data.y2, mreq, "in geometry.c" );
	mem_realloc( (void **)&data.z2, mreq, "in geometry.c" );
	mem_realloc( (void **)&data.bi, mreq, "in geometry.c" );

	for( i = nx; i < data.n; i++ )
	{
	  k= i- data.np;
	  xk= data.x1[k];
	  yk= data.y1[k];
	  data.x1[i]= xk* cs- yk* ss;
	  data.y1[i]= xk* ss+ yk* cs;
	  data.z1[i]= data.z1[k];
	  xk= data.x2[k];
	  yk= data.y2[k];
	  data.x2[i]= xk* cs- yk* ss;
	  data.y2[i]= xk* ss+ yk* cs;
	  data.z2[i]= data.z2[k];
	  data.bi[i]= data.bi[k];
	  itagi= data.itag[k];

	  if( itagi == 0)
		data.itag[i]=0;
	  if( itagi != 0)
		data.itag[i]= itagi+ iti;
	}

  } /* if( data.n >= n2) */

  if( data.m == 0 ) return( TRUE );

  data.m *= nop;
  nx= data.mp;

  /* Reallocate patch buffers */
  mreq = (size_t)data.m * sizeof(double);
  mem_realloc( (void **)&data.px,  mreq, "in geometry.c"  );
  mem_realloc( (void **)&data.py,  mreq, "in geometry.c" );
  mem_realloc( (void **)&data.pz,  mreq, "in geometry.c" );
  mem_realloc( (void **)&data.t1x, mreq, "in geometry.c" );
  mem_realloc( (void **)&data.t1y, mreq, "in geometry.c" );
  mem_realloc( (void **)&data.t1z, mreq, "in geometry.c" );
  mem_realloc( (void **)&data.t2x, mreq, "in geometry.c" );
  mem_realloc( (void **)&data.t2y, mreq, "in geometry.c" );
  mem_realloc( (void **)&data.t2z, mreq, "in geometry.c" );
  mem_realloc( (void **)&data.pbi, mreq, "in geometry.c" );
  mem_realloc( (void **)&data.psalp, mreq, "in geometry.c" );

  for( i = nx; i < data.m; i++ )
  {
	k = i-data.mp;
	xk= data.px[k];
	yk= data.py[k];
	data.px[i]= xk* cs- yk* ss;
	data.py[i]= xk* ss+ yk* cs;
	data.pz[i]= data.pz[k];
	xk= data.t1x[k];
	yk= data.t1y[k];
	data.t1x[i]= xk* cs- yk* ss;
	data.t1y[i]= xk* ss+ yk* cs;
	data.t1z[i]= data.t1z[k];
	xk= data.t2x[k];
	yk= data.t2y[k];
	data.t2x[i]= xk* cs- yk* ss;
	data.t2y[i]= xk* ss+ yk* cs;
	data.t2z[i]= data.t2z[k];
	data.psalp[i]= data.psalp[k];
	data.pbi[i]= data.pbi[k];

  } /* for( i = nx; i < data.m; i++ ) */

  return( TRUE );
}

/*-----------------------------------------------------------------------*/

/* subroutine wire generates segment geometry */
/* data for a straight wire of ns segments. */
  void
wire( double xw1, double yw1, double zw1,
	double xw2, double yw2, double zw2, double rad,
	double rdel, double rrad, int ns, int itg )
{
  int ist, i;
  size_t mreq;
  double xd, yd, zd, delz, rd, fns, radz;
  double xs1, ys1, zs1, xs2, ys2, zs2;

  if( ns < 1) return;

  ist= data.n;
  data.n= data.n+ ns;
  data.np= data.n;
  data.mp= data.m;
  data.ipsym=0;

  /* Reallocate tags buffer */
  mreq = (size_t)data.n * sizeof(int);
  mem_realloc( (void **)&data.itag, mreq, "in geometry.c" );

  /* Reallocate wire buffers */
  mreq = (size_t)data.n * sizeof(double);
  mem_realloc( (void **)&data.x1, mreq, "in geometry.c" );
  mem_realloc( (void **)&data.y1, mreq, "in geometry.c" );
  mem_realloc( (void **)&data.z1, mreq, "in geometry.c" );
  mem_realloc( (void **)&data.x2, mreq, "in geometry.c" );
  mem_realloc( (void **)&data.y2, mreq, "in geometry.c" );
  mem_realloc( (void **)&data.z2, mreq, "in geometry.c" );
  mem_realloc( (void **)&data.bi, mreq, "in geometry.c" );

  xd= xw2- xw1;
  yd= yw2- yw1;
  zd= zw2- zw1;

  if( fabs( rdel-1.0) >= 1.0e-6)
  {
	delz= sqrt( xd* xd+ yd* yd+ zd* zd);
	xd= xd/ delz;
	yd= yd/ delz;
	zd= zd/ delz;
	delz= delz*(1.0- rdel)/(1.0- pow(rdel, ns) );
	rd= rdel;
  }
  else
  {
	fns= ns;
	xd= xd/ fns;
	yd= yd/ fns;
	zd= zd/ fns;
	delz=1.0;
	rd=1.0;
  }

  radz= rad;
  xs1= xw1;
  ys1= yw1;
  zs1= zw1;

  for( i = ist; i < data.n; i++ )
  {
	data.itag[i]= itg;
	xs2= xs1+ xd* delz;
	ys2= ys1+ yd* delz;
	zs2= zs1+ zd* delz;
	data.x1[i]= xs1;
	data.y1[i]= ys1;
	data.z1[i]= zs1;
	data.x2[i]= xs2;
	data.y2[i]= ys2;
	data.z2[i]= zs2;
	data.bi[i]= radz;
	delz= delz* rd;
	radz= radz* rrad;
	xs1= xs2;
	ys1= ys2;
	zs1= zs2;
  }

  data.x2[data.n-1]= xw2;
  data.y2[data.n-1]= yw2;
  data.z2[data.n-1]= zw2;

  return;
}

/*-----------------------------------------------------------------------*/

