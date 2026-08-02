#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <sys/stat.h>

#include "mathlib.h"
#include "measurements.h"
#include "prerender/prerender_state.h"
#include "prerender/prerender_color.h"
#include "chroma/chroma_nearfield.h"
#include "shared.h"
#include "validation_dump.h"

/* Directory set by validation_dump_set_dir(); NULL disables all output. */
static char *validation_dir = NULL;

void validation_dump_set_dir(char *dir)
{
	validation_dir = dir;
}

void validation_dump_force_config(void)
{
	mathlib_t *lib;

	if (validation_dir == NULL)
		return;

	/* Force the NEC2 built-in solver for cross-machine reproducibility.  Runs
	 * after Read_Config() so it overrides the rc-file "Selected Mathlib", which
	 * mathlib_config_init() applies during config load. */
	lib = get_mathlib_by_id("nec2-builtin");
	if (lib == NULL)
		pr_warn("validation: nec2-builtin mathlib not available; "
			"numeric results may differ from reference\n");
	else
	{
		current_mathlib = lib;
		pr_notice("validation: forced mathlib to nec2-builtin for reproducibility\n");
	}

	/* Any rc-configurable setting that changes dumped output must be forced to a
	 * single canonical value here: validation data is a fixed cross-machine
	 * reference, so it cannot depend on the host's saved configuration.  Future
	 * configurable settings that affect output belong in this block, each given
	 * one unified value for the purpose of the validation data. */

	/* Pin the antenna-temperature inputs: ant_temp/ant_temp_tot/gt integrate a
	 * gain-weighted sky/earth brightness (measurements.c), selecting the sky and
	 * earth noise models and the table interpolation method, and classifying each
	 * pattern cell as sky or earth by the elevation-rotated horizon.  An rc
	 * override of any of these would make those columns host-dependent, so force
	 * the canonical models and a zero elevation. */
	rc_config.ant_temp_sky       = ANT_TEMP_SKY_SYNTH_AVG;
	rc_config.ant_temp_earth     = ANT_TEMP_EARTH_DG7YBN_RESIDENTIAL;
	rc_config.ant_temp_interp    = ANT_TEMP_INTERP;
	rc_config.ant_temp_elevation = 0.0;
	pr_notice("validation: forced antenna-temp models and elevation for reproducibility\n");

	/* Pin the polarization selection: _meas_calc() reads calc_data.pol_type to
	 * pick the single polarization whose max-gain bearing and gain-weighted
	 * integration produce the compared measurement columns gain_dev_px..nz,
	 * ant_temp, ant_temp_tot, and gt.  Restore_GUI_State() sets pol_type from
	 * the host's saved polarization toggles before this runs, so an unpinned
	 * value would make those columns host-dependent.  POL_TOTAL is the rc
	 * default and the committed baseline's value. */
	calc_data.pol_type = POL_TOTAL;
	pr_notice("validation: forced polarization to total for reproducibility\n");

	/* Pin the structure viewer to its default orientation, matching the reset
	 * button (set_view_preset for the default preset).  Viewer_Gain() and
	 * Viewer_Noise_Value() resolve the viewing direction from structure_view's
	 * rotation, and _meas_calc() recomputes gain_viewer at dump time via
	 * meas_calc(), so a host-restored rotation would make the viewer columns
	 * host-dependent.  This runs after structure_view is created in main() so
	 * the reset lands on the live view. */
	view_set_angles(structure_view, VIEW_DEFAULT_WR, VIEW_DEFAULT_WI);
	view_reset_pan(structure_view);
	pr_notice("validation: reset structure viewer to default orientation for reproducibility\n");
}

/*
 * dump_each - open a file under validation_dir, call dumper, close
 */
static void dump_each(const char *name, void (*dumper)(FILE *))
{
	char path[4096];
	FILE *fp = NULL;

	snprintf(path, sizeof(path), "%s/%s", validation_dir, name);
	if (!Open_File(&fp, path, "w"))
		return;
	dumper(fp);
	fclose(fp);
}

/* measurements.csv
 * meas_write_header/meas_write_data already iterate all fsteps */
static void dump_measurements(FILE *fp)
{
	meas_write_header(fp, ",");
	meas_write_data(fp, ",");
}

/* rdpat.csv
 * mhz, fstep, phi_idx, theta_idx, phi, theta,
 * gtot, tilt, axrt, gain_horiz, gain_vert, gain_rhcp, gain_lhcp, sens */
