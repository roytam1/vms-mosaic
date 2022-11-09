#ifndef __FSDITHER_H__
#define __FSDITHER_H__

extern void FS_Dither(ImageInfo *image, unsigned char *image_out,
        XColor *new_colors, int num_colors, unsigned char *alpha_in);

#endif
