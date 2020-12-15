/**
  ******************************************************************************
  * @file    system_stm32f30x.c
  * @author  MCD Application Team
  * @version V1.2.2
  * @date    27-February-2015
  * @brief   CMSIS Cortex-M4 Device Peripheral Access Layer System Source File.
  *          This file contains the system clock configuration for STM32F30x devices,
  *          and is generated by the clock configuration tool
  *          stm32f30x_Clock_Configuration_V1.0.0.xls
  *             
  * 1.  This file provides two functions and one global variable to be called from 
  *     user application:
  *      - SystemInit(): Setups the system clock (System clock source, PLL Multiplier
  *                      and Divider factors, AHB/APBx prescalers and Flash settings),
  *                      depending on the configuration made in the clock xls tool. 
  *                      This function is called at startup just after reset and 
  *                      before branch to main program. This call is made inside
  *                      the "startup_stm32f30x.s" file.
  *
  *      - SystemCoreClock variable: Contains the core clock (HCLK), it can be used
  *                                  by the user application to setup the SysTick 
  *                                  timer or configure other parameters.
  *                                     
  *      - SystemCoreClockUpdate(): Updates the variable SystemCoreClock and must
  *                                 be called whenever the core clock is changed
  *                                 during program execution.
  *
  * 2. After each device reset the HSI (8 MHz) is used as system clock source.
  *    Then SystemInit() function is called, in "startup_stm32f30x.s" file, to
  *    configure the system clock before to branch to main program.
  *
  * 3. If the system clock source selected by user fails to startup, the SystemInit()
  *    function will do nothing and HSI still used as system clock source. User can 
  *    add some code to deal with this issue inside the SetSysClock() function.
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */
/** @addtogroup CMSIS
  * @{
  */

/** @addtogroup stm32f30x_system
  * @{
  */  
  
/** @addtogroup STM32F30x_System_Private_Includes
  * @{
  */

#include "stm32f30x.h"
#include "stm32f30x_cmsis_cfg.h"

/**
  * @}
  */

/** @addtogroup STM32F30x_System_Private_TypesDefinitions
  * @{
  */

/**
  * @}
  */

/** @addtogroup STM32F30x_System_Private_Defines
  * @{
  */
/*!< Uncomment the following line if you need to relocate your vector Table in
     Internal SRAM. */ 
/* #define VECT_TAB_SRAM */
#define VECT_TAB_OFFSET  0x0 /*!< Vector Table base offset field. 
                                  This value must be a multiple of 0x200. */  

#if STM32F30X_CMSIS_CLKSRC == STM32F30X_CMSIS_CLKSRC_HSI
#endif
#if STM32F30X_CMSIS_CLKSRC == STM32F30X_CMSIS_CLKSRC_HSE
#  define SYSCLOCK_SETUP_HSE
#  define SYSCLOCK_CLKSRC  RCC_CFGR_SW_HSE
#endif
#if STM32F30X_CMSIS_CLKSRC == STM32F30X_CMSIS_CLKSRC_PLLCLKBYHSI
#  define SYSCLOCK_SETUP_PLL
#  define SYSCLOCK_PLL_INPUT  RCC_CFGR_PLLSRC_HSI_Div2
#  define SYSCLOCK_CLKSRC  RCC_CFGR_SW_PLL
#endif
#if STM32F30X_CMSIS_CLKSRC == STM32F30X_CMSIS_CLKSRC_PLLCLKBYHSE
#  define SYSCLOCK_SETUP_HSE
#  define SYSCLOCK_SETUP_PLL
#  define SYSCLOCK_PLL_INPUT  RCC_CFGR_PLLSRC_PREDIV1
#  define SYSCLOCK_CLKSRC  RCC_CFGR_SW_PLL
#endif

#if STM32F30X_CMSIS_CLOCKHZ <= 24000000
#define SYSCLOCK_FLASHLATENCY 0
#elif STM32F30X_CMSIS_CLOCKHZ <= 48000000
#define SYSCLOCK_FLASHLATENCY 1
#else
#define SYSCLOCK_FLASHLATENCY 2
#endif

/**
  * @}
  */ 

/** @addtogroup STM32F30x_System_Private_Macros
  * @{
  */

/**
  * @}
  */

/** @addtogroup STM32F30x_System_Private_Variables
  * @{
  */

  uint32_t SystemCoreClock = STM32F30X_CMSIS_CLOCKHZ;

  __I uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};

/**
  * @}
  */