static void dump_rdpat(FILE *fp)
{
	fprintf(fp, "mhz,fstep,phi_idx,theta_idx,phi,theta,"
		"gtot,tilt,axrt,gain_horiz,gain_vert,gain_rhcp,gain_lhcp,sens,"
		"polarization_factor_total\n");

	if (!isFlagSet(ENABLE_RDPAT))
		return;

	double dth = (double)fpat.dth * (double)TORAD;
	double dph = (double)fpat.dph * (double)TORAD;

	for (int fs = 0; fs < calc_data.steps_total; fs++)
	{
		if (!save.fstep[fs])
			continue;

		int idx = 0;
		double phi = (double)fpat.phis * (double)TORAD;

		for (int nph = 0; nph < fpat.nph; nph++)
		{
			double theta = (double)fpat.thets * (double)TORAD;

			for (int nth = 0; nth < fpat.nth; nth++)
			{
				double gtot = rad_pattern[fs].gtot[idx];

				fprintf(fp, "%.6f,%d,%d,%d,%.17g,%.17g,"
					"%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%d,%.17g\n",
					save.freq[fs], fs, nph, nth,
					phi * TODEG, theta * TODEG,
					gtot,
					rad_pattern[fs].tilt[idx],
					rad_pattern[fs].axrt[idx],
					gtot + Polarization_Factor(POL_HORIZ, fs, idx),
					gtot + Polarization_Factor(POL_VERT,  fs, idx),
					gtot + Polarization_Factor(POL_RHCP,  fs, idx),
					gtot + Polarization_Factor(POL_LHCP,  fs, idx),
					rad_pattern[fs].sens[idx],
					Polarization_Factor(POL_TOTAL, fs, idx));

				theta += dth;
				idx++;
			}

			phi += dph;
		}
	}
}

/* currents.csv
 * Per-fstep wire currents from crnt_fstep[]; geometry from data.segments[].
 * mhz, fstep, seg, tag, air, aii, bir, bii, cir, cii, cur_real, cur_imag,
 * x1, y1, z1, x2, y2, z2 */
static void dump_currents(FILE *fp)
{
	fprintf(fp, "mhz,fstep,seg,tag,"
		"air,aii,bir,bii,cir,cii,"
		"cur_real,cur_imag,"
		"x1,y1,z1,x2,y2,z2\n");

	if (data.n <= 0)
		return;

	for (int fs = 0; fs < calc_data.steps_total; fs++)
	{
		if (!save.fstep[fs] || !CRNT_FSTEP_AVAILABLE(fs))
			continue;

		crnt_t *c = &crnt_fstep[fs];

		for (int idx = 0; idx < data.n; idx++)
		{
			wire_segment_t *seg = &data.segments[idx];

			fprintf(fp, "%.6f,%d,%d,%d,"
				"%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,"
				"%.17g,%.17g,"
				"%.17g,%.17g,%.17g,%.17g,%.17g,%.17g\n",
				save.freq[fs], fs, idx + 1, seg->itag,
				c->air[idx], c->aii[idx],
				c->bir[idx], c->bii[idx],
				c->cir[idx], c->cii[idx],
				creal(c->cur[idx]), cimag(c->cur[idx]),
				seg->x1, seg->y1, seg->z1,
				seg->x2, seg->y2, seg->z2);
		}
	}
}

/* patch_currents.csv
 * Per-fstep patch currents from crnt_fstep[].  Patch center (px,py,pz) and
 * area come from the unscaled template save.{x,y,z,bi}temp[], one entry per
 * patch at n+i; tangents (t1,t2) are frequency-independent in data.patches[].
 * crnt.cur stores each patch as three rectangular (x,y,z) complex current
 * components at n+3*i, the layout the far-field integrator reads (fflds in
 * radiation.c, the patch loop in fields.c); the tangential surface currents
 * are the projections of that vector onto t1 and t2.
 * mhz, fstep, patch, area,
 * px, py, pz, t1x, t1y, t1z, t2x, t2y, t2z,
 * cur_xr, cur_xi, cur_yr, cur_yi, cur_zr, cur_zi */
