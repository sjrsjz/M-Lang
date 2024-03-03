_string_{
    R2Str(R:data)->string:={
        string:tmp;
        tmp.size=0;
        tmp.ad=new(512);
        R2T(data,tmp.ad);
        while(N(tmp.ad+tmp.size)->B!=0 or N(tmp.ad+tmp.size+1)->B!=0){tmp.size=tmp.size+2};
        tmp.size=tmp.size+2;
        return(tmp)
    }
}
Class:string{
    Public N:ad;Public N:size;
    Public _init_()->N:={
        ad=0;size=0
    }
    Public _destroy_()->N:={
        if(ad!=0){free(ad);ad=0};
        size=0;
    }
    _string_:={
        N:ad,N:size
    }
    Public "return(string)"(string:s)->N:={//prevent RAII destroys data
        _destroy_();
        ad=new(s.size);
        size=s.size;
        memcopy(ad,s.ad,size);
    }
    Public =(string:s)->string:={
        _destroy_();
        ad=new(s.size);
        
        size=s.size;
        memcopy(ad,s.ad,s.size);

        return(this)
    }
    Public const(N:str)->N:={
        _destroy_();
        N:i=0;
        while(N(str+i)->B!=0 or N(str+i+1)->B!=0){i=i+2};
        size=i+2;
        ad=new(size);
        memcopy(ad,str,size);
    }
    Public +(string:s)->string:={
        string:tmp;
        if(size!=0){
            tmp.size=s.size+size-2;
            tmp.ad=new(tmp.size);
            memcopy(tmp.ad,ad,size);
            memcopy(tmp.ad+size-2,s.ad,s.size);
        }{
            tmp=s;
        };
        return(tmp)
    }
    Public *(N:times)->string:={
        string:tmp;
        N:i=0;
        while(i<times){
            tmp=tmp+this;
            i=i+1
        };
        return(tmp)
    }
    Public "R()"()->R:={
        return(T2R(ad))
    }
    Public ==(string:s)->Boolen:={
        return(CmpStr(ad,s.ad))
    }
    Public !=(string:s)->Boolen:={
        return(not CmpStr(ad,s.ad))
    }
    Public "N()"()->N:={
        return(ad)
    }
    Public "sizeof()"()->N:={
        return(size/2)
    }
    Public "="(B:str[0])->N:={
        const(str)
    }
    Public "="(R:data)->N:={
        this=R2Str(data)
    }
    Public _new_(B:str[0])->string:={
        const(str);
        return(this)
    }
    Public _new_(N:str)->string:={
        const(str);
        return(this)
    }

}
