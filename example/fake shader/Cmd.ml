Extra:"kernel32.dll"{
    WinExec(N:lpCmdLine,N:show)->N:=WinExec
}
Class:Cmd{
    clear()->N:={
        return(WinExec(&"cmd /Q /c @cls",0));
    }
    nextl()->N:={
        return(WinExec(&"cmd /Q /c @echo.",0));
    }
    run(N:str):={
        string:T1.const(str);
        string:T2.const(&"cmd /Q /c ");
        T2=T2+T1;
        return(WinExec(T2.ad));
    }
}