#include "xnec2c.h"
#include "shared.h"

/* prnt sets up the print formats for impedance loading */
static void prnt(FILE *output_fp, int in1, int in2, int in3, double fl1, double fl2,
		   double fl3, double fl4, double fl5,
		   double fl6, char *ia, int ichar )
{
  /* record to be output and buffer used to make it */
  char record[101+ichar*4], buff[16];
  int in[3], i1, i;
  double fl[6];

  in[0]= in1;
  in[1]= in2;
  in[2]= in3;
  fl[0]= fl1;
  fl[1]= fl2;
  fl[2]= fl3;
  fl[3]= fl4;
  fl[4]= fl5;
  fl[5]= fl6;

  /* integer format */
  i1=0;
  strcpy( record, "\n " );

  if( (in1 == 0) && (in2 == 0) && (in3 == 0) )
  {
	strcat( record, " ALL" );
	i1=1;
  }

  for( i = i1; i < 3; i++ )
  {
	if( in[i] == 0)
	  strcat( record, "     " );
	else
	{
	  snprintf( buff, 6, "%5d", in[i] );
	  strcat( record, buff );
	}
  }

  /* floating point format */
  for( i = 0; i < 6; i++ )
  {
	if( fabs( fl[i]) >= 1.0e-20 )
	{
	  snprintf( buff, 15, " %11.4E", fl[i] );
	  strcat( record, buff );
	}
	else
	  strcat( record, "            " );
  }

  strcat( record, "   " );
  strcat( record, ia );
  fprintf( output_fp, "%s", record );

  return;
}

static void write_heading(FILE* output_fp)
{
	fprintf(output_fp, "\n\n\n"
		"                               __________________________________________\n"
		"                              |                                          |\n"
		"                              |  NUMERICAL ELECTROMAGNETICS CODE (nec2c) |\n"
		"                              |   Translated to 'C' in Double Precision  |\n"
		"                              |__________________________________________|\n");
}

static void write_comments(FILE *output_fp)
{
    int idx;

	fprintf(output_fp, "\n\n\n" "                               ---------------- COMMENTS ----------------\n");

	for (idx = 0; calc_data.comments[idx] != NULL; idx++)
		fprintf(output_fp, "                              %s\n", calc_data.comments[idx]);

}

static void write_structure_spec(FILE *output_fp)
{
	fprintf(output_fp, "\n\n\n"
		"                               -------- STRUCTURE SPECIFICATION --------\n"
		"                                     COORDINATES MUST BE INPUT IN\n"
		"                                     METERS OR BE SCALED TO METERS\n"
		"                                     BEFORE STRUCTURE INPUT IS ENDED\n");

	fprintf(output_fp, "\n"
		"  WIRE                                           "
		"                                      SEG FIRST  LAST  TAG\n"
		"   No:        X1         Y1         Z1         X2      "
		"   Y2         Z2       RADIUS   No:   SEG   SEG  No:");

	// This output is per-card at NEC file read time,
	// so not implementing per-card annotations for these sections:
    fprintf(output_fp, "\n   (NOT IMPLEMENTED)");
	// fprintf(output_fp, "\n\n\n");
	// -------- STRUCTURE SPECIFICATION --------
	// 
	fprintf(output_fp, "\n\n"
		"     TOTAL SEGMENTS USED: %d   SEGMENTS IN A"
		" SYMMETRIC CELL: %d   SYMMETRY FLAG: %d", data.n, data.np, data.ipsym);

	if (data.m > 0)
		fprintf(output_fp, "\n"
			"       TOTAL PATCHES USED: %d   PATCHES" " IN A SYMMETRIC CELL: %d", data.m, data.mp);

}

static void write_seg_data(FILE *output_fp, FILE *plot_fp)
{
    int i;

	fprintf(output_fp, "\n\n\n"
		"                               ---------- SEGMENTATION DATA ----------\n"
		"                                        COORDINATES IN METERS\n"
		"                           " " I+ AND I- INDICATE THE SEGMENTS BEFORE AND AFTER I\n");

	fprintf(output_fp, "\n"
		"   SEG    COORDINATES OF SEGM CENTER     SEGM    ORIENTATION"
		" ANGLES    WIRE    CONNECTION DATA   TAG\n"
		"   No:       X         Y         Z      LENGTH     ALPHA     "
		" BETA    RADIUS    I-     I    I+   No:");

	for (i = 0; i < data.n; i++)
	{
		double xw1 = data.x2[i] - data.x1[i];
		double yw1 = data.y2[i] - data.y1[i];
		double zw1 = data.z2[i] - data.z1[i];

		data.x[i] = (data.x1[i] + data.x2[i]) / 2.;
		data.y[i] = (data.y1[i] + data.y2[i]) / 2.;
		data.z[i] = (data.z1[i] + data.z2[i]) / 2.;
		double xw2 = xw1 * xw1 + yw1 * yw1 + zw1 * zw1;
		double yw2 = sqrt(xw2);

		yw2 = (xw2 / yw2 + yw2) * .5;
		data.si[i] = yw2;
		data.cab[i] = xw1 / yw2;
		data.sab[i] = yw1 / yw2;
		xw2 = zw1 / yw2;

		if (xw2 > 1.)
			xw2 = 1.;
		if (xw2 < -1.)
			xw2 = -1.;

		data.salp[i] = xw2;
		xw2 = asin(xw2) * TODEG;
		yw2 = atan2(yw1, xw1) * TODEG;

		fprintf(output_fp, "\n"	
			" %5d %9.4f %9.4f %9.4f %9.4f %9.4f %9.4f %9.4f %5d %5d %5d %5d",
			i + 1, data.x[i], data.y[i], data.z[i], data.si[i], xw2, yw2,
			data.bi[i], data.icon1[i], i + 1, data.icon2[i], data.itag[i]);

		if (plot_fp && plot.iplp1 == 1)
			fprintf(plot_fp, "%12.4E %12.4E %12.4E %12.4E %12.4E %12.4E %12.4E %5d %5d %5d\n",
				data.x[i], data.y[i], data.z[i], data.si[i], xw2, yw2,
				data.bi[i], data.icon1[i], i + 1, data.icon2[i]);

	}			/* for( i = 0; i < data.n; i++ ) */

}

static void write_surface_patch_data(FILE *output_fp)
{
    int i;

	if (data.m != 0)
	{
		fprintf(output_fp, "\n\n\n"
			"                                    --------- SURFACE PATCH DATA ---------\n"
			"                                             COORDINATES IN METERS\n\n"
			" PATCH      COORD. OF PATCH CENTER           UNIT NORMAL VECTOR      "
			" PATCH           COMPONENTS OF UNIT TANGENT VECTORS\n"
			"  No:       X          Y          Z          X        Y        Z      "
			" AREA         X1       Y1       Z1        X2       Y2      Z2");

		for (i = 0; i < data.m; i++)
		{
			double xw1 = (data.t1y[i] * data.t2z[i] - data.t1z[i] * data.t2y[i]) * data.psalp[i];
			double yw1 = (data.t1z[i] * data.t2x[i] - data.t1x[i] * data.t2z[i]) * data.psalp[i];
			double zw1 = (data.t1x[i] * data.t2y[i] - data.t1y[i] * data.t2x[i]) * data.psalp[i];

			fprintf(output_fp, "\n"	// 
				" %4d %10.5f %10.5f %10.5f  %8.4f %8.4f %8.4f"	// 
				" %10.5f  %8.4f %8.4f %8.4f  %8.4f %8.4f %8.4f",
				i + 1, data.px[i], data.py[i], data.pz[i], xw1, yw1, zw1, data.pbi[i],
				data.t1x[i], data.t1y[i], data.t1z[i], data.t2x[i], data.t2y[i], data.t2z[i]);

		}		/* for( i = 0; i < data.m; i++ ) */

	}			/* if( data.m == 0) */
}

static void write_data_card_detail(FILE *output_fp)
{
	// 
	// Skip displaying "DATA CARD No:" records that display at .nec file read time,
    // but we didn't store that info at read time. FIXME
	// 
    fprintf(output_fp, "\n  DATA CARD No:   1 CM (NOT IMPLEMENTED)");
}

