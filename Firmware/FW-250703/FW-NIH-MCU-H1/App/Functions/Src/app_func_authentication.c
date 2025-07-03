/**
 * @file app_func_authentication.c
 * @brief This file provides authentication functionality
 * @copyright Copyright (c) 2024
 */
#include "app_func_authentication.h"
#include "app_config.h"

static PKA_ECDSAVerifInTypeDef verifIn = {
		.coef 				= prime256v1_absA,
		.modulus 			= prime256v1_Prime,
		.basePointX 		= prime256v1_GeneratorX,
		.basePointY 		= prime256v1_GeneratorY,
		.primeOrder 		= prime256v1_Order,
};

static const ECC_PublicKey_t PublicKey_Admin = {
		.PublicKeyQx = APP_ECC_PUBKEY_QX_ADMIN,
		.PublicKeyQy = APP_ECC_PUBKEY_QY_ADMIN
};

static ECDSA_Data_t fw_image_ecdsa_data;
static uint8_t hash_verify_fail_num = 0;

/**
 * @brief Verify ECDSA signature is Admin class
 * 
 * @param ecdsa_data The ECDSA signature and hash message to be verified
 * @return true Verification passed
 * @return false Verification failed
 */
bool app_func_auth_verify_sign_admin(ECDSA_Data_t ecdsa_data) {
	bool result = false;
	hash_verify_fail_num = 0;

	verifIn.primeOrderSize 	= prime256v1_Order_len;
	verifIn.modulusSize		= prime256v1_Prime_len;
	verifIn.coefSign 		= prime256v1_A_sign;

	verifIn.RSign 			= ecdsa_data.RSign;
	verifIn.SSign 			= ecdsa_data.SSign;
	verifIn.hash 			= ecdsa_data.HashMsg;

	verifIn.pPubKeyCurvePtX = PublicKey_Admin.PublicKeyQx;
	verifIn.pPubKeyCurvePtY = PublicKey_Admin.PublicKeyQy;

	HAL_ERROR_CHECK(HAL_PKA_ECDSAVerif(&hpka, &verifIn, 5000));
	if (HAL_PKA_ECDSAVerif_IsValidSignature(&hpka) == 1UL) {
		(void)memcpy((uint8_t*)&fw_image_ecdsa_data.RSign, (uint8_t*)&ecdsa_data.RSign, sizeof(ecdsa_data.RSign));
		(void)memcpy((uint8_t*)&fw_image_ecdsa_data.SSign, (uint8_t*)&ecdsa_data.SSign, sizeof(ecdsa_data.SSign));
		(void)memcpy((uint8_t*)&fw_image_ecdsa_data.HashMsg, (uint8_t*)&ecdsa_data.HashMsg, sizeof(ecdsa_data.HashMsg));
		result = true;
	}
	else {
		result = false;
	}
	return result;
}

/**
 * @brief Compare the hash message from FRAM and the ECDSA signature
 * 
 * @param img_size The size of the firmware image to verify
 * @return uint8_t* Pointer of the number of failed verifications, 0 means verification passed. It is reset when verifying the ECDSA signature.
 */
uint8_t* app_func_auth_compare_fram_hash(uint32_t img_size) {
	if (img_size > 0U) {
		FW_Image_Packet_t imagePacket = {
				.ImageDataOffset = 0,
		};

		FW_Image_Info_t image_info = {
				.ImageSize = img_size,
		};
		uint32_t accmlt_size = img_size - 1U;
		uint32_t datasize = SIZE_FW_IMG_PKG;
		while((accmlt_size - imagePacket.ImageDataOffset) > datasize) {
			bsp_wdg_refresh();
			bsp_fram_read((uint32_t)ADDR_FW_IMG_BASE + imagePacket.ImageDataOffset, imagePacket.ImageData, (uint16_t)datasize);
			while(HAL_HASH_GetState(&hhash) != HAL_HASH_STATE_READY) {
				__NOP();
			};
			HAL_ERROR_CHECK(HAL_HASHEx_SHA256_Accmlt(&hhash, imagePacket.ImageData, datasize));
			imagePacket.ImageDataOffset += datasize;
		}
		datasize = img_size - imagePacket.ImageDataOffset;
		bsp_fram_read((uint32_t)ADDR_FW_IMG_BASE + imagePacket.ImageDataOffset, imagePacket.ImageData, (uint16_t)datasize);
		HAL_ERROR_CHECK(HAL_HASHEx_SHA256_Accmlt_End(&hhash, imagePacket.ImageData, datasize, image_info.ImageHash, 10));

		if (memcmp(fw_image_ecdsa_data.HashMsg, image_info.ImageHash, sizeof(fw_image_ecdsa_data.HashMsg)) == 0) {
			hash_verify_fail_num = 0;
		}
		else {
			(void)memset(&image_info, 0, sizeof(image_info));
			hash_verify_fail_num++;
		}
		bsp_fram_write((uint32_t)ADDR_FW_IMG_INFO, (uint8_t*)&image_info, (uint16_t)(sizeof(image_info)), true);
	}
	else {
		hash_verify_fail_num++;
	}
	return &hash_verify_fail_num;
}

