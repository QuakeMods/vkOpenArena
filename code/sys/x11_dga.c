#include "../client/client.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <X11/extensions/Xxf86dga.h>

#include "sys_local.h"
extern glwstate_t glw_state;
extern Display *dpy;
extern int scrnum;


static void *d_lib = NULL;

Bool (*_XF86DGAQueryVersion)( Display *dpy, int *majorVersion, int *minorVersion );
Bool (*_XF86DGAQueryExtension)( Display *dpy, int *event_base, int *error_base );
Status (*_XF86DGADirectVideo)( Display *dpy, int screen, int enable );


static sym_t d_list[] =
{
	{ (void**)&_XF86DGAQueryVersion, "XF86DGAQueryVersion" },
	{ (void**)&_XF86DGAQueryExtension, "XF86DGAQueryExtension" },
	{ (void**)&_XF86DGADirectVideo, "XF86DGADirectVideo" }
};


qboolean DGA_Init( Display *_dpy )
{
	int event_base, error_base;
	int ver_major = 0, ver_minor = 0;
	int i;

	glw_state.dga_ext = qfalse;

	if ( d_lib == NULL )
	{
		d_lib = Sys_LoadDll( "libXxf86dga.so.1", qtrue );
		if ( d_lib == NULL )
		{
			d_lib = Sys_LoadDll( "libXxf86dga.so.1", qtrue );
		}
		if ( d_lib == NULL )
		{
			printf( "...error loading libXxf86dga\n" );
			goto __fail;
		}
	}

	for ( i = 0 ; i < ARRAY_LEN( d_list ); i++ )
	{
		*d_list[ i ].symbol = Sys_GetFunAddr( d_lib, d_list[ i ].name );
		if ( *d_list[ i ].symbol == NULL )
		{
			printf( "...couldn't find '%s' in libXxf86dga\n", d_list[ i ].name );
			goto __fail;
		}
	}

	dpy = _dpy;

	if ( !_XF86DGAQueryExtension( dpy, &event_base, &error_base ) || !_XF86DGAQueryVersion( dpy, &ver_major, &ver_minor ) )
	{
		printf( "...DGA extension is not available.\n" );
		goto __fail;
	}

	printf( "...DGA extension version %i.%i detected.\n", ver_major, ver_minor );

	glw_state.dga_ext = qtrue;

	return qtrue;

__fail:
	DGA_Done();
	return qfalse;
}


void DGA_Done( void )
{
	if ( d_lib )
	{
		Sys_UnloadDll( d_lib );
		d_lib = NULL;
	}
	glw_state.dga_ext = qfalse;
}

void DGA_Mouse( qboolean enable )
{
	if ( !glw_state.dga_ext )
		return;

	_XF86DGADirectVideo( dpy, DefaultScreen( dpy ), enable ? XF86DGADirectMouse : 0 );
}
