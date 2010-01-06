
#include <windows.h>

/* The mingw-runtime startup code has _argc and _argv as visible
   symbols, as do the MS compilers.  But the mingw-w64-crt is
   different, but this file is not used on mingw64 */

extern void 
GA_startgraphapp(HINSTANCE Instance, HINSTANCE PrevInstance, int CmdShow);


int PASCAL
WinMain (HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CmdLine,
	 int CmdShow)
{
    extern int _argc;
    extern char **_argv;
    extern void AppMain(int argc, char **argv);

    GA_startgraphapp(Instance, PrevInstance, CmdShow);
    AppMain(_argc, _argv);

    return 0;
}
