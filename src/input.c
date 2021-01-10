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
  file created 4/11/80

 *********** Notice **********
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

#include "input.h"
#include "shared.h"

/*------------------------------------------------------------------------*/

/* Read_Comments()
 *
 * Reads CM comment cards from input file
 */
  gboolean
Read_Comments( void )
{
  char ain[3], line_buf[LINE_LEN];


  /* Look for CM or CE card */
  do
  {
	/* read a line from input file */
	if( Load_Line(line_buf, input_fp) == EOF )
	{
	  fprintf( stderr, _("xnec2c: Read_Comments():"
		  "unexpected EOF (End of File)\n") );
	  Stop( _("Read_Comments(): Error reading Comments\n"
			"Unexpected EOF (End of File)"), ERR_OK );
	  return( FALSE );
	}

	/* Capitalize first two characters (mnemonics) */
	if( (line_buf[0] > 0x60) && (line_buf[0] < 0x79) )
	  line_buf[0] = (char)toupper( (int)line_buf[0] );
	if( (line_buf[1] > 0x60) && (line_buf[1] < 0x79) )
	   line_buf[1] = (char)toupper( (int)line_buf[1] );

	/* Check that comment line is not short */
	if( strlen(line_buf) < 2 )
	{
	  fprintf( stderr, _("xnec2c: Read_Comments():"
		  "error reading Comments: "
		  "Comment mnemonic short or missing\n") );
	  Stop( _("Read_Comments(): Error reading Comments\n"
			"Comment mnemonic short or missing"), ERR_OK );
	  return( FALSE );
	}

	/* separate card's id mnemonic */
	Strlcpy( ain, line_buf, sizeof(ain) );

	/* Check for incorrect mnemonic */
	if( (strcmp(ain, "CM") != 0) && (strcmp(ain, "CE") != 0) )
	{
	  Stop( _("Read_Comments():\n"
			" Error reading input file\n"
			"Comment mnemonic incorrect"), ERR_OK );
	  return( FALSE );
	}
  }
  while( (strcmp(ain, "CE") != 0) );

  return( TRUE );
} /* Read_Comments() */

/*-----------------------------------------------------------------------*/

/* Tag_Seg_Error()
 *
 * Checks tag and segments number are valid (>1)
 */
  static gboolean
Tag_Seg_Error( int tag, int segs )
{
  gboolean retv = FALSE;

  if( tag <= 0 )
  {
	fprintf( stderr,
		_("xnec2c: Tag_Seg_Error(): geometry data card error - "
		"tag number is less than 1\n") );
	Stop( _("Tag_Seg_Error(): Geometry data error\n"
		  "Tag number is less than 1"), ERR_OK );
	retv = TRUE;
  }

  if( segs <= 0 )
  {
	fprintf( stderr,
		_("xnec2c: Tag_Seg_Error(): geometry data card error - "
		"number of segments is less than 1\n") );
	Stop( _("Tag_Seg_Error(): Geometry data error\n"
		  "Number of segments is less than 1"), ERR_OK );
	retv = TRUE;
  }

  return( retv );
}

/*-----------------------------------------------------------------------*/

/* datagn is the main routine for input of geometry data. */
  static gboolean
