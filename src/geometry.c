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
 *
 *  The official website and doumentation for xnec2c is available here:
 *    https://www.xnec2c.org/
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

/* Threshold for skipping O(n²) overlap check on large models */
#define SEGMENT_OVERLAP_THRESHOLD 1000

/* Calculate Manhattan distance between points */
static inline double
calc_manhattan_distance(double x1, double y1, double z1,
                       double x2, double y2, double z2)
{
  return fabs(x2 - x1) + fabs(y2 - y1) + fabs(z2 - z1);
}

/* Calculate connection threshold for a segment */
static inline double
calc_connection_threshold(double x1, double y1, double z1,
                         double x2, double y2, double z2)
{
  /* Calculate segment length using Euclidean distance */
  double dx = x2 - x1;
  double dy = y2 - y1;
  double dz = z2 - z1;
  return sqrt(dx*dx + dy*dy + dz*dz) * SMIN;
}

/* Check if two points are close enough to connect */
static inline gboolean
points_would_connect(double x1, double y1, double z1,
                    double x2, double y2, double z2,
                    double threshold)
{
  return calc_manhattan_distance(x1, y1, z1, x2, y2, z2) <= threshold;
}

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
    Stop( ERR_OK, _("Number of segments < 1") );
    return( FALSE );
  }

  if( fabs( ang2- ang1) < 360.00001)
  {
    int i;
    double ang, dang, xs1, xs2, zs1, zs2;
    mem_array_realloc(&data.segments, (data.n + data.m));

    ang= ang1* TORAD;
    dang=( ang2- ang1)* TORAD/ ns;
    xs1= rada* cos( ang);
    zs1= rada* sin( ang);

    for( i = ist; i < data.n; i++ )
    {
      ang += dang;
      xs2= rada* cos( ang);
      zs2= rada* sin( ang);
      data.segments[i].x1 = xs1;

      data.segments[i].y1=0.0;
      data.segments[i].z1 = zs1;
      data.segments[i].x2 = xs2;
      data.segments[i].y2=0.0;
      data.segments[i].z2 = zs2;
      xs1= xs2;
      zs1= zs2;
      data.segments[i].bi = rad;
      data.segments[i].itag = itg;

    } /* for( i = ist; i < data.n; i++ ) */

  } /* if( fabs( ang2- ang1) < 360.00001) */
  else
  {
    Stop( ERR_OK, _("Arc angle exceeds 360 degrees") );
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
  double xs, ys, zs, xa, xst, s1x, s1y;
  double s1z, s2x, s2y, s2z, saln, xt, yt;

  /* Reallocate patch buffers */
  if( ny == 0 ) data.m += 3;
  else data.m += 4;
  mem_array_realloc(&data.patches, data.m);
  /* Shift patches to make room for new ones */
  if( (ny == 0) && (nx != data.m) )
  {
    for( iy = data.m-1; iy > nx+2; iy-- )
    {
      ix = iy-3;
      data.patches[iy].px = data.patches[ix].px;
      data.patches[iy].py = data.patches[ix].py;
      data.patches[iy].pz = data.patches[ix].pz;
      data.patches[iy].pbi = data.patches[ix].pbi;
      data.patches[iy].psalp = data.patches[ix].psalp;
      data.patches[iy].t1x = data.patches[ix].t1x;
      data.patches[iy].t1y = data.patches[ix].t1y;
      data.patches[iy].t1z = data.patches[ix].t1z;
      data.patches[iy].t2x = data.patches[ix].t2x;
      data.patches[iy].t2y = data.patches[ix].t2y;
      data.patches[iy].t2z = data.patches[ix].t2z;
    }

  } /* if( (ny == 0) || (nx != m) ) */

  /* divide patch for connection */
  mi= nx-1;
  xs= data.patches[mi].px;
  ys= data.patches[mi].py;
  zs= data.patches[mi].pz;
  xa= data.patches[mi].pbi/4.0;
  xst= sqrt( xa)/2.0;
  s1x= data.patches[mi].t1x;
  s1y= data.patches[mi].t1y;
  s1z= data.patches[mi].t1z;
  s2x= data.patches[mi].t2x;
  s2y= data.patches[mi].t2y;
  s2z= data.patches[mi].t2z;
  saln= data.patches[mi].psalp;
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
    data.patches[mia].px = xs+ xt* s1x+ yt* s2x;
    data.patches[mia].py = ys+ xt* s1y+ yt* s2y;
    data.patches[mia].pz = zs+ xt* s1z+ yt* s2z;
    data.patches[mia].pbi = xa;
    data.patches[mia].t1x = s1x;
    data.patches[mia].t1y = s1y;
    data.patches[mia].t1z = s1z;
    data.patches[mia].t2x = s2x;
    data.patches[mia].t2y = s2y;
    data.patches[mia].t2z = s2z;
    data.patches[mia].psalp = saln;

    if( ix == 2)
      yt= -yt;

    if( (ix == 1) || (ix == 3) )
      xt= -xt;

    mia++;
  }

  if( nx <= data.mp)
    data.mp += 3;

  if( ny > 0 )
    data.patches[mi].pz=10000.0;

  /* Process new patches created */

  return;
}

/*-----------------------------------------------------------------------*/

/* connect sets up segment connection data in arrays icon1 and */
/* icon2 by searching for segment ends that are in contact.
 *
 * Note to spell checkers:  "conect" is the original FORTRAN function name
 * and we need to keep the name this way for historical reasons (mostly nostalgia)
 * because xnec2c is a C re-implementation of the original FORTRAN NEC2 code.
 * */
  gboolean
