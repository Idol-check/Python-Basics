/*******************************************************************************
 * Copyright (C) 2016, Huada Semiconductor Co., Ltd. All rights reserved.
 *
 * This software is owned and published by:
 * Huada Semiconductor Co., Ltd. ("HDSC").
 *
 * BY DOWNLOADING, INSTALLING OR USING THIS SOFTWARE, YOU AGREE TO BE BOUND
 * BY ALL THE TERMS AND CONDITIONS OF THIS AGREEMENT.
 *
 * This software contains source code for use with HDSC
 * components. This software is licensed by HDSC to be adapted only
 * for use in systems utilizing HDSC components. HDSC shall not be
 * responsible for misuse or illegal use of this software for devices not
 * supported herein. HDSC is providing this software "AS IS" and will
 * not be responsible for issues arising from incorrect user implementation
 * of the software.
 *
 * Disclaimer:
 * HDSC MAKES NO WARRANTY, EXPRESS OR IMPLIED, ARISING BY LAW OR OTHERWISE,
 * REGARDING THE SOFTWARE (INCLUDING ANY ACCOMPANYING WRITTEN MATERIALS),
 * ITS PERFORMANCE OR SUITABILITY FOR YOUR INTENDED USE, INCLUDING,
 * WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, THE IMPLIED
 * WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE OR USE, AND THE IMPLIED
 * WARRANTY OF NONINFRINGEMENT.
 * HDSC SHALL HAVE NO LIABILITY (WHETHER IN CONTRACT, WARRANTY, TORT,
 * NEGLIGENCE OR OTHERWISE) FOR ANY DAMAGES WHATSOEVER (INCLUDING, WITHOUT
 * LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION,
 * LOSS OF BUSINESS INFORMATION, OR OTHER PECUNIARY LOSS) ARISING FROM USE OR
 * INABILITY TO USE THE SOFTWARE, INCLUDING, WITHOUT LIMITATION, ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL OR CONSEQUENTIAL DAMAGES OR LOSS OF DATA,
 * SAVINGS OR PROFITS,
 * EVEN IF Disclaimer HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * YOU ASSUME ALL RESPONSIBILITIES FOR SELECTION OF THE SOFTWARE TO ACHIEVE YOUR
 * INTENDED RESULTS, AND FOR THE INSTALLATION OF, USE OF, AND RESULTS OBTAINED
 * FROM, THE SOFTWARE.
 *
 * This software may be replicated in part or whole for the licensed use,
 * with the restriction that this Disclaimer and Copyright notice must be
 * included with each copy of this software, whether used in part or whole,
 * at all times.
 */
/******************************************************************************/
/** \file main.c
 **
 ** \brief This sample demonstrates UART data receive and transfer by DMA.
 **
 **   - 2018-11-27  1.0  Hongjh First version for Device Driver Library of USART
 **
 ******************************************************************************/

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32_ddl.h"

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* DMAC */
#define DMA_UNIT                        (M4_DMA1)
#define DMA_CH                          (DmaCh0)
#define DMA_TRG_SEL                     (EVT_USART3_RI)

/* USART channel definition */
#define USART_CH                        (M4_USART3)

/* USART baudrate definition */
#define USART_BAUDRATE                  (115200ul)

/* USART RX Port/Pin definition */
#define USART_RX_PORT                   (PortE)
#define USART_RX_PIN                    (Pin04)
#define USART_RX_FUNC                   (Func_Usart3_Rx)

/* USART TX Port/Pin definition */
#define USART_TX_PORT                   (PortE)
#define USART_TX_PIN                    (Pin05)
#define USART_TX_FUNC                   (Func_Usart3_Tx)

/* USART interrupt  */
#define USART_EI_NUM                    (INT_USART3_EI)
#define USART_EI_IRQn                   (Int001_IRQn)

