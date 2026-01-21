/**
 * @file app_func_authentication.h
 * @brief This file contains all the function prototypes for the app_func_authentication.c file
 * @copyright Copyright (c) 2024
 */
#ifndef FUNCTIONS_INC_APP_AUTHENTICATION_H_
#define FUNCTIONS_INC_APP_AUTHENTICATION_H_
#include <stdint.h>
#include <stdbool.h>

#define USER_CLASS_ADMIN			0xFFU	/*!< The user class of the admin */
#define USER_CLASS_CLINICIAN		0x02U	/*!< The user class of the clinician */
#define USER_CLASS_PATIENT			0x01U	/*!< The user class of the patient */
#define USER_CLASS_INVALID			0x00U	/*!< The invalid user class */

#define	BANK2_ADDR					(FLASH_BASE + FLASH_BANK_SIZE)		/*!< The starting address of BANK2 */

typedef struct {
	uint8_t 		RSign[32];				/*!< ECDSA signature part r */
	uint8_t 		SSign[32];				/*!< ECDSA signature part s */
	uint8_t 		HashMsg[32];			/*!< Hash message */
} ECDSA_Data_t;

/**
 * @brief Verify ECDSA signature is Admin class
 * 
 * @param ecdsa_data The ECDSA signature and hash message to be verified
 * @return true Verification passed
 * @return false Verification failed
 */
bool app_func_auth_verify_sign_admin(ECDSA_Data_t ecdsa_data);

/**
 * @brief Compare the hash message from FRAM and the ECDSA signature
 *
 * @param img_size The size of the firmware image to verify
 * @return uint8_t* Pointer of the number of failed verifications, 0 means verification passed. It is reset when verifying the ECDSA signature.
 */
uint8_t* app_func_auth_compare_fram_hash(uint32_t img_size);

/**
 * @brief Compare the hash message from Flash and the image info
 *
 * @param img_size The size of the firmware image to verify
 * @param img_hash The hash of the firmware image to verify
 * @return true Verification passed
 * @return false Verification failed
 */
bool app_func_auth_compare_flash_hash(uint32_t img_size, uint8_t* img_hash);

/**
 * @brief When establishing a BLE connection, verify and confirm user class.
 * 
 * @param ecdsa_data The ECDSA signature and hash message to be verified
 * @return uint8_t User class
 */
uint8_t app_func_auth_user_class_get(ECDSA_Data_t ecdsa_data);

#endif /* FUNCTIONS_INC_APP_AUTHENTICATION_H_ */
