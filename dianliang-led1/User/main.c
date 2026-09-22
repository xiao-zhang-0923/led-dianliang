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
