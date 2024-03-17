# M Lang帮助文档

## M Lang简介

M Lang是一个C++编写的、高度轻量级的静态强类型编译性语言，支持Unicode

## M Lang用法

M Lang编译器用于编译与执行M Lang程序（M Lang目前不支持生成PE格式文件，必须带壳加载）

##### M Lang编译与运行

在命令行中运行：`"M Lang"`进行查看编译器帮助

M Lang有三种编译模式:

`buildIR source ir`编译源程序到中间代码

`buildMEXE ir mexe`编译中间代码到可执行文件（非PE格式！必须带壳加载）

`build source mexe`编译源程序到可执行文件（非PE格式！必须带壳加载）

`run source`直接编译源程序并运行

``runIR ir``编译中间代码并运行

``runMEXE mexe``运行可执行文件（非PE格式！必须带壳加载）

``mexe``运行可执行文件（非PE格式！必须带壳加载）

## M Lang语法（x86）

##### 类型

M Lang默认支持五种基本类型，分别是：

**N** *32位无符号整数*

**Z** *32位整数*

**R** *双精度小数*

**B** *字节*

**Boolen** *布尔值*

##### 运算

M Lang支持自动类型转换，默认转换方向是 **B** -> **N/Z** -> **R**

M Lang支持如下运算符，按优先级排序为

1. **&** *取地址*    **~** *取函数地址/当前地址*     **.** *成员访问/方法调用*     **sizeof:type / sizeof:(expr)** *计算类型大小*    **typeof:expr / typeof:(expr)** *计算类型字符串* 

2. **->** *指针到类型*     **+** *绝对值*     **-** *相反数*     **not** *逻辑非*

