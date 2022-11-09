/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%           QQQ   U   U   AAA   N   N  TTTTT  IIIII   ZZZZZ  EEEEE            %
%          Q   Q  U   U  A   A  NN  N    T      I        ZZ  E                %
%          Q   Q  U   U  AAAAA  N N N    T      I      ZZZ   EEEEE            %
%          Q  QQ  U   U  A   A  N  NN    T      I     ZZ     E                %
%           QQQQ   UUU   A   A  N   N    T    IIIII   ZZZZZ  EEEEE            %
%                                                                             %
%                                                                             %
%              Reduce the Number of Unique Colors in an Image                 %
%                                                                             %
%                                                                             %
%                                                                             %
%                           Software Design                                   %
%                             John Cristy                                     %
%                              July 1992                                      %
%                                                                             %
%  Copyright 1994 E. I. du Pont de Nemours and Company                        %
%                                                                             %
%  Permission to use, copy, modify, distribute, and sell this software and    %
%  its documentation for any purpose is hereby granted without fee,           %
%  provided that the above Copyright notice appear in all copies and that     %
%  both that Copyright notice and this permission notice appear in            %
%  supporting documentation, and that the name of E. I. du Pont de Nemours    %
%  and Company not be used in advertising or publicity pertaining to          %
%  distribution of the software without specific, written prior               %
%  permission.  E. I. du Pont de Nemours and Company makes no representations %
%  about the suitability of this software for any purpose.  It is provided    %
%  "as is" without express or implied warranty.                               %
%                                                                             %
%  E. I. du Pont de Nemours and Company disclaims all warranties with regard  %
%  to this software, including all implied warranties of merchantability      %
%  and fitness, in no event shall E. I. du Pont de Nemours and Company be     %
%  liable for any special, indirect or consequential damages or any           %
%  damages whatsoever resulting from loss of use, data or profits, whether    %
%  in an action of contract, negligence or other tortuous action, arising     %
%  out of or in connection with the use or performance of this software.      %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Realism in computer graphics typically requires using 24 bits/pixel to
%  generate an image.  Yet many graphic display devices do not contain
%  the amount of memory necessary to match the spatial and color
%  resolution of the human eye.  The QUANTIZE program takes a 24 bit
%  image and reduces the number of colors so it can be displayed on
%  raster devices with less bits per pixel.  In most instances, the
%  quantized image closely resembles the original reference image.
%
%  A reduction of colors in an image is also desirable for image
%  transmission and real-time animation.
%
%  Function Quantize takes a standard RGB or monochrome images and quantizes
%  them down to some fixed number of colors.
%
%  For purposes of color allocation, an image is a set of n pixels, where
%  each pixel is a point in RGB space.  RGB space is a 3-dimensional
%  vector space, and each pixel, pi, is defined by an ordered triple of
%  red, green, and blue coordinates, (ri, gi, bi).
%
%  Each primary color component (red, green, or blue) represents an
%  intensity which varies linearly from 0 to a maximum value, cmax, which
%  corresponds to full saturation of that color.  Color allocation is
%  defined over a domain consisting of the cube in RGB space with
%  opposite vertices at (0,0,0) and (cmax,cmax,cmax).  QUANTIZE requires
%  cmax = 255.
%
%  The algorithm maps this domain onto a tree in which each node
%  represents a cube within that domain.  In the following discussion
%  these cubes are defined by the coordinate of two opposite vertices:
%  the vertex nearest the origin in RGB space and the vertex farthest
%  from the origin.
%
%  The tree's root node represents the the entire domain, (0,0,0) through
%  (cmax,cmax,cmax).  Each lower level in the tree is generated by
%  subdividing one node's cube into eight smaller cubes of equal size.
%  This corresponds to bisecting the parent cube with planes passing
%  through the midpoints of each edge.
%
%  The basic algorithm operates in three phases: Classification,
%  Reduction, and Assignment.  Classification builds a color
%  description tree for the image.  Reduction collapses the tree until
%  it represents, at most, the number of colors desired in the output
%  image.  Assignment defines the output image's color map and sets
%  each pixel's color by reclassification in the reduced tree.
%
%  Classification begins by initializing a color description tree of
%  sufficient depth to represent each possible input color in a leaf.
%  However, it is impractical to generate a fully-formed color
%  description tree in the classification phase for realistic values of
%  cmax.  If color components in the input image are quantized to k-bit
%  precision, so that cmax = 2k-1, the tree would need k levels below the
%  root node to allow representing each possible input color in a leaf.
%  This becomes prohibitive because the tree's total number of nodes is
%  1 + sum(i=1, k, 8k).
%
%  A complete tree would require 19,173,961 nodes for k = 8, cmax = 255.
%  Therefore, to avoid building a fully populated tree, QUANTIZE: (1)
%  Initializes data structures for nodes only as they are needed; (2)
%  Chooses a maximum depth for the tree as a function of the desired
%  number of colors in the output image (currently log2(colormap size)).
%
%  For each pixel in the input image, classification scans downward from
%  the root of the color description tree.  At each level of the tree it
%  identifies the single node which represents a cube in RGB space
%  containing the pixel's color.  It updates the following data for each
%  such node:
%
%    n1 : Number of pixels whose color is contained in the RGB cube
%    which this node represents;
%
%    n2 : Number of pixels whose color is not represented in a node at
%    lower depth in the tree;  initially, n2 = 0 for all nodes except
%    leaves of the tree.
%
%    Sr, Sg, Sb : Sums of the red, green, and blue component values for
%    all pixels not classified at a lower depth.  The combination of
%    these sums and n2 will ultimately characterize the mean color of a
%    set of pixels represented by this node.
%
%  Reduction repeatedly prunes the tree until the number of nodes with
%  n2 > 0 is less than or equal to the maximum number of colors allowed
%  in the output image.  On any given iteration over the tree, it selects
%  those nodes whose n1 count is minimal for pruning and merges their
%  color statistics upward.  It uses a pruning threshold, ns, to govern
%  node selection as follows:
%
%    ns = 0
%    while number of nodes with (n2 > 0) > required maximum number of colors
%      prune all nodes such that n1 <= ns
%      Set ns to minimum n1 in remaining nodes
%
%  When a node to be pruned has offspring, the pruning procedure invokes
%  itself recursively in order to prune the tree from the leaves upward.
%  n2, Sr, Sg, and Sb in a node being pruned are always added to the
%  corresponding data in that node's parent.  This retains the pruned
%  node's color characteristics for later averaging.
%
%  For each node, n2 pixels exist for which that node represents the
%  smallest volume in RGB space containing those pixel's colors.  When n2
%  > 0 the node will uniquely define a color in the output image.  At the
%  beginning of reduction,  n2 = 0  for all nodes except a the leaves of
%  the tree which represent colors present in the input image.
%
%  The other pixel count, n1, indicates the total number of colors
%  within the cubic volume which the node represents.  This includes n1 -
%  n2 pixels whose colors should be defined by nodes at a lower level in
%  the tree.
%
%  Assignment generates the output image from the pruned tree.  The
%  output image consists of two parts: (1)  A color map, which is an
%  array of color descriptions (RGB triples) for each color present in
%  the output image; (2)  A pixel array, which represents each pixel as
%  an index into the color map array.
%
%  First, the assignment phase makes one pass over the pruned color
%  description tree to establish the image's color map.  For each node
%  with n2  > 0, it divides Sr, Sg, and Sb by n2 .  This produces the
%  mean color of all pixels that classify no lower than this node.  Each
%  of these colors becomes an entry in the color map.
%
%  Finally, the assignment phase reclassifies each pixel in the pruned
%  tree to identify the deepest node containing the pixel's color.  The
%  pixel's value in the pixel array becomes the index of this node's mean
%  color in the color map.
%
%  With the permission of USC Information Sciences Institute, 4676 Admiralty
%  Way, Marina del Rey, California  90292, this code was adapted from module
%  ALCOLS written by Paul Raveling.
%
%  The names of ISI and USC are not used in advertising or publicity
%  pertaining to distribution of the software without prior specific
%  written permission from ISI.
%
%
*/
/* Modified for use with VMS Mosaic by GEC */

