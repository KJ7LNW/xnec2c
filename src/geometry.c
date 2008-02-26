/*
 *  xnec2c - GTK2-based version of nec2c, the C translation of NEC2
 *  Copyright (C) 2003-2006 N. Kyriazis <neoklis<at>mailspeed.net>
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

#include "xnec2c.h"

/* common  /data/ */
data_t data;

/* common  /segj/ */
extern segj_t segj;

extern FILE *input_fp;

/* Segments for drawing structure */
GdkSegment *structure_segs;

/*-------------------------------------------------------------------*/

/* arc generates segment geometry data for an arc of ns segments */
  int
arc( int itg, int ns, long double rada,
	long double ang1, long double ang2, long double rad )
{
  int ist, i;
  long double ang, dang, xs1, xs2, zs1, zs2;
  size_t mreq;

  ist= data.n;
  data.n += ns;
  data.np= data.n;
  data.mp= data.m;
  data.ipsym=0;

  if( ns < 1)
	return(0);

  if( fabsl( ang2- ang1) < 360.00001l)
  {
	/* Reallocate tags buffer */
	mem_realloc( (void *)&data.itag,
		(data.n+data.m) * sizeof(int), "in geometry.c" );

	/* Reallocate wire buffers */
	mreq = data.n * sizeof(long double);
	mem_realloc( (void *)&data.x1, mreq, "in geometry.c" );
	mem_realloc( (void *)&data.y1, mreq, "in geometry.c" );
	mem_realloc( (void *)&data.z1, mreq, "in geometry.c" );
	mem_realloc( (void *)&data.x2, mreq, "in geometry.c" );
	mem_realloc( (void *)&data.y2, mreq, "in geometry.c" );
	mem_realloc( (void *)&data.z2, mreq, "in geometry.c" );
	mem_realloc( (void *)&data.bi, mreq, "in geometry.c" );

	ang= ang1* TA;
	dang=( ang2- ang1)* TA/ ns;
	xs1= rada* cosl( ang);
	zs1= rada* sinl( ang);

	for( i = ist; i < data.n; i++ )
	{
	  ang += dang;
	  xs2= rada* cosl( ang);
	  zs2= rada* sinl( ang);
	  data.x1[i]= xs1;

	  data.y1[i]=0.0l;
	  data.z1[i]= zs1;
	  data.x2[i]= xs2;
	  data.y2[i]=0.0l;
	  data.z2[i]= zs2;
	  xs1= xs2;
	  zs1= zs2;
	  data.bi[i]= rad;
	  data.itag[i]= itg;

	} /* for( i = ist; i < data.n; i++ ) */

  } /* if( fabsl( ang2- ang1) < 360.00001l) */
  else
  {
	fprintf( stderr,
		"xnec2c: arc() - arc angle exceeds 360 degrees\n");
	stop( "Arc angle exceeds 360 degrees", 1 );
  }

  return(0);
}

/*-----------------------------------------------------------------------*/

/* connect sets up segment connection data in arrays icon1 and */
/* icon2 by searching for segment ends that are in contact. */
  int
