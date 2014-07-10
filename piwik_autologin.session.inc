<?php
/**
 * @file
 * piwik_autologin.session.inc
 * @brief Class managing session variables in the module.
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

/**
 * Entity managing session variables used by the module.
 */
class Session {
  /**
   * Creating the $_SESSION['piwik_autologin'] array to store Piwik Autologin's session variables.
   */
  protected static function initSession() {
    !isset($_SESSION['piwik_autologin']) AND $_SESSION['piwik_autologin'] = array();
  }

  /**
   * Getting the logme URL.
   *
   * @param mixed $default_url
   *   A default value to return if the URL is not stored in the session.
   *
   * @return mixed
   *   A string with the URL if the logme URL is stored in the session, $default_url otherwise.
   */
  public static function getLogmeURL($default_url = NULL) {
    static::initSession();
    return isset($_SESSION['piwik_autologin']['logme_url']) ? $_SESSION['piwik_autologin']['logme_url'] : $default_url;
  }

  /**
   * Setting the logme URL.
   *
   * @param string $new_url
   *   New value for the logme URL.
   */
  public static function setLogmeURL($new_url) {
    static::initSession();
    $_SESSION['piwik_autologin']['logme_url'] = $new_url;
  }

  /**
   * Unsetting the logme URL.
   */
  public static function delLogmeURL() {
    unset($_SESSION['piwik_autologin']['logme_url']);
  }

  /**
   * Knowing if the link to Piwik has to be shown in the navigation menu.
   *
   * @return bool
   *   TRUE if it has to be shown, FALSE otherwise.
   */
  public static function getShowNaviLink($default_show_navi_link = NULL) {
    static::initSession();
    return isset($_SESSION['piwik_autologin']['show_navi_link']) AND $_SESSION['piwik_autologin']['show_navi_link'];
  }

  /**
   * Setting a new value for determining if the link to Piwik has to be shown in the navigation menu.
   *
   * @param bool $new_show_navi_link
   *   New value for showing the link to Piwik in the navigation menu. TRUE if the
   *   link has to be shown, FALSE otherwise.
   */
  public static function setShowNaviLink($new_show_navi_link) {
    static::initSession();
    !isset($_SESSION['piwik_autologin']) AND $_SESSION['piwik_autologin'] = array();
    $_SESSION['piwik_autologin']['show_navi_link'] = $new_show_navi_link;
  }

  /**
   * Unsetting all session variables used by the module.
   */
  public static function erase() {
    unset($_SESSION['piwik_autologin']);
  }
}
