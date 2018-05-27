#include <erl_nif.h>
#include <vips/vips.h>

static void vips_operation(unsigned char **buf, char operation[], char extension[], size_t *len_in, size_t *len_out, unsigned char **buf_out) {
    VipsImage *in = vips_image_new_from_buffer((void *)*buf, *len_in, "", NULL);
    VipsImage *out;
    VipsOperation *op;
    VipsOperation *new_op;
    GValue gvalue = { 0 };

    op = vips_operation_new(operation);

    /* Init a gvalue as an image, set it to in, use the gvalue to set the
     * operator property.
     */
    g_value_init( &gvalue, VIPS_TYPE_IMAGE );
    g_value_set_object( &gvalue, in );
    g_object_set_property( G_OBJECT( op ), "in", &gvalue );
    g_value_unset( &gvalue );

    /* We no longer need in: op will hold a ref to it as long as it needs
     * it. 
     */
    g_object_unref( in ); 


        /* Call the operation. This will look up the operation+args in the vips
     * operation cache and either return a previous operation, or build
     * this one. In either case, we have a new ref we mst release.
     */
    if( !(new_op = vips_cache_operation_build( op )) ) {
        g_object_unref( op );
        vips_error_exit( NULL ); 
    }
    g_object_unref( op );
    op = new_op;

    /* Now get the result from op. g_value_get_object() does not ref the
     * object, so we need to make a ref for out to hold.
     */
    g_value_init( &gvalue, VIPS_TYPE_IMAGE );
    g_object_get_property( G_OBJECT( op ), "out", &gvalue );
    out = VIPS_IMAGE( g_value_get_object( &gvalue ) );
    g_object_ref( out ); 
    g_value_unset( &gvalue );

    // /* All done: we can unref op. The output objects from op actually hold
    //  * refs back to it, so before we can unref op, we must unref them. 
    //  */
    vips_object_unref_outputs( VIPS_OBJECT( op ) ); 
    g_object_unref( op );

    vips_image_write_to_buffer(out, extension, (void **)buf_out, len_out, NULL);

}

static void smartcrop(unsigned char **buf, unsigned char **buf_out, char extension[], int width, int height, size_t *len_in, size_t *len_out)
{
    VipsImage *in = vips_image_new_from_buffer((void *)*buf, *len_in, "", NULL);
    VipsImage *out;

    vips_smartcrop(in, &out, width, height, NULL);
    vips_image_write_to_buffer(out, extension, (void **)buf_out, len_out, NULL);

    g_object_unref(in);
    g_object_unref(out);
}

ERL_NIF_TERM nif_smartcrop(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    unsigned char *buf = 0;
    char extension[1024];
    size_t len_out = 0;

    unsigned int width, height;
    ErlNifBinary bin_in, out;

    if (argc != 4 || !enif_inspect_binary(env, argv[0], &bin_in) ||
        !enif_get_string(env, argv[1], extension, sizeof(extension), ERL_NIF_LATIN1) ||
        !enif_get_uint(env, argv[2], &width) ||
        !enif_get_uint(env, argv[3], &height))
        return enif_make_badarg(env);


    smartcrop(&bin_in.data, &buf, extension, width, height, &bin_in.size, &len_out);
    enif_alloc_binary(len_out, &out);
    memcpy(out.data, buf, len_out);

    enif_release_binary(&bin_in);
    return enif_make_binary(env, &out);
}

ERL_NIF_TERM nif_vips_operation(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    unsigned char *buf = 0;
    char operation[1024];
    char extension[1024];
    size_t len_out = 0;

    ErlNifBinary bin_in, out;

    if (argc != 3 ||
        !enif_inspect_binary(env, argv[0], &bin_in) ||
        !enif_get_string(env, argv[1], operation, sizeof(operation), ERL_NIF_LATIN1) ||
        !enif_get_string(env, argv[2], extension, sizeof(extension), ERL_NIF_LATIN1))
        return enif_make_badarg(env);

    vips_operation(&bin_in.data, operation, extension, &bin_in.size, &len_out, &buf);
    enif_alloc_binary(len_out, &out);
    memcpy(out.data, buf, len_out);

    enif_release_binary(&bin_in);
    return enif_make_binary(env, &out);
}


ERL_NIF_TERM init_vips(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    vips_init("libvips");
    return enif_make_atom(env, "ok");
}

static ErlNifFunc nif_funcs[] = {
    {"nif_smartcrop", 4, nif_smartcrop, ERL_NIF_DIRTY_JOB_CPU_BOUND},
    {"nif_vips_operation", 3, nif_vips_operation, ERL_NIF_DIRTY_JOB_CPU_BOUND},
    {"init_vips", 0, init_vips, ERL_NIF_DIRTY_JOB_IO_BOUND}};

ERL_NIF_INIT(Elixir.Libvips, nif_funcs, NULL, NULL, NULL, NULL);