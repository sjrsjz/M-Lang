Extra:"kernel32.dll"{
    WinExec(N:lpCmdLine,N:show)->N:=WinExec
}
Class:Cmd{
    [Public]clear()->N:={
        return(WinExec(&"cmd /Q /c @cls",0));
    }
    [Public]nextl()->N:={
        return(WinExec(&"cmd /Q /c @echo.",0));
    }
    [Public]run(N:str):={
        string:T1.const(str);
        string:T2.const(&"cmd /Q /c ");
        T2=T2+T1;
        return(WinExec(T2.ad));
    }
}