datagn( void )
{
  char gm[3];

  /* input card mnemonic list */
  char *atst[] =
  {
	"GW", "GX", "GR", "GS", "GE","GM", "SP",\
	"SM", "GA", "SC", "GH", "GF", "CT"
  };

  int nwire, isct, itg, iy=0, iz;
  size_t mreq;
  int ix, i, ns, gm_num; /* geometry card id as a number */
  double rad, xs1, xs2, ys1, ys2, zs1, zs2;
  double x3=0, y3=0, z3=0, x4=0, y4=0, z4=0;
  double xw1, xw2, yw1, yw2, zw1, zw2;
  double dummy;

  data.ipsym=0;
  nwire=0;
  data.n=0;
  data.np=0;
  data.m=0;
  data.mp=0;
  isct=0;
  structure_proj_params.r_max = 0.0;

  /* read geometry data card and branch to */
  /* section for operation requested */
  do
  {
	if( !readgm(gm, &itg, &ns, &xw1, &yw1, &zw1, &xw2, &yw2, &zw2, &rad) )
	  return( FALSE );

	/* identify card id mnemonic */
	for( gm_num = 0; gm_num < NUM_GEOMN; gm_num++ )
	  if( strncmp( gm, atst[gm_num], 2) == 0 )
		break;

	if( gm_num != 9 ) isct=0;

	switch( gm_num )
	{
	  case GW: /* "gw" card, generate segment data for straight wire. */
		if( Tag_Seg_Error(itg, ns) ) return( FALSE );
		nwire++;

		if( rad != 0.0)
		{
		  xs1=1.0;
		  ys1=1.0;
		}
		else
		{
		  if( !readgm(gm, &ix, &iy, &xs1, &ys1, &zs1,
			  &dummy, &dummy, &dummy, &dummy) )
			  return( FALSE );

		  if( strcmp(gm, "GC" ) != 0 )
		  {
			fprintf( stderr,
				_("xnec2c: datagn(): geometry data card error "
				"no GC card for tapered wire\n") );
			Stop( _("datagn(): Geometry data error\n"
				  "No GC card for tapered wire"), ERR_OK );
			return( FALSE );
		  }

		  if( (ys1 == 0.0) || (zs1 == 0.0) )
		  {
			fprintf( stderr, _("xnec2c: datagn(): geometry GC data card error\n") );
			Stop( _("datagn(): Geometry GC data card error"), ERR_OK );
			return( FALSE );
		  }

		  rad= ys1;
		  ys1= pow( (zs1/ys1), (1.0/(ns-1.0)) );
		}

		wire( xw1, yw1, zw1, xw2, yw2, zw2, rad, xs1, ys1, ns, itg);
		continue;

		/* reflect structure along x,y, or z */
		/* axes or rotate to form cylinder.  */
	  case GX: /* "gx" card */
		if( (ns < 0) || (itg < 0) )
		{
		  fprintf( stderr, _("xnec2c: datagn(): geometry GX data card error\n") );
		  Stop( _("datagn(): Geometry GX data card error"), ERR_OK );
		  return( FALSE );
		}

		iy= ns/10;
		iz= ns- iy*10;
		ix= iy/10;
		iy= iy- ix*10;

		if( ix != 0)
		  ix=1;
		if( iy != 0)
		  iy=1;
		if( iz != 0)
		  iz=1;

		if( !reflc(ix, iy, iz, itg, ns) )
		  return( FALSE );
		continue;

	  case GR: /* "gr" card */
		if( (ns < 0) || (itg < 0) )
		{
		  fprintf( stderr, _("xnec2c: datagn(): geometry GR data card error\n") );
		  Stop( _("datagn(): Geometry GR data card error"), ERR_OK );
		  return( FALSE );
		}

		ix=-1;
		iz = 0;
		if( !reflc(ix, iy, iz, itg, ns) )
		  return( FALSE );
		continue;

	  case GS: /* "gs" card, scale structure dimensions by factor xw1 */
		if( (itg > 0) && (ns > 0) && (ns >= itg) )
		{
		  for( i = 0; i < data.n; i++ )
		  {
			if( (data.itag[i] >= itg) && (data.itag[i] <= ns) )
			{
			  data.x1[i]= data.x1[i]* xw1;
			  data.y1[i]= data.y1[i]* xw1;
			  data.z1[i]= data.z1[i]* xw1;
			  data.x2[i]= data.x2[i]* xw1;
			  data.y2[i]= data.y2[i]* xw1;
			  data.z2[i]= data.z2[i]* xw1;
			  data.bi[i]= data.bi[i]* xw1;
			}
		  }
		  /* FIXME corrects errors when GS follows GX but this is just a work-around */
		  data.np = data.n;
		  data.ipsym = 0;
		}
		else for( i = 0; i < data.n; i++ )
		{
		  data.x1[i]= data.x1[i]* xw1;
		  data.y1[i]= data.y1[i]* xw1;
		  data.z1[i]= data.z1[i]* xw1;
		  data.x2[i]= data.x2[i]* xw1;
		  data.y2[i]= data.y2[i]* xw1;
		  data.z2[i]= data.z2[i]* xw1;
		  data.bi[i]= data.bi[i]* xw1;
		}

		yw1= xw1* xw1;
		for( i = 0; i < data.m; i++ )
		{
		  data.px[i] = data.px[i]* xw1;
		  data.py[i] = data.py[i]* xw1;
		  data.pz[i] = data.pz[i]* xw1;
		  data.pbi[i]= data.pbi[i]* yw1;
		}
		continue;

	  case GE: /* "ge" card, terminate structure geometry input. */
		/* My addition, for drawing */
		if( ((data.n > 0) || (data.m > 0)) && !CHILD )
		  Init_Struct_Drawing();
		else if( (data.n == 0) && (data.m == 0) )
		{
		  Stop( _("No geometry data cards"), ERR_OK );
		  return( FALSE );
		}

		if( !conect(itg) ) return( FALSE );

		if( data.n != 0)
		{
		  /* Allocate wire buffers */
		  mreq = (size_t)data.n * sizeof(double);
		  mem_realloc( (void **)&data.si,   mreq, "in input.c" );
		  mem_realloc( (void **)&data.sab,  mreq, "in input.c" );
		  mem_realloc( (void **)&data.cab,  mreq, "in input.c" );
		  mem_realloc( (void **)&data.salp, mreq, "in input.c" );
		  mem_realloc( (void **)&data.x, mreq, "in input.c" );
		  mem_realloc( (void **)&data.y, mreq, "in input.c" );
		  mem_realloc( (void **)&data.z, mreq, "in input.c" );

		  for( i = 0; i < data.n; i++ )
		  {
			xw1= data.x2[i]- data.x1[i];
			yw1= data.y2[i]- data.y1[i];
			zw1= data.z2[i]- data.z1[i];
			data.x[i]=( data.x1[i]+ data.x2[i])/2.0;
			data.y[i]=( data.y1[i]+ data.y2[i])/2.0;
			data.z[i]=( data.z1[i]+ data.z2[i])/2.0;
			xw2= xw1* xw1+ yw1* yw1+ zw1* zw1;
			yw2= sqrt( xw2);
			//yw2=( xw2/yw2 + yw2)/2.0;
			data.si[i]= yw2;
			data.cab[i]= xw1/ yw2;
			data.sab[i]= yw1/ yw2;

			xw2= zw1/ yw2;
			if( xw2 > 1.0)
			  xw2=1.0;
			if( xw2 < -1.0)
			  xw2=-1.0;
			data.salp[i]= xw2;

			//xw2= asin( xw2)* TD;
			//yw2= atan2( yw1, xw1)* TD;

			if( (data.si[i] <= 1.0e-20) || (data.bi[i] <= 0.0) )
			{
			  fprintf( stderr, _("xnec2c: datagn(): segment data error\n") );
			  Stop( _("datagn(): Segment data error"), ERR_OK );
			  return( FALSE );
			}

		  } /* for( i = 0; i < data.n; i++ ) */

		} /* if( data.n != 0) */

		if( data.m != 0)
		{
		  for( i = 0; i < data.m; i++ )
		  {
			xw1=( data.t1y[i]* data.t2z[i] -
				data.t1z[i]* data.t2y[i])* data.psalp[i];
			yw1=( data.t1z[i]* data.t2x[i] -
				data.t1x[i]* data.t2z[i])* data.psalp[i];
			zw1=( data.t1x[i]* data.t2y[i] -
				data.t1y[i]* data.t2x[i])* data.psalp[i];
		  } /* for( i = 0; i < data.m; i++ ) */

		} /* if( data.m != 0) */

		data.npm  = data.n+data.m;
		data.np2m = data.n+2*data.m;
		data.np3m = data.n+3*data.m;

		return( TRUE );

		/* "gm" card, move structure or reproduce */
		/* original structure in new positions.   */
	  case GM:
		{
		  int tgf = (int)(rad + 0.5);
		  if( (tgf < 0) || (ns < 0) || (rad < 0.0) )
		  {
			fprintf( stderr, _("xnec2c: datagn(): move GM data card error\n") );
			Stop( _("datagn(): Move GM data card error"), ERR_OK );
			return( FALSE );
		  }
		  xw1= xw1* TORAD;
		  yw1= yw1* TORAD;
		  zw1= zw1* TORAD;
		  if( !move(xw1, yw1, zw1, xw2, yw2, zw2, (int)(rad+.5), ns, itg) )
			return( FALSE );
		}
		continue;

	  case SP: /* "sp" card, generate single new patch */
		ns++;

		if( itg != 0)
		{
		  fprintf( stderr, _("xnec2c: datagn(): patch data card error\n") );
		  Stop( _("datagn(): Patch data card error"), ERR_OK );
		  return( FALSE );
		}

		if( (ns == 2) || (ns == 4) )
		  isct=1;

		if( ns > 1)
		{
		  if( !readgm(gm, &ix, &iy, &x3, &y3,
				&z3, &x4, &y4, &z4, &dummy) )
			return( FALSE );

		  if( (ns == 2) || (itg > 0) )
		  {
			x4= xw1+ x3- xw2;
			y4= yw1+ y3- yw2;
			z4= zw1+ z3- zw2;
		  }

		  if( strcmp(gm, "SC") != 0 )
		  {
			fprintf( stderr, _("xnec2c: datagn(): patch data error\n") );
			Stop( _("datagn(): Patch data error"), ERR_OK );
			return( FALSE );
		  }

		} /* if( ns > 1) */
		else
		{
		  xw2= xw2* TORAD;
		  yw2= yw2* TORAD;
		}

		if( !patch( itg, ns, xw1, yw1, zw1, xw2,
			yw2, zw2, x3, y3, z3, x4, y4, z4) )
			return( FALSE );
		continue;

	  case SM: /* "sm" card, generate multiple-patch surface */
		if( (itg < 1) || (ns < 1) )
		{
		  fprintf( stderr, _("datagn(): xnec2c: patch card data error\n") );
		  Stop( _("datagn(): Patch data card error"), ERR_OK );
		  return( FALSE );
		}

		if( !readgm(gm, &ix, &iy, &x3, &y3,
			  &z3, &x4, &y4, &z4, &dummy) )
		  return( FALSE );

		if( (ns == 2) || (itg > 0) )
		{
		  x4= xw1+ x3- xw2;
		  y4= yw1+ y3- yw2;
		  z4= zw1+ z3- zw2;
		}

		if( strcmp(gm, "SC" ) != 0 )
		{
		  fprintf( stderr, _("xnec2c: datagn(): patch card data error\n") );
		  Stop( _("datagn(): Patch data card error"), ERR_OK );
		  return( FALSE );
		}

		if( !patch(itg, ns, xw1, yw1, zw1, xw2,
			yw2, zw2, x3, y3, z3, x4, y4, z4) )
		  return( FALSE );
		continue;

	  case GA: /* "ga" card, generate segment data for wire arc */
		if( Tag_Seg_Error(itg, ns) ) return( FALSE );
		nwire++;
		if( !arc(itg, ns, xw1, yw1, zw1, xw2) ) return( FALSE );
		continue;

	  case SC: /* "sc" card */
		if( isct == 0)
		{
		  fprintf( stderr, _("xnec2c: datagn(): patch data card error\n") );
		  Stop( _("datagn(): Patch data card error"), ERR_OK );
		  return( FALSE );
		}

		ns++;

		if( (itg != 0) || ((ns != 2) && (ns != 4)) )
		{
		  fprintf( stderr, _("xnec2c: datagn(): patch data card error\n") );
		  Stop( _("datagn(): Patch data card error"), ERR_OK );
		  return( FALSE );
		}

		xs1= x4;
		ys1= y4;
		zs1= z4;
		xs2= x3;
		ys2= y3;
		zs2= z3;
		x3= xw1;
		y3= yw1;
		z3= zw1;

		if( ns == 4)
		{
		  x4= xw2;
		  y4= yw2;
		  z4= zw2;
		}

		xw1= xs1;
		yw1= ys1;
		zw1= zs1;
		xw2= xs2;
		yw2= ys2;
		zw2= zs2;

		if( ns != 4)
		{
		  x4= xw1+ x3- xw2;
		  y4= yw1+ y3- yw2;
		  z4= zw1+ z3- zw2;
		}

		if( !patch(itg, ns, xw1, yw1, zw1, xw2,
			yw2, zw2, x3, y3, z3, x4, y4, z4) )
		  return( FALSE );
		continue;

	  case GH: /* "gh" card, generate helix */
		if( Tag_Seg_Error(itg, ns) ) return( FALSE );
		nwire++;
		helix( xw1, yw1, zw1, xw2, yw2, zw2, rad, ns, itg);
		continue;

	  case GF: /* "gf" card, not supported */
		fprintf( stderr, _("xnec2c: datagn(): \"GF\" card (NGF solution) "
				"is not supported\n") );
		Stop( _("datagn(): \"GF\" card (NGF solution)\n"
			  "is not supported"), ERR_OK );
		return( FALSE );

	  case CT: /* Ignore in-data comments (NEC4 compatibility) */
		fprintf( stderr, _("xnec2c: datagn(): ignoring CM card in geometry\n") );
		Stop( _("datagn(): Ignoring CM card in geometry"), ERR_OK );
		continue;

	  default: /* error message */
		fprintf( stderr, _("xnec2c: datagn(): geometry data card error\n") );
		fprintf( stderr,
			"%2s %3d %5d %10.5f %10.5f %10.5f"
			" %10.5f %10.5f %10.5f %10.5f\n",
			gm, itg, ns, xw1, yw1, zw1, xw2, yw2, zw2, rad );

		Stop( _("datagn(): Geometry data card error"), ERR_OK );
		return( FALSE );

	} /* switch( gm_num ) */

  } /* do */
  while( TRUE );

} /* datagn() */

