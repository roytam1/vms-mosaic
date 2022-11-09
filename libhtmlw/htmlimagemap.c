/*
 * This program was originally a server script for the DECthread HTTP_SERVER.
 * It provides support for client side image maps.
 *
 *    circle (cx,cy) r URL
 *	Return URL if point is within circle.
 *
 *    rectangle (x1,y1) (x2,y2) URL
 *	Return URL if point is within rectangle defined by any two
 *      oposing corners.
 *
 *    polygon (x1,y1) ... (xn,yn) URL
 *	Return URL if point is within polygon defined by list of
 *	points.
 *
 *  Author:	David Jones
 *  Date:	19-MAR-1994
 *  Revised:	20-JUL-1994	Add xlate operation to inp file.
 *  Revised:    27-SEP-1994	Support case sensitive paths.
 *  Revised:    13-JAN-1994	Make default work independant of position.
 *  Revised:     6-FEB-1995	Accept + as coordinate delimiter.
 *  Revised:    23-JUN-1995	Fix bugs in polygon processing.
 *  Revised:	10-JAN-1995	Declare sqrt() using math.h header file.
 *
 *  Modified for use with VMS Mosaic on 6-May-1998 by George Cook
 */ 
#include "../config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "../libhtmlw/HTMLp.h"
#include "../libhtmlw/HTMLPutil.h"

#ifndef DISABLE_TRACE
extern int htmlwTrace;
#endif

/**************************************************************************/
/* Test whether x/y point lies within polygon defined by point within
 * token list.
 */
static int check_polygon(CoordInfo *coords, int intx, int inty)
{
    int npnts, i;
    int count = 0;
#ifndef DISABLE_TRACE
    int prev = 0;
#endif
    float x1, y1, x2, y2;
    float x = intx;
    float y = inty;
    CoordInfo *coord = coords;
    CoordInfo *end;

    /*
     * Find number of points in polygon and duplicate first point
     */
    for (npnts = 0; coord->x > -1; npnts++)
	coord = coord->next;

    end = coord;
    coord->x = coords->x;
    coord->y = coords->y;
    coord = coords;
#ifndef DISABLE_TRACE
    if (htmlwTrace)
	fprintf(stderr, "Testing point (%d, %d)\n", intx, inty);
#endif
    /*
     * Count number of polygon sides that cross same y value and are
     * to left (less) than x value.  Line segment with endpoint at
     * same y as test point only counts if it is the endpoint with lower y.
     */
    for (i = 1; i <= npnts; i++) {
	/* Set vertices */
	x1 = coord->x;
	y1 = coord->y;
	coord = coord->next;
	x2 = coord->x;
	y2 = coord->y;
#ifndef DISABLE_TRACE
	if (htmlwTrace)
	    fprintf(stderr, "segment[%d]=(%f,%f)(%f,%f)\n", i, x1, y1, x2, y2);
#endif
	if (((y1 <= y) && (y2 > y)) || ((y1 > y) && (y2 <= y))) {
#ifndef DISABLE_TRACE
	    if (htmlwTrace)
	        fprintf(stderr, "   segment in proper y x range: %f %f\n",
			x1, x2);
#endif
	    /*
	     * Determine if test point is to right of line segment.
	     * Lying on a horizontal line counts as a YES since the
	     * the 2 adjacent sides will match also and cancel each other.
	     */
	    if ((x >= x1) && (x >= x2)) {
		count++;	/* Easy test */
	    } else if ((x < x1) && (x < x2)) {
		;		/* To left */
	    } else {
		/* Tougher case, find intercept position. */
		float slope = (x2 - x1) / (y2 - y1);

		x1 += slope * (y - y1);
		/* Test if we are 2 right of intercept position */
#ifndef DISABLE_TRACE
		if (htmlwTrace)
		    fprintf(stderr, "Intercept x: %f y: %f test: %f\n",
			    x1, y, x );
#endif
		if (x >= x1)
		    count++;
	    }
	} else if ((y1 == y2) && (y == y1)) {
	    /* 
	     * Edge is horizontal, see if test point on line and force
	     * match.  Otherwise endpoint test for the adjacent lines will
	     * determine crossings.
	     */
	    if (((x >= x1) && (x <= x2)) || ((x <= x1) && (x >= x2))) {
		count = 1;		/* force successfull match */
		break;
	    }
	}
#ifndef DISABLE_TRACE
	if (htmlwTrace && (count > prev)) {
	    fprintf(stderr, "    point is right of segment\n");
	    prev = count;
	}
#endif
    }
    /* Restore coord list terminator */
    end->x = -1;

#ifndef DISABLE_TRACE
    if (htmlwTrace)
        fprintf(stderr, "polygon cross count = %d", count);
#endif
    /*
     * Now we have the count, determine if we are inside the polygon.
     * We are inside if we are 2 right of an odd number of sides.
     */
    if ((count % 2) == 1)
	return 1;

    return 0;
} 

/********************************************************************/
MapInfo *FindMap(HTMLWidget hw, char *mapname)
{
    MapInfo *mptr = hw->html.map_list;

    while (mptr) {
	/* Note:  map names are case sensitive */
	/* Second check skips "#" if present */
	if (!strcmp(mptr->name, mapname) ||
	    !strcmp(mptr->name, mapname + 1)) {
#ifndef DISABLE_TRACE
	    if (htmlwTrace)
		fprintf(stderr, "Found map %s\n", mptr->name);
#endif
	    return (mptr);
	}
	mptr = mptr->next;
    }
    return (NULL);
}

AreaInfo *GetMapArea(MapInfo *map, int x, int y)
{
    AreaInfo *area = map->areaList;
    int found = 0;
    int min_x, max_x, min_y, max_y; 
    float dx, dy;
    CoordInfo *coord, *coord2;

    while (area) {
	switch (area->shape) {
	    case -1:			/* Bad area, skip it */
		break;

	    case AREA_RECT:		/* Rectangle */
		coord = area->coordList;
		coord2 = coord->next;
		min_x = coord->x < coord2->x ? coord->x : coord2->x;
		max_x = coord->x > coord2->x ? coord->x : coord2->x;
		min_y = coord->y < coord2->y ? coord->y : coord2->y;
		max_y = coord->y > coord2->y ? coord->y : coord2->y;
		if ((x >= min_x) && (x <= max_x) &&
		    (y >= min_y) && (y <= max_y))
		    found = 1;
		break;

	    case AREA_CIRCLE:		/* Circle */
		coord = area->coordList;
		coord2 = coord->next;
		dx = x - coord->x;
		dy = y - coord->y;
		if (sqrt((dx * dx) + (dy * dy)) <= coord2->x)
 		    found = 1;
		break;

	    case AREA_POLY:		/* Polygon */
		found = check_polygon(area->coordList, x, y);
		break;
	}
	if (found)
	    return (area);
	area = area->next;
    }
    return (NULL);
}
