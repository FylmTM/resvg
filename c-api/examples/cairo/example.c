#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <cairo.h>
#include <resvg.h>

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Usage:\n\texample in.svg out.png");
        abort();
    }

    resvg_init_log();

    resvg_options *opt = resvg_options_create();
    resvg_options_load_system_fonts(opt);

    resvg_render_tree *tree;
    int err = resvg_parse_tree_from_file(argv[1], opt, &tree);
    resvg_options_destroy(opt);
    if (err != RESVG_OK)
    {
        printf("Error id: %i\n", err);
        abort();
    }

    resvg_size size = resvg_get_image_size(tree);
    int width = (int)size.width;
    int height = (int)size.height;

    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);

    /* resvg doesn't support stride, so cairo_surface_t should have no padding */
    assert(cairo_image_surface_get_stride(surface) == (int)size.width * 4);

    unsigned char *surface_data = cairo_image_surface_get_data(surface);

    resvg_fit_to fit_to = { RESVG_FIT_TO_ORIGINAL, 1 };
    resvg_render(tree, fit_to, resvg_transform_identity(), width, height, (char*)surface_data);

    /* RGBA -> BGRA */
    for (int i = 0; i < width * height * 4; i += 4)
    {
        unsigned char r = surface_data[i + 0];
        surface_data[i + 0] = surface_data[i + 2];
        surface_data[i + 2] = r;
    }

    cairo_surface_write_to_png(surface, argv[2]);
    cairo_surface_destroy(surface);

    resvg_tree_destroy(tree);

    return 0;
}
