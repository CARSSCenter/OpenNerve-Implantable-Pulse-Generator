/**
 * @file app_mode_therapy_session.h
 * @brief This file contains all the function prototypes for the app_mode_therapy_session.c file
 * @copyright Copyright (c) 2024
 */
#ifndef APP_MODE_THERAPY_SESSION_H_
#define APP_MODE_THERAPY_SESSION_H_
#include <stdbool.h>

/**
 * @brief Start therapy session and check for short circuit events
 *
 * @return true There is no short circuit event and then therapy session start
 * @return false A short circuit event exists, so therapy session is stopped
 */
bool app_mode_therapy_start(void);

/**
 * @brief Stop therapy session
 *
 */
void app_mode_therapy_stop(void);

/**
 * @brief Confirm therapy session status
 *
 * @return true Therapy session has been activated
 * @return false The therapy session is not active
 */
bool app_mode_therapy_confirm(void);

/**
 * @brief Handler for therapy session mode
 *
 */
void app_mode_therapy_handler(void);

#endif /* APP_MODE_THERAPY_SESSION_H_ */
