#include <linux/mu3phy/mtk-phy.h>
#ifdef CONFIG_U3D_HAL_SUPPORT
#include <linux/mu3d/hal/mu3d_hal_osal.h>
#endif

#ifdef CONFIG_U3_PHY_AHB_SUPPORT

PHY_INT32 U3PhyWriteReg32(PHY_UINT32 addr, PHY_UINT32 data)
{
	if (0) os_printk(K_DEBUG, "%s addr=%x,data=%x\n", __func__, addr, data);
	writel(data, (void __iomem *)addr);
	return 0;
}

PHY_INT32 U3PhyReadReg32(PHY_UINT32 addr)
{
	return readl((void __iomem *)addr);
}

PHY_INT32 U3PhyWriteReg8(PHY_UINT32 addr, PHY_UINT8 data)
{
	os_writelmsk(addr&0xfffffffc, data<<((addr%4)*8), 0xff<<((addr%4)*8));

	return 0;
}

PHY_INT8 U3PhyReadReg8(PHY_UINT32 addr)
{
	return ((readl((void __iomem *)addr)>>((addr%4)*8))&0xff);
}

#endif