/* DMA block transfer complete interrupt */
#define DMA_BTC_INT_NUM                 (INT_DMA1_BTC0)
#define DMA_BTC_INT_IRQn                (Int002_IRQn)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static void ClkInit(void);
static void DmaInit(void);
static void DmaBtcIrqCallback(void);
static void UsartErrIrqCallback(void);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 *******************************************************************************
 ** \brief Initialize Clock.
 **
 ** \param [in] None
 **
 ** \retval None
 **
 ******************************************************************************/
static void ClkInit(void)
{
    stc_clk_xtal_cfg_t   stcXtalCfg;
    stc_clk_mpll_cfg_t   stcMpllCfg;
    en_clk_sys_source_t  enSysClkSrc;
    stc_clk_sysclk_cfg_t stcSysClkCfg;

    MEM_ZERO_STRUCT(enSysClkSrc);
    MEM_ZERO_STRUCT(stcSysClkCfg);
    MEM_ZERO_STRUCT(stcXtalCfg);
    MEM_ZERO_STRUCT(stcMpllCfg);

    /* Set bus clk div. */
    stcSysClkCfg.enHclkDiv  = ClkSysclkDiv1;
    stcSysClkCfg.enExclkDiv = ClkSysclkDiv2;
    stcSysClkCfg.enPclk0Div = ClkSysclkDiv1;
    stcSysClkCfg.enPclk1Div = ClkSysclkDiv2;
    stcSysClkCfg.enPclk2Div = ClkSysclkDiv4;
    stcSysClkCfg.enPclk3Div = ClkSysclkDiv4;
    stcSysClkCfg.enPclk4Div = ClkSysclkDiv2;
    CLK_SysClkConfig(&stcSysClkCfg);

    /* Switch system clock source to MPLL. */
    /* Use Xtal as MPLL source. */
    stcXtalCfg.enMode = ClkXtalModeOsc;
    stcXtalCfg.enDrv = ClkXtalLowDrv;
    stcXtalCfg.enFastStartup = Enable;
    CLK_XtalConfig(&stcXtalCfg);
    CLK_XtalCmd(Enable);

    /* MPLL config. */
    stcMpllCfg.pllmDiv = 1u; /* XTAL 8M / 1 */
    stcMpllCfg.plln = 50u;   /* 8M*50 = 400M */
    stcMpllCfg.PllpDiv = 4u; /* MLLP = 100M */
    stcMpllCfg.PllqDiv = 4u; /* MLLQ = 100M */
    stcMpllCfg.PllrDiv = 4u; /* MLLR = 100M */
    CLK_SetPllSource(ClkPllSrcXTAL);
    CLK_MpllConfig(&stcMpllCfg);

    /* flash read wait cycle setting */
    EFM_Unlock();
    EFM_SetLatency(EFM_LATENCY_4);
    EFM_Lock();

    /* Enable MPLL. */
    CLK_MpllCmd(Enable);

    /* Wait MPLL ready. */
    while (Set != CLK_GetFlagStatus(ClkFlagMPLLRdy))
    {
    }

    /* Switch system clock source to MPLL. */
    CLK_SetSysClkSource(CLKSysSrcMPLL);
}
uint8_t buf[30];
uint8_t rx_cnt;
/**
 *******************************************************************************
 ** \brief Initialize DMA.
 **
 ** \param [in] None
 **
 ** \retval None
 **
 ******************************************************************************/