/* This file is Copyright (C) 2005, 2006, 2007 - The VMS Mosaic Project */

#include "../config.h"
#include "mosaic.h"
#include "main.h"
#include "../libhtmlw/HTML.h"
#include <X11/Xlib.h>
#include "quantize.h"
#include "mo-www.h"

#ifndef DISABLE_TRACE
extern int srcTrace;
#endif

/*
  Define declarations.
*/
#define MaxNodes	266817
#define MaxTreeDepth	8	/* Log2(MaxRGB) */
#define NodesInAList	2048
#define MaxRGB		255

#define Max(x, y)  (((x) > (y)) ? (x) : (y))

#define MEMORY_FAILURE "Unable to quantize image: Memory allocation failed\n"

/*
  Structures.
*/
typedef struct _Node {
  struct _Node
    *parent,
    *child[8];
  unsigned char
    id,
    level,
    census,
    mid_red,
    mid_green,
    mid_blue;
  unsigned long
    number_colors,
    number_unique,
    total_red,
    total_green,
    total_blue;
} Node;

typedef struct _Nodes {
  Node nodes[NodesInAList];
  struct _Nodes *next;
} Nodes;

typedef struct _Cube {
  Node *root;
  XColor color, *colormap;
  unsigned int depth;
  unsigned long colors,
    pruning_threshold,
    next_pruning_threshold,
    distance,
    squares[MaxRGB + MaxRGB + 1];
  unsigned int shift[MaxTreeDepth + 1],
    nodes,
    free_nodes,
    color_number,
    num_to_reduce;
  Node *next_node;
  Nodes *node_queue;
} Cube;

