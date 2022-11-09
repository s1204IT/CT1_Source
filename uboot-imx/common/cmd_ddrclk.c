#include <common.h>
#include <asm/io.h>

#define SZ_DEC_1M       1000000
#define AIPS2_ARB_BASE_ADDR             0x02100000
#define AIPS_TZ2_BASE_ADDR              AIPS2_ARB_BASE_ADDR
#define AIPS2_OFF_BASE_ADDR             AIPS_TZ2_BASE_ADDR+0x80000
#define MMDC_P0_IPS_BASE_ADDR               AIPS2_OFF_BASE_ADDR+0x30000
#define MMDC_P1_IPS_BASE_ADDR               AIPS2_OFF_BASE_ADDR+0x34000
	
#define PHY_BASE_ADDR  0x800
#define MPMUR_OFFSET        PHY_BASE_ADDR + 0xB8

#define DIV2    1
#define DIV1    0

#define CCM_CBCMR_OFFSET 0x18
#define CCM_CBCDR_OFFSET        0x14
#define CCM_CDHIPR_OFFSET 0x48
	
#define AIPS1_ARB_BASE_ADDR             0x02000000
#define AIPS_TZ1_BASE_ADDR              AIPS1_ARB_BASE_ADDR
#define AIPS1_OFF_BASE_ADDR             AIPS_TZ1_BASE_ADDR+0x80000
#define CCM_IPS_BASE_ADDR               AIPS1_OFF_BASE_ADDR+0x44000
#define ANATOP_IPS_BASE_ADDR            AIPS1_OFF_BASE_ADDR+0x48000
	
#define CCM_CBCMR   (CCM_IPS_BASE_ADDR+CCM_CBCMR_OFFSET)
#define CCM_CBCDR   (CCM_IPS_BASE_ADDR+CCM_CBCDR_OFFSET)
#define CCM_CDHIPR  (CCM_IPS_BASE_ADDR+CCM_CDHIPR_OFFSET)
	
#define HW_ANADIG_PFD_528_RW        (ANATOP_IPS_BASE_ADDR+0x100)    // 528MHz Clock Phase Fractional Divider Control Register
#define HW_ANADIG_PLL_528_NUM   (ANATOP_IPS_BASE_ADDR+0x50) // Numerator of 528MHz PLL Fractional Loop Divider Register
#define HW_ANADIG_PLL_528_RW    (ANATOP_IPS_BASE_ADDR+0x30) // Anadig 528MHz PLL Control register
#define HW_ANADIG_PLL_528_DENOM     (ANATOP_IPS_BASE_ADDR+0x60) // Denominator of 528MHz PLL Fractional Loop Divider Register

#if 0
#define ADOPT_BASE_ADDR  0x400
#define MAPSR_OFFSET      ADOPT_BASE_ADDR + 0x04    // PSR (former MCR0 and PSM0)

#define EPIT1_IPS_BASE_ADDR             AIPS1_OFF_BASE_ADDR+0x50000
#define EPIT_BASE_ADDR          EPIT1_IPS_BASE_ADDR

#define EPIT_EPITCNR_OFFSET	0x10    //  32bit timer 3 counter reg
#define EPITCNR     EPIT_EPITCNR_OFFSET
	
unsigned int ref_clock(void)
{
    return 0xFFFFFFFF - readl(EPIT_BASE_ADDR + EPITCNR);
}

void exit_fail(void)
{
    u32 temp;

    // disable MMDC power savings so that the SDCLK can be probed if necessary
    reg32_write_mask((MMDC_P0_IPS_BASE_ADDR + MAPSR_OFFSET), 0x00000001, 0x00000001);

    while (1) {
        // delay between each print
        temp = ref_clock();
        while ((ref_clock() - temp) < 98304) ;  // wait approx 3 seconds 
    }
}
#endif