/** @addtogroup STM32F30x_System_Private_FunctionPrototypes
  * @{
  */

static void SetSysClock(void);

/**
  * @}
  */

/** @addtogroup STM32F30x_System_Private_Functions
  * @{
  */

/**
  * @brief  Setup the microcontroller system
  *         Initialize the Embedded Flash Interface, the PLL and update the 
  *         SystemFrequency variable.
  * @param  None
  * @retval None
  */
void SystemInit(void)
{
  /* FPU settings ------------------------------------------------------------*/
  #if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));  /* set CP10 and CP11 Full Access */
  #endif

  /* Reset the RCC clock configuration to the default reset state ------------*/
  /* Set HSION bit */
  RCC->CR |= (uint32_t)0x00000001;

  /* Reset CFGR register */
  RCC->CFGR &= 0xF87FC00C;

  /* Reset HSEON, CSSON and PLLON bits */
  RCC->CR &= (uint32_t)0xFEF6FFFF;

  /* Reset HSEBYP bit */
  RCC->CR &= (uint32_t)0xFFFBFFFF;

  /* Reset PLLSRC, PLLXTPRE, PLLMUL and USBPRE bits */
  RCC->CFGR &= (uint32_t)0xFF80FFFF;

  /* Reset PREDIV1[3:0] bits */
  RCC->CFGR2 &= (uint32_t)0xFFFFFFF0;

  /* Reset USARTSW[1:0], I2CSW and TIMs bits */
  RCC->CFGR3 &= (uint32_t)0xFF00FCCC;
  
  /* Disable all interrupts */
  RCC->CIR = 0x00000000;

  /* Configure the System clock source, PLL Multiplier and Divider factors, 
     AHB/APBx prescalers and Flash settings ----------------------------------*/
  SetSysClock();
  
#ifdef VECT_TAB_SRAM
  SCB->VTOR = SRAM_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal SRAM. */
#else
  SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal FLASH. */
#endif  
}

/**
   * @brief  Update SystemCoreClock variable according to Clock Register Values.
  *         The SystemCoreClock variable contains the core clock (HCLK), it can
  *         be used by the user application to setup the SysTick timer or configure
  *         other parameters.
  *           
  * @note   Each time the core clock (HCLK) changes, this function must be called
  *         to update SystemCoreClock variable value. Otherwise, any configuration
  *         based on this variable will be incorrect.         
  *     
  * @note   - The system frequency computed by this function is not the real 
  *           frequency in the chip. It is calculated based on the predefined 
  *           constant and the selected clock source:
  *             
  *           - If SYSCLK source is HSI, SystemCoreClock will contain the HSI_VALUE(*)
  *                                              
  *           - If SYSCLK source is HSE, SystemCoreClock will contain the HSE_VALUE(**)
  *                          
  *           - If SYSCLK source is PLL, SystemCoreClock will contain the HSE_VALUE(**) 
  *             or HSI_VALUE(*) multiplied/divided by the PLL factors.
  *         
  *         (*) HSI_VALUE is a constant defined in stm32f30x.h file (default value
  *             8 MHz) but the real value may vary depending on the variations
  *             in voltage and temperature.   
  *    
  *         (**) HSE_VALUE is a constant defined in stm32f30x.h file (default value
  *              8 MHz), user has to ensure that HSE_VALUE is same as the real
  *              frequency of the crystal used. Otherwise, this function may
  *              have wrong result.
  *                
  *         - The result of this function could be not correct when using fractional
  *           value for HSE crystal.
  *     
  * @param  None
  * @retval None
  */
void SystemCoreClockUpdate (void)
{
  uint32_t tmp = 0, pllmull = 0, pllsource = 0, prediv1factor = 0;

  /* Get SYSCLK source -------------------------------------------------------*/
  tmp = RCC->CFGR & RCC_CFGR_SWS;
  
  switch (tmp)
  {
    case 0x00:  /* HSI used as system clock */
      SystemCoreClock = HSI_VALUE;
      break;
    case 0x04:  /* HSE used as system clock */
      SystemCoreClock = HSE_VALUE;
      break;
    case 0x08:  /* PLL used as system clock */
      /* Get PLL clock source and multiplication factor ----------------------*/
      pllmull = RCC->CFGR & RCC_CFGR_PLLMULL;
      pllsource = RCC->CFGR & RCC_CFGR_PLLSRC;
      pllmull = ( pllmull >> 18) + 2;
      
      if (pllsource == 0x00)
      {
        /* HSI oscillator clock divided by 2 selected as PLL clock entry */
        SystemCoreClock = (HSI_VALUE >> 1) * pllmull;
      }
      else
      {
        prediv1factor = (RCC->CFGR2 & RCC_CFGR2_PREDIV1) + 1;
        /* HSE oscillator clock selected as PREDIV1 clock entry */
        SystemCoreClock = (HSE_VALUE / prediv1factor) * pllmull; 
      }      
      break;
    default: /* HSI used as system clock */
      SystemCoreClock = HSI_VALUE;
      break;
  }
  /* Compute HCLK clock frequency ----------------*/
  /* Get HCLK prescaler */
  tmp = AHBPrescTable[((RCC->CFGR & RCC_CFGR_HPRE) >> 4)];
  /* HCLK clock frequency */
  SystemCoreClock >>= tmp;  
}

