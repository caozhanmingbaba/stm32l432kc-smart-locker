#include "mbed.h"

#define Temp_Reg 0x00
#define Config_Reg 0x01
#define TLow_Reg 0x02
#define THigh_Reg 0x03
I2C TMP102(D1, D0);
const int TMP102Address = 0x90;
char ConfigRegisterTMP102[3];
char TemperatureRegister[2];
float Temperature;


DigitalOut buzzer(D2);  // 假设蜂鸣器连接到 D2 引脚

// 定义 SPI 引脚和常量
#define ALERT_THRESHOLD 3.0 // 设置加速度警报阈值为 3g
#define MOSI A6    // SDA
#define MISO A5    // SDO
#define SCLK A4    // SCL
#define CS_PIN D12 // CS

// 按钮定义
#define button1 D3  // 上按钮
#define button2 D10  // 下按钮

// ADXL345 寄存器地址
#define REG_DATA_FORMAT 0x31 // 数据格式控制寄存器 (DATA_FORMAT)
#define REG_POWER_CTL   0x2D // 电源控制寄存器 (POWER_CTL)
#define REG_BW_RATE     0x2C // 数据速率和带宽控制寄存器 (BW_RATE)
#define REG_DATAX0      0x32 // X轴数据的第一个字节寄存器 (DATAX0), 后续为DATAX1, DATAY0, DATAY1, DATAZ0, DATAZ1

// ADXL345 配置值
#define DATA_FORMAT_CONFIG 0x0B  // 数据格式配置 (REG_DATA_FORMAT, 地址 0x31)
#define POWER_CTL_CONFIG   0x08  // 电源控制配置 (REG_POWER_CTL, 地址 0x2D)
#define BW_RATE_CONFIG     0x0D  // 带宽速率配置 (REG_BW_RATE, 地址 0x2C)
#define Read_BIT           0x80  // SPI读取操作位: 当进行SPI读取时，寄存器地址的最高位 (MSB) 需要置1。

// SPI 通信参数
#define SPI_BIT_WIDTH    8       // SPI 数据位宽 (8位)
#define SPI_MODE         3       // SPI 模式 (CPOL=1, CPHA=1)
#define SPI_FREQUENCY    2000000 // SPI 通信频率 (2MHz)

// LED灯定义
DigitalOut green_led(D6);  // 绿色LED灯
DigitalOut red_led(D9);    // 红色LED灯

int input_sequence[4];  
int current_index = 0;
int correct_password[4] = {1, 1, 0, 0};  //密码上上下下

SPI spi(MOSI, MISO, SCLK);  // SPI通信对象
DigitalOut chipSelect(CS_PIN);  // 片选引脚

char buffer[6];              
int16_t accelData[3];       
float accel_in_G_form[3];    // 存储加速度数据（单位：g）

const float conversionFactor = 0.004;  // 转换因子


// 延时函数 (微秒级)
void delay_us(int us) {
    wait_us(us);  // 等待指定的微秒时间
}

// 密码验证函数
bool check_password() {
    for (int i = 0; i < 4; i++) {
        if (input_sequence[i] != correct_password[i]) {
            return false;
        }
    }
    return true;
}


// 配置加速度计
void configureAccelerometer() {
    chipSelect = 1;  
    spi.format(8, 3);  // 设置 SPI 数据格式
    spi.frequency(2000000);  // 设置 SPI 通信频率 
    
    // 配置数据格式寄存器
    chipSelect = 0;                     
    spi.write(REG_DATA_FORMAT);         
    spi.write(DATA_FORMAT_CONFIG);      
    chipSelect = 1;                     
    
    // 配置带宽速率寄存器
    chipSelect = 0;                     
    spi.write(REG_BW_RATE);             
    spi.write(BW_RATE_CONFIG);          
    chipSelect = 1;                     
    
    // 配置电源控制寄存器
    chipSelect = 0;                     
    spi.write(REG_POWER_CTL);           
    spi.write(POWER_CTL_CONFIG);        
    chipSelect = 1;                     
}

// 读取加速度计数据
void readAccelerometerData() {
    chipSelect = 0;  // 激活片选信号
    spi.write(REG_DATAX0 | Read_BIT| 0x40);  // 发送寄存器地址并设置读取操作
    for (int i = 0; i < 6; i++) {
        buffer[i] = spi.write(0x00);  // 读取数据
    }
    chipSelect = 1;  // 取消片选信号

    // 解析数据
    accelData[0] = (int16_t)((buffer[1] << 8) | buffer[0]);  // X轴数据
    accelData[1] = (int16_t)((buffer[3] << 8) | buffer[2]);  // Y轴数据
    accelData[2] = (int16_t)((buffer[5] << 8) | buffer[4]);  // Z轴数据
    
    // 转换为重力加速度单位 (g)
    for (int i = 0; i < 3; i++) {
        accel_in_G_form[i] = conversionFactor * accelData[i];  // 转换为重力加速度单位
    }
}