/*-----------------------------------------------------------------------*/

/* Read_Geometry()
 *
 * Reads geometry data from input file
 */
  gboolean
Read_Geometry( void )
{
  int idx;
  size_t mreq;

  /* Moved here from Read_Commands() */
  matpar.imat=0;
  data.n = data.m = 0;
  if( !datagn() ) return( FALSE );

  /* Memory allocation for temporary buffers */
  mreq = (size_t)data.npm * sizeof(double);
  mem_realloc( (void **)&save.xtemp,  mreq, "in input.c" );
  mem_realloc( (void **)&save.ytemp,  mreq, "in input.c" );
  mem_realloc( (void **)&save.ztemp,  mreq, "in input.c" );
  mem_realloc( (void **)&save.bitemp, mreq, "in input.c" );
  if( data.n > 0 )
  {
	mreq = (size_t)data.n * sizeof(double);
	mem_realloc( (void **)&save.sitemp, mreq, "in input.c" );
  }

  /* Memory allocation for primary interacton matrix. */
  mreq = (size_t)(data.np2m * (data.np + 2 * data.mp)) * sizeof(complex double);
  mem_realloc( (void **)&cm, mreq, "in input.c" );

  /* Memory allocation for current buffers */
  mreq = (size_t)data.npm * sizeof( double);
  mem_realloc( (void **)&crnt.air, mreq, "in input.c" );
  mem_realloc( (void **)&crnt.aii, mreq, "in input.c" );
  mem_realloc( (void **)&crnt.bir, mreq, "in input.c" );
  mem_realloc( (void **)&crnt.bii, mreq, "in input.c" );
  mem_realloc( (void **)&crnt.cir, mreq, "in input.c" );
  mem_realloc( (void **)&crnt.cii, mreq, "in input.c" );
  mreq = (size_t)data.np3m * sizeof( complex double);
  mem_realloc( (void **)&crnt.cur, mreq, "in input.c" );

  /* Memory allocation for loading buffers */
  mreq = (size_t)data.npm * sizeof(complex double);
  mem_realloc( (void **)&zload.zarray, mreq, "in input.c" );

  /* Save segment and patch data for freq scaling */
  if( data.n > 0 )
	for( idx = 0; idx < data.n; idx++ )
	{
	  save.xtemp[idx]  = data.x[idx];
	  save.ytemp[idx]  = data.y[idx];
	  save.ztemp[idx]  = data.z[idx];
	  save.sitemp[idx] = data.si[idx];
	  save.bitemp[idx] = data.bi[idx];
	}

  if( data.m > 0 )
	for( idx = 0; idx < data.m; idx++ )
	{
	  int j;

	  j = idx + data.n;
	  save.xtemp[j]  = data.px[idx];
	  save.ytemp[j]  = data.py[idx];
	  save.ztemp[j]  = data.pz[idx];
	  save.bitemp[j] = data.pbi[idx];
	}

  return( TRUE );
} /* Read_Geometry() */

/*------------------------------------------------------------------------*/

/* Read_Commands()
 *
 * Reads commands from input file and stores
 * them for later execution by user command
 */
  gboolean
