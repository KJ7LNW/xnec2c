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

  Program NEC(input,tape5=input,output,tape11,tape12,tape13,tape14,
  tape15,tape16,tape20,tape21)

  Numerical Electromagnetics Code (NEC2)  developed at Lawrence
  Livermore lab., Livermore, CA.  (contact G. Burke at 415-422-8414
  for problems with the NEC code. For problems with the vax implem-
  entation, contact J. Breakall at 415-422-8196 or E. Domning at 415
  422-5936)
  file created 4/11/80

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

/* Data need for execution and calculations */
extern calc_data_t calc_data;

/* Impeadance data */
extern impedance_data_t impedance_data;

/* common  /cmb/ */
extern complex long double *cm;

/* common  /crnt/ */
extern crnt_t crnt;

/* common  /data/ */
extern data_t data;

/* common  /fpat/ */
extern fpat_t fpat;

/* common  /gnd/ */
extern gnd_t gnd;

/* common  /vsorc/ */
extern vsorc_t vsorc;

/* common  /matpar/ */
extern matpar_t matpar;

/* common  /netcx/ */
extern netcx_t netcx;

/* common  /save/ */
extern save_t save;

/* common  /smat/ */
extern smat_t smat;

/* Antenna projection parameters */
extern projection_parameters_t structure_proj_params;

/* common  /zload/ */
extern zload_t zload;

/* pointer to input file */
extern FILE *input_fp;

/* Magnitude of seg/patch current/charge */
extern double *cmag, *ct1m, *ct2m;

/*------------------------------------------------------------------------*/

/* Read_Comments()
 *
 * Reads CM comment cards from input file
 */
  int
Read_Comments( void )
{
  char ain[3], line_buf[LINE_LEN];

  /* read a line from input file */
  if( load_line(line_buf, input_fp) == EOF )
	stop( "Error reading input file\n"
		  "Unexpected EOF (End of File)", 1 );

  /* separate card's id mnemonic */
  strncpy( ain, line_buf, 2 );
  ain[2] = '\0';

  /* if its a "cm" or "ce" card start reading comments */
  if( (strcmp(ain, "CM") == 0) ||
	  (strcmp(ain, "CE") == 0) )
  {
	/* Keep reading till a non "CM" card */
	while( strcmp(ain, "CM") == 0 )
	{
	  /* read a line from input file */
	  if( load_line(line_buf, input_fp) == EOF )
		stop( "Error reading input file\n"
			  "Unexpected EOF (End of File)", 1 );

	  /* separate card's id mnemonic */
	  strncpy( ain, line_buf, 2 );
	  ain[2] = '\0';

	} /* while( strcmp(ain, "CM") == 0 ) */

	/* no "ce" card at end of comments */
	if( strcmp(ain, "CE") != 0 )
	{
	  stop( "No CE card at end of Comments", 0 );
	  fseek( input_fp, (long)(-strlen(line_buf)-1), SEEK_CUR );
	  return(0);
	}

  } /* if( strcmp(ain, "CM") == 0 ... */
  else
	rewind( input_fp );

  return(0);

} /* Read_Comments() */

/*-----------------------------------------------------------------------*/

/* Read_Geometry()
 *
 * Reads geometry data from input file
 */
  int
Read_Geometry( void )
{
  int idx;
  size_t mreq;

  /* Moved here from Read_Commands() */
  matpar.imat=0;
  data.n = data.m = 0;
  datagn();

  /* Memory allocation for temporary buffers */
  mreq = data.npm * sizeof(long double);
  mem_realloc( (void *)&save.xtemp,  mreq, "in input.c" );
  mem_realloc( (void *)&save.ytemp,  mreq, "in input.c" );
  mem_realloc( (void *)&save.ztemp,  mreq, "in input.c" );
  mem_realloc( (void *)&save.bitemp, mreq, "in input.c" );
  if( data.n > 0 )
  {
	mreq = data.n * sizeof(long double);
	mem_realloc( (void *)&save.sitemp, mreq, "in input.c" );
  }

  /* Memory allocation for primary interacton matrix. */
  mreq = data.np2m * (data.np+2*data.mp);
  mreq *= sizeof(complex long double);
  mem_realloc( (void *)&cm, mreq, "in input.c" );

  /* Memory allocation for current buffers */
  mreq = data.npm * sizeof( long double);
  mem_realloc( (void *)&crnt.air, mreq, "in input.c" );
  mem_realloc( (void *)&crnt.aii, mreq, "in input.c" );
  mem_realloc( (void *)&crnt.bir, mreq, "in input.c" );
  mem_realloc( (void *)&crnt.bii, mreq, "in input.c" );
  mem_realloc( (void *)&crnt.cir, mreq, "in input.c" );
  mem_realloc( (void *)&crnt.cii, mreq, "in input.c" );
  mreq = data.np3m * sizeof( complex long double);
  mem_realloc( (void *)&crnt.cur, mreq, "in input.c" );

  /* Memory allocation for loading buffers */
  mem_realloc( (void *)&zload.zarray,
	  data.npm * sizeof(complex long double), "in input.c" );

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

  return(0);

} /* Read_Geometry() */

/*------------------------------------------------------------------------*/

/* Read_Commands()
 *
 * Reads commands from input file and stores
 * them for later execution by user command
 */
  int