static void DmaInit(void)
{
    stc_dma_config_t stcDmaInit;
    stc_irq_regi_conf_t stcIrqRegiCfg;

    /* Enable peripheral clock */
    PWC_Fcg0PeriphClockCmd(PWC_FCG0_PERIPH_DMA1 | PWC_FCG0_PERIPH_DMA2,Enable);

    /* Enable DMA. */
    DMA_Cmd(DMA_UNIT,Enable);

    /* Initialize DMA. */
    MEM_ZERO_STRUCT(stcDmaInit);
    stcDmaInit.u16BlockSize = 1u; /* 1 block */
//	   stcDmaInit.u32SrcAddr = (uint32_t)(buf); /* Set source address. */
    stcDmaInit.u32SrcAddr = (uint32_t)(&buf[1]); /* Set source address. */
    stcDmaInit.u32DesAddr = (uint32_t)(&USART_CH->DR);     /* Set destination address. */
    stcDmaInit.stcDmaChCfg.enSrcInc = AddressIncrease;  /* Set source address mode. */
    stcDmaInit.stcDmaChCfg.enDesInc = AddressFix;  /* Set destination address mode. */
    stcDmaInit.stcDmaChCfg.enIntEn = Enable;       /* Enable interrupt. */
    stcDmaInit.stcDmaChCfg.enTrnWidth = Dma8Bit;   /* Set data width 8bit. */
    stcDmaInit.stcDmaChCfg.enSrcRptEn=1;
    DMA_InitChannel(DMA_UNIT, DMA_CH, &stcDmaInit);
//    M4_DMA1->DTCTL0_f.CNT=10;
    /* Enable the specified DMA channel. */
   // DMA_ChannelCmd(DMA_UNIT, DMA_CH, Enable);

    /* Clear DMA flag. */
    DMA_ClearIrqFlag(DMA_UNIT, DMA_CH, TrnCpltIrq);


//    /* Enable peripheral circuit trigger function. */
    PWC_Fcg0PeriphClockCmd(PWC_FCG0_PERIPH_AOS,Enable);

//    /* Set DMA trigger source. */
    
//		 AOS_COM_Trigger1(EVT_AOS_STRG);//将公共触发源设置为 aos软件触发
     DMA_SetTriggerSrc(DMA_UNIT, DMA_CH, EVT_AOS_STRG);
     DMA_SetTriggerSrc(DMA_UNIT, DMA_CH, EVT_USART3_TI);//传输完成中断触发源
//     M4_AOS->DMA1_TRGSEL0_f.COMTRG_EN=3;//使能公共触发源
     DMA_EnableIrq(DMA_UNIT,DMA_CH,TrnCpltIrq);//开启传输完成中断
     DMA_DisableIrq(DMA_UNIT,DMA_CH,BlkTrnCpltIrq);//关闭块传输中断
		
		
    /* Set DMA block transfer complete IRQ */
    stcIrqRegiCfg.enIRQn = DMA_BTC_INT_IRQn;
    stcIrqRegiCfg.pfnCallback = &DmaBtcIrqCallback;
    stcIrqRegiCfg.enIntSrc = INT_DMA1_TC0;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);
}

/**
 *******************************************************************************
 ** \brief DMA block transfer complete irq callback function.
 **
 ** \param [in] None
 **
 ** \retval None
 **
 ******************************************************************************/
static void DmaBtcIrqCallback(void)
{
    DMA_ClearIrqFlag(DMA_UNIT, DMA_CH, TrnCpltIrq);
	// DMA_ClearIrqFlag(DMA_UNIT, DMA_CH, BlkTrnCpltIrq);
	  DMA_ChannelCmd(M4_DMA1, DmaCh0, Disable);         //??DMA
	
}

/**
 *******************************************************************************
 ** \brief USART RX error irq callback function.
 **
 ** \param [in] None
 **
 ** \retval None
 **
 ******************************************************************************/