static void write_freq_header(FILE *output_fp, int fr_idx, double fmhz)
{
    fprintf(output_fp, "\n\n\n"
        "                               --------- FREQUENCY --------\n"
        "                                FREQUENCY :%11.4E MHz\n"
        "                                WAVELENGTH:%11.4E Mtr", fmhz, CVEL / fmhz);

    fprintf(output_fp, "\n\n"
        "                        APPROXIMATE INTEGRATION EMPLOYED FOR SEGMENTS \n"
        "                        THAT ARE MORE THAN %.3f WAVELENGTHS APART", calc_data.rkh);

    if (calc_data.iexk == 1)
        fprintf(output_fp, "\n"
            "                        THE EXTENDED THIN WIRE KERNEL WILL BE USED");
}

static void write_freq_loading(FILE *output_fp, int fr_idx, double fmhz)
{
    int i;
    fprintf(output_fp, "\n\n\n"
        "                          ------ STRUCTURE IMPEDANCE LOADING ------");

    if (zload.nload == 0)
        fprintf(output_fp, "\n"
            "                                 THIS STRUCTURE IS NOT LOADED");
    else
    {

        fprintf(output_fp, "\n"
            "  LOCATION        RESISTANCE  INDUCTANCE  CAPACITANCE   "
            "  IMPEDANCE (OHMS)   CONDUCTIVITY  CIRCUIT\n"
            "  ITAG FROM THRU     OHMS       HENRYS      FARADS     "
            "  REAL     IMAGINARY   MHOS/METER      TYPE");

        for (i = 0; i < zload.nload; i++)
        {
            int jump = calc_data.ldtyp[i] + 1;
            int ldtags = calc_data.ldtag[i];

            switch (jump)
            {
            case 1:
                prnt(output_fp, ldtags, calc_data.ldtagf[i],
                               calc_data.ldtagt[i], calc_data.zlr[i],
                               calc_data.zli[i], calc_data.zlc[i], 0., 0., 0.,
                               " SERIES ", 2);
                break;

            case 2:
                prnt(output_fp, ldtags, calc_data.ldtagf[i],
                               calc_data.ldtagt[i], calc_data.zlr[i],
                               calc_data.zli[i], calc_data.zlc[i], 0., 0., 0.,
                               "PARALLEL", 2);
                break;

            case 3:
                prnt(output_fp, ldtags, calc_data.ldtagf[i],
                               calc_data.ldtagt[i], calc_data.zlr[i],
                               calc_data.zli[i], calc_data.zlc[i], 0., 0., 0.,
                               "SERIES (PER METER)", 5);
                break;

            case 4:
                prnt(output_fp, ldtags, calc_data.ldtagf[i],
                               calc_data.ldtagt[i], calc_data.zlr[i],
                               calc_data.zli[i], calc_data.zlc[i], 0., 0., 0.,
                               "PARALLEL (PER METER)", 5);
                break;

            case 5:
                prnt(output_fp, ldtags, calc_data.ldtagf[i],
                               calc_data.ldtagt[i], 0., 0., 0.,
                               calc_data.zlr[i], calc_data.zli[i], 0.,
                               "FIXED IMPEDANCE ", 4);
                break;

            case 6:
                prnt(output_fp, ldtags, calc_data.ldtagf[i],
                               calc_data.ldtagt[i], 0., 0., 0., 0., 0.,
                               calc_data.zlr[i], "  WIRE  ", 2);

            }	/* switch( jump ) */

        }

    }
}

static void write_freq_antenna_env(FILE *output_fp, int fr_idx, double fmhz)
{
    fprintf(output_fp, "\n\n\n"
        "                            -------- ANTENNA ENVIRONMENT --------");

    if (gnd.ksymp != 1)
    {
        if (gnd.iperf != 1)
        {
            complex double epsc = cmplx(save.epsr, -save.sig * data.wlam * 59.96);

            if (gnd.nradl != 0)
            {
                fprintf(output_fp, "\n"
                    "                            RADIAL WIRE GROUND SCREEN\n"
                    "                            %d WIRES\n"
                    "                            WIRE LENGTH: %8.2f METERS\n"
                    "                            WIRE RADIUS: %10.3E METERS",
                    gnd.nradl, save.scrwlt, save.scrwrt);

                fprintf(output_fp, "\n"
                    "                            MEDIUM UNDER SCREEN -");

            }	/* if( gnd.nradl != 0) */

            if (gnd.iperf != 2)
                fprintf(output_fp, "\n"
                    "                            FINITE GROUND - REFLECTION COEFFICIENT APPROXIMATION");
            else
            {
                if (cabs((ggrid.epscf - epsc) / epsc) >= 1.0e-3)
                {
                    fprintf(output_fp,
                        "\n ERROR IN GROUND PARAMETERS -"
                        "\n COMPLEX DIELECTRIC CONSTANT FROM FILE IS: %12.5E%+12.5Ej"
                        "\n                                REQUESTED: %12.5E%+12.5Ej",
                        creal(ggrid.epscf), cimag(ggrid.epscf),
                        creal(epsc), cimag(epsc));
                    return;
                }

                fprintf(output_fp, "\n"
                    "                            FINITE GROUND - SOMMERFELD SOLUTION");

            }	/* if( gnd.iperf != 2) */

            fprintf(output_fp, "\n"
                "                            RELATIVE DIELECTRIC CONST: %.3f\n"
                "                            CONDUCTIVITY: %10.3E MHOS/METER\n"
                "                            COMPLEX DIELECTRIC CONSTANT: %11.4E%+11.4Ej",
                save.epsr, save.sig, creal(epsc), cimag(epsc));

        }	/* if( gnd.iperf != 1) */
        else
            fprintf(output_fp, "\n" "                            PERFECT GROUND");

    }	/* if( gnd.ksymp != 1) */
    else
        fprintf(output_fp, "\n" "                            FREE SPACE");

}

static void write_freq_matrix_timing(FILE *output_fp, int fr_idx, double fmhz)
{
    // 
    // Excluding MATRIX TIMING
    // FIXME: Accumulate timings into a shared data structure.
    // 
    fprintf(output_fp, "\n\n\n"
        "                             ---------- MATRIX TIMING ----------\n"
        "                               FILL: 0 msec  FACTOR: 0 msec");

}

static void write_freq_antenna_inputs(FILE *output_fp, int fr_idx, double fmhz)
{
    int i;

    // From netwk()
    fprintf(output_fp, "\n\n\n"
        "                        --------- ANTENNA INPUT PARAMETERS ---------");

    fprintf(output_fp, "\n"
        "  TAG   SEG       VOLTAGE (VOLTS)         "
        "CURRENT (AMPS)         IMPEDANCE (OHMS)    "
        "    ADMITTANCE (MHOS)     POWER\n"
        "  No:   No:     REAL      IMAGINARY"
        "     REAL      IMAGINARY     REAL      "
        "IMAGINARY    REAL       IMAGINARY   (WATTS)");

    for (i = 0; i < vsorc.nsant; i++)
    {
        int isc1 = vsorc.isant[i] - 1;
        complex double vlt = vsorc.vsant[i], cux;

        complex double zped = impedance_data.zreal[fr_idx] + I * impedance_data.zimag[fr_idx];

        cux = 1 / (zped / vlt);

        complex double ymit = cux / vlt;
        double pwr = .5 * creal(vlt * conj(cux));

        fprintf(output_fp, "\n"
            " %4d %5d %11.4E %11.4E %11.4E %11.4E"
            " %11.4E %11.4E %11.4E %11.4E %11.4E",
            data.itag[isc1], isc1 + 1, creal(vlt), cimag(vlt), creal(cux), cimag(cux),
            impedance_data.zreal[fr_idx], impedance_data.zimag[fr_idx], creal(ymit),
            cimag(ymit), pwr);
    }
}