Read_Commands( void )
{
#define NUM_CMNDS  19
  /* input card mnemonic list */
  char *atst[NUM_CMNDS] =
  {
	"CM", "CP", "EK", "EN", "EX", \
	"FR", "GD", "GN", "KH", "LD", \
	"NE", "NH", "NT", "PQ", "PT", \
	"RP", "SY", "TL", "XQ"
  };

  char ain[3];
  long double tmp1, tmp2, tmp3, tmp4, tmp5, tmp6;
  int
	mpcnt, itmp1, itmp2, itmp3, itmp4,
	ain_num;     /* My addition, ain mnemonic as a number */
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
  calc_data.rkh = 1.0l;
  calc_data.iexk = 0;
  calc_data.iped = 0;
  calc_data.nfrq = 1;
  calc_data.fmhz = save.fmhz = CVEL;
  calc_data.mxfrq = 0.0;
  fpat.ixtyp = 0;
  fpat.nfeh = fpat.nrx = fpat.nry = 0;
  fpat.nry = fpat.nth = fpat.nph = 0;
  fpat.near = -1;
  gnd.ifar = -1;
  gnd.zrati = CPLX_10;
  gnd.ksymp = 1;
  gnd.nradl = gnd.iperf = 0;
  netcx.nonet = netcx.ntsol = netcx.masym = 0;
  netcx.npeq = data.np+2*data.mp;

  /* My additions */
  vsorc.nvqd = vsorc.nsant = zload.nldseg = zload.nload = 0;

  /* Allocate some buffers */
  mreq = data.np2m * sizeof(int);
  mem_realloc( (void *)&save.ip, mreq, "in input.c" );

  /* Memory allocation for symmetry array */
  smat.nop = netcx.neq/netcx.npeq;
  mreq = smat.nop * smat.nop;
  mem_realloc( (void *)&smat.ssx,
	  mreq * sizeof( complex long double), "in input.c" );

  /* main input section, exits at various points */
  /* depending on error conditions or end of job */
  while( TRUE )
  {
	/* Main input section - standard read statement - jumps */
	/* to appropriate section for specific parameter set up */
	readmn(
		ain, &itmp1, &itmp2, &itmp3, &itmp4,
		&tmp1, &tmp2, &tmp3, &tmp4, &tmp5, &tmp6, 1 );
	mpcnt++;

	/* identify command card id mnemonic */
	for( ain_num = 0; ain_num < NUM_CMNDS; ain_num++ )
	  if( strncmp( ain, atst[ain_num], 2) == 0 )
		break;

	/* take action according to card id mnemonic */
	switch( ain_num )
	{
	  case 0: /* "cm" card ignored, comments in data cards as in NEC4 */
		continue;

	  case 1: /* "cp" card ignored, maximum coupling between antennas */
		stop( "CP card is ignored\n"
			"Coupling calculation not implemented", 0 );
		continue; /* continue card input loop */

	  case 2: /* "ek" card,  extended thin wire kernel option */
		if( itmp1 == -1)
		  calc_data.iexk = 0;
		else
		  calc_data.iexk = 1;
		continue; /* continue card input loop */

	  case 3: /* "en" card, end data input, no action */
		break;

	  case 4: /* "ex" card, excitation parameters */
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
			mem_realloc( (void *)&vsorc.ivqd,
				vsorc.nvqd * sizeof(int), "in input.c" );
			mem_realloc( (void *)&vsorc.iqds,
				vsorc.nvqd * sizeof(int), "in input.c" );
			mem_realloc( (void *)&vsorc.vqd,
				vsorc.nvqd * sizeof(complex long double),
				"in input.c" );
			mem_realloc( (void *)&vsorc.vqds,
				vsorc.nvqd * sizeof(complex long double),
				"in input.c" );
			{
			  int indx = vsorc.nvqd-1;

			  vsorc.ivqd[indx]= isegno( itmp2, itmp3);
			  vsorc.vqd[indx]= cmplx( tmp1, tmp2);
			  if( cabsl( vsorc.vqd[indx]) < 1.0e-20l)
				vsorc.vqd[indx] = CPLX_10;

			  calc_data.iped   = itmp4- netcx.masym*10;
			  calc_data.zpnorm = tmp3;
			}

		  } /* if( fpat.ixtyp == 5) */
		  else
		  {
			/* Else, applied E field */
			vsorc.nsant++;
			mem_realloc( (void *)&vsorc.isant,
				vsorc.nsant * sizeof(int), "in input.c" );
			mem_realloc( (void *)&vsorc.vsant,
				vsorc.nsant * sizeof(complex long double),
				"in input.c" );
			{
			  int indx = vsorc.nsant-1;

			  vsorc.isant[indx]= isegno( itmp2, itmp3);
			  vsorc.vsant[indx]= cmplx( tmp1, tmp2);
			  if( cabsl( vsorc.vsant[indx]) < 1.0e-20l)
				vsorc.vsant[indx] = CPLX_10;

			  calc_data.iped= itmp4- netcx.masym*10;
			  calc_data.zpnorm= tmp3;
			  if( (calc_data.iped == 1) && (calc_data.zpnorm > 0.0l) )
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

	  case 5: /* "fr" card, frequency parameters */
		if( !CHILD )
		{
		  calc_data.nfrq = itmp2;
		  if( calc_data.nfrq <= 0)
			calc_data.nfrq = 1;
		}
		else
		  calc_data.nfrq = 1;

		/* Allocate normalization buffer */
		{
		  size_t mreq = calc_data.nfrq * sizeof(double);
		  mem_realloc( (void *)&impedance_data.zreal,
			  mreq, "in input.c" );
		  mem_realloc( (void *)&impedance_data.zimag,
			  mreq, "in input.c" );
		  mem_realloc( (void *)&impedance_data.zmagn,
			  mreq, "in input.c" );
		  mem_realloc( (void *)&impedance_data.zphase,
			  mreq, "in input.c" );
		  mem_realloc( (void *)&save.freq,
			  mreq, "in input.c" );
		  mem_realloc( (void *)&save.fstep,
			  calc_data.nfrq * sizeof(char), "in input.c" );
		}

		if( CHILD ) continue;

		calc_data.ifrq = itmp1;
		calc_data.fmhz = save.fmhz = tmp1;

		/* My addition, max frequency */
		if( itmp1 == 0 )
		  calc_data.mxfrq =
			(double)tmp1 + (double)tmp2 * (double)(itmp2-1);
		else
		  if( itmp1 == 1 )
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
			  (tmp2 - tmp1)/(long double)(calc_data.nfrq-1);
		  calc_data.ifrq  = 0;
		  calc_data.mxfrq = (double)tmp2; /* Max frequency */
		}
		else
		  if( calc_data.ifrq == 3 )
		  {
			calc_data.nfrq++;
			/* Multiplicative frequency stepping */
			if( calc_data.nfrq > 1 )
			  calc_data.delfrq =
				powl( (tmp2-tmp1), 1.0l/(long double)(calc_data.nfrq-1) );
			calc_data.ifrq  = 1;
			calc_data.mxfrq = (double)tmp2; /* Max frequency */
		  }
		  else
			calc_data.delfrq = tmp2;

		if( calc_data.iped == 1)
		  calc_data.zpnorm = 0.0l;
		continue; /* continue card input loop */

	  case 6: /* "gd" card, ground representation */
		fpat.epsr2 = tmp1;
		fpat.sig2  = tmp2;
		fpat.clt   = tmp3;
		fpat.cht   = tmp4;
		continue; /* continue card input loop */

	  case 7: /* "gn" card, ground parameters under the antenna */
		gnd.iperf = itmp1;
		gnd.nradl = itmp2;
		gnd.ksymp = 2;
		save.epsr = tmp1;
		save.sig  = tmp2;

		if( gnd.nradl != 0)
		{
		  if( gnd.iperf == 2)
		  {
			fprintf( stderr,
				"xnec2c: radial wire g.s. approximation may\n"
				"not be used with Sommerfeld ground option\n" );
			stop( "Radial wire g.s. approximation may not\n"
				"be used with Sommerfeld ground option", 1 );
		  }

		  save.scrwlt = tmp3;
		  save.scrwrt = tmp4;
		  continue; /* continue card input loop */
		}

		fpat.epsr2 = tmp3;
		fpat.sig2  = tmp4;
		fpat.clt   = tmp5;
		fpat.cht   = tmp6;
		continue; /* continue card input loop */

	  case 8: /* "kh" card, matrix integration limit */
		calc_data.rkh = tmp1;
		continue; /* continue card input loop */

	  case 9: /* "ld" card, loading parameters */
		{
		  int idx, nseg;
		  size_t mreq;

		  /* Reallocate loading buffers */
		  zload.nload++;
		  mreq = zload.nload * sizeof(int);
		  mem_realloc( (void *)&calc_data.ldtyp,
			  mreq, "in input.c" );
		  mem_realloc( (void *)&calc_data.ldtag,
			  mreq, "in input.c" );
		  mem_realloc( (void *)&calc_data.ldtagf,
			  mreq, "in input.c" );
		  mem_realloc( (void *)&calc_data.ldtagt,
			  mreq, "in input.c" );

		  mreq = zload.nload * sizeof(long double);
		  mem_realloc( (void *)&calc_data.zlr,
			  mreq, "in input.c" );
		  mem_realloc( (void *)&calc_data.zli,
			  mreq, "in input.c" );
		  mem_realloc( (void *)&calc_data.zlc,
			  mreq, "in input.c" );

		  idx = zload.nload-1;
		  calc_data.ldtyp[idx]= itmp1;
		  calc_data.ldtag[idx]= itmp2;
		  if( itmp4 == 0) itmp4= itmp3;
		  calc_data.ldtagf[idx]= itmp3;
		  calc_data.ldtagt[idx]= itmp4;

		  if( itmp4 < itmp3 )
		  {
			fprintf( stderr,
				"xnec2c: data fault on loading card no %d\n"
				"itag step1 %d is greater than itag step2 %d\n",
				zload.nload, itmp3, itmp4 );
			stop( "Data fault on loading card\n"
				"itag step1 is greater than itag step2", 1 );
		  }

		  calc_data.zlr[idx]= tmp1;
		  calc_data.zli[idx]= tmp2;
		  calc_data.zlc[idx]= tmp3;

		  /*** My addition, for drawing structure ***/
		  /* Work out absolute seg number of loaded segments (!) */
		  if( itmp1 == -1 ) /* Null all loads */
			zload.nldseg = 0;
		  else
			if( itmp2 == 0 ) /* No tag num */
			{
			  if( itmp3 == 0 ) /* All segs loaded */
			  {
				nseg = data.n;
				mem_realloc( (void *)&zload.ldsegn,
					(nseg+zload.nldseg)*sizeof(int), "in input.c" );
				mem_realloc( (void *)&zload.ldtype,
					(nseg+zload.nldseg)*sizeof(int), "in input.c" );
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
				mem_realloc( (void *)&zload.ldsegn,
					(zload.nldseg+nseg)*sizeof(int), "in input.c" );
				mem_realloc( (void *)&zload.ldtype,
					(zload.nldseg+nseg)*sizeof(int), "in input.c" );
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
					mem_realloc( (void *)&zload.ldsegn,
						(zload.nldseg+1)*sizeof(int), "in input.c" );
					mem_realloc( (void *)&zload.ldtype,
						(zload.nldseg+1)*sizeof(int), "in input.c" );
					zload.ldtype[zload.nldseg]   = itmp1;
					zload.ldsegn[zload.nldseg++] = idx+1;
				  }
			  }
			  else /* A range of segs of tag loaded */
			  {
				nseg = itmp4 - itmp3 + 1;
				if( nseg <= 0 ) /* Just one seg of tag (=itmp3) */
				  nseg = 1;
				mem_realloc( (void *)&zload.ldsegn,
					(zload.nldseg+nseg)*sizeof(int), "in input.c" );
				mem_realloc( (void *)&zload.ldtype,
					(zload.nldseg+nseg)*sizeof(int), "in input.c" );
				for( idx = 0; idx < nseg; idx++ )
				{
				  zload.ldtype[zload.nldseg]   = itmp1;
				  zload.ldsegn[zload.nldseg++] =
					isegno( itmp2, itmp3+idx );
				}
			  }
			}
		} /* case 8: */
		continue; /* continue card input loop */

	  case 10: case 11:  /* "ne"/"nh" cards, near field calculation parameters */
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

	  case 12: case 17: /* "nt" & "tl" cards, network parameters */
		{
		  int idx;
		  size_t mreq;

		  /* Re-allocate network buffers */
		  netcx.nonet++;
		  mreq = netcx.nonet * sizeof(int);
		  mem_realloc( (void *)&netcx.ntyp,  mreq, "in input.c" );
		  mem_realloc( (void *)&netcx.iseg1, mreq, "in input.c" );
		  mem_realloc( (void *)&netcx.iseg2, mreq, "in input.c" );

		  mreq = netcx.nonet * sizeof(long double);
		  mem_realloc( (void *)&netcx.x11r, mreq, "in input.c" );
		  mem_realloc( (void *)&netcx.x11i, mreq, "in input.c" );
		  mem_realloc( (void *)&netcx.x12r, mreq, "in input.c" );
		  mem_realloc( (void *)&netcx.x12i, mreq, "in input.c" );
		  mem_realloc( (void *)&netcx.x22r, mreq, "in input.c" );
		  mem_realloc( (void *)&netcx.x22i, mreq, "in input.c" );

		  idx = netcx.nonet-1;
		  if( ain_num == 12 )
			netcx.ntyp[idx] = 1;
		  else
		  {
			netcx.ntyp[idx] = 2;
			if( tmp1 == 0.0l )
			  stop( "Transmission Line impedance = 0\n"
				  "is not valid. Please correct", 1 );
		  }

		  netcx.iseg1[idx] = isegno( itmp1, itmp2);
		  netcx.iseg2[idx] = isegno( itmp3, itmp4);
		  netcx.x11r[idx]  = tmp1;
		  netcx.x11i[idx]  = tmp2;
		  netcx.x12r[idx]  = tmp3;
		  netcx.x12i[idx]  = tmp4;
		  netcx.x22r[idx]  = tmp5;
		  netcx.x22i[idx]  = tmp6;

		  if( (netcx.ntyp[idx] == 1) || (tmp1 > 0.0l) )
			continue; /* continue card input loop */

		  netcx.ntyp[idx] = 3;
		  netcx.x11r[idx] = -tmp1;
		  continue; /* continue card input loop */

		} /* case 12: case 17: */

	  case 13: case 14: /* "pq" and "pt" cards ignored, no printing */
		stop( "PQ and PT cards are ignored\n"
			"Printing to file not implemented", 0);
		continue; /* continue card input loop */

	  case 15: /* "rp" card, standard observation angle parameters */
		if( itmp1 == 1 )
		  stop( "Surface wave option (I1=1)\n"
			  "of RP command not implemented", 1 );

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
		  stop( "Gain averaging (XNDA ***1 or ***2)\n"
			  "of RP command not supported", 1 );
		if( fpat.iax != 0) fpat.iax = 1;
		if( fpat.ipd != 0) fpat.ipd = 1;
		if( (fpat.nth < 2) || (fpat.nph < 1) || (gnd.ifar == 1) )
		{
		  fpat.iavp = 0;
		  ClearFlag( ENABLE_RDPAT );
		}
		else
		  SetFlag( ENABLE_RDPAT );

		fpat.thets = tmp1;
		fpat.phis  = tmp2;
		fpat.dth   = tmp3;
		fpat.dph   = tmp4;
		fpat.rfld  = tmp5;
		fpat.gnor  = tmp6;

		/* Because of the interactive GUI, program
		 * execution is not triggered by any card */
		continue; /* continue card input loop */

	  case 16: /* "sy" TODO Compatibility with 4nec2 */
		continue;

	  case 18: /* "xq" execute card */
		/* Because of the interactive GUI, program
		 * execution is not triggered by any card.
		 * XQ now is the same as EN because of above */
		if( itmp1 == 0 )
		  break; /* No rad pat, stop reading commands */

		/* Read radiation pattern parameters */
		gnd.ifar  = 0;
		fpat.rfld = 0.0l;
		fpat.ipd  = 0;
		fpat.iavp = 0;
		fpat.inor = 0;
		fpat.iax  = 0;
		fpat.nth  = 91;
		fpat.nph  = 1;
		fpat.thets= 0.0l;
		fpat.phis = 0.0l;
		fpat.dth  = 1.0l;
		fpat.dph  = 0.0l;

		if( itmp1 == 2)
		  fpat.phis = 90.0l;
		else
		  if( itmp1 == 3)
		  {
			fpat.nph = 2;
			fpat.dph = 90.0l;
		  }

		SetFlag( ENABLE_RDPAT );

		/* Because of the interactive GUI, program
		 * execution is not triggered by any card.
		 * XQ now is the same as EN because of above */
		break;

	  default:
		fprintf( stderr,
			"xnec2c: faulty data card label after geometry section\n" );
		stop( "Faulty data card label\n"
			"after geometry section", 1 );
	} /* switch( ain_num ) */

	/* Allocate radiation pattern buffers */
	if( isFlagSet(ENABLE_RDPAT) )
	  Alloc_Rdpattern_Buffers( calc_data.nfrq+1, fpat.nth, fpat.nph );

  return(0);

  } /* while( TRUE ) */

} /* Read_Commands() */