#define DDR_FREQ_T_NUM 22
const int ddr_freq_table[DDR_FREQ_T_NUM][2] = {
//	{678000000,64},
//	{672000000,63},
//	{666000000,62},
//	{660000000,61},
//	{654000000,60},
//	{648000000,59},
//	{641000000,58},
//	{635000000,57},
//	{629000000,56},
//	{623000000,55},
//	{621000000,54},
//	{615000000,53},
//	{610000000,52},
//	{604000000,51},
	{599000000,50},
	{594000000,49},
	{591000000,48},
	{585000000,47},
	{580000000,46},
	{576000000,45},
	{571000000,44},
	{566000000,43},
	{561000000,42},
	{556000000,41},
	{552000000,40},
	{547000000,39},
	{542000000,38},
	{537000000,37},
	{532000000,36},
	{528000000,35},
	{500000000,34},
	{475000000,33},
	{452000000,32},
	{432000000,31},
	{413000000,30},
	{396000000,29},
//	{380000000,28},
//	{365000000,27},
//	{352000000,26},
//	{339000000,25},
//	{327000000,24},
//	{316000000,23},
//	{306000000,22},
//	{297000000,21},
//	{288000000,20},
//	{279000000,19},
//	{271000000,18},
//	{267000000,17},
//	{250000000,16},
//	{237000000,15},
//	{226000000,14},
//	{216000000,13},
//	{206000000,12},
//	{198000000,11},
//	{190000000,10},
//	{182000000,9},
//	{176000000,8},
//	{169000000,7},
//	{163000000,6},
//	{158000000,5},
//	{153000000,4},
//	{148000000,3},
//	{144000000,2},
//	{139000000,1},
//	{135000000,0},
	{0,-1}
};

static void reg32_write_mask(u32 addr, u32 wdata, u32 mask)
{
	u32 masked_wdata, rdata;

	rdata = readl(addr);
	masked_wdata = (((~mask) & rdata) | (mask & wdata));
    
	writel(masked_wdata, addr);
}