void couple( complex double *cur, double wlam );
static void write_freq_excitation(FILE *output_fp, FILE *plot_fp, int fr_idx, double fmhz)
{
	double tmp1, tmp2, tmp3, tmp4, tmp5, tmp6;
    double cmag, ph;
    complex double curi;

	char *hpol[3] = { "LINEAR", "RIGHT", "LEFT" };
    char *pnet[3] = { "        ", "STRAIGHT", " CROSSED" };

    int itmp1, itmp2, itmp3, itmp4, itmp5;

	int nthic, nphic, inc, i, j;

	nthic = 1;
	nphic = 1;
	inc = 1;
	netcx.nprint = 0;

    tmp1 = tmp2 = tmp3 = tmp4 = tmp5 = tmp6 = 0;
    itmp1 = itmp2 = itmp3 = itmp4 = itmp5 = 0;
	/*
	   l_54 
	 */
	do
	{
		if ((fpat.ixtyp != 0) && (fpat.ixtyp != 5))
		{
			if ((plot.iptflg <= 0) || (fpat.ixtyp == 4))
				fprintf(output_fp, "\n\n\n"
					"                             ---------- EXCITATION ----------");

			tmp5 = TORAD * calc_data.xpr5;
			tmp4 = TORAD * calc_data.xpr4;

			if (fpat.ixtyp == 4)
			{
				tmp1 = calc_data.xpr1 / data.wlam;
				tmp2 = calc_data.xpr2 / data.wlam;
				tmp3 = calc_data.xpr3 / data.wlam;
				tmp6 = calc_data.xpr6 / (data.wlam * data.wlam);

				fprintf(output_fp, "\n"
					"                                      CURRENT SOURCE\n"
					"                     -- POSITION (METERS) --       ORIENTATION (DEG)\n"
					"                     X          Y          Z       ALPHA        BETA   DIPOLE MOMENT\n"
					"               %10.5f %10.5f %10.5f  %7.2f     %7.2f    %8.3f",
                    calc_data.xpr1, calc_data.xpr2, calc_data.xpr3, calc_data.xpr4, calc_data.xpr5, calc_data.xpr6);
			}
			else
			{
				tmp1 = TORAD * calc_data.xpr1;
				tmp2 = TORAD * calc_data.xpr2;
				tmp3 = TORAD * calc_data.xpr3;
				tmp6 = calc_data.xpr6;

				if (plot.iptflg <= 0)
					fprintf(output_fp,
						"\n  PLANE WAVE - THETA: %7.2f deg, PHI: %7.2f deg,"
						" ETA=%7.2f DEG, TYPE - %s  AXIAL RATIO: %6.3f",
						calc_data.xpr1, calc_data.xpr2, calc_data.xpr3, hpol[fpat.ixtyp - 1], calc_data.xpr6);

			}	/* if( fpat.ixtyp == 4) */

		}		/* if( (fpat.ixtyp != 0) && (fpat.ixtyp <= 4) ) */

		/*
		   fills e field right-hand matrix 
		 */
		//etmns(tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, fpat.ixtyp, save.cur[fr_idx]);
        //data.wlam = CVEL/fmhz;

		/*
		   matrix solving (netwk calls solves) 
		 */
		if ((netcx.nonet != 0) && (inc <= 1))
		{
			fprintf(output_fp, "\n\n\n"
				"                                            " "---------- NETWORK DATA ----------");

			itmp3 = 0;
			itmp1 = netcx.ntyp[0];

			for (i = 0; i < 2; i++)
			{
				if (itmp1 == 3)
					itmp1 = 2;

				if (itmp1 == 2)
					fprintf(output_fp, "\n"
						"  -- FROM -  --- TO --      TRANSMISSION LINE       "
						" --------- SHUNT ADMITTANCES (MHOS) ---------   LINE\n"
						"  TAG   SEG  TAG   SEG    IMPEDANCE      LENGTH    "
						" ----- END ONE -----      ----- END TWO -----   TYPE\n"
						"  No:   No:  No:   No:         OHMS      METERS     "
						" REAL      IMAGINARY      REAL      IMAGINARY");
				else if (itmp1 == 1)
					fprintf(output_fp, "\n"
						"  -- FROM -  --- TO --            --------"
						" ADMITTANCE MATRIX ELEMENTS (MHOS) ---------\n"
						"  TAG   SEG  TAG   SEG   ----- (ONE,ONE) ------  "
						" ----- (ONE,TWO) -----   ----- (TWO,TWO) -------\n"
						"  No:   No:  No:   No:      REAL      IMAGINARY     "
						" REAL     IMAGINARY       REAL      IMAGINARY");

				for (j = 0; j < netcx.nonet; j++)
				{
					itmp2 = netcx.ntyp[j];

					if ((itmp2 / itmp1) != 1)
						itmp3 = itmp2;
					else
					{
						int idx4, idx5;

						itmp4 = netcx.iseg1[j];
						itmp5 = netcx.iseg2[j];
						idx4 = itmp4 - 1;
						idx5 = itmp5 - 1;

						fprintf(output_fp, "\n"
							" %4d %5d %4d %5d  %11.4E %11.4E  "
							"%11.4E %11.4E  %11.4E %11.4E %s",
							data.itag[idx4], itmp4, data.itag[idx5], itmp5,
							netcx.x11r[j], netcx.x11i[j], netcx.x12r[j], netcx.x12i[j],
							netcx.x22r[j], netcx.x22i[j], pnet[itmp2 - 1]);

					}	/* if(( itmp2/ itmp1) == 1) */

				}	/* for( j = 0; j < netcx.nonet; j++) */

				if (itmp3 == 0)
					break;

				itmp1 = itmp3;

			}	/* for( j = 0; j < netcx.nonet; j++) */

		}		/* if( (netcx.nonet != 0) && (inc <= 1) ) */

		if ((inc > 1) && (plot.iptflg > 0))
			netcx.nprint = 1;

		//netwk(cm, save.ip, save.cur[fr_idx]);
		//netcx.ntsol = 1;

        /*
		if (calc_data.iped != 0)
		{
			itmp1 = 4 * (mhz - 1);

			fnorm[itmp1] = creal(netcx.zped);
			fnorm[itmp1 + 1] = cimag(netcx.zped);
			fnorm[itmp1 + 2] = cabs(netcx.zped);
			fnorm[itmp1 + 3] = cang(netcx.zped);

			if (calc_data.iped != 2)
			{
				if (fnorm[itmp1 + 2] > zpnorm)
					zpnorm = fnorm[itmp1 + 2];
			}

		}*/		/* if( calc_data.iped != 0) */

		/*
		   printing structure currents 
		 */
		if (data.n != 0)
		{
			if (plot.iptflg != -1)
			{
				if (plot.iptflg <= 0)
				{
					fprintf(output_fp, "\n\n\n"
						"                           -------- CURRENTS AND LOCATION --------\n"
						"                                  DISTANCES IN WAVELENGTHS");

					fprintf(output_fp, "\n\n"
						"   SEG  TAG    COORDINATES OF SEGM CENTER     SEGM"
						"    ------------- CURRENT (AMPS) -------------\n"
						"   No:  No:       X         Y         Z      LENGTH"
						"     REAL      IMAGINARY    MAGN        PHASE");
				}
				else
				{
					if ((plot.iptflg != 3) && (inc <= 1))
						fprintf(output_fp, "\n\n\n"
							"                        -------- RECEIVING PATTERN PARAMETERS --------\n"
							"                                 ETA: %7.2f DEGREES\n"
							"                                 TYPE: %s\n"
							"                                 AXIAL RATIO: %6.3f\n\n"
							"                        THETA     PHI      ----- CURRENT ----    SEG\n"
							"                        (DEG)    (DEG)     MAGNITUDE    PHASE    No:",
							calc_data.xpr3, hpol[fpat.ixtyp - 1], calc_data.xpr6);

				}	/* if( plot.iptflg <= 0) */

			}	/* if( plot.iptflg != -1) */

			//fpat.ploss = 0.;
			itmp1 = 0;

			for (i = 0; i < data.n; i++)
			{
				if (plot.iptflg == -1)
					break; //continue; // break because plot.iptflg is invariant.

                //data.wlam = fmhz/CVEL;
				curi = save.cur[fr_idx][i] * data.wlam;
				cmag = cabs(curi);
				ph = cang(curi);
  //printf("%d. fmhz=%f save.cur[fr_idx][i]=%f curi=%f wlam=%f\n", i, fmhz, cabs(save.cur[fr_idx][i]), curi, data.wlam);

				//if ((zload.nload != 0) && (fabs(creal(zload.zarray[i])) >= 1.e-20))
				//	fpat.ploss += 0.5 * cmag * cmag * creal(zload.zarray[i]) * data.si[i];


				if (plot.iptflg >= 0)
				{
					if ((plot.iptag != 0) && (data.itag[i] != plot.iptag))
						continue;

					itmp1++;
					if ((itmp1 < plot.iptagf) || (itmp1 > plot.iptagt))
						continue;

					if (plot.iptflg != 0)
					{
						if (plot.iptflg >= 2)
						{
							//fnorm[inc - 1] = cmag;
							//isave = (i + 1);
						}

						if (plot.iptflg != 3)
						{
							fprintf(output_fp, "\n"
								"                      %7.2f  %7.2f   %11.4E  %7.2f  %5d",
								calc_data.xpr1, calc_data.xpr2, cmag, ph, i + 1);
							continue;
						}

					}	/* if( plot.iptflg != 0) */
					else
						fprintf(output_fp, "\n"
							" %5d %4d %9.4f %9.4f %9.4f %9.5f"
							" %11.4E %11.4E %11.4E %8.3f",
							i + 1, data.itag[i], data.x[i], data.y[i], data.z[i],
							data.si[i], creal(curi), cimag(curi), cmag, ph);

				}	/* if( plot.iptflg >= 0 ) */
				else
				{
					fprintf(output_fp, "\n"
						" %5d %4d %9.4f %9.4f %9.4f %9.5f"
						" %11.4E %11.4E %11.4E %8.3f",
						i + 1, data.itag[i], data.x[i], data.y[i], data.z[i],
						data.si[i], creal(curi), cimag(curi), cmag, ph);

					if (plot.iplp1 != 1)
						continue;

					if (plot.iplp2 == 1)
						fprintf(plot_fp, "%12.4E %12.4E\n", creal(curi), cimag(curi));
					else if (plot.iplp2 == 2)
						fprintf(plot_fp, "%12.4E %12.4E\n", cmag, ph);
				}

			}	/* for( i = 0; i < n; i++ ) */

			if (plot.iptflq != -1)
			{
				fprintf(output_fp, "\n\n\n"
					"                                  ------ CHARGE DENSITIES ------\n"
					"                                     DISTANCES IN WAVELENGTHS\n\n"
					"   SEG   TAG    COORDINATES OF SEG CENTER     SEG        "
					"  CHARGE DENSITY (COULOMBS/METER)\n"
					"   No:   No:     X         Y         Z       LENGTH   "
					"  REAL      IMAGINARY     MAGN       PHASE");

				itmp1 = 0;
				double fr = 1.e-6 / fmhz;
                //double fr = fmhz;

				for (i = 0; i < data.n; i++)
				{
					if (plot.iptflq != -2)
					{
						if ((plot.iptaq != 0) && (data.itag[i] != plot.iptaq))
							continue;

						itmp1++;
						if ((itmp1 < plot.iptaqf) || (itmp1 > plot.iptaqt))
							continue;

					}	/* if( plot.iptflq == -2) */

					//curi = fr * cmplx(-save.bii[fr_idx][i], save.bir[fr_idx][i]);
					curi = fr * cmplx(-crnt.bii[i], crnt.bir[i]);
					
					cmag = cabs(curi);
					ph = cang(curi);

					fprintf(output_fp, "\n"
                        " %5d %4d %9.4f %9.4f %9.4f %9.5f %11.4E %11.4E %11.4E %8.3f",
						i + 1, data.itag[i], data.x[i], data.y[i], data.z[i],
						data.si[i], creal(curi), cimag(curi), cmag, ph);

				}	/* for( i = 0; i < n; i++ ) */

			}	/* if( plot.iptflq != -1) */

		}		/* if( n != 0) */

		if (data.m != 0)
		{
			fprintf(output_fp, "\n\n\n"
				"                                       --------- SURFACE PATCH CURRENTS ---------\n"
				"                                                 DISTANCE IN WAVELENGTHS\n"
				"                                                 CURRENT IN AMPS/METER\n\n"
				"                                 --------- SURFACE COMPONENTS --------   "
				" ---------------- RECTANGULAR COMPONENTS ----------------\n"
				"  PCH   --- PATCH CENTER ---     TANGENT VECTOR 1    "
				" TANGENT VECTOR 2    ------- X ------    ------- Y ------   "
				" ------- Z ------\n  No:    X       Y       Z       MAG.    "
				"   PHASE     MAG.       PHASE    REAL   IMAGINARY    REAL  "
				" IMAGINARY    REAL   IMAGINARY");

			j = data.n - 3;
			itmp1 = -1;

			for (i = 0; i < data.m; i++)
			{
				j += 3;
				itmp1++;
				complex double ex = save.cur[fr_idx][j];
				complex double ey = save.cur[fr_idx][j + 1];
				complex double ez = save.cur[fr_idx][j + 2];
				complex double eth = ex * data.t1x[itmp1] + ey * data.t1y[itmp1] + ez * data.t1z[itmp1];
				complex double eph = ex * data.t2x[itmp1] + ey * data.t2y[itmp1] + ez * data.t2z[itmp1];
				double ethm = cabs(eth);
				double etha = cang(eth);
				double ephm = cabs(eph);
				double epha = cang(eph);

				fprintf(output_fp, "\n"
					" %4d %7.3f %7.3f %7.3f %11.4E %8.2f %11.4E %8.2f"
					" %9.2E %9.2E %9.2E %9.2E %9.2E %9.2E",
					i + 1, data.px[itmp1], data.py[itmp1], data.pz[itmp1],
					ethm, etha, ephm, epha, creal(ex), cimag(ex),
					creal(ey), cimag(ey), creal(ez), cimag(ez));

				if (plot.iplp1 != 1)
					continue;

				if (plot.iplp3 == 1)
					fprintf(plot_fp, "%12.4E %12.4E\n", creal(ex), cimag(ex));
				if (plot.iplp3 == 2)
					fprintf(plot_fp, "%12.4E %12.4E\n", creal(ey), cimag(ey));
				if (plot.iplp3 == 3)
					fprintf(plot_fp, "%12.4E %12.4E\n", creal(ez), cimag(ez));
				if (plot.iplp3 == 4)
					fprintf(plot_fp, "%12.4E %12.4E %12.4E %12.4E %12.4E %12.4E\n",
						creal(ex), cimag(ex), creal(ey), cimag(ey), creal(ez), cimag(ez));

			}	/* for( i=0; i<m; i++ ) */

		}		/* if( m != 0) */

		if ((fpat.ixtyp == 0) || (fpat.ixtyp == 5))
		{
			tmp1 = netcx.pin - netcx.pnls - fpat.ploss;
			tmp2 = 100. * tmp1 / netcx.pin;

			fprintf(output_fp, "\n\n\n"
				"                               ---------- POWER BUDGET ---------\n"
				"                               INPUT POWER   = %11.4E Watts\n"
				"                               RADIATED POWER= %11.4E Watts\n"
				"                               STRUCTURE LOSS= %11.4E Watts\n"
				"                               NETWORK LOSS  = %11.4E Watts\n"
				"                               EFFICIENCY    = %7.2f Percent",
                    netcx.pin, tmp1, fpat.ploss, netcx.pnls, tmp2);

		}		/* if( (fpat.ixtyp == 0) || (fpat.ixtyp == 5) ) */

		//igo = 4;

		//if (yparm.ncoup > 0)
		//	couple(save.cur[fr_idx], data.wlam);

        /*
		if (iflow == 7)
		{
			if ((fpat.ixtyp > 0) && (fpat.ixtyp < 4))
			{
				nthic++;
				inc++;
				calc_data.xpr1 += calc_data.xpr4;

				if (nthic <= calc_data.nthi)
					continue;	// continue excitation loop 

				nthic = 1;
				calc_data.xpr1 = thetis;
				calc_data.xpr2 = calc_data.xpr2 + calc_data.xpr5;
				nphic++;

				if (nphic <= calc_data.nphi)
					continue;	// continue excitation loop 

				break;

			}

			if (nfrq != 1)
			{
				jmp_floop = TRUE;
				break;	// continue the freq loop 
			}

			fprintf(output_fp, "\n\n\n");
			jmp_iloop = TRUE;

			break;	// continue card input loop 

		}*/		/* if( iflow == 7) */

//case 4:			/* label_71 */
		//igo = 5;

		/*
		   label_72 
		 */
//case 5:			/* near field calculation */

		/*
        if (fpat.near != -1)
		{
			nfpat();

			if (mhz == nfrq)
				fpat.near = -1;

			if (nfrq == 1)
			{
				fprintf(output_fp, "\n\n\n"); // <<<<< NEED THIS?
				jmp_iloop = TRUE;
				break;	// continue card input loop 
			}

		}
        */

		/*
		   label_78 
		 */
//case 6:			/* standard far field calculation */

/*
		if (gnd.ifar != -1)
		{
			fpat.pinr = netcx.pin;
			fpat.pnlr = netcx.pnls;
			rdpat();
		}

*/
		/*if ((fpat.ixtyp == 0) || (fpat.ixtyp >= 4))
		{
			//if (mhz == nfrq)
			//	gnd.ifar = -1;

			if (nfrq != 1)
			{
				//jmp_floop = TRUE;
				break; // continue card input loop 
			}

			fprintf(output_fp, "\n\n\n");
			//jmp_iloop = TRUE;
			break;

		}*/		/* if( (fpat.ixtyp == 0) || (fpat.ixtyp >= 4) ) */

		nthic++;
		inc++;
		//calc_data.xpr1 += calc_data.xpr4;

		if (nthic <= calc_data.nthi)
			continue;	// continue excitation loop 

		nthic = 1;
		//calc_data.xpr1 = thetis;
		//calc_data.xpr2 += calc_data.xpr5;
		nphic++;

		if (nphic > calc_data.nphi)
			break;

	}			/* do (l_54) */
	while (TRUE);

    /*
    // jump to freq. or input loop 
    if( jmp_iloop )
      break;

    if( jmp_floop )
      continue;

    nphic = 1;
    xpr2  = phiss;
    */

}

