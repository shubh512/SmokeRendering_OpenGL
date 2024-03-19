#include<windows.h>
#include<stdio.h>
#include<stdlib.h>
#include<gl/GL.h>
#include<GL/glu.h>
#include"OGL.h"

//Macros
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

// link with opengl library
#pragma comment(lib, "OpenGL32.lib")

//Global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

FILE *gpfile = NULL;

HWND ghwnd = NULL;
BOOL gbActive = FALSE;
DWORD DWSTYLE = 0;
WINDOWPLACEMENT wpPrev = {sizeof(WINDOWPLACEMENT)};
BOOL gbFullScreen = FALSE;

//OpenGl Global Var
HDC ghdc = NULL;
HGLRC ghrc = NULL; //H->Handle, GL->Grphics Lib, RC->Rendering Context Creates OPENGL context from device context


//Entry point function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszcmdline, int iCmdShow)
{
    //Function declarations
    int initialize(void);
    void uninitialize(void);
    void display(void);
    void update(void);

    //Local variable declarations 
    WNDCLASSEX wndclass;
    HWND hwnd;
    MSG msg;
    TCHAR szAppName[] = TEXT("sbp_window");
    int iResult = 0;
    BOOL bDone = FALSE;


    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int windowWidth = WIN_WIDTH;
    int windowHeight = WIN_HEIGHT;

    int x = (screenWidth - windowWidth) / 2;
    int y = (screenHeight - windowHeight) / 2;

    gpfile = fopen("log.txt", "w");
    if(gpfile == NULL)
    {
        MessageBox(NULL, TEXT("Log File Cannot be Opened!"), TEXT("Error"), MB_OK | MB_ICONERROR);
        exit(0);
    }
    fprintf(gpfile, "Program Started Successfully\n");
    //code
    //WNDCLASS initialization
    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; 
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.lpfnWndProc = WndProc;
    wndclass.hInstance = hInstance;
    wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH); //actual return value is HGDIOBJ
    wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON)); //
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW); // NULL indiacates user doesn't have its own RESOURCE so OS can provide its own 
                                                    // ID -> Identification Arrow cursor of windows
    wndclass.lpszClassName = szAppName;
    wndclass.lpszMenuName = NULL;
    wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));; 
    
    //Register wndclassex
    RegisterClassEx(&wndclass);

    //Create window
    hwnd = CreateWindowEx(WS_EX_APPWINDOW,
                        szAppName,  // creates window in memory
                        // TEXTMACRO("shubham_Balaji_patil"),
                        TEXT("shubham_Balaji_patil"), //Convert ANSI string to UNICODE TEXT macro is used
                        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE, // WS -> Window style that is overlapped window i.e. this window will pop up on other windows
                        x, // x
                        y, // y
                        WIN_WIDTH, // width
                        WIN_HEIGHT, // height
                        NULL,  // Parent window handle; if given NULL it assumes HEND_DESKTOP as parent windows as it is default window for OS
                        NULL,  // Handle of menu
                        hInstance, // Instance of which Window to be created
                        NULL);  
    
    ghwnd = hwnd;

    //intialization 
    iResult = initialize();
    if(iResult != 0)
    {
        MessageBox(hwnd, TEXT("Initialization failed!"), TEXT("Error"), MB_OK | MB_ICONERROR);
        DestroyWindow(hwnd);
    }


    ShowWindow(hwnd, iCmdShow);  // shows windows created by createwindow by passing handle and shown as default by OS iCmdShow
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);
    // Paint/Redraw the window
    // UpdateWindow(hwnd); // not required in game loop


    //Game Loop
    while(bDone == FALSE)
    {
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if(msg.message == WM_QUIT)
                bDone = TRUE;
            
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }               
        }
        else
        {            
            if(gbActive == TRUE)
            {
                //render
                display();
                //update
                update();
            }
        }
    }

    // uninitialization
    uninitialize();

    fclose(gpfile);
    return (int)msg.wParam;
}

