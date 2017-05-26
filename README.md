# Geany Prettify

Source code prettifier plugin for Geany.

## Features

* Prettifies source code files with the following extensions :
  * PHP : php.
  * HTML : html, htm, xml, twig.
  * CSS : css, less, pepss, sass, scss, styl.
  * JavaScript : js, json.
  * C : c, h.
  * C++ : cxx, hxx, cpp, hpp.
  * D : d.

## Installation

Install the [Prettify](https://github.com/senselogic/GEANY_PRETTIFY) command line tool.

Build the plugin with the following command lines :

```bash
gcc -O3 -c geany_prettify.c -fPIC `pkg-config --cflags geany` 
gcc -O3 geany_prettify.o -o geany_prettify.so -shared `pkg-config --libs geany`
```

Set the plugin folder path in Geany's startup preferences.

Enable the plugin.

Set the Prettify path in the plugin preferences.

## Usage

Use the Prettify option in the Tool menu.

## Version

0.1

## Author

Eric Pelzer (ecstatic.coder@gmail.com).

## License

This project is licensed under the GNU General Public License version 3.

See the [LICENSE.md](LICENSE.md) file for details.
