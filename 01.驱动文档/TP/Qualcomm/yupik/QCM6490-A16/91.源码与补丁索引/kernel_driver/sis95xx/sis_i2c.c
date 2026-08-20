/*
 * drivers/input/touchscreen/sis_i2c.c
 * I2C Touch panel driver for SiS 9200 family
 *
 * Copyright (C) 2015 SiS, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Date: 2018/02/12
 * Version:	Android_v3.01.03
 */
#include <linux/module.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include "sis_i2c.h"
#include <linux/linkage.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/unaligned.h>
#include <linux/uaccess.h>
#include <linux/crc-itu-t.h> /*For CRC*/
#ifdef _STD_RW_IO
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#ifndef CONFIG_REG_BY_BOARDINFO
#include <linux/of_gpio.h>
int sis_gpio_irq;
#endif
#ifdef CONFIG_TOUCHSCREEN_SIS_I2C_95XX
#define DEVICE_NAME "sis_hydra_touch_device"
#else
#define DEVICE_NAME "sis_aegis_touch_device"
#endif
static const int sis_char_devs_count = 1;        /* device count */
static int sis_char_major;/* must 0 */
static struct cdev sis_char_cdev;
static struct class *sis_char_class;
#endif

/* Addresses to scan */
static const unsigned short normal_i2c[] = { SIS_SLAVE_ADDR, I2C_CLIENT_END };
static struct workqueue_struct *sis_wq;
struct sis_ts_data *ts_bak;/* must 0 */
struct sisTP_driver_data *TPInfo;/* must NULL */
static void sis_tpinfo_clear(struct sisTP_driver_data *TPInfo, int max);
#ifdef _SHOWVOLTAGE
static char k_buf[MAX_BYTE] = {0};
#endif
bool get_tool_data = false;
#ifdef CONFIG_X86
/*static const struct i2c_client_address_data addr_data;*/
/* Insmod parameters */
static int sis_ts_detect
(struct i2c_client *client, struct i2c_board_info *info);
#endif

static struct delayed_work driver_reg_work;
static int g_tp_wakeup_flag = 0;
//static int g_tp_wakeup_last_value = 0;
static int gesture_mode_enable = 1;
static int poweroff_when_suspend_flag = 0;
static bool sis_irq_suspended;
void PrintBuffer(int start, int length, char *buf)
{
	int i;
	for (i = start; i < length; i++) {
		pr_info("%02x ", buf[i]);
		if (i != 0 && i % 30 == 0)
			pr_info("\n");
	}
	pr_info("\n");
}

static int sis_command_for_write(struct i2c_client *client, int wlength,
							unsigned char *wdata)
{
	int ret = SIS_ERR;
	struct i2c_msg msg[1];

	msg[0].addr = client->addr;
	msg[0].flags = 0;/*Write*/
	msg[0].len = wlength;
	msg[0].buf = (unsigned char *)wdata;
	ret = i2c_transfer(client->adapter, msg, 1);
	return ret;
}

static int sis_command_for_read(struct i2c_client *client, int rlength,
							unsigned char *rdata)
{
	int ret = SIS_ERR;
	struct i2c_msg msg[1];

	msg[0].addr = client->addr;
	msg[0].flags = I2C_M_RD;/*Read*/
	msg[0].len = rlength;
	msg[0].buf = rdata;
	ret = i2c_transfer(client->adapter, msg, 1);
        return ret;
}

static int sis_cul_unit(uint8_t report_id)
{
	int ret = NORMAL_LEN_PER_POINT;

	if (report_id != ALL_IN_ONE_PACKAGE) {
		if (IS_AREA(report_id) /*&& IS_TOUCH(report_id)*/)
			ret += AREA_LEN_PER_POINT;
		if (IS_PRESSURE(report_id))
			ret += PRESSURE_LEN_PER_POINT;
	}

	return ret;
}

static int sis_ReadPacket(struct i2c_client *client, uint8_t cmd, uint8_t *buf)
{
	uint8_t tmpbuf[MAX_BYTE] = {0};	/*MAX_BYTE = 64;*/
#ifdef _CHECK_CRC
	uint16_t buf_crc = 0;
	uint16_t package_crc = 0;
	int l_package_crc = 0;
	int crc_end = 0;
#endif
#ifdef _SHOWVOLTAGE
	int i = 0;
#endif
	int ret = SIS_ERR;
	int touchnum = 0;
	int p_count = 0;
	int touc_formate_id = 0;
	int locate = 0;
	bool read_first = true;
	/*
	* New i2c format
	* buf[0] = Low 8 bits of byte count value
	* buf[1] = High 8 bits of byte counte value
	* buf[2] = Report ID
	* buf[touch num * 6 + 2 ] = Touch informations;
	* 1 touch point has 6 bytes, it could be none if no touch
	* buf[touch num * 6 + 3] = Touch numbers
	*
	* One touch point information include 6 bytes, the order is
	*
	* 1. status = touch down or touch up
	* 2. id = finger id
	* 3. x axis low 8 bits
	* 4. x axis high 8 bits
	* 5. y axis low 8 bits
	* 6. y axis high 8 bits
	* */
	do {
		if (locate >= PACKET_BUFFER_SIZE) {
			pr_err("sis_ReadPacket: Buf Overflow\n");
			return SIS_ERR;
		}
		ret = sis_command_for_read(client, MAX_BYTE, tmpbuf);

#ifdef _DEBUG_PACKAGE
		pr_info("sis_ReadPacket: Buf_Data [0~63]\n");
		PrintBuffer(0, 0x3d, tmpbuf);
#endif

		if (ret < 0) {
			pr_err("sis_ReadPacket: i2c transfer error\n");
			return SIS_ERR_TRANSMIT_I2C;
		}
		/*error package length of receiving data*/
		else if (tmpbuf[P_BYTECOUNT] > MAX_BYTE) {
#ifdef _DEBUG_REPORT
			pr_err("sis_ReadPacket: Error Bytecount\n");
			PrintBuffer(0, 64, tmpbuf);
#endif
			return SIS_ERR;
		}
#ifdef _SHOWVOLTAGE
		//cmd Report ID: 0xa
		else if ((tmpbuf[P_REPORT_ID] & 0xf) == CMD_FORMAT) {
			//copy CMD_DATA to temp buf
			for(i = 0; i < MAX_BYTE; i++)
			{
				k_buf[i] = tmpbuf[i];
			}
			get_tool_data = true;
			return SIS_ERR_CMD_DATA;
		}
#endif
		if (read_first) {
			/* access NO TOUCH event unless BUTTON NO TOUCH event*/
			if (tmpbuf[P_BYTECOUNT] == 0/*NO_TOUCH_BYTECOUNT*/)
				return touchnum;/*touchnum is 0*/
			if (tmpbuf[P_BYTECOUNT] == 3/*EMPRY PACKET*/) {
#ifdef _DEBUG_REPORT
				pr_err("sis_ReadPacket: Empty packet");
				PrintBuffer(0, 64, tmpbuf);
#endif
				return SIS_ERR_EMPTY_PACKET;
			}
		}
		/*skip parsing data when two devices are registered
		 * at the same slave address*/
		/*parsing data when P_REPORT_ID && 0xf is TOUCH_FORMAT
		 * or P_REPORT_ID is ALL_IN_ONE_PACKAGE*/
		touc_formate_id = tmpbuf[P_REPORT_ID] & 0xf;

		if ((touc_formate_id != HIDI2C_FORMAT)
#ifndef CONFIG_TOUCHSCREEN_SIS_I2C_95XX
		&& (touc_formate_id != TOUCH_FORMAT)
#endif
		&& (touc_formate_id != BUTTON_FORMAT)
		&& (tmpbuf[P_REPORT_ID] != ALL_IN_ONE_PACKAGE)) {
			pr_err("sis_ReadPacket: Error Report_ID 0x%x\n",
				tmpbuf[P_REPORT_ID]);
			return SIS_ERR;
		}
		p_count = (int) tmpbuf[P_BYTECOUNT] - 1;	/*start from 0*/
		if (tmpbuf[P_REPORT_ID] != ALL_IN_ONE_PACKAGE) {
			if (touc_formate_id == HIDI2C_FORMAT) {
				p_count -= BYTE_CRC_HIDI2C;
#ifndef CONFIG_TOUCHSCREEN_SIS_I2C_95XX
			} else if (touc_formate_id == TOUCH_FORMAT) {
				p_count -= BYTE_CRC_I2C;/*delete 2 byte crc*/
#endif
			} else if (touc_formate_id == BUTTON_FORMAT) {
				p_count -= BYTE_CRC_BTN;
			} else {	/*should not be happen*/
				pr_err("sis_ReadPacket: delete crc error\n");
				return SIS_ERR;
			}
			if (IS_SCANTIME(tmpbuf[P_REPORT_ID])) {
				p_count -= BYTE_SCANTIME;
#ifdef CONFIG_TOUCHSCREEN_SIS_I2C_95XX
			} else {
				pr_err("sis_ReadPacket: Error Report_ID 0x%x, no scan time\n",
					tmpbuf[P_REPORT_ID]);
				return SIS_ERR;	
#endif
			}
		}
		/*else {}*/ /*For ALL_IN_ONE_PACKAGE*/
		if (read_first)
			touchnum = tmpbuf[p_count];
		else {
			if (tmpbuf[p_count] != 0) {
				pr_err("sis_ReadPacket: get error package\n");
				return SIS_ERR;
			}
		}

#ifdef _CHECK_CRC
		crc_end = p_count + (IS_SCANTIME(tmpbuf[P_REPORT_ID]) * 2);
		buf_crc = cal_crc(tmpbuf, 2, crc_end);
		/*sub bytecount (2 byte)*/
		l_package_crc = p_count + 1
		+ (IS_SCANTIME(tmpbuf[P_REPORT_ID]) * 2);
		package_crc = ((tmpbuf[l_package_crc] & 0xff)
		| ((tmpbuf[l_package_crc + 1] & 0xff) << 8));

		if (buf_crc != package_crc)	{
			pr_err("sis_ReadPacket: CRC Error\n");
			return SIS_ERR;
		}
#endif
		memcpy(&buf[locate], &tmpbuf[0], 64);
		/*Buf_Data [0~63] [64~128]*/
		locate += 64;
		read_first = false;
	} while (tmpbuf[P_REPORT_ID] != ALL_IN_ONE_PACKAGE &&
			tmpbuf[p_count] > 5);
	return touchnum;
}