static void dump_patch_currents(FILE *fp)
{
	fprintf(fp, "mhz,fstep,patch,area,"
		"px,py,pz,"
		"t1x,t1y,t1z,t2x,t2y,t2z,"
		"cur_xr,cur_xi,cur_yr,cur_yi,cur_zr,cur_zi\n");

	if (data.m <= 0)
		return;

	for (int fs = 0; fs < calc_data.steps_total; fs++)
	{
		if (!save.fstep[fs] || !CRNT_FSTEP_AVAILABLE(fs))
			continue;

		crnt_t *c = &crnt_fstep[fs];

		for (int i = 0; i < data.m; i++)
		{
			surface_patch_t *p = &data.patches[i];
			int gi = data.n + i;
			int cc = data.n + 3*i;

			fprintf(fp, "%.6f,%d,%d,%.17g,"
				"%.17g,%.17g,%.17g,"
				"%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,"
				"%.17g,%.17g,%.17g,%.17g,%.17g,%.17g\n",
				save.freq[fs], fs, i,
				save.bitemp[gi],
				save.xtemp[gi], save.ytemp[gi], save.ztemp[gi],
				p->t1x, p->t1y, p->t1z,
				p->t2x, p->t2y, p->t2z,
				creal(c->cur[cc]),   cimag(c->cur[cc]),
				creal(c->cur[cc+1]), cimag(c->cur[cc+1]),
				creal(c->cur[cc+2]), cimag(c->cur[cc+2]));
		}
	}
}

/**
 * dump_nearfield_points - write stored near-field phasors
 * @fp: destination CSV stream
 *
 * Write one row per point and enabled electric or magnetic channel. Magnitudes
 * and radian phases come directly from the saved near-field point.
 */
static void dump_nearfield_points(FILE *fp)
{
	fprintf(fp, "mhz,fstep,channel,point,"
		"px,py,pz,"
		"fx_mag,fx_ang,fy_mag,fy_ang,fz_mag,fz_ang\n");

	int npts = fpat.nrx * fpat.nry * fpat.nrz;

	for (int fs = 0; fs < calc_data.steps_total; fs++)
	{
		if (!save.fstep[fs] || !NF_FSTEP_AVAILABLE(fs))
			continue;

		near_field_t *nf = &near_field_fstep[fs];
		for (nf_channel_t channel = NF_CHAN_E;
			channel <= NF_CHAN_H; channel++)
		{
			int enabled;
			if (channel == NF_CHAN_E)
				enabled = fpat.nfeh & NEAR_EFIELD;
			else
				enabled = fpat.nfeh & NEAR_HFIELD;
			if (!enabled)
				continue;

			for (int i = 0; i < npts; i++)
			{
				near_field_point_t *pt = &nf->points[i];
				double fx_mag, fx_ang, fy_mag, fy_ang, fz_mag, fz_ang;
				if (channel == NF_CHAN_E)
				{
					fx_mag = pt->ex;
					fx_ang = pt->fex;
					fy_mag = pt->ey;
					fy_ang = pt->fey;
					fz_mag = pt->ez;
					fz_ang = pt->fez;
				}
				else
				{
					fx_mag = pt->hx;
					fx_ang = pt->fhx;
					fy_mag = pt->hy;
					fy_ang = pt->fhy;
					fz_mag = pt->hz;
					fz_ang = pt->fhz;
				}

				fprintf(fp, "%.6f,%d,%d,%d,"
					"%.17g,%.17g,%.17g,"
					"%.17g,%.17g,%.17g,%.17g,%.17g,%.17g\n",
					save.freq[fs], fs, channel, i,
					pt->px, pt->py, pt->pz,
					fx_mag, fx_ang, fy_mag, fy_ang, fz_mag, fz_ang);
			}
		}
	}
}


/* impedance.csv
 * Per-(fstep,port) nec2c ANTENNA INPUT PARAMETERS row.  V is the
 * frequency-constant EX voltage; current I=V/Z, admittance Y=1/Z, and power
 * P=0.5 Re(V conj(I)) derive from the stored port impedance Z and V.
 * mhz, fstep, port, tag, seg,
 * vreal, vimag, ireal, iimag, zreal, zimag, zmagn, zphase, yreal, yimag, power */
