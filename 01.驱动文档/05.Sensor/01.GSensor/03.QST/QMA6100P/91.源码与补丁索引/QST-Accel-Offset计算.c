
//ACC-offset校准参考文献：https://blog.csdn.net/sslive/article/details/52541634
/*
这里介绍一个简单的offset校准方法:设备正面朝上静止放置在水平面上时，加速度计
X/Y/Z轴分别取100个数据并分别求平均值，代入下面的公式求出offset(加速度计Z轴offset由于重力影响，
需要根据实际坐标轴方向加上或者减去重力g影响)；然后用加速度计的原始数据减去offset，得出校准后的数据。
*/

enum
{
	AXIS_X = 0,
	AXIS_Y = 1,
	AXIS_Z = 2,

	AXIS_TOTAL
};



static int cali_count = 0;
float offset_acc[3] = {0.0, 0.0, 0.0};
float accel_calibration_sum[3] = {0.0f, 0.0f, 0.0f};
#define MAX_CALI_COUNT      100
extern uint8_t Calibration_data[30];

uint8_t accel_init = 0;

void QstAccel_perform_calibration(float acc[3])
{
	unsigned char	buf_reg[6];
	short 				raw_acc_xyz[3];
	float 				acc_raw[3];
	uint8_t axis = 0;
	union byte_float
	{
		float float_data;
		uint8_t byte[4];
	} float_to_byte;

	ret = qma6100p_readreg(QMA6100P_XOUTL, buf_reg, 6);
	if(ret == QMA6100P_FAIL)
	{
		QMA6100P_ERR("read xyz read reg error!!!\n");
		return QMA6100P_FAIL;	
	}
	raw_acc_xyz[0] = (short)(((buf_reg[1]<<8))|(buf_reg[0]));
	raw_acc_xyz[1] = (short)(((buf_reg[3]<<8))|(buf_reg[2]));
	raw_acc_xyz[2] = (short)(((buf_reg[5]<<8))|(buf_reg[4]));
	
	raw_acc_xyz[0] = raw_acc_xyz[0]>>2;
	raw_acc_xyz[1] = raw_acc_xyz[1]>>2;
	raw_acc_xyz[2] = raw_acc_xyz[2]>>2;

		//m/s2
	acc_raw[0] = (float)(raw_acc_xyz[0] * ONE_G) / acc_lsb_div;
	acc_raw[1] = (float)(raw_acc_xyz[1] * ONE_G) / acc_lsb_div;
	acc_raw[2] = (float)(raw_acc_xyz[2] * ONE_G) / acc_lsb_div;

	if(imu_init != 1)//flash中已经存储校准标记？
	{
        if(cali_count == 0)
        {
            memset((void *)accel_calibration_sum, 0, sizeof(accel_calibration_sum));
            cali_count++;
        }
        else if(cali_count < MAX_CALI_COUNT)
        {
            for(axis = 0; axis < 3; axis++)
            {
                if(axis == 2)
				{
					accel_calibration_sum[axis] += (acc_raw[axis] - ONE_G);
				}
				else
				{
					accel_calibration_sum[axis] += acc_raw[axis];
				}
            }
            cali_count++;
        }
        else if(cali_count == MAX_CALI_COUNT)
        {
            for(axis = 0; axis < 3; axis++)
            {
                offset_acc[axis] = (accel_calibration_sum[axis] / (MAX_CALI_COUNT - 1));
            }
            accel_init = 1;

            float_to_byte.float_data = offset_acc[0];
            Calibration_data[0] = float_to_byte.byte[0];
            Calibration_data[1] = float_to_byte.byte[1];
            Calibration_data[2] = float_to_byte.byte[2];
            Calibration_data[3] = float_to_byte.byte[3];

            float_to_byte.float_data = offset_acc[1];
            Calibration_data[4] = float_to_byte.byte[0];
            Calibration_data[5] = float_to_byte.byte[1];
            Calibration_data[6] = float_to_byte.byte[2];
            Calibration_data[7] = float_to_byte.byte[3];

            float_to_byte.float_data = offset_acc[2];
            Calibration_data[8] = float_to_byte.byte[0];
            Calibration_data[9] = float_to_byte.byte[1];
            Calibration_data[10] = float_to_byte.byte[2];
            Calibration_data[11] = float_to_byte.byte[3];

            StoreCalibrationToFile();

            cali_count++;
        }
	}
    else		//有存储，直接计算
    {
        for(axis = 0; axis < 3; axis++)
        {
            acc[axis] = acc_raw[axis] - offset_acc[axis];
        }
    }
}