/**
  * @brief  Configures the System clock source, PLL Multiplier and Divider factors,
  *               AHB/APBx prescalers and Flash settings
  * @note   This function should be called only once the RCC clock configuration  
  *         is reset to the default reset state (done in SystemInit() function).             
  * @param  None
  * @retval None
  */
static void SetSysClock(void)
{
#ifdef SYSCLOCK_SETUP_HSE
  __IO uint32_t StartUpCounter = 0, HSEStatus = 0;
#endif

  /* Configure MCO clock outputs */
  RCC->CFGR |= (STM32F30X_CMSIS_MCOSRC << 24);

  /* At this stage the HSI is enabled and used as System clock source */

#ifdef SYSCLOCK_SETUP_HSE
// use pincfg document setting or backup from core plugin
# if defined(PINCFG_RCC_OSC_HSE_EXTERNAL_CLOCK_SOURCE) || (defined(STM32F30X_CMSIS_HSE_BYPASS) && STM32F30X_CMSIS_HSE_BYPASS)
  /* Bypass HSE oscillator */
  RCC->CR |= RCC_CR_HSEBYP;
# endif

  /* Enable HSE */
  RCC->CR |= RCC_CR_HSEON;
 
  /* Wait till HSE is ready and if Time out is reached exit */
  do
  {
  } while (((RCC->CR & RCC_CR_HSERDY) == RESET) && (++StartUpCounter != HSE_STARTUP_TIMEOUT));

  if ((RCC->CR & RCC_CR_HSERDY) == RESET)
  {
      /* If HSE fails to start-up, the application will have wrong clock
         configuration. User can add here some code to deal with this error */
  }
  else
#endif
  {
    /* HCLK */
    RCC->CFGR |= (STM32F30X_CMSIS_AHB << 4);
       
    /* PCLK2 */
    RCC->CFGR |= (STM32F30X_CMSIS_APB2 << 11);
     
    /* PCLK1 */
    RCC->CFGR |= (STM32F30X_CMSIS_APB1 << 8);

#ifdef SYSCLOCK_SETUP_PLL
    /* PLL configuration */
    RCC->CFGR |= (SYSCLOCK_PLL_INPUT | ((STM32F30X_CMSIS_PLL_MUL - 2) << 18));
# ifdef SYSCLOCK_SETUP_HSE
    RCC->CFGR2 |= (STM32F30X_CMSIS_HSE_PREDIV - 1);
# endif

    /* Enable the main PLL */
    RCC->CR |= RCC_CR_PLLON;

    /* Wait till the main PLL is ready */
    while((RCC->CR & RCC_CR_PLLRDY) == 0)
    {
    }
#endif

    /* Configure Flash prefetch, Instruction cache, Data cache and wait state */
    do
    {
      FLASH->ACR = FLASH_ACR_PRFTBE | SYSCLOCK_FLASHLATENCY;
    	/* Check that the new number of wait states is taken into account to access */
    	/* the Flash memory by reading the FLASH_ACR register */
    }
    while ((FLASH->ACR & FLASH_ACR_LATENCY) != SYSCLOCK_FLASHLATENCY);
    
#ifdef SYSCLOCK_CLKSRC
    /* Select the given clock as system clock source */
    RCC->CFGR |= SYSCLOCK_CLKSRC;

    /* Wait till the given clock is used as system clock source */
    while ((RCC->CFGR & RCC_CFGR_SWS ) != (SYSCLOCK_CLKSRC << 2));
    {
    }
#endif
  }

#ifdef STM32F30X_CMSIS_USB_DIV
  /* Configure USB clock */
  RCC->CFGR |= (STM32F30X_CMSIS_USB_DIV << 22);
#endif
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