// UNTESTED
static void write_freq_input_impedance_data(FILE *output_fp)
{
	double tmp1, tmp2, tmp3, tmp4, tmp5;

    int fr, nfrq, idx;

    //tmp1 = tmp2 = tmp3 = tmp4 = tmp5 = tmp6 = 0;
    //itmp1 = itmp2 = itmp3 = itmp4 = itmp5 = 0;

	if (calc_data.iped != 0)
	{
		int iss;

		if (vsorc.nvqd > 0)
			iss = vsorc.ivqd[vsorc.nvqd - 1];
		else
			iss = vsorc.isant[vsorc.nsant - 1];

		fprintf(output_fp, "\n\n\n"
			"                            "
			" -------- INPUT IMPEDANCE DATA --------\n"
			"                                     "
			" SOURCE SEGMENT No: %d\n"
			"                                  "
			" NORMALIZATION FACTOR:%12.5E\n\n"
			"              ----------- UNNORMALIZED IMPEDANCE ----------  "
			"  ------------ NORMALIZED IMPEDANCE -----------\n"
			"      FREQ    RESISTANCE    REACTANCE    MAGNITUDE    PHASE  "
			"  RESISTANCE    REACTANCE    MAGNITUDE    PHASE\n"
			"       MHz       OHMS         OHMS         OHMS     DEGREES  "
			"     OHMS         OHMS         OHMS     DEGREES", iss, calc_data.zpnorm);

        nfrq = 0;
        for (fr = 0; fr < calc_data.FR_cards; fr++)
            nfrq += calc_data.freq_loop_data[fr].freq_steps;

		for (idx = 0; idx < nfrq; idx++)
		{
            complex double zped = impedance_data.zreal[idx] + I * impedance_data.zimag[idx];

            tmp1 = save.freq[idx];
			tmp2 = creal(zped) / calc_data.zpnorm;
			tmp3 = cimag(zped) / calc_data.zpnorm;
			tmp4 = cabs(zped) / calc_data.zpnorm;
			tmp5 = cang(zped);

			fprintf(output_fp, "\n"
				" %9.3f   %11.4E  %11.4E  %11.4E  %7.2f  "
				" %11.4E  %11.4E  %11.4E  %7.2f",
				tmp1,
                creal(zped),  cimag(zped),  cabs(zped),  cang(zped),
                tmp2,         tmp3,         tmp4,        tmp5);

		}		/* for( i = 0; i < itmp1; i++ ) */

		fprintf(output_fp, "\n\n\n");

	}			/* if( calc_data.iped != 0) */
}

