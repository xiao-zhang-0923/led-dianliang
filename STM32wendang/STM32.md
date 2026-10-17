# STM32多端口GPIO流水灯寄存器实现

[TOC]

## 0 前言

作为嵌入式新手，本次实验没有使用HAL库，可直接查阅STM32参考手册，通过寄存器地址操作GPIO。实验过程中会遇到外设时钟、掩码等诸多问题。本文记录入门概念、硬件接线、代码，以及同学们在操作过程的踩坑点，加深对寄存器映射和GPIO的理解。

## 1 STM32寄存器底层逻辑

### 1.1什么是STM32 GPIO

基于电路的知识来说，GPIO叫通用输入输出引脚，是STM32单片机的基础外设，外设是集成在芯片内部独立硬件功能模块，是芯片CPU与外界的桥梁。GPIO可以读取外部的电平，也可以输出高低电平。

STM32的GPIO总共有8种工作模式：输入浮空、输入上拉、输入下拉、模拟输入、开漏输出、推挽式输出、推挽式复用功能、开漏复用功能。

本次流水灯实验选用的是**通用推挽输出模式**，须把GPIO配置成输出模式，通过输出高、低电平来控制LED灯的亮灭。输入模式则可以添加按键开开关。

STM32把GPIO引脚分成了GPIOA、GPIOB、GPIOC三个组，每一组下面最多有16个引脚，命名为Px0‑Px15。详细引脚图如下：

![image-20260921000921741](STM32.assets/image-20260921000921741.png)

### 1.2寄存器映射

这里我要说到存储器映射和寄存器映射。每个外设存在多个寄存器，每个寄存器都分配了唯一的寄存器地址。芯片厂商需要为各个片上外设分配内存地址，作为外设的**基地址**区间，这个过程叫做**存储器映射**。而**寄存器映射**是在此基础上利用外设基地址加上寄存器**偏移地址**，基地址与偏移地址组合为**寄存器地址**。

依靠寄存器映射，把硬件寄存器地址映射到内存地址空间。CPU通过这个地址，找到对应的寄存器，完成读写操作，以此控制外设。

举个例子，GPIOA 有自己的基地址0x40010800，再加上 0x0C 偏移，就得到 GPIOA_ODR 输出寄存器的地址。

### 1.3外设时钟

STM32上电之后，时钟默认关闭，需要先在寄存器RCC_APB2ENR把对应端口组别的时钟打开才能操作GPIO。

:red_circle:注意：时钟是按**整个端口组开启的**。GPIOA时钟，是PA0-PA15全部引脚获得时钟。只有开启时钟，配置GPIO寄存器才会生效，LED灯实现亮灭。

在第2节我会详细写出如何查找外设寄存器地址。

## 2 基于寄存器方式实现流水灯程序编写

## 2.1点亮流水灯

我们主要归结为以下几个步骤：

:large_blue_diamond:打开GPIO外设时钟

![image-20260921132245567](STM32.assets/image-20260921132245567.png)

可以看出，总线矩阵连接到RCC，APB2 总线接GPIOA、GPIOB、GPIOC 。RCC 管理外设时钟，向 GPIO提供时钟信号。所以我们来查阅STM32中文参考手册得到他们的寄存器地址。

外设时钟寄存器地址

![image-20260921183333171](STM32.assets/image-20260921183333171.png)

可知RCC的基地址为0x40021000。

![image-20260921184417234](STM32.assets/image-20260921184417234.png)

RCC的偏移地址为0x18，RCC基地址0x40021000加上偏移地址0x18构成寄存器地址0x40021018，就可以定位RCC内寄存器。

打开外部时钟

位2、位3、位4、分别是GPIOA、GPIOB、GPIOC的端口时钟，对应位写1，为打开时钟，写 0 关闭。代码编写如下：

```c
//宏定义寄存器地址方便编程
#define RCC_APB2ENR  (*(unsigned int *)0x40021018)
   //开启GPIOA（数字1左移两位得到00000100位2)、GPIOB(位3)、GPIOC(位4)时钟
    RCC_APB2ENR |= (1<<2) | (1<<3) | (1<<4);
```

对应GPIOA、GPIOB、GPIOC时钟打开。

:large_blue_diamond:GPIO寄存器地址,配置GPIO推挽输出

本实验我设置的是PA0、PB1、PC14三个引脚：

