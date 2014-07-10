<?php
/**
 * @file
 * piwik_autologin.piwik_users.inc
 * @brief Representing a Piwik user on Drupal.
 * @author Romain Ducher <r.ducher@agence-codecouleurs.fr>
 * @license https://www.gnu.org/licenses/gpl-3.0.txt
 * @section LICENSE
 *
 * Copyright 2014 Code Couleurs
 *
 * This file is part of Piwik Autologin.
 *
 * Piwik Autologin is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Piwik Autologin is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS OR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Piwik Autologin. If not, see <http://www.gnu.org/licenses/>.
 */

namespace piwik_autologin;

if (!function_exists('\piwik_autologin\get_piwik_idsite')) {
  require_once 'piwik_autologin.utils.inc';
}

/**
 * Representing a Piwik user.
 */
class PiwikUser {
  /** Table ID. */
  protected $id;

  /** Login on Piwik. */
  protected $userlogin;

  /** Piwik Password's MD5. */
  protected $md5password;

  /** Drupal user ID. */
  protected $uid;

  /** Name of the table containing Piwik Users in Drupal. */
  protected static $tableName = 'piwik_autologin_piwik_users';

  /**
   * Constructor.
   *
   * @param string $userlogin
   *   Login on Piwik.
   * @param string $password
   *   Password on Piwik, clear or MD5.
   * @param int $drupal_uid
   *   Drupal user ID.
   * @param int $id
   *   Piwik user ID in Drupal tables.
   * @param bool $clear_password
   *   TRUE if the password is clear, FALSE otherwise.
   */
  public function __construct($userlogin, $password, $drupal_uid, $id = 0, $clear_password = TRUE) {
    $this->userlogin = $userlogin;
    $this->md5password = $clear_password ? md5($password) : $password;
    $this->uid = intval($drupal_uid);
    $this->id = intval($id);
  }

  /**
   * Getter for $this->userlogin.
   *
   * @return string
   *   $this->userlogin.
   */
  public function getUserlogin() {
    return $this->userlogin;
  }

  /**
   * Setter for $this->userlogin.
   *
   * @param string $new_userlogin
   *   New value for $this->userlogin.
   */
  public function setUserlogin($new_userlogin) {
    $this->userlogin = $new_userlogin;
  }

  /**
   * Getter for $this->md5password.
   *
   * @return string
   *   $this->md5password.
   */
  public function getMD5Password() {
    return $this->md5password;
  }

  /**
   * Setter for $this->md5password.
   *
   * @param string $new_password
   *   New value for $this->md5password.
   * @param bool $is_password_clear
   *   TRUE if the password is clear, FALSE if it is MD5-encoded.
   */
  public function setMD5Password($new_password, $is_password_clear) {
    $this->md5password = $is_password_clear ? md5($new_password) : $new_password;
  }

  /**
   * Getter for $this->uid.
   *
   * @return int
   *   $this->uid.
   */
  public function getDrupalUID() {
    return $this->uid;
  }

  /**
   * Setter for $this->uid.
   *
   * @param int $new_uid
   *   New value for $this->uid.
   */
  public function setDrupalUID($new_uid) {
    $this->uid = $new_uid;
  }

  /**
   * Saving a Piwik user in the database.
   *
   * @return bool
   *   TRUE if it was successful, FALSE otherwise.
   */
  public function insert() {
    try {
      $this->id = db_insert(static::$tableName)->fields($this->toArray())->execute();
      return TRUE;
    }
    catch (Exception $e) {
      // Logging the exception.
      watchdog_exception('user', $e);
      return FALSE;
    }
  }

  /**
   * Updating a Piwik user in the database.
   *
   * @return bool|int
   *   The db_update result or FALSE if an exception occurs.
   */
  public function update() {
    try {
      return db_update(static::$tableName)
        ->fields($this->toArray(FALSE))
        ->condition('id', $this->id, '=')
        ->execute();
    }
    catch (Exception $e) {
      // Logging the exception.
      watchdog_exception('user', $e);
      return FALSE;
    }
  }

  /**
   * Deleting a Piwik user in the database.
   *
   * @return bool|mixed
   *   The db_delete result or FALSE if an exception occurs.
   */
  public function delete() {
    try {
      return db_delete(static::$tableName)
        ->condition('id', $this->id, '=')
        ->execute();
    }
    catch (Exception $e) {
      // Logging the exception.
      watchdog_exception('user', $e);
      return FALSE;
    }
  }

  /**
   * Retrieving a Piwik User with a Drupal User ID.
   *
   * @param int $uid
   *   Drupal user ID.
   *
   * @return NULL|PiwikUser
   *   The corresponding Piwik User or NULL if it does not exist.
   */
  public static function getByDrupalUserID($uid) {
    $result = db_query(sprintf(
      "SELECT * FROM %s where uid = %d",
      static::$tableName,
      intval($uid)
    ))->fetchObject();

    if ($result) {
      return new PiwikUser($result->userlogin, $result->md5password, $result->uid, $result->id, FALSE);
    }
    else {
      return NULL;
    }
  }

  /**
   * Retrieving a Piwik equivalent of the connected user.
   *
   * @return NULL|PiwikUser
   *   The Piwik equivalent of the connected user.
   */
  public static function getByConnectedUser() {
    global $user;
    return static::getByDrupalUserID($user->uid);
  }

  /**
   * Checks if a Drupal user has got a Piwik equivalent.
   *
   * @param int $uid
   *   Drupal user ID.
   */
  public static function piwikUserExists($uid) {
    $result = db_query(sprintf(
      "SELECT COUNT(*) FROM %s where uid = %d",
      static::$tableName,
      intval($uid)
    ))->fetchField();

    return $result == 1;
  }

  /**
   * Converting the object into an array.
   *
   * @param bool $with_db_id
   *   TRUE if the database ID has to be included in the array FALSE otherwise.
   *
   * @return array
   *   Array whose keys are attributes names and whose values are their values.
   */
  public function toArray($with_db_id = TRUE) {
    $res = array(
      'userlogin' => $this->userlogin,
      'md5password' => $this->md5password,
      'uid' => $this->uid,
    );

    if ($with_db_id) {
      $res['id'] = $this->id;
    }

    return $res;
  }

  /**
   * Building an URL to sign in a user to Piwik using Piwik's logme feature.
   *
   * It just builds the URL following Piwik inscructions for logme. This method
   * does not ensure that the URL is valid or that the user will be logged into
   * Piwik using it. However it should be OK if all the Piwik requirements
   * are respected.
   *
   * @see http://piwik.org/faq/how-to/faq_30/
   * @return string
   *   The corresponding logme URL
   */
  public function getLogmeURL() {
    $piwik = get_piwik_instance();
    return $piwik->get_logme_url($this->userlogin, $this->md5password, variable_get('site_name', ''), get_piwik_idsite(), FALSE);
  }

  /**
   * Is the Piwik user empty? It is empty if its login is an empty string.
   *
   * @return bool
   *   TRUE if the user is empty, FALSE otherwise.
   */
  public function isEmpty() {
    return empty($this->userlogin) OR $this->md5password == md5('');
  }

  /**
   * Retrieving a Piwik userlogin with a Drupal user ID.
   *
   * @param int $drupal_uid
   *   Drupal user ID.
   *
   * @return NULL|string
   *   The correspoding userlogin if the Piwik user exists, NULL otherwise.
   */
  public static function getUserloginByDrupalUID($drupal_uid) {
    $piwik_user = static::getByDrupalUserID($drupal_uid);
    return is_null($piwik_user) ? NULL : $piwik_user->userlogin;
  }
}
