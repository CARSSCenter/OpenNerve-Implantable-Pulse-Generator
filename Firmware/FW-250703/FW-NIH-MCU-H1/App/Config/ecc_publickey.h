/**
 * @file ecc_publickey.h
 * @brief Placeholder ECC public keys for firmware integration
 *
 * @details
 * This header defines placeholder ECC public keys used in the firmware.
 * All key data has been zeroed out for security and compliance with
 * open-source distribution or internal sharing policies.
 *
 * The ECC-related data structures and access mechanisms remain intact.
 * Users must replace the zeroed key values with their own valid ECC public keys
 * before building or deploying the firmware.
 *
 * @note This is a sanitized version and does not contain any sensitive key material.
 *
 * @copyright Copyright (c) 2025
 */
#ifndef INC_APP_CONFIG_ECC_PUBLICKEY_H_
#define INC_APP_CONFIG_ECC_PUBLICKEY_H_
#include <stdint.h>

#define	APP_ECC_PUBKEY_QX_ADMIN							\
{														\
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		\
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		\
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		\
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		\
}														/*!< ECC public key point xQ for administrator class */

#define	APP_ECC_PUBKEY_QY_ADMIN							\
{														\
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		\
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		\
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		\
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		\
}														/*!< ECC public key point yQ for administrator class */

#define	APP_ECC_PUBKEY_QX_CLINICIAN						\
{														\
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		\
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		\
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		\
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		\
}														/*!< ECC public key point xQ for clinician class */

#define	APP_ECC_PUBKEY_QY_CLINICIAN						\
{														\
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		\
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		\
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		\
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		\
}														/*!< ECC public key point yQ for clinician class */

#define	APP_ECC_PUBKEY_QX_PATIENT						\
{														\
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		\
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		\
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		\
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		\
}														/*!< ECC public key point xQ for patient class */

#define	APP_ECC_PUBKEY_QY_PATIENT						\
{														\
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		\
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		\
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		\
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		\
}														/*!< ECC public key point yQ for patient class */

typedef struct {
	uint8_t PublicKeyQx[32]; /*!< ECC public key point xQ */
	uint8_t PublicKeyQy[32]; /*!< ECC public key point yQ */
} ECC_PublicKey_t;

#define LEN_ECDSA_PUBKEY	sizeof(ECC_PublicKey_t)		/*!< The length of the ECC public key */

#endif /* INC_APP_CONFIG_ECC_PUBLICKEY_H_ */
