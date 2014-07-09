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
    if (!class_exists('\cc_piwik\CC_Piwik')) {
      require_once 'libs/cc_piwik/cc_piwik.php';
    }

    $piwik = new \cc_piwik\CC_Piwik(variable_get('piwik_autologin_piwik_url', ''));
    return $piwik->get_logme_url($this->userlogin, $this->md5password, variable_get('site_name', ''), get_piwik_idsite(), FALSE);
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
   * @return bool
   *   TRUE if it updates only the desired Piwik user in the database,
   *   FALSE otherwise.
   */
  public function update() {
    try {
      $update_pu = db_update(static::$tableName)
        ->fields($this->toArray(FALSE))
        ->condition('id', $this->id, '=')
        ->execute()->fetchField();
      return $update_pu == 1;
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
   * @return mixed
   *   The db_delete result.
   */
  public function delete() {
    try {
      $delete_pu = db_delete(static::$tableName)
        ->condition('id', $this->id, '=')
        ->execute()->fetchField() == 1;
      return $delete_pu == 1;
    }
    catch (Exception $e) {
      // Logging the exception.
      watchdog_exception('user', $e);
      return FALSE;
    }
  }
}
