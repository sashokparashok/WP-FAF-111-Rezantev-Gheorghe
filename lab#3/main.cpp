#include <windows.h>
#include <math.h>
#include <stdio.h>

#include "resources.h"


/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
// All the functions i used mainly to make the code cleaner
void DrawTheLines(const HDC &,const RECT&);
void DrawTheWorkingArea(const HDC& ,const RECT& );
void DrawGeometry(const HDC& ,const RECT& );
void CreateGradient(const HDC &,const int ,const int ,const int ,const int );
void CreateButtons(const HWND& ,const RECT&);

/*  Make the class name into a global variable  */
HINSTANCE hInst;
// Create all the needed object handles
HWND hwndSquareButton,hwndCircleButton,hwndBezierButton;
char szClassName[ ] = "Lab#3";

int WINAPI WinMain (HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow)
{
    HWND hwnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */
    hInst=hThisInstance;
    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default colour as the background of the window */
    wincl.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx (&wincl))
        return 0;

    /* The class is registered, let's create the program*/
    hwnd = CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           szClassName,         /* Classname */
           "Graphics",          /* Title Text */
           WS_OVERLAPPEDWINDOW, /* default window */
           CW_USEDEFAULT,       /* Windows decides the position */
           CW_USEDEFAULT,       /* where the window ends up on the screen */
           800,                 /* The programs width */
           500,                 /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           NULL,                /* No menu */
           hThisInstance,       /* Program Instance handler */
           NULL                 /* No Window Creation data */
           );

    /* Make the window visible on the screen */
    ShowWindow (hwnd, nCmdShow);

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage (&messages, NULL, 0, 0))
    {
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
    }

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}