static void dump_impedance(FILE *fp)
{
	fprintf(fp, "mhz,fstep,port,tag,seg,"
		"vreal,vimag,ireal,iimag,zreal,zimag,zmagn,zphase,"
		"yreal,yimag,power\n");

	int n_ports = Num_Feedpoint_Ports();

	for (int fs = 0; fs < calc_data.steps_total; fs++)
	{
		if (!save.fstep[fs])
			continue;

		for (int p = 0; p < n_ports; p++)
		{
			int seg = Feedpoint_Port_Seg(p);
			complex double V = Feedpoint_Port_Voltage(p);
			complex double Z = impedance_data[fs].zreal[p] +
				I * impedance_data[fs].zimag[p];
			complex double Ic = V / Z;
			complex double Y = 1.0 / Z;
			double P = 0.5 * creal( V * conj(Ic) );

			fprintf(fp, "%.6f,%d,%d,%d,%d,"
				"%.17g,%.17g,%.17g,%.17g,"
				"%.17g,%.17g,%.17g,%.17g,"
				"%.17g,%.17g,%.17g\n",
				save.freq[fs], fs, p, Feedpoint_Port_Tag(p), seg,
				creal(V), cimag(V), creal(Ic), cimag(Ic),
				impedance_data[fs].zreal[p], impedance_data[fs].zimag[p],
				impedance_data[fs].zmagn[p], impedance_data[fs].zphase[p],
				creal(Y), cimag(Y), P);
		}
	}
}


/* nf_pre.csv
 * Per-fstep near-field presentation resolved at draw by
 * chroma_proj_frame_nearfield() at the static baseline.  Each of E, H, and
 * Poynting carries a displacement (dx,dy,dz) and its parallel palette color;
 * an absent field emits the canonical zero row so the column schema stays
 * fixed across antennas.
 * mhz, fstep, point,
 * e_dx, e_dy, e_dz, e_r, e_g, e_b,
 * h_dx, h_dy, h_dz, h_r, h_g, h_b,
 * pov_dx, pov_dy, pov_dz, pov_r, pov_g, pov_b, pov_max */
static void dump_nf_pre(FILE *fp)
{
	fprintf(fp, "mhz,fstep,point,"
		"e_dx,e_dy,e_dz,e_r,e_g,e_b,"
		"h_dx,h_dy,h_dz,h_r,h_g,h_b,"
		"pov_dx,pov_dy,pov_dz,pov_r,pov_g,pov_b,"
		"pov_max\n");

	int npts = fpat.nrx * fpat.nry * fpat.nrz;

	for (int fs = 0; fs < calc_data.steps_total; fs++)
	{
		if (!save.fstep[fs] || !NF_FSTEP_AVAILABLE(fs))
			continue;

		nf_frame_t ef = chroma_proj_frame_nearfield(fs, NF_CHAN_E);
		nf_frame_t hf = chroma_proj_frame_nearfield(fs, NF_CHAN_H);
		nf_frame_t pf = chroma_proj_frame_nearfield(fs, NF_CHAN_POV);

		/* Peak Poynting magnitude over the static real vectors */
		near_field_t *nf = &near_field_fstep[fs];
		double pov_max = 0.0;
		for (int i = 0; i < npts; i++)
		{
			double er[3], hr[3], px, py, pz, pr;
			nf_real_vector(&nf->points[i], NF_CHAN_E, FALSE, 0.0,
				rc_config.nf_static_mode, er);
			nf_real_vector(&nf->points[i], NF_CHAN_H, FALSE, 0.0,
				rc_config.nf_static_mode, hr);
			pr = nf_poynting(er, hr, &px, &py, &pz);
			if (pr > pov_max) pov_max = pr;
		}

		for (int i = 0; i < npts; i++)
		{
			nf_vector_t e = ef.vecs ? ef.vecs[i] : (nf_vector_t){0};
			nf_vector_t h = hf.vecs ? hf.vecs[i] : (nf_vector_t){0};
			nf_vector_t p = pf.vecs ? pf.vecs[i] : (nf_vector_t){0};
			rgb_f_t ec = ef.colors ? ef.colors[i] : (rgb_f_t){0};
			rgb_f_t hc = hf.colors ? hf.colors[i] : (rgb_f_t){0};
			rgb_f_t pc = pf.colors ? pf.colors[i] : (rgb_f_t){0};

			fprintf(fp, "%.6f,%d,%d,"
				"%g,%g,%g,%g,%g,%g,"
				"%g,%g,%g,%g,%g,%g,"
				"%g,%g,%g,%g,%g,%g,"
				"%g\n",
				save.freq[fs], fs, i,
				e.dx, e.dy, e.dz, ec.r, ec.g, ec.b,
				h.dx, h.dy, h.dz, hc.r, hc.g, hc.b,
				p.dx, p.dy, p.dz, pc.r, pc.g, pc.b,
				pov_max);
		}
	}
}