/**
 * @brief Compare the hash message from Flash and the image info
 *
 * @param img_size The size of the firmware image to verify
 * @param img_hash The hash of the firmware image to verify
 * @return true Verification passed
 * @return false Verification failed
 */
bool app_func_auth_compare_flash_hash(uint32_t img_size, uint8_t* img_hash) {
	uint8_t 	ImageHash[32] = {0};

	HAL_ERROR_CHECK(HAL_HASHEx_SHA256_Start(&hhash, (uint8_t*)BANK2_ADDR, img_size, ImageHash, 10));
	if (memcmp(ImageHash, img_hash, sizeof(ImageHash)) == 0) {
		return true;
	}
	else {
		return false;
	}
}

/**
 * @brief When establishing a BLE connection, verify and confirm user class.
 * 
 * @param ecdsa_data The ECDSA signature and hash message to be verified
 * @return uint8_t User class
 */
uint8_t app_func_auth_user_class_get(ECDSA_Data_t ecdsa_data) {
	ECC_PublicKey_t publickey;

	verifIn.primeOrderSize 	= prime256v1_Order_len;
	verifIn.modulusSize		= prime256v1_Prime_len;
	verifIn.coefSign 		= prime256v1_A_sign;
	verifIn.RSign 			= ecdsa_data.RSign;
	verifIn.SSign 			= ecdsa_data.SSign;
	verifIn.hash 			= ecdsa_data.HashMsg;

	//USER_CLASS_ADMIN
	(void)memcpy((uint8_t*)&publickey.PublicKeyQx, (const uint8_t*)&PublicKey_Admin.PublicKeyQx, sizeof(PublicKey_Admin.PublicKeyQx));
	(void)memcpy((uint8_t*)&publickey.PublicKeyQy, (const uint8_t*)&PublicKey_Admin.PublicKeyQy, sizeof(PublicKey_Admin.PublicKeyQy));

	verifIn.pPubKeyCurvePtX = publickey.PublicKeyQx;
	verifIn.pPubKeyCurvePtY = publickey.PublicKeyQy;

	if(HAL_PKA_ECDSAVerif(&hpka, &verifIn, 5000) != HAL_OK)	{
		Error_Handler();
	}

	if (HAL_PKA_ECDSAVerif_IsValidSignature(&hpka) == 1UL) {
		return USER_CLASS_ADMIN;
	}

	//USER_CLASS_CLINICIAN
	app_func_para_data_get((const uint8_t*)HPID_LINKED_CRC_KEY, (uint8_t*)&publickey, (uint8_t)sizeof(publickey));

	verifIn.pPubKeyCurvePtX = publickey.PublicKeyQx;
	verifIn.pPubKeyCurvePtY = publickey.PublicKeyQy;

	if(HAL_PKA_ECDSAVerif(&hpka, &verifIn, 5000) != HAL_OK)	{
		Error_Handler();
	}

	if (HAL_PKA_ECDSAVerif_IsValidSignature(&hpka) == 1UL) {
		return USER_CLASS_CLINICIAN;
	}

	//USER_CLASS_PATIENT
	app_func_para_data_get((const uint8_t*)HPID_LINKED_PRC_KEY, (uint8_t*)&publickey, (uint8_t)sizeof(publickey));

	verifIn.pPubKeyCurvePtX = publickey.PublicKeyQx;
	verifIn.pPubKeyCurvePtY = publickey.PublicKeyQy;

	if(HAL_PKA_ECDSAVerif(&hpka, &verifIn, 5000) != HAL_OK)	{
		Error_Handler();
	}

	if (HAL_PKA_ECDSAVerif_IsValidSignature(&hpka) == 1UL) {
		return USER_CLASS_PATIENT;
	}

	return USER_CLASS_INVALID;
}
