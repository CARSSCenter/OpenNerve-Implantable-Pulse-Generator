/**
 * @file app_mode_bsl.c
 * @brief This file provides management of the BSL mode
 * @copyright Copyright (c) 2024
 */
#include "app_mode_bsl.h"
#include "app_config.h"

#define LEN_FLASH_SIZE	16U									/*!< The length of one write to flash memory */

/**
 * @brief Handler for BSL mode
 * 
 */
void app_mode_bsl_handler(void) {
	FLASH_OBProgramInitTypeDef OBInit;
    HAL_FLASHEx_OBGetConfig(&OBInit);

	FLASH_EraseInitTypeDef EraseInitStruct = {
			.TypeErase = FLASH_TYPEERASE_PAGES,
			.Page = 0,
			.NbPages = FLASH_PAGE_NB, /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */ /* parasoft-suppress MISRAC2012-RULE_10_4-b "This definition comes from HAL." */
	};

	if ((OBInit.USERConfig & OB_SWAP_BANK_ENABLE) == OB_SWAP_BANK_DISABLE) {
		EraseInitStruct.Banks = FLASH_BANK_2;
		OBInit.USERConfig = OB_SWAP_BANK_ENABLE;
	}
	else {
		EraseInitStruct.Banks = FLASH_BANK_1;
		OBInit.USERConfig = OB_SWAP_BANK_DISABLE;
	}

	OBInit.OptionType = OPTIONBYTE_USER;
	OBInit.USERType = OB_USER_SWAP_BANK;

	FW_Image_Info_t image_info;
	uint32_t Offset = 0;
	static uint8_t FlashData[LEN_FLASH_SIZE];
	uint32_t PageError = 0;
	uint8_t fail_times = 0;

	while(fail_times < 3U) {
		bsp_wdg_refresh();
		HAL_ERROR_CHECK(HAL_FLASH_Unlock());

		bsp_fram_read((uint32_t)ADDR_FW_IMG_INFO, (uint8_t*)&image_info, (uint16_t)sizeof(image_info));

		PageError = 0;
		HAL_ERROR_CHECK(HAL_FLASHEx_Erase(&EraseInitStruct, &PageError));

		uint32_t DataAddress = (uint32_t)&FlashData[0]; /* parasoft-suppress MISRAC2012-RULE_11_4-a "Confirmed compliance with this rule." */
		uint32_t bank2_base = BANK2_ADDR;

		Offset = 0;
		while(Offset < image_info.ImageSize) {
			bsp_wdg_refresh();
			bsp_fram_read(ADDR_FW_IMG_BASE + Offset, FlashData, LEN_FLASH_SIZE);
			HAL_ERROR_CHECK(HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, bank2_base + Offset, DataAddress));
			Offset += LEN_FLASH_SIZE;
		}
		HAL_ERROR_CHECK(HAL_ICACHE_Invalidate());
		HAL_ERROR_CHECK(HAL_FLASH_Lock());
		bsp_wdg_refresh();
		if (app_func_auth_compare_flash_hash(image_info.ImageSize, image_info.ImageHash)) {
			HAL_ERROR_CHECK(HAL_FLASH_Unlock());
			HAL_ERROR_CHECK(HAL_ICACHE_Invalidate());
			HAL_ERROR_CHECK(HAL_FLASH_OB_Unlock());
			HAL_ERROR_CHECK(HAL_FLASHEx_OBProgram(&OBInit));
			HAL_ERROR_CHECK(HAL_FLASH_OB_Launch());
			//Option byte launch generates Option byte reset
		}
		else {
			fail_times++;
		}
	}
	HAL_ERROR_CHECK(HAL_FLASH_Lock());

	app_func_sm_current_state_set(STATE_ACT);
}