static void write_freq_radiation_pattern(FILE *output_fp, FILE *plot_fp, int fr_idx, double fmhz)
{
	char *hpol[3] = { "LINEAR", "RIGHT ", "LEFT  " };
	char *igtp[2] = { "----- POWER GAINS ----- ", "--- DIRECTIVE GAINS ---" };
	char *igax[4] = { " MAJOR", " MINOR", " VERTC", " HORIZ" };
	char *igntp[5] = { " MAJOR AXIS", "  MINOR AXIS", "    VERTICAL", "  HORIZONTAL", "       TOTAL " };

	char *hclif=NULL, *isens;
	int i, j, jump, itmp1, itmp2, kth, kph, itmp3, itmp4;
	double exrm=0., exra=0., prad, gcon, gcop, gmax, pint, tmp1, tmp2;
	double phi, thet, tha, erdm=0., erda=0., ethm2, ethm, *gain = NULL;
	double etha, ephm2, ephm, epha, tilta, emajr2, eminr2, axrat;
	double dfaz, dfaz2, cdfaz, tstor1=0., tstor2, stilta, gnmj;
	double gnmn, gnv, gnh, gtot, tmp3, tmp4, da, tmp5, tmp6;
	complex double eth, eph, erd;

	int idx;

	if (gnd.ifar > 1)
	{
		fprintf(output_fp, "\n\n\n"
			"                               " "------ FAR FIELD GROUND PARAMETERS ------\n\n");

		jump = FALSE;
		if (gnd.ifar > 3)
		{
			fprintf(output_fp, "\n"
				"                               "
				"--- RADIAL WIRE GROUND SCREEN ---\n"
				"                               "
				"NUM OF WIRES= %d\n"
				"                               "
				"WIRE LENGTH= %8.2f METERS\n"
				"                               "
				"WIRE RADIUS= %10.3E METERS", gnd.nradl, save.scrwlt, save.scrwrt);

			if (gnd.ifar == 4)
				jump = TRUE;

		}		/* if( gnd.ifar > 3) */

		if (!jump)
		{
			if ((gnd.ifar == 2) || (gnd.ifar == 5))
				hclif = "LINEAR";
			if ((gnd.ifar == 3) || (gnd.ifar == 6))
				hclif = "CIRCULAR";

			//gnd.cl = fpat.clt / data.wlam;
			//gnd.ch = fpat.cht / data.wlam;
			//gnd.zrati2 = csqrt(1. / cmplx(fpat.epsr2, -fpat.sig2 * data.wlam * 59.96));

			fprintf(output_fp, "\n"
				"                               "
				"--- %s CLIFF ---\n"
				"                               "
				"EDGE DISTANCE= %9.2f METERS\n"
				"                               "
				"       HEIGHT= %9.2f METERS\n"
				"                               "
				"--- SECOND MEDIUM ---\n"
				"                               "
				"RELATIVE DIELECTRIC CONST= %10.3f\n"
				"                               "
				"      GROUND CONDUCTIVITY= %10.3f MHOS",
				hclif, fpat.clt, fpat.cht, fpat.epsr2, fpat.sig2);

		}		/* if( ! jump ) */

	}			/* if( gnd.ifar > 1) */

	if (gnd.ifar == 1)
	{
		fprintf(output_fp, "\n\n\n"
			"                             "
			"------- RADIATED FIELDS NEAR GROUND --------\n\n"
			"    ------- LOCATION -------     --- E(THETA) ---    "
			" ---- E(PHI) ----    --- E(RADIAL) ---\n"
			"      RHO    PHI        Z           MAG    PHASE     "
			"    MAG    PHASE        MAG     PHASE\n"
			"    METERS DEGREES    METERS      VOLTS/M DEGREES   "
			"   VOLTS/M DEGREES     VOLTS/M  DEGREES");
	}
	else
	{
		itmp1 = 2 * fpat.iax;
		itmp2 = itmp1 + 1;

		fprintf(output_fp, "\n\n\n"
			"                             " "---------- RADIATION PATTERNS -----------\n");

		if (fpat.rfld >= 1.0e-20)
		{
			exrm = 1. / fpat.rfld;
			exra = fpat.rfld / data.wlam;
			exra = -360. * (exra - floor(exra));

			fprintf(output_fp, "\n"
				"                             "
				"RANGE: %13.6E METERS\n"
				"                             "
				"EXP(-JKR)/R: %12.5E AT PHASE: %7.2f DEGREES\n", fpat.rfld, exrm, exra);
		}

		fprintf(output_fp, "\n"
			" ---- ANGLES -----     %23s      ---- POLARIZATION ----  " // POWER GAINS or DIRECTIVE GAINS
			" ---- E(THETA) ----    ----- E(PHI) ------\n"
			"  THETA      PHI      %6s   %6s    TOTAL       AXIAL    "  // VERTC/HORIZ or MAJOR/MINOR
			"  TILT  SENSE   MAGNITUDE    PHASE    MAGNITUDE     PHASE\n"
			" DEGREES   DEGREES        DB       DB       DB       RATIO  "
			" DEGREES            VOLTS/M   DEGREES     VOLTS/M   DEGREES",
			igtp[fpat.ipd], igax[itmp1], igax[itmp2]);

	}			/* if( gnd.ifar == 1) */

	if ((fpat.ixtyp == 0) || (fpat.ixtyp == 5))
	{
		gcop = data.wlam * data.wlam * 2. * M_PI / (376.73 * fpat.pinr);
		prad = fpat.pinr - fpat.ploss - fpat.pnlr;
		gcon = gcop;
		if (fpat.ipd != 0)
			gcon = gcon * fpat.pinr / prad;
	}
	else if (fpat.ixtyp == 4)
	{
		fpat.pinr = 394.51 * calc_data.xpr6 * calc_data.xpr6 * data.wlam * data.wlam;
		gcop = data.wlam * data.wlam * 2. * M_PI / (376.73 * fpat.pinr);
		prad = fpat.pinr - fpat.ploss - fpat.pnlr;
		gcon = gcop;
		if (fpat.ipd != 0)
			gcon = gcon * fpat.pinr / prad;
	}
	else
	{
		gcon = 4. * M_PI / (1. + calc_data.xpr6 * calc_data.xpr6);
		gcop = gcon;
	}

	i = 0;
	gmax = -1.e+10;
	pint = 0.;
	tmp1 = fpat.dph * TORAD;
	tmp2 = .5 * fpat.dth * TORAD;
	phi = fpat.phis - fpat.dph;

	idx = 0;
	for (kph = 1; kph <= fpat.nph; kph++)
	{
		phi += fpat.dph;
		thet = fpat.thets - fpat.dth;

		for (kth = 1; kth <= fpat.nth; kth++)
		{
			thet += fpat.dth;

			// Can this move above the for(kph) loop?
			if ((gnd.ksymp == 2) && (thet > 90.01) && (gnd.ifar != 1))
				continue;

			tha = thet * TORAD;

			eth = rad_pattern[fr_idx].eth[idx];
			eph = rad_pattern[fr_idx].eph[idx];
			erd = rad_pattern[fr_idx].erd[idx];

			idx++;

			// Re-calculate tilta below because the xnec2c emagr2/eminr2 numbers
			// need to be recalculated from an intermediate value of tilta, so
			// don't use this: tilta = rad_pattern[fr_idx].tilta[idx];

			if (gnd.ifar == 1)
			{
				erdm = cabs(erd);
				erda = cang(erd);
			}

			ethm2 = creal(eth * conj(eth));
			ethm = sqrt(ethm2);
			etha = cang(eth);
			ephm2 = creal(eph * conj(eph));
			ephm = sqrt(ephm2);
			epha = cang(eph);

			/*
			   elliptical polarization calc.
			 */
			if (gnd.ifar != 1)
			{
				if ((ethm2 <= 1.0e-20) && (ephm2 <= 1.0e-20))
				{
					tilta = 0.;
					emajr2 = 0.;
					eminr2 = 0.;
					axrat = 0.;
					isens = " ";
				}
				else
				{
					dfaz = epha - etha;
					if (epha >= 0.)
						dfaz2 = dfaz - 360.;
					else
						dfaz2 = dfaz + 360.;

					if (fabs(dfaz) > fabs(dfaz2))
						dfaz = dfaz2;

					cdfaz = cos(dfaz * TORAD);
					tstor1 = ethm2 - ephm2;
					tstor2 = 2. * ephm * ethm * cdfaz;
					tilta = .5 * atan2(tstor2, tstor1);
					stilta = sin(tilta);
					tstor1 = tstor1 * stilta * stilta;
					tstor2 = tstor2 * stilta * cos(tilta);
					emajr2 = -tstor1 + tstor2 + ethm2;
					eminr2 = tstor1 - tstor2 + ephm2;
					if (eminr2 < 0.)
						eminr2 = 0.;

					axrat = sqrt(eminr2 / emajr2);
					tilta = tilta * TODEG;
					if (axrat <= 1.0e-5)
						isens = hpol[0];
					else if (dfaz <= 0.)
						isens = hpol[1];
					else
						isens = hpol[2];

				}	/* if( (ethm2 <= 1.0e-20) && (ephm2 <= 1.0e-20) ) */

				gnmj = db10(gcon * emajr2);
				gnmn = db10(gcon * eminr2);
				gnv = db10(gcon * ethm2);
				gnh = db10(gcon * ephm2);
				gtot = db10(gcon * (ethm2 + ephm2));

				// These values can be calculated from eth,eph,erd but we have them
				// so lets use them:
				/*
				DONT USE THIS HERE, idx has been incremented.
				Maybe use them?
				int mgidx = rad_pattern[idx].max_gain_idx[pol];
				gtot = rad_pattern[fr_idx].gtot[idx];
				axrat = rad_pattern[fr_idx].axrt[idx];
				isens = rad_pattern[fr_idx].isens[idx];
				*/

				if (fpat.inor > 0)
				{
					i++;
					switch (fpat.inor)
					{
					case 1:
						tstor1 = gnmj;
						break;

					case 2:
						tstor1 = gnmn;
						break;

					case 3:
						tstor1 = gnv;
						break;

					case 4:
						tstor1 = gnh;
						break;

					case 5:
						tstor1 = gtot;
					}

					gain[i - 1] = tstor1;
					if (tstor1 > gmax)
						gmax = tstor1;

				}	/* if( fpat.inor > 0) */

				if (fpat.iavp != 0)
				{
					tstor1 = gcop * (ethm2 + ephm2);
					tmp3 = tha - tmp2;
					tmp4 = tha + tmp2;

					if (kth == 1)
						tmp3 = tha;
					else if (kth == fpat.nth)
						tmp4 = tha;

					da = fabs(tmp1 * (cos(tmp3) - cos(tmp4)));
					if ((kph == 1) || (kph == fpat.nph))
						da *= .5;
					pint += tstor1 * da;

					if (fpat.iavp == 2)
						continue;
				}

				if (fpat.iax != 1)
				{
					tmp5 = gnmj;
					tmp6 = gnmn;
				}
				else
				{
					tmp5 = gnv;
					tmp6 = gnh;
					// DONT USE THIS HERE, idx has been incremented.
					//tmp5 = rad_pattern[fr_idx].gtot[idx] + Polarization_Factor(POL_VERT, idx, mgidx);
					//tmp6 = rad_pattern[fr_idx].gtot[idx] + Polarization_Factor(POL_HORIZ, idx, mgidx);
				}

				ethm = ethm * data.wlam;
				ephm = ephm * data.wlam;

				if (fpat.rfld >= 1.0e-20)
				{
					ethm = ethm * exrm;
					etha = etha + exra;
					ephm = ephm * exrm;
					epha = epha + exra;
				}

				fprintf(output_fp, "\n"
					" %7.2f %9.2f  %8.2f %8.2f %8.2f %11.4f"
					" %9.2f %6s %11.4E %9.2f %11.4E %9.2f",
					thet, phi, tmp5, tmp6, gtot, axrat, tilta, isens, ethm, etha, ephm, epha);


				if (plot.iplp1 != 3)
					continue;

				if (plot.iplp3 != 0)
				{
					if (plot.iplp2 == 1)
					{
						if (plot.iplp3 == 1)
							fprintf(plot_fp, "%12.4E %12.4E %12.4E\n", thet, ethm, etha);
						else if (plot.iplp3 == 2)
							fprintf(plot_fp, "%12.4E %12.4E %12.4E\n", thet, ephm, epha);
					}

					if (plot.iplp2 == 2)
					{
						if (plot.iplp3 == 1)
							fprintf(plot_fp, "%12.4E %12.4E %12.4E\n", phi, ethm, etha);
						else if (plot.iplp3 == 2)
							fprintf(plot_fp, "%12.4E %12.4E %12.4E\n", phi, ephm, epha);
					}
				}

				if (plot.iplp4 == 0)
					continue;

				if (plot.iplp2 == 1)
				{
					switch (plot.iplp4)
					{
					case 1:
						fprintf(plot_fp, "%12.4E %12.4E\n", thet, tmp5);
						break;
					case 2:
						fprintf(plot_fp, "%12.4E %12.4E\n", thet, tmp6);
						break;
					case 3:
						fprintf(plot_fp, "%12.4E %12.4E\n", thet, gtot);
					}
				}

				if (plot.iplp2 == 2)
				{
					switch (plot.iplp4)
					{
					case 1:
						fprintf(plot_fp, "%12.4E %12.4E\n", phi, tmp5);
						break;
					case 2:
						fprintf(plot_fp, "%12.4E %12.4E\n", phi, tmp6);
						break;
					case 3:
						fprintf(plot_fp, "%12.4E %12.4E\n", phi, gtot);
					}
				}

				continue;
			}	/* if( gnd.ifar != 1) */

			// else, gnd.ifar == 1:
			fprintf(output_fp, "\n"
				" %9.2f %7.2f %9.2f  %11.4E %7.2f  %11.4E %7.2f  %11.4E %7.2f",
				fpat.rfld, phi, thet, ethm, etha, ephm, epha, erdm, erda);

		}		/* for( kth = 1; kth <= fpat.nth; kth++ ) */

	}			/* for( kph = 1; kph <= fpat.nph; kph++ ) */

	if (fpat.iavp != 0)
	{
		tmp3 = fpat.thets * TORAD;
		tmp4 = tmp3 + fpat.dth * TORAD * (double) (fpat.nth - 1);
		tmp3 = fabs(fpat.dph * TORAD * (double) (fpat.nph - 1) * (cos(tmp3) - cos(tmp4)));
		pint /= tmp3;
		tmp3 /= M_PI;

		fprintf(output_fp, "\n\n\n"
			"  AVERAGE POWER GAIN: %11.4E - SOLID ANGLE"
			" USED IN AVERAGING: (%+7.4f)*M_PI STERADIANS", pint, tmp3);
	}

	if (fpat.inor > 0)
	{
		if (fabs(fpat.gnor) > 1.0e-20)
			gmax = fpat.gnor;
		itmp1 = (fpat.inor - 1);

		fprintf(output_fp, "\n\n\n"
			"                             "
			" ---------- NORMALIZED GAIN ----------\n"
			"                                      %6s GAIN\n"
			"                                  "
			" NORMALIZATION FACTOR: %.2f db\n\n"
			"    ---- ANGLES ----                ---- ANGLES ----"
			"                ---- ANGLES ----\n"
			"    THETA      PHI        GAIN      THETA      PHI  "
			"      GAIN      THETA      PHI       GAIN\n"
			"   DEGREES   DEGREES        DB     DEGREES   DEGREES "
			"       DB     DEGREES   DEGREES       DB", igntp[itmp1], gmax);

		itmp2 = fpat.nph * fpat.nth;
		itmp1 = (itmp2 + 2) / 3;
		itmp2 = itmp1 * 3 - itmp2;
		itmp3 = itmp1;
		itmp4 = 2 * itmp1;

		if (itmp2 == 2)
			itmp4--;

		for (i = 0; i < itmp1; i++)
		{
			itmp3++;
			itmp4++;
			j = i / fpat.nth;
			tmp1 = fpat.thets + (double) (i - j * fpat.nth) * fpat.dth;
			tmp2 = fpat.phis + (double) (j) * fpat.dph;
			j = (itmp3 - 1) / fpat.nth;
			tmp3 = fpat.thets + (double) (itmp3 - j * fpat.nth - 1) * fpat.dth;
			tmp4 = fpat.phis + (double) (j) * fpat.dph;
			j = (itmp4 - 1) / fpat.nth;
			tmp5 = fpat.thets + (double) (itmp4 - j * fpat.nth - 1) * fpat.dth;
			tmp6 = fpat.phis + (double) (j) * fpat.dph;
			tstor1 = gain[i] - gmax;

			if (((i + 1) == itmp1) && (itmp2 != 0))
			{
				if (itmp2 != 2)
				{
					tstor2 = gain[itmp3 - 1] - gmax;
					fprintf(output_fp, "\n"
						" %9.2f %9.2f %9.2f   %9.2f %9.2f %9.2f   ",
						tmp1, tmp2, tstor1, tmp3, tmp4, tstor2);
					free_ptr((void *) &gain);
					return;
				}

				fprintf(output_fp, "\n" " %9.2f %9.2f %9.2f   ", tmp1, tmp2, tstor1);
				free_ptr((void *) &gain);
				return;

			}	/* if( ((i+1) == itmp1) && (itmp2 != 0) ) */

			tstor2 = gain[itmp3 - 1] - gmax;
			pint = gain[itmp4 - 1] - gmax;

			fprintf(output_fp, "\n"
				" %9.2f %9.2f %9.2f   %9.2f %9.2f %9.2f   %9.2f %9.2f %9.2f",
				tmp1, tmp2, tstor1, tmp3, tmp4, tstor2, tmp5, tmp6, pint);

		}		/* for( i = 0; i < itmp1; i++ ) */
	}
}