/*-----------------------------------------------------------------------*/

  int
readmn( char *gm, int *i1, int *i2, int *i3, int *i4,
	long double *f1, long double *f2, long double *f3,
	long double *f4, long double *f5, long double *f6, int err )
{
  char line_buf[LINE_LEN];
  int nlin, i, line_idx;
  int nint = 4, nflt = 6;
  int iarr[4] = { 0, 0, 0, 0 };
  long double rarr[6] = { 0.0l, 0.0l, 0.0l, 0.0l, 0.0l, 0.0l };

  /* read a line from input file */
  load_line( line_buf, input_fp );

  /* get line length */
  nlin= strlen( line_buf );

  /* abort if card's mnemonic too short or missing */
  if( nlin < 2 )
  {
	gm[0] = '\0';
	fprintf( stderr,
		"xnec2c: command data card error\n"
		"card's mnemonic code too short or missing\n" );
	stop( "Command data card error\n"
		"Mnemonic code too short or missing", err );
  }

  /* extract card's mnemonic code */
  strncpy( gm, line_buf, 2 );
  gm[2] = '\0';

  /* Return if only mnemonic on card */
  if( nlin == 2 )
  {
	*i1 = *i2 = *i3 = *i4 = 0;
	*f1 = *f2 = *f3 = *f4 = *f5 = *f6 = 0.0l;
	return(0);
  }

  /* Compatibility with NEC4,
   * comments between data cards */
  if( strncmp(gm, "CM", 2) == 0 )
	return(0);

  /* read integers from line */
  line_idx = 1;
  for( i = 0; i < nint; i++ )
  {
	/* Find first numerical character */
	while(
		((line_buf[++line_idx] <  '0')  ||
		 (line_buf[  line_idx] >  '9')) &&
		(line_buf[  line_idx] != '+')   &&
		(line_buf[  line_idx] != '-') )
	  if( (line_buf[line_idx] == '\0') )
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
		return(0);
	  }

	/* read an integer from line */
	iarr[i] = atoi( &line_buf[line_idx] );

	/* traverse numerical field to next ' ' or ',' or '\0' */
	line_idx--;
	while(
		(line_buf[++line_idx] != ' ') &&
		(line_buf[  line_idx] != '	') &&
		(line_buf[  line_idx] != ',') &&
		(line_buf[  line_idx] != '\0') )
	{
	  /* test for non-numerical characters */
	  if( ((line_buf[line_idx] <  '0')  ||
		   (line_buf[line_idx] >  '9')) &&
		   (line_buf[line_idx] != '+')  &&
		   (line_buf[line_idx] != '-') )
	  {
		gm[0] = '\0';
		fprintf( stderr,
			"xnec2c: command data card \"%s\" error\n"
			"non-numerical char '%c' in integer field at char %d\n",
			gm, line_buf[line_idx], (line_idx+1) );
		stop( "Command data card error\n"
			"Non-numerical character in integer field", err );
	  }

	} /* while( (line_buff[++line_idx] ... */

	/* Return on end of line */
	if( line_buf[line_idx] == '\0' )
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
	  return(0);
	}

  } /* for( i = 0; i < nint; i++ ) */

  /* read long doubles from line */
  for( i = 0; i < nflt; i++ )
  {
	/* Find first numerical character */
	while( ((line_buf[++line_idx] <  '0')  ||
		  (line_buf[  line_idx] >  '9')) &&
		  (line_buf[  line_idx] != '+')  &&
		  (line_buf[  line_idx] != '-')  &&
		  (line_buf[  line_idx] != '.') )
	  if( (line_buf[line_idx] == '\0') )
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
		return(0);
	  }

	/* read a long double from line */
	rarr[i] = atof( &line_buf[line_idx] );

	/* traverse numerical field to next ' ' or ',' or '\0'*/
	line_idx--;
	while(
		(line_buf[++line_idx] != ' ') &&
		(line_buf[  line_idx] != '	') &&
		(line_buf[  line_idx] != ',') &&
		(line_buf[  line_idx] != '\0') )
	{
	  /* test for non-numerical characters */
	  if( ((line_buf[line_idx] <  '0')  ||
		   (line_buf[line_idx] >  '9')) &&
		   (line_buf[line_idx] != '.')  &&
		   (line_buf[line_idx] != '+')  &&
		   (line_buf[line_idx] != '-')  &&
		   (line_buf[line_idx] != 'E')  &&
		   (line_buf[line_idx] != 'e') )
	  {
		gm[0] = '\0';
		fprintf( stderr,
			"xnec2c: command data card \"%s\" error\n"
			"non-numerical char '%c' in float field at char %d\n",
			gm, line_buf[line_idx], (line_idx+1) );
		stop( "Command data card error\n"
			"Non-numerical character in float field", err );
	  }

	} /* while( (line_buff[++line_idx] ... */

	/* Return on end of line */
	if( line_buf[line_idx] == '\0' )
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
	  return(0);
	}

  } /* for( i = 0; i < nflt; i++ ) */

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

  return(0);
}