void ts_report_key(struct i2c_client *client, uint8_t keybit_state)
{
	int i = 0;
	/*check keybit_state is difference with pre_keybit_state*/
	uint8_t diff_keybit_state = 0x0;
	/*button location for binary*/
	uint8_t key_value = 0x0;
	/*button is up or down*/
	uint8_t  key_pressed = 0x0;
	struct sis_ts_data *ts = i2c_get_clientdata(client);

	if (!ts) {
		pr_err("%s error: Missing Platform Data!\n", __func__);
		return;
	}

	diff_keybit_state = TPInfo->pre_keybit_state ^ keybit_state;

	if (diff_keybit_state) {
		for (i = 0; i < BUTTON_KEY_COUNT; i++) {
			if ((diff_keybit_state >> i) & 0x01) {
				key_value = diff_keybit_state & (0x01 << i);
				key_pressed = (keybit_state >> i) & 0x01;
				switch (key_value) {
				case MSK_COMP:
				input_report_key
				(ts->input_dev, KEY_COMPOSE, key_pressed);
				pr_err("%s : MSK_COMP %d\n"
					, __func__ , key_pressed);
					break;
				case MSK_BACK:
				input_report_key
					(ts->input_dev, KEY_BACK, key_pressed);
				pr_err("%s : MSK_BACK %d\n"
					, __func__ , key_pressed);
					break;
				case MSK_MENU:
				input_report_key
					(ts->input_dev, KEY_MENU, key_pressed);
				pr_err("%s : MSK_MENU %d\n"
					, __func__ , key_pressed);
					break;
				case MSK_HOME:
				input_report_key
					(ts->input_dev, KEY_HOME, key_pressed);
				pr_err("%s : MSK_HOME %d\n"
					, __func__ , key_pressed);
					break;
				case MSK_NOBTN:

				default:
					break;
				}
			}
		}
		TPInfo->pre_keybit_state = keybit_state;
	}

	
}