//Callback function
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    //code
    //Local Function declarations
    void toggleFullScreen(void);
    void resize(int, int);

    //code
    switch(iMsg)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            if(gpfile)
            {
                fprintf(gpfile, "Program Ended Successfully\n");
            }
            break;

        case WM_SETFOCUS:
            gbActive = TRUE;
            break;

        case WM_KILLFOCUS:
            gbActive = FALSE;
            break;

        case WM_SIZE:
            resize(LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_ERASEBKGND:
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set the clear color to blue
            glClear(GL_COLOR_BUFFER_BIT);
            return 1;

        case WM_KEYDOWN:
            switch(LOWORD(wParam))
            {
                case VK_ESCAPE:
                    DestroyWindow(hwnd);
                    break;
            }
            break;

        case WM_CHAR:
            switch(LOWORD(wParam))
            {
                case 'F':
                case 'f':
                    if(gbFullScreen == FALSE)
                    {
                        toggleFullScreen();
                        gbFullScreen = TRUE;
                    }
                    else
                    {
                        toggleFullScreen();
                        gbFullScreen = FALSE;
                    }
                    break;
            }
            break;

        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;

        default:
            break;
    }

    return DefWindowProc(hwnd, iMsg, wParam, lParam);  //Def Window Proc-> Default Window Procedure
}

void toggleFullScreen(void)
{
    //local var declarations
    MONITORINFO MI = {sizeof(MONITORINFO)};

    if(gbFullScreen == FALSE)
    {
        DWSTYLE = GetWindowLong(ghwnd, GWL_STYLE);
        if(DWSTYLE & WS_OVERLAPPEDWINDOW)
        {
            if(GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &MI))
            {
                SetWindowLong(ghwnd, GWL_STYLE, DWSTYLE &~WS_OVERLAPPEDWINDOW);
                SetWindowPos(ghwnd, HWND_TOP, MI.rcMonitor.left, MI.rcMonitor.top, MI.rcMonitor.right - MI.rcMonitor.left, MI.rcMonitor.bottom - MI.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED);
            }
        }
        ShowCursor(FALSE);
    }

    else 
    {
        SetWindowPlacement(ghwnd, &wpPrev);
        SetWindowLong(ghwnd, GWL_STYLE, DWSTYLE | WS_OVERLAPPEDWINDOW);
        SetWindowPos(ghwnd, HWND_TOP, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);
        ShowCursor(TRUE);
    }
}

/* WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX */ 

int initialize(void)
{
    //function declarations
    //code
    PIXELFORMATDESCRIPTOR pfd;
    int iPixelFormatIndex = 0;
    ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

    //Initialization of PixelFormatDescriptor
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cRedBits = 8;
    pfd.cBlueBits = 8;
    pfd.cGreenBits = 8;
    pfd.cAlphaBits = 8;
    
    //get the DC
    ghdc = GetDC(ghwnd);
    if(ghdc == NULL)
    {
        fprintf(gpfile, "Get DC Failed!\n");
        return -1;
    }
    iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
    if(iPixelFormatIndex == 0)
    {
        fprintf(gpfile, "Get iPixelFormatIndex Failed!\n");
        return -2;
    }
    
    if(SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
    {
        fprintf(gpfile, "Get SetPixelFormat Failed!\n");
        return -3;
    }


    ghrc = wglCreateContext(ghdc);
    if(ghrc == NULL) 
    {
        fprintf(gpfile, "wglCreateContext Failed!\n");
        return -4;
    }
    //ghdc will end its role and gives control to ghrc
    //make rendering context current
    if(wglMakeCurrent(ghdc, ghrc) == FALSE)
    {
        fprintf(gpfile, "wglMakeCurrent Failed!\n");
        return -5;
    }

    //set the clear color of window to blue
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    return 0;
}

void resize(width, height)
{
    //code
    if(height <= 0)
        height = 1; 

    glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

    //1. Following call is done for model view transforamtion
    // glTranslatef(0.0f, 0.0f, -3.0f);

    //2. Following call need to be done during viewing transformation
    gluLookAt(0.0f, 0.0f, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    glBegin(GL_TRIANGLES);
    glColor3f(1.0f, 0.5f, 0.5f);
    glVertex3f(-0.5f, -0.5f, 0.0f);
    glVertex3f(0.5f, -0.5f, 0.0f);
    glVertex3f(0.0f, 0.5f, 0.0f);
    glEnd();


	SwapBuffers(ghdc);
}

void update(void)
{

}

void uninitialize(void)
{
    //function declarations
    void toggleFullScreen();

    //make hdc as current dc
    if(wglGetCurrentDC() == ghdc)
    {
        wglMakeCurrent(NULL, NULL);
    }

    //code
    if(gbFullScreen == TRUE)
    {
        toggleFullScreen();
        gbFullScreen = FALSE;
    }

    //Destroy rendering context
    if(ghrc)
    {
        wglDeleteContext(ghrc);
    }
    ghrc = NULL;
    
    //release the hdc
    if(ghdc)
    {
        ReleaseDC(ghwnd, ghdc);
    }
    ghdc = NULL;
    DeleteDC(ghdc);
    fprintf(gpfile, "Program Ended Successfully!\n");
}