/* For DDR frequencies >= 528MHz  */
/* PLL denom is set to 1000, numerator is varied from 0, 200, 400, 600, 800, 999  */
/* This yields fractional part as 0, 0.2, 0.4, 0.6, 0.8, and 0.999 */
u32 increase_pll528(int i)
{
	u32 numerator;
	u32 ddr_freq_above_528;     // pass back the integer value fo the ddr freq
	double ddr_freq_act;        // this variable is used to calculate the actual ddr freq
	int temp;

	// make sure that mmdc_ch0_axi_podf is div-by-1
	reg32_write_mask(CCM_CBCDR, 0x00000000, 0x00380000);

	// poll the mmdr_ch0_podf_busy bit
	while (readl(CCM_CDHIPR) & 0x00000010) ;

	temp = readl(CCM_CBCDR);
	if ((temp & 0x00380000) != 0) {
		printf("MMDC podf didn't get changed! \n");
		reg32_write_mask(CCM_CBCDR, 0x00000000, 0x00380000);
	}
	// switch the periph_clk domain to "pll3" (480MHz USB OTG PLL3) while we are re-programming 528PLL
	// first, set CBCMR:periph_clk2_sel to select "pll3" bits[13:12]=00 for the periph_clk2_clk clock mux
	reg32_write_mask(CCM_CBCMR, 0x00000000, 0x00003000);
	// next, set CBCDR:periph_clk_slk to the periph_clk2_clk clock source (that got set up for "pll3")
	// bit[25]=1
	reg32_write_mask(CCM_CBCDR, 0x02000000, 0x02000000);

	// set div_sel to 1 to have a multiplier of 22 (bit 0); 24*(22 + num/den)
	reg32_write_mask(HW_ANADIG_PLL_528_RW, 0x00000001, 0x00000001);

	// MFD is always 1000 (to allow for div-by-1000 in the equation MFN/(MFD)
	//reg32_write(HW_ANADIG_PLL_528_DENOM, 1000);
	writel(1000, HW_ANADIG_PLL_528_DENOM);

	// increase dividers that are sourced from the 528PLL: AXI_CLK_ROOT and 132M_CLK_ROOT
	// set CBCDR:axi_podf to div-by-3 bits[18:16]=010 and CBCDR:ahb_podf to div-by-6 bits[12:10]=101
	reg32_write_mask(CCM_CBCDR, 0x00021400, 0x00071C00);

	// 528MHz < freq < 576
	// this chooses the PLL output as source for DDR
	if ((i >= 0) && (i <= 10)) {
		// first, for the pre_periph_clk_sel mux, choose the PLL2 output (not PFD)
		// Ensure pre_periph_clk_sel mux is set to the 528PLL, CBCMR:pre_periph_clk_sel bits[19:18]=00
		reg32_write_mask(CCM_CBCMR, 0x00000000, 0x000C0000);

		//reg32_write(HW_ANADIG_PLL_528_NUM, i * (200));
		writel(i * (200), HW_ANADIG_PLL_528_NUM);
		
		// wait for 528MHz PLL to lock
		while (!(readl(HW_ANADIG_PLL_528_RW) & 0x80000000)) ;

		ddr_freq_act = (float)24 *(22 + (float)(i * 200) / 1000);
		ddr_freq_above_528 = ddr_freq_act;  // turn it to an integer
		// next, set CBCDR:periph_clk_slk to the 528PLL
		// bit[25]=0
		reg32_write_mask(CCM_CBCDR, 0x00000000, 0x02000000);

		return ddr_freq_above_528;
	}
	// 580MHz < freq < 591MHz
	// this chooses the PFD2 output as source for DDR
	else if ((i >= 11) && (i <= 29)) {
		// first, for the pre_periph_clk_sel mux, choose the PFD2 output
		// ungate PFD2, clear bit 23
		reg32_write_mask(HW_ANADIG_PFD_528_RW, 0x00000000, 0x00800000);
		// Switch CBCMR pre_periph_clk_sel to the PFD2 source 
		// bits[19:18] = 01
		reg32_write_mask(CCM_CBCMR, 0x00040000, 0x000C0000);

		// 580MHz < freq < 591MHz
		// use 480Mhz (div_sel=0) and PFD2 with N=16
		if ((i >= 11) && (i <= 13)) {
			// set div_sel to 0 to have a multiplier of 20 (bit 0); 24*(20 + num/den)
			reg32_write_mask(HW_ANADIG_PLL_528_RW, 0x00000000, 0x00000001);

			// now set PFD2 frac (N) to 16
			reg32_write_mask(HW_ANADIG_PFD_528_RW, 0x00100000, 0x003F0000);

			if (i == 11) {
			    //reg32_write(HW_ANADIG_PLL_528_NUM, 1500);
			    writel(1500, HW_ANADIG_PLL_528_NUM);
			    ddr_freq_act = (float)24 *(20 + (float)1500 / 1000) * (float)18 / 16;
			} else if (i == 12) {
			    //reg32_write(HW_ANADIG_PLL_528_NUM, 1700);
			    writel(1700, HW_ANADIG_PLL_528_NUM);
			    ddr_freq_act = (float)24 *(20 + (float)1700 / 1000) * (float)18 / 16;
			} else if (i == 13) {
			    //reg32_write(HW_ANADIG_PLL_528_NUM, 1900);
			    writel(1900, HW_ANADIG_PLL_528_NUM);
			    ddr_freq_act = (float)24 *(20 + (float)1900 / 1000) * (float)18 / 16;
			}
			// wait for 528MHz PLL to lock
			while (!(readl(HW_ANADIG_PLL_528_RW) & 0x80000000)) ;

			ddr_freq_above_528 = ddr_freq_act;  // turn it to an integer
			// next, set CBCDR:periph_clk_slk to the 528PLL mux
			// bit[25]=0
			reg32_write_mask(CCM_CBCDR, 0x00000000, 0x02000000);

			return ddr_freq_above_528;
		}

		/* freq = 594 to 620MHz */
		// use 528Mhz (div_sel=1) and PFD2 with N=16
		else if ((i >= 14) && (i <= 19)) {
			// set div_sel to 0 to have a multiplier of 22 (bit 1); 24*(22 + num/den)
			reg32_write_mask(HW_ANADIG_PLL_528_RW, 0x00000001, 0x00000001);

			// program the PLL num/den fractional part to increase PLL output
			//reg32_write(HW_ANADIG_PLL_528_NUM, (i - 14) * 200);
			writel((i - 14) * 200, HW_ANADIG_PLL_528_NUM);

			// wait for 528MHz PLL to lock
			while (!(readl(HW_ANADIG_PLL_528_RW) & 0x80000000)) ;

			// now set PFD2 frac (N) to 16
			reg32_write_mask(HW_ANADIG_PFD_528_RW, 0x00100000, 0x003F0000);

			ddr_freq_act = (float)24 *(22 + (float)(i - 14) * 200 / 1000) * (float)18 / 16;
			ddr_freq_above_528 = ddr_freq_act;  // turn it to an integer
			// next, set CBCDR:periph_clk_slk to the 528PLL mux
			// bit[25]=0
			reg32_write_mask(CCM_CBCDR, 0x00000000, 0x02000000);

			return ddr_freq_above_528;
		}

		/* freq = 623 to 678MHz */
		// use 480Mhz (div_sel=0) and PFD2 with N=14        
		else if ((i >= 20) && (i <= 29)) {
			// set div_sel to 0 to have a multiplier of 20 (bit 0); 24*(20 + num/den)
			reg32_write_mask(HW_ANADIG_PLL_528_RW, 0x00000000, 0x00000001);

			// program the PLL num/den fractional part to increase PLL output
			//reg32_write(HW_ANADIG_PLL_528_NUM, (i - 19) * 200);
			writel((i - 19) * 200, HW_ANADIG_PLL_528_NUM);

			// now set PFD2 frac (N) to 14
			reg32_write_mask(HW_ANADIG_PFD_528_RW, 0x000E0000, 0x003F0000);

			ddr_freq_act = (float)24 *(20 + (float)(i - 19) * 200 / 1000) * (float)18 / 14;
			ddr_freq_above_528 = ddr_freq_act;  // turn it to an integer
			// next, set CBCDR:periph_clk_slk to the 528PLL mux
			// bit[25]=0
			reg32_write_mask(CCM_CBCDR, 0x00000000, 0x02000000);

			return ddr_freq_above_528;
		}
	}                           // fail if invalid freq
	else {
		printf("Error, outside of supported frequencies, setting back to 528MHz \n");
		//reg32_write(HW_ANADIG_PLL_528_NUM, 0);
		writel(0, HW_ANADIG_PLL_528_NUM);
	}
}

