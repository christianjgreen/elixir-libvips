#include <erl_nif.h>
#include <vips/vips8>

#include <iostream>
#include <fstream>
#include <sstream>

using namespace vips;

static void smartcrop_path(char path[], int width, int height, char** buf, size_t* len) {
    VImage in = VImage::new_from_file(path);
    VImage out = in.smartcrop(width, height, (VImage::option()->set("interesting", VIPS_INTERESTING_ATTENTION)));
    out.write_to_buffer(".png", (void **) buf, len, NULL); 
}

static void smartcrop_buffer(unsigned char **buf, unsigned char **buf_out, int width, int height, size_t* len_in, size_t* len_out) {
    VImage in = VImage::new_from_buffer((void *) *buf, *len_in, "", NULL);
    VImage out = in.smartcrop(width, height, (VImage::option()->set("interesting", VIPS_INTERESTING_ATTENTION)));
    out.write_to_buffer(".png", (void **) buf_out, len_out, NULL);  
}

ERL_NIF_TERM smartcrop(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    char path[2048];
    char *buf = 0;
    size_t len = 0;

    unsigned int width, height;
    ErlNifBinary result_binary;
    ERL_NIF_TERM result_term;

    if (argc != 3 || !enif_get_string(env, argv[0], path, sizeof(path), ERL_NIF_LATIN1) ||
        !enif_get_uint(env, argv[1], &width) ||
        !enif_get_uint(env, argv[2], &height))
        return enif_make_badarg(env);

    try
    {
        smartcrop_path(path, width, height, &buf, &len);
    }
    catch (VError err)
    {   
        printf("%s", err.what());
        return enif_raise_exception(env, enif_make_int(env, -69));
    }
    enif_alloc_binary(len, &result_binary);
    memcpy(result_binary.data, buf, len);
    result_term = enif_make_binary(env, &result_binary);
    free(buf);
    return result_term;
}

ERL_NIF_TERM smartcrop_buffer(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    unsigned char *buf = 0;
    size_t len_out = 0;

    unsigned int width, height;
    ErlNifBinary bin_in, out;

    if (argc != 3 ||
        !enif_inspect_binary(env, argv[0], &bin_in) ||
        !enif_get_uint(env, argv[1], &width) ||
        !enif_get_uint(env, argv[2], &height))
        return enif_make_badarg(env);
    try
    {
        smartcrop_buffer(&bin_in.data, &buf, width, height, &bin_in.size, &len_out);
        enif_alloc_binary(len_out, &out);
        memcpy(out.data, buf, len_out);
    }
    catch (VError err)
    {   
        printf("%s", err.what());
        return enif_raise_exception(env, enif_make_int(env, -69));
    }

    enif_release_binary(&bin_in);
    return enif_make_binary(env, &out);
}

ERL_NIF_TERM init_vips(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    vips_init("libvips");
    return enif_make_atom(env, "ok");
}

static ErlNifFunc nif_funcs[] = {
    {"nif_smartcrop", 3, smartcrop, ERL_NIF_DIRTY_JOB_CPU_BOUND},
    {"nif_smartcrop_buffer", 3, smartcrop_buffer, ERL_NIF_DIRTY_JOB_CPU_BOUND},
    {"init_vips", 0, init_vips, ERL_NIF_DIRTY_JOB_IO_BOUND}};

ERL_NIF_INIT(Elixir.Libvips, nif_funcs, NULL, NULL, NULL, NULL);