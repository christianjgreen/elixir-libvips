defmodule LibvipsTest do
  use ExUnit.Case
  doctest Libvips

  test "greets the world" do
    assert Libvips.hello() == :world
  end
end