conect( int ignd )
{
  int i, iz, ic, j, jx, ix, ixx, iseg, iend, jend, jump;
  double sep=0.0, xi1, yi1, zi1, xi2, yi2, zi2;
  double slen, xa, ya, za, xs, ys, zs;

  /* Pre-allocate connection buffer based on typical wire geometry:
   * Most segments have 2 connections (previous/next), with some
   * having up to 4 at junctions */
  segj.maxcon = data.n * 4;

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
      Stop( ERR_OK, _("Error in conect(): np > n") );
      return( FALSE );
    }

    if( (data.np == data.n) && (data.mp == data.m) )
      data.ipsym=0;

  } /* if( ignd != 0) */

  if( data.n != 0)
  {
    mem_array_realloc(&data.segments, (data.n + data.m));

    for( i = 0; i < data.n; i++ )
    {
      data.segments[i].icon1 = data.segments[i].icon2 = 0;
      iz = i+1;
      xi1= data.segments[i].x1;
      yi1= data.segments[i].y1;
      zi1= data.segments[i].z1;
      xi2= data.segments[i].x2;
      yi2= data.segments[i].y2;
      zi2= data.segments[i].z2;
      slen = calc_connection_threshold(xi1, yi1, zi1, xi2, yi2, zi2);

      /* determine connection data for end 1 of segment. */
      jump = FALSE;
      if( ignd > 0)
      {
        if( zi1 <= -slen)
        {
          pr_err("geometry data error: segment %d extends below ground\n", iz);
          Stop( ERR_OK, _("Geometry data error\n"
                "Segment extends below ground") );
          return( FALSE );
        }

        if( zi1 <= slen)
        {
          data.segments[i].icon1 = iz;
          data.segments[i].z1=0.0;
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

        if (points_would_connect(xi1, yi1, zi1,
                               data.segments[ic].x1, data.segments[ic].y1, data.segments[ic].z1,
                               slen)) {
          data.segments[i].icon1 = -(ic+1);
          break;
        }

        if (points_would_connect(xi1, yi1, zi1,
                               data.segments[ic].x2, data.segments[ic].y2, data.segments[ic].z2,
                               slen)) {
          data.segments[i].icon1 = (ic+1);
          break;
        }

      } /* for( j = 1; j < data.n; j++) */

      } /* if( ! jump ) */

      /* determine connection data for end 2 of segment. */
      if( (ignd > 0) || jump )
      {
        if( zi2 <= -slen)
        {
          pr_err("geometry data error: segment %d extends below ground\n", iz);
          Stop( ERR_OK, _("Geometry data error\n"
                "Segment extends below ground") );
          return( FALSE );
        }

        if( zi2 <= slen)
        {
          if(data.segments[i].icon1 == iz )
          {
            pr_err("geometry data error: segment %d lies in ground plane\n", iz);
            Stop( ERR_OK, _("Geometry data error\n"
                  "Segment lies in ground plane") );
            return( FALSE );
          }

          data.segments[i].icon2 = iz;
          data.segments[i].z2=0.0;
          continue;

        } /* if( zi2 <= slen) */

      } /* if( ignd > 0) */

      ic= i;
      for( j = 1; j < data.n; j++ )
      {
        ic++;
        if( ic >= data.n)
          ic=0;

        if (points_would_connect(xi2, yi2, zi2,
                               data.segments[ic].x1, data.segments[ic].y1, data.segments[ic].z1,
                               slen)) {
          data.segments[i].icon2 = (ic+1);
          break;
        }

        if (points_would_connect(xi2, yi2, zi2,
                               data.segments[ic].x2, data.segments[ic].y2, data.segments[ic].z2,
                               slen)) {
          data.segments[i].icon2 = -(ic+1);
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
        xs= data.patches[ix].px;
        ys= data.patches[ix].py;
        zs= data.patches[ix].pz;

        for( iseg = 0; iseg < data.n; iseg++ )
        {
          xi1= data.segments[iseg].x1;
          yi1= data.segments[iseg].y1;
          zi1= data.segments[iseg].z1;
          xi2= data.segments[iseg].x2;
          yi2= data.segments[iseg].y2;
          zi2= data.segments[iseg].z2;
          slen = calc_connection_threshold(xi1, yi1, zi1, xi2, yi2, zi2);

          /* for first end of segment */
          /* connection - divide patch into 4 patches at present array loc. */
          if (points_would_connect(xi1, yi1, zi1, xs, ys, zs, slen)) {
            data.segments[iseg].icon1=PCHCON+ i;
            ic=0;
            subph( i, ic );
            break;
          }

          if (points_would_connect(xi2, yi2, zi2, xs, ys, zs, slen)) {
            data.segments[iseg].icon2=PCHCON+ i;
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
      pr_err("ipsym = 0\n");
      Stop( ERR_OK, _("Error: ipsym = 0") );
      return( FALSE );
    }
  } /* if( iseg != 1) */

  /* No wire segments */
  if( data.n <= 0) return( TRUE );
  mem_array_realloc(&segj.jco, segj.maxcon);

  /* Adjust connected segment ends to exactly coincide.
   * Also find old seg. connecting to new segment */
  iseg = 0;
  for( j = 0; j < data.n; j++ )
  {
    jx = j+1;
    iend=-1;
    jend=-1;
    ix= data.segments[j].icon1;
    ic=1;
    segj.jco[0]= -jx;
    xa= data.segments[j].x1;
    ya= data.segments[j].y1;
    za= data.segments[j].z1;

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
            if (segj.maxcon > data.n*(data.n-1))
            {
              pr_err("Segment connection count exceeds a full mesh, which is unlikely: data.n=%ld segj.maxcon=%lu\n",
                (long)data.n, (long)segj.maxcon);
	      Stop(ERR_STOP, "Segment connection count exceeds a full mesh, which is unlikely!\n"
		   "Check for overlapping or nearly-connected segments\n");
	      return FALSE;
            }
            mem_array_realloc(&segj.jco, segj.maxcon);
          }
          segj.jco[ic-1]= ix* jend;

          ixx = ix-1;
          if( jend != 1)
          {
            xa= xa+ data.segments[ixx].x1;
            ya= ya+ data.segments[ixx].y1;
            za= za+ data.segments[ixx].z1;
            ix= data.segments[ixx].icon1;
            continue;
          }

          xa= xa+ data.segments[ixx].x2;
          ya= ya+ data.segments[ixx].y2;
          za= za+ data.segments[ixx].z2;
          ix= data.segments[ixx].icon2;

        } /* do */
        while( ix != 0 );

        if( jump && (iend == 1) ) break;
        else if( jump )
        {
          iend=1;
          jend=1;
          ix= data.segments[j].icon2;
          ic=1;
          segj.jco[0]= jx;
          xa= data.segments[j].x2;
          ya= data.segments[j].y2;
          za= data.segments[j].z2;
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
            data.segments[ixx].x1 = xa;
            data.segments[ixx].y1 = ya;
            data.segments[ixx].z1 = za;
            continue;
          }

          ixx = ix-1;
          data.segments[ixx].x2 = xa;
          data.segments[ixx].y2 = ya;
          data.segments[ixx].z2 = za;

        } /* for( i = 0; i < ic; i++ ) */

        if( ic >= 3)
          iseg++;

      } /*if( (ix != 0) && (ix != j) && (ix <= PCHCON) ) */

      if( iend == 1) break;

      iend=1;
      jend=1;
      ix= data.segments[j].icon2;
      ic=1;
      segj.jco[0]= jx;
      xa= data.segments[j].x2;
      ya= data.segments[j].y2;
      za= data.segments[j].z2;

    } /* while( TRUE ) */

  } /* for( j = 0; j < data.n; j++ ) */
  mem_array_realloc(&segj.ax, segj.maxcon);
  mem_array_realloc(&segj.bx, segj.maxcon);
  mem_array_realloc(&segj.cx, segj.maxcon);

  return( TRUE );
}