/*-----------------------------------------------------------------------*/

  int
readgm( char *gm, int *i1, int *i2, long double *x1,
	long double *y1, long double *z1, long double *x2,
	long double *y2, long double *z2, long double *rad, int err )
{
  char line_buf[LINE_LEN];
  int nlin, i, line_idx;
  int nint = 2, nflt = 7;
  int iarr[2] = { 0, 0 };
  long double rarr[7] = { 0.0l, 0.0l, 0.0l, 0.0l, 0.0l, 0.0l, 0.0l };


  /* read a line from input file */
  load_line( line_buf, input_fp );

  /* get line length */
  nlin= strlen( line_buf );

  /* abort if card's mnemonic too short or missing */
  if( nlin < 2 )
  {
	gm[0] = '\0';
	fprintf( stderr,
		"xnec2c: geometry data card error\n"
		"card's mnemonic code too short or missing\n" );
	stop( "Geometry data card error\n"
		"Card's mnemonic code too short or missing", err );
  }

  /* extract card's mnemonic code */
  strncpy( gm, line_buf, 2 );
  gm[2] = '\0';

  /* Return if only mnemonic on card */
  if( nlin == 2 )
  {
	*i1 = *i2 = 0;
	*x1 = *y1 = *z1 = *x2 = *y2 = *z2 = *rad = 0.0l;
	return(0);
  }

  /* read integers from line */
  line_idx = 1;
  for( i = 0; i < nint; i++ )
  {
	/* Find first numerical character */
	while(
		((line_buf[++line_idx] <  '0')  ||
		 (line_buf[  line_idx] >  '9')) &&
		 (line_buf[  line_idx] != '+')  &&
		 (line_buf[  line_idx] != '-') )
	  if( (line_buf[line_idx] == '\0') )
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
		return(0);
	  }

	/* read an integer from line */
	iarr[i] = atoi( &line_buf[line_idx] );

	/* traverse numerical field to next ' ' or ',' or '\0' */
	line_idx--;
	while(
		(line_buf[++line_idx] != ' ') &&
		(line_buf[  line_idx] != '	') &&
		(line_buf[  line_idx] != ',') &&
		(line_buf[  line_idx] != '\0') )
	{
	  /* test for non-numerical characters */
	  if(
		  ((line_buf[line_idx] <  '0')  ||
		   (line_buf[line_idx] >  '9')) &&
		   (line_buf[line_idx] != '+')  &&
		   (line_buf[line_idx] != '-') )
	  {
		gm[0] = '\0';
		fprintf( stderr,
			"xnec2c: geometry data card \"%s\" error\n"
			"non-numerical char '%c' in integer field at char %d\n",
			gm, line_buf[line_idx], (line_idx+1)  );
		stop( "Geometry data card error\n"
			"Non-numerical character in integer field", err );
	  }

	} /* while( (line_buff[++line_idx] ... */

	/* Return on end of line */
	if( line_buf[line_idx] == '\0' )
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
	  return(0);
	}

  } /* for( i = 0; i < nint; i++ ) */

  /* read long doubles from line */
  for( i = 0; i < nflt; i++ )
  {
	/* Find first numerical character */
	while(
		((line_buf[++line_idx] <  '0') ||
		 (line_buf[  line_idx] >  '9')) &&
		(line_buf[  line_idx] != '+')  &&
		(line_buf[  line_idx] != '-')  &&
		(line_buf[  line_idx] != '.') )
	  if( (line_buf[line_idx] == '\0') )
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
		return(0);
	  }

	/* read a long double from line */
	rarr[i] = atof( &line_buf[line_idx] );

	/* traverse numerical field to next ' ' or ',' or '\0' */
	line_idx--;
	while(
		(line_buf[++line_idx] != ' ') &&
		(line_buf[  line_idx] != '	') &&
		(line_buf[  line_idx] != ',') &&
		(line_buf[  line_idx] != '\0') )
	{
	  /* test for non-numerical characters */
	  if(
		  ((line_buf[line_idx] <  '0')  ||
		   (line_buf[line_idx] >  '9')) &&
		   (line_buf[line_idx] != '.')  &&
		   (line_buf[line_idx] != '+')  &&
		   (line_buf[line_idx] != '-')  &&
		   (line_buf[line_idx] != 'E')  &&
		   (line_buf[line_idx] != 'e') )
	  {
		gm[0] = '\0';
		fprintf( stderr,
			"xnec2c: geometry data card \"%s\" error\n"
			"Non-numerical char '%c' in float field at char %d\n",
			gm, line_buf[line_idx], (line_idx+1) );
		stop( "Geometry data card error\n"
			"Non-numerical character in float field", err );
	  }

	} /* while( (line_buff[++line_idx] ... */

	/* Return on end of line */
	if( line_buf[line_idx] == '\0' )
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
	  return(0);
	}

  } /* for( i = 0; i < nflt; i++ ) */

  *i1  = iarr[0];
  *i2  = iarr[1];
  *x1  = rarr[0];
  *y1  = rarr[1];
  *z1  = rarr[2];
  *x2  = rarr[3];
  *y2  = rarr[4];
  *z2  = rarr[5];
  *rad = rarr[6];

  return(0);
}