/* struct_colors.csv
 * Per-fstep magnitude ranges and patch flow projections from
 * struct_colors[].  One range row per fstep carries the cmin/cmax scalars;
 * one patch_flow row per patch carries the tangent phasor projection, with
 * the range scalars repeated.
 * mhz, fstep, entity_kind, entity_idx,
 * flow0, flow1, flow2, flow3,
 * wire_crnt_cmin, wire_crnt_cmax, wire_chrg_cmin, wire_chrg_cmax,
 * patch_crnt_cmin, patch_crnt_cmax */
static void dump_struct_colors(FILE *fp)
{
	fprintf(fp, "mhz,fstep,entity_kind,entity_idx,"
		"flow0,flow1,flow2,flow3,"
		"wire_crnt_cmin,wire_crnt_cmax,"
		"wire_chrg_cmin,wire_chrg_cmax,"
		"patch_crnt_cmin,patch_crnt_cmax\n");

	if (struct_colors == NULL)
		return;

	for (int fs = 0; fs < calc_data.steps_total; fs++)
	{
		if (!save.fstep[fs])
			continue;

		struct_colors_t *sc = &struct_colors[fs];

		fprintf(fp, "%.6f,%d,range,0,"
			"0,0,0,0,"
			"%g,%g,%g,%g,%g,%g\n",
			save.freq[fs], fs,
			sc->wire_crnt_cmin, sc->wire_crnt_cmax,
			sc->wire_chrg_cmin, sc->wire_chrg_cmax,
			sc->patch_crnt_cmin, sc->patch_crnt_cmax);

		if (sc->patch_flow_data != NULL)
			for (int i = 0; i < data.m; i++)
			{
				float *fl = sc->patch_flow_data[i];
				fprintf(fp, "%.6f,%d,patch_flow,%d,"
					"%g,%g,%g,%g,"
					"%g,%g,%g,%g,%g,%g\n",
					save.freq[fs], fs, i,
					fl[0], fl[1], fl[2], fl[3],
					sc->wire_crnt_cmin, sc->wire_crnt_cmax,
					sc->wire_chrg_cmin, sc->wire_chrg_cmax,
					sc->patch_crnt_cmin, sc->patch_crnt_cmax);
			}
	}
}

/* noise_temp.csv
 * Per-fstep sky/earth noise temperature tensor from noise_temp[].  The full
 * sky x earth x method cross product is emitted; t_sky depends on (sky,method),
 * t_earth on (earth,method).
 * mhz, fstep, sky_model, earth_model, method, t_sky, t_earth */
static void dump_noise_temp(FILE *fp)
{
	fprintf(fp, "mhz,fstep,sky_model,earth_model,method,t_sky,t_earth\n");

	if (noise_temp == NULL)
		return;

	for (int fs = 0; fs < calc_data.steps_total; fs++)
	{
		if (!save.fstep[fs])
			continue;

		noise_temp_t *nt = &noise_temp[fs];

		for (int sky = 0; sky < ANT_TEMP_SKY_COUNT; sky++)
			for (int earth = 0; earth < ANT_TEMP_EARTH_COUNT; earth++)
				for (int method = 0; method < ANT_TEMP_METHOD_COUNT; method++)
					fprintf(fp, "%.6f,%d,%d,%d,%d,%.17g,%.17g\n",
						save.freq[fs], fs, sky, earth, method,
						nt->t_sky[sky][method],
						nt->t_earth[earth][method]);
	}
}

/* geom_aggregate.csv
 * Frequency-independent geometry scalars and per-patch corner/tangent frames
 * from geom_pre.  Heterogeneous rows share a 12-wide value space discriminated
 * by the kind column so a single PDL frame holds all geometry-tier data.
 * kind, idx, v0..v11 */
static void dump_geom_aggregate(FILE *fp)
{
	fprintf(fp, "kind,idx,v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11\n");

	fprintf(fp, "scalar,0,"
		"%.17g,%.17g,%.17g,%.17g,%.17g,"
		"0,0,0,0,0,0,0\n",
		geom_pre.scene_radius,
		geom_pre.excitation_cx, geom_pre.excitation_cy, geom_pre.excitation_cz,
		geom_pre.nf_dr_norm);

	if (geom_pre.patch_corners != NULL)
		for (int i = 0; i < data.m; i++)
		{
			patch_corners_t *pc = &geom_pre.patch_corners[i];
			fprintf(fp, "patch_corners,%d,"
				"%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,"
				"%.17g,%.17g,%.17g,%.17g,%.17g,%.17g\n",
				i,
				pc->c0x, pc->c0y, pc->c0z,
				pc->c1x, pc->c1y, pc->c1z,
				pc->c2x, pc->c2y, pc->c2z,
				pc->c3x, pc->c3y, pc->c3z);
		}

	if (geom_pre.patch_tangent_frame != NULL)
		for (int i = 0; i < data.m; i++)
		{
			patch_tangent_frame_t *pt = &geom_pre.patch_tangent_frame[i];
			fprintf(fp, "patch_tangent,%d,"
				"%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,"
				"%.17g,%.17g,%.17g,0,0,0\n",
				i,
				pt->cx, pt->cy, pt->cz,
				pt->st1x, pt->st1y, pt->st1z,
				pt->st2x, pt->st2y, pt->st2z);
		}
}

