Class:Shader{
    [Public]color(R:x,R:y,N:frame)->Boolen:={
        R:d=x*x+y*y;
        N:k=frame-N(frame/10.0)*10+5;
        return(d>0.8-k*0.1 and d<0.9);
    }
}