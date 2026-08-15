/* drivers/video/sc8810/lcd_gc9307_spi.c
 *
 *
 *
 *
 * Copyright (C) 2010 Spreadtrum
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "../sprdfb_chip_common.h"
#include "../sprdfb.h"
#include "../sprdfb_panel.h"
#define printk printf

#define  LCD_DEBUG
#ifdef LCD_DEBUG
#define LCD_PRINT printk
#else
#define LCD_PRINT(...)
#endif
#define REG32(x)              (*((volatile uint32 *)(x)))
#define GC9307_SpiWriteCmd(cmd) \
{ \
	spi_send_cmd((cmd & 0xFF));\
}

#define  GC9307_SpiWriteData(data)\
{ \
	spi_send_data((data & 0xFF));\
}

#define  GC9307_SpiRead(data,len)\
{ \
	spi_read(data,len);\
}

#define BGRA32toRBG565(b,g,r)		((((r>>3)&0x1f)<<11)|(((g>>2)&0x3f)<<5)|(((b>>3)&0x1f)<<0))
#define LCM_GPIO_RSTN	(50)
#define GC9307_SPI_SPEED 		(48*1000*1000UL)
#define	SPI_MODE_0	(0|0)
// #define BL_GPIO_EN	(47)

extern unsigned char start_send_pixels_flag;

static int32_t GC9307_reset(void)
{
	static int32_t is_first_run = 1;

	//turn on backlight
	// sprd_gpio_request(NULL,BL_GPIO_EN);
	// sprd_gpio_direction_output(NULL,BL_GPIO_EN,1);
	// sprd_gpio_set(NULL,BL_GPIO_EN,1);

	if(is_first_run)
	{
		sprd_gpio_request(NULL,LCM_GPIO_RSTN);
		sprd_gpio_direction_output(NULL,LCM_GPIO_RSTN,1);
		is_first_run = 0;
	}
	sprd_gpio_set(NULL,LCM_GPIO_RSTN,1);
	mdelay(10);
	sprd_gpio_set(NULL,LCM_GPIO_RSTN,0);
	mdelay(10);
	sprd_gpio_set(NULL,LCM_GPIO_RSTN,1);
	mdelay(20);
}
static int32_t GC9307_refresh(struct panel_spec *self,void *base)
{
	int i = 0;
	uint16_t *prgb = (uint16_t *)base;
	uint16_t rgb;
	LCD_PRINT("GC9307_freshbuffer base=%p\r\n",base);
	start_send_pixels_flag = 1;
	spi_send_cmd_t spi_send_cmd = self->info.spi->ops->spi_send_cmd;
	spi_send_data_t spi_send_data = self->info.spi->ops->spi_send_data;
	spi_read_t spi_read = self->info.spi->ops->spi_read;
	uint32_t lcm_id[4]={0};
	sprd_gpio_set(NULL, LCM_GPIO_CDPIN,1);
	//GC9307_SpiWriteCmd(0x09);
	//GC9307_SpiRead(lcm_id,4);
	//printf("sprdfb:GC9307_read_id lcm id[0-3] = 0x%x %x %x %x\n",lcm_id[0],lcm_id[1],lcm_id[2],lcm_id[3]);
	
	for(i = 0 ; i < 284*240;i++)
	{
		rgb = *prgb++;
		GC9307_SpiWriteData(rgb>>8);
		GC9307_SpiWriteData(rgb&0xff);
	}
	GC9307_SpiWriteCmd(0x29);
	return 0;
}

static int32_t GC9307_init(struct panel_spec *self)
{
	uint32_t data = 0;
	spi_send_cmd_t spi_send_cmd = self->info.spi->ops->spi_send_cmd;
	spi_send_data_t spi_send_data = self->info.spi->ops->spi_send_data;
	spi_read_t spi_read = self->info.spi->ops->spi_read;
	GC9307_reset();




	GC9307_SpiWriteCmd(0xfe);
	GC9307_SpiWriteCmd(0xef);

	GC9307_SpiWriteCmd(0x36);
	GC9307_SpiWriteData(0x48);

	GC9307_SpiWriteCmd(0x3a);
	GC9307_SpiWriteData(0x05);
		
	GC9307_SpiWriteCmd(0x20);

	GC9307_SpiWriteCmd(0x86);	
	GC9307_SpiWriteData(0x98);
	GC9307_SpiWriteCmd(0x89);	
	GC9307_SpiWriteData(0x13);
	GC9307_SpiWriteCmd(0x8a);	
	GC9307_SpiWriteData(0x40);
	GC9307_SpiWriteCmd(0x8b);	
	GC9307_SpiWriteData(0x80);
	GC9307_SpiWriteCmd(0x8d);
	GC9307_SpiWriteData(0x33);	
	GC9307_SpiWriteCmd(0x8e);	 
	GC9307_SpiWriteData(0x0f);	

	GC9307_SpiWriteCmd(0xEC);
	GC9307_SpiWriteData(0x33);
	GC9307_SpiWriteData(0x07);
	GC9307_SpiWriteData(0xAF);

	GC9307_SpiWriteCmd(0xB6);
	GC9307_SpiWriteData(0x00);
	GC9307_SpiWriteData(0x00);
	GC9307_SpiWriteData(0x23);

	GC9307_SpiWriteCmd(0xe8);
	GC9307_SpiWriteData(0x12);
	GC9307_SpiWriteData(0x00);

	GC9307_SpiWriteCmd(0xf6);
	GC9307_SpiWriteData(0x00);
	
	GC9307_SpiWriteCmd(0xff);
	GC9307_SpiWriteData(0x62);

	GC9307_SpiWriteCmd(0x99);	
	GC9307_SpiWriteData(0x3e);
	GC9307_SpiWriteCmd(0x9d);	
	GC9307_SpiWriteData(0x4b);

	GC9307_SpiWriteCmd(0x98);  
	GC9307_SpiWriteData(0x3e);
	GC9307_SpiWriteCmd(0x9c);	
	GC9307_SpiWriteData(0x4b);	
	GC9307_SpiWriteCmd(0xc3);	
	GC9307_SpiWriteData(0x10);
	GC9307_SpiWriteCmd(0xc4);	
	GC9307_SpiWriteData(0x10);
	GC9307_SpiWriteCmd(0xc9);	
	GC9307_SpiWriteData(0x05);

	GC9307_SpiWriteCmd(0xF0);
	GC9307_SpiWriteData(0xc7);
	GC9307_SpiWriteData(0x08);
	GC9307_SpiWriteData(0x08);
	GC9307_SpiWriteData(0x08);
	GC9307_SpiWriteData(0x05);
	GC9307_SpiWriteData(0x2e);

	GC9307_SpiWriteCmd(0xF1);
	GC9307_SpiWriteData(0x45);
	GC9307_SpiWriteData(0x8f);
	GC9307_SpiWriteData(0x6f);
	GC9307_SpiWriteData(0x33);
	GC9307_SpiWriteData(0x36);
	GC9307_SpiWriteData(0x4f);

	GC9307_SpiWriteCmd(0xF2);
	GC9307_SpiWriteData(0xc7);
	GC9307_SpiWriteData(0x08);
	GC9307_SpiWriteData(0x08);
	GC9307_SpiWriteData(0x08);
	GC9307_SpiWriteData(0x05);
	GC9307_SpiWriteData(0x2e);

	GC9307_SpiWriteCmd(0xF3);
	GC9307_SpiWriteData(0x45);
	GC9307_SpiWriteData(0x8f);
	GC9307_SpiWriteData(0x6f);
	GC9307_SpiWriteData(0x33);
	GC9307_SpiWriteData(0x36);
	GC9307_SpiWriteData(0x4f);

	//GC9307_SpiWriteCmd(0xE9);
	//GC9307_SpiWriteData(0x08);//2DATA

	GC9307_SpiWriteCmd(0x35);
	GC9307_SpiWriteData(0x00);//TE
	GC9307_SpiWriteCmd(0x44);
	GC9307_SpiWriteData(0x00);
	GC9307_SpiWriteData(0x0A);//TE scan line
	GC9307_SpiWriteCmd(0x11);
	
	mdelay(120);
	GC9307_SpiWriteCmd(0x29);
	GC9307_SpiWriteCmd(0x2c);

	LCD_PRINT("GC9307_init\n");
}

static int32_t GC9307_enter_sleep(struct panel_spec *self, uint8_t is_sleep)
{
	spi_send_cmd_t spi_send_cmd = self->info.spi->ops->spi_send_cmd;
	spi_send_data_t spi_send_data = self->info.spi->ops->spi_send_data;
	spi_read_t spi_read = self->info.spi->ops->spi_read;

	if(is_sleep==1){
		//Sleep In
		GC9307_SpiWriteCmd(0x28);
		mdelay(120);
		GC9307_SpiWriteCmd(0x10);
		mdelay(10);
	}else{
		//Sleep Out
		GC9307_SpiWriteCmd(0x11);
		mdelay(120);
		GC9307_SpiWriteCmd(0x29);
		mdelay(10);
	}
	return 0;
}



static int32_t GC9307_set_window(struct panel_spec *self,
		uint16_t left, uint16_t top, uint16_t right, uint16_t bottom)
{
	uint32_t *test_data[4] = {0};
	spi_send_cmd_t spi_send_cmd = self->info.spi->ops->spi_send_cmd;
	spi_send_data_t spi_send_data = self->info.spi->ops->spi_send_data;
	spi_read_t spi_read = self->info.spi->ops->spi_read;

	//top=top+36;
	//bottom=bottom+36;
	//left=left+18;
	//right=right+18;

	GC9307_SpiWriteCmd(0x2A);
	GC9307_SpiWriteData((left>>8));// set left address
	GC9307_SpiWriteData((left&0xff));
	GC9307_SpiWriteData((right>>8));// set right address
	GC9307_SpiWriteData((right&0xff));

	GC9307_SpiWriteCmd(0x2B);
	GC9307_SpiWriteData((top>>8));// set left address
	GC9307_SpiWriteData((top&0xff));
	GC9307_SpiWriteData((bottom>>8));// set bottom address
	GC9307_SpiWriteData((bottom&0xff));
	GC9307_SpiWriteCmd(0x2C);

	return 0;
}
static int32_t GC9307_invalidate(struct panel_spec *self)
{
	LCD_PRINT("GC9307_invalidate\n");

	return self->ops->panel_set_window(self, 0, 0,
		self->width - 1, self->height - 1);
}



static int32_t GC9307_invalidate_rect(struct panel_spec *self,
				uint16_t left, uint16_t top,
				uint16_t right, uint16_t bottom)
{
	LCD_PRINT("GC9307_invalidate_rect \n");

	return self->ops->panel_set_window(self, left, top,
			right, bottom);
}
static int32_t GC9307_read_id(struct panel_spec *self)
{
	LCD_PRINT("sprdfb:GC9307_read_id\n");
	spi_send_cmd_t spi_send_cmd = self->info.spi->ops->spi_send_cmd;
	spi_send_data_t spi_send_data = self->info.spi->ops->spi_send_data;
	spi_read_t spi_read = self->info.spi->ops->spi_read;
	uint32_t lcm_id[4]={0};
	GC9307_reset();

	GC9307_SpiWriteCmd(0xFE);
	GC9307_SpiWriteCmd(0xEF);
	GC9307_SpiWriteCmd(0x04);
	GC9307_SpiRead(lcm_id,4);
	LCD_PRINT("sprdfb:GC9307_read_id lcm id[0-3] = 0x%x %x %x %x\n",lcm_id[0],lcm_id[1],lcm_id[2],lcm_id[3]);
	if(lcm_id[1] == 0x93 && lcm_id[2] == 0x07)
		return 0x9307;
	return 0x00;
}

static struct panel_operations lcd_GC9307_spi_operations = {
	.panel_init = GC9307_init,
	.panel_set_window = GC9307_set_window,
	.panel_invalidate_rect= GC9307_invalidate_rect,
	.panel_invalidate = GC9307_invalidate,
	.panel_enter_sleep = GC9307_enter_sleep,
	.panel_readid          = GC9307_read_id,
	.panel_refresh		=GC9307_refresh,
};
/*
static struct info_spi lcd_GC9307_spi_info = {
	.cmd_bus_mode  = SPRDFB_PANEL_TYPE_SPI,
	.bus_width = 1, //18,16
	.bpp = 16,
	.te_pol = SPRDFB_POLARITY_POS,
	.te_sync_delay = 3,
};
*/
static struct info_spi lcd_GC9307_spi_info = {
	.cmd_bus_mode  = SPRDFB_PANEL_TYPE_SPI,
	.bus_num = 0,
	.bus_width = 1,
	.cs = 0,
	.cd_gpio = LCM_GPIO_CDPIN,
	.spi_mode = 1,
	.spi_pol_mode = SPI_MODE_0,
	.speed = GC9307_SPI_SPEED,
};

struct panel_spec lcd_gc9307_spi_spec = {
	.width = 240,
	.height = 284,
	.fps = 33,
	.type = SPRDFB_PANEL_TYPE_SPI,
	.direction = LCD_DIRECT_NORMAL,
	.info = {
		.spi = &lcd_GC9307_spi_info
	},
	.ops = &lcd_GC9307_spi_operations,
};