/*
  Global variables.
*/
static Cube cube;

int Quantize_Found_Alpha;
int Quantize_Found_NZero_Alpha;

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  I n i t i a l i z e N o d e                                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function InitializeNode allocates memory for a new node in the color cube
%  tree and presets all fields to zero.
%
%  The format of the InitializeNode routine is:
%
%      node = InitializeNode(node, id, level, mid_red, mid_green, mid_blue)
%
%  A description of each parameter follows.
%
%    o node: The InitializeNode routine returns this integer address.
%
%    o id: Specifies the child number of the node.
%
%    o level: Specifies the level in the classification the node resides.
%
%    o mid_red: Specifies the mid point of the red axis for this node.
%
%    o mid_green: Specifies the mid point of the green axis for this node.
%
%    o mid_blue: Specifies the mid point of the blue axis for this node.
%
%
*/
static Node *InitializeNode(unsigned int id, unsigned int level, Node *parent,
			    unsigned int mid_red, unsigned int mid_green,
			    unsigned int mid_blue)
{
  register Node *node;

  if (!cube.free_nodes) {
    register Nodes *nodes;

    /*
      Allocate a new nodes of nodes.
    */
    nodes = (Nodes *) malloc(sizeof(Nodes));
    if (!nodes)
      return((Node *) NULL);
    nodes->next = cube.node_queue;
    cube.node_queue = nodes;
    cube.next_node = nodes->nodes;
    cube.free_nodes = NodesInAList;
  }
  cube.nodes++;
  cube.free_nodes--;
  node = cube.next_node++;
  memset(node, 0, sizeof(Node));
  /** memset does them
  for (i = 0; i < 8; i++)
    node->child[i] = (Node *) NULL;
  node->number_colors = 0;
  node->number_unique = 0;
  node->total_red = 0;
  node->total_green = 0;
  node->total_blue = 0;
  node->census = 0;
  **/
  node->parent = parent;
  node->id = id;
  node->level = level;
  node->mid_red = mid_red;
  node->mid_green = mid_green;
  node->mid_blue = mid_blue;
  return(node);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  P r u n e C h i l d                                                        %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function PruneChild deletes the given node and merges its statistics into
%  its parent.
%
%  The format of the PruneSubtree routine is:
%
%      PruneChild(node)
%
%  A description of each parameter follows.
%
%    o node: pointer to node in color cube tree that is to be pruned.
%
%
*/
static void PruneChild(register Node *node)
{
  register Node *parent = node->parent;

  /*
    Merge color statistics into parent.
  */
  parent->census &= ~(1 << node->id);
  parent->number_unique += node->number_unique;
  parent->total_red += node->total_red;
  parent->total_green += node->total_green;
  parent->total_blue += node->total_blue;
  cube.nodes--;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  P r u n e L e v e l                                                        %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Procedure PruneLevel deletes all nodes at the bottom level of the color
%  tree merging their color statistics into their parent node.
%
%  The format of the PruneLevel routine is:
%
%      PruneLevel(node)
%
%  A description of each parameter follows.
%
%    o node: pointer to node in color cube tree that is to be pruned.
%
%
*/
static void PruneLevel(register Node *node)
{
  /*
    Traverse any children.
  */
  if (node->census) {
    register int id;

    for (id = 0; id < 8; id++) {
      if (node->census & (1 << id))
        PruneLevel(node->child[id]);
    }
  }
  if (node->level == cube.depth)
    PruneChild(node);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  C l o s e s t C o l o r                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Procedure ClosestColor traverses the color cube tree at a particular node
%  and determines which colormap entry best represents the input color.
%
%  The format of the ClosestColor routine is:
%
%      ClosestColor(node)
%
%  A description of each parameter follows.
%
%    o node: The address of a structure of type Node which points to a
%      node in the color cube tree that is to be pruned.
%
%
*/
static void ClosestColor(register Node *node)
{
  /*
    Traverse any children.
  */
  if (node->census) {
    register unsigned int id;

    for (id = 0; id < 8; id++) {
      if (node->census & (1 << id))
        ClosestColor(node->child[id]);
    }
  }
  if (node->number_unique) {
    register XColor *color;
    register unsigned int blue_distance, green_distance, red_distance;
    register unsigned long distance;

    /*
      Determine if this color is "closest".
    */
    color = cube.colormap + node->number_colors;
    red_distance = color->red - cube.color.red + MaxRGB;
    green_distance = color->green - cube.color.green + MaxRGB;
    blue_distance = color->blue - cube.color.blue + MaxRGB;
    distance = cube.squares[red_distance] + cube.squares[green_distance] +
               cube.squares[blue_distance];
    if (distance < cube.distance) {
      cube.distance = distance;
      cube.color_number = (unsigned short) node->number_colors;
    }
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  C o l o r M a p                                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Procedure ColorMap traverses the color cube tree and notes each colormap
%  entry.  A colormap entry is any node in the color cube tree where the
%  number of unique colors is not zero.
%
%  The format of the Colormap routine is:
%
%      ColorMap(node)
%
%  A description of each parameter follows.
%
%    o node: The address of a structure of type Node which points to a
%      node in the color cube tree that is to be pruned.
%
%
*/
static void ColorMap(register Node *node)
{
  /*
    Traverse any children.
  */
  if (node->census) {
    register unsigned int id;

    for (id = 0; id < 8; id++) {
      if (node->census & (1 << id))
        ColorMap(node->child[id]);
    }
  }
  if (node->number_unique) {
    unsigned long num = node->number_unique >> 1;

    /*
      Colormap entry is defined by the mean color in this cube.
    */
    cube.colormap[cube.colors].red =
				(node->total_red + num) / node->number_unique;
    cube.colormap[cube.colors].green =
				(node->total_green + num) / node->number_unique;
    cube.colormap[cube.colors].blue =
				(node->total_blue + num) / node->number_unique;
    cube.colormap[cube.colors].pixel = cube.colors;
    cube.colormap[cube.colors].flags = DoRed | DoGreen | DoBlue;
    node->number_colors = cube.colors++;
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  D i t h e r I m a g e                                                      %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Procedure DitherImage uses the Floyd-Steinberg algorithm to dither the
%  image.  Refer to "An Adaptive Algorithm for Spatial GreySscale", Robert W.
%  Floyd and Louis Steinberg, Proceedings of the S.I.D., Volume 17(2), 1976.
%
%  First find the closest representation to the reference pixel color in the
%  colormap, the node pixel is assigned this color.  Next, the colormap color
%  is subtracted from the reference pixels color, this represents the
%  quantization error.  Various amounts of this error are added to the pixels
%  ahead and below the node pixel to correct for this error.  The error
%  proportions are:
%
%            P     7/16
%      3/16  5/16  1/16
%
%  The error is distributed left-to-right for even scanlines and right-to-left
%  for odd scanlines.
%
%  The format of the DitherImage routine is:
%
%      newimage = DitherImage(image, width, height, alpha)
%
%  A description of each parameter follows.
%
%    o image: Specifies a pointer to 24 bit image data
%
%    o width: The image width
%
%    o height: The image height
%
%    o alpha: RGB or RGBA image
%
*/
static unsigned char *DitherImage(unsigned char *image, unsigned int width,
				  unsigned int height, int alpha)
{
  typedef struct _ErrorPacket {
    int red, green, blue;
  } ErrorPacket;

  ErrorPacket *error;
  int cache[1 << 18];
  int step, qstep;
  int pixel = 3 + (alpha ? 1 : 0);
  Node *node;
  int elimit[MaxRGB * 2 + 1];
  unsigned char rlimit[3 * (MaxRGB + 1)];
  register int i, blue_error, green_error, red_error;
  register int *error_limit = elimit;
  register ErrorPacket *cs, *ns;
  register unsigned char *q, *p;
  register unsigned char *range_limit = rlimit;
  unsigned char *newimage;
  unsigned char blue, green, red;
  unsigned int id, quantum, x, y;
  unsigned short index;

  /*
    Allocate memory.
  */
  error = (ErrorPacket *)calloc((width + 2) << 1, sizeof(ErrorPacket));
  newimage = (unsigned char *)malloc(width * height);

  if (!error || !newimage) {
    fprintf(stderr, "Unable to dither image: Memory allocation failed\n");
    if (error)
	free(error);
    if (newimage)
	free(newimage);
    return(NULL);
  }
  /*
    Initialize color cache.
  */
  memset(cache, 255, (1 << 18) * sizeof(int));
  /*
    Initialize error tables.
  */
  /** calloc does it
  for (i = 0; i < ((width + 2) << 1); i++)
    error[i].red = error[i].green =  error[i].blue = 0;
  **/
  /*
    Initialize error limit (constrain error).
  */
  quantum = Max(cube.colors >> 4, 1);
  error_limit += MaxRGB;
  step = 0;
  for (i = 0; i < ((MaxRGB + 1) / quantum); i++) {
    error_limit[i] = step;
    error_limit[-i] = (-step);
    step++;
  }
  if (quantum > 3) {
    for ( ; i < (3 * (MaxRGB + 1) / quantum); i++) {
      error_limit[i] = step;
      error_limit[-i] = (-step);
      step += (i & 0x01) ? 0 : 1;
    }
  }
  for ( ; i <= MaxRGB; i++) {
    error_limit[i] = step;
    error_limit[-i] = (-step);
  }
  /*
    Initialize range tables.
  */
  for (i = 0; i <= MaxRGB; i++) {
    range_limit[i] = 0;
    range_limit[i + MaxRGB + 1] = (unsigned char) i;
    range_limit[i + (MaxRGB + 1) * 2] = MaxRGB;
  }
  range_limit += MaxRGB + 1;
  /*
    Dither image.
  */
  for (y = 0; y < height; y++) {
    q = image + width * y * pixel;
    p = newimage + width * y;
    step = y & 0x01 ? -1 : 1;
    qstep = step * pixel;
    if (step < 0) {
      /*
        Distribute error right-to-left for odd scanlines.
      */
      q += (width - 1) * pixel;
      p += width - 1;
      ns = error + width;
      cs = ns + width + 2;
    } else {
      cs = error + 1;
      ns = cs + width + 2;
    }
    for (x = 0; x < width; x++) {
      red_error = error_limit[(cs->red + 8) / 16];
      green_error = error_limit[(cs->green + 8) / 16];
      blue_error = error_limit[(cs->blue + 8) / 16];
      red = range_limit[(int) *q + red_error];
      green = range_limit[(int) *(q + 1) + green_error];
      blue = range_limit[(int) *(q + 2) + blue_error];
      i = (blue >> 2) << 12 | (green >> 2) << 6 | red >> 2;
      if (cache[i] < 0) {
          /*
            Identify the deepest node containing the pixel's color.
          */
          node = cube.root;
          for ( ; ; ) {
            id = (red > node->mid_red ? 1 : 0) |
                 (green > node->mid_green ? 1 : 0) << 1 |
                 (blue > node->mid_blue ? 1 : 0) << 2;
            if ((node->census & (1 << id)) == 0)
              break;
            node = node->child[id];
          } 
          /*
            Find closest color among siblings and their children.
          */
          cube.color.red = red;
          cube.color.green = green;
          cube.color.blue = blue;
          cube.distance = (unsigned long) (~0);
          ClosestColor(node->parent);
          cache[i] = cube.color_number;
      }
      index = (unsigned short)cache[i];
      red_error = (int)red - cube.colormap[index].red;
      green_error = (int)green - cube.colormap[index].green;
      blue_error = (int)blue - cube.colormap[index].blue;
      *p = index;
      p += step;
      q += qstep;
      /*
        Propagate the error in these proportions:
                Q     7/16
          3/16  5/16  1/16
      */
      cs->red = cs->green = cs->blue = 0;
      cs += step;
      cs->red += 7 * red_error;
      cs->green += 7 * green_error;
      cs->blue += 7 * blue_error;
      ns -= step;
      ns->red += 3 * red_error;
      ns->green += 3 * green_error;
      ns->blue += 3 * blue_error;
      ns += step;
      ns->red += 5 * red_error;
      ns->green += 5 * green_error;
      ns->blue += 5 * blue_error;
      ns += step;
      ns->red += red_error;
      ns->green += green_error;
      ns->blue += blue_error;
    }
  }
  /*
    Free up memory.
  */
  free((char *)error);

  return(newimage);
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  A s s i g n m e n t                                                        %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Procedure Assignment generates the output image from the pruned tree.
%  The output image consists of two parts: (1)  A color map, which is an
%  array of color descriptions (RGB triples) for each color present in
%  the output image; (2)  A pixel array, which represents each pixel as
%  an index into the color map array.
%
%  First, the assignment phase makes one pass over the pruned color
%  description tree to establish the image's color map.  For each node
%  with n2  > 0, it divides Sr, Sg, and Sb by n2.  This produces the
%  mean color of all pixels that classify no lower than this node.  Each
%  of these colors becomes an entry in the color map.
%
%  Finally, the assignment phase reclassifies each pixel in the pruned
%  tree to identify the deepest node containing the pixel's color.  The
%  pixel's value in the pixel array becomes the index of this node's mean
%  color in the color map.
%
%  The format of the Assignment routine is:
%
%      newimage = Assignment(image, dither, width, height, colrs, alpha)
%
%  A description of each parameter follows.
%
%    o image: Specifies a pointer to the image data.
%
%    o dither: Set this integer value to something other than zero to
%      dither the quantized image.  The basic strategy of dithering is to
%      trade intensity resolution for spatial resolution by averaging the
%      intensities of several neighboring pixels.  Images which suffer
%      from severe contouring when quantized can be improved with the
%      technique of dithering.  Severe contouring generally occurs when
%      quantizing to very few colors, or to a poorly-chosen colormap.
%      Note, dithering is a computationally expensive process and will
%      increase processing time significantly.
%
%    o width: The image width
%
%    o height: The image height
%
%    o colrs: the X color map for the image.
%
%    o alpha: RGB or RGBA image
%
*/
static unsigned char *Assignment(unsigned char *image, unsigned int dither,
			         unsigned int width, unsigned int height,
				 XColor *colrs, int alpha)
{
  unsigned char *newimage;

  /*
    Allocate image colormap.
  */
  cube.colormap = colrs;
  cube.colors = 0;
  ColorMap(cube.root);
  /*
    Create a reduced color image.
  */
  if (dither)
    newimage = DitherImage(image, width, height, alpha);

  if (!dither || !newimage) {
    register int i;
    register Node *node;
    register unsigned char *p = image;
    register unsigned char *q;
    register unsigned int id;
    unsigned int size = width * height;

    newimage = q = malloc(size);
    if (!newimage)
      return(NULL);
    for (i = 0; i < size; i++) {
      /*
        Identify the deepest node containing the pixel's color.
      */
      node = cube.root;
      for ( ; ; ) {
        id = (*p > node->mid_red ? 1 : 0) |
             (*(p + 1) > node->mid_green ? 1 : 0) << 1 |
             (*(p + 2) > node->mid_blue ? 1 : 0) << 2;
        if ((node->census & (1 << id)) == 0)
          break;
        node = node->child[id];
      }
      /*
        Find closest color among siblings and their children.
      */
      cube.color.red = *p++;
      cube.color.green = *p++;
      cube.color.blue = *p++;
      cube.distance = (unsigned long) (~0);
      ClosestColor(node->parent);
      *q++ = cube.color_number;
      if (alpha)
	p++;
    }
  }
  return newimage;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  C l a s s i f i c a t i o n                                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Procedure Classification begins by initializing a color description tree
%  of sufficient depth to represent each possible input color in a leaf.
%  However, it is impractical to generate a fully-formed color
%  description tree in the classification phase for realistic values of
%  cmax.  If color components in the input image are quantized to k-bit
%  precision, so that cmax = 2k-1, the tree would need k levels below the
%  root node to allow representing each possible input color in a leaf.
%  This becomes prohibitive because the tree's total number of nodes is
%  1 + sum(i=1, k, 8k).
%
%  A complete tree would require 19,173,961 nodes for k = 8, cmax = 255.
%  Therefore, to avoid building a fully populated tree, QUANTIZE: (1)
%  Initializes data structures for nodes only as they are needed; (2)
%  Chooses a maximum depth for the tree as a function of the desired
%  number of colors in the output image (currently log2(colormap size)).
%
%  For each pixel in the input image, classification scans downward from
%  the root of the color description tree.  At each level of the tree it
%  identifies the single node which represents a cube in RGB space
%  containing the pixel's color.  It updates the following data for each
%  such node:
%
%    n1 : Number of pixels whose color is contained in the RGB cube
%    which this node represents;
%
%    n2 : Number of pixels whose color is not represented in a node at
%    lower depth in the tree; initially, n2 = 0 for all nodes except
%    leaves of the tree.
%
%    Sr, Sg, Sb : Sums of the red, green, and blue component values for
%    all pixels not classified at a lower depth.  The combination of
%    these sums and n2 will ultimately characterize the mean color of a
%    set of pixels represented by this node.
%
%  The format of the Classification routine is:
%
%      Classification(image, size, alpha)
%
%  A description of each parameter follows.
%
%    o image: Specifies a pointer to the image data.
%
%    o size:  Specifies number of pixels in the image.
%
%    o alpha:  Specifies if image is RGB or RGBA.
%
*/
static void Classification(unsigned char *image, unsigned int size, int alpha)
{
  register int i;
  register Node *node;
  register unsigned char *p = image;
  register unsigned int bisect, id, level;

  Quantize_Found_Alpha = Quantize_Found_NZero_Alpha = 0;

  for (i = 0; i < size; i++) {
    if (cube.nodes > MaxNodes) {
      /*
        Prune one level if the color tree is too large.
      */
      PruneLevel(cube.root);
      cube.depth--;
    }
    /*
      Start at the root and descend the color cube tree.
    */
    node = cube.root;
    for (level = 1; level <= cube.depth; level++) {
      id = (*p > node->mid_red ? 1 : 0) |
           (*(p + 1) > node->mid_green ? 1 : 0) << 1 |
           (*(p + 2) > node->mid_blue ? 1 : 0) << 2;
      if (node->child[id] == (Node *) NULL) {
        /*
          Set colors of new node to contain pixel.
        */
        node->census |= 1 << id;
        bisect = (unsigned int) (1 << (MaxTreeDepth - level)) >> 1;
        node->child[id] = InitializeNode(id, level, node,
            			  node->mid_red + (id & 1 ? bisect : -bisect),
            			  node->mid_green + (id & 2 ? bisect : -bisect),
            			  node->mid_blue + (id & 4 ? bisect : -bisect));
        if (!node->child[id]) {
          fprintf(stderr, MEMORY_FAILURE);
          mo_exit();
        }
        if (level == cube.depth)
          cube.colors++;
      }
      /*
        Record the number of colors represented by this node.  Shift by
        level in the color description tree.
      */
      node = node->child[id];
      node->number_colors += 1 << cube.shift[level];
    }
    /*
      Increment unique color count and sum RGB values for this leaf for
      later derivation of the mean cube color.
    */
    node->number_unique++;
    node->total_red += *p++;
    node->total_green += *p++;
    node->total_blue += *p++;
    if (alpha) {
      if (!Quantize_Found_NZero_Alpha && (*p != 255)) {
	Quantize_Found_Alpha = 1;
	if (*p)
	  Quantize_Found_NZero_Alpha = 1;
      }
      p++;
    }
  }
#ifndef DISABLE_TRACE
  if (srcTrace && Quantize_Found_Alpha) {
    fprintf(stderr, "[QUANTIZE] Found Alpha Channel\n");
    if (!Quantize_Found_NZero_Alpha)
      fprintf(stderr, "[QUANTIZE] Alpha Channel is all zero\n");
  }
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  I n i t i a l i z e C u b e                                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function InitializeCube initialize the Cube data structure.
%
%  The format of the InitializeCube routine is:
%
%      InitializeCube(number_pixels, number_colors)
%
%  A description of each parameter follows.
%
%    o number_pixels: Specifies the number of individual pixels in the image.
%
%    o number_colors: This integer value indicates the maximum number of
%      colors in the quantized image or colormap.  Here we use this value
%      to determine the depth of the color description tree.
%
*/
static void InitializeCube(unsigned int number_pixels,
			   unsigned int number_colors)
{
  char c = 1;
  register int i;
  unsigned int bits, level, max_shift;
  static int log4[6] = {4, 16, 64, 256, 1024, ~0};

  /*
    Initialize tree to describe color cube.  Depth is: Log4(colormap size) + 2;
  */
  cube.node_queue = (Nodes *) NULL;
  cube.nodes = 0;
  cube.free_nodes = 0;
  cube.depth = 2;
  for (i = 0; i < 6; i++) {
    if (number_colors <= log4[i])
      break;
    cube.depth = i + 2;
  }
  /*
    Initialize the shift values.
  */
  for (bits = 0; c; bits++)
    c <<= 1;
  for (max_shift = sizeof(unsigned int) * bits; number_pixels; max_shift--)
    number_pixels >>= 1;
  for (level = 0; level <= cube.depth; level++) {
    cube.shift[level] = max_shift;
    if (max_shift)
      max_shift--;
  }
  /*
    Initialize root node.
  */
  cube.root = InitializeNode(0, 0, (Node *) NULL, (MaxRGB + 1) >> 1,
			     (MaxRGB + 1) >> 1, (MaxRGB + 1) >> 1);
  if (!cube.root) {
    fprintf(stderr, MEMORY_FAILURE);
    mo_exit();
  }
  cube.root->parent = cube.root;
  cube.root->number_colors = (unsigned long) (~0);
  cube.colors = 0;
  /*
    Initialize the square values.
  */
  for (i = (-MaxRGB); i <= MaxRGB; i++)
    cube.squares[i + MaxRGB] = i * i;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e d u c e                                                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function Reduce traverses the color cube tree and prunes any node whose
%  number of colors fall below a particular threshold.
%
%  The format of the Reduce routine is:
%
%      Reduce(node)
%
%  A description of each parameter follows.
%
%    o node: pointer to node in color cube tree that is to be pruned.
%
%
*/
static void Reduce(register Node *node)
{
  /*
    Traverse any children.
  */
  if (node->census) {
    register unsigned int id;

    for (id = 0; id < 8; id++) {
      if (node->census & (1 << id))
        Reduce(node->child[id]);
    }
  }

  if (cube.num_to_reduce && (node->number_colors <= cube.pruning_threshold)) {
    PruneChild(node);
    /*
      Stop reduction when pruned enough.
    */
    cube.num_to_reduce--;
  } else {
    /*
      Find minimum pruning threshold.
    */
    if (node->number_unique > 0)
      cube.colors++;
    if (node->number_colors < cube.next_pruning_threshold)
      cube.next_pruning_threshold = node->number_colors;
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e d u c t i o n                                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function Reduction repeatedly prunes the tree until the number of nodes
%  with n2 > 0 is less than or equal to the maximum number of colors allowed
%  in the output image.  On any given iteration over the tree, it selects
%  those nodes whose n1 count is minimal for pruning and merges their
%  color statistics upward.  It uses a pruning threshold, ns, to govern
%  node selection as follows:
%
%    ns = 0
%    while number of nodes with (n2 > 0) > required maximum number of colors
%      prune all nodes such that n1 <= ns
%      Set ns to minimum n1 in remaining nodes
%
%  When a node to be pruned has offspring, the pruning procedure invokes
%  itself recursively in order to prune the tree from the leaves upward.
%  n2, Sr, Sg, and Sb in a node being pruned are always added to the
%  corresponding data in that node's parent.  This retains the pruned
%  node's color characteristics for later averaging.
%
%  For each node, n2 pixels exist for which that node represents the
%  smallest volume in RGB space containing those pixel's colors.  When n2
%  > 0 the node will uniquely define a color in the output image.  At the
%  beginning of reduction,  n2 = 0  for all nodes except the leaves of
%  the tree which represent colors present in the input image.
%
%  The other pixel count, n1, indicates the total number of colors
%  within the cubic volume which the node represents.  This includes n1 -
%  n2 pixels whose colors should be defined by nodes at a lower level in
%  the tree.
%
%  The format of the Reduction routine is:
%
%      Reduction(number_colors)
%
%  A description of each parameter follows.
%
%    o number_colors: This integer value indicates the maximum number of
%      colors in the quantized image or colormap.  The actual number of
%      colors allocated to the colormap may be less than this value, but
%      never more.  Note, the number of colors is restricted to a value
%      less than or equal to 65536 if the continuous_tone parameter has a
%      value of zero.
%
%
*/
static void Reduction(unsigned int number_colors)
{
  cube.next_pruning_threshold = 1;
  while (cube.colors > number_colors) {
    cube.pruning_threshold = cube.next_pruning_threshold;
    cube.next_pruning_threshold = cube.root->number_colors - 1;
    cube.num_to_reduce = cube.colors - number_colors;
    cube.colors = 0;
    Reduce(cube.root);
  }
  return;
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  Q u a n t i z e I m a g e                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function QuantizeImage analyzes the colors within a reference image and
%  chooses a fixed number of colors to represent the image.  The goal of the
%  algorithm is to minimize the difference between the input and output image
%  while minimizing the processing time.
%
%  The format of the QuantizeImage routine is:
%
%     newimage = QuantizeImage(image, w, h, number_colors, dither, colrs, alpha)
%
%  A description of each parameter follows.
%
%    o image: Specifies a pointer to the RGB image data.
%
%    o w: The width of the image.
%
%    o h: The height of the image.
%
%    o number_colors: This integer value indicates the maximum number of
%      colors in the quantized image or colormap.  The actual number of
%      colors allocated to the colormap may be less than this value, but
%      never more.
%
%    o dither: Set this integer value to something other than zero to
%      dither the quantized image.  The basic strategy of dithering is to
%      trade intensity resolution for spatial resolution by averaging the
%      intensities of several neighboring pixels.  Images which suffer
%      from severe contouring when quantized can be improved with the
%      technique of dithering.  Severe contouring generally occurs when
%      quantizing to very few colors, or to a poorly-chosen colormap.
%      Note, dithering is a computationally expensive process and will
%      increase processing time significantly.
%
%    o colrs: the X color map for the image.
%
%    o Alpha: RGB (0) or RGBA (1) image
%
*/
unsigned char *QuantizeImage(unsigned char *image, unsigned int w,
			     unsigned int h, unsigned int number_colors,
			     unsigned int dither, XColor *colrs, int alpha) 
{
  Nodes *nodes;
  unsigned int size = w * h;
  unsigned char *newimage;
  int i;

  InitializeCube(size, number_colors);
  Classification(image, size, alpha);

  if (cube.colors > number_colors) {
      int ori_colrs = cube.colors;

      /* Too many colors, so reduce them */
      Reduction(number_colors);
#ifndef DISABLE_TRACE
      if (srcTrace)
          fprintf(stderr, "[QUANTIZE] Reduced from %d to %d colors\n",
		  ori_colrs, cube.colors);
#endif
      /* Dither below if requested */
  } else {
      /* Not reduced, so nothing to dither */
      dither = 0;
  }
  newimage = Assignment(image, dither, w, h, colrs, alpha);

  if (!newimage) {
     application_user_feedback("Image too large:  Quantization failure");
     return NULL;
  }
#ifndef DISABLE_TRACE
  if (srcTrace)
     fprintf(stderr, "[QUANTIZE] Quantized %d colors\n", cube.colors);
#endif
  /*
    Fix up the X color map
  */
  for (i = 0; i < cube.colors; i++) {
     colrs[i].red = colrs[i].red << 8;
     colrs[i].green = colrs[i].green << 8;
     colrs[i].blue = colrs[i].blue << 8;
  }
  /*
    Release color cube tree storage.
  */
  do {
    nodes = cube.node_queue->next;
    free((char *) cube.node_queue);
    cube.node_queue = nodes;
  } while (cube.node_queue);

  return newimage;
}


/* Callback for use by LIBHTMLW */
void ImageQuantize(Widget w, XtPointer clid, XtPointer calld)
{
  ImageInfo *pic = (ImageInfo *) calld;
  XColor *colrs = pic->colrs;
  unsigned char *new;

  new = QuantizeImage(pic->rgb, pic->width, pic->height, 256, 1, colrs, 0);
  if (new) {
    free(pic->rgb);
    pic->rgb = new;
    pic->num_colors = cube.colors;
  }
  return;
}