/*  This function is called by the Windows function DispatchMessage()  */

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    static RECT rect,oldRect,invalidationRect;                                //The oldRect is used for window resizing handling , the invalidationRect for invalidating only the drawing area
    static BOOL drawing_circle=FALSE,drawing_square=TRUE,drawing_bezier=FALSE,//Checks what is being drawn at the moment
    first_point=TRUE,                                                         //Checks if the first point of the bezier is being drawn
    button_pressed_in_area=FALSE,                                             //Checks if the button was pressed and if it happened inside the drawing area
    mouse_moving=FALSE;                                                       //Checks if the mouse is moving in order to paint the tracing of the object

    static HDC hdc,
    hdcMem;
    static HBITMAP hBitmap;
    BITMAP bitmap;

    static int figureCount=0;                                                  //Counts the number of figures in the drawing area
    static POINT arrayPoints[100][5];                                          //The array that holds the data of the figures

    static float xDisp,yDisp,slope,displacement;                               //Used to move and resize the pictures drawn . The last two variables are used to calculate the function
                                                                               //that the points will move on when resizing (the resize is a convergence towards the center)
    static int xFin=0,yFin=0,xFinSecond=0,yFinSecond=0;                        //Used to draw the tracing of elemens with the last two needed for Bezier only
    static int resizeCount;                                                    //Used to limit the resize to a certain number of times


    switch (message)                  /* handle the messages */
    {
        case WM_CREATE:

            GetClientRect(hwnd,&rect);
            //oldRect gets the same value as current rect as to avoid division by 0 and unintended resizing at the beginning
            oldRect=rect;
            //Creates the needed switches
            CreateButtons(hwnd,rect);
            // Loads the bitmap from resources
            hBitmap = LoadBitmap(hInst,MAKEINTRESOURCE(LOGO));
            GetObject(hBitmap,sizeof(BITMAP),&bitmap);
            break;

        case WM_SIZE:

            GetClientRect(hwnd,&rect);
            //Resizes all buttons based on the window size
            MoveWindow(hwndSquareButton,rect.right*3/4,rect.bottom/32,rect.right/8,30,TRUE);
            MoveWindow(hwndCircleButton,rect.right*7/8,rect.bottom/32,rect.right/8,30,TRUE);
            MoveWindow(hwndBezierButton,rect.right*3/4,rect.bottom/8,rect.right/8,30,TRUE);
            //Set the displacement of the drawings inside the window to new , moved coordinates
            xDisp=(float)rect.right/oldRect.right;
            yDisp=(float)rect.bottom/oldRect.bottom;

            oldRect.bottom=rect.bottom;
            oldRect.right=rect.right;

            for(int i=0;i<figureCount;i++) {
                for(int j=0;j<4;j++) {
                    arrayPoints[i][j].x*=xDisp;
                    arrayPoints[i][j].y*=yDisp;
                }
            }

            InvalidateRect(hwnd,NULL,TRUE);
            break;

        case WM_COMMAND:
            //Sets the buttons to active and puts the flags in the needed positions
            switch(wParam)
            {
                case ID_SWITCH_CIRCLE:
                SendMessage(hwndCircleButton,BM_SETCHECK,1,0);
                SendMessage(hwndBezierButton,BM_SETCHECK,0,0);
                SendMessage(hwndSquareButton,BM_SETCHECK,0,0);
                drawing_bezier=drawing_square=FALSE;
                drawing_circle=TRUE;
                break;

                case ID_SWITCH_SQUARE:
                SendMessage(hwndCircleButton,BM_SETCHECK,0,0);
                SendMessage(hwndBezierButton,BM_SETCHECK,0,0);
                SendMessage(hwndSquareButton,BM_SETCHECK,1,0);
                drawing_bezier=drawing_circle=FALSE;
                drawing_square=TRUE;
                break;

                case ID_SWITCH_BEZIER:
                SendMessage(hwndCircleButton,BM_SETCHECK,0,0);
                SendMessage(hwndBezierButton,BM_SETCHECK,1,0);
                SendMessage(hwndSquareButton,BM_SETCHECK,0,0);
                drawing_square=drawing_circle=FALSE;
                drawing_bezier=TRUE;
                break;

            }
            break;

        case WM_RBUTTONDOWN:
            //The setting up of invalidation region
            GetClientRect(hwnd,&rect);
            invalidationRect.right=rect.right*3/4-16;
            invalidationRect.top=rect.top+130;
            invalidationRect.bottom=rect.bottom-5;
            invalidationRect.left=rect.right*1/4+16;
            //The deletion a figure and refreshing the picture
            figureCount--;
            if(figureCount<0) figureCount=0;
            InvalidateRect(hwnd,&invalidationRect,TRUE);
            break;

        case WM_MOUSEMOVE:
            //Exits case in case the mouse is out of the needed area
            if(!button_pressed_in_area) break;
            mouse_moving=TRUE;
            //invalidation
            GetClientRect(hwnd,&rect);
            invalidationRect.right=rect.right*3/4-16;
            invalidationRect.top=rect.top+130;
            invalidationRect.bottom=rect.bottom-5;
            invalidationRect.left=rect.right*1/4+16;
             //Checks if the moving is being done in the drawing area and ends drawing in case it isn't
            if ((LOWORD(lParam)<rect.right/4+20) || (LOWORD(lParam)>rect.right*3/4-20) ||
                (HIWORD(lParam)<135) || (HIWORD(lParam)>rect.bottom-6)) {SendMessage(hwnd,WM_LBUTTONUP,NULL,lParam) ;break;}

            hdc=GetDC(hwnd);
            //Depending on the selected drawing sets the points for the tracing of figures
            if (drawing_circle) {
            xFin=LOWORD(lParam);
            yFin=HIWORD(lParam);
            }

            if (drawing_square) {
            xFin=LOWORD(lParam);
            yFin=HIWORD(lParam);
            }

            if (drawing_bezier && !first_point) {
               xFin=LOWORD(lParam);
               yFin=HIWORD(lParam);
            } else if (drawing_bezier) {
               xFinSecond=LOWORD(lParam);
               yFinSecond=HIWORD(lParam);
            }
            //Send message to repaint drawing area
            InvalidateRect(hwnd,&invalidationRect,TRUE);
            ReleaseDC(hwnd,hdc);

            break;

        case WM_LBUTTONDOWN:
            //Checks if the button clicking is being done in the drawing area and sets focus to main window
            SetFocus(hwnd);
            GetClientRect(hwnd,&rect);
            if ((LOWORD(lParam)<rect.right/4+14) || (LOWORD(lParam)>rect.right*3/4-14) ||
                (HIWORD(lParam)<125) || (HIWORD(lParam)>rect.bottom-6)) break;

            button_pressed_in_area=TRUE;
            hdc=GetDC(hwnd);
            //Gets initial points for the figure being drawn and sets a information value in order to read the right thing from the array afterwards
            if (drawing_square) {
                arrayPoints[figureCount][4].x=ID_SWITCH_SQUARE;
                arrayPoints[figureCount][0].x=LOWORD(lParam);
                arrayPoints[figureCount][0].y=HIWORD(lParam);
            }

            if (drawing_bezier && first_point) {
                arrayPoints[figureCount][4].x=ID_SWITCH_BEZIER;
                arrayPoints[figureCount][0].x=LOWORD(lParam);
                arrayPoints[figureCount][0].y=HIWORD(lParam);
                first_point=FALSE;
            } else if (drawing_bezier) {
                first_point=TRUE;
                figureCount--;
                arrayPoints[figureCount][2].x=LOWORD(lParam);
                arrayPoints[figureCount][2].y=HIWORD(lParam);
            }

            if (drawing_circle) {
                arrayPoints[figureCount][4].x=ID_SWITCH_CIRCLE;
                arrayPoints[figureCount][0].x=LOWORD(lParam);
                arrayPoints[figureCount][0].y=HIWORD(lParam);
            }
            ReleaseDC(hwnd,hdc);
            break;

        case WM_LBUTTONUP:
            //Makes sure the drawing started inside the canvas
            if(!button_pressed_in_area) break;
            mouse_moving=FALSE;
            //invalidation
            GetClientRect(hwnd,&rect);
            invalidationRect.right=rect.right*3/4-16;
            invalidationRect.top=rect.top+130;
            invalidationRect.bottom=rect.bottom-5;
            invalidationRect.left=rect.right*1/4+16;

            hdc=GetDC(hwnd);
            //Gets the end points of the figures
            if (drawing_square) {
                arrayPoints[figureCount][1].x=LOWORD(lParam);
                arrayPoints[figureCount][1].y=HIWORD(lParam);
            }

            if (drawing_bezier && !first_point) {
                arrayPoints[figureCount][1].x=LOWORD(lParam);
                arrayPoints[figureCount][1].y=HIWORD(lParam);

            } else if (drawing_bezier) {
                arrayPoints[figureCount][3].x=LOWORD(lParam);
                arrayPoints[figureCount][3].y=HIWORD(lParam);
                InvalidateRect(hwnd,&invalidationRect,TRUE);
            }

            if (drawing_circle) {
                arrayPoints[figureCount][1].x=LOWORD(lParam);
                arrayPoints[figureCount][1].y=HIWORD(lParam);
            }
            //Increments nr of figures
            figureCount++;
            button_pressed_in_area=FALSE;
            //The bezier uses 4 points instead of 2 like the others so different actions when invalidating
            if (!drawing_bezier) {
               InvalidateRect(hwnd,&invalidationRect,TRUE);
            }

            ReleaseDC(hwnd,hdc);

            break;

        case WM_KEYDOWN:
            //invalidation
            GetClientRect(hwnd,&rect);
            invalidationRect.right=rect.right*3/4-16;
            invalidationRect.top=rect.top+130;
            invalidationRect.bottom=rect.bottom-5;
            invalidationRect.left=rect.right*1/4+16;
            //Resizes canvas based on convergence on the line formed by the center point of the canvas and a point of the figure
            switch(wParam) {
            case VK_UP:

            if (resizeCount<0) break;
              for(int i=0;i<figureCount;i++) {
                for(int j=0;j<4;j++) {

                    slope=(arrayPoints[i][j].y-(rect.bottom-126)/2)/(float)(arrayPoints[i][j].x-rect.right/2);
                    displacement=arrayPoints[i][j].y-slope*arrayPoints[i][j].x;

                    arrayPoints[i][j].x=(rect.right/2)-(rect.right/2-arrayPoints[i][j].x)*1.1;
                    arrayPoints[i][j].y=roundf(arrayPoints[i][j].x*slope+displacement);
                }
            }
            resizeCount--;
            break;

            case VK_DOWN:

              if(resizeCount>10) break;
              for(int i=0;i<figureCount;i++) {
                for(int j=0;j<4;j++) {

                    slope=(arrayPoints[i][j].y-(rect.bottom-126)/2)/(float)(arrayPoints[i][j].x-rect.right/2);
                    displacement=arrayPoints[i][j].y-slope*arrayPoints[i][j].x;

                    arrayPoints[i][j].x=(rect.right/2)-(rect.right/2-arrayPoints[i][j].x)*0.9;
                    arrayPoints[i][j].y=roundf(arrayPoints[i][j].x*slope+displacement);


                }
            }
            resizeCount++;
            break;
            }
            InvalidateRect(hwnd,&invalidationRect,TRUE);
            break;

        case WM_PAINT:
            hdc=BeginPaint(hwnd,&ps);
            GetClientRect(hwnd,&rect);
            //Create figures
            SelectObject(hdc,GetStockObject(NULL_BRUSH));
            for (int i=0;i<figureCount;i++) {
                switch(arrayPoints[i][4].x){
                case ID_SWITCH_SQUARE:
                Rectangle(hdc,arrayPoints[i][0].x,arrayPoints[i][0].y,arrayPoints[i][1].x,arrayPoints[i][1].y);
                break;
                case ID_SWITCH_CIRCLE:
                Ellipse(hdc,arrayPoints[i][0].x,arrayPoints[i][0].y,arrayPoints[i][1].x,arrayPoints[i][1].y);
                break;
                case ID_SWITCH_BEZIER:
                PolyBezier(hdc,arrayPoints[i],4);
                break;
                }
            }
            //Create traces
            if (mouse_moving) {

            if (drawing_circle) {
                Ellipse(hdc,arrayPoints[figureCount][0].x,arrayPoints[figureCount][0].y,xFin,yFin);
            }

            if (drawing_bezier && !first_point) {
                MoveToEx(hdc,arrayPoints[figureCount][0].x,arrayPoints[figureCount][0].y,NULL);
                LineTo(hdc,xFin,yFin);
            } else if (drawing_bezier) {
                MoveToEx(hdc,arrayPoints[figureCount][0].x,arrayPoints[figureCount][0].y,NULL);
                LineTo(hdc,xFin,yFin);

                MoveToEx(hdc,arrayPoints[figureCount][2].x,arrayPoints[figureCount][2].y,NULL);
                LineTo(hdc,xFinSecond,yFinSecond);
            }

            if (drawing_square) {
            Rectangle(hdc,arrayPoints[figureCount][0].x,arrayPoints[figureCount][0].y,xFin,yFin);
            }

            }

            SelectObject(hdc,GetStockObject(WHITE_BRUSH));
            //Create the gradients
            CreateGradient(hdc,0,0,rect.right/4+10,rect.bottom+1);
            CreateGradient(hdc,rect.right*3/4-10,0,rect.right+1,rect.bottom+1);
            //Creates the background for drawing
            DrawTheWorkingArea(hdc,rect);
            //Adds the lines to the drawing
            DrawTheLines(hdc,rect);
            //Adds figures to the mix
            DrawGeometry(hdc,rect);
            //Make bmp
            hdcMem = CreateCompatibleDC(hdc);
            SelectObject(hdcMem,hBitmap);
            BitBlt(hdc,rect.right/4+16,6,185,120,hdcMem,0,0,SRCCOPY);
            DeleteDC(hdcMem);
            EndPaint(hwnd,&ps);
            break;

        case WM_CTLCOLORSTATIC:
            SetBkMode((HDC)wParam,TRANSPARENT);
            return (LRESULT)GetStockObject(NULL_BRUSH);

        case WM_GETMINMAXINFO:
            //Setting the minimal size for the window
            MINMAXINFO *ptMinMax;
            ptMinMax=(MINMAXINFO*)lParam;

            ptMinMax->ptMinTrackSize.x=450;
            ptMinMax->ptMinTrackSize.y=300;
            break;

        case WM_DESTROY:

            PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
            break;
        default:                      /* for messages that we don't deal with */
            return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}



