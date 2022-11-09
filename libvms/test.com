$ cc/define=test cmdline.c
$ set command/object mosaic_cld.cld
$ link cmdline,mosaic_cld
