defmodule Libvips do
  @compile {:autoload, true}
  @on_load {:init, 0}

  def init do
    with :ok <- :erlang.load_nif('./src/vips', 0),
         :ok <- init_vips() do
      :ok
    else
      err ->
        raise """
        #{err}
        _________________________________
        An error occurred when loading Libvips.
        Make sure you have a C compiler and Erlang 20 installed.
        """
    end
  end

  def init_vips
  def init_vips, do: exit(:nif_library_not_loaded)

  def nif_smartcrop(file_path, width, height)
  def nif_smartcrop(_, _, _), do: exit(:nif_library_not_loaded)

  def nif_smartcrop_buffer(image, width, height)
  def nif_smartcrop_buffer(_, _, _), do: exit(:nif_library_not_loaded)

  def smartcrop(file_path, width, height) do
    file_path
    |> :binary.bin_to_list()
    |> nif_smartcrop(width, height)
  end

  def smartcrop_buffer(file_buf, width, height) when is_binary(file_buf),
    do: nif_smartcrop_buffer(file_buf, width, height)

  def smartcrop_buffer(_, _, _), do: exit(:nif_library_not_loaded)
end