![image-20260921213201332](STM32.assets/image-20260921213201332.png)

可以看到A、B、C三类端口基地址。

![image-20260922161721551](STM32.assets/image-20260922161721551.png)

![image-20260922162142595](STM32.assets/image-20260922162142595.png)

因为STM32为32bit处理器，每个引脚控制四个bit，故有8位引脚，分高位和低位。当GPIO引脚为0-7，如PA1-pA7,为低寄存器_CRL,偏移地址为0x00,高寄存器CRH偏移地址为0x04，所以PA0为0x40010800，PB1为0x40010C00，PC13/14为0x40011004。

```c
#define GPIOA_CRL    (*(unsigned int *)0x40010800)
#define GPIOB_CRL    (*(unsigned int *)0x40010C00)
#define GPIOC_CRH    (*(unsigned int *)0x40011004)
```

PA0控制bit0-3,设置最大速率2MHZ,配置码则为0x00000002，通过掩码先清除4个配置位,再写入配置值,以此来配合灯推挽输出。

```c
 //---------- PA0 CRL bit0‑3推挽输出2MHz ----------
    GPIOA_CRL &= 0xFFFFFFF0;
    GPIOA_CRL |= 0x00000002;

    //---------- PB1 CRL bit4‑7推挽输出2MHz ----------
    GPIOB_CRL &= 0xFFFFFF0F;
    GPIOB_CRL |= 0x00000020;

    //---------- PC13 GPIOC_CRH bit20‑23，推挽输出2MHz ----------
    GPIOC_CRH &= 0xFF0FFFFF;
    GPIOC_CRH |= 0x00200000;

    //---------- PC14 GPIOC_CRH bit24‑27，推挽输出2MHz ----------
    GPIOC_CRH &= 0xF0FFFFFF;
    GPIOC_CRH |= 0x02000000;
```

![image-20260921213731141](STM32.assets/image-20260921213731141.png)

除了配置好GPIO工作模式，还需要输出数据，偏移地址为0C,所以我们将应交的后两位换成0C为寄存器ODR用来输出数据。

```c
#define GPIOA_ODR    (*(unsigned int *)0x4001080C)
#define GPIOB_ODR    (*(unsigned int *)0x40010C0C)
#define GPIOC_ODR    (*(unsigned int *)0x4001100C)
```

:large_blue_diamond:低电平点亮LED灯

以点亮PA0为例，将1移位到对应位上，如0位：1<<0,PB1:1<<1,PC13<<13.y因为低电平点亮，在PA0引脚上取反。流水灯再添加while循环，1s延时。

```c
   void delay_1s(void)
{
    unsigned int i,j;
    for(i=0;i<800;i++)
        for(j=0;j<12000U;j++);
}
   while(1)
    {
        //1.点亮PC14，PA0、PB1、PC13熄灭
        GPIOA_ODR |=  (1<<0);
        GPIOB_ODR |=  (1<<1);
        GPIOC_ODR |=  (1<<13);
        GPIOC_ODR &= ~(1<<14);
        delay_1s();

        //2.点亮PA0，PC14、PB1、PC13熄灭
        GPIOA_ODR &= ~(1<<0);
        GPIOB_ODR |=  (1<<1);
        GPIOC_ODR |=  (1<<13) | (1<<14);
        delay_1s();

        //3.点亮PB1，PC14、PA0、PC13熄灭
        GPIOA_ODR |=  (1<<0);
        GPIOB_ODR &= ~(1<<1);
        GPIOC_ODR |=  (1<<13) | (1<<14);
        delay_1s();

        //4.点亮PC13板载LED，PC14、PA0、PB1熄灭
        GPIOA_ODR |=  (1<<0);
        GPIOB_ODR |=  (1<<1);
        GPIOC_ODR |=  (1<<14);
        GPIOC_ODR &= ~(1<<13);
        delay_1s();
    }
```

## 2.2程序编写

:large_blue_diamond:添加启动问件，他是stm32启动文件md.s是寄存器方式编写的必要文件，可以去ARM官网上查找。

![image-20260921135944890](STM32.assets/image-20260921135944890.png)

![image-20260921140059406](STM32.assets/image-20260921140059406.png)

:large_blue_diamond:其次创建main.c文件编写我们的主程序

