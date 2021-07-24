# Direct2D SVG to PNG Renderer

Converts SVG files to PNG files using [Direct2D SVG Renderer](https://docs.microsoft.com/en-us/windows/win32/direct2d/svg-support) and
[ID2D1SvgDocument](https://docs.microsoft.com/en-us/windows/win32/api/d2d1svg/nn-d2d1svg-id2d1svgdocument).

This command line tool is used to test Direct2D SVG rendering.

## Building

Build commands for MSVC 2019 (in Windows Command Prompt):

    cl /nologo /c /Od /W4 /WX /EHsc d2dsvg2png-tester.cpp
    link /nologo /WX d2dsvg2png-tester.obj d2d1.lib windowscodecs.lib ole32.lib shlwapi.lib /OUT:d2dsvg2png-tester.exe

## Running

To convert SVG to PNG:

    d2dsvg2png-tester input.svg output.png
