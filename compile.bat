cd Tools
set BISON_PKGDATADIR=share/bison
flex.exe -8 -Cf -o../generated/lex.yy.c ../rave.l
bison.exe -dv -o../generated/y.tab.c ../rave.y -r state