3. **\* ** *乘法*     **/** *除法* 

4. **%** *求余*   **\\** *整除*

5. **+** *加法*     **-** *减法*

6. **and** *逻辑与* 

7. **or** *逻辑或*     **xor** *逻辑异或*

8. **=** *赋值*

###### & 运算符

**&** 运算符只能取出传递指针的表达式的指针，返回一个 **N** 类型

###### -> 运算符

**->** 运算符是一个类型不安全的运算符，用法为 `address->type` 其中 *address* 是 **N** 类型的数据指针，*type* 是类型名称，该运算符会无条件将 *address* 指向的数据当作 *type* 类型进行处理

###### 逻辑运算

M Lang内置了四种 ~~布尔~~ 逻辑运算： **and** , **or** , **xor** , **not**

M Lang的 **Boolen** 类型的运算非常复杂，它的特性如下：

1. **Boolen** 类型实际上是32位单精度小数，也就是说明 **Boolen** 类型可以充当小数类型

2. 四种 ~~布尔~~ 逻辑运算实际都是浮点运算（这与一般的逻辑运算不同）

3. **Boolen** 类型的取值范围是 **0 ~ 1** ,尽管可以超出此范围，但是可能会导致一些奇怪的逻辑问题

4. 四种 ~~布尔~~ 逻辑运算的实际实现：
   
   1. A **or** B 等价于  *A + B - A * B*
   
   2. A **and** B 等价于 *A \* B*
   
   3. A **xor** B 等价于  *(1 - A * B) \* (A + B - A * B)*
   
   4. **not** A 等价于  *1 - A*

5. 当一个 **Boolen** 类型向 **N** 类型或 **Z** 类型转换时，相当于
   
   `ret = (int) (A > 0.5) ` 

6. 当非 **R** 类型向 **Boolen** 类型转换时，总是被转换成 0 或 1

7. 涉及到流程控制时，依据对应 ~~布尔~~ 值的概率随机选择分支

8. 此运算规则下德 · 摩根律仍然成立

9. **R** 类型与 **Boolen** 类型之间的转换与单精度、双精度小数之间的转换一致

###### 程序集

M Lang存在程序集的概念，它是一组函数/变量的集合，定义如下

```
SetName{
    body
}
```

*SetName* 是程序集的名称，*body* 是定义的程序集变量和函数

M Lang支持动态链接库的声明，格式为：

```
Extra:"DLL Path"{
    Name(Type:Arg1,Type:Arg2,...)->RetType:=ApiName;
    ...
}
```

###### 变量

M Lang是静态类型的，变量在使用前必须定义，定义格式为：

```
Public/Private Type:VarName    //定义类型为Type，名称为VarName的变量
Public/Private Type:VarName[dimension1][dimension2][...]    //定义类型为Type，名称为VarName的数组
```

*Type* 可以是基础类型，也可以是自定义的数据类型或类。Public/Private为可访问性，仅在类中生效,可忽略

变量的定义格式在程序集，函数，类，数据结构通用，参数声明也使用该格式

**注意：** M Lang存在RAII机制，会在变量的创建与释放时分别调用构造与析构函数（如果存在）

###### 函数

M Lang的函数定义格式如下

```
Transit ThisArg stdcall/cdecl Public/Private Name(Type:Arg1,Type:Arg2,...)->RetType:={
    body
}
```

*RetType* 为返回类型，可以为定长数组。Public/Private为可访问性，仅在类中生效，可忽略。stdcall/cdecl为函数平栈方式，默认为stdcall。Transit为是否为中转函数，带上该定义的函数函数体返回值类型是 **N** ，返回值必须是有效的函数地址，在执行结束后会直接跳转到返回的函数地址，ThisArg是中转后的this指针，Transit可忽略。

**注意：** 函数必须在程序集/类中定义，当在程序集中定义时，函数对全局公开，当在类中定义时，函数默认是私有的，可以通过加前缀控制其可访问性

###### 类

M Lang支持简单的类系统，定义格式如下

```
Class:Name{
    body
    _init_()->N:={
        //initialize
    }
    _destroy_()->N:={
        //destroy
    }
    Public "return(Type)"(TypeA:RetType)->N:={
        //return自动调用函数，用于防止自动释放机制破坏数据导致无法返回正确结果
    }
    Public "Type()"()->Type:={
        //自动类型转换调用
    }
    Public _new_(args)->Type:={
        //用于创建临时对象，即 ClassName(args) 的写法
    }
    Public "sizeof()"(args)->Type:={
        //重载 sizeof:(o,args)
    }
    Public "typeof()"(args)->Type:={
        //重载 typeof:(o,args)
    }
    Public operator(args)->Type:={
        //重载运算符
    }
}
```

*body* 中可定义成员以及方法。\_*init*\_ 是构造函数，\_*destroy*\_ 是析构函数，*return(Type)* 是返回值深复制，防止类（如String）释放导致数据异常

###### 字面量

M Lang中的字面量默认是允许跨行的且为 **B** 数组类型，用双引号包括。转义符支持 \\\\（反斜杠）、\"（引号）、\n（换行符）

如下：

```
"Hello World!"

"Hello\nWorld!"

"Hello
World!"
```

当 "\" 后不匹配转义的时候，解释为原始字面量

###### 内置函数

break()

continue()

Pause()

Block()

case(Boolen:condition){}

_IR\_(){"ir code"};

rand()->R

if(Boolen:bool){code};

if(Boolen:bool){codeA}{codeB};

while(Boolen:condition){code};

do_while(Boolen:condition){code};

for(init code;Boolen:condition;iterator){code};

return(object)

print(N:str1,N:str2,...)

printN(N:data)

printR(R:data)

printZ(Z:data)

printB(B:data)

printBoolen(Boolen:data)    true/false/unclear

srand(N:seed)

new(N:size)->N

free(N:ptr)

DebugOutput(N:str)

T2R(N:data)->R    string to real

CmpStr(N:str1,N:str2)->N

R2T(R:data,N:buffer)->N

input(N:buffer,N:num)->N

memcopy(N:dist,N:src,N:size)->N

CmpMem(N:ptrA,N:ptrB,N:size)->N



###### 注意:

在M Lang中分号之间的语句算一整个语句（即使语句里包含大括号包括的代码块），因而以下语句是不同的语句：

```
if(a){    //if a then A else B
    A
}{
    B
};
```

```
if(a){    //if a then A
    A
};{
    B
};
```

```
if(a)    //if a then A() else B()
    A()
    B()
```



这在 **case** 函数也有体现，具体为：

```
Block()    //不要在Block()后加分号！
    case(a){
    
    }
    case(b){
    
    }
    ...
    case(n){
    
    };    //一定要加分号(除了该语句是最后一句)结尾！！！
```