static void increase_528pfd2(int i, int divider)
{
	u32 frac;

	// switch the periph_clk domain to "pll3" (480MHz USB OTG PLL?)
	// first, set CBCMR:periph_clk2_sel to select "pll3" bits[13:12]=00 for the periph_clk2_clk clock mux
	reg32_write_mask(CCM_CBCMR, 0x00000000, 0x00003000);

	// next, set CBCDR:periph_clk_sel to the periph_clk2_sel clock source (that got set up for "pll3")
	// bit[25]=1
	reg32_write_mask(CCM_CBCDR, 0x02000000, 0x02000000);

	// ungate PFD2, clear bit 23
	reg32_write_mask(HW_ANADIG_PFD_528_RW, 0x00000000, 0x00800000);

	// make sure PLL numerator is 0, which maintains a 528MHz PLL output into the PFD2    
	//reg32_write(HW_ANADIG_PLL_528_NUM, 0);
	writel(0,HW_ANADIG_PLL_528_NUM);

	// also, make sure DIV_SELECT is set to 1 in the CCM_ANALOG_PLL_528 (HW_ANADIG_PLL_528_RW)
	// register to ensure Fout=Fref*22 (24MHz * 22) = 528MHz
	reg32_write_mask(HW_ANADIG_PLL_528_RW, 0x00000001, 0x00000001);

	// wait for 528MHz PLL to lock
	while (!(readl(HW_ANADIG_PLL_528_RW) & 0x80000000)) ;

	// set dividers that are sourced from the 528PLL: AXI_CLK_ROOT and 132M_CLK_ROOT to default 
	// set CBCDR:axi_podf to div-by-1 bits[18:16]=001 and CBCDR:ahb_podf to div-by-4 bits[12:10]=011
	reg32_write_mask(CCM_CBCDR, 0x00010C00, 0x00071C00);

	// configure the MMDC divider in the CCM
	// also configure the frac divider
	if (divider == DIV1) {

		// mmdc_ch0_axi_podf is div-by-1
		reg32_write_mask(CCM_CBCDR, 0x00000000, 0x00380000);
		// poll the mmdr_ch0_podf_busy bit
		while (readl(CCM_CDHIPR) & 0x00000010) ;

		frac = 53 - i;
	} else if (divider == DIV2) {

		// mmdc_ch0_axi_podf is div-by-2
		reg32_write_mask(CCM_CBCDR, 0x00080000, 0x00380000);
		// poll the mmdr_ch0_podf_busy bit
		while (readl(CCM_CDHIPR) & 0x00000010) ;

		frac = 35 - i;
	} else {
		printf("** Incorrect divder value \n");
		//exit_fail();
	}

	// write the frac value into PFD2
	frac = frac << 16;
	reg32_write_mask(HW_ANADIG_PFD_528_RW, frac, 0x003F0000);

	// Switch CBCMR pre_periph_clk_sel to the 400M PFD2 source 
	// bits[19:18] = 01
	reg32_write_mask(CCM_CBCMR, 0x00040000, 0x000C0000);

	// Now switch CBCDR:periph_clk_sel to the pre_periph_clk_sel mux clock source 
	// bit[25]=0
	reg32_write_mask(CCM_CBCDR, 0x00000000, 0x02000000);
}