![image-20260921140357342](STM32.assets/image-20260921140357342.png)

![image-20260921140318219](STM32.assets/image-20260921140318219.png)

:red_circle:注意，两个重要文件都需放在项目文件夹下。

由于上文已经详细说明各个代码模块编写思路，这里附上完整代码：

:large_blue_diamond:第一个实验为3个LED灯，轮流闪烁，间隔时长1秒。

```c
typedef unsigned int  unsignedint;

#define RCC_APB2ENR  (*(unsigned int *)0x40021018)

#define GPIOA_CRL    (*(unsigned int *)0x40010800)
#define GPIOA_ODR    (*(unsigned int *)0x4001080C)

#define GPIOB_CRL    (*(unsigned int *)0x40010C00)
#define GPIOB_ODR    (*(unsigned int *)0x40010C0C)

#define GPIOC_CRH    (*(unsigned int *)0x40011004)
#define GPIOC_ODR    (*(unsigned int *)0x4001100C)

//软件近似1s延时
void delay_1s(void)
{
    unsigned int i,j;
    for(i=0;i<800;i++)
        for(j=0;j<12000U;j++);
}

void LED_GPIO_Init(void)
{
    //开启GPIOA(bit2)、GPIOB(bit3)、GPIOC(bit4)时钟
    RCC_APB2ENR |= (1<<2) | (1<<3) | (1<<4);

    //---------- PA0配置 CRL bit0‑3 推挽输出 2MHz  0010(0x2) ----------
    GPIOA_CRL &= 0xFFFFFFF0;   //清除PA0对应的4位
    GPIOA_CRL |= 0x00000002;

    //---------- PB1配置 CRL bit4‑7 推挽输出2MHz ----------
    GPIOB_CRL &= 0xFFFFFF0F;  
    GPIOB_CRL |= 0x00000020;

    //---------- PC14配置 GPIOC_CRH bit24~27 推挽输出2MHz ----------
    GPIOC_CRH &= 0xF0FFFFFF;  
    GPIOC_CRH |= 0x02000000;

    //全部LED初始熄灭，输出高电平（低电平点亮）
    GPIOA_ODR |= (1<<0);
    GPIOB_ODR |= (1<<1);
    GPIOC_ODR |= (1<<14);
}

int main(void)
{
    LED_GPIO_Init();
    while(1)
    {
        //点亮PA0，PB1、PC14熄灭
        GPIOA_ODR &= ~(1<<0);
        GPIOB_ODR |=  (1<<1);
        GPIOC_ODR |=  (1<<14);
        delay_1s();

        //点亮PB1，PA0、PC14熄灭
        GPIOA_ODR |=  (1<<0);
        GPIOB_ODR &= ~(1<<1);
        GPIOC_ODR |=  (1<<14);
        delay_1s();

        //点亮PC14，PA0、PB1熄灭
        GPIOA_ODR |=  (1<<0);
        GPIOB_ODR |=  (1<<1);
        GPIOC_ODR &= ~(1<<14);
        delay_1s();
    }
}
```

:large_blue_diamond:第二个实验需要把板载灯PC13加入流水灯中,，我需要把PC13也加入循环中，完整代码如下:

```c
typedef unsigned int  unsignedint;

#define RCC_APB2ENR  (*(unsigned int *)0x40021018)

#define GPIOA_CRL    (*(unsigned int *)0x40010800)
#define GPIOA_ODR    (*(unsigned int *)0x4001080C)

#define GPIOB_CRL    (*(unsigned int *)0x40010C00)
#define GPIOB_ODR    (*(unsigned int *)0x40010C0C)

#define GPIOC_CRH    (*(unsigned int *)0x40011004)
#define GPIOC_ODR    (*(unsigned int *)0x4001100C)

//软件近似1s延时
void delay_1s(void)
{
    unsigned int i,j;
    for(i=0;i<800;i++)
        for(j=0;j<12000U;j++);
}

void LED_GPIO_Init(void)
{
    //开启GPIOA(bit2) 、GPIOB(bit3)、GPIOC(bit4)时钟
    RCC_APB2ENR |= (1<<2) | (1<<3) | (1<<4);

    //---------- PA0 CRL bit0‑3 推挽输出2MHz ----------
    GPIOA_CRL &= 0xFFFFFFF0;
    GPIOA_CRL |= 0x00000002;

    //---------- PB1 CRL bit4‑7 推挽输出2MHz ----------
    GPIOB_CRL &= 0xFFFFFF0F;
    GPIOB_CRL |= 0x00000020;

    //---------- PC13 GPIOC_CRH bit20‑23 备份域IO，推挽输出2MHz ----------
    GPIOC_CRH &= 0xFF0FFFFF;
    GPIOC_CRH |= 0x00200000;

    //---------- PC14 GPIOC_CRH bit24‑27 备份域IO，推挽输出2MHz ----------
    GPIOC_CRH &= 0xF0FFFFFF;
    GPIOC_CRH |= 0x02000000;

    //全部LED初始状态熄灭，输出高电平
    GPIOA_ODR |= (1<<0);
    GPIOB_ODR |= (1<<1);
    GPIOC_ODR |= (1<<13) | (1<<14);
}

int main(void)
{
    LED_GPIO_Init();
    while(1)
    {
        //1.点亮PC14，PA0、PB1、PC13熄灭
        GPIOA_ODR |=  (1<<0);
        GPIOB_ODR |=  (1<<1);
        GPIOC_ODR |=  (1<<13);
        GPIOC_ODR &= ~(1<<14);
        delay_1s();

        //2.点亮PA0，PC14、PB1、PC13熄灭
        GPIOA_ODR &= ~(1<<0);
        GPIOB_ODR |=  (1<<1);
        GPIOC_ODR |=  (1<<13) | (1<<14);
        delay_1s();

        //3.点亮PB1，PC14、PA0、PC13熄灭
        GPIOA_ODR |=  (1<<0);
        GPIOB_ODR &= ~(1<<1);
        GPIOC_ODR |=  (1<<13) | (1<<14);
        delay_1s();

        //4.点亮PC13板载LED，PC14、PA0、PB1熄灭
        GPIOA_ODR |=  (1<<0);
        GPIOB_ODR |=  (1<<1);
        GPIOC_ODR |=  (1<<14);
        GPIOC_ODR &= ~(1<<13);
        delay_1s();
    }
}
```

## 3 硬件接线与实验现象说明

如图是简易接线图，其中我的蓝灯、红灯、绿灯分别放到了PA0、PB1、PC14引脚。

![image-20260921141548523](STM32.assets/image-20260921141548523.png)

烧录我选择stlink下载。



![image-20260921140820661](STM32.assets/image-20260921140820661.png)

编译无错误后，再进行烧录。



![image-20260922002906626](STM32.assets/image-20260922002906626.png)

:large_blue_diamond:第一个实验为3个LED灯，轮流闪烁，间隔时长1秒。

实验现象为红蓝绿三个灯轮流亮：

![image-20260922003749853](STM32.assets/image-20260922003749853.png)

:large_blue_diamond:第二个实验需要把板载灯PC13加入流水灯中,，我需要把PC13也加入循环中。

```c
    GPIOC_ODR |= (1<<13) | (1<<14);

        //4.点亮PC13，PC14、PA0、PB1熄灭
        GPIOA_ODR |=  (1<<0);
        GPIOB_ODR |=  (1<<1);
        GPIOC_ODR |=  (1<<14);
        GPIOC_ODR &= ~(1<<13);
        delay_1s();

```

实验现象如下：

![image-20260922004608633](STM32.assets/image-20260922004608633.png)

可以看到PC13成功点亮。

## 4 总结

从寄存器方式实现流水灯，我更加深刻的理解到了寄存器的工作原理。首先需要了解位操作<<移位、|=赋值、&=与、~取反在单片机寄存器配置中的实际用途。再到写入时钟寄存器地址，可以向外设GPIO下达指令，RCC_APB2ENR开启时钟信号，CRL、CRH设置GPIO工作方式，每一个GPIO引脚占用连续4个bit位，ODR输出寄存器负责控制引脚输出高低电平等作用，读取地址了解引脚状态来驱动外部器件工作。该流水灯为低电平（LED3.3V与0V引脚电压差）点亮 LED，因此将对应 bit 清零时灯点亮，置 1 时灯熄灭。其中，学会看懂查找板子手册也很重要，学习如何查找寄存器地址，映射到程序编写，引脚接线，再进行烧录，操作一个比较完整的的项目，不断排查错误，锻炼了我的完整项目实操能力。