/* geom_trig.csv
 * Radiation-pattern grid trig tables from geom_pre.  theta rows carry the
 * solid-angle weight; phi rows have no solid angle (0.0).
 * axis, idx, sin_val, cos_val, solid_angle */
static void dump_geom_trig(FILE *fp)
{
	fprintf(fp, "axis,idx,sin_val,cos_val,solid_angle\n");

	if (geom_pre.sin_theta != NULL)
		for (int i = 0; i < fpat.nth; i++)
			fprintf(fp, "theta,%d,%.17g,%.17g,%.17g\n",
				i, geom_pre.sin_theta[i], geom_pre.cos_theta[i],
				geom_pre.solid_angle[i]);

	if (geom_pre.sin_phi != NULL)
		for (int i = 0; i < fpat.nph; i++)
			fprintf(fp, "phi,%d,%.17g,%.17g,0\n",
				i, geom_pre.sin_phi[i], geom_pre.cos_phi[i]);
}

/* geom_topology.csv
 * Far-field grid edge connectivity from geom_pre; integer vertex indices.
 * axis, edge_idx, va, vb */
static void dump_geom_topology(FILE *fp)
{
	fprintf(fp, "axis,edge_idx,va,vb\n");

	for (int i = 0; i < geom_pre.n_theta_edges; i++)
		fprintf(fp, "theta,%d,%u,%u\n",
			i, geom_pre.theta_topo[i].va, geom_pre.theta_topo[i].vb);

	for (int i = 0; i < geom_pre.n_phi_edges; i++)
		fprintf(fp, "phi,%d,%u,%u\n",
			i, geom_pre.phi_topo[i].va, geom_pre.phi_topo[i].vb);
}


void Save_Validation_Tree(void)
{
	if (validation_dir == NULL)
		return;

	if (isFlagClear(FREQ_LOOP_DONE))
	{
		Notice(GTK_BUTTONS_OK, _("Validation Tree"), "Cannot save: frequency loop not done");
		return;
	}

	if (mkdir(validation_dir, 0755) < 0 && errno != EEXIST)
	{
		Notice(GTK_BUTTONS_OK, _("Validation Tree"), "Cannot create output directory");
		return;
	}

	/* The frequency loop populates near_field_fstep[] per step: New_Frequency()
	 * runs the full single-frequency solve for every step in the non-forked
	 * path and calls Near_Field_Pattern() (gated only on ENABLE_NEAREH) plus
	 * Save_Nearfield_Data() (xnec2c.c). No batch recompute is needed; a
	 * re-solve here would overwrite the global netcx.zped at stale post-loop
	 * frequency state and corrupt the impedance dump. */

	char *orig = setlocale(LC_NUMERIC, "C");

	g_rec_mutex_lock(&freq_data_lock);

	dump_each("measurements.csv",     dump_measurements);
	dump_each("rdpat.csv",            dump_rdpat);
	dump_each("currents.csv",         dump_currents);
	dump_each("patch_currents.csv",   dump_patch_currents);
	dump_each("nearfield_points.csv", dump_nearfield_points);
	dump_each("nf_pre.csv",           dump_nf_pre);
	dump_each("struct_colors.csv",    dump_struct_colors);
	dump_each("noise_temp.csv",       dump_noise_temp);
	dump_each("impedance.csv",        dump_impedance);
	dump_each("geom_aggregate.csv",   dump_geom_aggregate);
	dump_each("geom_trig.csv",        dump_geom_trig);
	dump_each("geom_topology.csv",    dump_geom_topology);

	g_rec_mutex_unlock(&freq_data_lock);

	setlocale(LC_NUMERIC, orig);

	pr_notice("validation tree written to: %s\n", validation_dir);
}