static u32 increase_ddr_freq(int i)
{
    u32 ddr_freq;

	// First make sure that the PHY measurement unit is NOT in bypass mode 
	reg32_write_mask((MMDC_P0_IPS_BASE_ADDR+MPMUR_OFFSET), 0x00000000, 0x00000400);
	reg32_write_mask((MMDC_P1_IPS_BASE_ADDR+MPMUR_OFFSET), 0x00000000, 0x00000400);

	// use PFD2 as clock source, with div-by-2    
	// 135MHz < DDR freq < 264MHz
	if ((i >= 0) && (i <= 17)) {
		increase_528pfd2(i, DIV2);
		ddr_freq = (528 * 18 / (35 - i)) / 2;
		return ddr_freq;
	}
	// use PFD2 as clock source, div-by-1
	// 271MHz < DDR freq < 500MHz
	else if ((i >= 18) && (i <= 34)) {
		increase_528pfd2(i, DIV1);
		ddr_freq = (528 * 18 / (53 - i));
		return ddr_freq;
	}
	// for freq >=528MHz
	// will either use the PLL2 or PFD2 output depending on freq
	else if ((i >= 35) && (i <= 59)) {

		// have to normalize the variable 'i' starting back at zero
		ddr_freq = increase_pll528(i - 35);
		return ddr_freq;
	}
	else {
		printf("**Not a supported freq, stopping \n");
	    //exit_fail();
	}
}

static int set_ddr_clk(u32 ddr_clk)
{
	u32 ddr_freq;
	
	int i,freq_i;
	for(i=0;i<DDR_FREQ_T_NUM;i++){
		if(ddr_freq_table[i][0] <= ddr_clk)
			break;
	}
	
	freq_i = ddr_freq_table[i][1];
	ddr_freq = increase_ddr_freq(freq_i);
	
	printf("ddr_freq %d\n",ddr_freq);
	
	return 0;
}
	
int do_ddrclkops(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int rc = 0;
	int freq = 0;

	if(argc!=2){
		cmd_usage(cmdtp);
		return 1;
	}
	
	freq=simple_strtoul((char*)argv[1],NULL,10);
	
	freq *= SZ_DEC_1M;
	
	set_ddr_clk(freq);

	return rc;
}

U_BOOT_CMD(
	ddrclk, 2, 1, do_ddrclkops,
	"Setup/Display DDR clock",
	"ddrclk <DDR clock in MHz> - Setup/Display DDR clock\n");
