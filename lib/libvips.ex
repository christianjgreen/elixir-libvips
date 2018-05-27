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

  def nif_smartcrop(file_buf, extension, width, height)
  def nif_smartcrop(_, _, _, _), do: exit(:nif_library_not_loaded)

  def nif_vips_operation(file_buf, operation, extenion)
  def nif_vips_operation(_, _, _), do: exit(:nif_library_not_loaded) 

end