//The function that will draw the basic static lines .
void
DrawTheLines(const HDC & hdc,const RECT& rect) {
    HPEN hPen;
    SetBkMode(hdc,TRANSPARENT);
    //Creating the pen and drawing the line . If the line style isn't solid a weight bigger than 1 will force it to solid
    hPen = CreatePen(PS_DASHDOT,0,RGB(5,5,20));
    SelectObject(hdc,hPen);
    MoveToEx(hdc,10,rect.bottom*15/16,NULL);
    LineTo(hdc,rect.right/4,rect.bottom*15/16);
    //Deleting the pen
    DeleteObject(hPen);

    hPen = CreatePen(PS_DASH,1,RGB(255,0,200));
    SelectObject(hdc,hPen);
    MoveToEx(hdc,10,rect.bottom*14/16,NULL);
    LineTo(hdc,rect.right/4,rect.bottom*14/16);
    DeleteObject(hPen);

    hPen = CreatePen(PS_DOT,0,RGB(0,0,255));
    SelectObject(hdc,hPen);
    MoveToEx(hdc,10,rect.bottom*13/16,NULL);
    LineTo(hdc,rect.right/4,rect.bottom*13/16);
    DeleteObject(hPen);

    hPen = CreatePen(PS_DASHDOTDOT,0,RGB(200,0,100));
    SelectObject(hdc,hPen);
    MoveToEx(hdc,10,rect.bottom*12/16,NULL);
    LineTo(hdc,rect.right/4,rect.bottom*12/16);
    DeleteObject(hPen);

    hPen = CreatePen(PS_SOLID,4,RGB(5,255,20));
    SelectObject(hdc,hPen);
    MoveToEx(hdc,10,rect.bottom*11/16,NULL);
    LineTo(hdc,rect.right/4,rect.bottom*11/16);
    DeleteObject(hPen);

    SelectObject(hdc,GetStockObject(BLACK_PEN));

}

