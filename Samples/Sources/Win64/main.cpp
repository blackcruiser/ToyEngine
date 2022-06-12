#include "Application.h"

int main()
{
    TEApplication app;
    app.Init();
    app.Run();
    app.Cleanup();

    return 0;
}