static void sis_ts_work_func(struct work_struct *work)
{
	struct sis_ts_data *ts = container_of(work, struct sis_ts_data, work);
	int ret = SIS_ERR;
	int point_unit;
	uint8_t buf[PACKET_BUFFER_SIZE] = {0};
	uint8_t i = 0, fingers = 0;
	uint8_t px = 0, py = 0, pstatus = 0;
	uint8_t p_area = 0;
	uint8_t p_preasure = 0;
	int button_key;
	int p_button;

	bool all_touch_up = true;


	mutex_lock(&ts->mutex_wq);
	/* I2C or SMBUS block data read */
	ret = sis_ReadPacket(ts->client, SIS_CMD_NORMAL, buf);
#ifdef _DEBUG_PACKAGE_WORKFUNC
	pr_info("sis_ts_work_func: Buf_Data [0~63]\n");
	PrintBuffer(0, 64, buf);
	if ((buf[P_REPORT_ID] != ALL_IN_ONE_PACKAGE) && (ret > 5)) {
		pr_info("sis_ts_work_func: Buf_Data [64~125]\n");
		PrintBuffer(64, 128, buf);
	}
#endif

	/*Error Number*/
	if (ret < 0) {
		if (ret == -1)
			pr_info("sis_ts_work_func: ret = -1\n");
		goto err_free_allocate;
	}
	/*New button format*/
	else if ((buf[P_REPORT_ID] & 0xf) == BUTTON_FORMAT) {
		p_button = ((int) buf[P_BYTECOUNT] - 1 ) - BYTE_CRC_BTN + 1;
		button_key = ((buf[p_button] & 0xff)
		| ((buf[p_button + 1] & 0xff) << 8));
		ts_report_key(ts->client, button_key);
	}

	/* access NO TOUCH event unless BUTTON NO TOUCH event */
	else if (ret == 0) {
		fingers = 0;
		sis_tpinfo_clear(TPInfo, MAX_FINGERS);
		goto label_send_report;
	}
	sis_tpinfo_clear(TPInfo, MAX_FINGERS);

	/*Parser and Get the sis9200 data*/
	point_unit = sis_cul_unit(buf[P_REPORT_ID]);
	fingers = ret;

	TPInfo->fingers = fingers = (fingers > MAX_FINGERS ? 0 : fingers);

	/*fingers 10 =  0 ~ 9*/
	for (i = 0; i < fingers; i++) {
		if ((buf[P_REPORT_ID] != ALL_IN_ONE_PACKAGE) && (i >= 5)) {
			/*Calc point status*/
			pstatus = BYTE_BYTECOUNT + BYTE_ReportID
					+ ((i - 5) * point_unit);
			pstatus += 64;
		} else {
			pstatus = BYTE_BYTECOUNT + BYTE_ReportID
					+ (i * point_unit);
					/*Calc point status*/
		}
	    px = pstatus + 2;	/*Calc point x_coord*/
	    py = px + 2;	/*Calc point y_coord*/
		if ((buf[pstatus]) == TOUCHUP) {
			TPInfo->pt[i].Width = 0;
			TPInfo->pt[i].Height = 0;
			TPInfo->pt[i].Pressure = 0;
		} else if (buf[P_REPORT_ID] == ALL_IN_ONE_PACKAGE
					&& (buf[pstatus] & TOUCHDOWN) ) {
			TPInfo->pt[i].Width = 1;
			TPInfo->pt[i].Height = 1;
			TPInfo->pt[i].Pressure = 1;
		} else if ( buf[pstatus] & TOUCHDOWN ) {
#ifdef CONFIG_TOUCHSCREEN_SIS_I2C_95XX
			p_area = py + 3;
			p_preasure = py + 2;
			/*area*/
			if (IS_AREA(buf[P_REPORT_ID])) {
				TPInfo->pt[i].Width = (buf[p_area] & 0xff) 
				| ((buf[p_area+1] & 0xff) << 8);
				TPInfo->pt[i].Height = (buf[p_area+2] & 0xff) 
				| ((buf[p_area+3] & 0xff) << 8);
#else
			p_area = py + 2;
			p_preasure = py + 4;
			/*area*/
			if (IS_AREA(buf[P_REPORT_ID])) {
				TPInfo->pt[i].Width = buf[p_area];
				TPInfo->pt[i].Height = buf[p_area + 1];
#endif
			} else {
				TPInfo->pt[i].Width = 1;
				TPInfo->pt[i].Height = 1;
			}

			/*preasure*/
			if (IS_PRESSURE(buf[P_REPORT_ID]))
				TPInfo->pt[i].Pressure = (buf[p_preasure]);
			else
				TPInfo->pt[i].Pressure = 1;

		} else {
			pr_err("sis_ts_work_func: Error Touch Status\n");
			goto err_free_allocate;
		}
                if(rotation_flag){
                    TPInfo->pt[i].id = (buf[pstatus + 1]);
                    TPInfo->pt[i].x = ((buf[py] & 0xff)
                    | ((buf[py + 1] & 0xff) << 8));
                    TPInfo->pt[i].y = (4095-((buf[px] & 0xff)
                    | ((buf[px + 1] & 0xff) << 8)));
                }else{
		    TPInfo->pt[i].id = (buf[pstatus + 1]);
		    TPInfo->pt[i].x = ((buf[px] & 0xff)
		    | ((buf[px + 1] & 0xff) << 8));
		    TPInfo->pt[i].y = ((buf[py] & 0xff)
		    | ((buf[py + 1] & 0xff) << 8));
                }
	}
#ifdef _DEBUG_REPORT
	for (i = 0; i < TPInfo->fingers; i++) {
		pr_info("sis_ts_work_func: i = %d, id = %d, x = %d, y = %d"
		", pstatus = %d, width = %d, height = %d, pressure = %d\n"
		, i, TPInfo->pt[i].id, TPInfo->pt[i].x
		, TPInfo->pt[i].y , buf[pstatus], TPInfo->pt[i].Width
		, TPInfo->pt[i].Height, TPInfo->pt[i].Pressure);
	}
#endif

label_send_report:
/* Report co-ordinates to the multi-touch stack */

	for (i = 0; ((i < TPInfo->fingers) && (i < MAX_FINGERS)); i++) {
		if (TPInfo->pt[i].Pressure) {
			input_report_key(ts->input_dev, BTN_TOUCH, 1);
			TPInfo->pt[i].Width *= AREA_UNIT;
			input_report_abs(ts->input_dev,
				ABS_MT_TOUCH_MAJOR, TPInfo->pt[i].Width);
			TPInfo->pt[i].Height *= AREA_UNIT;
			input_report_abs(ts->input_dev,
				ABS_MT_TOUCH_MINOR, TPInfo->pt[i].Height);
			input_report_abs(ts->input_dev,
				ABS_MT_PRESSURE, TPInfo->pt[i].Pressure);
			input_report_abs(ts->input_dev,
				ABS_MT_POSITION_X, TPInfo->pt[i].x);
			input_report_abs(ts->input_dev,
				ABS_MT_POSITION_Y, TPInfo->pt[i].y);
			input_report_abs(ts->input_dev,
			ABS_MT_TRACKING_ID, TPInfo->pt[i].id);
			input_mt_sync(ts->input_dev);
			all_touch_up = false;
		}

		if (i == (TPInfo->fingers - 1) && all_touch_up == true) {
			input_report_key(ts->input_dev, BTN_TOUCH, 0);
			input_mt_sync(ts->input_dev);
		}
	}

	if (TPInfo->fingers == 0) {
		input_report_key(ts->input_dev, BTN_TOUCH, 0);
		input_mt_sync(ts->input_dev);
	}

	input_sync(ts->input_dev);

err_free_allocate:
/*
	if (ts->use_irq) {
#if ( LINUX_VERSION_CODE < KERNEL_VERSION (4, 2, 0) )
		if ((ts->desc->irq_data.state_use_accessors
		& IRQD_IRQ_DISABLED) == IRQ_STATUS_DISABLED)
#else
		if ((ts->desc->irq_common_data.state_use_accessors
		& IRQD_IRQ_DISABLED) == IRQ_STATUS_DISABLED)
#endif
			enable_irq_wake(ts->client->irq);
	}
*/
	mutex_unlock(&ts->mutex_wq);
	return;
}

static void sis_tpinfo_clear(struct sisTP_driver_data *TPInfo, int max)
{
	int i = 0;

	for (i = 0; i < max; i++) {
		TPInfo->pt[i].id = -1;
		TPInfo->pt[i].x = 0;
		TPInfo->pt[i].y = 0;
		TPInfo->pt[i].Pressure = 0;
		TPInfo->pt[i].Width = 0;
	}
	TPInfo->id = 0x0;
	TPInfo->fingers = 0;
}

static enum hrtimer_restart sis_ts_timer_func(struct hrtimer *timer)
{
	struct sis_ts_data *ts = container_of(timer, struct sis_ts_data, timer);

	queue_work(sis_wq, &ts->work);
	if (!ts->use_irq)	/*For Polling mode*/
	    hrtimer_start(&ts->timer, ktime_set(0, TIMER_NS), HRTIMER_MODE_REL);
	return HRTIMER_NORESTART;
}

static irqreturn_t sis_ts_irq_handler(int irq, void *dev_id)
{
	struct sis_ts_data *ts = dev_id;

/*
#if ( LINUX_VERSION_CODE < KERNEL_VERSION (4, 2, 0) )
	if ((ts->desc->irq_data.state_use_accessors
	& IRQD_IRQ_DISABLED) == IRQ_STATUS_ENABLED)
#else
	if ((ts->desc->irq_common_data.state_use_accessors
	& IRQD_IRQ_DISABLED) == IRQ_STATUS_ENABLED)
#endif
		disable_irq_nosync(ts->client->irq);
*/

	if (gesture_mode_enable == 1 && g_tp_wakeup_flag == 1) {
		input_report_key(ts->input_dev, KEY_WAKEUP, 1);
		input_sync(ts->input_dev);
		input_report_key(ts->input_dev, KEY_WAKEUP, 0);
		input_sync(ts->input_dev);
        pr_err("%s g_tp_wakeup_flag:%d\n",__func__, g_tp_wakeup_flag);
	}

	if(ts_bak->is_cmd_mode == MODE_IS_TOUCH) {
		queue_work(sis_wq, &ts->work);
	}
	else {
#ifdef _DEBUG_REPORT
		pr_err("sis drop the first packet\n");
#endif		
		ts_bak->is_cmd_mode = MODE_IS_TOUCH;
#if ( LINUX_VERSION_CODE < KERNEL_VERSION (4, 2, 0) )
			if ((ts_bak->desc->irq_data.state_use_accessors &
					IRQD_IRQ_DISABLED) == IRQ_STATUS_DISABLED) {
#else
			if ((ts_bak->desc->irq_common_data.state_use_accessors &
					IRQD_IRQ_DISABLED) == IRQ_STATUS_DISABLED) {

#endif
				enable_irq(ts_bak->client->irq);
			}
	}
	
	return IRQ_HANDLED;
}

uint16_t cal_crc(char *cmd, int start, int end)
{
	int i = 0;
	uint16_t crc = 0;
	for (i = start; i <= end ; i++)
		crc = (crc<<8) ^ crc16tab[((crc>>8) ^ cmd[i])&0x00FF];
	return crc;
}

uint16_t cal_crc_with_cmd(char *data, int start, int end, uint8_t cmd)
{
	int i = 0;
	uint16_t crc = 0;

	crc = (crc<<8) ^ crc16tab[((crc>>8) ^ cmd)&0x00FF];
	for (i = start; i <= end ; i++)
		crc = (crc<<8) ^ crc16tab[((crc>>8) ^ data[i])&0x00FF];
	return crc;
}

void write_crc(unsigned char *buf, int start, int end)
{
	uint16_t crc = 0;
	crc = cal_crc(buf, start , end);
	buf[end+1] = (crc >> 8) & 0xff;
	buf[end+2] = crc & 0xff;
}

#ifdef _STD_RW_IO
#define BUFFER_SIZE MAX_BYTE
static ssize_t sis_cdev_write(struct file *file, const char __user *buf,
								size_t count,
								loff_t *f_pos)
{
	int ret = 0;
	char *kdata;
	char cmd;
	pr_info("sis_cdev_write.\n");
	if (ts_bak == 0)
		return SIS_ERR_CLIENT;
#if 0
	ret = access_ok(VERIFY_WRITE, buf, BUFFER_SIZE);
	if (!ret) {
		pr_err("cannot access user space memory\n");
		return SIS_ERR_ACCESS_USER_MEM;
	}
#endif
	if (count > BUFFER_SIZE)
		count = BUFFER_SIZE;
	if (count == 0)
		return 0;

	kdata = kmalloc(BUFFER_SIZE, GFP_KERNEL);
	if (kdata == 0)
		return SIS_ERR_ALLOCATE_KERNEL_MEM;

	ret = copy_from_user(kdata, buf, count);
	if (ret) {
		pr_err("copy_from_user fail\n");
		kfree(kdata);
		return SIS_ERR_COPY_FROM_USER;
	}

	cmd = kdata[6];

/*Write & Read*/
	ret = sis_command_for_write(ts_bak->client, count, kdata);
	if (ret < 0) {
		pr_err("i2c_transfer write error %d\n", ret);
		kfree(kdata);
		return SIS_ERR_TRANSMIT_I2C;
	}
	if (copy_to_user((char *) buf, kdata, count)) {
		pr_err("copy_to_user fail\n");
		ret = SIS_ERR_COPY_FROM_KERNEL;
	}
	kfree(kdata);
	return ret;
}

/*for get system time*/
static ssize_t sis_cdev_read(struct file *file, char __user *buf,
								size_t count,
								loff_t *f_pos)
{
#ifdef _SHOWVOLTAGE
	int i;
#endif
	int ret = 0;
	char *kdata;
	char cmd;

	pr_info("sis_cdev_read.\n");
	if (ts_bak == 0)
		return SIS_ERR_CLIENT;
#if 0
        ret = access_ok(VERIFY_WRITE, buf, BUFFER_SIZE);
	if (!ret) {
		pr_err("cannot access user space memory\n");
		return SIS_ERR_ACCESS_USER_MEM;
	}
#endif
	if (count > BUFFER_SIZE)
		count = BUFFER_SIZE;
	if (count == 0)
		return 0;

	kdata = kmalloc(BUFFER_SIZE, GFP_KERNEL);
	if (kdata == 0)
		return SIS_ERR_ALLOCATE_KERNEL_MEM;
	ret = copy_from_user(kdata, buf, count);
	if (ret) {
		pr_err("copy_from_user fail\n");
		kfree(kdata);
		return SIS_ERR_COPY_FROM_USER;
	}

	cmd = kdata[6];
	/*for making sure AP communicates with SiS driver */
	if (cmd == 0xa2) {
		kdata[0] = 5;
		kdata[1] = 0;
		kdata[3] = 'S';
		kdata[4] = 'i';
		kdata[5] = 'S';
		if (copy_to_user((char *) buf, kdata, count)) {
			pr_err("copy_to_user fail\n");
			kfree(kdata);
			return SIS_ERR_COPY_FROM_KERNEL;
		}
		kfree(kdata);
		return 3;
	}
#ifdef _SHOWVOLTAGE	
	mutex_lock(&ts_bak->mutex_wq);
	if (get_tool_data) {
		for(i = 0; i < MAX_BYTE; i++)
		{
			kdata[i] = k_buf[i];
		}
		get_tool_data = false;
	} else { 
#endif
/* Write & Read */
	ret = sis_command_for_read(ts_bak->client, MAX_BYTE, kdata);
	if (ret < 0) {
		pr_err("i2c_transfer read error %d\n", ret);
		kfree(kdata);
		return SIS_ERR_TRANSMIT_I2C;
	}
#ifdef _SHOWVOLTAGE
	}
	mutex_unlock(&ts_bak->mutex_wq);
#endif
	ret = kdata[0] | (kdata[1] << 8);

/*
    for ( i = 0; i < BUFFER_SIZE - 1; i++ ) {
	    kdata[i] = kdata[i+1];
    }
*/
#ifdef _DEBUG_REPORT
	int i;
	pr_info("%d\n", ret);
	for (i = 0; i < ret && i < BUFFER_SIZE; i++)
		pr_info("%02x ", kdata[i]);
	pr_info("\n");
#endif
	if (copy_to_user((char *) buf, kdata, count)) {
		pr_info("copy_to_user fail\n");
		ret = SIS_ERR_COPY_FROM_KERNEL;
	}
	kfree(kdata);
	return ret;
}

#undef BUFFER_SIZE

static int sis_cdev_open(struct inode *inode, struct file *filp)
{
	pr_info("sis_cdev_open.\n");
	if (ts_bak == 0)
		return SIS_ERR_CLIENT;

	msleep(200);
	if (ts_bak->use_irq) {
#if ( LINUX_VERSION_CODE < KERNEL_VERSION (4, 2, 0) )
		if ((ts_bak->desc->irq_data.state_use_accessors
		& IRQD_IRQ_DISABLED) == IRQ_STATUS_ENABLED) {
#else
		if ((ts_bak->desc->irq_common_data.state_use_accessors
		& IRQD_IRQ_DISABLED) == IRQ_STATUS_ENABLED) {
#endif
			disable_irq(ts_bak->client->irq);
		} else {
#if ( LINUX_VERSION_CODE < KERNEL_VERSION (4, 2, 0) )
			pr_info("sis_cdev_open:IRQ_STATUS: %x\n",
			(ts_bak->desc->irq_data.state_use_accessors &
				IRQD_IRQ_DISABLED));
#else
			pr_info("sis_cdev_open:IRQ_STATUS: %x\n",
			(ts_bak->desc->irq_common_data.state_use_accessors &
				IRQD_IRQ_DISABLED));
#endif
		}
	}
	ts_bak->is_cmd_mode = (MODE_IS_CMD | MODE_CHANGE);
	hrtimer_cancel(&ts_bak->timer);
	flush_workqueue(sis_wq);/* only flush sis_wq */
	msleep(200);
	return 0; /* success */
}

static int sis_cdev_release(struct inode *inode, struct file *filp)
{
	pr_info("sis_cdev_release.\n");
	msleep(200);
	if (ts_bak == 0)
		return SIS_ERR_CLIENT;
	if (ts_bak->use_irq) {
#if ( LINUX_VERSION_CODE < KERNEL_VERSION (4, 2, 0) )
		if ((ts_bak->desc->irq_data.state_use_accessors &
				IRQD_IRQ_DISABLED) == IRQ_STATUS_DISABLED) {
#else
		if ((ts_bak->desc->irq_common_data.state_use_accessors &
				IRQD_IRQ_DISABLED) == IRQ_STATUS_DISABLED) {

#endif
			enable_irq(ts_bak->client->irq);
		}
	} else
		hrtimer_start(&ts_bak->timer,
			ktime_set(1, 0), HRTIMER_MODE_REL);
	ts_bak->is_cmd_mode = MODE_CHANGE;
	return 0;
}

static const struct file_operations sis_cdev_fops = {
	.owner	= THIS_MODULE,
	.read	= sis_cdev_read,
	.write	= sis_cdev_write,
	.open	= sis_cdev_open,
	.release	= sis_cdev_release,
};

static int sis_setup_chardev(struct sis_ts_data *ts)
{
	dev_t dev = MKDEV(sis_char_major, 0);
	int alloc_ret = 0;
	int cdev_err = 0;
	int input_err = 0;
	struct device *class_dev = NULL;
	void *ptr_err;
	int err = 0;
	pr_info("sis_setup_chardev.\n");
	if (ts == NULL) {
		input_err = -ENOMEM;
		goto error;
	}
	/* dynamic allocate driver handle */
	alloc_ret = alloc_chrdev_region(&dev, 0,
			sis_char_devs_count, DEVICE_NAME);
	if (alloc_ret)
		goto error;
	sis_char_major = MAJOR(dev);
	cdev_init(&sis_char_cdev, &sis_cdev_fops);
	sis_char_cdev.owner = THIS_MODULE;
	cdev_err = cdev_add(&sis_char_cdev,
		MKDEV(sis_char_major, 0), sis_char_devs_count);
	if (cdev_err)
		goto error;
	pr_info("%s driver(major %d) installed.\n",
			DEVICE_NAME, sis_char_major);
	/* register class */
	#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,4,0)
	sis_char_class = class_create(DEVICE_NAME);
#else
	sis_char_class = class_create(THIS_MODULE, DEVICE_NAME);
#endif
	err = IS_ERR(ptr_err = sis_char_class);
	if (err)
		goto err2;
	class_dev = device_create(sis_char_class, NULL,
		MKDEV(sis_char_major, 0), NULL, DEVICE_NAME);
	err = IS_ERR(ptr_err = class_dev);
	if (err)
		goto err;
	return 0;
error:
	if (cdev_err == 0)
		cdev_del(&sis_char_cdev);
	if (alloc_ret == 0)
		unregister_chrdev_region
				(MKDEV(sis_char_major, 0), sis_char_devs_count);
	if (input_err != 0)
		pr_err("sis_ts_bak error!\n");
err:
	device_destroy(sis_char_class, MKDEV(sis_char_major, 0));
err2:
	class_destroy(sis_char_class);
	return SIS_ERR;
}
#endif

#ifndef CONFIG_REG_BY_BOARDINFO
#if 0
static int sis_pinctrl_init(struct sis_ts_data *ts)
{
        //int ret = 0;

        ts->pinctrl = devm_pinctrl_get(&ts->client->dev);
        //if (IS_ERR_OR_NULL(ts->pinctrl)) {
                //FTS_ERROR("Failed to get pinctrl, please check dts");
                //ret = PTR_ERR(ts->pinctrl);
          //      goto err_pinctrl_get;
        //}

        ts->pins_active = pinctrl_lookup_state(ts->pinctrl, "pmx_ts_active");
        //if (IS_ERR_OR_NULL(ts->pins_active)) {
        //        FTS_ERROR("Pin state[active] not found");
        //        ret = PTR_ERR(ts->pins_active);
        //        goto err_pinctrl_lookup;
        //}

        ts->pins_suspend = pinctrl_lookup_state(ts->pinctrl, "pmx_ts_suspend");
        //if (IS_ERR_OR_NULL(ts->pins_suspend)) {
        //        FTS_ERROR("Pin state[suspend] not found");
        //        ret = PTR_ERR(ts->pins_suspend);
        //        goto err_pinctrl_lookup;
        //}

        ts->pins_release = pinctrl_lookup_state(ts->pinctrl, "pmx_ts_release");
        //if (IS_ERR_OR_NULL(ts->pins_release)) {
        //        FTS_ERROR("Pin state[release] not found");
        //        ret = PTR_ERR(ts->pins_release);
        //}

        return 0;
//err_pinctrl_lookup:
//        if (ts->pinctrl) {
//                devm_pinctrl_put(ts->pinctrl);
//        }
//err_pinctrl_get:
//        ts->pinctrl = NULL;
//        ts->pins_release = NULL;
//        ts->pins_suspend = NULL;
//        ts->pins_active = NULL;
//        return ret;
}
#endif

int sis_irq_config(struct sis_ts_data *ts)
{
	struct device_node *np;
	/* irq_flags unused after of_get_named_gpio migration */
	struct device *dev;

	pr_info("sis_irq_config\n");
	dev = &ts->client->dev;
	np = dev->of_node;

        rotation_flag = of_property_read_bool(np,"sis_rotation");
	sis_gpio_irq = of_get_named_gpio(np, "tp_int-gpio", 0);
	pr_info("sis_gpio_irq=%d\n", sis_gpio_irq);
	ts->irq = gpio_to_irq(sis_gpio_irq);
	
	    if (gpio_request_one(sis_gpio_irq, GPIOF_IN, "tp_int-gpio") != 0) {
		//gpio_free(sis_gpio_irq);
		pr_err("sis_irq_config  gpio_request error\n");
	        goto error;
	    }
	
        gpio_direction_input(sis_gpio_irq);


	ts->en_gpio = of_get_named_gpio(np, "tp_en-gpio", 0);
	pr_info("en_gpio=%d\n", ts->en_gpio);

	    if (gpio_request(ts->en_gpio, "tp_en-gpio") != 0) {
		//gpio_free(ts->en_gpio);
		pr_err("sis_irq_config  gpio_request rst error\n");
	        goto error;
	    }

        ts->rst_gpio = of_get_named_gpio(np, "tp_rst-gpio", 0);
	pr_info("rst_gpio=%d\n", ts->rst_gpio);

	    if (gpio_request(ts->rst_gpio, "tp_rst-gpio") != 0) {
		//gpio_free(ts->rst_gpio);
		pr_err("sis_irq_config  gpio_request rst error\n");
	        goto error;
	    }
	ts->tp_mode1_gpio = of_get_named_gpio(np, "tp_mode1-gpio", 0);
	pr_info("tp_mode1_gpio=%d\n", ts->tp_mode1_gpio);
	
	if (gpio_is_valid(ts->tp_mode1_gpio)){
	    if (gpio_request(ts->tp_mode1_gpio, "tp_mode1-gpio") != 0) {
		    //gpio_free(ts->tp_mode1_gpio);
		    pr_err("sis_irq_config  gpio_request  49 error\n");
	        goto error;
	    }
	}

	ts->tp_mode2_gpio = of_get_named_gpio(np, "tp_mode2-gpio", 0);
	pr_info("tp_mode2_gpio=%d\n", ts->tp_mode2_gpio);

	if (gpio_is_valid(ts->tp_mode2_gpio)){
	    if (gpio_request(ts->tp_mode2_gpio, "tp_mode2-gpio") != 0) {
	   	    //gpio_free(ts->tp_mode2_gpio);
		    pr_err("sis_irq_config  gpio_request 162 error\n");
	        goto error;
	    }
	}
//        sis_pinctrl_init(ts);

//        pinctrl_select_state(ts->pinctrl, ts->pins_active);

        gpio_direction_output(ts->en_gpio, 1);
        mdelay(5);
        gpio_direction_output(ts->rst_gpio, 0);
        mdelay(5);
        gpio_direction_output(ts->rst_gpio, 1);

	if (gpio_is_valid(ts->tp_mode1_gpio)){
        gpio_direction_output(ts->tp_mode1_gpio, 1);
	}

	if (gpio_is_valid(ts->tp_mode2_gpio)){
        gpio_direction_output(ts->tp_mode2_gpio, 1);
	}
        return 0;
error:
	if (gpio_is_valid(ts->tp_mode2_gpio)){
		gpio_free(ts->tp_mode2_gpio);
	}
	if (gpio_is_valid(ts->tp_mode1_gpio)){
                gpio_free(ts->tp_mode1_gpio);
        }
        if (gpio_is_valid(ts->rst_gpio)){
                gpio_free(ts->rst_gpio);
        }
        if (gpio_is_valid(ts->en_gpio)){
                gpio_free(ts->en_gpio);
        }
        if (gpio_is_valid(sis_gpio_irq)){
                gpio_free(sis_gpio_irq);
        }
	pr_err("request gpio fail,return -1\n");
	return -1;

}
#endif
static ssize_t tp_mode1_show(struct device *dev,
                                struct device_attribute *attr, char *buf)
{
		if (gpio_is_valid(ts_bak->tp_mode1_gpio)){
        		return scnprintf(buf, PAGE_SIZE, "tp_mode1 is%d  \r\n",
                        !gpio_get_value(ts_bak->tp_mode1_gpio));
		}else{
			pr_err("tp_mode1_gpio is unvalid\n");
			return 0;
		}
}

static ssize_t tp_mode1_store(struct device *dev,
                struct device_attribute *attr, const char *buf, size_t count)
{

        int value;
        int error = -1;
        error = sscanf(buf, "%d", &value);
        if (error == -1){
		pr_err("echo error\n");
                return -EINVAL;
	}
		pr_err("tp_mode1: %d\n",value);
		if (gpio_is_valid(ts_bak->tp_mode1_gpio)){
			if(value==0){
				gpio_direction_output(ts_bak->tp_mode1_gpio, 1);
				pr_err("pull up tp_mode1_gpio\n");
                                mdelay(5);
                                gpio_direction_output(ts_bak->en_gpio, 0);
                                mdelay(50);
                                gpio_direction_output(ts_bak->en_gpio, 1);
                                mdelay(5);
			}else{
				gpio_direction_output(ts_bak->tp_mode1_gpio, 0);
				pr_err("pull down tp_mode1_gpio\n");
                                mdelay(5);
                                gpio_direction_output(ts_bak->en_gpio, 0);
                                mdelay(50);
                                gpio_direction_output(ts_bak->en_gpio, 1);
                                mdelay(5);

			}
		}else{
			pr_err("tp_mode1_gpio is unvalid\n");
		}

		return count;
}

static ssize_t tp_mode2_show(struct device *dev,
                                struct device_attribute *attr, char *buf)
{
		if (gpio_is_valid(ts_bak->tp_mode2_gpio)){
        		return scnprintf(buf, PAGE_SIZE, "tp_mode2 is:%d  \r\n",
                        !gpio_get_value(ts_bak->tp_mode2_gpio));
		}else{
			pr_err("tp_mode2_gpio is unvalid\n");
			return 0;
		}
}

static ssize_t tp_mode2_store(struct device *dev,
                struct device_attribute *attr, const char *buf, size_t count)
{
        int value;
        int error = -1;
        error = sscanf(buf, "%d", &value);
        if (error == -1){
		pr_err("echo error\n");
                return -EINVAL;
	}
		pr_err("tp_mode2: %d\n",value);
		if (gpio_is_valid(ts_bak->tp_mode2_gpio)){
			if(value==0){
				gpio_direction_output(ts_bak->tp_mode2_gpio, 1);
				pr_err("pull up tp_mode2_gpio\n");
                                mdelay(5);
                                gpio_direction_output(ts_bak->en_gpio, 0);
                                mdelay(50);
                                gpio_direction_output(ts_bak->en_gpio, 1);
                                mdelay(5);

			}else{
				gpio_direction_output(ts_bak->tp_mode2_gpio, 0);
				pr_err("pull down tp_mode2_gpio\n");
                                mdelay(5);
                                gpio_direction_output(ts_bak->en_gpio, 0);
                                mdelay(50);
                                gpio_direction_output(ts_bak->en_gpio, 1);
                                mdelay(5);

			}
		}else{
			pr_err("tp_mode2_gpio is unvalid\n");
		}

		return count;
}

static ssize_t gesture_mode_show(struct device *dev,
                                struct device_attribute *attr, char *buf)
{
                         return scnprintf(buf, PAGE_SIZE, "geseture mode is:%d  \r\n",
                         gesture_mode_enable);
}

static ssize_t gesture_mode_store(struct device *dev,
                struct device_attribute *attr, const char *buf, size_t count)
{
         int value;
         int error = -1;
         error = sscanf(buf, "%d", &value);
         if (error == -1){
                 pr_err("echo error\n");
                 return -EINVAL;
         }
                 pr_err("gesture mode: %d\n",value);
                         if(value==0){
                                 gesture_mode_enable=0;
                                 pr_err("disable gesture mode\n");
                         }else{
                                 gesture_mode_enable=1;
                                 pr_err("enable gesture mode\n");
                         }

                 return count;
}
static ssize_t poweroff_when_suspend_show(struct device *dev,
                                struct device_attribute *attr, char *buf)
{
   return scnprintf(buf, PAGE_SIZE, "if device will %s when suspend\r\n",
   (poweroff_when_suspend_flag?"poweroff":"poweron"));
}

static ssize_t poweroff_when_suspend_store(struct device *dev,
                struct device_attribute *attr, const char *buf, size_t count)
{
   int value;
   int error = -1;
   error = sscanf(buf, "%d", &value);
   if (error == -1){
           pr_err("echo error\n");
           return -EINVAL;
  }
   pr_err("set device %s when suspend\n",(value?"poweroff":"poweron"));
   if(value==0){
           poweroff_when_suspend_flag=0;
           pr_err("power on when device suspend\n");
   }else{
           poweroff_when_suspend_flag=1;
           pr_err("poweroff when device suspend\n");
   }

   return count;
}

static DEVICE_ATTR_RW(tp_mode1);
static DEVICE_ATTR_RW(tp_mode2);
static DEVICE_ATTR_RW(gesture_mode);
static DEVICE_ATTR_RW(poweroff_when_suspend);

static struct attribute *sis_attributes[] = {
        &dev_attr_tp_mode1.attr,
        &dev_attr_tp_mode2.attr,
        &dev_attr_gesture_mode.attr,
        &dev_attr_poweroff_when_suspend.attr,
        NULL,
};

static struct attribute_group sis_attribute_group = {
        .attrs = sis_attributes
};

static int ts_create_sysfs(struct sis_ts_data *ts)
{
        int ret = 0;

        ret = sysfs_create_group(&ts->client->dev.kobj, &sis_attribute_group);
        if (ret) {
                pr_err("[EX]: sysfs_create_group() failed!!\n");
                sysfs_remove_group(&ts->client->dev.kobj, &sis_attribute_group);
                return -ENOMEM;
        }

        pr_info("[EX]: sysfs_create_group() succeeded!!\n");
        return ret;
}

#if defined(CONFIG_DRM)
#include <drm/drm_panel.h>
#include <linux/soc/qcom/panel_event_notifier.h>

static struct drm_panel *sis_active_panel;
static struct device_node *sis_np;
static void *sis_notifier_cookie;

static void sis_touch_panel_notifier_callback(enum panel_event_notifier_tag tag,
		struct panel_event_notification *notification, void *client_data)
{
	if (!notification) {
		pr_err("sis: invalid panel notification");
		return;
	}

	pr_info("sis: panel notif type:%d early:%d",
		notification->notif_type,
		notification->notif_data.early_trigger);

	switch (notification->notif_type) {
	case DRM_PANEL_EVENT_UNBLANK:
		if (notification->notif_data.early_trigger) {
			pr_debug("sis: resume early, skip");
		} else {
			g_tp_wakeup_flag = 0;
			if (gesture_mode_enable)
				disable_irq_wake(ts_bak->client->irq);
			if (poweroff_when_suspend_flag) {
				pr_info("sis: poweron TP on resume by gpio");
				if (gpio_is_valid(ts_bak->en_gpio))
					gpio_direction_output(ts_bak->en_gpio, 1);
				mdelay(5);
				if (gpio_is_valid(ts_bak->rst_gpio)) {
					gpio_direction_output(ts_bak->rst_gpio, 0);
					mdelay(5);
					gpio_direction_output(ts_bak->rst_gpio, 1);
				}
			}
			if (sis_irq_suspended) {
				enable_irq(ts_bak->client->irq);
				sis_irq_suspended = false;
			}
		}
		break;
	case DRM_PANEL_EVENT_BLANK:
		if (notification->notif_data.early_trigger) {
			if (gesture_mode_enable) {
				g_tp_wakeup_flag = 1;
				enable_irq_wake(ts_bak->client->irq);
			} else if (!sis_irq_suspended) {
				disable_irq(ts_bak->client->irq);
				sis_irq_suspended = true;
			}
			if (poweroff_when_suspend_flag) {
				pr_info("sis: poweroff TP on suspend by gpio");
				if (gpio_is_valid(ts_bak->rst_gpio))
					gpio_direction_output(ts_bak->rst_gpio, 0);
				if (gpio_is_valid(ts_bak->en_gpio))
					gpio_direction_output(ts_bak->en_gpio, 0);
			}
		} else {
			pr_debug("sis: suspend late, skip");
		}
		break;
	case DRM_PANEL_EVENT_BLANK_LP:
		pr_debug("sis: received lp event");
		break;
	default:
		pr_debug("sis: unhandled panel notif %d",
			 notification->notif_type);
		break;
	}
}

static void sis_tp_register_panel_notifier(struct work_struct *p_work)
{
	int i, count;
	struct device_node *node;
	struct drm_panel *panel;
	void *cookie;

	if (!sis_np) {
		pr_err("sis: of_node is NULL");
		return;
	}

	count = of_count_phandle_with_args(sis_np, "panel", NULL);
	if (count <= 0) {
		pr_err("sis: find panel phandle count(%d) fail", count);
		return;
	}

	for (i = 0; i < count; i++) {
		node = of_parse_phandle(sis_np, "panel", i);
		panel = of_drm_find_panel(node);
		of_node_put(node);
		if (IS_ERR_OR_NULL(panel)) {
			pr_err("sis: of_drm_find_panel fail idx=%d", i);
			continue;
		}

		sis_active_panel = panel;
		cookie = panel_event_notifier_register(
			PANEL_EVENT_NOTIFICATION_PRIMARY,
			PANEL_EVENT_NOTIFIER_CLIENT_PRIMARY_TOUCH,
			sis_active_panel,
			sis_touch_panel_notifier_callback,
			NULL);
		if (!cookie || IS_ERR(cookie)) {
			pr_err("sis: panel_event_notifier_register fail");
			sis_active_panel = NULL;
			return;
		}
		sis_notifier_cookie = cookie;
		pr_info("sis: registered panel_event_notifier");
		return;
	}
	pr_err("sis: no usable drm_panel found");
}

static DECLARE_DELAYED_WORK(sis_panel_notif_work, sis_tp_register_panel_notifier);
#endif /* CONFIG_DRM */

static int sis_ts_probe(struct i2c_client *client)
{
	int ret = 0;
	struct sis_ts_data *ts = NULL;
	struct sis_i2c_rmi_platform_data *pdata = NULL;

	pr_info("sis_ts_probe \n");
	TPInfo = kzalloc(sizeof(struct sisTP_driver_data), GFP_KERNEL);
	if (TPInfo == NULL) {
		ret = -ENOMEM;
		goto err_alloc_data_failed;
	}
	ts = kzalloc(sizeof(struct sis_ts_data), GFP_KERNEL);
	if (ts == NULL) {
		ret = -ENOMEM;
		goto err_alloc_data_failed;
	}
	ts_bak = ts;

	mutex_init(&ts->mutex_wq);
	/*1. Init Work queue and necessary buffers*/
	INIT_WORK(&ts->work, sis_ts_work_func);
	ts->client = client;
	i2c_set_clientdata(client, ts);
	//pdata = client->dev.platform_data;
	if (pdata)
		ts->power = pdata->power;
	if (ts->power) {
		ret = ts->power(1);
		if (ret < 0) {
			pr_err("sis_ts_probe power on failed\n");
			goto err_power_failed;
		}
	}
	/*2. Allocate input device*/
	ts->input_dev = input_allocate_device();
	if (ts->input_dev == NULL) {
		ret = -ENOMEM;
		pr_err("sis_ts_probe: Failed to allocate input device\n");
		goto err_input_dev_alloc_failed;
	}
	/*This input device name should be the same to IDC file name.*/
	/*"SiS9200-i2c-touchscreen"*/
	ts->input_dev->name = "sis_touch";

	set_bit(EV_ABS, ts->input_dev->evbit);
	set_bit(EV_KEY, ts->input_dev->evbit);
	set_bit(EV_SYN, ts->input_dev->evbit); 
	set_bit(ABS_MT_POSITION_X, ts->input_dev->absbit);
	set_bit(ABS_MT_POSITION_Y, ts->input_dev->absbit);
	set_bit(ABS_MT_TRACKING_ID, ts->input_dev->absbit);

	set_bit(INPUT_PROP_DIRECT, ts->input_dev->propbit);
	set_bit(ABS_MT_PRESSURE, ts->input_dev->absbit);
	set_bit(ABS_MT_TOUCH_MAJOR, ts->input_dev->absbit);
	set_bit(ABS_MT_TOUCH_MINOR, ts->input_dev->absbit);


	input_set_abs_params(ts->input_dev, ABS_MT_PRESSURE,
						0, PRESSURE_MAX, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR,
						0, AREA_LENGTH_LONGER, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MINOR,
						0, AREA_LENGTH_SHORT, 0, 0);

	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X,
						0, SIS_MAX_X, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y,
						0, SIS_MAX_Y, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_TRACKING_ID,
						0, 15, 0, 0);

	/* add for touch keys */
	set_bit(BTN_TOUCH, ts->input_dev->keybit);
	set_bit(KEY_COMPOSE, ts->input_dev->keybit);
	set_bit(KEY_BACK, ts->input_dev->keybit);
	set_bit(KEY_MENU, ts->input_dev->keybit);
	set_bit(KEY_HOME, ts->input_dev->keybit);
        set_bit(KEY_WAKEUP, ts->input_dev->keybit);

	ts->is_cmd_mode = MODE_IS_TOUCH;
	/*3. Register input device to core*/
	ret = input_register_device(ts->input_dev);
	if (ret) {
		pr_err("sis_ts_probe: Unable to register %s input device\n",
				ts->input_dev->name);
		goto err_input_register_device_failed;
	}
	/*4. irq or timer setup*/
#ifndef CONFIG_REG_BY_BOARDINFO
	ret=sis_irq_config(ts);
	if(ret)
		goto err_input_register_device_failed;
	client->irq = ts->irq;	
#endif
#ifdef _I2C_INT_ENABLE
#ifdef CONFIG_REG_BY_BOARDINFO
	client->irq = gpio_to_irq(GPIO_IRQ);
#endif
	ret = request_irq(client->irq, sis_ts_irq_handler,
				IRQF_TRIGGER_FALLING|IRQF_ONESHOT, client->name, ts);
	if (ret == 0)
		ts->use_irq = 1;
	else
		dev_err(&client->dev, "request_irq failed\n");
#endif

	ts->desc = irq_to_desc(ts_bak->client->irq);
	hrtimer_init(&ts->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	ts->timer.function = sis_ts_timer_func;
	if (!ts->use_irq)
		hrtimer_start(&ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);

	pr_info("sis_ts_probe: Start touchscreen %s in %s mode\n",
			ts->input_dev->name,
			ts->use_irq ? "interrupt" : "polling");
#ifdef _STD_RW_IO
	ret = sis_setup_chardev(ts);
	if (ret)
		pr_err("sis_setup_chardev fail\n");
#endif
	pr_info("sis SIS_SLAVE_ADDR: %d\n", SIS_SLAVE_ADDR);
	
	ret = ts_create_sysfs(ts);
        if (ret)
                pr_err("create sysfs node fail\n");
#if defined(CONFIG_DRM)
	sis_np = client->dev.of_node;
	schedule_delayed_work(&sis_panel_notif_work, msecs_to_jiffies(13600));
#endif
	return 0;
err_input_register_device_failed:
	input_free_device(ts->input_dev);
err_input_dev_alloc_failed:
err_power_failed:
	kfree(ts);
err_alloc_data_failed:
	return ret;
}

static void sis_ts_remove(struct i2c_client *client)
{
	struct sis_ts_data *ts = i2c_get_clientdata(client);
        pr_err("sis_ts_remove\n");
#if defined(CONFIG_DRM)
	cancel_delayed_work_sync(&sis_panel_notif_work);
	if (sis_notifier_cookie) {
		panel_event_notifier_unregister(sis_notifier_cookie);
		sis_notifier_cookie = NULL;
		sis_active_panel = NULL;
	}
#endif
	if (ts->use_irq)
		free_irq(client->irq, ts);
	else
		hrtimer_cancel(&ts->timer);
        if (gpio_is_valid(ts->tp_mode2_gpio)){
                gpio_free(ts->tp_mode2_gpio);
        }
        if (gpio_is_valid(ts->tp_mode1_gpio)){
                gpio_free(ts->tp_mode1_gpio);
        }
        if (gpio_is_valid(ts->rst_gpio)){
                gpio_free(ts->rst_gpio);
        }
        if (gpio_is_valid(ts->en_gpio)){
                gpio_free(ts->en_gpio);
        }
        if (gpio_is_valid(sis_gpio_irq)){
                gpio_free(sis_gpio_irq);
        }

	input_unregister_device(ts->input_dev);
	kfree(ts);
}

static const struct i2c_device_id sis_ts_id[] = {
	{ SIS_I2C_NAME, 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, sis_ts_id);

#ifndef CONFIG_REG_BY_BOARDINFO
static struct of_device_id sis_ts_dt_ids[] = {
	{ .compatible = "sis,sis_touch" },
	{ }
};
#endif
static struct i2c_driver sis_ts_driver = {
	.probe		= sis_ts_probe,
	.remove		= sis_ts_remove,
#ifdef CONFIG_X86
	.class		= I2C_CLASS_HWMON,
	.detect		= sis_ts_detect,
	.address_list	= normal_i2c,
#endif
	.id_table	= sis_ts_id,
	.driver = {
		.name	= SIS_I2C_NAME,
#ifndef CONFIG_REG_BY_BOARDINFO
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(sis_ts_dt_ids),
#endif
	},
};



static void i2c_driver_register_delayed(struct work_struct *work) {
    int ret = 0;

    pr_info(" start init sis_ts driver!\n");
    i2c_add_driver(&sis_ts_driver);
    if (ret < 0)
        pr_info("Failed to register I2C driver: %d\n", ret);
}

static int __init sis_ts_init(void)
{
	pr_info("sis_ts_init ver. 3.01.03\n");
	sis_wq = create_singlethread_workqueue("sis_wq");

	if (!sis_wq)
		return -ENOMEM;

	pr_info("sis_ts_init after egalax driver init finish\n");
        INIT_DELAYED_WORK(&driver_reg_work, i2c_driver_register_delayed);
        schedule_delayed_work(&driver_reg_work, msecs_to_jiffies(5000));



//	return i2c_add_driver(&sis_ts_driver);
	return 0;
}

#ifdef CONFIG_X86
/* Return 0 if detection is successful, -ENODEV otherwise */
static int sis_ts_detect(struct i2c_client *client,
		       struct i2c_board_info *info)
{
	const char *type_name;
	pr_info("sis_ts_detect\n");
	type_name = "sis_i2c_ts";
	strlcpy(info->type, type_name, I2C_NAME_SIZE);
	return 0;
}
#endif

static void __exit sis_ts_exit(void)
{
#ifdef _STD_RW_IO
	dev_t dev;
#endif

	pr_info("sis_ts_exit\n");
	i2c_del_driver(&sis_ts_driver);
	if (sis_wq)
		destroy_workqueue(sis_wq);

#ifdef _STD_RW_IO
	dev = MKDEV(sis_char_major, 0);
	cdev_del(&sis_char_cdev);
	unregister_chrdev_region(dev, sis_char_devs_count);
	device_destroy(sis_char_class, MKDEV(sis_char_major, 0));
	class_destroy(sis_char_class);
#endif
        cancel_delayed_work_sync(&driver_reg_work);
}
late_initcall(sis_ts_init);
//module_init(sis_ts_init);
module_exit(sis_ts_exit);
MODULE_DESCRIPTION("SiS 9200 Family Touchscreen Driver");
MODULE_LICENSE("GPL");



