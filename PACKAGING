Hello distribution package maintainer:

First, thank you for maintaining xnec2c for your distro!

Please read through this document for ideas on how xnec2c might be
packaged.


=== Desktop Install ===
xnec2c now supports desktop integration via xdg-utils.

Note that running `make desktop-install` is intended for end-users.
Instead, as a package distributor, will want to add something
like this to your build after installing the packaged files:

    update-mime-database /usr/share/mime || :
	update-desktop-database || :
	gtk-update-icon-cache /usr/share/icons/hicolor ||

For example, see the bottom of ./Makefile.am under 'desktop-install:'
or the %post section of ./xnec2c.spec .


=== Dependencies ===
If Gnome development packages are installed then you probably have all
the "required" dependencies.  Notably:
  * gtk3 / gdk3
  * pango
  * cairo

In addition, the xnec2c package has optional linear algebra dependencies
that you can include either as "required" to directly pull them when
installing xnec2c from your package manager, or as "suggested" packages
for performance enhancement.  These linear algebra packages are supported:

   * OpenBLAS (serial, pthreads, and openmp)
   * ATLAS (serial, threaded)
   * Intel MKL (serial, GNU threads, Intel threads, TBB threads)

The package names will vary by distribution, so, if available, you
can flag these as either "required" or "suggested" dependencies of
xnec2c depending on how your distro's package installer handles this.
My suggestion would be to make these dependencies "required" to provide
the best out-of-box experience with xnec2c, but I'll leave that to
your discretion.

For technical reasons detailed below, xnec2c uses dlopen() to find
available linear algebra packages at runtime using a set of known shared
library .so names.  If a .so is found it will be tested and loaded for
use so the user can benchmark and choose the library that works best
for their use.  If a .so is not found, it will skip the .so and try a
different one.

The most common .so names are supported are as follows; if your
linear algebra libraries provide any (or all) of these below then
xnec2c will detect and use whatever is available.  If your library .so
names are not listed here then please open an issue on the Github page
(https://github.com/KJ7LNW/xnec2c) so we can add support for accelerated
linear algebra in your distribution:

	ATLAS Threaded
		libtatlas.so.3

	ATLAS, Serial
		libsatlas.so.3

	OpenBLAS+LAPACKe, Serial
		libopenblas.so
		libopenblas_serial.so.0

	OpenBLAS+LAPACKe, OpenMP
		libopenblaso.so
		libopenblas_openmp.so.0

	OpenBLAS+LAPACKe, pthreads
		libopenblasp.so
		libopenblas_pthreads.so.0

	Generic, typically managed by `alternatives` or similar:
		liblapacke.so.3
		liblapack_atlas.so.3 (threaded or serial depending on selection)

	Intel MKL (Serial, TBB Threads, Intel Threads, GNU Threads)
		libmkl_rt.so

See "File->Math Libraries->Mathlib Help"
and "File->Mathlib Benchmarks->Benchmark Help"
for usage details.

==== Technical Details from an email on December 28, 2021 ====

Tom DL1JBE at Gentoo noted 

> * The new integration for LINPACK and others is good. It would be better if
>    xnec2 would not just use what is installed by accident on the system, but
>    would allow a decision by a '--enable-xx' or '--with-xx' switch during
>    build configuration.

That was the original intention (really, we tried!); we went to
dynamic loading because linking against local .so or .a paths
makes it impossible to switch libraries after building (for some
distributions like CentOS and SUSE). The function exports across the
same class of linear algebra library implementations have the same
name so it turned into a (semi-)multual-exclusive list of options like
--enable-{atlas,openblas,cblas,intelmkl}-{pthreads,serial,openmp} which
complicated the build process and made library selection less flexible
(and not all distributions had the same mutual-exclusivity rules for
linking!).

For example, a SUSE user could build against the OpenMP version
of OpenBLAS but then never have access to the pthreads or serial
implementations without recompiling.  Some distributions use
`alternatives` or `eselect` or similar like Gentoo and Debian/Ubuntu useto
switch libraries, which is useful for advanced users.  Other distributions
name them like libopenblas_openmp.so and symlink switching isn't an
option, you just have to link against the one you want.  As it turns out,
xnec2c works best with dynamic loading because it can use a threaded
interactive BLAS implementation for fast single frequency simulation,
but a serial batch BLAS implementation for parallel simulation forked
per frequency.

One of the design goals for linear algebra integration was to provide a
runtime benchmark option across the available linear algebra libraries so
the user can find the one best for their local system (See File->Mathlib
Benchmarks) so we created our own mathlib.c linear algebra abstraction
to scan for available libraries at startup and use the selected library
with dlopen() when selected in File->Math Libraries.

Having the libraries available as different names without using
`alternatives`-style switching makes it easier to benchmark and allows
users to select the active library in xnec2c from the File menu without
restarting the program.  (It would be neat if distributions provided
symlinks for both an `alternatives`-managed implementation as well as
unique names per implementation for runtime linking.)

I encourage distribution packagers to add suggested (or required)
package dependencies for whichever .so files that could benefit
xnec2c for enabling accelerated linear algebra packages for ATLAS
and OpenBLAS/LAPACKe (so dlopen() will find them via /etc/ld.so.*
or LD_LIBRARY_PATH).  Gentoo's BLAS packages are listed here:
https://wiki.gentoo.org/wiki/BLAS_and_LAPACK_Providers .

These are the currently detected .so files, so let me know if you need
others for your distribution. See the top of src/mathlib.c for details:

	liblapack_atlas.so.3
	liblapacke.so.3
	libopenblas.so
	libopenblaso.so
	libopenblasp.so
	libopenblas_openmp.so.0
	libopenblas_pthreads.so.0
	libopenblas_serial.so.0
	libsatlas.so.3
	libtatlas.so.3

Thanks again for using and packaging xnec2c in your distribution.  Feel
free to contact me if you need anything!

Sincerely,

Eric Wheeler
KJ7LNW
