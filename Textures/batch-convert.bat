pushd oltex

FOR %%G IN (*.pcx) DO (call :subroutine "%%G")

popd

GOTO :eof

:subroutine
 set ALA=%1
 @rem echo %ALA:pcx=bmp%
 magick %1 %ALA:pcx=bmp%
 GOTO :eof