conect( int ignd )
{
  int i, iz, ic, j, jx, ix, ixx, iseg, iend, jend, nsflg, jump;
  long double sep=0.0l, xi1, yi1, zi1, xi2, yi2, zi2;
  long double slen, xa, ya, za, xs, ys, zs;

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
	  fprintf( stderr,
		  "xnec2c: error - np > n in conect()\n" );
	  stop( "conect(): np > n", 1 );
	}

	if( (data.np == data.n) && (data.mp == data.m) )
	  data.ipsym=0;

  } /* if( ignd != 0) */

  if( data.n != 0)
  {
	/* Allocate memory to connections */
	mem_realloc( (void *)&data.icon1,
		(data.n+data.m) * sizeof(int), "in geometry.c" );
	mem_realloc( (void *)&data.icon2,
		(data.n+data.m) * sizeof(int), "in geometry.c" );

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
	  slen= sqrtl( (xi2- xi1)*(xi2- xi1) + (yi2- yi1) *
		  (yi2- yi1) + (zi2- zi1)*(zi2- zi1) ) * SMIN;

	  /* determine connection data for end 1 of segment. */
	  jump = FALSE;
	  if( ignd > 0)
	  {
		if( zi1 <= -slen)
		{
		  fprintf( stderr,
			  "xnec2c: geometry data error:\n"
			  "segment %d extends below ground\n", iz );
		  stop( "Geometry data error:\n"
			  "Segment extends below ground", 1 );
		}

		if( zi1 <= slen)
		{
		  data.icon1[i]= iz;
		  data.z1[i]=0.0l;
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

		  sep= fabsl( xi1- data.x1[ic]) +
			fabsl(yi1- data.y1[ic])+ fabsl(zi1- data.z1[ic]);
		  if( sep <= slen)
		  {
			data.icon1[i]= -(ic+1);
			break;
		  }

		  sep= fabsl( xi1- data.x2[ic]) +
			fabsl(yi1- data.y2[ic])+ fabsl(zi1- data.z2[ic]);
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
			  "xnec2c: geometry data error:\n"
			  "segment %d extends below ground\n", iz );
		  stop( "Geometry data error:\n"
			  "Segment extends below ground", 1 );
		}

		if( zi2 <= slen)
		{
		  if( data.icon1[i] == iz )
		  {
			fprintf( stderr,
				"xnec2c: geometry data error:\n"
				"segment %d lies in ground plane\n", iz );
			stop( "Geometry data error:\n"
				"Segment lies in ground plane", 1 );
		  }

		  data.icon2[i]= iz;
		  data.z2[i]=0.0l;
		  continue;

		} /* if( zi2 <= slen) */

	  } /* if( ignd > 0) */

	  ic= i;
	  for( j = 1; j < data.n; j++ )
	  {
		ic++;
		if( ic >= data.n)
		  ic=0;

		sep= fabsl(xi2- data.x1[ic]) +
		  fabsl(yi2- data.y1[ic])+ fabsl(zi2- data.z1[ic]);
		if( sep <= slen)
		{
		  data.icon2[i]= (ic+1);
		  break;
		}

		sep= fabsl(xi2- data.x2[ic]) +
		  fabsl(yi2- data.y2[ic])+ fabsl(zi2- data.z2[ic]);
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
		  slen=( fabsl(xi2- xi1) +
			  fabsl(yi2- yi1)+ fabsl(zi2- zi1))* SMIN;
		  sep= fabsl(xi1- xs)+ fabsl(yi1- ys)+ fabsl(zi1- zs);

		  /* connection - divide patch into 4
		   * patches at present array loc. */
		  if( sep <= slen)
		  {
			data.icon1[iseg]=PCHCON+ i;
			ic=0;
			subph( i, ic );
			break;
		  }

		  sep= fabsl(xi2- xs)+ fabsl(yi2- ys)+ fabsl(zi2- zs);
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
	  fprintf( stderr,
		  "xnec2c: error: ipsym = 0 in conect()\n" );
	  stop( "Error: ipsym = 0 in conect()", 1 );
	}

	ic= iseg/2;
	if( iseg == 8) ic=3;

  } /* if( iseg != 1) */

  if( data.n == 0)
	return(0);

  /* Allocate to connection buffers */
  mem_realloc( (void *)&segj.jco,
	  segj.maxcon * sizeof(int), "in geometry.c" );

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
		nsflg=0;

		do
		{
		  if( ix == 0 )
		  {
			fprintf( stderr,
				"xnec2c: connect - segment connection"
				" error for segment: %d\n", ix );
			stop( "Segment connection error", 1 );
		  }

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
			mem_realloc( (void *)&segj.jco,
				segj.maxcon * sizeof(int), "in geometry.c" );
		  }
		  segj.jco[ic-1]= ix* jend;

		  if( ix > 0)
			nsflg=1;

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

		if( jump && (iend == 1) )
		  break;
		else
		  if( jump )
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

		sep= (long double)ic;
		xa= xa/ sep;
		ya= ya/ sep;
		za= za/ sep;

		for( i = 0; i < ic; i++ )
		{
		  ix= segj.jco[i];
		  if( ix <= 0)
		  {
			ix=- ix;
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

	  if( iend == 1)
		break;

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

  mem_realloc( (void *)&segj.ax,
	  segj.maxcon * sizeof(long double), "in geometry.c" );
  mem_realloc( (void *)&segj.bx,
	  segj.maxcon * sizeof(long double), "in geometry.c" );
  mem_realloc( (void *)&segj.cx,
	  segj.maxcon * sizeof(long double), "in geometry.c" );

  return(0);
}

/*-----------------------------------------------------------------------*/

/* subroutine helix generates segment geometry */
/* data for a helix of ns segments */
void helix(
	long double s, long double hl,
	long double a1, long double b1,
	long double a2, long double b2,
	long double rad, int ns, int itg )
{
  int ist, i, mreq;
  long double zinc, copy;

  ist = data.n;
  data.n += ns;
  data.np = data.n;
  data.mp = data.m;
  data.ipsym = 0;

  if( ns < 1)
	return;

  zinc= fabsl( hl/ ns);

  /* Reallocate tags buffer */
  mem_realloc( (void *)&data.itag,
	  (data.n) * sizeof(int), "in geometry.c" );

  /* Reallocate wire buffers */
  mreq = data.n * sizeof(long double);
  mem_realloc( (void *)&data.x1, mreq, "in geometry.c" );
  mem_realloc( (void *)&data.y1, mreq, "in geometry.c" );
  mem_realloc( (void *)&data.z1, mreq, "in geometry.c" );
  mem_realloc( (void *)&data.x2, mreq, "in geometry.c" );
  mem_realloc( (void *)&data.y2, mreq, "in geometry.c" );
  mem_realloc( (void *)&data.z2, mreq, "in geometry.c" );
  mem_realloc( (void *)&data.bi, mreq, "in geometry.c" );

  data.z1[ist] = 0.0l;
  for( i = ist; i < data.n; i++ )
  {
	data.bi[i]= rad;
	data.itag[i]= itg;

	if( i != ist )
	  data.z1[i]= data.z1[i-1]+ zinc;

	data.z2[i]= data.z1[i]+ zinc;

	if( a2 == a1)
	{
	  if( b1 == 0.0l)
		b1= a1;

	  data.x1[i]= a1* cosl(2.0l* PI* data.z1[i]/ s);
	  data.y1[i]= b1* sinl(2.0l* PI* data.z1[i]/ s);
	  data.x2[i]= a1* cosl(2.0l* PI* data.z2[i]/ s);
	  data.y2[i]= b1* sinl(2.0l* PI* data.z2[i]/ s);
	}
	else
	{
	  if( b2 == 0.0l)
		b2= a2;

	  data.x1[i]=( a1+( a2- a1)* data.z1[i] /
		  fabsl( hl))* cosl(2.0l* PI* data.z1[i]/ s);
	  data.y1[i]=( b1+( b2- b1)* data.z1[i] /
		  fabsl( hl))* sinl(2.0l* PI* data.z1[i]/ s);
	  data.x2[i]=( a1+( a2- a1)* data.z2[i] /
		  fabsl( hl))* cosl(2.0l* PI* data.z2[i]/ s);
	  data.y2[i]=( b1+( b2- b1)* data.z2[i] /
		  fabsl( hl))* sinl(2.0l* PI* data.z2[i]/ s);

	} /* if( a2 == a1) */

	if( hl > 0.0l)
	  continue;

	copy= data.x1[i];
	data.x1[i]= data.y1[i];
	data.y1[i]= copy;
	copy= data.x2[i];
	data.x2[i]= data.y2[i];
	data.y2[i]= copy;

  } /* for( i = ist; i < data.n; i++ ) */

  return;
}

/*-----------------------------------------------------------------------*/

/* isegno returns the segment number of the mth segment having the */
/* tag number itagi.  if itagi=0 segment number m is returned. */
int isegno( int itagi, int mx)
{
  int icnt, i, iseg;

  if( mx <= 0)
  {
	fprintf( stderr,
		"xnec2c: check data, parameter specifying segment\n"
		"position in a group of equal tags must not be zero\n" );
	stop( "Parameter specifying segment position\n"
		"in a group of equal tags is zero", 1 );
  }

  icnt=0;
  if( itagi == 0)
  {
	iseg = mx;
	return( iseg );
  }

  if( data.n > 0)
  {
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
	  "xnec2c: no segment has an itag of %d\n",  itagi );
  stop( "Segment tag number error", 1 );
  return(1);
}

/*-----------------------------------------------------------------------*/

/* subroutine move moves the structure with respect to its */
/* coordinate system or reproduces structure in new positions. */
/* structure is rotated about x,y,z axes by rox,roy,roz */
/* respectively, then shifted by xs,ys,zs */
void move( long double rox, long double roy,
	long double roz, long double xs, long double ys,
	long double zs, int its, int nrpt, int itgi )
{
  int nrp, ix, i1, k, ir, i, ii, mreq;
  long double sps, cps, sth, cth, sph, cph, xx, xy;
  long double xz, yx, yy, yz, zx, zy, zz, xi, yi, zi;

  if( fabsl( rox)+ fabsl( roy) > 1.0e-10l)
	data.ipsym= data.ipsym*3;

  sps= sinl( rox);
  cps= cosl( rox);
  sth= sinl( roy);
  cth= cosl( roy);
  sph= sinl( roz);
  cph= cosl( roz);
  xx= cph* cth;
  xy= cph* sth* sps- sph* cps;
  xz= cph* sth* cps+ sph* sps;
  yx= sph* cth;
  yy= sph* sth* sps+ cph* cps;
  yz= sph* sth* cps- cph* sps;
  zx=- sth;
  zy= cth* sps;
  zz= cth* cps;

  if( nrpt == 0)
	nrp=1;
  else
	nrp= nrpt;

  ix=1;
  if( data.n > 0)
  {
	i1= isegno( its, 1);
	if( i1 < 1)
	  i1= 1;

	ix= i1;
	if( nrpt == 0)
	  k= i1-1;
	else
	{
	  k= data.n;
	  /* Reallocate tags buffer */
	  mreq = data.n+data.m + (data.n+1-i1)*nrpt;
	  mem_realloc( (void *)&data.itag,
		  mreq * sizeof(int), "in geometry.c" );

	  /* Reallocate wire buffers */
	  mreq = (data.n+(data.n+1-i1)*nrpt) * sizeof(long double);
	  mem_realloc( (void *)&data.x1, mreq, "in geometry.c" );
	  mem_realloc( (void *)&data.y1, mreq, "in geometry.c" );
	  mem_realloc( (void *)&data.z1, mreq, "in geometry.c" );
	  mem_realloc( (void *)&data.x2, mreq, "in geometry.c" );
	  mem_realloc( (void *)&data.y2, mreq, "in geometry.c" );
	  mem_realloc( (void *)&data.z2, mreq, "in geometry.c" );
	  mem_realloc( (void *)&data.bi, mreq, "in geometry.c" );
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
	i1 = 0;
	if( nrpt == 0)
	  k= 0;
	else
	  k = data.m;

	/* Reallocate patch buffers */
	mreq = data.m * (1+nrpt) * sizeof(long double);
	mem_realloc( (void *)&data.px,  mreq, "in geometry.c" );
	mem_realloc( (void *)&data.py,  mreq, "in geometry.c" );
	mem_realloc( (void *)&data.pz,  mreq, "in geometry.c" );
	mem_realloc( (void *)&data.t1x, mreq, "in geometry.c" );
	mem_realloc( (void *)&data.t1y, mreq, "in geometry.c" );
	mem_realloc( (void *)&data.t1z, mreq, "in geometry.c" );
	mem_realloc( (void *)&data.t2x, mreq, "in geometry.c" );
	mem_realloc( (void *)&data.t2y, mreq, "in geometry.c" );
	mem_realloc( (void *)&data.t2z, mreq, "in geometry.c" );
	mem_realloc( (void *)&data.pbi, mreq, "in geometry.c" );
	mem_realloc( (void *)&data.psalp, mreq, "in geometry.c" );

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
	return;

  data.np= data.n;
  data.mp= data.m;
  data.ipsym=0;

  return;
}

/*-----------------------------------------------------------------------*/

/* patch generates and modifies patch geometry data */
int
patch(
	int nx, int ny,
	long double ax1, long double ay1, long double az1,
	long double ax2, long double ay2, long double az2,
	long double ax3, long double ay3, long double az3,
	long double ax4, long double ay4, long double az4 )
{
  int mi, ntp, iy, ix, mreq;
  long double s1x=0.0l, s1y=0.0l, s1z=0.0l;
  long double s2x=0.0l, s2y=0.0l, s2z=0.0l, xst=0.0l;
  long double znv, xnv, ynv, xa, xn2, yn2;
  long double zn2, salpn, xs, ys, zs, xt, yt, zt;

  /* new patches.  for nx=0, ny=1,2,3,4 patch is (respectively) */;
  /* arbitrary, rectagular, triangular, or quadrilateral. */
  /* for nx and ny  > 0 a rectangular surface is produced with */
  /* nx by ny rectangular patches. */

  data.m++;
  mi= data.m-1;

  /* Reallocate patch buffers */
  mreq = data.m * sizeof(long double);
  mem_realloc( (void *)&data.px,  mreq, "in geometry.c" );
  mem_realloc( (void *)&data.py,  mreq, "in geometry.c" );
  mem_realloc( (void *)&data.pz,  mreq, "in geometry.c" );
  mem_realloc( (void *)&data.t1x, mreq, "in geometry.c" );
  mem_realloc( (void *)&data.t1y, mreq, "in geometry.c" );
  mem_realloc( (void *)&data.t1z, mreq, "in geometry.c" );
  mem_realloc( (void *)&data.t2x, mreq, "in geometry.c" );
  mem_realloc( (void *)&data.t2y, mreq, "in geometry.c");
  mem_realloc( (void *)&data.t2z, mreq, "in geometry.c" );
  mem_realloc( (void *)&data.pbi, mreq, "in geometry.c" );
  mem_realloc( (void *)&data.psalp, mreq, "in geometry.c" );

  if( nx > 0)
	ntp=2;
  else
	ntp= ny;

  if( ntp <= 1)
  {
	data.px[mi]= ax1;
	data.py[mi]= ay1;
	data.pz[mi]= az1;
	data.pbi[mi]= az2;
	znv= cosl( ax2);
	xnv= znv* cosl( ay2);
	ynv= znv* sinl( ay2);
	znv= sinl( ax2);
	xa= sqrtl( xnv* xnv+ ynv* ynv);

	if( xa >= 1.0e-6l)
	{
	  data.t1x[mi]=- ynv/ xa;
	  data.t1y[mi]= xnv/ xa;
	  data.t1z[mi]=0.0l;
	}
	else
	{
	  data.t1x[mi]=1.0l;
	  data.t1y[mi]=0.0l;
	  data.t1z[mi]=0.0l;
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
	xa= sqrtl( xnv* xnv+ ynv* ynv+ znv* znv);
	xnv= xnv/ xa;
	ynv= ynv/ xa;
	znv= znv/ xa;
	xst= sqrtl( s1x* s1x+ s1y* s1y+ s1z* s1z);
	data.t1x[mi]= s1x/ xst;
	data.t1y[mi]= s1y/ xst;
	data.t1z[mi]= s1z/ xst;

	if( ntp <= 2)
	{
	  data.px[mi]= ax1+.5l*( s1x+ s2x);
	  data.py[mi]= ay1+.5l*( s1y+ s2y);
	  data.pz[mi]= az1+.5l*( s1z+ s2z);
	  data.pbi[mi]= xa;
	}
	else
	{
	  if( ntp != 4)
	  {
		data.px[mi]=( ax1+ ax2+ ax3)/3.0l;
		data.py[mi]=( ay1+ ay2+ ay3)/3.0l;
		data.pz[mi]=( az1+ az2+ az3)/3.0l;
		data.pbi[mi]=.5l* xa;
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
		xst= sqrtl( xn2* xn2+ yn2* yn2+ zn2* zn2);
		salpn=1.0l/(3.0l*( xa+ xst));
		data.px[mi]=( xa*( ax1+ ax2+ ax3) +
			xst*( ax1+ ax3+ ax4))* salpn;
		data.py[mi]=( xa*( ay1+ ay2+ ay3) +
			xst*( ay1+ ay3+ ay4))* salpn;
		data.pz[mi]=( xa*( az1+ az2+ az3) +
			xst*( az1+ az3+ az4))* salpn;
		data.pbi[mi]=.5l*( xa+ xst);
		s1x=( xnv* xn2+ ynv* yn2+ znv* zn2)/ xst;

		if( s1x <= 0.9998l)
		{
		  fprintf( stderr,
			  "xnec2c: error -- corners of quadrilateral\n"
			  "patch do not lie in a plane\n" );
		  stop( "Corners of quadrilateral patch\n"
			  "do not lie in a plane", 1);
		}

	  } /* if( ntp != 4) */

	} /* if( ntp <= 2) */

  } /* if( ntp <= 1) */

  data.t2x[mi]= ynv* data.t1z[mi]- znv* data.t1y[mi];
  data.t2y[mi]= znv* data.t1x[mi]- xnv* data.t1z[mi];
  data.t2z[mi]= xnv* data.t1y[mi]- ynv* data.t1x[mi];
  data.psalp[mi]=1.0l;

  if( nx != 0)
  {
	data.m += nx*ny-1;

	/* Reallocate patch buffers */
	mreq = data.m * sizeof(long double);
	mem_realloc( (void *)&data.px,  mreq, "in geometry.c" );
	mem_realloc( (void *)&data.py,  mreq, "in geometry.c" );
	mem_realloc( (void *)&data.pz,  mreq, "in geometry.c" );
	mem_realloc( (void *)&data.t1x, mreq, "in geometry.c" );
	mem_realloc( (void *)&data.t1y, mreq, "in geometry.c" );
	mem_realloc( (void *)&data.t1z, mreq, "in geometry.c" );
	mem_realloc( (void *)&data.t2x, mreq, "in geometry.c" );
	mem_realloc( (void *)&data.t2y, mreq, "in geometry.c" );
	mem_realloc( (void *)&data.t2z, mreq, "in geometry.c" );
	mem_realloc( (void *)&data.pbi, mreq, "in geometry.c" );
	mem_realloc( (void *)&data.psalp, mreq, "in geometry.c" );

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
		xst= (long double)ix;
		data.px[mi]= xn2+ xst* s1x;
		data.py[mi]= yn2+ xst* s1y;
		data.pz[mi]= zn2+ xst* s1z;
		data.pbi[mi]= xa;
		data.psalp[mi]=1.0l;
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

  return(0);
}

/*-----------------------------------------------------------------------*/

/*** this function was an 'entry point' (part of) 'patch()' ***/
void subph( int nx, int ny )
{
  int mia, ix, iy, mi, mreq;
  long double xs, ys, zs, xa, xst, s1x, s1y;
  long double s1z, s2x, s2y, s2z, saln, xt, yt;

  /* Reallocate patch buffers */
  if( ny == 0 )
	data.m += 3;
  else
	data.m += 4;

  mreq = data.m * sizeof(long double);
  mem_realloc( (void *)&data.px,  mreq, "in geometry.c" );
  mem_realloc( (void *)&data.py,  mreq, "in geometry.c" );
  mem_realloc( (void *)&data.pz,  mreq, "in geometry.c" );
  mem_realloc( (void *)&data.t1x, mreq, "in geometry.c" );
  mem_realloc( (void *)&data.t1y, mreq, "in geometry.c" );
  mem_realloc( (void *)&data.t1z, mreq, "in geometry.c" );
  mem_realloc( (void *)&data.t2x, mreq, "in geometry.c" );
  mem_realloc( (void *)&data.t2y, mreq, "in geometry.c" );
  mem_realloc( (void *)&data.t2z, mreq, "in geometry.c" );
  mem_realloc( (void *)&data.pbi, mreq, "in geometry.c" );
  mem_realloc( (void *)&data.psalp, mreq, "in geometry.c" );
  if( !CHILD )
	mem_realloc( (void *)&structure_segs,
		(data.n+2*data.m)*sizeof(GdkSegment), "in geometry.c" );

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
  xa= data.pbi[mi]/4.0l;
  xst= sqrtl( xa)/2.0l;
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
	  yt=- yt;

	if( (ix == 1) || (ix == 3) )
	  xt=- xt;

	mia++;
  }

  if( nx <= data.mp)
	data.mp += 3;

  if( ny > 0 )
	data.pz[mi]=10000.0l;

  /* Process new patches created */
  if( ! CHILD )
	New_Patch_Data();

  return;
}

/*-----------------------------------------------------------------------*/

/* reflc reflects partial structure along x,y, or z axes or rotates */
/* structure to complete a symmetric structure. */
  int
reflc( int ix, int iy, int iz, int itx, int nop )
{
  int iti, i, nx, itagi, k, mreq;
  long double e1, e2, fnop, sam, cs, ss, xk, yk;

  data.np= data.n;
  data.mp= data.m;
  data.ipsym=0;
  iti= itx;

  if( ix >= 0)
  {
	if( nop == 0)
	  return(0);

	data.ipsym=1;

	/* reflect along z axis */
	if( iz != 0)
	{
	  data.ipsym=2;

	  if( data.n > 0 )
	  {
		/* Reallocate tags buffer */
		mem_realloc( (void *)&data.itag,
			(2*data.n+data.m) * sizeof(int), "in geometry.c" );

		/* Reallocate wire buffers */
		mreq = 2*data.n * sizeof(long double);
		mem_realloc( (void *)&data.x1, mreq, "in geometry.c" );
		mem_realloc( (void *)&data.y1, mreq, "in geometry.c" );
		mem_realloc( (void *)&data.z1, mreq, "in geometry.c" );
		mem_realloc( (void *)&data.x2, mreq, "in geometry.c" );
		mem_realloc( (void *)&data.y2, mreq, "in geometry.c" );
		mem_realloc( (void *)&data.z2, mreq, "in geometry.c" );
		mem_realloc( (void *)&data.bi, mreq, "in geometry.c" );

		for( i = 0; i < data.n; i++ )
		{
		  nx= i+ data.n;
		  e1= data.z1[i];
		  e2= data.z2[i];

		  if( (fabsl(e1)+fabsl(e2) <= 1.0e-5l) || (e1*e2 < -1.0e-6l) )
		  {
			fprintf( stderr,
				"xnec2c: geometry data error:\n"
				"segment %d lies in plane of symmetry\n", i+1 );
			stop( "Geometry data error:\n"
				"Segment lies in plane of symmetry", 1 );
		  }

		  data.x1[nx]= data.x1[i];
		  data.y1[nx]= data.y1[i];
		  data.z1[nx]=- e1;
		  data.x2[nx]= data.x2[i];
		  data.y2[nx]= data.y2[i];
		  data.z2[nx]=- e2;
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
		mreq = 2*data.m * sizeof(long double);
		mem_realloc( (void *)&data.px,  mreq, "in geometry.c" );
		mem_realloc( (void *)&data.py,  mreq, "in geometry.c" );
		mem_realloc( (void *)&data.pz,  mreq, "in geometry.c" );
		mem_realloc( (void *)&data.t1x, mreq, "in geometry.c" );
		mem_realloc( (void *)&data.t1y, mreq, "in geometry.c" );
		mem_realloc( (void *)&data.t1z, mreq, "in geometry.c" );
		mem_realloc( (void *)&data.t2x, mreq, "in geometry.c" );
		mem_realloc( (void *)&data.t2y, mreq, "in geometry.c" );
		mem_realloc( (void *)&data.t2z, mreq, "in geometry.c" );
		mem_realloc( (void *)&data.pbi, mreq, "in geometry.c" );
		mem_realloc( (void *)&data.psalp, mreq, "in geometry.c" );

		for( i = 0; i < data.m; i++ )
		{
		  nx = i+data.m;
		  if( fabsl(data.pz[i]) <= 1.0e-10l)
		  {
			fprintf( stderr,
				"xnec2c: geometry data error:\n"
				"patch %d lies in plane of symmetry\n", i+1 );
			stop( "Geometry data error:\n"
				"Patch lies in plane of symmetry", 1 );
		  }

		  data.px[nx]= data.px[i];
		  data.py[nx]= data.py[i];
		  data.pz[nx]=- data.pz[i];
		  data.t1x[nx]= data.t1x[i];
		  data.t1y[nx]= data.t1y[i];
		  data.t1z[nx]=- data.t1z[i];
		  data.t2x[nx]= data.t2x[i];
		  data.t2y[nx]= data.t2y[i];
		  data.t2z[nx]=- data.t2z[i];
		  data.psalp[nx]=- data.psalp[i];
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
		mem_realloc( (void *)&data.itag,
			2 * data.n * sizeof(int), "in geometry.c" );
		/* Reallocate wire buffers */
		mreq = 2*data.n * sizeof(long double);
		mem_realloc( (void *)&data.x1, mreq, "in geometry.c" );
		mem_realloc( (void *)&data.y1, mreq, "in geometry.c" );
		mem_realloc( (void *)&data.z1, mreq, "in geometry.c" );
		mem_realloc( (void *)&data.x2, mreq, "in geometry.c" );
		mem_realloc( (void *)&data.y2, mreq, "in geometry.c" );
		mem_realloc( (void *)&data.z2, mreq, "in geometry.c" );
		mem_realloc( (void *)&data.bi, mreq, "in geometry.c" );

		for( i = 0; i < data.n; i++ )
		{
		  nx= i+ data.n;
		  e1= data.y1[i];
		  e2= data.y2[i];

		  if( (fabsl(e1)+fabsl(e2) <= 1.0e-5l) || (e1*e2 < -1.0e-6l) )
		  {
			fprintf( stderr,
				"xnec2c: geometry data error:\n"
				"segment %d lies in plane of symmetry\n", i+1 );
			stop( "Geometry data error:\n"
				"Segment lies in plane of symmetry", 1 );
		  }

		  data.x1[nx]= data.x1[i];
		  data.y1[nx]=- e1;
		  data.z1[nx]= data.z1[i];
		  data.x2[nx]= data.x2[i];
		  data.y2[nx]=- e2;
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
		mreq = 2*data.m * sizeof(long double);
		mem_realloc( (void *)&data.px,  mreq, "in geometry.c" );
		mem_realloc( (void *)&data.py,  mreq, "in geometry.c" );
		mem_realloc( (void *)&data.pz,  mreq, "in geometry.c" );
		mem_realloc( (void *)&data.t1x, mreq, "in geometry.c" );
		mem_realloc( (void *)&data.t1y, mreq, "in geometry.c" );
		mem_realloc( (void *)&data.t1z, mreq, "in geometry.c" );
		mem_realloc( (void *)&data.t2x, mreq, "in geometry.c" );
		mem_realloc( (void *)&data.t2y, mreq, "in geometry.c" );
		mem_realloc( (void *)&data.t2z, mreq, "in geometry.c" );
		mem_realloc( (void *)&data.pbi, mreq, "in geometry.c" );
		mem_realloc( (void *)&data.psalp, mreq, "in geometry.c" );

		for( i = 0; i < data.m; i++ )
		{
		  nx= i+data.m;
		  if( fabsl( data.py[i]) <= 1.0e-10l)
		  {
			fprintf( stderr,
				"xnec2c: geometry data error:\n"
				"patch %d lies in plane of symmetry\n", i+1 );
			stop( "Geometry data error:\n"
				"Patch lies in plane of symmetry", 1 );
		  }

		  data.px[nx]= data.px[i];
		  data.py[nx]=- data.py[i];
		  data.pz[nx]= data.pz[i];
		  data.t1x[nx]= data.t1x[i];
		  data.t1y[nx]=- data.t1y[i];
		  data.t1z[nx]= data.t1z[i];
		  data.t2x[nx]= data.t2x[i];
		  data.t2y[nx]=- data.t2y[i];
		  data.t2z[nx]= data.t2z[i];
		  data.psalp[nx]=- data.psalp[i];
		  data.pbi[nx]= data.pbi[i];

		} /* for( i = m2; i <= data.m; i++ ) */

		data.m= data.m*2;

	  } /* if( data.m >= m2) */

	} /* if( iy != 0) */

	/* reflect along x axis */
	if( ix == 0 )
	  return(0);

	if( data.n > 0 )
	{
	  /* Reallocate tags buffer */
	  mem_realloc( (void *)&data.itag,
		  2 * data.n * sizeof(int), "in geometry.c" );

	  /* Reallocate wire buffers */
	  mreq = 2*data.n * sizeof(long double);
	  mem_realloc( (void *)&data.x1, mreq, "in geometry.c" );
	  mem_realloc( (void *)&data.y1, mreq, "in geometry.c" );
	  mem_realloc( (void *)&data.z1, mreq, "in geometry.c" );
	  mem_realloc( (void *)&data.x2, mreq, "in geometry.c" );
	  mem_realloc( (void *)&data.y2, mreq, "in geometry.c" );
	  mem_realloc( (void *)&data.z2, mreq, "in geometry.c" );
	  mem_realloc( (void *)&data.bi, mreq, "in geometry.c" );

	  for( i = 0; i < data.n; i++ )
	  {
		nx= i+ data.n;
		e1= data.x1[i];
		e2= data.x2[i];

		if( (fabsl(e1)+fabsl(e2) <= 1.0e-5l) || (e1*e2 < -1.0e-6l) )
		{
		  fprintf( stderr,
			  "xnec2c: geometry data error:\n"
			  "segment %d lies in plane of symmetry\n", i+1 );
		  stop( "Geometry data error:\n"
			  "Segment lies in plane of symmetry", 1 );
		}

		data.x1[nx]=- e1;
		data.y1[nx]= data.y1[i];
		data.z1[nx]= data.z1[i];
		data.x2[nx]=- e2;
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

	if( data.m == 0 )
	  return(0);

	/* Reallocate patch buffers */
	mreq = 2*data.m * sizeof(long double);
	mem_realloc( (void *)&data.px,  mreq, "in geometry.c" );
	mem_realloc( (void *)&data.py,  mreq, "in geometry.c" );
	mem_realloc( (void *)&data.pz,  mreq, "in geometry.c" );
	mem_realloc( (void *)&data.t1x, mreq, "in geometry.c" );
	mem_realloc( (void *)&data.t1y, mreq, "in geometry.c" );
	mem_realloc( (void *)&data.t1z, mreq, "in geometry.c" );
	mem_realloc( (void *)&data.t2x, mreq, "in geometry.c" );
	mem_realloc( (void *)&data.t2y, mreq, "in geometry.c" );
	mem_realloc( (void *)&data.t2z, mreq, "in geometry.c" );
	mem_realloc( (void *)&data.pbi, mreq, "in geometry.c" );
	mem_realloc( (void *)&data.psalp, mreq, "in geometry.c" );

	for( i = 0; i < data.m; i++ )
	{
	  nx= i+data.m;
	  if( fabsl( data.px[i]) <= 1.0e-10l)
	  {
		fprintf( stderr,
			"xnec2c: geometry data error:\n"
			"patch %d lies in plane of symmetry\n", i+1 );
		stop( "Geometry data error:\n"
			"Patch lies in plane of symmetry", 1 );
	  }

	  data.px[nx]=- data.px[i];
	  data.py[nx]= data.py[i];
	  data.pz[nx]= data.pz[i];
	  data.t1x[nx]=- data.t1x[i];
	  data.t1y[nx]= data.t1y[i];
	  data.t1z[nx]= data.t1z[i];
	  data.t2x[nx]=- data.t2x[i];
	  data.t2y[nx]= data.t2y[i];
	  data.t2z[nx]= data.t2z[i];
	  data.psalp[nx]=- data.psalp[i];
	  data.pbi[nx]= data.pbi[i];
	}

	data.m= data.m*2;
	return(0);

  } /* if( ix >= 0) */

  /* reproduce structure with rotation to form cylindrical structure */
  fnop= (long double)nop;
  data.ipsym=-1;
  sam=TP/ fnop;
  cs= cosl( sam);
  ss= sinl( sam);

  if( data.n > 0)
  {
	data.n *= nop;
	nx= data.np;

	/* Reallocate tags buffer */
	mem_realloc( (void *)&data.itag,
		data.n * sizeof(int), "in geometry.c" );

	/* Reallocate wire buffers */
	mreq = data.n * sizeof(long double);
	mem_realloc( (void *)&data.x1, mreq, "in geometry.c" );
	mem_realloc( (void *)&data.y1, mreq, "in geometry.c" );
	mem_realloc( (void *)&data.z1, mreq, "in geometry.c" );
	mem_realloc( (void *)&data.x2, mreq, "in geometry.c" );
	mem_realloc( (void *)&data.y2, mreq, "in geometry.c" );
	mem_realloc( (void *)&data.z2, mreq, "in geometry.c" );
	mem_realloc( (void *)&data.bi, mreq, "in geometry.c" );

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

  if( data.m == 0 )
	return(0);

  data.m *= nop;
  nx= data.mp;

  /* Reallocate patch buffers */
  mreq = data.m * sizeof(long double);
  mem_realloc( (void *)&data.px,  mreq, "in geometry.c"  );
  mem_realloc( (void *)&data.py,  mreq, "in geometry.c" );
  mem_realloc( (void *)&data.pz,  mreq, "in geometry.c" );
  mem_realloc( (void *)&data.t1x, mreq, "in geometry.c" );
  mem_realloc( (void *)&data.t1y, mreq, "in geometry.c" );
  mem_realloc( (void *)&data.t1z, mreq, "in geometry.c" );
  mem_realloc( (void *)&data.t2x, mreq, "in geometry.c" );
  mem_realloc( (void *)&data.t2y, mreq, "in geometry.c" );
  mem_realloc( (void *)&data.t2z, mreq, "in geometry.c" );
  mem_realloc( (void *)&data.pbi, mreq, "in geometry.c" );
  mem_realloc( (void *)&data.psalp, mreq, "in geometry.c" );

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

  return(0);
}

/*-----------------------------------------------------------------------*/

/* subroutine wire generates segment geometry */
/* data for a straight wire of ns segments. */
void wire( long double xw1, long double yw1, long double zw1,
	long double xw2, long double yw2, long double zw2, long double rad,
	long double rdel, long double rrad, int ns, int itg )
{
  int ist, i, mreq;
  long double xd, yd, zd, delz, rd, fns, radz;
  long double xs1, ys1, zs1, xs2, ys2, zs2;

  if( ns < 1)
	return;

  ist= data.n;
  data.n= data.n+ ns;
  data.np= data.n;
  data.mp= data.m;
  data.ipsym=0;

  /* Reallocate tags buffer */
  mem_realloc( (void *)&data.itag,
	  data.n * sizeof(int), "in geometry.c" );

  /* Reallocate wire buffers */
  mreq = data.n * sizeof(long double);
  mem_realloc( (void *)&data.x1, mreq, "in geometry.c" );
  mem_realloc( (void *)&data.y1, mreq, "in geometry.c" );
  mem_realloc( (void *)&data.z1, mreq, "in geometry.c" );
  mem_realloc( (void *)&data.x2, mreq, "in geometry.c" );
  mem_realloc( (void *)&data.y2, mreq, "in geometry.c" );
  mem_realloc( (void *)&data.z2, mreq, "in geometry.c" );
  mem_realloc( (void *)&data.bi, mreq, "in geometry.c" );

  xd= xw2- xw1;
  yd= yw2- yw1;
  zd= zw2- zw1;

  if( fabsl( rdel-1.0l) >= 1.0e-6l)
  {
	delz= sqrtl( xd* xd+ yd* yd+ zd* zd);
	xd= xd/ delz;
	yd= yd/ delz;
	zd= zd/ delz;
	delz= delz*(1.0l- rdel)/(1.0l- powl(rdel, ns) );
	rd= rdel;
  }
  else
  {
	fns= ns;
	xd= xd/ fns;
	yd= yd/ fns;
	zd= zd/ fns;
	delz=1.0l;
	rd=1.0l;
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