Read_Commands( void )
{
  /* input card mnemonic list */
  char *atst[NUM_CMNDS] = { COMMANDS };

  char ain[3];
  double tmp1, tmp2, tmp3, tmp4, tmp5, tmp6;
  int
	mpcnt, itmp1, itmp2, itmp3, itmp4,
	ain_num;   /* My addition, ain mnemonic as a number */
  size_t mreq; /* My addition, size req. for malloc's   */

  /* initializations etc from original fortran code */
  mpcnt = 0;

  /* Matrix parameters */
  if( matpar.imat == 0)
  {
	netcx.neq  = data.n+2*data.m;
	netcx.neq2 = 0;
  }

  /* default values for input parameters and flags */
  calc_data.rkh = 1.0;
  calc_data.iexk = 0;
  calc_data.iped = 0;
  calc_data.nfrq = 1;
  calc_data.fmhz = CVEL;
  save.fmhz = CVEL;
  calc_data.mxfrq = 0.0;
  fpat.dth   = 0.0;
  fpat.thets = 0.0;
  fpat.ixtyp = 0;
  fpat.nfeh  = 0;
  fpat.nrx   = 0;
  fpat.nry   = 0;
  fpat.nry   = 0;
  fpat.nth   = 0;
  fpat.nph   = 0;
  fpat.near = -1;
  gnd.ifar  = -1;
  gnd.zrati = CPLX_10;
  gnd.ksymp = 1;
  gnd.nradl = 0;
  gnd.iperf = 0;
  netcx.nonet = 0;
  netcx.ntsol = 0;
  netcx.masym = 0;
  netcx.npeq = data.np+2*data.mp;

  /* My additions */
  vsorc.nvqd   = 0;
  vsorc.nsant  = 0;
  zload.nldseg = 0;
  zload.nload  = 0;

  /* Allocate some buffers */
  mreq = (size_t)data.np2m * sizeof(int);
  mem_realloc( (void **)&save.ip, mreq, "in input.c" );

  /* Memory allocation for symmetry array */
  smat.nop = netcx.neq/netcx.npeq;
  mreq = (size_t)(smat.nop * smat.nop) * sizeof( complex double);
  mem_realloc( (void **)&smat.ssx, mreq, "in input.c" );

  /* main input section, exits at various points */
  /* depending on error conditions or end of job */
  while( TRUE )
  {
	/* Main input section - standard read statement - jumps */
	/* to appropriate section for specific parameter set up */
	if( !readmn(
		  ain, &itmp1, &itmp2, &itmp3, &itmp4,
		  &tmp1, &tmp2, &tmp3, &tmp4, &tmp5, &tmp6) )
	  return( FALSE );

	mpcnt++;

	/* identify command card id mnemonic */
	for( ain_num = 0; ain_num < NUM_CMNDS; ain_num++ )
	  if( strncmp( ain, atst[ain_num], 2) == 0 )
		break;

	/* take action according to card id mnemonic */
	switch( ain_num )
	{
	  case CM: /* "cm" card ignored, comments in data cards as in NEC4 */
		fprintf( stderr, _("xnec2c: Read_Commands():"
			" ignoring CM card in commands\n") );
		Stop( _("Read_Commands(): Ignoring CM card in commands"),
			ERR_OK );
		continue;

	  case CP: /* "cp" card ignored, maximum coupling between antennas */
		Stop( _("Read_Commands(): CP card is ignored\n"
			  "Coupling calculation not implemented"), ERR_OK );
		continue; /* continue card input loop */

	  case EK: /* "ek" card,  extended thin wire kernel option */
		if( itmp1 == -1)
		  calc_data.iexk = 0;
		else
		  calc_data.iexk = 1;
		continue; /* continue card input loop */

	  case EN: /* "en" card, end data input, no action */
		break;

	  case EX: /* "ex" card, excitation parameters */
		netcx.masym = itmp4/10;
		fpat.ixtyp  = itmp1;

		/* Applied E field or current discontinuity */
		if( (fpat.ixtyp == 0) || (fpat.ixtyp == 5) )
		{
		  netcx.ntsol = 0;

		  /* Current discontinuity E source */
		  if( fpat.ixtyp == 5 )
		  {
			vsorc.nvqd++;
			mreq = (size_t)vsorc.nvqd * sizeof(int);
			mem_realloc( (void **)&vsorc.ivqd, mreq, "in input.c" );
			mem_realloc( (void **)&vsorc.iqds, mreq, "in input.c" );
			mreq = (size_t)vsorc.nvqd * sizeof(complex double);
			mem_realloc( (void **)&vsorc.vqd, mreq, "in input.c" );
			mem_realloc( (void **)&vsorc.vqds, mreq, "in input.c" );
			{
			  int indx = vsorc.nvqd-1;

			  if( (vsorc.ivqd[indx] = isegno(itmp2, itmp3)) < 0 )
				return( FALSE ); /* my addition, error */
			  vsorc.vqd[indx]= cmplx( tmp1, tmp2);
			  if( cabs( vsorc.vqd[indx]) < 1.0e-20)
				vsorc.vqd[indx] = CPLX_10;

			  calc_data.iped   = itmp4- netcx.masym*10;
			  calc_data.zpnorm = tmp3;
			}

		  } /* if( fpat.ixtyp == 5) */
		  else
		  {
			/* Else, applied E field */
			vsorc.nsant++;
			mreq = (size_t)vsorc.nsant * sizeof(int);
			mem_realloc( (void **)&vsorc.isant, mreq, "in input.c" );
			mreq = (size_t)vsorc.nsant * sizeof(complex double);
			mem_realloc( (void **)&vsorc.vsant, mreq, "in input.c" );
			{
			  int indx = vsorc.nsant-1;

			  if( (vsorc.isant[indx] = isegno(itmp2, itmp3)) < 0 )
				  return( FALSE ); /* my addition, error condition */
			  vsorc.vsant[indx]= cmplx( tmp1, tmp2);
			  if( cabs( vsorc.vsant[indx]) < 1.0e-20)
				vsorc.vsant[indx] = CPLX_10;

			  calc_data.iped= itmp4- netcx.masym*10;
			  calc_data.zpnorm= tmp3;
			  if( (calc_data.iped == 1) && (calc_data.zpnorm > 0.0) )
				calc_data.iped=2;
			}

		  } /* if( fpat.ixtyp == 5 ) */

		} /* if( (fpat.ixtyp == 0) || (fpat.ixtyp == 5) ) */
		else
		{
		  /* For fpat.ixtyp = 2|3|4 = incident field or current source */
		  calc_data.nthi= itmp2;
		  calc_data.nphi= itmp3;
		  calc_data.xpr1= tmp1;
		  calc_data.xpr2= tmp2;
		  calc_data.xpr3= tmp3;
		  calc_data.xpr4= tmp4;
		  calc_data.xpr5= tmp5;
		  calc_data.xpr6= tmp6;
		  vsorc.nsant=0;
		  vsorc.nvqd=0;
		  calc_data.thetis= tmp1;
		  calc_data.phiss= tmp2;
		}

		SetFlag( ENABLE_EXCITN );
		continue; /* continue card input loop */

	  case FR: /* "fr" card, frequency parameters */
		if( !CHILD )
		{
		  calc_data.nfrq = itmp2;
		  if( calc_data.nfrq <= 0)
			calc_data.nfrq = 1;
		}
		else calc_data.nfrq = 1;

		/* Allocate normalization buffer */
		{
		  mreq = (size_t)calc_data.nfrq * sizeof(double);
		  mem_realloc( (void **)&impedance_data.zreal, mreq, "in input.c" );
		  mem_realloc( (void **)&impedance_data.zimag, mreq, "in input.c" );
		  mem_realloc( (void **)&impedance_data.zmagn, mreq, "in input.c" );
		  mem_realloc( (void **)&impedance_data.zphase, mreq, "in input.c" );
		  mem_realloc( (void **)&save.freq, mreq, "in input.c" );
		  mreq = (size_t)calc_data.nfrq * sizeof(char);
		  mem_realloc( (void **)&save.fstep, mreq, "in input.c" );
		}

		if( CHILD ) continue;

		calc_data.ifrq = itmp1;
		calc_data.fmhz = save.fmhz = tmp1;

		/* My addition, max frequency */
		if( itmp1 == 0 )
		  calc_data.mxfrq =
			(double)tmp1 + (double)tmp2 * (double)(itmp2-1);
		else if( itmp1 == 1 )
		  calc_data.mxfrq =
			(double)tmp1 * pow( (double)tmp2, (double)(itmp2-1) );

		/* My addition, extra features in "fr" card. */
		/* Specifies lower and upper value of frequency range */
		if( calc_data.ifrq == 2 )
		{
		  calc_data.nfrq++;
		  /* Linear frequency stepping */
		  if( calc_data.nfrq > 1 )
			calc_data.delfrq =
			  (tmp2 - tmp1)/(double)(calc_data.nfrq-1);
		  calc_data.ifrq  = 0;
		  calc_data.mxfrq = (double)tmp2; /* Max frequency */
		}
		else if( calc_data.ifrq == 3 )
		{
		  calc_data.nfrq++;
		  /* Multiplicative frequency stepping */
		  if( calc_data.nfrq > 1 )
			calc_data.delfrq =
			  pow( (tmp2-tmp1), 1.0/(double)(calc_data.nfrq-1) );
		  calc_data.ifrq  = 1;
		  calc_data.mxfrq = (double)tmp2; /* Max frequency */
		}
		else calc_data.delfrq = tmp2;

		if( calc_data.iped == 1)
		  calc_data.zpnorm = 0.0;
		continue; /* continue card input loop */

	  case GD: /* "gd" card, ground representation */
		fpat.epsr2 = tmp1;
		fpat.sig2  = tmp2;
		fpat.clt   = tmp3;
		fpat.cht   = tmp4;
		continue; /* continue card input loop */

	  case GN: /* "gn" card, ground parameters under the antenna */
		gnd.iperf = itmp1;
		gnd.nradl = itmp2;
		gnd.ksymp = 2;
		save.epsr = tmp1;
		save.sig  = tmp2;

		/* Theta must be less than 90 if ground present */
		double test = (double)(fpat.nth - 1) * fpat.dth + fpat.thets;
		if( (gnd.ifar != 1) && (test > 90.0) )
		{
		  fprintf( stderr,
			  _("xnec2c: Read_Commands(): theta > 90 deg. with ground specified\n") );
		  Stop( _("Read_Commands(): Theta > 90 deg with ground specified\n"
				"Please check RP card data and correct"), ERR_OK );
		  return( FALSE );
		}

		if( gnd.nradl > 0)
		{
		  if( gnd.iperf == 2)
		  {
			fprintf( stderr,
				_("xnec2c: Read_Commands(): radial wire g.s. approximation\n"
				"may not be used with Sommerfeld ground option\n") );
			Stop( _("Read_Commands(): radial wire g.s. approximation\n"
				  "may not be used with Sommerfeld ground option"), ERR_OK );
			return( FALSE );
		  }

		  save.scrwlt = tmp3;
		  save.scrwrt = tmp4;
		  continue; /* continue card input loop */
		} /* if( gnd.nradl > 0) */
		else
		{
		  save.scrwlt = 0.0;
		  save.scrwrt = 0.0;
		}

		fpat.epsr2 = tmp3;
		fpat.sig2  = tmp4;
		fpat.clt   = tmp5;
		fpat.cht   = tmp6;
		continue; /* continue card input loop */

	  case KH: /* "kh" card, matrix integration limit */
		calc_data.rkh = tmp1;
		continue; /* continue card input loop */

	  case LD: /* "ld" card, loading parameters */
		{
		  int idx, nseg;

		  /* Reallocate loading buffers */
		  zload.nload++;
		  mreq = (size_t)zload.nload * sizeof(int);
		  mem_realloc( (void **)&calc_data.ldtyp,  mreq, "in input.c" );
		  mem_realloc( (void **)&calc_data.ldtag,  mreq, "in input.c" );
		  mem_realloc( (void **)&calc_data.ldtagf, mreq, "in input.c" );
		  mem_realloc( (void **)&calc_data.ldtagt, mreq, "in input.c" );

		  mreq = (size_t)zload.nload * sizeof(double);
		  mem_realloc( (void **)&calc_data.zlr, mreq, "in input.c" );
		  mem_realloc( (void **)&calc_data.zli, mreq, "in input.c" );
		  mem_realloc( (void **)&calc_data.zlc, mreq, "in input.c" );

		  idx = zload.nload-1;
		  calc_data.ldtyp[idx]= itmp1;
		  calc_data.ldtag[idx]= itmp2;
		  if( itmp4 == 0) itmp4= itmp3;
		  calc_data.ldtagf[idx]= itmp3;
		  calc_data.ldtagt[idx]= itmp4;

		  if( itmp4 < itmp3 )
		  {
			fprintf( stderr,
				_("xnec2c: Read_Commands(): data fault on loading card no %d\n"
				"itag step1 %d is greater than itag step2 %d\n"),
				zload.nload, itmp3, itmp4 );
			Stop( _("Read_Commands(): Data fault on loading card\n"
				  "itag step1 is greater than itag step2"), ERR_OK );
			return( FALSE );
		  }

		  calc_data.zlr[idx]= tmp1;
		  calc_data.zli[idx]= tmp2;
		  calc_data.zlc[idx]= tmp3;

		  /*** My addition, for drawing structure ***/
		  /* Work out absolute seg number of loaded segments (!) */
		  if( itmp1 == -1 ) /* Null all loads */
			zload.nldseg = 0;
		  else if( itmp2 == 0 ) /* No tag num */
		  {
			if( itmp3 == 0 ) /* All segs loaded */
			{
			  nseg = data.n;
			  mreq = (size_t)(nseg + zload.nldseg) * sizeof(int);
			  mem_realloc( (void **)&zload.ldsegn, mreq, "in input.c" );
			  mem_realloc( (void **)&zload.ldtype, mreq, "in input.c" );
			  for( idx = 0; idx < nseg; idx++ )
			  {
				zload.ldtype[zload.nldseg]   = itmp1;
				zload.ldsegn[zload.nldseg++] = idx+1;
			  }
			}
			else /* Absolute seg num specified */
			{
			  nseg = itmp4 - itmp3 + 1;
			  if( nseg <= 0 ) nseg = 1;
			  mreq = (size_t)(nseg + zload.nldseg) * sizeof(int);
			  mem_realloc( (void **)&zload.ldsegn, mreq, "in input.c" );
			  mem_realloc( (void **)&zload.ldtype, mreq, "in input.c" );
			  for( idx = 0; idx < nseg; idx++ )
			  {
				zload.ldtype[zload.nldseg]   = itmp1;
				zload.ldsegn[zload.nldseg++] = itmp3+idx;
			  }
			}

		  } /* if( itmp2 == 0 ) */
		  else /* Tag num specified */
		  {
			if( (itmp3 == 0) && (itmp4 == 0) ) /* All segs of tag loaded */
			{
			  for( idx = 0; idx < data.n; idx++ )
				if( data.itag[idx] == itmp2 )
				{
				  mreq = (size_t)(zload.nldseg + 1) * sizeof(int);
				  mem_realloc( (void **)&zload.ldsegn, mreq, "in input.c" );
				  mem_realloc( (void **)&zload.ldtype, mreq, "in input.c" );
				  zload.ldtype[zload.nldseg]   = itmp1;
				  zload.ldsegn[zload.nldseg++] = idx+1;
				}
			}
			else /* A range of segs of tag loaded */
			{
			  nseg = itmp4 - itmp3 + 1;
			  if( nseg <= 0 ) nseg = 1; /* Just one seg of tag (=itmp3) */
			  mreq = (size_t)(zload.nldseg + nseg) * sizeof(int);
			  mem_realloc( (void **)&zload.ldsegn, mreq, "in input.c" );
			  mem_realloc( (void **)&zload.ldtype, mreq, "in input.c" );
			  for( idx = 0; idx < nseg; idx++ )
			  {
				zload.ldtype[zload.nldseg] = itmp1;
				if( (zload.ldsegn[zload.nldseg++] =
					  isegno(itmp2, itmp3+idx)) < 0 )
				  return( FALSE );
			  }
			}
		  }
		} /* case 8: */
		continue; /* continue card input loop */

	  case NE: case NH:  /* "negcnh" cards, near field calculation parameters */
		if( ain_num == 11 )
		  fpat.nfeh |= NEAR_HFIELD;
		else
		  fpat.nfeh |= NEAR_EFIELD;

		fpat.near = itmp1;
		fpat.nrx  = itmp2;
		fpat.nry  = itmp3;
		fpat.nrz  = itmp4;
		fpat.xnr  = tmp1;
		fpat.ynr  = tmp2;
		fpat.znr  = tmp3;
		fpat.dxnr = tmp4;
		fpat.dynr = tmp5;
		fpat.dznr = tmp6;

		if( (fpat.nrx > 0) &&
			(fpat.nry > 0) &&
			(fpat.nrz > 0) &&
			(fpat.near != -1) )
		{
		  SetFlag( ENABLE_NEAREH );
		  SetFlag( ALLOC_NEAREH_BUFF );
		  SetFlag( ALLOC_PNTING_BUFF );
		}

		/* Because of the interactive GUI, program
		 * execution is not triggered by any card */
		continue; /* continue card input loop */

	  case NT: case TL: /* "nt" & "tl" cards, network parameters */
		{
		  int idx;

		  /* Re-allocate network buffers */
		  netcx.nonet++;
		  mreq = (size_t)netcx.nonet * sizeof(int);
		  mem_realloc( (void **)&netcx.ntyp,  mreq, "in input.c" );
		  mem_realloc( (void **)&netcx.iseg1, mreq, "in input.c" );
		  mem_realloc( (void **)&netcx.iseg2, mreq, "in input.c" );

		  mreq = (size_t)netcx.nonet * sizeof(double);
		  mem_realloc( (void **)&netcx.x11r, mreq, "in input.c" );
		  mem_realloc( (void **)&netcx.x11i, mreq, "in input.c" );
		  mem_realloc( (void **)&netcx.x12r, mreq, "in input.c" );
		  mem_realloc( (void **)&netcx.x12i, mreq, "in input.c" );
		  mem_realloc( (void **)&netcx.x22r, mreq, "in input.c" );
		  mem_realloc( (void **)&netcx.x22i, mreq, "in input.c" );

		  idx = netcx.nonet-1;
		  if( ain_num == 12 )
			netcx.ntyp[idx] = 1;
		  else
		  {
			netcx.ntyp[idx] = 2;
			if( tmp1 == 0.0 )
			{
			  fprintf( stderr,
				  _("xnec2c: Read_Commands(): Transmission Line impedance = 0\n"
				  "is not valid. Please correct NT or TL card\n") );
			  Stop( _("Read_Commands(): Transmission Line impedance = 0\n"
					"is not valid. Please correct NT or TL card"), ERR_OK );
			  return( FALSE );
			}
		  }

		  if( ((netcx.iseg1[idx] = isegno(itmp1, itmp2)) < 0) ||
			  ((netcx.iseg2[idx] = isegno(itmp3, itmp4)) < 0) )
		  {
			fprintf( stderr,
				_("xnec2c: Read_Commands(): Segment number error in TL or NT card\n") );
			Stop( _("Read_Commands(): Segment number\n"
				"error in NT or TL card"), ERR_OK );
			return( FALSE );
		  }
		  netcx.x11r[idx]  = tmp1;
		  netcx.x11i[idx]  = tmp2;
		  netcx.x12r[idx]  = tmp3;
		  netcx.x12i[idx]  = tmp4;
		  netcx.x22r[idx]  = tmp5;
		  netcx.x22i[idx]  = tmp6;

		  if( (netcx.ntyp[idx] == 1) || (tmp1 > 0.0) )
			continue; /* continue card input loop */

		  netcx.ntyp[idx] = 3;
		  netcx.x11r[idx] = -tmp1;
		  continue; /* continue card input loop */

		} /* case 12: case 17: */

	  case PQ: case PT: /* "pq" and "pt" cards ignored, no printing */
		fprintf( stderr,
			_("xnec2c: Read_Commands(): PQ and PT cards are ignored\n"
			"Printing to file not implemented\n") );
		Stop( _("Read_Commands(): PQ and PT cards are ignored\n"
			  "Printing to file not implemented"), ERR_OK );
		continue; /* continue card input loop */

	  case RP: /* "rp" card, standard observation angle parameters */
		if( itmp1 == 1 )
		{
		  fprintf( stderr,
			  _("xnec2c: Read_Commands(): Surface wave option (I1=1)\n"
			  "of RP command not implemented\n") );
		  Stop( _("Read_Commands(): Surface wave option (I1=1)\n"
				"of RP command not implemented"), ERR_OK );
		  return( FALSE );
		}

		gnd.ifar = itmp1;
		fpat.nth = itmp2;
		fpat.nph = itmp3;

		if( fpat.nth <= 0)
		  fpat.nth=1;
		if( fpat.nph <= 0)
		  fpat.nph=1;

		fpat.ipd  = itmp4 / 10;
		fpat.iavp = itmp4 - fpat.ipd*10;
		fpat.inor = fpat.ipd / 10;
		fpat.ipd  = fpat.ipd - fpat.inor*10;
		fpat.iax  = fpat.inor / 10;
		fpat.inor = fpat.inor - fpat.iax*10;

		if( fpat.iavp )
		{
		  fprintf( stderr,
			  _("xnec2c: Read_Commands(): Gain averaging (XNDA ***1 or ***2)\n"
			  "of RP command not implemented\n") );
		  Stop( _("Read_Commands(): Gain averaging (XNDA ***1 or ***2)\n"
				"of RP command not supported"), ERR_OK );
		  return( FALSE );
		}
		if( fpat.iax != 0) fpat.iax = 1;
		if( fpat.ipd != 0) fpat.ipd = 1;
		fpat.thets = tmp1;
		fpat.phis  = tmp2;
		fpat.dth   = tmp3;
		fpat.dph   = tmp4;
		fpat.rfld  = tmp5;
		fpat.gnor  = tmp6;

		/* Theta must be less than 90 if ground present */
		tmp1 = (double)(fpat.nth - 1) * fpat.dth + fpat.thets;
		if( (gnd.ksymp == 2) && (gnd.ifar != 1) && (tmp1 > 90.0) )
		{
		  fprintf( stderr,
			  _("xnec2c: Read_Commands(): Theta > 90 deg. with ground specified\n"
			  "Please check RP card data and correct\n") );
		  Stop( _("Read_Commands(): Theta > 90 deg. with ground specified\n"
				"Please check RP card data and correct"), ERR_OK );
		  return( FALSE );
		}

		/* Because of the interactive GUI, program
		 * execution is not triggered by any card */
		continue; /* continue card input loop */

	  case SY: /* "sy" TODO Compatibility with 4nec2.
				  Too difficult, may never happen :-( */
		continue;

	  case XQ: /* "xq" execute card */
		/* Because of the interactive GUI, program
		 * execution is not triggered by any card.
		 * XQ now is the same as EN because of above */
		if( itmp1 == 0 )
		  break; /* No rad pat, stop reading commands */

		/* Read radiation pattern parameters */
		gnd.ifar  = 0;
		fpat.rfld = 0.0;
		fpat.ipd  = 0;
		fpat.iavp = 0;
		fpat.inor = 0;
		fpat.iax  = 0;
		fpat.nth  = 91;
		fpat.nph  = 1;
		fpat.thets= 0.0;
		fpat.phis = 0.0;
		fpat.dth  = 1.0;
		fpat.dph  = 0.0;

		if( itmp1 == 2)
		  fpat.phis = 90.0;
		else if( itmp1 == 3)
		{
		  fpat.nph = 2;
		  fpat.dph = 90.0;
		}

		SetFlag( ENABLE_RDPAT );

		/* Because of the interactive GUI, program
		 * execution is not triggered by any card.
		 * XQ now is the same as EN because of above */
		break;

	  case ZO: /* My addition, impedance against which VSWR is calculated */
		calc_data.zo = (double)itmp1;

		/* Set the Zo spinbutton value */
		if( freqplots_window_builder )
		{
		  GtkWidget *spin = Builder_Get_Object(
			  freqplots_window_builder, "freqplots_zo_spinbutton" );
		  gtk_spin_button_set_value( GTK_SPIN_BUTTON(spin), (gdouble)calc_data.zo );
		}
		continue;

	  default:
		fprintf( stderr,
			_("xnec2c: Read_Commands(): faulty data "
			"card label after geometry section\n") );
		Stop( _("Read_Commands(): Faulty data card\n"
			  "label after geometry section"), ERR_OK );
		return( FALSE );
	} /* switch( ain_num ) */

	/* Disable radiation pattern plots */
	if( (fpat.nth < 1) || (fpat.nph < 1) || (gnd.ifar == 1) )
	  ClearFlag( ENABLE_RDPAT );
	else
	{
	  /* Allocate radiation pattern buffers */
	  Alloc_Rdpattern_Buffers( calc_data.nfrq+1, fpat.nth, fpat.nph );
	  SetFlag( ENABLE_RDPAT );
	}

	return( TRUE );
  } /* while( TRUE ) */

} /* Read_Commands() */