static void write_freq_near_fields(FILE * output_fp, FILE * plot_fp, int fr_idx, double fmhz, int nfeh)
{
	int i, j, kk;
	double xob, yob;
	double zob, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, xxx;
	complex double ex, ey, ez;

	int idx;

	// If xnec2c isn't rendering these, then they need computed:
	if (isFlagClear(ENABLE_NEAREH) || !near_field[fr_idx].valid )
		return;

	if (nfeh != 1)
	{
		fprintf(output_fp, "\n\n\n"
			"                             -------- NEAR ELECTRIC FIELDS --------\n"
			"     ------- LOCATION -------     ------- EX ------    ------- EY ------    ------- EZ ------\n"
			"      X         Y         Z       MAGNITUDE   PHASE    MAGNITUDE   PHASE    MAGNITUDE   PHASE\n"
			"    METERS    METERS    METERS     VOLTS/M  DEGREES    VOLTS/M   DEGREES     VOLTS/M  DEGREES");
	}
	else
	{
		fprintf(output_fp, "\n\n\n"
			"                                   -------- NEAR MAGNETIC FIELDS ---------\n\n"
			"     ------- LOCATION -------     ------- HX ------    ------- HY ------    ------- HZ ------\n"
			"      X         Y         Z       MAGNITUDE   PHASE    MAGNITUDE   PHASE    MAGNITUDE   PHASE\n"
			"    METERS    METERS    METERS      AMPS/M  DEGREES      AMPS/M  DEGREES      AMPS/M  DEGREES");
	}

	idx = 0;
	for (i = 0; i < fpat.nrz; i++)
	{
		for (j = 0; j < fpat.nry; j++)
		{
			for (kk = 0; kk < fpat.nrx; kk++)
			{
				xob = near_field[fr_idx].px[idx];
				yob = near_field[fr_idx].py[idx];
				zob = near_field[fr_idx].pz[idx];

				if (nfeh == 1)	/* Magnetic field */
				{
					tmp1 = near_field[fr_idx].hx[idx];
					tmp3 = near_field[fr_idx].hy[idx];
					tmp5 = near_field[fr_idx].hz[idx];
					tmp2 = near_field[fr_idx].fhx[idx] / TORAD;
					tmp4 = near_field[fr_idx].fhy[idx] / TORAD;
					tmp6 = near_field[fr_idx].fhz[idx] / TORAD;
				}
				else	/* Electric field */
				{
					tmp1 = near_field[fr_idx].ex[idx];
					tmp3 = near_field[fr_idx].ey[idx];
					tmp5 = near_field[fr_idx].ez[idx];
					tmp2 = near_field[fr_idx].fex[idx] / TORAD;
					tmp4 = near_field[fr_idx].fey[idx] / TORAD;
					tmp6 = near_field[fr_idx].fez[idx] / TORAD;
				}

				fprintf(output_fp, "\n"
					" %9.4f %9.4f %9.4f  %11.4E %7.2f  %11.4E %7.2f  %11.4E %7.2f",
					xob, yob, zob, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6);

				idx++;

				if (plot.iplp1 != 2)
					continue;

				if (plot.iplp4 < 0)
					xxx = xob;
				else if (plot.iplp4 == 0)
					xxx = yob;
				else
					xxx = zob;

				if (plot.iplp2 == 2)
				{
					switch (plot.iplp3)
					{
					case 1:
						fprintf(plot_fp, "%12.4E %12.4E %12.4E\n", xxx, tmp1, tmp2);
						break;
					case 2:
						fprintf(plot_fp, "%12.4E %12.4E %12.4E\n", xxx, tmp3, tmp4);
						break;
					case 3:
						fprintf(plot_fp, "%12.4E %12.4E %12.4E\n", xxx, tmp5, tmp6);
						break;
					case 4:
						fprintf(plot_fp, "%12.4E %12.4E %12.4E %12.4E %12.4E %12.4E %12.4E\n",
							xxx, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6);
					}
					continue;
				}

				if (plot.iplp2 != 1)
					continue;

				// FIXME: ex/y/z are not stored complex, these numbers are going to be wrong:
				// See fields.c:nfpat() for how they are stored in near_field.*
				// Not sure we can reconstruct them, but maybe.
				// if m=cabs(z), c=carg(z) then,
				// z = (+/-) m(sqrt( (1+tan^2(c)) / (1+tan^2(c)) # real
				//     (+/-) I*sqrt(m^2*tan^2(c)/(1+tan^2(c))) # imag
				// The sign is lost, however.
				//
				fprintf(plot_fp, "FIXME: these values are incorrect, ex/y/z are not stored complex\n");

				switch (plot.iplp3)
				{

				case 1:
					fprintf(plot_fp, "%12.4E %12.4E %12.4E\n", xxx, creal(ex), cimag(ex));
					break;
				case 2:
					fprintf(plot_fp, "%12.4E %12.4E %12.4E\n", xxx, creal(ey), cimag(ey));
					break;
				case 3:
					fprintf(plot_fp, "%12.4E %12.4E %12.4E\n", xxx, creal(ez), cimag(ez));
					break;
				case 4:
					fprintf(plot_fp, "%12.4E %12.4E %12.4E %12.4E %12.4E %12.4E %12.4E\n",
						xxx, creal(ex), cimag(ex), creal(ey), cimag(ey), creal(ez), cimag(ez));
				}

			}	/* for( kk = 0; kk < fpat.nrx; kk++ ) */

		}		/* for( j = 0; j < fpat.nry; j++ ) */

	}			/* for( i = 0; i < fpat.nrz; i++ ) */
}