/*-----------------------------------------------------------------------*/

/* geometry_data_free()
 *
 * Releases the geometry segment and patch arrays and the segment-connection
 * work buffers.
 */
  void
geometry_data_free( void )
{
  mem_array_free( &data.segments );
  mem_array_free( &data.patches );
  mem_array_free( &segj.jco );
  mem_array_free( &segj.ax );
  mem_array_free( &segj.bx );
  mem_array_free( &segj.cx );

} /* geometry_data_free() */

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
  double z, zinc;

  if( ns < 1) return;

  ist = data.n;
  data.n += ns;
  data.np = data.n;
  data.mp = data.m;
  data.ipsym = 0;
  if( hl < 0.0 )
  {
    hl  = -hl;
    tsp = -tsp;
  }
  mem_array_realloc(&data.segments, data.n);

  /* For a proper helix */
  if( (hl != 0.0) && (tsp != 0.0) )
  {
    z = 0.0;
    zinc = hl / (double)ns;
    for( i = ist; i < data.n; i++ )
    {
      data.segments[i].bi = rad;
      data.segments[i].itag = itg;
      data.segments[i].z1 = z;
      z += zinc;
      data.segments[i].z2 = z;

      if( a2 == a1)
      {
        if( b1 == 0.0) b1 = a1;
        data.segments[i].x1 = a1* cos(M_2PI* data.segments[i].z1 / tsp);
        data.segments[i].y1 = b1* sin(M_2PI* data.segments[i].z1 / tsp);
        data.segments[i].x2 = a1* cos(M_2PI* data.segments[i].z2 / tsp);
        data.segments[i].y2 = b1* sin(M_2PI* data.segments[i].z2 / tsp);
      }
      else
      {
        if( b2 == 0.0) b2 = a2;
        data.segments[i].x1=( a1+( a2- a1)* data.segments[i].z1 / hl)*
          cos(M_2PI* data.segments[i].z1 / tsp);
        data.segments[i].y1=( b1+( b2- b1)* data.segments[i].z1 / hl)*
          sin(M_2PI* data.segments[i].z1 / tsp);
        data.segments[i].x2=( a1+( a2- a1)* data.segments[i].z2 / hl)*
          cos(M_2PI* data.segments[i].z2 / tsp);
        data.segments[i].y2=( b1+( b2- b1)* data.segments[i].z2 / hl)*
          sin(M_2PI* data.segments[i].z2 / tsp);
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
      data.segments[i].bi = rad;
      data.segments[i].itag = itg;
      data.segments[i].z1 = 0.0;
      data.segments[i].z2 = 0.0;

      data.segments[i].x1 = a1 * cos( phi );
      data.segments[i].y1 = b1 * sin( phi );
      a1 -= da;
      b1 -= db;
      phi += dphi;
      data.segments[i].x2 = a1 * cos( phi );
      data.segments[i].y2 = b1 * sin( phi );

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
    pr_err("check data, parameter specifying segment: position in a group of equal tags must not be zero\n");
    Stop( ERR_OK, _("Parameter specifying segment\n"
          "position in a group of equal tags is zero") );
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
      if(data.segments[i].itag != itagi )
        continue;

      icnt++;
      if( icnt == mx)
      {
        iseg= i+1;
        return( iseg );
      }

    } /* for( i = 0; i < data.n; i++ ) */

  } /* if( data.n > 0) */

  pr_err("no segment has an itag of %d\n", itagi);

  if( itagi == 0 )
  {
    Stop( ERR_OK, _("Segment tag number error: Tag 0 is reserved and cannot be used.\n"
            "Check your geometry cards (especially GM cards) to ensure tag values are valid.\n"
            "For GM cards: verify 'start from tag' and 'tag increment' don't create tag 0.") );
  }
  else
  {
    char error_msg[256];
    snprintf(error_msg, sizeof(error_msg),
             _("Segment tag number error: No segment found with tag %d.\n"
               "You are referencing an unknown tag or segment in a control card.\n"
               "Valid tags in your geometry are those assigned to GW/GA/GH/SP cards."), itagi);
    Stop(ERR_OK, "%s", error_msg);
  }

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
      mem_array_realloc(&data.segments,
                        (data.n + data.m + (data.n + 1 - i1) * nrpt));
    }

    for( ir = 0; ir < nrp; ir++ )
    {
      for( i = i1-1; i < data.n; i++ )
      {
        xi= data.segments[i].x1;
        yi= data.segments[i].y1;
        zi= data.segments[i].z1;
        data.segments[k].x1 = xi* xx+ yi* xy+ zi* xz+ xs;
        data.segments[k].y1 = xi* yx+ yi* yy+ zi* yz+ ys;
        data.segments[k].z1 = xi* zx+ yi* zy+ zi* zz+ zs;
        xi= data.segments[i].x2;
        yi= data.segments[i].y2;
        zi= data.segments[i].z2;
        data.segments[k].x2 = xi* xx+ yi* xy+ zi* xz+ xs;
        data.segments[k].y2 = xi* yx+ yi* yy+ zi* yz+ ys;
        data.segments[k].z2 = xi* zx+ yi* zy+ zi* zz+ zs;
        data.segments[k].bi = data.segments[i].bi;
        data.segments[k].itag = data.segments[i].itag;
        if(data.segments[i].itag != 0)
          data.segments[k].itag = data.segments[i].itag + itgi;

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
    mem_array_realloc(&data.patches, (data.m * (nrpt + 1)));

    for( ii = 0; ii < nrp; ii++ )
    {
      for( i = i1; i < data.m; i++ )
      {
        xi= data.patches[i].px;
        yi= data.patches[i].py;
        zi= data.patches[i].pz;
        data.patches[k].px = xi* xx+ yi* xy+ zi* xz+ xs;
        data.patches[k].py = xi* yx+ yi* yy+ zi* yz+ ys;
        data.patches[k].pz = xi* zx+ yi* zy+ zi* zz+ zs;
        xi= data.patches[i].t1x;
        yi= data.patches[i].t1y;
        zi= data.patches[i].t1z;
        data.patches[k].t1x = xi* xx+ yi* xy+ zi* xz;
        data.patches[k].t1y = xi* yx+ yi* yy+ zi* yz;
        data.patches[k].t1z = xi* zx+ yi* zy+ zi* zz;
        xi= data.patches[i].t2x;
        yi= data.patches[i].t2y;
        zi= data.patches[i].t2z;
        data.patches[k].t2x = xi* xx+ yi* xy+ zi* xz;
        data.patches[k].t2y = xi* yx+ yi* yy+ zi* yz;
        data.patches[k].t2z = xi* zx+ yi* zy+ zi* zz;
        data.patches[k].psalp = data.patches[i].psalp;
        data.patches[k].pbi = data.patches[i].pbi;
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
  double s1x=0.0, s1y=0.0, s1z=0.0;
  double s2x=0.0, s2y=0.0, s2z=0.0, xst;
  double znv, xnv, ynv, xa, xn2, yn2;
  double zn2;

  /* new patches.  for nx=0, ny=1,2,3,4 patch is (respectively) */
  /* arbitrary, rectagular, triangular, or quadrilateral. */
  /* for nx and ny  > 0 a rectangular surface is produced with */
  /* nx by ny rectangular patches. */

  data.m++;
  mi= data.m-1;
  mem_array_realloc(&data.patches, data.m);

  if( nx > 0) ntp=2;
  else ntp= ny;

  if( ntp <= 1)
  {
    data.patches[mi].px = ax1;
    data.patches[mi].py = ay1;
    data.patches[mi].pz = az1;
    data.patches[mi].pbi = az2;
    znv= cos( ax2);
    xnv= znv* cos( ay2);
    ynv= znv* sin( ay2);
    znv= sin( ax2);
    xa= sqrt( xnv* xnv+ ynv* ynv);

    if( xa >= 1.0e-6)
    {
      data.patches[mi].t1x = -ynv/ xa;
      data.patches[mi].t1y = xnv/ xa;
      data.patches[mi].t1z=0.0;
    }
    else
    {
      data.patches[mi].t1x=1.0;
      data.patches[mi].t1y=0.0;
      data.patches[mi].t1z=0.0;
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
    data.patches[mi].t1x = s1x/ xst;
    data.patches[mi].t1y = s1y/ xst;
    data.patches[mi].t1z = s1z/ xst;

    if( ntp <= 2)
    {
      data.patches[mi].px = ax1+.5*( s1x+ s2x);
      data.patches[mi].py = ay1+.5*( s1y+ s2y);
      data.patches[mi].pz = az1+.5*( s1z+ s2z);
      data.patches[mi].pbi = xa;
    }
    else
    {
      if( ntp != 4)
      {
        data.patches[mi].px=( ax1+ ax2+ ax3)/3.0;
        data.patches[mi].py=( ay1+ ay2+ ay3)/3.0;
        data.patches[mi].pz=( az1+ az2+ az3)/3.0;
        data.patches[mi].pbi=.5* xa;
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
        data.patches[mi].px=( xa*( ax1+ ax2+ ax3) +
            xst*( ax1+ ax3+ ax4))* salpn;
        data.patches[mi].py=( xa*( ay1+ ay2+ ay3) +
            xst*( ay1+ ay3+ ay4))* salpn;
        data.patches[mi].pz=( xa*( az1+ az2+ az3) +
            xst*( az1+ az3+ az4))* salpn;
        data.patches[mi].pbi=.5*( xa+ xst);
        s1x=( xnv* xn2+ ynv* yn2+ znv* zn2)/ xst;

        if( s1x <= 0.9998)
        {
          pr_err("corners of quadrilateral: patch do not lie in a plane\n");
          Stop( ERR_OK, _("Corners of quadrilateral\n"
                "patch do not lie in a plane") );
          return( FALSE );
        }

      } /* if( ntp != 4) */

    } /* if( ntp <= 2) */

  } /* if( ntp <= 1) */

  data.patches[mi].t2x = ynv* data.patches[mi].t1z - znv* data.patches[mi].t1y;
  data.patches[mi].t2y = znv* data.patches[mi].t1x - xnv* data.patches[mi].t1z;
  data.patches[mi].t2z = xnv* data.patches[mi].t1y - ynv* data.patches[mi].t1x;
  data.patches[mi].psalp=1.0;

  if( nx != 0)
  {
    int  iy, ix;
    double xs, ys, zs, xt, yt, zt;

    data.m += nx*ny-1;
    mem_array_realloc(&data.patches, data.m);

    xn2= data.patches[mi].px - s1x- s2x;
    yn2= data.patches[mi].py - s1y- s2y;
    zn2= data.patches[mi].pz - s1z- s2z;
    xs= data.patches[mi].t1x;
    ys= data.patches[mi].t1y;
    zs= data.patches[mi].t1z;
    xt= data.patches[mi].t2x;
    yt= data.patches[mi].t2y;
    zt= data.patches[mi].t2z;

    for( iy = 0; iy < ny; iy++ )
    {
      xn2 += s2x;
      yn2 += s2y;
      zn2 += s2z;

      for( ix = 1; ix <= nx; ix++ )
      {
        xst= (double)ix;
        data.patches[mi].px = xn2+ xst* s1x;
        data.patches[mi].py = yn2+ xst* s1y;
        data.patches[mi].pz = zn2+ xst* s1z;
        data.patches[mi].pbi = xa;
        data.patches[mi].psalp=1.0;
        data.patches[mi].t1x = xs;
        data.patches[mi].t1y = ys;
        data.patches[mi].t1z = zs;
        data.patches[mi].t2x = xt;
        data.patches[mi].t2y = yt;
        data.patches[mi].t2z = zt;
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
        mem_array_realloc(&data.segments, (2 * data.n + data.m));

        for( i = 0; i < data.n; i++ )
        {
          nx= i+ data.n;
          e1= data.segments[i].z1;
          e2= data.segments[i].z2;

          if( (fabs(e1)+fabs(e2) <= 1.0e-5) || (e1*e2 < -1.0e-6) )
          {
            pr_err("geometry data error: segment %d lies in plane of symmetry\n", i + 1);
            Stop( ERR_OK, _("Geometry data error\n"
                  "Segment lies in plane of symmetry") );
            return( FALSE );
          }

          data.segments[nx].x1 = data.segments[i].x1;
          data.segments[nx].y1 = data.segments[i].y1;
          data.segments[nx].z1 = -e1;
          data.segments[nx].x2 = data.segments[i].x2;
          data.segments[nx].y2 = data.segments[i].y2;
          data.segments[nx].z2 = -e2;
          itagi= data.segments[i].itag;

          if( itagi == 0)
            data.segments[nx].itag=0;
          if( itagi != 0)
            data.segments[nx].itag = itagi+ iti;

          data.segments[nx].bi = data.segments[i].bi;

        } /* for( i = 0; i < data.n; i++ ) */

        data.n= data.n*2;
        iti= iti*2;

      } /* if( data.n > 0) */

      if( data.m > 0 )
      {
        mem_array_realloc(&data.patches, (2 * data.m));

        for( i = 0; i < data.m; i++ )
        {
          nx = i+data.m;
          if( fabs(data.patches[i].pz) <= 1.0e-10)
          {
            pr_err("geometry data error: patch %d lies in plane of symmetry\n", i + 1);
            Stop( ERR_OK, _("Geometry data error\n"
                  "Patch lies in plane of symmetry") );
            return( FALSE );
          }

          data.patches[nx].px = data.patches[i].px;
          data.patches[nx].py = data.patches[i].py;
          data.patches[nx].pz = -data.patches[i].pz;
          data.patches[nx].t1x = data.patches[i].t1x;
          data.patches[nx].t1y = data.patches[i].t1y;
          data.patches[nx].t1z = -data.patches[i].t1z;
          data.patches[nx].t2x = data.patches[i].t2x;
          data.patches[nx].t2y = data.patches[i].t2y;
          data.patches[nx].t2z = -data.patches[i].t2z;
          data.patches[nx].psalp = -data.patches[i].psalp;
          data.patches[nx].pbi = data.patches[i].pbi;
        }

        data.m= data.m*2;

      } /* if( data.m >= m2) */

    } /* if( iz != 0) */

    /* reflect along y axis */
    if( iy != 0)
    {
      if( data.n > 0)
      {
        mem_array_realloc(&data.segments, (2 * data.n));

        for( i = 0; i < data.n; i++ )
        {
          nx= i+ data.n;
          e1= data.segments[i].y1;
          e2= data.segments[i].y2;

          if( (fabs(e1)+fabs(e2) <= 1.0e-5) || (e1*e2 < -1.0e-6) )
          {
            pr_err("geometry data error: segment %d lies in plane of symmetry\n", i + 1);
            Stop( ERR_OK, _("Geometry data error\n"
                  "Segment lies in plane of symmetry") );
            return( FALSE );
          }

          data.segments[nx].x1 = data.segments[i].x1;
          data.segments[nx].y1 = -e1;
          data.segments[nx].z1 = data.segments[i].z1;
          data.segments[nx].x2 = data.segments[i].x2;
          data.segments[nx].y2 = -e2;
          data.segments[nx].z2 = data.segments[i].z2;
          itagi= data.segments[i].itag;

          if( itagi == 0)
            data.segments[nx].itag=0;
          if( itagi != 0)
            data.segments[nx].itag = itagi+ iti;

          data.segments[nx].bi = data.segments[i].bi;

        } /* for( i = n2-1; i < data.n; i++ ) */

        data.n= data.n*2;
        iti= iti*2;

      } /* if( data.n >= n2) */

      if( data.m > 0 )
      {
        mem_array_realloc(&data.patches, (2 * data.m));

        for( i = 0; i < data.m; i++ )
        {
          nx= i+data.m;
          if( fabs(data.patches[i].py) <= 1.0e-10)
          {
            pr_err("geometry data error: patch %d lies in plane of symmetry\n", i + 1);
            Stop( ERR_OK, _("Geometry data error\n"
                  "Patch lies in plane of symmetry") );
            return( FALSE );
          }

          data.patches[nx].px = data.patches[i].px;
          data.patches[nx].py = -data.patches[i].py;
          data.patches[nx].pz = data.patches[i].pz;
          data.patches[nx].t1x = data.patches[i].t1x;
          data.patches[nx].t1y = -data.patches[i].t1y;
          data.patches[nx].t1z = data.patches[i].t1z;
          data.patches[nx].t2x = data.patches[i].t2x;
          data.patches[nx].t2y = -data.patches[i].t2y;
          data.patches[nx].t2z = data.patches[i].t2z;
          data.patches[nx].psalp = -data.patches[i].psalp;
          data.patches[nx].pbi = data.patches[i].pbi;

        } /* for( i = m2; i <= data.m; i++ ) */

        data.m= data.m*2;

      } /* if( data.m >= m2) */

    } /* if( iy != 0) */

    /* reflect along x axis */
    if( ix == 0 ) return( TRUE );

    if( data.n > 0 )
    {
      mem_array_realloc(&data.segments, (2 * data.n));

      for( i = 0; i < data.n; i++ )
      {
        nx= i+ data.n;
        e1= data.segments[i].x1;
        e2= data.segments[i].x2;

        if( (fabs(e1)+fabs(e2) <= 1.0e-5) || (e1*e2 < -1.0e-6) )
        {
          pr_err("geometry data error: segment %d lies in plane of symmetry\n", i + 1);
          Stop( ERR_OK, _("Geometry data error\n"
                "Segment lies in plane of symmetry") );
          return( FALSE );
        }

        data.segments[nx].x1 = -e1;
        data.segments[nx].y1 = data.segments[i].y1;
        data.segments[nx].z1 = data.segments[i].z1;
        data.segments[nx].x2 = -e2;
        data.segments[nx].y2 = data.segments[i].y2;
        data.segments[nx].z2 = data.segments[i].z2;
        itagi= data.segments[i].itag;

        if( itagi == 0)
          data.segments[nx].itag=0;
        if( itagi != 0)
          data.segments[nx].itag = itagi+ iti;

        data.segments[nx].bi = data.segments[i].bi;
      }

      data.n= data.n*2;

    } /* if( data.n > 0) */

    if( data.m == 0 ) return( TRUE );
    mem_array_realloc(&data.patches, (2 * data.m));

    for( i = 0; i < data.m; i++ )
    {
      nx= i+data.m;
      if( fabs(data.patches[i].px) <= 1.0e-10)
      {
        pr_err("geometry data error: patch %d lies in plane of symmetry\n", i + 1);
        Stop( ERR_OK, _("Geometry data error\n"
              "Patch lies in plane of symmetry") );
        return( FALSE );
      }

      data.patches[nx].px = -data.patches[i].px;
      data.patches[nx].py = data.patches[i].py;
      data.patches[nx].pz = data.patches[i].pz;
      data.patches[nx].t1x = -data.patches[i].t1x;
      data.patches[nx].t1y = data.patches[i].t1y;
      data.patches[nx].t1z = data.patches[i].t1z;
      data.patches[nx].t2x = -data.patches[i].t2x;
      data.patches[nx].t2y = data.patches[i].t2y;
      data.patches[nx].t2z = data.patches[i].t2z;
      data.patches[nx].psalp = -data.patches[i].psalp;
      data.patches[nx].pbi = data.patches[i].pbi;
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
    mem_array_realloc(&data.segments, data.n);

    for( i = nx; i < data.n; i++ )
    {
      k= i- data.np;
      xk= data.segments[k].x1;
      yk= data.segments[k].y1;
      data.segments[i].x1 = xk* cs- yk* ss;
      data.segments[i].y1 = xk* ss+ yk* cs;
      data.segments[i].z1 = data.segments[k].z1;
      xk= data.segments[k].x2;
      yk= data.segments[k].y2;
      data.segments[i].x2 = xk* cs- yk* ss;
      data.segments[i].y2 = xk* ss+ yk* cs;
      data.segments[i].z2 = data.segments[k].z2;
      data.segments[i].bi = data.segments[k].bi;
      itagi= data.segments[k].itag;

      if( itagi == 0)
        data.segments[i].itag=0;
      if( itagi != 0)
        data.segments[i].itag = itagi+ iti;
    }

  } /* if( data.n >= n2) */

  if( data.m == 0 ) return( TRUE );

  data.m *= nop;
  nx= data.mp;
  mem_array_realloc(&data.patches, data.m);

  for( i = nx; i < data.m; i++ )
  {
    k = i-data.mp;
    xk= data.patches[k].px;
    yk= data.patches[k].py;
    data.patches[i].px = xk* cs- yk* ss;
    data.patches[i].py = xk* ss+ yk* cs;
    data.patches[i].pz = data.patches[k].pz;
    xk= data.patches[k].t1x;
    yk= data.patches[k].t1y;
    data.patches[i].t1x = xk* cs- yk* ss;
    data.patches[i].t1y = xk* ss+ yk* cs;
    data.patches[i].t1z = data.patches[k].t1z;
    xk= data.patches[k].t2x;
    yk= data.patches[k].t2y;
    data.patches[i].t2x = xk* cs- yk* ss;
    data.patches[i].t2y = xk* ss+ yk* cs;
    data.patches[i].t2z = data.patches[k].t2z;
    data.patches[i].psalp = data.patches[k].psalp;
    data.patches[i].pbi = data.patches[k].pbi;

  } /* for( i = nx; i < data.m; i++ ) */

  return( TRUE );
}

/*-----------------------------------------------------------------------*/

/* Check if any segments would connect to themselves */
static gboolean
verify_self_connections(void)
{
  gboolean retval = TRUE;
  for (int i = 0; i < data.n; i++) {
    double slen = calc_connection_threshold(data.segments[i].x1,
                                            data.segments[i].y1,
                                            data.segments[i].z1,
                                            data.segments[i].x2,
                                            data.segments[i].y2,
                                            data.segments[i].z2);
    double sep = calc_manhattan_distance(data.segments[i].x1,
                                         data.segments[i].y1,
                                         data.segments[i].z1,
                                         data.segments[i].x2,
                                         data.segments[i].y2,
                                         data.segments[i].z2);
    if (sep <= slen) {
      static int last_tag = -1;
      static int msg_count = 0;
      if (data.segments[i].itag != last_tag) {
        last_tag = data.segments[i].itag;
        msg_count = 0;
      }
      if (msg_count < 3) {
        pr_warn("tag=%d/seg=%d: endpoint distance=%.3e is too close (below connection threshold=%.3e); segment will connect to itself\n",
          data.segments[i].itag, i+1, sep, slen);
        msg_count++;
      }
      else if (msg_count == 3) {
        pr_warn("tag=%d: suppressing additional self-connection warnings\n",
                data.segments[i].itag);
        msg_count++;
      }
      retval = FALSE;
    }
  }
  return retval;
}

/* Check for segments that are too close or overlapping */
static gboolean
verify_segment_overlaps(void)
{
  gboolean retval = TRUE;
  for (int i = 0; i < data.n; i++) {
    for (int j = 0; j < data.n; j++) {
      if (i == j) continue;
      
      /* Check if segments connect at endpoints */
      double slen = calc_connection_threshold(data.segments[i].x1,
                                              data.segments[i].y1,
                                              data.segments[i].z1,
                                              data.segments[i].x2,
                                              data.segments[i].y2,
                                              data.segments[i].z2);
      if (points_would_connect(data.segments[i].x1, data.segments[i].y1, data.segments[i].z1,
                               data.segments[j].x1, data.segments[j].y1, data.segments[j].z1, slen) ||
          points_would_connect(data.segments[i].x1, data.segments[i].y1, data.segments[i].z1,
                               data.segments[j].x2, data.segments[j].y2, data.segments[j].z2, slen) ||
          points_would_connect(data.segments[i].x2, data.segments[i].y2, data.segments[i].z2,
                               data.segments[j].x1, data.segments[j].y1, data.segments[j].z1, slen) ||
          points_would_connect(data.segments[i].x2, data.segments[i].y2, data.segments[i].z2,
                               data.segments[j].x2, data.segments[j].y2, data.segments[j].z2, slen)) {
        continue;
      }

      /* Check if segments are too close along their length */
      double dx = data.segments[j].x1 - data.segments[i].x1;
      double dy = data.segments[j].y1 - data.segments[i].y1;
      double dz = data.segments[j].z1 - data.segments[i].z1;
      double mid_dist = sqrt(dx*dx + dy*dy + dz*dz);
      if (mid_dist <= SMIN * mid_dist * 2) {
        static int last_tag = -1;
        static int msg_count = 0;
        if (data.segments[i].itag != last_tag) {
          last_tag = data.segments[i].itag;
          msg_count = 0;
        }
          if (msg_count < 3) {
            pr_warn("tag=%d/seg=%d: distance=%.3e to tag=%d/seg=%d closer than 2x connection threshold %.3e; unintended connections possible\n",
              data.segments[i].itag, i+1, mid_dist, data.segments[j].itag,
              j+1, SMIN * mid_dist * 2);
            msg_count++;
          }
          else if (msg_count == 3) {
            pr_warn("tag=%d: suppressing additional overlap warnings\n",
                    data.segments[i].itag);
            msg_count++;
          }
        retval = FALSE;
        break;
      }
    }
  }
  return retval;
}

/* Check for segments with very different lengths that could cause connection issues */
static gboolean
verify_relative_lengths(void)
{
  gboolean retval = TRUE;
  for (int i = 0; i < data.n; i++) {
    double xi = data.segments[i].x2 - data.segments[i].x1;
    double yi = data.segments[i].y2 - data.segments[i].y1;
    double zi = data.segments[i].z2 - data.segments[i].z1;
    double seg_len = sqrt(xi*xi + yi*yi + zi*zi);
    
    for (int j = 0; j < data.n; j++) {
      if (i == j) continue;
      double xj = data.segments[j].x2 - data.segments[j].x1;
      double yj = data.segments[j].y2 - data.segments[j].y1;
      double zj = data.segments[j].z2 - data.segments[j].z1;
      double other_len = sqrt(xj*xj + yj*yj + zj*zj);
      if (seg_len < other_len * 0.01) {
        /* Calculate connection thresholds for both segments */
        double slen_i = calc_connection_threshold(data.segments[i].x1,
                                                  data.segments[i].y1,
                                                  data.segments[i].z1,
                                                  data.segments[i].x2,
                                                  data.segments[i].y2,
                                                  data.segments[i].z2);
        double slen_j = calc_connection_threshold(data.segments[j].x1,
                                                  data.segments[j].y1,
                                                  data.segments[j].z1,
                                                  data.segments[j].x2,
                                                  data.segments[j].y2,
                                                  data.segments[j].z2);
        
        /* Check if any endpoint of segment i is within either threshold of any endpoint of segment j */
        if ((calc_manhattan_distance(data.segments[i].x1, data.segments[i].y1, data.segments[i].z1,
                                     data.segments[j].x1, data.segments[j].y1, data.segments[j].z1) <= slen_i) ||
            (calc_manhattan_distance(data.segments[i].x1, data.segments[i].y1, data.segments[i].z1,
                                     data.segments[j].x2, data.segments[j].y2, data.segments[j].z2) <= slen_i) ||
            (calc_manhattan_distance(data.segments[i].x2, data.segments[i].y2, data.segments[i].z2,
                                     data.segments[j].x1, data.segments[j].y1, data.segments[j].z1) <= slen_i) ||
            (calc_manhattan_distance(data.segments[i].x2, data.segments[i].y2, data.segments[i].z2,
                                     data.segments[j].x2, data.segments[j].y2, data.segments[j].z2) <= slen_i) ||
            (calc_manhattan_distance(data.segments[i].x1, data.segments[i].y1, data.segments[i].z1,
                                     data.segments[j].x1, data.segments[j].y1, data.segments[j].z1) <= slen_j) ||
            (calc_manhattan_distance(data.segments[i].x1, data.segments[i].y1, data.segments[i].z1,
                                     data.segments[j].x2, data.segments[j].y2, data.segments[j].z2) <= slen_j) ||
            (calc_manhattan_distance(data.segments[i].x2, data.segments[i].y2, data.segments[i].z2,
                                     data.segments[j].x1, data.segments[j].y1, data.segments[j].z1) <= slen_j) ||
            (calc_manhattan_distance(data.segments[i].x2, data.segments[i].y2, data.segments[i].z2,
                                     data.segments[j].x2, data.segments[j].y2, data.segments[j].z2) <= slen_j)) {
          
          static int last_tag = -1;
          static int msg_count = 0;
          if (data.segments[i].itag != last_tag) {
            last_tag = data.segments[i].itag;
            msg_count = 0;
          }
          if (msg_count < 3) {
            pr_warn("tag=%d/seg=%d: length=%.3e is too short (and close enough) to tag=%d/seg=%d length=%.3e that connection detection may fail or short\n",
              data.segments[i].itag, i+1, seg_len, data.segments[j].itag, j+1,
              other_len);
            msg_count++;
          }
          else if (msg_count == 3) {
            pr_warn("tag=%d: suppressing additional connection detection may fail or short warnings\n",
                    data.segments[i].itag);
            msg_count++;
          }
          retval = FALSE;
          break;
        }
      }
    }
  }
  return retval;
}

/* Check segment lengths relative to wavelength */
static gboolean
verify_wavelength_limits(double wavelength)
{
  gboolean retval = TRUE;
  for (int i = 0; i < data.n; i++) {
    double xi = data.segments[i].x2 - data.segments[i].x1;
    double yi = data.segments[i].y2 - data.segments[i].y1;
    double zi = data.segments[i].z2 - data.segments[i].z1;
    double seg_len = sqrt(xi*xi + yi*yi + zi*zi);
    double seg_lambda = seg_len / wavelength;

    if (seg_lambda < 0.001) {
      static int last_tag_short = -1;
      static int msg_count_short = 0;
      if (data.segments[i].itag != last_tag_short) {
        last_tag_short = data.segments[i].itag;
        msg_count_short = 0;
      }
      if (msg_count_short < 3) {
        pr_err("tag=%d/seg=%d: length/wavelength ratio of %.3f < 0.001; similarity of current components leads to numerical inaccuracies (min=0.001, fine=0.05, recommended <= 0.1)\n",
          data.segments[i].itag, i+1, seg_lambda);
        msg_count_short++;
      }
      else if (msg_count_short == 3) {
        pr_err("tag=%d: suppressing additional short wavelength ratio errors\n",
               data.segments[i].itag);
        msg_count_short++;
      }
      retval = FALSE;
    }
    else if (seg_lambda > 0.1) {
      static int last_tag_long = -1;
      static int msg_count_long = 0;
      if (data.segments[i].itag != last_tag_long) {
        last_tag_long = data.segments[i].itag;
        msg_count_long = 0;
      }
      if (msg_count_long < 3) {
        pr_warn("tag=%d/seg=%d: length/wavelength ratio of %.3f > 0.1; current sampling resolution limited by segment spacing (min=0.001, fine=0.05, recommended <= 0.1)\n",
          data.segments[i].itag, i+1, seg_lambda);
        msg_count_long++;
      }
      else if (msg_count_long == 3) {
        pr_warn("tag=%d: suppressing additional long wavelength ratio warnings\n",
                data.segments[i].itag);
        msg_count_long++;
      }
      retval = FALSE;
    }
    else if (seg_lambda > 0.05) {
      static int last_tag_info = -1;
      static int msg_count_info = 0;
      if (data.segments[i].itag != last_tag_info) {
        last_tag_info = data.segments[i].itag;
        msg_count_info = 0;
      }
      if (msg_count_info < 3) {
        pr_info("tag=%d/seg=%d: length/wavelength ratio of %.3f > 0.05; shorter segments increase accuracy in regions of rapid current change (min=0.001, fine=0.05, recommended <=0.1)\n",
          data.segments[i].itag, i+1, seg_lambda);
        msg_count_info++;
      }
      else if (msg_count_info == 3) {
        pr_info("tag=%d: suppressing additional accuracy improvement suggestions\n",
                data.segments[i].itag);
        msg_count_info++;
      }
    }
  }
  return retval;
}

/* Check segment length/radius ratios */
static gboolean
verify_kernel_limits(void)
{
  gboolean retval = TRUE;
  for (int i = 0; i < data.n; i++) {
    double xi = data.segments[i].x2 - data.segments[i].x1;
    double yi = data.segments[i].y2 - data.segments[i].y1;
    double zi = data.segments[i].z2 - data.segments[i].z1;
    double seg_len = sqrt(xi*xi + yi*yi + zi*zi);
    double delta_a = seg_len / data.segments[i].bi;

    if (!calc_data.iexk)
    {
      if (delta_a < 2.0) {
        static int last_tag_std_err = -1;
        static int msg_count_std_err = 0;
        if (data.segments[i].itag != last_tag_std_err) {
          last_tag_std_err = data.segments[i].itag;
          msg_count_std_err = 0;
        }
          if (msg_count_std_err < 3) {
            pr_err("tag=%d/seg=%d: length/radius ratio of %.1f < 2.0; standard thin-wire kernel field error exceeds 1%% by a large margin (min=2.0, 1%%accurate=8.0)\n",
              data.segments[i].itag, i+1, delta_a);
            msg_count_std_err++;
          }
          else if (msg_count_std_err == 3) {
            pr_err("tag=%d: suppressing additional standard kernel ratio errors\n",
                   data.segments[i].itag);
            msg_count_std_err++;
          }
        retval = FALSE;
      }
      else if (delta_a < 8.0) {
        static int last_tag_std_warn = -1;
        static int msg_count_std_warn = 0;
        if (data.segments[i].itag != last_tag_std_warn) {
          last_tag_std_warn = data.segments[i].itag;
          msg_count_std_warn = 0;
        }
          if (msg_count_std_warn < 3) {
            pr_warn("tag=%d/seg=%d: length/radius ratio of %.1f < 8.0; standard thin-wire kernel field error exceeds 1%% (min=2.0, 1%%accurate=8.0)\n",
              data.segments[i].itag, i+1, delta_a);
            msg_count_std_warn++;
          }
          else if (msg_count_std_warn == 3) {
            pr_warn("tag=%d: suppressing additional standard kernel ratio warnings\n",
                    data.segments[i].itag);
            msg_count_std_warn++;
          }
      }
    }
    else
    {
      if (delta_a < 0.5) {
        static int last_tag_ext_err = -1;
        static int msg_count_ext_err = 0;
        if (data.segments[i].itag != last_tag_ext_err) {
          last_tag_ext_err = data.segments[i].itag;
          msg_count_ext_err = 0;
        }
          if (msg_count_ext_err < 3) {
            pr_err("tag=%d/seg=%d: length/radius ratio of %.1f < 0.5; extended thin-wire kernel field error exceeds 1%% by a large margin (min=0.5, 1%%accurate=2.0)\n",
              data.segments[i].itag, i+1, delta_a);
            msg_count_ext_err++;
          }
          else if (msg_count_ext_err == 3) {
            pr_err("tag=%d: suppressing additional extended kernel ratio errors\n",
                   data.segments[i].itag);
            msg_count_ext_err++;
          }
        retval = FALSE;
      }
      else if (delta_a < 2.0) {
        static int last_tag_ext_warn = -1;
        static int msg_count_ext_warn = 0;
        if (data.segments[i].itag != last_tag_ext_warn) {
          last_tag_ext_warn = data.segments[i].itag;
          msg_count_ext_warn = 0;
        }
          if (msg_count_ext_warn < 3) {
            pr_warn("tag=%d/seg=%d: length/radius ratio of %.1f < 2.0; extended thin-wire kernel field error exceeds 1%% (min=0.5, 1%%accurate=2.0)\n",
              data.segments[i].itag, i+1, delta_a);
            msg_count_ext_warn++;
          }
          else if (msg_count_ext_warn == 3) {
            pr_warn("tag=%d: suppressing additional extended kernel ratio warnings\n",
                    data.segments[i].itag);
            msg_count_ext_warn++;
          }
      }
    }
  }
  return retval;
}

/* Main verification function that calls individual test functions */
  gboolean verify_segments(void)
{
  gboolean retval = TRUE;
  retval &= verify_self_connections();

  if (data.n > SEGMENT_OVERLAP_THRESHOLD && !rc_config.force_verify_segments) {
    pr_warn("Skipping overlap check for %d segments (use --force-verify to enable)\n", data.n);
  }
  else {
    if (data.n > SEGMENT_OVERLAP_THRESHOLD && rc_config.force_verify_segments) {
      pr_info("Forcing overlap check for %d segments\n", data.n);
    }
    retval &= verify_segment_overlaps();
  }

  retval &= verify_relative_lengths();

  /* Check kernel limits - independent of frequency */
  retval &= verify_kernel_limits();

  if (calc_data.FR_cards == 0) {
    pr_debug("verify_segments() skipping frequency-dependent checks - no frequency data\n");
    return retval;
  }

  /* Get shortest wavelength from highest frequency */
  double wavelength = CVEL;
  for (int i = 0; i < calc_data.FR_cards; i++) {
    double max_freq = calc_data.freq_loop_data[i].max_freq;
    if (max_freq > 0.0)
      wavelength = fmin(wavelength, CVEL/(max_freq * 1e6));
  }

  retval &= verify_wavelength_limits(wavelength);

  return retval;
}

/* subroutine wire generates segment geometry */
/* data for a straight wire of ns segments. */
  void
wire( double xw1, double yw1, double zw1,
    double xw2, double yw2, double zw2, double rad,
    double rdel, double rrad, int ns, int itg )
{
  int ist, i;
  double xd, yd, zd, delz, rd, fns, radz;
  double xs1, ys1, zs1, xs2, ys2, zs2;

  if( ns < 1) return;

  ist= data.n;
  data.n= data.n+ ns;
  data.np= data.n;
  data.mp= data.m;
  data.ipsym=0;
  mem_array_realloc(&data.segments, data.n);

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
    data.segments[i].itag = itg;
    xs2= xs1+ xd* delz;
    ys2= ys1+ yd* delz;
    zs2= zs1+ zd* delz;
    data.segments[i].x1 = xs1;
    data.segments[i].y1 = ys1;
    data.segments[i].z1 = zs1;
    data.segments[i].x2 = xs2;
    data.segments[i].y2 = ys2;
    data.segments[i].z2 = zs2;
    data.segments[i].bi = radz;
    delz= delz* rd;
    radz= radz* rrad;
    xs1= xs2;
    ys1= ys2;
    zs1= zs2;
  }

  data.segments[data.n - 1].x2 = xw2;
  data.segments[data.n - 1].y2 = yw2;
  data.segments[data.n - 1].z2 = zw2;

  return;
}

/*-----------------------------------------------------------------------*/