/*-----------------------------------------------------------------------*/

/* datagn is the main routine for input of geometry data. */
  int
datagn( void )
{
  char gm[3];

  /* input card mnemonic list */
#define GM_NUM  12
  char *atst[GM_NUM] =
  {
	"GW", "GX", "GR", "GS", "GE", "GM", \
    "SP", "SM", "GA", "SC", "GH", "GF"
  };

  int nwire, isct, itg, iy, iz, mreq;
  int ix, i, ns, gm_num; /* geometry card id as a number */
  long double rad, xs1, xs2, ys1, ys2, zs1, zs2;
  long double x3=0, y3=0, z3=0, x4=0, y4=0, z4=0;
  long double xw1, xw2, yw1, yw2, zw1, zw2;
  long double dummy;

  data.ipsym=0;
  nwire=0;
  data.n=0;
  data.np=0;
  data.m=0;
  data.mp=0;
  isct=0;
  structure_proj_params.r_max = 0.0l;

  /* read geometry data card and branch to */
  /* section for operation requested */
  do
  {
	readgm( gm, &itg, &ns, &xw1, &yw1, &zw1, &xw2, &yw2, &zw2, &rad, 1 );

	/* identify card id mnemonic */
	for( gm_num = 0; gm_num < GM_NUM; gm_num++ )
	  if( strncmp( gm, atst[gm_num], 2) == 0 )
		break;

	if( gm_num != 10 ) isct=0;

	switch( gm_num )
	{
	  case 0: /* "gw" card, generate segment data for straight wire. */
		Tag_Seg_Error( itg, ns );
		nwire++;

		if( rad != 0.0)
		{
		  xs1=1.0l;
		  ys1=1.0l;
		}
		else
		{
		  readgm( gm, &ix, &iy, &xs1, &ys1, &zs1,
			  &dummy, &dummy, &dummy, &dummy, 1 );

		  if( strcmp(gm, "GC" ) != 0 )
		  {
			fprintf( stderr,
				"xnec2c: geometry data card error "
				"no GC card for tapered wire\n" );
			stop( "Geometry data error\n"
				"No GC card for tapered wire", 1 );
		  }

		  if( (ys1 == 0) || (zs1 == 0) )
		  {
			fprintf( stderr, "xnec2c: geometry GC data card error\n" );
			stop( "Geometry GC data card error", 1 );
		  }

		  rad= ys1;
		  ys1= powl( (zs1/ys1), (1.0l/(ns-1.0l)) );
		}

		wire( xw1, yw1, zw1, xw2, yw2, zw2, rad, xs1, ys1, ns, itg);
		continue;

		/* reflect structure along x,y, or z */
		/* axes or rotate to form cylinder.  */
	  case 1: /* "gx" card */
		if( (ns < 0) || (itg < 0) )
		{
		  fprintf( stderr, "xnec2c: geometry GX data card error\n" );
		  stop( "Geometry GX data card error", 1 );
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

		reflc( ix, iy, iz, itg, ns);
		continue;

	  case 2: /* "gr" card */
		if( (ns < 0) || (itg < 0) )
		{
		  fprintf( stderr, "xnec2c: geometry GR data card error\n" );
		  stop( "Geometry GR data card error", 1 );
		}

		ix=-1;
		iz = 0;
		reflc( ix, iy, iz, itg, ns);
		continue;

	  case 3: /* "gs" card, scale structure dimensions by factor xw1 */

		if( (itg > 0) && (ns > 0) && (ns >= itg) )
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
		else
		  for( i = 0; i < data.n; i++ )
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

	  case 4: /* "ge" card, terminate structure geometry input. */
		/* My addition, for drawing */
		if( ((data.n > 0) || (data.m > 0)) && !CHILD )
		  Init_Struct_Drawing();
		else if( (data.n == 0) && (data.m == 0) )
		  stop( "No geometry data cards", 1 );

		conect(itg);
		if( data.n != 0)
		{
		  /* Allocate wire buffers */
		  mreq = data.n * sizeof(long double);
		  mem_realloc( (void *)&data.si,   mreq, "in geometry.c" );
		  mem_realloc( (void *)&data.sab,  mreq, "in geometry.c" );
		  mem_realloc( (void *)&data.cab,  mreq, "in geometry.c" );
		  mem_realloc( (void *)&data.salp, mreq, "in geometry.c" );
		  mem_realloc( (void *)&data.x, mreq, "in geometry.c" );
		  mem_realloc( (void *)&data.y, mreq, "in geometry.c" );
		  mem_realloc( (void *)&data.z, mreq, "in geometry.c" );

		  for( i = 0; i < data.n; i++ )
		  {
			xw1= data.x2[i]- data.x1[i];
			yw1= data.y2[i]- data.y1[i];
			zw1= data.z2[i]- data.z1[i];
			data.x[i]=( data.x1[i]+ data.x2[i])/2.0l;
			data.y[i]=( data.y1[i]+ data.y2[i])/2.0l;
			data.z[i]=( data.z1[i]+ data.z2[i])/2.0l;
			xw2= xw1* xw1+ yw1* yw1+ zw1* zw1;
			yw2= sqrtl( xw2);
			yw2=( xw2/ yw2+ yw2)*.5l;
			data.si[i]= yw2;
			data.cab[i]= xw1/ yw2;
			data.sab[i]= yw1/ yw2;
			xw2= zw1/ yw2;

			if( xw2 > 1.0l)
			  xw2=1.0l;
			if( xw2 < -1.0l)
			  xw2=-1.0l;

			data.salp[i]= xw2;
			xw2= asinl( xw2)* TD;
			yw2= atan2l( yw1, xw1)* TD;

			if( (data.si[i] <= 1.0e-20l) || (data.bi[i] <= 0.0l) )
			{
			  fprintf( stderr, "xnec2c: segment data error\n" );
			  stop( "Segment data error", 1 );
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

		return(0);

		/* "gm" card, move structure or reproduce */
		/* original structure in new positions.   */
	  case 5:
		{
		  int tgf = (int)(rad + 0.5l);
		  if( (tgf < 0) || (ns < 0) || (rad < 0.0l) )
		  {
			fprintf( stderr, "xnec2c: move GM data card error\n" );
			stop( "Move GM data card error", 1 );
		  }
		  xw1= xw1* TA;
		  yw1= yw1* TA;
		  zw1= zw1* TA;
		  move( xw1, yw1, zw1, xw2, yw2, zw2, (int)(rad+.5l), ns, itg);
		}
		continue;

	  case 6: /* "sp" card, generate single new patch */
		ns++;

		if( itg != 0)
		{
		  fprintf( stderr, "xnec2c: patch data card error\n" );
		  stop( "Patch data card error", 1 );
		}

		if( (ns == 2) || (ns == 4) )
		  isct=1;

		if( ns > 1)
		{
		  readgm( gm, &ix, &iy, &x3, &y3,
			  &z3, &x4, &y4, &z4, &dummy, 1 );

		  if( (ns == 2) || (itg > 0) )
		  {
			x4= xw1+ x3- xw2;
			y4= yw1+ y3- yw2;
			z4= zw1+ z3- zw2;
		  }

		  if( strcmp(gm, "SC") != 0 )
		  {
			fprintf( stderr, "xnec2c: patch data error\n" );
			stop( "Patch data error", 1 );
		  }

		} /* if( ns > 1) */
		else
		{
		  xw2= xw2* TA;
		  yw2= yw2* TA;
		}

		patch( itg, ns, xw1, yw1, zw1, xw2,
			yw2, zw2, x3, y3, z3, x4, y4, z4);
		continue;

	  case 7: /* "sm" card, generate multiple-patch surface */
		if( (itg < 1) || (ns < 1) )
		{
		  fprintf( stderr, "xnec2c: patch card data error\n" );
		  stop( "Patch data card error", 1 );
		}

		readgm( gm, &ix, &iy, &x3, &y3,
			&z3, &x4, &y4, &z4, &dummy, 1 );

		if( (ns == 2) || (itg > 0) )
		{
		  x4= xw1+ x3- xw2;
		  y4= yw1+ y3- yw2;
		  z4= zw1+ z3- zw2;
		}

		if( strcmp(gm, "SC" ) != 0 )
		{
		  fprintf( stderr, "xnec2c: patch card data error\n" );
		  stop( "Patch data card error", 1 );
		}

		patch( itg, ns, xw1, yw1, zw1, xw2,
			yw2, zw2, x3, y3, z3, x4, y4, z4);
		continue;

	  case 8: /* "ga" card, generate segment data for wire arc */
		Tag_Seg_Error( itg, ns );
		nwire++;
		arc( itg, ns, xw1, yw1, zw1, xw2);
		continue;

	  case 9: /* "sc" card */
		if( isct == 0)
		{
		  fprintf( stderr, "xnec2c: patch data card error\n" );
		  stop( "Patch data card error", 1 );
		}

		ns++;

		if( (itg != 0) || ((ns != 2) && (ns != 4)) )
		{
		  fprintf( stderr, "xnec2c: patch data card error\n" );
		  stop( "Patch data card error", 1 );
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

		patch( itg, ns, xw1, yw1, zw1, xw2,
			yw2, zw2, x3, y3, z3, x4, y4, z4);
		continue;

	  case 10: /* "gh" card, generate helix */
		Tag_Seg_Error( itg, ns );
		nwire++;
		helix( xw1, yw1, zw1, xw2, yw2, zw2, rad, ns, itg);
		continue;

	  case 11: /* "gf" card, not supported */
		stop( "\"GF\" card (NGF solution)\n"
			"is not supported", 1 );

	  default: /* error message */
		fprintf( stderr, "xnec2c: geometry data card error\n" );
		fprintf( stderr,
			"%2s %3d %5d %10.5LF %10.5LF %10.5LF"
			" %10.5LF %10.5LF %10.5LF %10.5LF\n",
			gm, itg, ns, xw1, yw1, zw1, xw2, yw2, zw2, rad );

		stop( "Geometry data card error", 1 );

	} /* switch( gm_num ) */

  } /* do */
  while( TRUE );

} /* datagn() */

/*-----------------------------------------------------------------------*/

/* Tag_Seg_Error()
 *
 * Checks tag and segments number are valid (>1)
 */
  gboolean
Tag_Seg_Error( int tag, int segs )
{
  gboolean retv = FALSE;

  if( tag <= 0 )
  {
	fprintf( stderr,
		"xnec2c: geometry data card error\n"
		"tag number is less than 1\n" );
	stop( "Geometry data error\n"
		"Tag number is less than 1", 1 );
	retv = TRUE;
  }

  if( segs <= 0 )
  {
	fprintf( stderr,
		"xnec2c: geometry data card error\n"
		"number of segments is less than 1\n" );
	stop( "Geometry data error\n"
		"Number of segments is less than 1", 1 );
	retv = TRUE;
  }

  return( retv );
}