/*static void write_freq_(FILE *output_fp, int fr_idx, double fmhz)
{
}
*/

static void write_freq_far_field_data(FILE * output_fp, int fr_idx, double fmhz)
{
	time_t rawtime;
	struct tm *info;
	char buffer[80], *filename;

	int idx, kth, kph;
	double phi, thet;

	idx = 0;
	time(&rawtime);
	info = localtime(&rawtime);
	strftime(buffer, sizeof(buffer) - 1, "%F     %H:%M:%S", info);

    filename = rc_config.input_file;
    while (filename && *filename != '\0' && *filename != '/' && *filename != '\\')
        filename++;

    if (!*filename)
        filename = rc_config.input_file;
    else
        filename++;

	fprintf(output_fp, "                      xnec2c ver. %s\n\n", VERSION);
	fprintf(output_fp, "%-45s%s\n\n", filename, buffer);
	fprintf(output_fp, "         --------------- FAR FIELD PATTERN DATA ---------------\n\n");
	fprintf(output_fp, "Frequency = %.2f MHz\n\n", fmhz);
	fprintf(output_fp, "   Reference = %.1f dBi (what is this? Hard-coded at 0 for now.)\n", 0.0);

    for (kth = 0; kth < fpat.nth; kth++)
	{
        //thet += fpat.dth;
        thet = fpat.thets + (fpat.dth*kth);

		fprintf(output_fp, "\nAzimuth Pattern   Elevation angle = %.1f deg.\n",
            90-thet);
		fprintf(output_fp, " Deg      V dB      H dB      Tot dB\n");

        idx = kth;

        // exclude the 360'th degree:
        for (kph = 0; kph < fpat.nph-1; kph++)
		{
            phi = fpat.phis + (fpat.dph*kph);

            double gnv = rad_pattern[fr_idx].gtot[idx] + Polarization_Factor( POL_VERT, fr_idx, idx);
            double gnh = rad_pattern[fr_idx].gtot[idx] + Polarization_Factor( POL_HORIZ, fr_idx, idx);
            double gtot = rad_pattern[fr_idx].gtot[idx];

            if (gnv < -99.99) gnv = -99.99;
            if (gnh < -99.99) gnh = -99.99;
            if (gtot < -99.99) gtot = -99.99;

            fprintf(output_fp, "%5.1f    %6.2f    %6.2f    %6.2f\n",
                phi, gnv, gnh, gtot);

            idx += fpat.nth;
		}
	}
}