// 计算加速度模
float calculateMagnitude(float x, float y, float z) {
    return sqrt(x * x + y * y + z * z); // 计算三维加速度的模
}

// 检查加速度模是否超过阈值并触发蜂鸣器报警
void checkForAlert() {
    float magnitude = calculateMagnitude(accel_in_G_form[0], accel_in_G_form[1], accel_in_G_form[2]);
    if (magnitude > ALERT_THRESHOLD) {
        buzzer = 1;  // 启动蜂鸣器
        delay_us(500000);  // 蜂鸣器响 0.5 秒
        buzzer = 0;  // 关闭蜂鸣器
    }
}

// 显示加速度数据
void displayAccelerometerData() {
    printf("===============\n");
    printf("x = %+1.2fg\n", accel_in_G_form[0]);
    printf("y = %+1.2fg\n", accel_in_G_form[1]);
    printf("z = %+1.2fg\n", accel_in_G_form[2]);
}


//温度计函数
void ConfigureTMP102()
{
ConfigRegisterTMP102[0] = Config_Reg;
ConfigRegisterTMP102[1] = 0x61; // Byte 1
ConfigRegisterTMP102[2] = 0xA0; // Byte 2
TMP102.write(TMP102Address, ConfigRegisterTMP102, 3);
}

void Set_Thermostat_Temp_Low()
{
ConfigRegisterTMP102[0] = TLow_Reg;
ConfigRegisterTMP102[1] = 0x14; // Byte 1 --20 Degrees Celsius
ConfigRegisterTMP102[2] = 0x00; // Byte 2
TMP102.write(TMP102Address, ConfigRegisterTMP102, 3);
}

void Set_Thermostat_Temp_High()
{
ConfigRegisterTMP102[0] = THigh_Reg;
ConfigRegisterTMP102[1] = 0x1A; // Byte 1
ConfigRegisterTMP102[2] = 0x00; // Byte 2
TMP102.write(TMP102Address, ConfigRegisterTMP102, 3);
}

void Set_Alert_Polarity()
{
// Setting the POL bit to 1 for active-high ALERT pin
ConfigRegisterTMP102[0] = Config_Reg;
ConfigRegisterTMP102[1] |= 0x02;  // Set POL bit to 1 (active-high ALERT)
TMP102.write(TMP102Address, ConfigRegisterTMP102, 3);
}



int main() {
    configureAccelerometer();  // 配置加速度计
    
    
    unsigned short M;
    char L;
    ConfigureTMP102();
    Set_Thermostat_Temp_Low();
    Set_Thermostat_Temp_High();
    ConfigRegisterTMP102[0] = Temp_Reg;
    TMP102.write(TMP102Address, ConfigRegisterTMP102, 1);

    green_led = 0;  
    red_led = 0;
    
    while (true) {
        // 读取加速度计数据并显示
        readAccelerometerData();
        displayAccelerometerData();

        // 检查是否超过加速度阈值并报警
        checkForAlert();
        
        // 确保加速度计和按钮检测之间没有冲突
        delay_us(50000);  // 微秒级延时，避免过长延时

        // 检测按钮状态
    
    TMP102.read(TMP102Address, TemperatureRegister, 2);
    M = TemperatureRegister[0] << 4;
    L = TemperatureRegister[1] >> 4;
    M = M + L;

    // Convert the result to temperature in Celsius
    Temperature = 0.0625 * M;

    // Print the temperature to the console (optional)
    printf("Temperature Register Value = %u\n\r", M);
    printf("Temperature in Celsius= %.3f\n\r", Temperature);
    
    
    if (button1 == 1) {  // 上按钮被按下
    delay_us(300000);  // 防止抖动
    input_sequence[current_index] = 1;  // 输入1（代表上按钮）
    current_index++;  // 移动到下一个输入位置
}

    if (button2 == 1) {  // 下按钮被按下
    delay_us(300000);  // 防止抖动
    input_sequence[current_index] = 0;  // 输入0（代表下按钮）
    current_index++;  // 移动到下一个输入位置
}


        // 检查是否输入完毕
    if (current_index == 4) {
    if (check_password()) {
        green_led = 1;  // 开启绿色LED
        delay_us(500000);  // 闪烁效果
        green_led = 0;
        delay_us(500000);
    } else {
        red_led = 1;  // 开启红色LED
        delay_us(500000);  // 闪烁效果
        red_led = 0;
        delay_us(500000);
    }
    current_index = 0;  // 重置输入状态
    delay_us(500000);  // 延时，防止过快进入下一轮输入
    }

    }


}