/*-----------------------------------------------------------------------*/

  gboolean
readmn( char *mn, int *i1, int *i2, int *i3, int *i4,
	double *f1, double *f2, double *f3,
	double *f4, double *f5, double *f6 )
{
  char
	*line_buf = NULL,
	*startptr = NULL,
	*endptr   = NULL;
  int len, i, idx;
  int nint = 4, nflt = 6;
  int iarr[4] = { 0, 0, 0, 0 };
  double rarr[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
  int eof; /* EOF error flag */


  /* Clear return values */
  *i1 = *i2 = *i3 = *i4 = 0;
  *f1 = *f2 = *f3 = *f4 = *f5 = *f6 = 0.0;

  /* read a line from input file */
  mem_alloc((void **)&line_buf, LINE_LEN, "in readmn()");
  if( line_buf == NULL ) return( FALSE );
  startptr = line_buf;
  eof = Load_Line( line_buf, input_fp );

  /* Capitalize first two characters (mnemonics) */
  if( (line_buf[0] > 0x60) && (line_buf[0] < 0x79) )
	line_buf[0] = (char)toupper( (int)line_buf[0] );
  if( (line_buf[1] > 0x60) && (line_buf[1] < 0x79) )
	line_buf[1] = (char)toupper( (int)line_buf[1] );

  if( eof == EOF )
  {
	Strlcpy( mn, "EN", 3 );
	fprintf( stderr,
		_("xnec2c: readmn(): command data card error\n"
		"Unexpected EOF while reading input file - appending EN card\n") );
	Stop( _("readmn(): Command data card error\n"
		  "Unexpected EOF while reading input file\n"
			"Uppending a default EN card"), ERR_OK );
	free_ptr( (void **)&startptr );
	return( FALSE );
  }

  /* get line length */
  len = (int)strlen( line_buf );

  /* abort if card's mnemonic too short or missing */
  if( len < 2 )
  {
	Strlcpy( mn, "XX", 3 );
	fprintf( stderr,
		_("xnec2c: readmn(): command data card error\n"
		"card's mnemonic code too short or missing\n") );
	Stop( _("readmn(): Command data card error\n"
		  "Mnemonic code too short or missing"), ERR_OK );
	free_ptr( (void **)&startptr );
	return( FALSE );
  }

  /* extract card's mnemonic code */
  Strlcpy( mn, line_buf, 3 );

  /* Return if only mnemonic on card */
  if( len == 2 )
  {
	free_ptr( (void **)&startptr );
	return( TRUE );
  }

  /* Compatibility with NEC4, comments between data cards */
  if( strncmp(mn, "CM", 2) == 0 )
  {
	free_ptr( (void **)&startptr );
	return( TRUE );
  }

  /* check line for spurious characters */
  for( idx = 2; idx < len; idx++ )
  {
	if( ((line_buf[idx] >= '0') &&
		 (line_buf[idx] <= '9')) ||
		 (line_buf[idx] == ' ')  ||
		 (line_buf[idx] == '.')  ||
		 (line_buf[idx] == ',')  ||
		 (line_buf[idx] == '+')  ||
		 (line_buf[idx] == '-')  ||
		 (line_buf[idx] == 'E')  ||
		 (line_buf[idx] == 'e')  ||
		 (line_buf[idx] == '\t') ||
		 (line_buf[idx] == '\0') )
	  continue;
	else
	  break;
  }
  if( idx < len )
  {
	fprintf( stderr,
		_("xnec2c: readmn(): command data card \"%s\" error\n"
		"Spurious character '%c' at column %d\n"),
		mn, line_buf[idx], idx+1 );
	Stop( _("readmn(): Command data card error\n"
		  "Spurious character in command card"), ERR_OK );
	free_ptr( (void **)&startptr );
	return( FALSE );
  }

  /* Read integers from line */
  line_buf += 2;
  for( i = 0; i < nint; i++ )
  {
	/* read an integer from line */
	iarr[i] = (int)strtol( line_buf, &endptr, 10 );
	if( *endptr == '\0' ) break;
	line_buf = endptr + 1;
  } /* for( i = 0; i < nint; i++ ) */

  /* Return if no floats are specified in the card */
  if( *endptr == '\0' )
  {
	*i1= iarr[0];
	*i2= iarr[1];
	*i3= iarr[2];
	*i4= iarr[3];
	*f1= rarr[0];
	*f2= rarr[1];
	*f3= rarr[2];
	*f4= rarr[3];
	*f5= rarr[4];
	*f6= rarr[5];
	free_ptr( (void **)&startptr );
	return( TRUE );
  }

  /* read doubles from line */
  for( i = 0; i < nflt; i++ )
  {
	/* read a double from line */
	rarr[i] = Strtod( line_buf, &endptr );
	if( *endptr == '\0' ) break;
	line_buf = endptr + 1;
  } /* for( i = 0; i < nflt; i++ ) */

  /* Return values on normal exit */
  *i1= iarr[0];
  *i2= iarr[1];
  *i3= iarr[2];
  *i4= iarr[3];
  *f1= rarr[0];
  *f2= rarr[1];
  *f3= rarr[2];
  *f4= rarr[3];
  *f5= rarr[4];
  *f6= rarr[5];

  free_ptr( (void **)&startptr );
  return( TRUE );
} /* readmn() */

/*-----------------------------------------------------------------------*/

  gboolean
readgm( char *gm, int *i1, int *i2, double *x1,
	double *y1, double *z1, double *x2,
	double *y2, double *z2, double *rad )
{
  char
	*line_buf = NULL,
	*startptr = NULL,
	*endptr   = NULL;
  int len, i, idx;
  int nint = 2, nflt = 7;
  int iarr[2] = { 0, 0 };
  double rarr[7] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
  int eof; /* EOF error flag */


  /* Clear return values */
  *i1 = *i2 = 0;
  *x1 = *y1 = *z1 = *x2 = *y2 = *z2 = *rad = 0.0;

  /* read a line from input file */
  mem_alloc((void **)&line_buf, LINE_LEN, "in readgm()");
  if( line_buf == NULL ) return( FALSE );
  startptr = line_buf;
   eof = Load_Line( line_buf, input_fp );

  /* Capitalize first two characters (mnemonics) */
  if( (line_buf[0] > 0x60) && (line_buf[0] < 0x79) )
	line_buf[0] = (char)toupper( (int)line_buf[0] );
  if( (line_buf[1] > 0x60) && (line_buf[1] < 0x79) )
	line_buf[1] = (char)toupper( (int)line_buf[1] );

  if( eof == EOF )
  {
	Strlcpy( gm, "GE", 3 );
	fprintf( stderr,
		_("xnec2c: readgm(): geometry data card error\n"
		"Unexpected EOF while reading input file - appending GE card\n") );
	Stop( _("readgm(): Geometry data card error\n"
		  "Unexpected EOF while reading input file\n"
		  "Uppending a default GE card"), ERR_OK );
	free_ptr( (void **)&startptr );
	return( FALSE );
  }

  /* get line length */
  len = (int)strlen( line_buf );

  /* abort if card's mnemonic too short or missing */
  if( len < 2 )
  {
	Strlcpy( gm, "XX", 3 );
	fprintf( stderr,
		_("xnec2c: readgm(): geometry data card error\n"
		"card's mnemonic code too short or missing\n") );
	Stop( _("readgm(): Geometry data card error\n"
		  "Card's mnemonic code too short or missing"), ERR_OK );
	free_ptr( (void **)&startptr );
	return( FALSE );
  }

  /* extract card's mnemonic code */
  Strlcpy( gm, line_buf, 3 );

  /* Return if only mnemonic on card */
  if( len == 2 )
  {
	free_ptr( (void **)&startptr );
	return( TRUE );
  }

  /* Compatibility with NEC4,
   * comments between data cards */
  if( strcmp(gm, "CM") == 0 )
  {
	free_ptr( (void **)&startptr );
	return( TRUE );
  }

  /* check line for spurious characters */
  for( idx = 2; idx < len; idx++ )
  {
	if( ((line_buf[idx] >= '0') &&
		 (line_buf[idx] <= '9')) ||
		 (line_buf[idx] == ' ')  ||
		 (line_buf[idx] == '.')  ||
		 (line_buf[idx] == ',')  ||
		 (line_buf[idx] == '+')  ||
		 (line_buf[idx] == '-')  ||
		 (line_buf[idx] == 'E')  ||
		 (line_buf[idx] == 'e')  ||
		 (line_buf[idx] == '\t') ||
		 (line_buf[idx] == '\0') )
	  continue;
	else
	  break;
  }
  if( idx < len )
  {
	fprintf( stderr,
		_("xnec2c: readgm(): geometry data card \"%s\" error\n"
		"Spurious character '%c' at column %d\n"),
		gm, line_buf[idx], idx+1 );
	Stop( _("readmn(): Geometry data card error\n"
		  "Spurious character in command card"), ERR_OK );
	free_ptr( (void **)&startptr );
	return( FALSE );
  }

  /* read integers from line */
  line_buf += 2;
  for( i = 0; i < nint; i++ )
  {
	/* read an integer from line, reject spurious chars */
	iarr[i] = (int)strtol( line_buf, &endptr, 10 );
	if( *endptr == '\0' ) break;
	line_buf = endptr + 1;
  } /* for( i = 0; i < nint; i++ ) */

  /* Return if no floats are specified in the card */
  if( *endptr == '\0' )
  {
	*i1 = iarr[0];
	*i2 = iarr[1];
	*x1 = rarr[0];
	*y1 = rarr[1];
	*z1 = rarr[2];
	*x2 = rarr[3];
	*y2 = rarr[4];
	*z2 = rarr[5];
	*rad= rarr[6];
	free_ptr( (void **)&startptr );
	return( TRUE );
  }

  /* read doubles from line */
  for( i = 0; i < nflt; i++ )
  {
	/* read a double from line */
	rarr[i] = Strtod( line_buf, &endptr );
	if( *endptr == '\0' ) break;
	line_buf = endptr + 1;
  } /* for( i = 0; i < nflt; i++ ) */

  /* Return values on normal exit */
  *i1  = iarr[0];
  *i2  = iarr[1];
  *x1  = rarr[0];
  *y1  = rarr[1];
  *z1  = rarr[2];
  *x2  = rarr[3];
  *y2  = rarr[4];
  *z2  = rarr[5];
  *rad = rarr[6];

  free_ptr( (void **)&startptr );
  return( TRUE );
} /* readgm() */

/*-----------------------------------------------------------------------*/