// Creates pretty rectangles and stuff for the canvas image
void
DrawTheWorkingArea(const HDC& hdc,const RECT& rect) {
    HPEN hPen;
    hPen=CreatePen(PS_SOLID,5,RGB(255,255,255));
    SelectObject(hdc,hPen);
    SelectObject(hdc,GetStockObject(NULL_BRUSH));
    Rectangle(hdc,rect.right/4+11,125,rect.right*3/4-11,rect.bottom);
    DeleteObject(hPen);

    hPen=CreatePen(PS_DASHDOTDOT,1,RGB(15,15,15));
    SelectObject(hdc,hPen);
    Rectangle(hdc,rect.right/4+12,126,rect.right*3/4-12,rect.bottom-2);
    Rectangle(hdc,rect.right/4+14,128,rect.right*3/4-14,rect.bottom-4);
    DeleteObject(hPen);

    SelectObject(hdc,GetStockObject(NULL_PEN));
    SelectObject(hdc,GetStockObject(WHITE_BRUSH));
    Rectangle(hdc,rect.right/4+11,0,rect.right*3/4-11,124);


}

//Adds lines and figures that are on the gradients
void
DrawGeometry(const HDC& hdc,const RECT& rect) {
    HPEN hPen;
    HBRUSH hBrush;

    hPen=CreatePen(PS_SOLID,2,RGB(30,30,30));
    SelectObject(hdc,hPen);
    hBrush=CreateSolidBrush(RGB(0,180,180));
    SelectObject(hdc,hBrush);
    RoundRect(hdc,rect.right*15/16-10,rect.bottom*7/8,rect.right*15/16+rect.right*1/16-10,rect.bottom*7/8+rect.right*1/16,15,15);
    DeleteObject(hPen);
    DeleteObject(hBrush);

    hPen=CreatePen(PS_DASH,1,RGB(255,0,0));
    SelectObject(hdc,hPen);
    hBrush=CreateSolidBrush(RGB(130,0,130));
    SelectObject(hdc,hBrush);
    Ellipse(hdc,rect.right*3/4,rect.bottom*7/8,rect.right*3/4+rect.right*1/16,rect.bottom*7/8+rect.right*1/16);
    DeleteObject(hPen);
    DeleteObject(hBrush);

    hPen=(HPEN)GetStockObject(NULL_PEN);
    SelectObject(hdc,hPen);
    hBrush=CreateSolidBrush(RGB(80,80,0));
    SelectObject(hdc,hBrush);
    Chord(hdc,rect.right*3/4,rect.bottom*3/4,rect.right*3/4+rect.right*1/16,rect.bottom*3/4+rect.right*1/16,5000,1500,100,155);
    DeleteObject(hPen);
    DeleteObject(hBrush);

    hPen=CreatePen(PS_DOT,1,RGB(255,255,255));
    SelectObject(hdc,hPen);
    hBrush=CreateSolidBrush(RGB(30,30,30));
    SelectObject(hdc,hBrush);
    Pie(hdc,rect.right*15/16-10,rect.bottom*3/4,rect.right*15/16+rect.right*1/16-10,rect.bottom*3/4+rect.right*1/16,10,500,100,15);
    DeleteObject(hPen);
    DeleteObject(hBrush);

    SelectObject(hdc,GetStockObject(BLACK_PEN));
    SelectObject(hdc,GetStockObject(NULL_BRUSH));
}

