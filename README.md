# `cvkstart`

`cvkstart` aims to be a small utility library that with helps the initalization and boilerplate of Vulkan.

`cvkstart` is written in pure C, has no dependencies other than Vulkan itself and `alloca.h`.
It performs no dynamic allocations nor does it need a custom allocator to be provided. 

(If you use C++, you might rather use [vk-bootstrap](https://github.com/charles-lunarg/vk-bootstrap))

## Structure

* `Makefile`: Contains compilation commands for static library `.a` (*TODO: Header only and windows*).
* `src`     : Folder containing all the source for the project.
  * cvkstart.h : The main project's header.
  * cvkstart.c : Contains all the project's code.
  * test.c : Contains a very simple program that can be built to test the basic functionnality of the lib.
* `compile_commands.json` : Compilation database for `clangd`.

## Documentation

For now, all the functions, structures, structure members are documented following a doxygen style.

## Naming

I named it `cvkstart` rather than `cvkbootstrap` even though it aims to be basically the same thing, because it
is not a port or bindings to `vkbootstrap`.

