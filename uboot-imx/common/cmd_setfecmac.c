#include <common.h>

#define FECMAC_OFF				0x15
#define FECMAC_CHECK_OFF		0x1d

static int do_set_fecmac(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	unsigned long temp;
	unsigned char data[8];
	int ret, i,check;
	char tmpbuff[32];
	unsigned char buff[512];
	
	memset(data,0x00,sizeof(data));
	memset(tmpbuff,'\0',sizeof(tmpbuff));
	memset(buff,0x00,sizeof(buff));

	if (argc == 1) {
		goto print_mac_info;
	}

	if (argc != 2) {
		ret = -1;
		goto error;
	}
	for (i = 0,check = 0;  i < 6;  i++) {
		if (!parse_num(*(&argv[1]), &temp, &argv[1], ":")) {
			ret = -2;
			goto error;
		}
		if (temp > 0xFF) {
			ret = -3;
			goto error;
		}
		data[i] = temp & 0xFF;
		check += data[i];
	}
	
	if(read_serialmac(buff)) {
		ret = -4;
		goto error;
	}
	
	memcpy(buff+FECMAC_OFF,data,sizeof(data));
	memcpy(buff+FECMAC_CHECK_OFF,&check,sizeof(check));
	
	/* Set the value */
	if(saveserialmac(buff)) {
		ret = -5;
		goto error;
	}
	
print_mac_info:
	/* Read the Mac address and print it */
	if(read_serialmac(buff)) {
		ret = -6;
		goto error;
	}
	
	sprintf(tmpbuff,"0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x"
				,buff[FECMAC_OFF], buff[FECMAC_OFF+1], buff[FECMAC_OFF+2], buff[FECMAC_OFF+3], buff[FECMAC_OFF+4], buff[FECMAC_OFF+5]);
	printf("\nFEC MAC address: %s\n\n",tmpbuff);
	
	return 0;

error:
	printf("Wrong value for set_fecmac. Error=%d\n\n", ret);
}

U_BOOT_CMD(
	set_fecmac,	2,	0,	do_set_fecmac,
	"Set FEC MAC address",
	"[0x##:0x##:0x##:0x##:0x##:0x##]");