static void UsartErrIrqCallback(void)
{
    if (Set == USART_GetStatus(USART_CH, UsartFrameErr))
    {
        USART_ClearStatus(USART_CH, UsartFrameErr);
    }

    if (Set == USART_GetStatus(USART_CH, UsartParityErr))
    {
        USART_ClearStatus(USART_CH, UsartParityErr);
    }

    if (Set == USART_GetStatus(USART_CH, UsartOverrunErr))
    {
        USART_ClearStatus(USART_CH, UsartOverrunErr);
    }
}

 static void UsartRxIrqCallback(void)
{
	
    buf[rx_cnt++] = USART_RecData(USART_CH);
	  if(rx_cnt>=10)//uart 0
		{
			  DMA_ClearIrqFlag(DMA_UNIT, DMA_CH, TrnCpltIrq);
				DMA_SetSrcAddress (M4_DMA1, DmaCh0, (uint32_t)(&buf[1]));
        DMA_ChannelCmd(DMA_UNIT, DMA_CH, Enable);
			  DMA_SetTransferCnt(M4_DMA1, DmaCh0, 9);
        USART_SendData(USART_CH, buf[0]);
			  rx_cnt=0;
		}
//		  if(rx_cnt>=10)//uart 0
//		{
//			  DMA_ClearIrqFlag(DMA_UNIT, DMA_CH, TrnCpltIrq);
//				DMA_SetSrcAddress (M4_DMA1, DmaCh0, (uint32_t)(buf));
//        DMA_ChannelCmd(DMA_UNIT, DMA_CH, Enable);
//			  DMA_SetTransferCnt(M4_DMA1, DmaCh0, 10);
////        USART_SendData(USART_CH, buf[0]);
//			  AOS_SW_Trigger();
//		 // 	DMA_SetTriggerSrc(DMA_UNIT, DMA_CH, EVT_USART3_TI);
//			  rx_cnt=0;
//		}
   
}
/**
 *******************************************************************************
 ** \brief  Main function of project
 **
 ** \param  None
 **
 ** \retval int32_t return value, if needed
 **
 ******************************************************************************/


int32_t main(void)
{
    en_result_t enRet = Ok;
    stc_irq_regi_conf_t stcIrqRegiCfg;
    uint32_t u32Fcg1Periph = PWC_FCG1_PERIPH_USART1 | PWC_FCG1_PERIPH_USART2 | \
                             PWC_FCG1_PERIPH_USART3 | PWC_FCG1_PERIPH_USART4;
    const stc_usart_uart_init_t stcInitCfg = {
        UsartIntClkCkNoOutput,
        UsartClkDiv_64,
        UsartDataBits8,
        UsartDataLsbFirst,
        UsartOneStopBit,
        UsartParityNone,
        UsartSamleBit8,
        UsartStartBitFallEdge,
        UsartRtsEnable,
    };

    /* Initialize Clock */
    ClkInit();

    /* Initialize DMA */
    DmaInit();

    /* Enable peripheral clock */
    PWC_Fcg1PeriphClockCmd(u32Fcg1Periph, Enable);

    /* Initialize USART IO */
    PORT_SetFunc(USART_RX_PORT, USART_RX_PIN, USART_RX_FUNC, Disable);
    PORT_SetFunc(USART_TX_PORT, USART_TX_PIN, USART_TX_FUNC, Disable);

    /* Initialize USART */
    enRet = USART_UART_Init(USART_CH, &stcInitCfg);
    if (enRet != Ok)
    {
        while (1)
        {
        }
    }
    else
    {
    }

    /* Set baudrate */
    enRet = USART_SetBaudrate(USART_CH, 1200);
    if (enRet != Ok)
    {
        while (1)
        {
        }
    }
    else
    {
    }

		
    /* Set USART RX IRQ */
    stcIrqRegiCfg.enIRQn = Int000_IRQn;
    stcIrqRegiCfg.pfnCallback = &UsartRxIrqCallback;
    stcIrqRegiCfg.enIntSrc = INT_USART3_RI;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);
    /* Set USART RX error IRQ */
    stcIrqRegiCfg.enIRQn = USART_EI_IRQn;
    stcIrqRegiCfg.pfnCallback = &UsartErrIrqCallback;
    stcIrqRegiCfg.enIntSrc = USART_EI_NUM;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);

    /*Enable TX && RX && RX interrupt function*/
    USART_FuncCmd(USART_CH, UsartTx, Enable);
    USART_FuncCmd(USART_CH, UsartRx, Enable);
    USART_FuncCmd(USART_CH, UsartRxInt, Enable);


    while (1)
    {
			
         
    }
}

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
