##include<"string.ml">;
Extra:"user32.dll"{
	msgbox(N:a,N:title,N:msg,N:b)->N:=MessageBoxA
}
Main{
    main()->N:={
        string:str1;
        string:str2; 
        string:str3;
        B:buffer[128];
        
        str1.const(&"Hello ");
        str2.const(&"World");       
        str3.const(&"! ");        
        str2=str1+str2+str3+R2Str(100);
        str3.const(&"Hello World! 100");
        print(str2.ad);
        if(str2==str3){
            print(&" Equal");
        }{
            print(&" Different")
        };
        str1.const(&"
");
        str3=str1+str3;
        str3=str3*10;
        print(str3.ad);
        N:g;
       
        
        input(&buffer,128);
    }
}