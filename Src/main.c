/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ssd1306.h"
#include "ssd1306_tests.h"
#include "ssd1306_fonts.h"
#include "ssd1306_conf.h"
#include <stdio.h>
#include <string.h>
#include "FLASH_SECTOR_F4.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
	SCREEN_START,
	SCREEN_REP,
	SCREEN_DEBUG,
	SCREEN_HIGHSCORE,
	SCREEN_GAMEOVER
} Screen;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
static int start_pressed = 0; // boolean
static int highscore_pressed = 0; // boolean
static int calibrate_down_pressed = 0; // boolean
static int calibrate_up_pressed = 0; // boolean
static int debug_pressed = 0; // boolean
static int canPushUp = 0; // boolean


// drawStart instance variables
static int xStart = 128;
static int speed = 2;

static Screen current_screen = SCREEN_START;
static int pushup_count = 0;
static uint32_t hiScore; // read by flash drive on system initialization
static int isGameOver = 0; // boolean
static int isHighScore = 0; // boolean
static const int startTime = 60;
static int countdown = startTime;
static int calibrate_up = 0;
static int calibrate_down = 0;

// Utrasonic sensor variables
static uint32_t pMillis;
static uint16_t distance  = 0;  // cm

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM1_Init(void);
/* USER CODE BEGIN PFP */
static void drawReps(void);
static void drawStart(void);
static void drawHighScore(void);
static void drawGameOver(void);
static void drawTimer(void);
static void drawDistance(void);
static void drawCalibrations(void);
static int readStartButton(void);
static int readHighScoreButton(void);
static int readCalibrateDownButton(void);
static int readCalibrateUpButton(void);
static int readDebugButton(void);
void readUltraSonic(void);
static void buzz(int time);
static void checkForPushUp(void);
static void reset(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
  ssd1306_Init();
  Flash_Read_Data(0x08060000, &hiScore, 5);
  HAL_TIM_Base_Start(&htim1);
  HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);  // pull the TRIG pin low initially
  buzz(15000);
  isGameOver = 0;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  switch (current_screen) {
		  case SCREEN_START:
			  readCalibrateUpButton();
			  readCalibrateDownButton();
			  readDebugButton();
			  drawStart();
			  if(readStartButton() == 1)
				  current_screen = SCREEN_REP;
			  else if(readHighScoreButton() == 1)
				  current_screen = SCREEN_HIGHSCORE;
			  else if (readDebugButton() == 1)
				  current_screen = SCREEN_DEBUG;
			  break;
		  case SCREEN_REP:
			  readUltraSonic();
			  checkForPushUp();
			  drawTimer();
			  drawReps();
			  if (countdown == 0)
				  current_screen = SCREEN_GAMEOVER;
			  else if (readStartButton() == 1) {
				  current_screen = SCREEN_START;
				  reset();
			  }
			  else if (readDebugButton() == 1)
				  current_screen = SCREEN_DEBUG;
			  break;
		  case SCREEN_DEBUG:
			  readCalibrateUpButton();
			  readCalibrateDownButton();
			  readUltraSonic();
			  drawDistance();
			  drawCalibrations();
			  if (readDebugButton() == 1)
				  current_screen = SCREEN_START;
			  break;
		  case SCREEN_HIGHSCORE:
			  drawHighScore();
			  if (readStartButton() == 1) {
				  current_screen = SCREEN_START;
				  reset();
			  }
			  break;
		  case SCREEN_GAMEOVER:
			  drawGameOver();
			  if (readStartButton() == 1) {
				  current_screen = SCREEN_START;
				  reset();
			  }
			  break;
	  }
  }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 180;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 180-1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 10000-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 9000-1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GREEN_LED_Pin|BUZZER_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LD2_Pin|TRIG_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : blue_button_Pin */
  GPIO_InitStruct.Pin = blue_button_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(blue_button_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : GREEN_LED_Pin BUZZER_Pin */
  GPIO_InitStruct.Pin = GREEN_LED_Pin|BUZZER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : START_BUTTON_Pin HIGHSCORE_BUTTON_Pin */
  GPIO_InitStruct.Pin = START_BUTTON_Pin|HIGHSCORE_BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : CALIBRATE_UP_BUTTON_Pin ECHO_Pin */
  GPIO_InitStruct.Pin = CALIBRATE_UP_BUTTON_Pin|ECHO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LD2_Pin TRIG_Pin */
  GPIO_InitStruct.Pin = LD2_Pin|TRIG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : CALIBRATE_DOWN_BUTTON_Pin */
  GPIO_InitStruct.Pin = CALIBRATE_DOWN_BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(CALIBRATE_DOWN_BUTTON_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/**
 * @brief Performs necessary resets to start a new workout
 * @retval none
 */
static void reset(void) {
	current_screen = SCREEN_START;
	isGameOver = 0;
	isHighScore = 0;
	pushup_count = 0;
	countdown = startTime;
	canPushUp = 0;
	HAL_TIM_Base_Stop_IT(&htim2);
	HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_RESET);
}
/**
 * @brief Draws a countdown timer on the SSD1306 OLED
 * @retval none
 */
static void drawTimer(void) {
	// Clear screen
	ssd1306_Fill(Black);
	// Format time and write on screen
	int minutes = countdown / 60;
	int seconds = countdown - minutes*60;
	char str[10];
	if (seconds < 0)
		snprintf(str, 10, "0:00");
	else if (seconds < 10)
		snprintf(str, 10, "%d:0%d", minutes, seconds);
	else
		snprintf(str, 10, "%d:%d", minutes, seconds);
	ssd1306_SetCursor(0, 0);
	ssd1306_WriteString(str, Font_11x18, White);
}
/**
 * @brief Draws a scrolling start text on SSD1306 OLED
 * @retval none
 */
static void drawStart(void) {
	// Clear screen
	ssd1306_Fill(Black);
	// Moves x-position to the left by set speed
	xStart -= speed;
	// Draw text
	ssd1306_SetCursor(xStart, 19);
	ssd1306_WriteString("Press start!", Font_16x26, White);
	ssd1306_UpdateScreen();
}
/**
 * @brief Draws number of reps performed in middle of SSD1306 OLED
 * @retval none
 */
static void drawReps(void) {
	// Convert the push-up count to a writable string
	char str[10];
	snprintf(str, 10, "%d", pushup_count);
	int text_width = strlen(str) * 16;
	// Draw push-up count to the right center of screen
	ssd1306_SetCursor(80-text_width/2, 19);
	ssd1306_WriteString(str, Font_16x26, White);
	ssd1306_UpdateScreen();
}
/**
 * @brief Draws saved highscore in middle of SSD1306 OLED
 * @retval none
 */
static void drawHighScore(void) {
	ssd1306_Fill(Black);
	ssd1306_SetCursor(0, 0);
	ssd1306_WriteString("Current high score", Font_7x10, White);
	ssd1306_SetCursor(0, 27);
	char str[9];
	snprintf(str, 9, "%u reps", hiScore);
	ssd1306_WriteString(str, Font_16x26, White);
	ssd1306_UpdateScreen();
}
/**
 * @brief Draws final score and makes final beeps on SSD1306 OLED
 * @retval none
 */
static void drawGameOver(void) {
	if (isGameOver == 0) {
		for (int i = 0; i < 10; i++) {
			buzz(15000);
			HAL_Delay(50);
		}
		// check for high score
		if (pushup_count > hiScore) {
			hiScore = pushup_count;
			Flash_Write_Data(0x08060000, &hiScore, 5);
		}
		isGameOver = 1;
	}
	ssd1306_Fill(Black);
	ssd1306_SetCursor(0, 0);
	ssd1306_WriteString("Your score", Font_7x10, White);
	ssd1306_SetCursor(0, 27);
	char str[9];
	snprintf(str, 9, "%d reps", pushup_count);
	ssd1306_WriteString(str, Font_16x26, White);
	ssd1306_UpdateScreen();
}
/**
 * @brief Draws ultrasonic sensor read distance on SSD1306 OLED
 * @retval none
 */
static void drawDistance(void) {
	ssd1306_Fill(Black);
	ssd1306_SetCursor(0, 0);
	char str[30];
	snprintf(str, 30, "Distance: %d", distance);
	ssd1306_WriteString(str, Font_7x10, White);
}
/**
 * @brief Draws saved pushup calibrations on SSD1306 OLED
 * @retval none
 */
static void drawCalibrations(void) {
	ssd1306_SetCursor(0, 12);
	char str[30];
	snprintf(str, 30, "Up pos: %d", calibrate_up);
	ssd1306_WriteString(str, Font_7x10, White);
	ssd1306_SetCursor(0, 24);
	snprintf(str, 30, "Down pos: %d", calibrate_down);
	ssd1306_WriteString(str, Font_7x10, White);
	ssd1306_UpdateScreen();
}
/**
 * @brief Checks if highscore button is pushed
 * @retval 1 if pressed, 0 if not pressed
 */
static int readHighScoreButton(void) {
	// increments count when button is pressed (LOW -> HIGH ---- "Rising edge case")
	if (HAL_GPIO_ReadPin(HIGHSCORE_BUTTON_GPIO_Port, HIGHSCORE_BUTTON_Pin) == GPIO_PIN_SET && highscore_pressed == 0) {
		highscore_pressed = 1;
		return 1;
	}
	else if (HAL_GPIO_ReadPin(HIGHSCORE_BUTTON_GPIO_Port, HIGHSCORE_BUTTON_Pin) == GPIO_PIN_RESET && highscore_pressed == 1)
		highscore_pressed = 0;
	return 0;
}
/**
 * @brief Checks if start button is pushed
 * @retval 1 if pressed, 0 if not pressed
 */
static int readStartButton(void) {
	// starts timer when button is pressed (LOW -> HIGH ---- "Rising edge case")
	if (HAL_GPIO_ReadPin(START_BUTTON_GPIO_Port, START_BUTTON_Pin) == GPIO_PIN_RESET && start_pressed == 0) {
		start_pressed = 1;
		HAL_TIM_Base_Start_IT(&htim2);
		return 1;
	}
	else if (HAL_GPIO_ReadPin(START_BUTTON_GPIO_Port, START_BUTTON_Pin) == GPIO_PIN_SET && start_pressed == 1) {
		start_pressed = 0;
	}
	return 0;
}
/**
 * @brief Checks if debug button is pushed
 * @retval 1 if pressed, 0 if not pressed
 */
static int readDebugButton(void) {
	if (HAL_GPIO_ReadPin(blue_button_GPIO_Port, blue_button_Pin) == GPIO_PIN_SET && debug_pressed == 0) {
		debug_pressed = 1;
		return 1;
	}
	else if (HAL_GPIO_ReadPin(blue_button_GPIO_Port, blue_button_Pin) == GPIO_PIN_RESET && debug_pressed == 1) {
		debug_pressed = 0;
	}
	return 0;
}
/**
 * @brief Checks if calibrate down button is pushed
 * @retval 1 if pressed, 0 if not pressed
 */
static int readCalibrateDownButton(void) {
	if (HAL_GPIO_ReadPin(CALIBRATE_DOWN_BUTTON_GPIO_Port, CALIBRATE_DOWN_BUTTON_Pin) == GPIO_PIN_RESET && calibrate_down_pressed == 0) {
		readUltraSonic();
		calibrate_down_pressed = 1;
		calibrate_down = distance;
		buzz(15000);
		return 1;
	}
	else if (HAL_GPIO_ReadPin(CALIBRATE_DOWN_BUTTON_GPIO_Port, CALIBRATE_DOWN_BUTTON_Pin) == GPIO_PIN_SET && calibrate_down_pressed == 1) {
		calibrate_down_pressed = 0;
	}
	return 0;
}
/**
 * @brief Checks if calibrate up button is pushed
 * @retval 1 if pressed, 0 if not pressed
 */
static int readCalibrateUpButton(void) {
	if (HAL_GPIO_ReadPin(CALIBRATE_UP_BUTTON_GPIO_Port, CALIBRATE_UP_BUTTON_Pin) == GPIO_PIN_RESET && calibrate_up_pressed == 0) {
		readUltraSonic();
		calibrate_up_pressed = 1;
		calibrate_up = distance;
		buzz(15000);
		return 1;
	}
	else if (HAL_GPIO_ReadPin(CALIBRATE_UP_BUTTON_GPIO_Port, CALIBRATE_UP_BUTTON_Pin) == GPIO_PIN_SET && calibrate_up_pressed == 1) {
		calibrate_up_pressed = 0;
	}
	return 0;
}

/**
 * @brief Records the time it takes for sound waves to bounce back from ultrasonic sensor.
 * @retval none
 */
void readUltraSonic(void) {
	uint32_t Value1 = 0;
	uint32_t Value2 = 0;
	// Sends pulse to the ultrasonic sensor ("Hey ultrasonic...can you record distance please?")
	HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_SET);  // pull the TRIG pin HIGH
	__HAL_TIM_SET_COUNTER(&htim1, 0);
	while (__HAL_TIM_GET_COUNTER (&htim1) < 10);  // wait for 10 us
	HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);  // pull the TRIG pin low

	pMillis = HAL_GetTick(); // used this to avoid infinite while loop  (for timeout)
	// wait for the echo pin to go high
	while (!(HAL_GPIO_ReadPin (ECHO_GPIO_Port, ECHO_Pin)) && pMillis + 10 >  HAL_GetTick());
		Value1 = __HAL_TIM_GET_COUNTER (&htim1);

	pMillis = HAL_GetTick(); // used this to avoid infinite while loop (for timeout)
	// wait for the echo pin to go low
	while ((HAL_GPIO_ReadPin (ECHO_GPIO_Port, ECHO_Pin)) && pMillis + 50 > HAL_GetTick());
		Value2 = __HAL_TIM_GET_COUNTER (&htim1);

	distance = (Value2-Value1)* 0.034/2; // distance = time * speed of sound / 2 (divided by 2 because round trip)
	HAL_Delay(50);
}
/**
 * @brief Makes piezo buzzer do a small beep
 * @retval none
 */
static void buzz(int time) {
	HAL_GPIO_TogglePin(BUZZER_GPIO_Port, BUZZER_Pin);
	__HAL_TIM_SET_COUNTER(&htim1, 0);
	while (__HAL_TIM_GET_COUNTER (&htim1) < time);
	HAL_GPIO_TogglePin(BUZZER_GPIO_Port, BUZZER_Pin);
}
/**
 * @brief Counts a pushup when a full pushup is performed, and lights the LED when in down position
 */
static void checkForPushUp(void) {
	if (distance < calibrate_down + 5 && canPushUp == 0) {
		HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_SET);
		canPushUp = 1;
	}
	else if (distance < calibrate_up + 20 && distance > calibrate_up - 5 && canPushUp == 1) {
		HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_RESET);
		pushup_count++;
		buzz(15000);
		canPushUp = 0;
	}
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM2)
		countdown--;
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