void write_nec2_output()
{
	FILE *output_fp, *plot_fp, *far_fp;
	int idx, fr;

	double prev_freq_mhz = calc_data.freq_mhz;

	// The Frequency_Scale_Geometry() function scales geometry by a fraction 
	// of the speed of light, so setting freq_mhz to the speed of light
	// makes it scale by 1.0.  The original nec2c code writes patch data
	// before scaling takes place, so we have to rescale if the frequency loop
	// already ran:

	calc_data.freq_mhz = CVEL;
	Frequency_Scale_Geometry();
	calc_data.freq_mhz = prev_freq_mhz;

	output_fp = fopen("nec2c.out", "w");
	if (!output_fp)
	{
		perror("nec2.out");
		return;
	}

	far_fp = fopen("far.out", "w");
	if (!far_fp)
	{
		perror("far.out");
		return;
	}

	if (plot.plotfile[0] != 0)
	{
		plot_fp = fopen(plot.plotfile, "w");
		if (!plot_fp)
		{
			perror(plot.plotfile);
			return;
		}
	}
	else
		plot_fp = NULL;

	write_heading(output_fp);
	write_comments(output_fp);

	write_structure_spec(output_fp);	// partially implemented

	write_seg_data(output_fp, plot_fp);
	write_surface_patch_data(output_fp);

	write_data_card_detail(output_fp);	// not implemented

	double fmhz;

	idx = 0;

	// foreach FR card:
	for (fr = 0; fr < calc_data.FR_cards; fr++, idx++)
	{
		fmhz = calc_data.freq_loop_data[fr].min_freq;
		int fr_idx;

		// foreach frequency in the FR card:
		for (fr_idx = 0; fr_idx < calc_data.freq_loop_data[fr].freq_steps; fr_idx++)
		{
			if (fr_idx > 0)
			{
				/*
				   Increment frequency: ifreq is "IFRQ (I1) from the FR card specification: * 0 -
				   linear stepping * 1 - multiplicative stepping 
				 */
				if (calc_data.freq_loop_data[fr].ifreq == 1)
					fmhz *= calc_data.freq_loop_data[fr].delta_freq;
				else
					fmhz += calc_data.freq_loop_data[fr].delta_freq;
			}

			calc_data.freq_mhz = fmhz;
			g_mutex_unlock(&freq_data_lock);
			New_Frequency();
			g_mutex_lock(&freq_data_lock);
			// calc_data.freq_mhz = prev_freq_mhz;
			/*
			 */

			write_freq_header(output_fp, fr_idx, fmhz);
			write_freq_loading(output_fp, fr_idx, fmhz);
			write_freq_antenna_env(output_fp, fr_idx, fmhz);
			write_freq_matrix_timing(output_fp, fr_idx, fmhz);
			write_freq_antenna_inputs(output_fp, fr_idx, fmhz);

			// Needs New_Frequency to set save.cur[fr_idx][] correctly per frequency.
			// Maybe keep these in a buffer?
			write_freq_excitation(output_fp, plot_fp, fr_idx, fmhz);

			if (fpat.nfeh & NEAR_EFIELD)
				write_freq_near_fields(output_fp, plot_fp, fr_idx, fmhz, 0);

			if (fpat.nfeh & NEAR_HFIELD)
				write_freq_near_fields(output_fp, plot_fp, fr_idx, fmhz, 1);

			write_freq_radiation_pattern(output_fp, plot_fp, fr_idx, fmhz);

			write_freq_far_field_data(far_fp, fr_idx, fmhz);
			//(output_fp, plot_fp, fr_idx, fmhz, 1);

			// END OF FREQ LOOP
		}
	}

	write_freq_input_impedance_data(output_fp);

	// FIXME: use correct timing.
	fprintf(output_fp, "\n\n  TOTAL RUN TIME: 0 msec");

	fprintf(output_fp, "\n");
	fclose(output_fp);
	fclose(far_fp);
	if (plot_fp != NULL)
		fclose(plot_fp);
}


