/*
 * Command for testing RAM.
 *
 * Copyright (C) 2011 SANYO-CE
 */
#include <common.h>
#include "watchdog.h"

/*
** sdramtest : SDRAM random test
**  sdramtest <start-address> <size> [option 1:short]
*/
static int ram_test (unsigned long addr, unsigned long size, int type, int verbose)
{
	volatile int *p,*px;
	unsigned long pa;
	int i,j,k,v,x;
	int *pp;
	int p0[4]={0x00000000,0xFFFFFFFF,0x55555555,0xAAAAAAAA};
	int p1[4]={0x00000000,0x00000000,0x00000000,0x00000000};
	int p2[4]={0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF};
	int p3[4]={0x55555555,0x55555555,0x55555555,0x55555555};
	int p4[4]={0xAAAAAAAA,0xAAAAAAAA,0xAAAAAAAA,0xAAAAAAAA};
	int p5[4]={0x00000000,0xFFFFFFFF,0x00000000,0xFFFFFFFF};
	int p6[4]={0x55555555,0xAAAAAAAA,0x55555555,0xAAAAAAAA};
	int p7[4]={0x00000000,0x55555555,0xFFFFFFFF,0xAAAAAAAA};

	if(verbose)
		printf("Test from 0x%08lx to 0x%08lx (%ldM) type:0x%x\n",addr,addr+size,size/0x100000,type);
	
	/* 0000/FFFF/AAAA/5555 test */
	switch(type){
	case 2:
	case 3:
		pp=p1;
		break;
	case 4:
	case 5:
		pp=p2;
		break;
	case 6:
	case 7:
		pp=p3;
		break;
	case 8:
	case 9:
		pp=p4;
		break;
	case 10:
	case 11:
		pp=p5;
		break;
	case 12:
	case 13:
		pp=p6;
		break;
	case 14:
	case 15:
		pp=p7;
		break;
	case 100:
		break;
	default:
		pp=p0;
		break;
	}
	
	if(type!=100){
		printf("%08x/%08x/%08x/%08x test\n",pp[0],pp[1],pp[2],pp[3]);
		for(j=0;j<4;j++){
			p=(volatile int*)addr;
			for(i=0;i<(size/sizeof(int));i++){
				if(type&1)
					v=*p;
				*p++=pp[(i+j)%4];
				if((i%100)==0){
					WATCHDOG_RESET();  /* reset safe */
				}
			}
			p=(volatile int*)addr;
			for(i=0;i<(size/sizeof(int));i++){
				v=*p++;
				x=pp[(i+j)%4];
				if(v!=x){
					printf("%x %x - %x\n",(int)p-4,v,x);
					return 1;
				}
				if((i%100)==0){
					WATCHDOG_RESET();  /* reset safe */
				}
			}
			if(verbose)
				printf("%d/%d %lx - %lx pass\n",j,4,addr,addr+size);
		}
	}
	
	if(type!=100){
		/* write address value itself */
		printf("address test\n");
		p=(volatile int*)addr;
		for(i=0;i<(size/sizeof(int));i++ ){
			pa=(unsigned long)p;
			if(i&1)
				*p++=(pa&0xFFFF0000)|((~pa)&0x0000FFFF);
			else
				*p++=((~pa)&0xFFFF0000)|(pa&0x0000FFFF);
			if((i%100)==0){
				WATCHDOG_RESET();  /* reset safe */
			}
		}
		p=(volatile int*)addr;
		for(i=0;i<(size/sizeof(int));i++){
			pa=(unsigned long)p;
			if(i&1)
				x=(pa&0xFFFF0000)|((~pa)&0x0000FFFF);
			else
				x=((~pa)&0xFFFF0000)|(pa&0x0000FFFF);
			v=*p++;
			if(v!=x){
				printf ("%x : %x - %x\n", (int)p - 4, v, x);
				return 1;
			}
			if((i%100)==0){
				WATCHDOG_RESET();  /* reset safe */
			}
		}
		if(verbose)
			printf("address pattern %lx - %lx pass\n",addr,addr+size);
	}
	
	/* cache size test */
	printf("cache size test\n");
	for(k=0;k<4;k++){
		printf("cache size test 1st\n");
		for(i=0;i<(32/sizeof(int));i++){
			p=(volatile int*)(addr+i*sizeof(int));
			px=(volatile int*)(addr+i*sizeof(int));
			while((unsigned int)p<(addr+size)){
				*p=(int)((unsigned long)p+k);
				p=(int*)((unsigned int)p+32);
				if(((addr+size)/2)<=(unsigned long)p){
					v=*px;
					x=(unsigned long)px+k;
					if(v!=x){
						printf("%p : %x - %x\n",px,v,x);
						return 1;
					}
					px=(int*)((unsigned int)px+32);
				}
			}
			WATCHDOG_RESET();  /* reset safe */
		}
		printf("cache size test 2nd\n");
		for(i=0;i<(32/sizeof(int));i++ ){
			p=(volatile int*)(addr+i*sizeof(int));
			while((unsigned int)p<(addr+size)){
				v=*p;
				x=(unsigned long)p+k;
				if(v!=x){
					printf("%p : %x - %x\n",p,v,x);
					return 1;
				}
				p=(int*)((unsigned int)p+32);
			}
			WATCHDOG_RESET();  /* reset safe */
		}
		if(verbose)
			printf("%d/%d %lx - %lx pass\n",k,4,addr,addr+size);
	}
	
	return 0;
}

static int do_sdramtest(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int i,j;
	unsigned long addr=0;
	unsigned long size=0;
	int verbose=1;
	int rc,type;
	int type_limit=16;
	unsigned long opt=0;

	if(argc<3){
		cmd_usage(cmdtp);
		return 1;
	}
	
	addr=simple_strtoul((char*)argv[1],NULL,16);
	size=simple_strtoul((char*)argv[2],NULL,16);
	
	if(argc==4){
		opt=simple_strtoul((char*)argv[3],NULL,10);
		switch(opt){
		case 1:
		case 2:
			type_limit=1;
			break;
		default:
			opt=0;
		}
	}
	
	for (type=0;type<type_limit;type++){
		rc=ram_test(addr,size,((opt==2)?100:type),verbose);
		if(rc!=0)
			break;
	}
	
	return 0;
}

U_BOOT_CMD(
	sdramtest,	4,	3,	do_sdramtest,
	"test SDRAM",
	"<start address> <lenght> [option 1:short,2:cache size test only]");