//Creates the side gradients
void
CreateGradient(const HDC & hdc,const int xInit,const int yInit,const int xFin,const int yFin) {
    HBRUSH hBrush;
    for(int i=0;i<256;i++){
        hBrush=CreateSolidBrush(RGB(i,i,255));
        SelectObject(hdc,hBrush);
        SelectObject(hdc,GetStockObject(NULL_PEN));
        Rectangle(hdc,xInit,yInit,xFin,yFin-i*(yFin)/255);
        DeleteObject(hBrush);

    }
    SelectObject(hdc,GetStockObject(NULL_BRUSH));
    SelectObject(hdc,GetStockObject(BLACK_PEN));
}

//Buttons
void
CreateButtons(const HWND& hwnd,const RECT& rect){
    hwndSquareButton = CreateWindowEx((DWORD)NULL,TEXT("button"),
                                      TEXT("Rectangle"),WS_CHILD|WS_VISIBLE|BS_RADIOBUTTON,
                                      rect.right*3/4,rect.bottom/32,rect.right/8,30,hwnd,(HMENU)ID_SWITCH_SQUARE,
                                      hInst,NULL);
    hwndCircleButton = CreateWindowEx((DWORD)NULL,TEXT("button"),
                                      TEXT("Ellipse"),WS_CHILD|WS_VISIBLE|BS_RADIOBUTTON,
                                      rect.right*7/8,rect.bottom/32,rect.right/8,30,hwnd,(HMENU)ID_SWITCH_CIRCLE,
                                      hInst,NULL);
    hwndBezierButton = CreateWindowEx((DWORD)NULL,TEXT("button"),
                                      TEXT("Bezier"),WS_CHILD|WS_VISIBLE|BS_RADIOBUTTON,
                                      rect.right*3/4,rect.bottom/8,rect.right/8,30,hwnd,(HMENU)ID_SWITCH_BEZIER,
                                      hInst,NULL);
    SendMessage(hwndSquareButton,BM_SETCHECK,1,0);
}
