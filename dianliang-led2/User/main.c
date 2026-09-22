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

    //全部LED初始状态熄灭，输出高电平（低电平点亮）
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