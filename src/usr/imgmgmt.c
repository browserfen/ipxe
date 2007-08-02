/*
 * Copyright (C) 2007 Michael Brown <mbrown@fensystems.co.uk>.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <gpxe/image.h>
#include <gpxe/downloader.h>
#include <gpxe/monojob.h>
#include <gpxe/open.h>
#include <gpxe/uri.h>
#include <usr/imgmgmt.h>

/** @file
 *
 * Image management
 *
 */

/**
 * Fetch an image
 *
 * @v uri_string	URI as a string (e.g. "http://www.nowhere.com/vmlinuz")
 * @v name		Name for image, or NULL
 * @v register_image	Image registration routine
 * @ret rc		Return status code
 */
int imgfetch ( struct image *image, const char *uri_string,
	       int ( * image_register ) ( struct image *image ) ) {
	struct uri *uri;
	int rc;

	if ( ! ( uri = parse_uri ( uri_string ) ) )
		return -ENOMEM;

	image_set_uri ( image, uri );

	if ( ( rc = create_downloader ( &monojob, image, image_register,
					LOCATION_URI, uri ) ) == 0 )
		rc = monojob_wait();

	uri_put ( uri );
	return rc;
}

/**
 * Load an image
 *
 * @v image		Image
 * @ret rc		Return status code
 */
int imgload ( struct image *image ) {
	int rc;

	/* Try to load image */
	if ( ( rc = image_autoload ( image ) ) != 0 )
		return rc;

	return 0;
}

/**
 * Execute an image
 *
 * @v image		Image
 * @ret rc		Return status code
 */
int imgexec ( struct image *image ) {
	return image_exec ( image );
}

/**
 * Identify the first loaded image
 *
 * @ret image		Image, or NULL
 */
struct image * imgautoselect ( void ) {
	struct image *image;

	for_each_image ( image ) {
		if ( image->flags & IMAGE_LOADED )
			return image;
	}

	return NULL;
}

/**
 * Display status of an image
 *
 * @v image		Executable/loadable image
 */
void imgstat ( struct image *image ) {
	printf ( "%s: %zd bytes", image->name, image->len );
	if ( image->type )
		printf ( " [%s]", image->type->name );
	if ( image->flags & IMAGE_LOADED )
		printf ( " [LOADED]" );
	if ( image->cmdline )
		printf ( " \"%s\"", image->cmdline );
	printf ( "\n" );
}

/**
 * Free an image
 *
 * @v image		Executable/loadable image
 */
void imgfree ( struct image *image ) {
	unregister_image ( image );
}
