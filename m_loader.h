#ifndef M_LOADER_H_
#define M_LOADER_H_

DWORD WINAPI loader_th(void *data)
{
    printf(" \\");
    
    while (1)
    {
        if (*(int*)data == -1) break;
        Sleep(250);
        printf("\b|");
        Sleep(250);
        printf("\b/");
        Sleep(250);
        printf("\b-");
        Sleep(250);
        printf("\b\\");
    }

    *(int*)data = 1;

    return 0;
}

void start_loader(int *_switch)
{
    CreateThread(NULL, 0, loader_th, _switch, 0, NULL);
}

void stop_loader(int *_switch, int _exit_code)
{
    *_switch = -1;
    while (*_switch == -1);
    if (_exit_code == -1) printf("\bfailed\n");
    else printf("\bdone\n");
}

#endif