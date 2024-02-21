// Simple fan control utility for MSI WIND / LG X110 Netbooks on Linux
// by Jonas Diemer (diemer@gmx.de)
// Based on LGXfan created by Tord Lindstrom (pukko@home.se)

// Compile: "cc     kb3700_fanctrl.c   -o kb3700_fanctrl"
// Run as root.


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/io.h>
#include <unistd.h>


#define EC_SC 0x66
#define EC_DATA 0x62


#define IBF 1
#define OBF 0
#define EC_SC_READ_CMD 0x80
#define EC_SC_WRITE_CMD 0x81
#define EC_SC_SCI_CMD 0x84

// EC addresses
#define EC_TEMP 0x68
#define EC_CURRENT_FAN_SPEED 0x71
#define EC_TEMP_THRESHOLDS_1 0x69
#define EC_FAN_SPEEDS_1 0x72

// Hysteresis: fan 1->0 8deg, fan 2->1 4 deg 3->2 4 deg
// "Good values": Fan speeds:   0  31  45  60  65  80  80  85
//                Temp thresh   0  50  59  64  76  80  85  93

// Not used fan values (leftovers from a second fan?)
#define EC_TEMP_THRESHOLDS_2 0x81
#define EC_FAN_SPEEDS_2 0x89

static int wait_ec(const uint32_t port, const uint32_t flag, const char value)
{
	uint8_t data;
	int i;

	i = 0;
	data = inb(port);

	while ( (((data >> flag)&0x1)!=value) && (i++ < 100)) {
		usleep(1000);
		data = inb(port);
	}
	if (i >= 100)
	{
		printf("wait_ec error on port 0x%x, data=0x%x, flag=0x%x, value=0x%x\n", port, data, flag, value);
		return 1;
	}

	return 0;
}

// For read_ec & write_ec command sequence see
// section "4.10.1.4 EC Command Program Sequence" in
// http://wiki.laptop.org/images/a/ab/KB3700-ds-01.pdf 
static uint8_t read_ec(const uint32_t port)
{
	uint8_t value;

	wait_ec(EC_SC, IBF, 0);
	outb(EC_SC_READ_CMD, EC_SC);

	wait_ec(EC_SC, IBF, 0);
	outb(port, EC_DATA);

	//wait_ec(EC_SC, EC_SC_IBF_FREE);
	wait_ec(EC_SC, OBF, 1);
	value = inb(EC_DATA);

	return value;
}

static void write_ec(const uint32_t port, const uint8_t value)
{
	wait_ec(EC_SC, IBF, 0);
	outb(EC_SC_WRITE_CMD, EC_SC);

	wait_ec(EC_SC, IBF, 0);
	outb(port, EC_DATA);

	wait_ec(EC_SC, IBF, 0);
	outb(value, EC_DATA);

	wait_ec(EC_SC, IBF, 0);

	return;
}

static void do_ec(const uint32_t cmd, const uint32_t port, const uint8_t value)
{
	wait_ec(EC_SC, IBF, 0);
	outb(cmd, EC_SC);

	wait_ec(EC_SC, IBF, 0);
	outb(port, EC_DATA);

	wait_ec(EC_SC, IBF, 0);
	outb(value, EC_DATA);

	wait_ec(EC_SC, IBF, 0);

	return;
}

static void dump_fan_config(void)
{
	printf("Dump FAN\n");
        int raw_duty1 = read_ec(0xCE);
        int val_duty1 = (int) ((double) raw_duty1 / 255.0 * 100.0);
	int raw_rpm1 = (read_ec(0xD0) << 8) + (read_ec(0xD1));
	int val_rpm1;
	if (raw_rpm1 == 0)
		val_rpm1 = 0;
	else
		val_rpm1 = 2156220 / raw_rpm1;
        int raw_duty2 = read_ec(0xCF);
        int val_duty2 = (int) ((double) raw_duty2 / 255.0 * 100.0);
	int raw_rpm2 = (read_ec(0xD2) << 8) + (read_ec(0xD3));
	int val_rpm2;
	if (raw_rpm2 == 0)
		val_rpm2 = 0;
	else
		val_rpm2 = 2156220 / raw_rpm2;
	printf("FAN1 Duty: %d %%\n", val_duty1);
	printf("FAN1 RPMs: %d RPM\n", val_rpm1);
	printf("FAN2 Duty: %d %%\n", val_duty2);
	printf("FAN2 RPMs: %d RPM\n", val_rpm2);
	printf("CPU Temp: %d °C\n", read_ec(0x07));
}

static void test_fan_config(int fanidx, int duty_percentage)
{
	double v_d = ((double) duty_percentage) / 100.0 * 255.0;
	int v_i = (int) v_d;
	printf("Test FAN %d%% to %d\n", duty_percentage, v_i);
	if (fanidx == 1) do_ec(0x99, 0x01, v_i);
	if (fanidx == 2) do_ec(0x99, 0x02, v_i);
	dump_fan_config();
}


int main(int argc, char *argv[])
{
    int Result;

    printf("Simple fan control utility for MSI Wind and clones\n");
    printf("USE AT YOUR OWN RISK!!\n");

    // 初始化I/O权限
    Result = ioperm(0x62, 1, 1);
    Result += ioperm(0x66, 1, 1);

    // 检查I/O权限是否成功设置
    if (Result == 0) {
        // 跳过程序名称参数
        argc--;
        argv++;

        // 检查参数数量并相应地调用函数
        if (argc == 0) {
            // 没有提供额外参数，调用dump_fan_config
            dump_fan_config();
        } else if (argc == 2) {
            // 提供了两个参数，将它们转换为整数并调用test_fan_config
            int param1 = atoi(argv[0]);
            int param2 = atoi(argv[1]);
            test_fan_config(param1, param2);
        } else {
            // 参数数量不符合要求，打印错误信息
            printf("Invalid number of arguments. Please provide two integers as arguments.\n");
            exit(1);
        }
    } else {
        printf("ioperm() failed!\n");
        exit(1);
    }
    return 0;
}

