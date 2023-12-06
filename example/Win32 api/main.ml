##include<"struct.ml">;
##include<"api.ml">;
Main{
    main()->N:={
        WinMain(0,0,0,1);
    }
}

WinProc{
    N:Button1;
    OnCreate(N:hwnd)->N:={
        Button1=CreateWindow(0,&"Button",&"Click",1342177280,110,90,150,100,hwnd,0,0,0);
    }
    OnMsg(MSG:msg)->N:={
        if(msg.hwnd==Button1 and msg.message==513){
            msgbox(0,&"Clicked!",&"msg",0)
        }
    }
}

Window{
    WndProc(N:hWnd,N:Msg,N:wParam,N:lParam)->N:={
        if(Msg==2){
            PostQuitMessage(0);return(0)
        };
        return(DefWindowProc(hWnd,Msg,wParam,lParam))
    }
    WinMain(N:hInstance,N:hPrevInstance,N:szCmdLine,N:nCmdShow)->N:={
        N:hWnd;MSG:Msg;WNDCLASS:WndClass;
        N:AppName=&"MyApp";
        N:Title=&"The Hello Program";
        WndClass.style=3;
        WndClass.lpfnwndproc=~WndProc;
        WndClass.cbClsextra=0;
        WndClass.cbWndExtra=0;
        WndClass.hInstance=hInstance;
        WndClass.hIcon=LoadIcon (0,&"32512");
        WndClass.hCursor=LoadCursor (0,&"32512");
        WndClass.hbrBackground=GetStockObject(0);
        WndClass.lpszMenuName=&"";
        WndClass.lpszClassName=AppName;
        if(RegisterClass(WndClass)==0){
            msgbox(0,&"error",&"This program requires Windows NT!",0);
        };
        hWnd=CreateWindow (0,AppName,Title,13565952,100,100,400,300,0,0,hInstance,&"");
        OnCreate(hWnd);
        ShowWindow(hWnd,nCmdShow);
        UpdateWindow(hWnd);
        while(GetMessage(Msg,0,0,0)!=0){
            TranslateMessage(Msg);
            DispatchMessage(Msg);
            OnMsg(Msg);
        };
        return(Msg.wParam);
    }
}